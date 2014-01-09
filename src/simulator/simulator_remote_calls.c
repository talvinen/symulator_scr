#include "simulator_data.h"
#include "board_help_calcs.h"
#include "../common/rpc_all.h"
#include "simulator_remote_calls.h"

static void harvester_move_to_sim_call(const tpl_bin * const tb, Harvester_Id harvester_id);
static void get_harvester_coordinates_sim_call(const tpl_bin * const __attribute__((unused)) tb,
												Harvester_Id harvester_id);


static void harvester_move_to_sim_reply(gboolean move_done, gboolean have_minerals, int sockfd);
static void get_harvester_coordinates_sim_reply(int x_coord, int y_coord, int sockfd);

void (*callback_function_sim[]) (const tpl_bin * const tb, Harvester_Id harvester_id) = {
	harvester_move_to_sim_call,
	get_harvester_coordinates_sim_call
};

void harvester_move_to_sim_call(const tpl_bin * const tb, Harvester_Id harvester_id) {
	gboolean move_done = TRUE;
	gboolean have_minerals = FALSE;
	
	if (board_coord_sys == NULL || tb->sz <= 0 || tb->addr == NULL)
		move_done = FALSE;
	
	if (move_done) {
		Board_Coord_Param *field;
		Object_Coord_On_Board *harvester_coord;
		
		harvester_coord = (Object_Coord_On_Board *)tb->addr;
		
		g_static_rec_mutex_lock(&board_coord_sys_mutex);
		
		field = get_field_of_board(harvester_coord->x_coord, harvester_coord->y_coord);
		
		if (field)
			if (*field == EMPTY_FIELD)
				*field = HARVESTER_ON_FIELD;
			else if (*field == MINERAL_ON_FIELD) {
				*field = HARVESTER_ON_FIELD;
				if (!harvesters_param[harvester_id].have_minerals) {
					have_minerals = TRUE;
					char key[10];
					(void)sprintf(key, "%d%d", harvester_coord->x_coord, harvester_coord->y_coord);
					(void)g_hash_table_remove(minerals_param, key);
					//harvesters_param[harvester_id].have_minerals = TRUE;
				}
			} else
				move_done = FALSE;
		else
			move_done = FALSE;
		
		int *x_coord;
		int *y_coord;
		if (move_done) {
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
	harvester_move_to_sim_reply(move_done, have_minerals, socket);
}

void harvester_move_to_sim_reply(gboolean move_done, gboolean have_minerals, int sockfd) {
	int32_t reply1, reply2;
	tpl_node *tn;
	
	reply1 = (int32_t)move_done;
	reply2 = (int32_t)have_minerals;
	
	tn = tpl_map("ii", &reply1, &reply2);
	tpl_pack(tn, 0);
	
	
	tpl_dump(tn, TPL_FD, sockfd);
	
	tpl_free(tn);
}

void send_simulator_drop_zones_info(int sockfd) {
	tpl_node *tn;
	tpl_bin tb;
	
	tb.addr = (void *)drop_zones_param;
	tb.sz = sizeof(Object_Coord_On_Board) * SIM_REFINERYS_NUMBER;
	
	tn = tpl_map("B", &tb);
	tpl_pack(tn, 0);
	
	tpl_dump(tn, TPL_FD, sockfd);
	
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
	
	tpl_dump(tn, TPL_FD, sockfd);
	
	tpl_free(tn);
}

