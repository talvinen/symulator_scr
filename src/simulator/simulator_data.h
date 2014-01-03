#ifndef SIMULATOR_DATA_H
#define SIMULATOR_DATA_H

#include <cairo.h>
#include <gtk/gtk.h>
#include "simulator_config.h"
#include "../common/rpc_data.h"

#define SIM_IMGS_NUMBER 4
#define SIM_REFINERYS_NUMBER 3

enum {
	EMPTY_FIELD = 0,
	OBJECT_ON_FIELD,
};

typedef int Harvester_Id;

typedef struct {
	Object_Coord_On_Board harvester_coord;
	Harvester_Id harvester_id;
	int harvester_socket;
} Harvester_Param;

typedef struct {
	Object_Coord_On_Board refinery_coord;
	int width;
	int height;
} Refinery_Param;

typedef struct {
	cairo_surface_t *image;
	double width;
	double height;
} Simulator_Imgs;

extern Simulator_Params sim_params;
extern int *board_coord_sys;
extern Harvester_Param *harvesters_param;
extern Refinery_Param refinerys_param[SIM_REFINERYS_NUMBER];
extern Simulator_Imgs simulator_imgs[SIM_IMGS_NUMBER];
extern GStaticRecMutex board_coord_sys_mutex;

#endif//SIMULATOR_DATA_H
