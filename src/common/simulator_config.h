#ifndef SIMULATOR_CONFIG_H
#define SIMULATOR_CONFIG_H

#include <glib.h>

typedef struct {
	int port_number;
	int number_of_harvesters;
	int width_of_board;//number of fields
	int height_of_board;//number of fields
	int number_of_minerals;//percent of the board occupied by minerals
} Simulator_Params;

gboolean read_simulator_config(Simulator_Params * const sim_params);

#endif//SIMULATOR_CONFIG_H
