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
#include <sys/epoll.h>
#include <syscall.h>
#include <iostream>

#define PORT 		"18200"
#define BACKLOG 	1024
#define MAX_EVENTS 	1024

struct thread_arg {
	int clientsock;
	pthread_t tid;
	unsigned int nid;
};


using namespace std;

int sockfd = -1, efd = -1;
struct addrinfo hints, *addr = NULL;

/*clean all resouces and exit*/
void cleanup_and_exit(const char *errmsg) {
	
	/*print error message*/
	perror(errmsg);
	/*clean up resources*/
	if(sockfd >=0 )
		close(sockfd);
	if(efd >=0 )
		close(efd);
	if(addr != NULL) {
		freeaddrinfo(addr);
		addr = NULL;
	}
	/*exit */
	exit(EXIT_FAILURE);
}
/*The handler of client request
This func will read client's socket to get a number, 
and then decomposite into prime factors which will be sent
back to client*/
void *thread_handler(void *arg) {
	struct thread_arg *p = (struct thread_arg *)arg;
	int clientfd, ret, len;
	uint32_t num;
	uint32_t result[32];
	char str[32];
	unsigned int i,f;
	
	clientfd = p->clientsock;
	sprintf(str, "Thread(%u): ", p->nid);
	free(p);
	p = NULL;


	ret = recv(clientfd, &num, sizeof(uint32_t), 0);
	if(ret != sizeof(uint32_t))
	{
		perror("recv");
		return NULL;
	}
	/*convert to host order*/
	num = ntohl(num);
	cout << str << "Received number " << num 
		<< " from some client." <<endl;
	
	cout << str << "Computing prime factors." << endl;
	cout << str << "Prime factors of " << num << " are:" << endl;	
	i = 0;
	
	/*prime factor decomposition*/
	while(num > 1 && num % 2 == 0) {
		cout << str << "2" << endl;
		result[i++] = htonl(2);
		num = num/2;
	}
	f = 3;
	while(num > 1) {
		if(num % f ==0) {
			cout << str << f << endl;
			result[i++] = htonl(f);
			num = num/f;
		}
		else {
			f = f + 2;
		}
	}
	/*no prime factor found, return 0 or 1 to client*/	
	if(i == 0) {
		if(num == 0) {
			result[i++] = htonl(0);
		} else {
			result[i++] = htonl(1);
		}
	}

	/*send result to client*/
	cout << str << "Sent prime factors back to that client." << endl;
	len = sizeof(uint32_t)*i;
	ret = send(clientfd, &result[0], len, 0);
	if(ret != len) {
		perror("send");
	}
	
	/*close*/
	close(clientfd);
	
	return NULL;
}

int main() {
	struct addrinfo hints, *addr;
	struct epoll_event ev, events[MAX_EVENTS];
	int ret, nfds, clientsock, n;
	pthread_attr_t attr;
	struct thread_arg *parg;
	struct sockaddr client;
	socklen_t addrlen;
	char str[64];
	unsigned int nid;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
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
	if(bind(sockfd, addr->ai_addr, addr->ai_addrlen) == -1) {
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
	if(pthread_attr_init(&attr) == -1) {
		cleanup_and_exit("pthread_attr_init");
	}
	/*set pthread to detached state, resources will be freed when returned*/
	if(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
		cleanup_and_exit("pthead_attr_setdetachstate");
	}	
	for(;;) {
		cout << "Server is running and ready to " 
			"receive connections on port "PORT"..."
			<< endl;

		/*use epoll to monitor all sockets*/
		nfds = epoll_wait(efd, events, MAX_EVENTS, -1);
		if(nfds == -1) {
			cleanup_and_exit("epoll_wait");
		}
		for(n = 0; n < nfds; n++) {
			if(events[n].data.fd == sockfd) {
				/*accept new client*/
				addrlen = sizeof(client);
				
				clientsock = accept(sockfd, &client, &addrlen);
				if(clientsock == -1) {
					cleanup_and_exit("accept");;
				}
				inet_ntop(client.sa_family, 
				    &(((sockaddr_in *)(&client))->sin_addr),
				    str, sizeof(str));
				cout << "Accept client from " << str << endl;
				ev.events = EPOLLIN;
				ev.data.fd = clientsock;
				/*add client socket to epoll monitoring list*/
				if (epoll_ctl(efd, EPOLL_CTL_ADD, clientsock, 
					&ev) == -1) 
				{
					cleanup_and_exit("epoll_ctl: conn_sock");
				}
			}
			else {
				/*client data is ready to read, 
					create new thread to handle it*/

				parg = (struct thread_arg *)
					malloc(sizeof(struct thread_arg));
				if(parg == NULL) {cleanup_and_exit("malloc");}
				parg->clientsock = clientsock;
				parg->nid = nid++;	
				ret = pthread_create(&parg->tid, &attr,
					 thread_handler, parg);
				if(ret != 0) {
					cleanup_and_exit("pthread_create");
				}
				cout << "Client data is ready, "
					"create a thread to handle request."
					 << endl; 

				/*remove socket from monitoring list*/
				epoll_ctl(efd, EPOLL_CTL_DEL, clientsock, &ev);
				
			}
		}
	}
	
	return 0;
}

