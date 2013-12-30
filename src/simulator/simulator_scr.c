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

#include "tpl.h"

#include "simulator_config.h"

#include "simulator_scr.h"

#include "../common/rpc_all.h"

#define HARVESTER_IMG "icons/AAT-Battle-Tank-icon.png"
#define BOARD_IMG "icons/Taklamakan_desert_sand_dunes_landsat_7.png"

static void run_simulator(void);

static gboolean on_draw_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
static void drawBoard(cairo_t *cr, gint width, gint height);
static void drawGrid(cairo_t *cr, gint width, gint height);
static void drawHarvester(cairo_t *cr, gint width, gint height);

void sigCatcher(int n);
void *run_server(void *ptr);
void *do_work(void *ptr);

void init_board_coord_sys(void);
void init_board_environment(void);
void init_harvester_on_board(void);

int *getFieldOfBoard(int x_coord, int y_coord);
void getCoordOfField(int * const x_coord, int * const y_coord, int index);

gboolean harvester_move_to(tpl_bin tb);

enum {
	EMPTY_FIELD = 0,
	HARVESTER_ON_FIELD
};

//******************GLOBAL VARIABLES********************
static Simulator_Params sim_params = {0};
static int *board_coord_sys = NULL;

static struct {
  cairo_surface_t *harvester_img;
  cairo_surface_t *board_img;
} glob;
//******************************************************

GStaticRecMutex board_coord_sys_mutex = G_STATIC_REC_MUTEX_INIT;

//gboolean do_drawing2(gpointer data);

gboolean (*callback_function_serv[]) (tpl_bin tb) = {
	harvester_move_to
};

int main(int argc, char *argv[]) {
	GThread *main_sim_thread;
	GError *err;
	
	signal(SIGCHLD, sigCatcher);
	
	if (!read_simulator_config(&sim_params))
		return 1;
	printf("Parametry:\nport_number = %d\nharvester_number = %d\nwidth_of_board = %d\nheight_of_board = %d\n",
		sim_params.portNumber, sim_params.harvesterNumber, sim_params.widthOfBoard, sim_params.heightOfBoard);
	
	if (g_thread_supported()) {
		g_thread_init(NULL);
		gdk_threads_init();// Called to initialize internal mutex "gdk_threads_mutex".
	} else {
		printf("g_thread NOT supported\n");
		return 1;
	}
	
	init_board_coord_sys();
	init_board_environment();
	
	if ((main_sim_thread = g_thread_create((GThreadFunc)run_server, NULL, TRUE, &err)) == NULL) {
		printf("Thread create failed: %s!!\n", err->message);
		g_error_free(err);
		return 1;
     }
	
	gtk_init(&argc, &argv);
	
	run_simulator();
	
	return 0;
}

static void run_simulator(void) {
	GtkWidget *main_window;
	GtkWidget *draw_area;
	
	glob.harvester_img = cairo_image_surface_create_from_png(HARVESTER_IMG);
	glob.board_img = cairo_image_surface_create_from_png(BOARD_IMG);
	
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

static gboolean on_draw_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
	cairo_t *cr = gdk_cairo_create( gtk_widget_get_window (widget));
	gint width = widget->allocation.width;
	gint height = widget->allocation.height;
	
	drawBoard(cr, width, height);
	drawGrid(cr, width, height);
	drawHarvester(cr, width, height);
	return FALSE;
}

static void drawBoard(cairo_t *cr, gint width, gint height) {
	cairo_save(cr);
	
	double w = cairo_image_surface_get_width(glob.board_img);
	double h = cairo_image_surface_get_height(glob.board_img);
	
	cairo_scale(cr, width/w, height/h);
		
	cairo_set_source_surface(cr, glob.board_img, 0, 0);
	cairo_paint(cr);
	
	cairo_restore(cr);
}

static void drawGrid(cairo_t *cr, gint width, gint height) {
	gint deltaHeight = height / sim_params.heightOfBoard;
	gint deltaWidth = width / sim_params.widthOfBoard;
	
	cairo_save(cr);
	
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_set_line_width(cr, 2);
	
	int i;
	for (i = 0; i < sim_params.heightOfBoard; i++ ) {
		cairo_move_to(cr, 0, deltaHeight * i);
		cairo_line_to(cr, width, deltaHeight * i);
	}
	for (i = 0; i < sim_params.widthOfBoard; i++ ) {
		cairo_move_to(cr, deltaWidth * i, 0);
		cairo_line_to(cr, deltaWidth * i, height);
	}
	cairo_stroke(cr);
	cairo_restore(cr);
}

