#include <cairo.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>
#include <gtk/gtk.h>
//#include <math.h>

//#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
//#include <netdb.h>
#include <errno.h>

#include <signal.h>

#include <tpl.h>

#include "../common/simulator_config.h"

#include "simulator_scr.h"
#include "simulator_utils.h"
#include "simulator_img_defines.h"
#include "simulator_data.h"
#include "simulator_draw.h"
#include "simulator_remote_calls.h"
#include "board_help_calcs.h"

#include "../common/rpc_all.h"
#include "../common/rpc_data.h"

static void run_simulator(void);
gboolean stop_simulator(void);

void *run_server(void *ptr);
void *harvester_thread_work(void *harvester_param_vp);

void init_simulator_images(void);
void init_board_coord_sys(void);
void init_harvesters_param(void);
void init_board_environment(void);
void init_harvesters_on_board(void);
void init_refinerys_param(void);
void init_refinerys_on_board(void);
void init_drop_zones_param(void);
void init_drop_zones_on_board(void);
void init_minerals_param(void);
void init_minerals_on_board(void);

//******************GLOBAL VARIABLES********************
Simulator_Params sim_params = {0};
Board_Coord_Param *board_coord_sys = NULL;
Harvester_Param *harvesters_param = NULL;
GHashTable *minerals_param = NULL;

Refinery_Param refinerys_param[SIM_REFINERYS_NUMBER] = {0};
Object_Coord_On_Board drop_zones_param[SIM_REFINERYS_NUMBER] = {0};
Simulator_Imgs simulator_imgs[SIM_IMGS_NUMBER] = {0};

static const int sim_refinery_width = 3;
static const int sim_refinery_height = 3;

GMainLoop *main_loop;

GStaticRecMutex board_coord_sys_mutex = G_STATIC_REC_MUTEX_INIT;
//******************************************************

int main(int argc, char *argv[]) {		
	 if (g_thread_supported()) {
		 g_thread_init(NULL);
		 gdk_threads_init();
	 } else {
		 printf("g_thread NOT supported\n");
		 return 1;
	 }

	
	if (!read_simulator_config(&sim_params))
		return 1;
		
	if (sim_params.number_of_harvesters > sim_params.height_of_board - sim_refinery_height)
		sim_params.number_of_harvesters = sim_params.height_of_board - sim_refinery_height;
	if (sim_params.number_of_minerals > 30)
		sim_params.number_of_minerals = 30;
	printf("Parametry:\nport_number = %d\nnumber_of_harvesters = %d\n\
width_of_board = %d\nheight_of_board = %d\nnumber_of_minerals(percent of the board occupied by minerals) = %d\n",
		sim_params.port_number, sim_params.number_of_harvesters,
		sim_params.width_of_board, sim_params.height_of_board, sim_params.number_of_minerals);
	
	init_board_environment();
	
	(void)g_thread_create((GThreadFunc)run_server, NULL, FALSE, NULL);
	
	gtk_init(&argc, &argv);
	main_loop = g_main_loop_new(NULL, FALSE);
	
	init_simulator_images();
	
	run_simulator();
		
	return 0;
}

static void run_simulator(void) {
	GtkWidget *main_window;
	GtkWidget *draw_area;
	
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	
	draw_area = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(main_window), draw_area);
	
	g_signal_connect(G_OBJECT(draw_area), "expose-event",
		G_CALLBACK(on_draw_event), NULL);
	
	g_timeout_add(1000, do_drawing2, draw_area);
	g_signal_connect(G_OBJECT(main_window), "destroy",
		G_CALLBACK(stop_simulator), NULL);
	
	gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600); 
	gtk_window_set_title(GTK_WINDOW(main_window), "Symulator");
	
	gtk_widget_show_all(main_window);
	
	g_main_loop_run(main_loop);
}

gboolean stop_simulator(void) {
	g_main_loop_quit(main_loop);
}

gboolean do_drawing2(gpointer data) {
	GtkWidget *widget = (GtkWidget *)data;
	gint width = widget->allocation.width;
	gint height = widget->allocation.height;
	
	gtk_widget_queue_draw_area(widget, 0, 0, width, height);
	return TRUE;
}

