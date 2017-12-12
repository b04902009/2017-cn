#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#define N 100
#define KB 1024
#define DATA 0
#define ACK 1
#define FIN 2
#define FINACK 3
typedef struct Packet{
	int type, seq_num, data_len;
	char data[1025];
} Packet;
int max(int a, int b){
	return a > b? a : b;
}
int create_UDP_socket(int port){
	int socket_fd;
	struct sockaddr_in server_addr;
	if((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) <0) {
		perror("socket failed\n");
		exit(EXIT_FAILURE);
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);
	if(bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <0) {
		perror("bind failed\n");
		exit(1);
	}
	return socket_fd;
}
void set_addr(struct sockaddr_in* addr, char* ip, int port){
	memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(ip);
    addr->sin_port = htons(port);
}
int recv_packet(int fd, Packet* p, struct sockaddr_in* addr){
	int nbytes, length = sizeof(*addr);
	if((nbytes = recvfrom(fd, p, sizeof(*p), 0, (struct sockaddr*)addr, (socklen_t*)&length)) < 0) {
		perror("packet receiving failed\n");
	}
	return nbytes;
}
int send_packet(int fd, Packet* p, struct sockaddr_in* addr){
	int nbytes, length = sizeof(*addr);
	if((nbytes = sendto(fd, p, sizeof(*p), 0, (struct sockaddr*)addr, *(socklen_t*)&length)) < 0){
		perror("packet sending failed\n");
	}
	return nbytes;
}