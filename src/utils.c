#include "../inc/ping.h"

uint16_t    calculate_checksum(void* addr, int len){
    uint16_t *ptr = (u_int16_t *)addr;
    uint32_t sum = 0;

    while (len > 1){
        sum += *ptr++;
        len -= 2;
    }
    if (len == 1)
        sum += *(uint8_t *)ptr;
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);
    return (uint16_t)(~sum);
}

void    build_icmp_packet(t_packet *pkt, int sequence_number){
    struct timespec ts;
    int packet_size = sizeof(*pkt);
    
	if (sequence_number == 0){
        bzero(pkt, packet_size);
        pkt->hdr.type = ICMP_ECHO;
        pkt->hdr.code = 0;
        pkt->hdr.un.echo.id = getpid();
    }
    pkt->hdr.un.echo.sequence = sequence_number;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    memset(pkt->msg, 0, sizeof(pkt->msg));
    memcpy(pkt->msg, &ts, sizeof(ts));
    pkt->hdr.checksum = 0;
    pkt->hdr.checksum = calculate_checksum(pkt, packet_size);
	
}

void	init_penv(t_ping *p, char *ip_or_fqdn, t_config *conf){
	init_sockaddr(p, ip_or_fqdn);
	p->pid = getpid();
	for (int i = 0; i < 256; ++i){
		p->handlers[i] = handle_unknow;
	}
	p->handlers[ICMP_ECHOREPLY] = handle_echo_reply;
	p->handlers[ICMP_DEST_UNREACH] = handle_dest_unread;
	p->handlers[ICMP_TIME_EXCEEDED] = handle_time_exceeded;
	p->config = conf;
}

double	calcul_time(struct timespec t1, struct timespec t2){
	double ds = t2.tv_sec - t1.tv_sec;
	double dns = t2.tv_nsec - t1.tv_nsec;
	return ds * 1.e3 + dns * 1.e-6;
}

void	parse_packet(char *buf, t_ctx *ctx){
	ctx->ip = (struct iphdr *)buf;
	ctx->icmp = (struct icmphdr *)(buf + ctx->ip->ihl * 4);
}

int	is_it_for_us(t_ping *p, struct icmphdr *hdr){
	if (hdr->un.echo.id != p->pid)
		return 0;
	return 1;
}

int open_rawsocket(){
	int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (socket_fd < 0){
        perror("Error while creating the socket");
        fprintf(stderr, "You need root permissions to create a raw socket.\n");
        exit(1);
    }
	return socket_fd;
}
void	init_sockaddr(t_ping *p_env, char *ip_or_fqdn){
	struct addrinfo *res;
	struct addrinfo hints;
	int code;

	memset(&p_env->sa, 0, sizeof(p_env->sa));
	p_env->sa.sin_family = AF_INET;
	p_env->fqdn = ip_or_fqdn;
	if (inet_pton(AF_INET, ip_or_fqdn, &p_env->sa.sin_addr.s_addr) == 1)
		;
	else {
		p_env->fqdn = ip_or_fqdn;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_RAW;
		code = getaddrinfo(ip_or_fqdn, NULL, &hints, &res);
		if (code){
			fprintf(stderr, "Error: %s\n", gai_strerror(code));
			exit(1);
		}
		p_env->sa = *(struct sockaddr_in *)res->ai_addr;
		freeaddrinfo(res);
	}
	const char *e = inet_ntop(AF_INET, &p_env->sa.sin_addr, p_env->ip, INET_ADDRSTRLEN);
	if (e == NULL){
		perror("Error: inet_ntop");
		exit(1);
	}
}

void	init_stats(t_stats *s, int *sequence){
	s->received = 0;
	s->transmitted = sequence;
	s->max = 0;
	s->min = 1e4;
	s->sum = 0;
	s->sumsq = 0;
}

void	update_stats(t_stats *s, double roundtrip){
	++s->received;
	if (roundtrip > s->max)
		s->max = roundtrip;
	if (roundtrip < s->min)
		s->min = roundtrip;
	s->sum += roundtrip;
	s->sumsq += roundtrip * roundtrip;
}

void	display_stats(t_stats *s, char *fqdn){
	int transmitted = *s->transmitted;
	int loss = (transmitted > 0)
		? (int)(((double)(transmitted - s->received) / transmitted) * 100.0)
		: 0;
	printf("--- %s ping statistics ---\n%d packets transmitted, %d packets received, %d%% packet loss\n", 
		fqdn,
		transmitted,
		s->received,
		loss);
	if (s->received == 0)
		return ;
	double avg = s->sum / s->received;
	double var = (s->sumsq / s->received) - avg * avg;
	double stddev = sqrt(fmax(var, 0.0));
	printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f\n",
		s->min,
		avg,
		s->max,
		stddev);
}

void display_help(){
	puts("Usage: ping [OPTION...] HOST ...\n"
		"Send ICMP ECHO_REQUEST packets to network hosts.\n"
		"\nOptions valid for all request types:\n\n"
		"-v, --verbose              verbose output\n"
		"\nOptions valid for --echo requests:\n\n"
		"-?, --help                 give this help list\n"
		"\nMandatory or optional arguments to long options are also mandatory or optional\nfor any corresponding short options.\n"
		"\nOptions marked with (root only) are available only to superuser.\n"
		"\nReport (unexistant) bugs to <dbislimi@student.42nice.fr>."
		);
}

int	parse_args(int ac, char **av, t_config *conf){
	const struct option long_opt[] = {
		{"verbose", no_argument, 0, 'v'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};
	int opt;
	opterr = 0;
	while ((opt = getopt_long(ac, av, "v", long_opt, NULL)) != -1)
	{
		printf("optopt: %c\nopt: %c\n", optopt, opt);
		switch (opt){
			case 'v':
				conf->verbose = 1;
				break;
			case 'h':
				display_help();
				exit(0);
			case '?':
				if (optopt == '?'){
					display_help();
					exit(0);
				}
				fprintf(stderr, "Try 'ping --help' for more information.\n");
				exit(64);
				break;
		}
	}
	if (optind + 1 != ac){
		fprintf(stderr, "Error: program should have only 1 argument.\n");
		exit(1);
	}
	return optind;
}
