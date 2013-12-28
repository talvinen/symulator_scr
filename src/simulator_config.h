#ifndef SIMULATOR_CONFIG_H
#define SIMULATOR_CONFIG_H

#include <glib.h>

typedef struct {
	int portNumber;
	int harvesterNumber;
	int widthOfBoard;//number of fields
	int heightOfBoard;//number of fields
} Simulator_Params;

gboolean read_simulator_config(Simulator_Params * const sim_params);

#endif//SIMULATOR_CONFIG_H
