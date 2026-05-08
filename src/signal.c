#include "../inc/ping.h"

void sigint_handler(int sig){
	(void)sig;
	g_is_running = 0;
}
