#include <stdio.h>
#include <libconfig.h>

#include "simulator_config.h"
 
gboolean read_simulator_config(Simulator_Params * const sim_params) {
	config_t cfg;               /*Returns all parameters in this structure */
	config_setting_t *setting;
	long int paramHarvesterNumber;
	long int paramWidthOfBoard;
	long int paramHeightOfBoard;
	long int paramPortNumber;
	
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
    
    if (!config_lookup_int(&cfg, "port_number", &paramPortNumber))
		ret_params = FALSE;
 
    if (!config_lookup_int(&cfg, "harvester_number", &paramHarvesterNumber))
		ret_params = FALSE;
	
	if (!config_lookup_int(&cfg, "width_of_board", &paramWidthOfBoard))
		ret_params = FALSE;
	
	if (!config_lookup_int(&cfg, "height_of_board", &paramHeightOfBoard))
		ret_params = FALSE;
	
    config_destroy(&cfg);
    
    if (ret_params) {
		sim_params->portNumber = (int)paramPortNumber;
		sim_params->harvesterNumber = (int)paramHarvesterNumber;
		sim_params->widthOfBoard = (int)paramWidthOfBoard;
		sim_params->heightOfBoard = (int)paramHeightOfBoard;
		
		if (sim_params->harvesterNumber > sim_params->heightOfBoard)
			sim_params->harvesterNumber = sim_params->heightOfBoard;
	}
    
    return ret_params;
}
