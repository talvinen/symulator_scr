#include "simulator_data.h"
#include "board_help_calcs.h"
#include "../common/rpc_all.h"
#include "simulator_remote_calls.h"

static void harvester_move_to_sim_call(const tpl_bin * const tb, Harvester_Id harvester_id);
static void get_harvester_coordinates_sim_call(const tpl_bin * const __attribute__((unused)) tb,
												Harvester_Id harvester_id);


static void harvester_move_to_sim_reply(gboolean ret, int sockfd);
static void get_harvester_coordinates_sim_reply(int x_coord, int y_coord, int sockfd);

void (*callback_function_sim[]) (const tpl_bin * const tb, Harvester_Id harvester_id) = {
	harvester_move_to_sim_call,
	get_harvester_coordinates_sim_call
};

void harvester_move_to_sim_call(const tpl_bin * const tb, Harvester_Id harvester_id) {
	gboolean ret = TRUE;
	
	if (board_coord_sys == NULL || tb->sz <= 0 || tb->addr == NULL)
		ret = FALSE;
	
	if (ret) {
		Board_Coord_Param *field;
		Object_Coord_On_Board *harvester_coord;
		
		harvester_coord = (Object_Coord_On_Board *)tb->addr;
		
		g_static_rec_mutex_lock(&board_coord_sys_mutex);
		
		field = get_field_of_board(harvester_coord->x_coord, harvester_coord->y_coord);
		
		if (field && *field == EMPTY_FIELD) {
			*field = HARVESTER_ON_FIELD;
		} else
			ret = FALSE;
		
		int *x_coord;
		int *y_coord;
		if (ret) {
			x_coord = &(harvesters_param[harvester_id].harvester_coord.x_coord);
			y_coord = &(harvesters_param[harvester_id].harvester_coord.y_coord);
			
			field = get_field_of_board(*x_coord, *y_coord);
			*field = EMPTY_FIELD;
			
			*x_coord = harvester_coord->x_coord;
			*y_coord = harvester_coord->y_coord;
		}
	
		g_static_rec_mutex_unlock(&board_coord_sys_mutex);
	}
	
	int socket = harvesters_param[harvester_id].harvester_socket;
	harvester_move_to_sim_reply(ret, socket);
}

void harvester_move_to_sim_reply(gboolean ret, int sockfd) {
	int32_t reply;
	tpl_node *tn;
	
	reply = (int32_t)ret;
	
	tn = tpl_map("i", &reply);
	tpl_pack(tn, 0);
	
	send_to(tn, sockfd);
	
	tpl_free(tn);
}

void get_harvester_coordinates_sim_call(const tpl_bin * const __attribute__((unused)) tb, Harvester_Id harvester_id) {
	int x_coord;
	int y_coord;
	
	x_coord = harvesters_param[harvester_id].harvester_coord.x_coord;
	y_coord = harvesters_param[harvester_id].harvester_coord.y_coord;
	
	int socket = harvesters_param[harvester_id].harvester_socket;
	get_harvester_coordinates_sim_reply(x_coord, y_coord, socket);
}

void get_harvester_coordinates_sim_reply(int x_coord, int y_coord, int sockfd) {
	tpl_node *tn;
	
	tn = tpl_map("ii", (int32_t *)&x_coord, (int32_t *)&y_coord);
	tpl_pack(tn, 0);
	
	send_to(tn, sockfd);
	
	tpl_free(tn);
}

