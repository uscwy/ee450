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
#include <sys/wait.h>
#include <iostream>

#define PORT 		"18200"
#define BACKLOG 	1024
#define MAX_EVENTS 	1024

struct thread_arg {
	int clientsock;
	struct sockaddr client;
};


using namespace std;

int sockfd = -1, efd = -1;
struct addrinfo hints, *addr = NULL;


void cleanup_and_exit(const char *errmsg) {
	
	/*print error message*/
	perror(errmsg);
	/*clean up resources*/
	if(sockfd >=0 )
		close(sockfd);
	if(efd >=0 )
		close(fd);
	if(addr != NULL) {
		freeaddrinfo(addr);
		addr = NULL;
	}
	/*exit */
	exit(EXIT_FAILURE);
}

void *thread_handler(void *arg) {
	struct thread_arg *p = arg;
	int clientfd, ret, len;
	uint32_t num;
	uint32_t result[32];
	unsigned int i,f;
	
	clientfd = p->clientsock;
	free(p);
	p = NULL;
	
	ret = recv(clientfd, &num, sizeof(uint32_t));
	if(ret < sizeof(uint32_t))
	{
		perror("recv");
		return NULL;
	}
	/*convert to host order*/
	num = ntohl(num);
	i = 0;
	/*prime factor decomposition*/
	while(num % 2 == 0) {
		result[i++] = htonl(2);
		num = num/2;
	}
	while(num > 1) {
		if(n % f ==0) {
			result[i++] = htonl(f);
			num = num/f;
		}
		else {
			f = f + 2;
		}
	}
	
	/*send result to client*/
	len = sizeof(uint32_t)*i;
	ret = send(clientsock, &result[0], len);
	if(ret != len) {
		perror("send");
	}
	
	/*close*/
	close(clientsock);
	
	return NULL;
}

int main() {
	struct epoll_event ev, events[MAX_EVENTS];
	int ret, nfds, n, addrlen, clentsock;
	pthread_attr_t attr;
	pthread_t tid;
	struct thread_arg *arg;
	struct sockaddr client;
	
	memset(&hint, 0, sizeof(hints));
	
	if((ret = getaddrinfo(NULL, PORT, &hints, &addr)) != 0) {
		cleanup_and_exit("getaddrinfo");
	}
	/*creat socket descriptor*/
	if((sockfd = socket(addr->ai_family, addr->ai_socktype, 
			addr->ai_protocol)) < 0)
	{
		cleanup_and_exit("socket");
	}
	/*reserve port on server*/
	if(bind(sockfd, addr->ai_address, addr->ai_addrlen) == -1) {
		cleanup_and_exit("bind");
	}
	freeaddrinfo(addr);
	addr = NULL;
	
	/*start to listen*/
	if(listen(sockfd, BACKLOG) == -1) {
		cleanup_and_exit("listen");
	}
	
	/*use epoll to monitor all sockets for better performance*/
	if((efd = epoll_create(BACKLOG)) < 0) {
		cleanup_and_exit("epoll_create");
	}
	
	ev.events = EPOLLIN;
	ev.data.fd = sockfd;
	if (epoll_ctl(efd, EPOLL_CTL_ADD, sockfd, &ev) == -1) {
		cleanup_and_exit("epoll_ctl");
	}
	/*initialize pthread attr*/
	if((s = pthread_attr_init(&attr)) == -1) {
		cleanup_and_exit("pthread_attr_init");
	}
	
	for(;;) {
		/*use epoll to monitor all sockets*/
		nfds = epoll_wait(efd, events, MAX_EVENTS, -1);
		if(nfds == -1) {
			cleanup_and_exit("epoll_wait");
		}
		for(n = 0; n < nfds; n++) {
			if(events[n].data.fd == sockfd) {
				/*accept new client*/
				clentsock = accept(sockfd, &client, &addrlen);
				if(clientsock == -1) {
					cleanup_and_exit("accept");;
				}
				ev.events = EPOLLIN;
				ev.data.fd = clientsock;
				/*add client socket to epoll monitoring list*/
				if (epoll_ctl(efd, EPOLL_CTL_ADD, clientsock, &ev) == -1) {
					cleanup_and_exit("epoll_ctl: conn_sock");
				}
			}
			else {
				/*client data is ready to read, create new thread to handle it*/
				parg = malloc(sizeof(struct thread_arg));
				if(arg == NULL) {cleanup_and_exit("malloc");}
				parg->clientsock = clientsock;
				
				ret = pthread_create(&tid, &attr, thread_handler, parg);
				if(ret != 0) {
					cleanup_and_exit("pthread_create");
				}
				
			}
		}
	}
	accept
	select
	
	return 0;
}