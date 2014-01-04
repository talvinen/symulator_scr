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
#include "../common/rpc_all.h"
#include "../common/rpc_data.h"

#define NUMBER_N 500
#define THREADS_N 5
 

int connect_to_simulator(const char * const host, int port);
void *driver_thread_work(void *ptr);
gboolean respond_from_simulator(int sockfd);

//******************GLOBAL VARIABLES********************
Simulator_Params sim_params = {0};
//******************************************************

static const char * const serv_adrr = "localhost";

int main(int argc, char *argv[]) {	
		int sock_fd = 0;
		
		if (!read_simulator_config(&sim_params))
			return 1;
		
		pthread_t driver_threads[sim_params.number_of_harvesters];
		
		Object_Coord_On_Board mvtd;
		
		mvtd.x_coord = 0;
		mvtd.y_coord = 0;
		
		int driver_thread = 0;
		while (driver_thread < sim_params.number_of_harvesters)
			(void)pthread_create(&driver_threads[driver_thread++], NULL, &driver_thread_work, NULL);
		
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
	int sock_fd = connect_to_simulator(serv_adrr, sim_params.port_number);
	
	if (sock_fd < 0) {
		fprintf(stderr, "ERROR unable to connect to server %s on port %d\n", serv_adrr, 8787);
	} else {
		fprintf(stderr, "success connect to server %s on port %d\n", serv_adrr, 8787);
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

void sendToCli(const int sockfd, const Object_Coord_On_Board const *mvtd) {
	tpl_node *tn;
	tpl_bin tb;
	int function_name;
	
	tb.addr = (void *)mvtd;
	tb.sz = sizeof(Object_Coord_On_Board);
	
	function_name = HARVESTER_MOVE_TO_SIM_CALL;
	
	tn = tpl_map("iB", &function_name, &tb);
	tpl_pack(tn,0);
	
	tpl_dump(tn, TPL_FD, sockfd);
	tpl_free(tn);
}

gboolean respond_from_simulator(int sockfd) {
	tpl_node *tn = NULL;
	tpl_bin tb;
	void *img = NULL;
	int rc = 0;
	size_t sz = 0;
	
	int resp;
	gboolean ret;
	
	rc = tpl_gather(TPL_GATHER_BLOCKING, sockfd, &img, &sz);
	
	if (rc > 0) {
		tn = tpl_map("i", &resp);
		tpl_load(tn, TPL_MEM, img, sz);
		tpl_unpack(tn, 0);
		tpl_free(tn);
	}
	ret = (gboolean)resp;
	return ret;
}
