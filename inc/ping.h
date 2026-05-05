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

typedef struct s_packet {
    struct icmphdr hdr;
    char msg[56];
} t_packet;

typedef struct s_ping {
	struct sockaddr_in sa;
	char	ip[INET_ADDRSTRLEN];
	char	*fqdn;
	int		sockfd;
} t_ping;

uint16_t    calculate_checksum(void* addr, int len);
void        build_icmp_packet(t_packet *pkt, int sequence_number);