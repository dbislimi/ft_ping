#include "../inc/ping.h"

void	handle_echo_reply(t_packet *pkt, t_ctx *ctx){
	int icmp_len = ctx->pkt_len - ctx->ip->ihl * 4;

	if (ctx->p_env->fqdn == NULL)
		printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n", icmp_len, ctx->p_env->ip, ctx->icmp->un.echo.sequence, ctx->ip->ttl, ctx->time);
	else
		printf("%d bytes from %s (%s): icmp_seq=%d ttl=%d time=%.3f ms\n", icmp_len, ctx->p_env->fqdn, ctx->p_env->ip, ctx->icmp->un.echo.sequence, ctx->ip->ttl, ctx->time);
}

void	handle_dest_unread(t_packet *pkt, t_ctx *ctx){

}

void	handle_time_exceeded(t_packet *pkt, t_ctx *ctx){

}

void	handle_unknow(t_packet *pkt, t_ctx *ctx){

}
