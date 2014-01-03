#include <stdio.h>
#include <libconfig.h>

#include "simulator_config.h"
 
gboolean read_simulator_config(Simulator_Params * const sim_params) {
	config_t cfg;               /*Returns all parameters in this structure */
	config_setting_t *setting;
	long int harvesters_number_param;
	long int width_of_board_param;
	long int height_of_board_param;
	long int port_number_param;
	
	char *config_file_name = "config/simulator_config.txt";
 
    /*Initialization */
    config_init(&cfg);
 
    /* Read the file. If there is an error, report it and exit. */
    if (!config_read_file(&cfg, config_file_name)) {
        printf("\n Config file <simulator_config.txt> error:%d - %s",
			config_error_line(&cfg), config_error_text(&cfg));
        
        config_destroy(&cfg);
        return FALSE;
    }
    
    gboolean ret_params = TRUE;
    
    if (!config_lookup_int(&cfg, "port_number", &port_number_param))
		ret_params = FALSE;
 
    if (!config_lookup_int(&cfg, "harvester_number", &harvesters_number_param))
		ret_params = FALSE;
	
	if (!config_lookup_int(&cfg, "width_of_board", &width_of_board_param))
		ret_params = FALSE;
	
	if (!config_lookup_int(&cfg, "height_of_board", &height_of_board_param))
		ret_params = FALSE;
	
    config_destroy(&cfg);
    
    if (ret_params) {
		sim_params->port_number = (int)port_number_param;
		sim_params->harvesters_number = (int)harvesters_number_param;
		sim_params->width_of_board = (int)width_of_board_param;
		sim_params->height_of_board = (int)height_of_board_param;
		
		if (sim_params->harvesters_number > sim_params->height_of_board)
			sim_params->harvesters_number = sim_params->height_of_board;
	}
    
    return ret_params;
}
