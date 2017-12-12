#include "udp_socket.h"
#define AGENT_PORT 5001
int main(int argc, char *argv[]){ 
					// sender_ip, sender_port, receiver_ip, receiver_port, loss_rate
	// Initialize
	if(argc != 6){
		perror("wrong arguments\n");
		exit(1);
	}
	char sender_ip[N], receiver_ip[N];
	strcpy(sender_ip, argv[1]);
	int sender_port = atoi(argv[2]);
	strcpy(receiver_ip, argv[3]);
	int receiver_port = atoi(argv[4]);
	float required_loss_rate = atof(argv[5]);
	
	// Create UDP socket
	int socket_fd = create_UDP_socket(AGENT_PORT);
	struct sockaddr_in sender, receiver, client;
	set_addr(&sender, sender_ip, sender_port);
	set_addr(&receiver, receiver_ip, receiver_port);

	// Initialization
	int loss_num = 0, recv_num = 0;
	Packet p;
	int first = 1;

	srand(time(NULL));
	while(1){
		if(recv_packet(socket_fd, &p, &client)){
			if(p.type == DATA){
				printf("get\tdata\t#%d\n", p.seq_num);
				if((float)rand()/RAND_MAX <= required_loss_rate){
				// if(p.seq_num == 4 && first--){		// For testing
					loss_num++;
					float loss_rate = (float)loss_num / (recv_num + loss_num);
					printf("drop\tdata\t#%d\tloss rate = %.4f\n", p.seq_num, loss_rate);
				}
				else{
					recv_num++;
					float loss_rate = (float)loss_num / (recv_num + loss_num);
					if(send_packet(socket_fd, &p, &receiver))
						printf("fwd\tdata\t#%d\tloss rate = %.4f\n", p.seq_num, loss_rate);
				}
			}
			else if(p.type == FIN){
				printf("get\tfin\n");
				if(send_packet(socket_fd, &p, &receiver))
					printf("fwd\tfin\n");
			}
			if(p.type == ACK){
				printf("get\tack\t#%d\n", p.seq_num);
				if(send_packet(socket_fd, &p, &sender))
					printf("fwd\tack\t#%d\n", p.seq_num);
			}
			else if(p.type == FINACK){
				printf("get\tfinack\n");
				if(send_packet(socket_fd, &p, &sender))
					printf("fwd\tfinack\n");
				break;
			}
		}
	}
	return 0;
}