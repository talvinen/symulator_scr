#include <cairo.h>
#include <gtk/gtk.h>
//#include <math.h>

//#include <unistd.h>
#include <stdlib.h>
#include <string.h>

//#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
//#include <netdb.h>
#include <errno.h>

#include <signal.h>

#include <tpl.h>

#include "simulator_config.h"

#include "simulator_scr.h"
#include "simulator_img_defines.h"
#include "simulator_data.h"
#include "simulator_draw.h"
#include "simulator_remote_calls.h"
#include "board_help_calcs.h"

#include "../common/rpc_all.h"
#include "../common/rpc_data.h"

static void run_simulator(void);

void sigCatcher(int n);
void *run_server(void *ptr);
void *harvester_thread_work(void *harvester_param_vp);

void init_simulator_images(void);
void init_board_coord_sys(void);
void init_harvesters_param(void);
void init_board_environment(void);
void init_harvesters_on_board(void);
void init_refinerys_param(void);
void init_refinerys_on_board(void);

//******************GLOBAL VARIABLES********************
Simulator_Params sim_params = {0};
int *board_coord_sys = NULL;
Harvester_Param *harvesters_param = NULL;

Refinery_Param refinerys_param[SIM_REFINERYS_NUMBER];
Simulator_Imgs simulator_imgs[SIM_IMGS_NUMBER];

GStaticRecMutex board_coord_sys_mutex = G_STATIC_REC_MUTEX_INIT;
//******************************************************

int main(int argc, char *argv[]) {
	GThread *main_sim_thread;
	GError *err;
	
	signal(SIGCHLD, sigCatcher);
	
	if (!read_simulator_config(&sim_params))
		return 1;
	printf("Parametry:\nport_number = %d\nharvester_number = %d\nwidth_of_board = %d\nheight_of_board = %d\n",
		sim_params.port_number, sim_params.harvesters_number, sim_params.width_of_board, sim_params.height_of_board);
	
	if (g_thread_supported()) {
		g_thread_init(NULL);
		gdk_threads_init();// Called to initialize internal mutex "gdk_threads_mutex".
	} else {
		printf("g_thread NOT supported\n");
		return 1;
	}
	
	init_board_coord_sys();
	init_harvesters_param();
	init_refinerys_param();
	init_refinerys_on_board();
	init_board_environment();
	
	if ((main_sim_thread = g_thread_create((GThreadFunc)run_server, NULL, TRUE, &err)) == NULL) {
		printf("Thread create failed: %s!!\n", err->message);
		g_error_free(err);
		return 1;
     }
	
	gtk_init(&argc, &argv);
	
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
	g_signal_connect(main_window, "destroy",
		G_CALLBACK(gtk_main_quit), NULL);
	
	gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600); 
	gtk_window_set_title(GTK_WINDOW(main_window), "Symulator");
	
	gtk_widget_show_all(main_window);

	gtk_main();	
}

gboolean do_drawing2(gpointer data) {
	GtkWidget *widget = (GtkWidget *)data;
	gint width = widget->allocation.width;
	gint height = widget->allocation.height;
	
	gtk_widget_queue_draw_area(widget, 0, 0, width, height);
	return TRUE;
}

void sigCatcher(int n) {
	wait3(NULL, WNOHANG, NULL);
}

void *run_server(void *ptr) {
	int sock_fd, newsock_fd;
	socklen_t cli_len;
	struct sockaddr_in serv_addr, cli_addr;
	
	GThread *harvester_threads[sim_params.harvesters_number];
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
	
	Harvester_Id harvester_id = 0;
	while (TRUE) {
		newsock_fd = accept(sock_fd, (struct sockaddr *) &cli_addr, &cli_len);
		
		if (newsock_fd < 0) {
			fprintf(stderr, "ERROR on accept\n");
			break;
		}
		
		harvesters_param[harvester_id].harvester_id = harvester_id;
		harvesters_param[harvester_id].harvester_socket = newsock_fd;
		
		if ((harvester_threads[harvester_id] = g_thread_create((GThreadFunc)harvester_thread_work,
			(void *)&(harvesters_param[harvester_id]), TRUE, &err)) == NULL) {
			
			printf("Thread create failed: %s!!\n", err->message);
			g_error_free(err);
			close(harvesters_param[harvester_id].harvester_socket);
			break;
		}
		++harvester_id;
		
		if (harvester_id == sim_params.harvesters_number)
			break;
		
	} //while
	
	harvester_id = 0;
	while (harvester_id < sim_params.harvesters_number)
		g_thread_join(harvester_threads[harvester_id++]);
	
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
	int images_index;
	for (images_index = 0; images_index < SIM_IMGS_NUMBER; ++images_index) {
		simulator_imgs[images_index].image =
			cairo_image_surface_create_from_png(simulator_images_path[images_index]);
		
		simulator_imgs[images_index].width =
			cairo_image_surface_get_width(simulator_imgs[images_index].image);
		
		simulator_imgs[images_index].height =
			cairo_image_surface_get_height(simulator_imgs[images_index].image);
	}
}

void init_board_coord_sys(void) {
	if (board_coord_sys == NULL)
		board_coord_sys = g_malloc0((gsize)(sizeof(int)
		* sim_params.width_of_board
		* sim_params.height_of_board));
}

void init_harvesters_param(void) {
	if (harvesters_param == NULL)
		harvesters_param = g_malloc0((gsize)(sizeof(Harvester_Param) * sim_params.harvesters_number));
}

void init_board_environment(void) {
	if (board_coord_sys == NULL)
		return;
	
	init_harvesters_on_board();
}

void init_harvesters_on_board(void) {
	int x_coord;
	int y_coord;
	
	//Wszystkie pojazdy ustawiane sa na pozycji startowej w prawym krancu planszy w pionie
	x_coord = sim_params.width_of_board - 1;
	
	for (y_coord = 0; y_coord < sim_params.harvesters_number; ++y_coord) {
		int *field;
		field = get_field_of_board(x_coord, y_coord);
		*field = OBJECT_ON_FIELD;
		
		harvesters_param[y_coord].harvester_coord.x_coord = x_coord;
		harvesters_param[y_coord].harvester_coord.y_coord = y_coord;
	}
}

void init_refinerys_param(void) {
	int x_coord;
	int y_coord;
	
	int sim_refinery_width = 3;
	int sim_refinery_height = 3;
	
	x_coord = sim_params.width_of_board - 1;
	
	int i;
	for (i = 0; i < SIM_REFINERYS_NUMBER; ++i) {
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
				int *field;
				field = get_field_of_board(next_x_coord, y_coord);
				*field = OBJECT_ON_FIELD;
			}
		}
	}
}
