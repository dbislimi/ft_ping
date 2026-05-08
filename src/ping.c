#include "../inc/ping.h"

volatile sig_atomic_t g_is_running = 1;

int handle_response(t_ping *p, t_packet *pkt, double time){
	struct sockaddr_in sender_addr;
	socklen_t	addrlen = sizeof(sender_addr);
	char buf[IP_MAXPACKET];
	struct timespec recvt;
	t_ctx			ctx;

	ctx.p_env = p;
	ctx.time = time;
	ctx.pkt_len = recvfrom(p->sockfd, &buf, sizeof(buf), 0, (struct sockaddr *)&sender_addr, &addrlen);
	if (ctx.pkt_len == -1){
		perror("Error from recvfrom.");
		exit(1);
	}
	parse_packet(buf, &ctx);
	if (is_it_for_us(p, ctx.icmp) == 0)
		return 0;
	p->handlers[ctx.icmp->type](pkt, &ctx);
	return 1;
}

void	start_loop(t_ping *p){
	t_stats	stats;
	t_packet pkt;
	struct pollfd pfd;
	struct sockaddr_in sender_addr;
	char buf[IP_MAXPACKET]; //stock complet ip pkt (ip header + icmp)

	p->sequence_number = 0;
	init_stats(&stats, &p->sequence_number);
	pfd.fd = p->sockfd;
	pfd.events = POLLIN;
	while (g_is_running){
		struct timespec recvt;
		int ready;
		double time;

		build_icmp_packet(&pkt, p->sequence_number++);
        sendto(p->sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&p->sa, sizeof(p->sa));
		ready = poll(&pfd, 1, 1e3);
		clock_gettime(CLOCK_MONOTONIC, &recvt);
		time = calcul_time(*(struct timespec *)pkt.msg, recvt);
		switch (ready){
			case -1:
				perror("poll");
				return ;
			case 0:
				printf("timeout\n");
				continue;
			case 1:
				if (handle_response(p, &pkt, time) == 0)
					continue;
		}
		double roundtrip = 1e3 - time;
		update_stats(&stats, roundtrip);
		usleep(roundtrip * 1e3);
	}
	display_stats(&stats, p->fqdn);
}

void ping(char *ip_or_fqdn){
	t_ping ping_env;

    ping_env.sockfd = open_rawsocket();
	init_penv(&ping_env, ip_or_fqdn);
	printf("PING %s (%s): %d data bytes\n",
		ping_env.fqdn,
		ping_env.ip,
		ICMP_PAYLOAD_SIZE);
    start_loop(&ping_env);
}

int main(int ac, char **av){
    if (ac != 2){
        fprintf(stderr, "Error: should have 1 argument.\n");
        exit(1);
    }
	struct sigaction sa;

	sa.sa_handler = sigint_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGINT, &sa, NULL) == -1){
		perror("Sigaction");
		return 1;
	}
    ping(av[1]);
	return 0;
}
