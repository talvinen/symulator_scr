#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <tpl.h>
#include <glib.h>

#include "../common/simulator_config.h"
#include "../common/rpc_data.h"
#include "driver_data.h"
 

int connect_to_simulator(const char * const host, int port);
void *driver_thread_work(void *ptr);

//******************GLOBAL VARIABLES********************
Simulator_Params sim_params = {0};
Object_Coord_On_Board *drop_zones_param = NULL;
int drop_zones_param_size = 0;
//******************************************************

static const char * const serv_adrr = "localhost";

int main(int argc, char *argv[]) {	
		if (!read_simulator_config(&sim_params))
			return 1;
		
		pthread_t driver_threads[sim_params.number_of_harvesters];
		
		int sock_fd = connect_to_simulator(serv_adrr, sim_params.port_number);
		
		if (sock_fd < 0) {
			fprintf(stderr, "ERROR unable to connect to server %s on port %d\n", serv_adrr, 8787);
			exit(1);
		}
		
		recv_info_from_simulator(sock_fd);
		close(sock_fd);
		
		if (drop_zones_param == NULL)
			exit(1);
		
		int driver_thread = 0;
		while (driver_thread < sim_params.number_of_harvesters)
			if (pthread_create(&driver_threads[driver_thread++], NULL, &driver_thread_work, NULL) == 0)
				printf("Udane\n");
		
		driver_thread = 0;
		while (driver_thread < sim_params.number_of_harvesters)
			(void)pthread_join(driver_threads[driver_thread++], NULL);
		
		
		//sleep(10);
		//tpl_bin tb;
	//while(1) {
		//sleep(1);
		//sendToCli(sockfd, &mvtd);
		//if(!respond_from_simulator(sockfd))
			//break;
		//	printf("\nNO MOVE\n");
		//++(mvtd.x_coord);
		//++(mvtd.y_coord);
	//}
	//close(sockfd);
	//printf("dupa\n");
	return 0;
}

void *driver_thread_work(void *ptr) {
	const int sock_fd = connect_to_simulator(serv_adrr, sim_params.port_number);
	srand(time(NULL));
	
	if (sock_fd < 0) {
		fprintf(stderr, "ERROR unable to connect to server %s on port %d\n", serv_adrr, 8787);
	} else {
		fprintf(stderr, "success connect to server %s on port %d\n", serv_adrr, 8787);
		
		int x_coord, y_coord;
		int x_coord_current, y_coord_current;
		get_harvester_coordinates_drv_call(sock_fd);
		get_harvester_coordinates_drv_recv(&x_coord, &y_coord, sock_fd);
		
		printf("x_coord %d y_coord %d\n", x_coord, y_coord);
		
		x_coord_current = x_coord;
		y_coord_current = y_coord;
		Object_Coord_On_Board mvtd;
		while(TRUE) {
			int x_coord_add;
			int y_coord_add;
			
			mvtd.x_coord = x_coord_current;
			mvtd.y_coord = y_coord_current;
			
			int rand_move = rand() % 7;
			
			if (rand_move == 0) {
				x_coord_add = -1;
				y_coord_add = 1;
			} else if (rand_move == 1) {
				x_coord_add = 1;
				y_coord_add = 1;
			} else if (rand_move == 2) {
				x_coord_add = 1;
				y_coord_add = -1;
			} else if (rand_move == 3) {
				x_coord_add = 0;
				y_coord_add = 1;
			} else if (rand_move == 4) {
				x_coord_add = 1;
				y_coord_add = 0;
			} else if (rand_move == 5) {
				x_coord_add = 0;
				y_coord_add = -1;
			} else if (rand_move == 6) {
				x_coord_add = -1;
				y_coord_add = 0;
			}
			
			if (mvtd.x_coord == sim_params.width_of_board - 1)
				if (x_coord_add == 1)
					x_coord_add = 0;
			
			if (mvtd.x_coord == 0)
					if (x_coord_add == -1)
						x_coord_add = 0;
			
			
			if (mvtd.y_coord == sim_params.height_of_board - 1)
				if (y_coord_add == 1)
					y_coord_add = 0;
			
			if (mvtd.y_coord == 0)
					if (y_coord_add == -1)
						y_coord_add = 0;
			
			mvtd.x_coord += x_coord_add;
			mvtd.y_coord += y_coord_add;
			
			harvester_move_to_drv_call(sock_fd, &mvtd);
			gboolean move_done, have_minerals;
			harvester_move_to_drv_recv(&move_done, &have_minerals, sock_fd);
			
			if (move_done) {
				x_coord_current = mvtd.x_coord;
				y_coord_current = mvtd.y_coord;
			}
				
			sleep(1);
		}
	}
	//while (TRUE);
	return NULL;
}

int connect_to_simulator(const char * const host, int port) {
	int sockfd;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd < 0) {
		return -1;
	}
	
	server = gethostbyname(host);
	
	if (server == NULL) {
		return -1;
	}
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	
	serv_addr.sin_family = AF_INET;
	
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	
	serv_addr.sin_port = htons(port);
	
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		return -1;
	}
	
	return sockfd;
}
