#include "simulator_data.h"
#include "simulator_img_defines.h"
#include "simulator_draw.h"

typedef struct {
	gint width;
	gint height;
	gint delta_height;
	gint delta_width;
} Draw_Params;

static void draw_board(cairo_t *cr, const Draw_Params * const draw_params);
static void draw_grid(cairo_t *cr, const Draw_Params * const draw_params);
static void draw_harvesters(cairo_t *cr, const Draw_Params * const draw_params);
static void draw_minerals(cairo_t *cr, const Draw_Params * const draw_params);
static void draw_refinerys(cairo_t *cr, const Draw_Params * const draw_params);

gboolean on_draw_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
	cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
	gint width = widget->allocation.width;
	gint height = widget->allocation.height;
	
	Draw_Params draw_params;
	
	draw_params.width = width;
	draw_params.height = height;
	draw_params.delta_width = width / sim_params.width_of_board;
	draw_params.delta_height = height / sim_params.height_of_board;
	
	draw_board(cr, &draw_params);
	draw_grid(cr, &draw_params);
	draw_minerals(cr, &draw_params);
	draw_refinerys(cr, &draw_params);
	draw_harvesters(cr, &draw_params);
	return FALSE;
}

static void draw_board(cairo_t *cr, const Draw_Params * const draw_params) {
	cairo_save(cr);
	
	double image_width = simulator_imgs[BOARD_IMG].width;
	double image_height = simulator_imgs[BOARD_IMG].height;
	
	cairo_scale(cr, draw_params->width / image_width, draw_params->height / image_height);
		
	cairo_set_source_surface(cr, simulator_imgs[BOARD_IMG].image, 0, 0);
	cairo_paint(cr);
	
	cairo_restore(cr);
}

static void draw_grid(cairo_t *cr, const Draw_Params * const draw_params) {
	cairo_save(cr);
	
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_set_line_width(cr, 2);
	
	int i;
	for (i = 0; i < sim_params.height_of_board; i++ ) {
		cairo_move_to(cr, 0, draw_params->delta_height * i);
		cairo_line_to(cr, draw_params->width, draw_params->delta_height * i);
	}
	for (i = 0; i < sim_params.width_of_board; i++ ) {
		cairo_move_to(cr, draw_params->delta_width * i, 0);
		cairo_line_to(cr, draw_params->delta_width * i, draw_params->height);
	}
	cairo_stroke(cr);
	cairo_restore(cr);
}

static void draw_harvesters(cairo_t *cr, const Draw_Params * const draw_params) {	
	int harvester_id;
	int size = sim_params.harvesters_number;
	for (harvester_id = 0; harvester_id < size; ++harvester_id) {
		int x_coord = harvesters_param[harvester_id].harvester_coord.x_coord;
		int y_coord = harvesters_param[harvester_id].harvester_coord.y_coord;
		
		cairo_save(cr);
	
		double image_width = simulator_imgs[HARVESTER_IMG].width;
		double image_height = simulator_imgs[HARVESTER_IMG].height;
	
		cairo_translate(cr, draw_params->delta_width * x_coord,
			draw_params->delta_height * y_coord);
		cairo_scale(cr, draw_params->delta_width / image_width,
			draw_params->delta_height / image_height);
		
		cairo_set_source_surface(cr, simulator_imgs[HARVESTER_IMG].image, 0, 0);
		cairo_paint(cr);
	
		cairo_restore(cr);
	}
}

static void draw_minerals(cairo_t *cr, const Draw_Params * const draw_params) {
	int i;
	for (i = 0; i < 3; ++i) {
		
		cairo_save(cr);
	
		double image_width = simulator_imgs[MINERALS_IMG].width;
		double image_height = simulator_imgs[MINERALS_IMG].height;
		
		int x = i + 5;
	
		cairo_translate(cr, draw_params->delta_width * x, draw_params->delta_height * x);
		cairo_scale(cr, draw_params->delta_height / image_width,
			draw_params->delta_height / image_height);
		
		cairo_set_source_surface(cr, simulator_imgs[MINERALS_IMG].image, 0, 0);
		cairo_paint(cr);
	
		cairo_restore(cr);
	}
}

static void draw_refinerys(cairo_t *cr, const Draw_Params * const draw_params) {
	double image_width = simulator_imgs[REFINERY_IMG].width;
	double image_height = simulator_imgs[REFINERY_IMG].height;
	
	int i;
	for (i = 0; i < SIM_REFINERYS_NUMBER; ++i) {
		int x_coord = refinerys_param[i].refinery_coord.x_coord;
		int y_coord = refinerys_param[i].refinery_coord.y_coord;
		
		cairo_save(cr);
		
		cairo_translate(cr, draw_params->delta_width * x_coord, draw_params->delta_height * y_coord);
		cairo_scale(cr, draw_params->delta_width * refinerys_param[i].width / image_width,
		draw_params->delta_height * refinerys_param[i].width / image_height);
		
		cairo_set_source_surface(cr, simulator_imgs[REFINERY_IMG].image, 0, 0);
		cairo_paint(cr);
	
		cairo_restore(cr);
	}
}