static void drawHarvester(cairo_t *cr, gint width, gint height) {
	gint deltaHeight = height / sim_params.heightOfBoard;
	gint deltaWidth = width / sim_params.widthOfBoard;
	
	int x_coord, y_coord;
	
	int index;
	int size = sim_params.widthOfBoard * sim_params.heightOfBoard;
	for (index = 0; index < size; ++index) {
		getCoordOfField(&x_coord, &y_coord, index);
		
		g_static_rec_mutex_lock(&board_coord_sys_mutex);
		
		int *field;
		int value_of_field;
		field = getFieldOfBoard(x_coord, y_coord);
		value_of_field = *field;
		
		g_static_rec_mutex_unlock(&board_coord_sys_mutex);
		
		if (value_of_field != HARVESTER_ON_FIELD) {
			continue;
		}
		
		cairo_save(cr);
		
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_set_line_width(cr, 2);
	
	
		double w = cairo_image_surface_get_width(glob.harvester_img);
		double h = cairo_image_surface_get_height(glob.harvester_img);
	
		cairo_translate(cr, deltaWidth * x_coord, deltaHeight * y_coord);
		cairo_scale(cr, deltaWidth/w, deltaHeight/h);
		
		cairo_set_source_surface(cr, glob.harvester_img, 0, 0);
		cairo_paint(cr);
	
		cairo_restore(cr);
	}
	
	/*cairo_save(cr);
	
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_set_line_width(cr, 2);
	
	//static int i = 0;
	//static int x = 1;
	
	double w = cairo_image_surface_get_width(glob.harvester_img);
	double h = cairo_image_surface_get_height(glob.harvester_img);
	
	cairo_translate(cr, deltaWidth * i, deltaHeight * i);
	cairo_scale(cr, deltaWidth/w, deltaHeight/h);
		
	cairo_set_source_surface(cr, glob.harvester_img, 0, 0);
	cairo_paint(cr);
	
	cairo_restore(cr);
	
	//if (i == sim_params.widthOfBoard - 1)
	//	x = -1;
	//else if (i == 0)
	//	x = 1;
		
	//i += x;
	*/
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
	int sockfd;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	
	GThread *harvester_threads[sim_params.harvesterNumber];
	int harvester_socket[sim_params.harvesterNumber];
	GError *err;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	
	if (sockfd < 0) {
		fprintf(stderr, "ERROR opening socket");
		return NULL;
	}
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(sim_params.portNumber);
	
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr, "ERROR on binding\n");
		return NULL;
	}
	
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	
	int clientsNumber = 0;
	while (1) {
		harvester_socket[clientsNumber] = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		
		if (harvester_socket[clientsNumber] < 0) {
			fprintf(stderr, "ERROR on accept\n");
			break;
		}
		
		if ((harvester_threads[clientsNumber] = g_thread_create((GThreadFunc)do_work,
			(void *)&(harvester_socket[clientsNumber]), TRUE, &err)) == NULL) {
			
			printf("Thread create failed: %s!!\n", err->message);
			g_error_free(err);
			close(harvester_socket[clientsNumber]);
			break;
		}
		++clientsNumber;
		
		if (clientsNumber == sim_params.harvesterNumber)
			break;
		
	} //while
	
	int i = 0;
	while (i < sim_params.harvesterNumber)
		g_thread_join(harvester_threads[i++]);
	
	close(sockfd);
	return NULL;
}

void *do_work(void *ptr) {
	int rc = 0;
	int sock;
	gboolean run = TRUE;
	
	tpl_node *tn = NULL;
	void *img = NULL;
	size_t sz = 0;
	
	tpl_bin tb;
	int32_t function_name;
	
	sock = *(int *)ptr;
	
	while (run) {
		
		rc = tpl_gather(TPL_GATHER_BLOCKING, sock, &img, &sz);
		
		if (rc > 0) {
			tn = tpl_map("iB", &function_name, &tb);
			tpl_load(tn, TPL_MEM, img, sz);
			tpl_unpack(tn, 0);
			
			if (function_name >=0 && function_name < END_FUNCTION)
				(*callback_function_serv[function_name])(tb);
			
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

void init_board_coord_sys(void) {
	if (board_coord_sys == NULL)
		board_coord_sys = g_malloc0((gsize)(sizeof(int)
		* sim_params.widthOfBoard
		* sim_params.heightOfBoard));
}

void init_board_environment(void) {
	if (board_coord_sys == NULL)
		return;
	
	init_harvester_on_board();
}

void init_harvester_on_board(void) {
	int *field;
	int x_coord;
	int y_coord;
	
	x_coord = sim_params.widthOfBoard - 1;
	
	for (y_coord = 0; y_coord < sim_params.harvesterNumber; ++y_coord) {
		int *field;
		field = getFieldOfBoard(x_coord, y_coord);
		*field = HARVESTER_ON_FIELD;
	}
}

gboolean harvester_move_to(tpl_bin tb) {
	if (board_coord_sys == NULL)
		return FALSE;
		
	if (tb.sz <= 0)
		return FALSE;
	
	Move_To_Data *mvd = (Move_To_Data *)tb.addr;	
	
	gboolean ret = FALSE;
	
	g_static_rec_mutex_lock(&board_coord_sys_mutex);
	
	int *field;
	field = getFieldOfBoard(mvd->x_coord, mvd->y_coord);
		
	if (*field == EMPTY_FIELD) {
		*field = HARVESTER_ON_FIELD;
		ret = TRUE;
	}
	
	g_static_rec_mutex_unlock(&board_coord_sys_mutex);
	
	return ret;
}

int *getFieldOfBoard(int x_coord, int y_coord) {
	int *field;
	field = board_coord_sys + sim_params.widthOfBoard * y_coord + x_coord;
	return field;
}

void getCoordOfField(int * const x_coord, int * const y_coord, int index) {
	*x_coord = index % sim_params.widthOfBoard;
	*y_coord = index / sim_params.widthOfBoard;
}
