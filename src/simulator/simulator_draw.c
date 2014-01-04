#include <cairo.h>
#include <librsvg/rsvg.h>

#include "simulator_data.h"
#include "simulator_img_defines.h"
#include "simulator_draw.h"

typedef struct {
	gint width;
	gint height;
	double delta_height;
	double delta_width;
} Draw_Params;

static void draw_board(cairo_t *cr, const Draw_Params * const draw_params);
static void draw_grid(cairo_t *cr, const Draw_Params * const draw_params);
static void draw_harvesters(cairo_t *cr, const Draw_Params * const draw_params);
static void draw_minerals(cairo_t *cr, const Draw_Params * const draw_params);
static void draw_refinerys(cairo_t *cr, const Draw_Params * const draw_params);
static void draw_drop_zones(cairo_t *cr, const Draw_Params * const draw_params);

gboolean on_draw_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data) {
	cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
	gint width = widget->allocation.width;
	gint height = widget->allocation.height;
	
	//printf("width %d height %d\n", width, height);
	
	Draw_Params draw_params;
	
	draw_params.width = width;
	draw_params.height = height;
	draw_params.delta_width = width / (double)sim_params.width_of_board;
	draw_params.delta_height = height / (double)sim_params.height_of_board;
	
	draw_board(cr, &draw_params);
	draw_grid(cr, &draw_params);
	draw_minerals(cr, &draw_params);
	draw_refinerys(cr, &draw_params);
	draw_harvesters(cr, &draw_params);
	draw_drop_zones(cr, &draw_params);
	return FALSE;
}

static void draw_board(cairo_t *cr, const Draw_Params * const draw_params) {
	const int image_width = simulator_imgs[BOARD_IMG].width;
	const int image_height = simulator_imgs[BOARD_IMG].height;
	
	cairo_save(cr);
	
	//cairo_translate(cr, 0.0, 0.0);
	
	cairo_scale(cr, draw_params->width / (double)image_width,
		draw_params->height / (double)image_height);
		
	rsvg_handle_render_cairo(simulator_imgs[BOARD_IMG].image, cr);
	
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
	const int image_width = simulator_imgs[HARVESTER_IMG].width;
	const int image_height = simulator_imgs[HARVESTER_IMG].height;
	
	int index;
	int size = sim_params.number_of_harvesters;
	for (index = 0; index < size; ++index) {
		int x_coord = harvesters_param[index].harvester_coord.x_coord;
		int y_coord = harvesters_param[index].harvester_coord.y_coord;
				
		cairo_save(cr);
	
		cairo_translate(cr, draw_params->delta_width * x_coord,
			draw_params->delta_height * y_coord);
		cairo_scale(cr, draw_params->delta_width / image_width,
			draw_params->delta_height / image_height);
		
		rsvg_handle_render_cairo(simulator_imgs[HARVESTER_IMG].image, cr);
	
		cairo_restore(cr);
	}
}

static void draw_minerals(cairo_t *cr, const Draw_Params * const draw_params) {
	const int number_of_minerals = sim_params.width_of_board * sim_params.height_of_board
		* sim_params.number_of_minerals / 100;
	const int image_width = simulator_imgs[MINERALS_IMG].width;
	const int image_height = simulator_imgs[MINERALS_IMG].height;
	
		
	int index;
	for (index = 0; index < number_of_minerals; ++index) {
		
		if (!minerals_param[index].is_exist)
			continue;
		
		cairo_save(cr);
		
		int x_coord = minerals_param[index].mineral_coord.x_coord;
		int y_coord = minerals_param[index].mineral_coord.y_coord;
	
		cairo_translate(cr, draw_params->delta_width * x_coord, draw_params->delta_height * y_coord);
		cairo_scale(cr, draw_params->delta_width / image_width,
			draw_params->delta_height / image_height);
		
		rsvg_handle_render_cairo(simulator_imgs[MINERALS_IMG].image, cr);
	
		cairo_restore(cr);
	}
}

static void draw_refinerys(cairo_t *cr, const Draw_Params * const draw_params) {
	const int image_width = simulator_imgs[REFINERY_IMG].width;
	const int image_height = simulator_imgs[REFINERY_IMG].height;
	
	int index;
	for (index = 0; index < SIM_REFINERYS_NUMBER; ++index) {
		int x_coord = refinerys_param[index].refinery_coord.x_coord;
		int y_coord = refinerys_param[index].refinery_coord.y_coord;
		
		cairo_save(cr);
		
		cairo_translate(cr, draw_params->delta_width * x_coord, draw_params->delta_height * y_coord);
		cairo_scale(cr, draw_params->delta_width * refinerys_param[index].width / image_width,
		draw_params->delta_height * refinerys_param[index].height / image_height);
		
		rsvg_handle_render_cairo(simulator_imgs[REFINERY_IMG].image, cr);
			
		cairo_restore(cr);
	}
}

static void draw_drop_zones(cairo_t *cr, const Draw_Params * const draw_params) {
	const int image_width = simulator_imgs[DROP_ZONE_IMG].width;
	const int image_height = simulator_imgs[DROP_ZONE_IMG].height;
	
	int index;
	for (index = 0; index < SIM_REFINERYS_NUMBER; ++index) {
		int x_coord = drop_zones_param[index].x_coord;
		int y_coord = drop_zones_param[index].y_coord;
				
		cairo_save(cr);
	
		cairo_translate(cr, draw_params->delta_width * x_coord,
			draw_params->delta_height * y_coord);
		cairo_scale(cr, draw_params->delta_width / image_width,
			draw_params->delta_height / image_height);
		
		rsvg_handle_render_cairo(simulator_imgs[DROP_ZONE_IMG].image, cr);	
		cairo_restore(cr);
	}
}
