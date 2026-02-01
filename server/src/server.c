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

	printf("%d\n", argc);

	int sockfd;
	char *hostname = "127.0.0.1";
	struct sockaddr cli_addr;
	socklen_t cli_addrlen;
	const char *port = "4320";

	init_server_socket(&sockfd, hostname, port, &cli_addr, &cli_addrlen);

	snd_pcm_t *handlec = init_capture_handle();	

	void *read_buffer = malloc(alsa_dev.period_time * alsa_dev.fmt_size * alsa_dev.channels_n);
	uint32_t frames_to_capture = alsa_dev.period_time;

	while(true){
		size_t nbytes = capture_data(alsa_dev, handlec, read_buffer, frames_to_capture);
		printf("captured %d bytes\n", nbytes);

		int	r = send_data(sockfd, read_buffer, nbytes, &cli_addr, cli_addrlen);
		printf("sent %d bytes\n", nbytes);
	}

	
	free(read_buffer);
	close(sockfd);
	
	return 0;
}
