#ifndef RPC_ALL_H
#define RPC_ALL_H

#include <tpl.h>

typedef enum {
	HARVESTER_MOVE_TO_SIM_CALL = 0,
	GET_HARVESTER_COORDINATES_SIM_CALL,
	END_SIM_CALL
} Simulator_Name_Calls;

void send_to(tpl_node *tn, int sockfd);

#endif//RPC_ALL_H
