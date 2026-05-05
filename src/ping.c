#include "../inc/ping.h"

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
	if (inet_pton(AF_INET, ip_or_fqdn, &p_env->sa.sin_addr.s_addr) == 1)
		p_env->fqdn = NULL;
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

double	calcul_time(struct timespec t1, struct timespec t2){
	double ds = t2.tv_sec - t1.tv_sec;
	double dns = t2.tv_nsec - t1.tv_nsec;
	return ds * 1.e3 + dns * 1.e-6;
}

void sending(t_ping *p_env){
	struct sockaddr_in sender_addr;
	socklen_t sender_len = sizeof(sender_addr);
	t_packet pkt;
	char buf[IP_MAXPACKET]; //stock complet ip pkt (ip header + icmp)
	struct iphdr *ip_hdr = (struct iphdr *)buf;

	for (int i = 1; i < 3; ++i){
		int pkt_len;
		int hdr_len;
		struct icmphdr *icmp_hdr;
		struct timespec recvt;

        build_icmp_packet(&pkt, i);
		if (i == 1){
			if (p_env->fqdn == NULL)
				printf("PING %s (%s) %d(%d) bytes of data.\n", p_env->ip, p_env->ip, sizeof(pkt.msg), sizeof(pkt) + sizeof(struct iphdr));
			else
				printf("PING %s (%s) %d(%d) bytes of data.\n", p_env->fqdn, p_env->ip, sizeof(pkt.msg), sizeof(pkt) + sizeof(struct iphdr));
		}
        sendto(p_env->sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&p_env->sa, sizeof(p_env->sa));
		pkt_len = recvfrom(p_env->sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&sender_addr, &sender_len);
		clock_gettime(CLOCK_MONOTONIC, &recvt);
		if (pkt_len == -1){
			perror("Error from recvfrom.");
			exit(1);
		}
		hdr_len = ip_hdr->ihl * 4;
		icmp_hdr = (struct icmphdr *)(buf + hdr_len); //skip the ipv4 header
		if (icmp_hdr->un.echo.id != pkt.hdr.un.echo.id){
			fprintf(stderr, "Info: not the right id.");
			continue;
		}
		if (icmp_hdr->type != ICMP_ECHOREPLY){
			fprintf(stderr, "Info: not ECHOREPLY.");
			continue;
		}
		// payload = (char *)icmp_hdr + sizeof(struct icmphdr);
		double ms = calcul_time(*(struct timespec *)pkt.msg, recvt); 
		int icmp_len = pkt_len - hdr_len;
		// printf("%d\n", icmp_hdr->un.echo.sequence);
		uint8_t ttl = ip_hdr->ttl;
		if (p_env->fqdn == NULL)
			printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%f ms\n", icmp_len, p_env->ip, icmp_hdr->un.echo.sequence, ttl, ms);
		else
			printf("%d bytes from %s (%s): icmp_seq=%d ttl=%d time=%f ms\n", icmp_len, p_env->fqdn, p_env->ip, icmp_hdr->un.echo.sequence, ttl, ms);

    }
}

void ping(char *ip_or_fqdn){
	t_ping ping_env;
	int socket_fd = -1;
	struct addrinfo *res;

    ping_env.sockfd = open_rawsocket();
	init_sockaddr(&ping_env, ip_or_fqdn);
    sending(&ping_env);
	freeaddrinfo(res);
}

int main(int ac, char **av){
    if (ac != 2){
        fprintf(stderr, "Error: should have 1 argument.\n");
        exit(1);
    }
    ping(av[1]);
	return 0;
}
