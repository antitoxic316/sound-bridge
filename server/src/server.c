/*
** server.c -- a stream socket server demo
** Origin: http://beej.us/guide/bgnet/examples/server.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <sched.h>

#include "sound.h"
#include "sharedbuffer.h"
#include "debug.h"

extern struct alsa_info alsa_dev;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void init_server_socket(int *sockfd, const char* cli_hostname, const char* port, struct sockaddr *cli_addr, socklen_t* cli_addrlen){
	struct addrinfo hints, *p, *servinfo;
	struct sigaction sa;
	int yes=1;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(cli_hostname, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((*sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
		if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		*cli_addrlen = p->ai_addrlen;
		memcpy(cli_addr, p->ai_addr, p->ai_addrlen);
		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}
}

int send_data(int sockfd, uint8_t *buf, size_t n, struct sockaddr* dest_addr, socklen_t addr_len){
	int ret = 0;
	if((ret = sendto(sockfd, buf, n, 0,
			dest_addr, addr_len)) == -1){
		perror("recvfrom");
		exit(1);
	}

	return ret;
}


void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}

struct alsa_thread_job_args {
	struct alsa_info *sink_info;
	snd_pcm_t *handlec;
	struct shared_buffer *sh_buff;
	size_t tmp_buff_len;
};

void *alsa_job_threaded(void *arg){
	struct alsa_thread_job_args *job_info = arg;
	struct alsa_info *sink_info = job_info->sink_info;

	size_t tmp_buff_len = sink_info->period_time * sink_info->fmt_size * sink_info->channels_n;
	uint8_t *tmp_buff = malloc(tmp_buff_len);
	if(!tmp_buff){
		perror("malloc");
		exit(1);
	}

	while(true) {
		size_t nbytes = capture_data(
			sink_info,
			job_info->handlec, 
			tmp_buff, 
			tmp_buff_len / sink_info->channels_n / sink_info->fmt_size
		);
		dbg_printf(DEBUG_LOG_ALSA_IO, "captured %ld bytes\n", nbytes);

		nbytes = shared_buffer_write(job_info->sh_buff, tmp_buff, nbytes, 1);
	}

	free(tmp_buff);
	return NULL;
}

struct inet_thread_job_args {
	int sockfd;
	struct shared_buffer *sh_buff;
	size_t tmp_buff_len;
	struct sockaddr *dst_addr;
	socklen_t dst_addrlen;
};

void *inet_job_threaded(void *arg){
	struct inet_thread_job_args *job_info = arg;

	uint8_t *tmp_buff = malloc(job_info->tmp_buff_len);

	while(true){
		int ret = shared_buffer_read(job_info->sh_buff, tmp_buff, job_info->tmp_buff_len, 1);

		int	r = send_data(
			job_info->sockfd, 
			tmp_buff, 
			ret, 
			job_info->dst_addr, 
			job_info->dst_addrlen
		);
		//printf("sent %d bytes\n", r);

	}

	free(tmp_buff);
	return NULL;
}


int main(int argc, char *argv[])
{
	struct sigaction sa;
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	//from utils/debug.h
	debug_type_bitmap = 0x0F; //0b00001111 all debug levels on 


	printf("%d\n", argc);

	int sockfd;
	char *hostname = "127.0.0.1";
	struct sockaddr cli_addr;
	socklen_t cli_addrlen;
	const char *port = "4320";

	init_server_socket(&sockfd, hostname, port, &cli_addr, &cli_addrlen);

	snd_pcm_t *handlec = init_capture_handle();	

	struct shared_buffer *sh_buff = shared_buffer_init(alsa_dev.buffer_time);

	int ret;

	struct alsa_thread_job_args alsa_job_info = {
		.sink_info = &alsa_dev,
		.handlec = handlec,
		.sh_buff = sh_buff,
	};

	pthread_t alsa_thread;
	ret = pthread_create(
		&alsa_thread, 
		NULL, 
		alsa_job_threaded, 
		(void *) &alsa_job_info
	);
	if(ret){
		perror("pthread_create");
		exit(1);
	}


	struct inet_thread_job_args inet_job_info = {
		.sockfd = sockfd,
		.dst_addr = &cli_addr,
		.dst_addrlen = cli_addrlen,
		.tmp_buff_len = alsa_dev.period_time,
		.sh_buff = sh_buff,
	};

	pthread_t inet_thread;
	ret = pthread_create(
		&inet_thread, 
		NULL, 
		inet_job_threaded, 
		(void *) &inet_job_info
	);
	if(ret){
		perror("pthread_create");
		exit(1);
	}

	pthread_join(inet_thread, NULL); //suspend until some network error
	pthread_join(alsa_thread, NULL);

	close(sockfd);
	shared_buffer_free(sh_buff);
	
	return 0;
}
