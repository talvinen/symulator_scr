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

#include <signal.h>

#include "simulator_config.h"

#include "simulator_scr.h"

#define HARVESTER_IMG "icons/AAT-Battle-Tank-icon.png"
#define BOARD_IMG "icons/Taklamakan_desert_sand_dunes_landsat_7.png"

struct {
  cairo_surface_t *harvester_img;
  cairo_surface_t *board_img;
} glob;

static gboolean on_draw_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);

static void drawBoard(cairo_t *cr, gint width, gint height);
static void drawGrid(cairo_t *cr, gint width, gint height);
static void drawHarvester(cairo_t *cr, gint width, gint height);

void sigCatcher(int n);
void *run_server(void *ptr);

static Simulator_Params sim_params;

//gboolean do_drawing2(gpointer data);

int main(int argc, char *argv[]) {
	GThread *Thread1;
	GError *err1 = NULL;
	
	//Simulator_Params sim_params;
	
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
	
	GtkWidget *main_window;
	GtkWidget *draw_area;
		
	gtk_init(&argc, &argv);
	
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
	
	if ((Thread1 = g_thread_create((GThreadFunc)run_server, /*(void *)&sim_params.portNumber*/NULL, TRUE, &err1)) == NULL) {
		printf("Thread create failed: %s!!\n", err1->message);
		g_error_free(err1);
		return 1;
     }

	gtk_main();	
	return 0;
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
	gint deltaHeight = height / 10;
	gint deltaWidth = width / 10;
	
	cairo_save(cr);
	
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_set_line_width(cr, 2);
	
	int i;
	for (i = 0; i < 10; i++ ) {
		cairo_move_to(cr, 0, deltaHeight * i);
		cairo_line_to(cr, width, deltaHeight * i);
	}
	for (i = 0; i < 10; i++ ) {
		cairo_move_to(cr, deltaWidth * i, 0);
		cairo_line_to(cr, deltaWidth * i, height);
	}
	cairo_stroke(cr);
	cairo_restore(cr);
}

static void drawHarvester(cairo_t *cr, gint width, gint height) {
	gint deltaHeight = height / 10;
	gint deltaWidth = width / 10;
	
	cairo_save(cr);
	
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_set_line_width(cr, 2);
	
	static int i = 0;
	static int x = 1;
	
	double w = cairo_image_surface_get_width(glob.harvester_img);
	double h = cairo_image_surface_get_height(glob.harvester_img);
	
	cairo_translate(cr, deltaWidth * i, deltaHeight * i);
	cairo_scale(cr, deltaWidth/w, deltaHeight/h);
		
	cairo_set_source_surface(cr, glob.harvester_img, 0, 0);
	cairo_paint(cr);
	
	cairo_restore(cr);
	
	if (i == 9)
		x = -1;
	else if (i == 0)
		x = 1;
		
	i += x;
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
	int sockfd, newsockfd, portno, pid;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	
	if (sockfd < 0) {
		fprintf(stderr, "ERROR opening socket");
		return NULL;
	}
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = /**(int *)ptr*/sim_params.portNumber;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr, "ERROR on binding\n");
		return NULL;
	}
	
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	
	int clientsNumber = 0;
	while (1) {
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		
		if (newsockfd < 0) {
			fprintf(stderr, "ERROR on accept\n");
			break;
		}
		
		++clientsNumber;
		
		//if (clientsNumber == 
		
		pid = fork();
		
		if (pid < 0) {
			fprintf(stderr, "ERROR on fork\n");
			break;
		}
		
		if (pid == 0)  {
			close(sockfd);
			//doWork(newsockfd);
			break;
		} else {
			close(newsockfd);
		}
	} //while
	
	close(sockfd);
	return NULL;
}
