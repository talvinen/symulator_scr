#ifndef SIMULATOR_CONFIG_H
#define SIMULATOR_CONFIG_H

#include <glib.h>

typedef struct {
	int port_number;
	int harvesters_number;
	int width_of_board;//number of fields
	int height_of_board;//number of fields
} Simulator_Params;

gboolean read_simulator_config(Simulator_Params * const sim_params);

#endif//SIMULATOR_CONFIG_H
