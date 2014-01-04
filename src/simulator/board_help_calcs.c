#include "simulator_data.h"
#include "board_help_calcs.h"

Board_Coord_Param *get_field_of_board(int x_coord, int y_coord) {
	Board_Coord_Param *field = NULL;
	if (x_coord >= 0 && x_coord < sim_params.width_of_board && y_coord >= 0 && y_coord < sim_params.height_of_board)
		field = board_coord_sys + sim_params.width_of_board * y_coord + x_coord;
	return field;
}

void get_coord_of_field(int * const x_coord, int * const y_coord, int index) {
	if (index < sim_params.width_of_board * sim_params.height_of_board) {
		*x_coord = index % sim_params.width_of_board;
		*y_coord = index / sim_params.width_of_board;
	} else {
		*x_coord = -1;
		*y_coord = -1;
	}
}