void *run_server(void *ptr) {
	int sock_fd, newsock_fd;
	socklen_t cli_len;
	struct sockaddr_in serv_addr, cli_addr;
	
	GThread *harvester_threads[sim_params.number_of_harvesters];
	GError *err;
		
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		fprintf(stderr, "ERROR opening socket");
		return NULL;
	}
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(sim_params.port_number);
	
	if (bind(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr, "ERROR on binding\n");
		return NULL;
	}
	
	listen(sock_fd, 5);
	cli_len = sizeof(cli_addr);
	
	Harvester_Id harvester_id = -1;
	while (harvester_id < sim_params.number_of_harvesters) {
		newsock_fd = accept(sock_fd, (struct sockaddr *) &cli_addr, &cli_len);
		
		if (newsock_fd < 0) {
			fprintf(stderr, "ERROR on accept\n");
			stop_simulator();
		}
		
		if (harvester_id == -1) {
			send_simulator_drop_zones_info(newsock_fd);
			close(newsock_fd);
			++harvester_id;
			continue;
		}
		
		harvesters_param[harvester_id].harvester_id = harvester_id;
		harvesters_param[harvester_id].harvester_socket = newsock_fd;
		harvesters_param[harvester_id].have_minerals = FALSE;
		
		harvester_threads[harvester_id] = g_thread_create((GThreadFunc)harvester_thread_work,
			(void *)&(harvesters_param[harvester_id++]), TRUE, &err);
	}
	while (harvester_id < sim_params.number_of_harvesters)
		g_thread_join(harvester_threads[harvester_id--]);
	
	close(sock_fd);
	return NULL;
}

void *harvester_thread_work(void *harvester_param_vp) {
	Harvester_Param *harvester_param;
	int rc = 0;
	int sock;
	gboolean run = TRUE;
	
	tpl_node *tn = NULL;
	void *img = NULL;
	size_t sz = 0;
	
	tpl_bin tb;
	int32_t function_name;
	
	harvester_param = (Harvester_Param *)harvester_param_vp;
	sock = harvester_param->harvester_socket;
	
	while (run) {
		
		rc = tpl_gather(TPL_GATHER_BLOCKING, sock, &img, &sz);
		
		if (rc > 0) {
			tn = tpl_map("iB", &function_name, &tb);
			tpl_load(tn, TPL_MEM, img, sz);
			tpl_unpack(tn, 0);
			
			if (function_name >=0 && function_name < END_SIM_CALL)
				(*callback_function_sim[function_name])(&tb, harvester_param->harvester_id);
			
			free(tb.addr);
			tb.addr = NULL;
			tb.sz = 0;
			tpl_free(tn);
		}
		
		if (rc <= 0) {
			run = FALSE;
		}
	}
	
	close(sock);
	return NULL;
}

void init_simulator_images(void) {
	RsvgDimensionData dimension_img;
	int images_index;
	for (images_index = 0; images_index < SIM_IMGS_NUMBER; ++images_index) {
		simulator_imgs[images_index].image =
			rsvg_handle_new_from_file(simulator_images_path[images_index], NULL);
			
		g_assert(simulator_imgs[images_index].image != NULL);
				
		rsvg_handle_get_dimensions(simulator_imgs[images_index].image, &dimension_img);
		
		simulator_imgs[images_index].width = dimension_img.width;
		simulator_imgs[images_index].height = dimension_img.height;
	}
}

void init_harvesters_param(void) {
	if (harvesters_param == NULL)
		harvesters_param = g_malloc0((gsize)(sizeof(Harvester_Param) * sim_params.number_of_harvesters));
}

void init_board_coord_sys(void) {
	if (board_coord_sys == NULL)
		board_coord_sys = g_malloc0((gsize)(sizeof(Board_Coord_Param)
		* sim_params.width_of_board
		* sim_params.height_of_board));
}

void init_board_environment(void) {
	init_board_coord_sys();
	
	init_harvesters_param();
	init_harvesters_on_board();
	
	init_refinerys_param();
	init_refinerys_on_board();
	
	init_drop_zones_param();
	
	init_minerals_param();
	init_minerals_on_board();
}

void init_harvesters_on_board(void) {
	//Wszystkie pojazdy ustawiane sa na pozycji startowej w prawym krancu planszy w pionie
	const int x_coord = sim_params.width_of_board - 1;
	int y_coord;
	
	for (y_coord = 0; y_coord < sim_params.number_of_harvesters; ++y_coord) {
		Board_Coord_Param *field;
		field = get_field_of_board(x_coord, y_coord);
		*field = HARVESTER_ON_FIELD;
		
		harvesters_param[y_coord].harvester_coord.x_coord = x_coord;
		harvesters_param[y_coord].harvester_coord.y_coord = y_coord;
	}
}

