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

#define SRV_PORT	18200
#define SRV_NAME	"127.0.0.1"
#define MAX_NUM		(1<<32)
#define BUFLEN		128

using namespace std;

int main() {
	unsigned int num = 0;
	int i, sockfd;
	struct sockaddr_in saddr;
	struct addrinfo hints, *res = NULL;
	unit32_t buf[BUFLEN];
	uint32_t *pnum;

	
	cout << "Please provide an integer > 1:" << endl; 
	cin >> num;
	
	/*check the number if it is too big*/
	if(num > MAX_NUM) {
		cout << num << "is too big" << endl;
		return 0;
	}
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; /*only try ipv4 address*/
	hints.ai_socktype = SOCK_STREAM;
	/*fill address struct*/
	if((ret = getaddrinfo(SRV_NAME, SRV_PORT, &hints, &res)) != 0) {
		cerr << "getaddrinfo:" << gai_strerror(ret) << endl;
		exit(EXIT_FAILURE);
	}
	/*create a TCP socket fd*/
	if((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol) < 0) {
		cerr << "create socket fail" << endl;
		freeaddrinfo(res);
		exit(EXIT_FAILURE);
	}
	/*connect to server*/
	if((ret = connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
		cerr << "Cannot connect to server "SRV_NAME << ret << end;
		close(sockfd);
		freeaddrinfo(res);
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(res);
	
	/*convert num to network order and fill in buf*/
	buf[0] = htonl((unit32_t)num);
	/*send num to server*/
	ret = send(sockfd, &buf[0], sizeof(uint32_t), 0);
	if(ret < sizeof(unit32_t)) {
		cerr << "Send fail " << ret << endl;
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	cout << "Sent " << num << " to server on port "SRV_PORT << endl;
	/*receive message returned*/
	ret = recv(sockfd, &buf[0], sizeof(buf), 0);
	if(ret < sizeof(uint32_t)) {
		cerr << "recv fail " << ret << endl;
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	cout << "Received the following prime factors from server:" << endl;
	/*get factors from buf, each factor take 32bits length, no separator*/
	for(i = 0; i < ret/sizeof(unit32_t); i++) {
		/*convert to host order and print factors, */
		cout << ntohl(buf[i]) << endl;
	}
	
	cout << "Done!" << endl;
	close(sockfd);
	return 0;
}
