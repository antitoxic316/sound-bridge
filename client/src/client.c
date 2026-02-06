#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <pthread.h>

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


void client_listen(int *sockfd, const char* port){
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((*sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (bind(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: bind");
			close(*sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to bind\n");
		exit(-1);
	}

	freeaddrinfo(servinfo);
}

int recieve_data(int sockfd, uint8_t *buf, size_t n){
	struct sockaddr_storage sender_addr;
	socklen_t sender_addrlen = sizeof(sender_addr);

	int ret = 0;
	if((ret = recvfrom(sockfd, buf, n, 0,
			(struct sockaddr*) &sender_addr, &sender_addrlen)) == -1){
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
	snd_pcm_t *handlep;
	struct shared_buffer *sh_buff;
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

	while(true){
		size_t recvd = shared_buffer_read(job_info->sh_buff, tmp_buff, tmp_buff_len, 1);

		size_t nbytes = playback_data(
			job_info->sink_info,
			job_info->handlep, 
			tmp_buff, 
			recvd / sink_info->channels_n / sink_info->fmt_size
		);
		if(nbytes)
			dbg_printf(DEBUG_LOG_ALSA_IO, "playbacked %ld bytes\n", nbytes);
	}

	free(tmp_buff);
	return NULL;
}

struct inet_thread_job_args {
	int sockfd;
	struct shared_buffer *sh_buff;
	size_t tmp_buff_len;
};

void *inet_job_threaded(void *arg){
	struct inet_thread_job_args *job_info = arg;

	uint8_t *tmp_buff = malloc(job_info->tmp_buff_len);
	if(!tmp_buff){
		perror("malloc");
		exit(1);
	}

	while(true){
		int	r = recieve_data(
			job_info->sockfd, 
			tmp_buff, 
			job_info->tmp_buff_len
		);
		if(r)
			dbg_printf(DEBUG_LOG_INET_IO, "sent %d bytes\n", r);

		r = shared_buffer_write(job_info->sh_buff, tmp_buff, r, 1);
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

	int sockfd;
	const char *port = "4320";
	
	client_listen(&sockfd, port);
	snd_pcm_t *handlep = init_playback_handle();

	struct shared_buffer *sh_buff = shared_buffer_init(alsa_dev.buffer_time);

	int ret;
	struct inet_thread_job_args inet_job_info = {
		.sockfd = sockfd,
		.sh_buff = sh_buff,
		.tmp_buff_len =  alsa_dev.period_time
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

	struct alsa_thread_job_args alsa_job_info = {
		.sink_info = &alsa_dev,
		.handlep = handlep,
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

	pthread_join(inet_thread, NULL); //suspend until some network error
	pthread_join(alsa_thread, NULL);

	shared_buffer_free(sh_buff);
	close(sockfd);

	return 0;
}