void init_refinerys_param(void) {
	int i;
	for (i = 0; i < SIM_REFINERYS_NUMBER; ++i) {
		int x_coord;
		int y_coord;
		
		if (i == SIM_REFINERYS_NUMBER - 1)
			x_coord = sim_params.width_of_board - sim_refinery_width;
		else
			x_coord = 0;
			
		if (i == 0)
			y_coord = 0;
		else
			y_coord = sim_params.height_of_board - sim_refinery_height;
		
		refinerys_param[i].refinery_coord.x_coord = x_coord;
		refinerys_param[i].refinery_coord.y_coord = y_coord;
		
		refinerys_param[i].width = sim_refinery_width;
		refinerys_param[i].height = sim_refinery_height;
	}
}

void init_refinerys_on_board(void) {
	int i;
	for (i = 0; i < SIM_REFINERYS_NUMBER; ++i) {
		int x_coord = refinerys_param[i].refinery_coord.x_coord;
		int y_coord = refinerys_param[i].refinery_coord.y_coord;
		
		int y;
		for (y = 0; y < refinerys_param[i].height; ++y, ++y_coord) {
			int x;
			int next_x_coord = x_coord;
			for (x = 0; x < refinerys_param[i].width; ++x, ++next_x_coord) {
				Board_Coord_Param *field;
				field = get_field_of_board(next_x_coord, y_coord);
				*field = REFINERY_ON_FIELD;
			}
		}
	}
}

void init_minerals_param(void) {
	if (minerals_param == NULL)
		minerals_param = g_hash_table_new_full(g_str_hash, g_str_equal, gHashTableDestroyNotify, gHashTableDestroyNotify);
}

void init_minerals_on_board(void) {
	int seed;
	const int number_of_minerals = sim_params.width_of_board
		* sim_params.height_of_board * sim_params.number_of_minerals / 100;
		
	seed = time(NULL);
	srand(seed);
	
	int min_x= sim_refinery_width + 1;
	int min_y = sim_refinery_height + 1;
	
	int index;
    for (index = 0; index < number_of_minerals; ++index) {
		int x_coord = rand() % ((sim_params.width_of_board - (min_x + 1)) - min_x + 1) + min_x;
		int y_coord = rand() % ((sim_params.height_of_board - (min_y + 1)) - min_y + 1) + min_y;
		
		Board_Coord_Param *field;
		field = get_field_of_board(x_coord, y_coord);
		
		if (*field == EMPTY_FIELD) {
			*field = MINERAL_ON_FIELD;
			
			gchar *coord_key = g_strdup_printf("%d%d", x_coord, y_coord);
			
			Object_Coord_On_Board *mineral_coord =
									(Object_Coord_On_Board *)malloc(sizeof(Object_Coord_On_Board));
			mineral_coord->x_coord = x_coord;
			mineral_coord->y_coord = y_coord;
			
			g_hash_table_insert(minerals_param, coord_key, mineral_coord);
		}
	}
}

void init_drop_zones_param(void) {
	int i;
	for (i = 0; i < SIM_REFINERYS_NUMBER; ++i) {
		int x_coord;
		int y_coord;
		
		if (i == SIM_REFINERYS_NUMBER - 1)
			x_coord = sim_params.width_of_board - sim_refinery_width - 1;
		else
			x_coord = sim_refinery_width;
			
		if (i == 0)
			y_coord = sim_refinery_height - 1;
		else
			y_coord = sim_params.height_of_board - 1;
		
		drop_zones_param[i].x_coord = x_coord;
		drop_zones_param[i].y_coord = y_coord;
	}
}

void init_drop_zones_on_board(void) {
	int i;
	for (i = 0; i < SIM_REFINERYS_NUMBER; ++i) {
		int x_coord = drop_zones_param[i].x_coord;
		int y_coord = drop_zones_param[i].y_coord;
		
		Board_Coord_Param *field;
		field = get_field_of_board(x_coord, y_coord);
		*field = DROP_ZONE_ON_FIELD;
	}
}
