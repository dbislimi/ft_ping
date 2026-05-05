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
