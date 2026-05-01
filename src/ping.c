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

typedef struct s_packet {
    struct icmphdr hdr;
    char msg[64];
} t_packet;


uint16_t    calculate_checksum(void* addr, int len){
    uint16_t *ptr = (u_int16_t *)addr;
    uint32_t sum = 0;

    while (len > 1){
        sum += *ptr++;
        len -= 2;
    }
    if (len == 1){
        sum += *(uint8_t *)ptr;
    }
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);
    return (uint16_t)(~sum);
}

void    build_icmp_packet(t_packet *pkt, int sequence_number){
    struct timespec ts;
    int packet_size = sizeof(*pkt);
    
    if (sequence_number == 1){
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

int ping(){
    int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    t_packet pkt;
    struct sockaddr_in sa;

    if (socket_fd < 0){
        perror("Error while creating the socket");
        fprintf(stderr, "You need root permissions to create a raw socket.\n");
        exit(1);
    }
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    for (int i = 1; i < 5; ++i){
        build_icmp_packet(&pkt, i);
        sendto();
    }
    
}

int main(){
    ping();
}
