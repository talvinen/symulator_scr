#include "driver_data.h"
#include "driver_remote_calls.h"
#include "../common/rpc_all.h"
#include <tpl.h>

void recv_info_from_simulator(int sockfd) {
	tpl_node *tn;
	tpl_bin tb;
	void *img;
	int rc;
	size_t sz;
	
	rc = tpl_gather(TPL_GATHER_BLOCKING, sockfd, &img, &sz);
	
	if (rc > 0) {
		tn = tpl_map("B", &tb);
		tpl_load(tn, TPL_MEM, img, sz);
		tpl_unpack(tn, 0);
		tpl_free(tn);
	}
	drop_zones_param = (Object_Coord_On_Board *)tb.addr;
	drop_zones_param_size = tb.sz / sizeof(Object_Coord_On_Board);
}

void harvester_move_to_drv_call(const int sockfd, const Object_Coord_On_Board const *mvtd) {
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

void harvester_move_to_drv_recv(gboolean *move_done, gboolean *have_minerals,
									gboolean *minerals_collected, int sockfd) {
	tpl_node *tn = NULL;
	void *img = NULL;
	int rc = 0;
	size_t sz = 0;
	
	char recv1, recv2, recv3;
	
	rc = tpl_gather(TPL_GATHER_BLOCKING, sockfd, &img, &sz);
	
	if (rc > 0) {
		tn = tpl_map("ccc", &recv1, &recv2, &recv3);
		tpl_load(tn, TPL_MEM, img, sz);
		tpl_unpack(tn, 0);
		tpl_free(tn);
	}
	*move_done = (gboolean)recv1;
	*have_minerals = (gboolean)recv2;
	*minerals_collected = (gboolean)recv3;
}

void get_harvester_coordinates_drv_call(const int sockfd) {
	tpl_node *tn;
	tpl_bin tb;
	int function_name;
	
	tb.addr = NULL;
	tb.sz = 0;
	
	function_name = GET_HARVESTER_COORDINATES_SIM_CALL;
	
	tn = tpl_map("iB", &function_name, &tb);
	tpl_pack(tn,0);
	
	tpl_dump(tn, TPL_FD, sockfd);
	tpl_free(tn);
}

void get_harvester_coordinates_drv_recv(int *x_coord, int *y_coord, int sockfd) {
	tpl_node *tn = NULL;
	void *img = NULL;
	int rc = 0;
	size_t sz = 0;
	
	rc = tpl_gather(TPL_GATHER_BLOCKING, sockfd, &img, &sz);
	
	if (rc > 0) {
		tn = tpl_map("ii", x_coord, y_coord);
		tpl_load(tn, TPL_MEM, img, sz);
		tpl_unpack(tn, 0);
		tpl_free(tn);
	}
}
