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

#include "sound.h"

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
	hints.ai_family = AF_UNSPEC;
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
	printf("ret: %d\n", ret);

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

	int sockfd;
	const char *port = "4320";
	
	client_listen(&sockfd, port);
	snd_pcm_t *handlep = init_playback_handle();

	size_t buff_size = alsa_dev.period_time * alsa_dev.fmt_size * alsa_dev.channels_n;
	uint8_t *inet_buff = malloc(buff_size);


	while(true){
		int recvd = recieve_data(sockfd, inet_buff, buff_size);
		printf("recvd: %d\n", recvd);

		size_t nframes = recvd / alsa_dev.channels_n / alsa_dev.fmt_size;

		playback_data(alsa_dev, handlep, inet_buff, nframes);
	}


	free(inet_buff);
	close(sockfd);

	return 0;
}

