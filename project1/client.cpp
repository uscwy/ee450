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

#define SRV_PORT	"18200"
#define SRV_NAME	"127.0.0.1"
#define MAX_NUM		(1<<31)
#define BUFLEN		128

int sockfd;
struct addrinfo *res = NULL;

using namespace std;

void cleanup_exit(const char *errmsg) {

	if(sockfd >= 0) {
		close(sockfd);
	}

	if(res != NULL) {
		freeaddrinfo(res);
	}

	if(errmsg != NULL) {
		perror(errmsg);
		exit(EXIT_FAILURE);
	}
	return;
}

int main(int argc, char **argv) {
	unsigned int num = 0, i;
	int sockfd, ret;
	struct addrinfo hints;
	uint32_t buf[BUFLEN];

	/*get number from shell parameter*/
	if(argc > 1) {
		num = atoi(argv[1]);
	}
	else
	{
		cout << "Please provide an integer > 1:" << endl; 
		cin >> num;
	}
	/*check the number if it is too big*/
	if(num == 0 || num > (unsigned)MAX_NUM) {
		cout << "Invalid number " << num  << endl;
		return 0;
	}
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; /*only try ipv4 address*/
	hints.ai_socktype = SOCK_STREAM;
	/*fill address struct*/
	if((ret = getaddrinfo(SRV_NAME, SRV_PORT, &hints, &res)) != 0) {
		cerr << "getaddrinfo:" << gai_strerror(ret) << endl;
		cleanup_exit("getaddrinfo");
	}
	/*create a TCP socket fd*/
	if((sockfd = socket(res->ai_family, res->ai_socktype, 
			res->ai_protocol)) < 0) {
		cleanup_exit("create socket fail");
	}

	/*connect to server*/
	if((ret = connect(sockfd, res->ai_addr, res->ai_addrlen)) == -1) {
		cerr << "Cannot connect to server "SRV_NAME << endl;
		cleanup_exit("connect");
	}
	freeaddrinfo(res);
	res = NULL;
	/*convert num to network order and fill in buf*/
	buf[0] = htonl((uint32_t)num);
	/*send num to server*/
	ret = send(sockfd, &buf[0], sizeof(uint32_t), 0);
	if(ret < (signed)sizeof(uint32_t)) {
		cleanup_exit("send");
	}

	cout << "Sent " << num << " to server on port "SRV_PORT << endl;
	/*receive message returned*/
	ret = recv(sockfd, &buf[0], sizeof(buf), 0);
	if(ret < (signed)sizeof(uint32_t)) {
		cleanup_exit("recv");
	}
	cout << "Received the following prime factors from server:" << endl;
	/*get factors from buf, each factor take 32bits length, no separator*/
	for(i = 0; i < ret/sizeof(uint32_t); i++) {
		/*convert to host order and print factors, */
		cout << ntohl(buf[i]) << endl;
	}
	
	cout << "Done!" << endl;

	cleanup_exit(NULL);

	return 0;
}

