#ifndef SIMULATOR_DRAW_H
#define SIMULATOR_DRAW_H

#include <cairo.h>
#include <glib.h>

gboolean on_draw_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);

#endif//SIMULATOR_DRAW_H
