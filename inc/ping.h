#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <error.h>
#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <math.h>

#define ICMP_PAYLOAD_SIZE 56

extern volatile sig_atomic_t g_is_running;

void sigint_handler(int sig);

typedef struct s_ping t_ping;

typedef struct s_packet {
    struct icmphdr hdr;
    char msg[ICMP_PAYLOAD_SIZE];
} t_packet;


typedef struct s_ctx {
	int				pkt_len;
	double			time;
	struct icmphdr	*icmp;
	struct iphdr	*ip;
	t_ping			*p_env;
} t_ctx;

typedef void (*t_icmp_handler)(t_packet *, t_ctx *);

typedef struct s_ping {
	int		sockfd;
	struct sockaddr_in sa;
	char	ip[INET_ADDRSTRLEN];
	char	*fqdn;
	uint		sequence_number;
	int		pid;
	t_icmp_handler handlers[256];
} t_ping;

typedef struct s_stats {
	uint	*transmitted;
	uint	received;

	double	sum;
	double	sumsq;
	double	min;
	double	max;
	double	stddev;
}	t_stats;

uint16_t    calculate_checksum(void* addr, int len);
void        build_icmp_packet(t_packet *pkt, int sequence_number);
void		parse_packet(char *buf, t_ctx *ctx);
int			is_it_for_us(t_ping *p, struct icmphdr *hdr);
double		calcul_time(struct timespec t1, struct timespec t2);
void	init_penv(t_ping *p, char *ip_or_fqdn);

void	handle_echo_reply(t_packet *pkt, t_ctx *ctx);
void	handle_dest_unread(t_packet *pkt, t_ctx *ctx);
void	handle_time_exceeded(t_packet *pkt, t_ctx *ctx);
void	handle_unknow(t_packet *pkt, t_ctx *ctx);

void	init_sockaddr(t_ping *p_env, char *ip_or_fqdn);
int open_rawsocket();
void	init_stats(t_stats *s, int *sequence);
void	update_stats(t_stats *s, double roundtrip);
void	display_stats(t_stats *s, char *fqdn);