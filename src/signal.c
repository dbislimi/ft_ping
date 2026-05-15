#include "../inc/ping.h"

void sigint_handler(int sig){
	(void)sig;
	FLAG_SIGINT = 0;
}
