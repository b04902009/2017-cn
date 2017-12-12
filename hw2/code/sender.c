#include "udp_socket.h"
#define SENDER_PORT 5000
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
	FILE *fp = fopen(file_path, "r");
	if(fp == NULL){
		perror("file open failed\n");
		exit(1);
	}
	
	// Create UDP socket
	int socket_fd = create_UDP_socket(SENDER_PORT);
	struct sockaddr_in agent;
	set_addr(&agent, agent_ip, agent_port);

	// Select: timeout in 1 seconds
	fd_set input_set;
	struct timeval timeout;
	timeout.tv_sec = 1;				// wait 1 seconds
    timeout.tv_usec = 0;			// 		0 milliseconds
    int ready_for_reading = 0;

    // Initialization
    int threshold = 16, winsize = 1, acked_seq_num = 0, result, end = 0;
	Packet p;
	int send_max_seq = 0, last_seq;

	while(1){
		// Send packets
		int window_lower_bound = acked_seq_num + 1;
		int window_upper_bound = acked_seq_num + winsize;
		for(int i = window_lower_bound; i <= window_upper_bound && !end; i++){
			fseek(fp, (i-1)*KB, SEEK_SET);
			// fseek(fp, KB, SEEK_CUR);
			if((result = fread(p.data, 1, KB, fp)) < 0){
				perror("file read error\n");
				exit(1);
			}
			if(result == 0){		// End of reading file
				last_seq = i-1;
				break;
			}
			p.type = DATA;
			p.data_len = result;
			p.seq_num = i;
			if(send_packet(socket_fd, &p, &agent)){
				if(i > send_max_seq)
					printf("send\tdata\t#%d,\twinSize = %d\n", p.seq_num, winsize);
				else
					printf("resnd\tdata\t#%d,\twinSize = %d\n", p.seq_num, winsize);
			}
		}
		
		// Change window size and maximum sequence number sent
		send_max_seq = max(window_upper_bound, send_max_seq);
		int old_winsize = winsize;
		if(winsize < threshold)
			winsize *= 2;
		else
			winsize++;


		// Receive packets
		int recv_num = 0;
		while(acked_seq_num != window_upper_bound){
		// for(int i = 0; i < old_winsize; i++){
			// When ACKed all packets, send FIN and receive FINACK 
			if(acked_seq_num == last_seq && last_seq){
				end = 1;
				p.type = FIN;
				if(send_packet(socket_fd, &p, &agent))
					printf("send\tfin\n");
				if(recv_packet(socket_fd, &p, &agent))
					printf("recv\tfinack\n");
				break;
			}

			// Initialize fd set
			FD_ZERO(&input_set);			// Empty the FD Set
			FD_SET(socket_fd, &input_set);	// Listen to the input descriptor

			// Receiving ACK & Timeout
			ready_for_reading = select(socket_fd+1, &input_set, NULL, NULL, &timeout);
			if(ready_for_reading == -1) {
				perror("socket reading failed\n");
				exit(1);
			} 
			else if(!ready_for_reading){	// Timeout
				threshold = max(old_winsize/2, 1);
				winsize = 1;
				printf("time\tout\t\tthreshold = %d\n", threshold);
				break;
			}
			else{
				if(recv_packet(socket_fd, &p, &agent)){
					if(p.type == ACK){
						printf("recv\tack\t#%d\n", p.seq_num);
						if(p.seq_num == acked_seq_num + 1)
							acked_seq_num++;
					}
				}
			}
		}
		if(end)
			break;
	}

	fclose(fp);
	return 0;
}