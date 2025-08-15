#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 2048 // max number of bytes we can get at once 
#define MSG_SIZE 64

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buf[MAXDATASIZE+1] = {'\0', };
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure


	for(int i = 0; i < 20; i++){

	char test_buff[2048] = {'\0',};

	//msg start header
	while(strcmp(test_buff, "\n\r\r")){
		int ret;
		ret = recv(sockfd, test_buff, 3, 0);
		if(ret == -1){
			perror("recv");
			exit(1);
		}
		
		/*
		if(recv(sockfd, test_buff, 3, 0) == -1){
			perror("recv");
			exit(1);
		}
		*/
	}

	int bytes_written = 0;
	int ret;
	while(1){
		if((ret = recv(sockfd, &test_buff[bytes_written], 64, 0)) == -1){
			perror("recv");
			exit(1);
		}
		
		bytes_written += ret;
		if(bytes_written >= 2048 - 64){
			perror("buffer overflow");
			exit(1);
		}

		printf("client buffer: %s\n", test_buff);

		//printf("end of buffer: %s\n", &test_buff[bytes_written-3]);

		if(!strcmp("\n\n\n", &test_buff[bytes_written-3])){
			break;
		}
	}

	if(send(sockfd, "\n\n\r", 3, 0) == -1){
		perror("send");
	}

	printf("client: received '%s'\n", test_buff);
	printf("%d\n", i);

	}

	close(sockfd);

	return 0;
}

