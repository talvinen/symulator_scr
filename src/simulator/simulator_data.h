#ifndef SIMULATOR_DATA_H
#define SIMULATOR_DATA_H

#include <librsvg/rsvg.h>
#include <gtk/gtk.h>
#include "../common/simulator_config.h"
#include "../common/rpc_data.h"

#define SIM_IMGS_NUMBER 5
#define SIM_REFINERYS_NUMBER 3

typedef enum {
	EMPTY_FIELD = 0,
	HARVESTER_ON_FIELD,
	REFINERY_ON_FIELD,
	MINERAL_ON_FIELD,
	DROP_ZONE_ON_FIELD
} Board_Coord_Param;

typedef int Harvester_Id;

typedef struct {
	Object_Coord_On_Board harvester_coord;
	Harvester_Id harvester_id;
	int harvester_socket;
	gboolean have_minerals;
} Harvester_Param;

typedef struct {
	Object_Coord_On_Board refinery_coord;
	int width;
	int height;
} Refinery_Param;

typedef struct {
	RsvgHandle *image;
	int width;
	int height;
} Simulator_Imgs;


extern Simulator_Params sim_params;
extern Board_Coord_Param *board_coord_sys;
extern Harvester_Param *harvesters_param;
extern GHashTable *minerals_param;
extern Refinery_Param refinerys_param[SIM_REFINERYS_NUMBER];
extern Object_Coord_On_Board drop_zones_param[SIM_REFINERYS_NUMBER];
extern Simulator_Imgs simulator_imgs[SIM_IMGS_NUMBER];
extern GStaticRecMutex board_coord_sys_mutex;

#endif//SIMULATOR_DATA_H
