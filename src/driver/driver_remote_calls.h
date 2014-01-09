#ifndef DRIVER_REMOTE_CALLS_H
#define DRIVER_REMOTE_CALLS_H

#include "../common/rpc_data.h"
#include <glib.h>

void recv_info_from_simulator(int sockfd);
void harvester_move_to_drv_call(const int sockfd, const Object_Coord_On_Board const *mvtd);
void harvester_move_to_drv_recv(gboolean *move_done, gboolean *have_minerals, int sockfd);

void get_harvester_coordinates_drv_call(const int sockfd);
void get_harvester_coordinates_drv_recv(int *x_coord, int *y_coord, int sockfd);

#endif//DRIVER_REMOTE_CALLS_H
