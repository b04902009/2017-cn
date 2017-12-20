#include "udp_socket.h"
#define RECEIVER_PORT 5002
#define BUFSIZE 32
// #define BUFSIZE 5 		// For testing
int main(int argc, char *argv[]){ // agent_ip, agent_port, file_path
	// Initialize
	if(argc != 4){
		perror("wrong arguments\n");
		exit(1);
	}
	char agent_ip[N], file_path[N];
	strcpy(agent_ip, argv[1]);
	int agent_port = atoi(argv[2]);
	strcpy(file_path, argv[3]);
	
	// Open file
	FILE *fp = fopen(file_path, "w");
	if(fp == NULL){
		perror("file open failed\n");
		exit(1);
	}

	// Create UDP socket
	int socket_fd = create_UDP_socket(RECEIVER_PORT);
	struct sockaddr_in agent, client;
	set_addr(&agent, agent_ip, agent_port);

	// Initialization
	Packet buf[BUFSIZE];
	int num = 0, acked_seq_num = 0;
	Packet p;

	while(1){
		if(recv_packet(socket_fd, &p, &agent)){
			if(num == BUFSIZE){		// Buffer overflow -> flush
				for(int i = 0; i < BUFSIZE; i++)
					fwrite(buf[i].data, 1, buf[i].data_len, fp);
				printf("drop\tdata\t#%d\n", p.seq_num);
				p.type = ACK;
				p.seq_num = acked_seq_num;
				if(send_packet(socket_fd, &p, &agent))
					printf("send\tack\t#%d\n", p.seq_num);
				printf("flush\n");
				num = 0;
			}
			else if(p.type == DATA){
				p.type = ACK;
				if(p.seq_num == acked_seq_num + 1){
					acked_seq_num++;
					printf("recv\tdata\t#%d\n", p.seq_num);
					for(int i = 0; i < p.data_len; i++)
						buf[num].data[i] = p.data[i];
					buf[num].data_len = p.data_len;
					num++;
				}
				else{
					printf("drop\tdata\t#%d\n", p.seq_num);
					p.seq_num = acked_seq_num;
				}
				if(send_packet(socket_fd, &p, &agent))
					printf("send\tack\t#%d\n", p.seq_num);
			}
			else if(p.type == FIN){
				p.type = FINACK;
				if(send_packet(socket_fd, &p, &agent)){
					printf("recv\tfin\n");
					printf("send\tfinack\n");
					break;
				}
			}
		}
	}
	// flush
	printf("flush\n");
	for(int i = 0; i < num; i++)
		fwrite(buf[i].data, 1, buf[i].data_len, fp);

	fclose(fp);
	return 0;
}