
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "button-grid.h"

static void button_pressed_callback(GtkWidget *button_grid, guint column_number, guint row_number, gpointer user_data)
{
	button_grid_set_button_value(BUTTON_GRID(button_grid), column_number, row_number, 1 - button_grid_get_button_value(BUTTON_GRID(button_grid), column_number, row_number));
}

int main(int argc, char **argv)
{
	GtkWindow *window;
	GtkFrame *frame;
	ButtonGrid *button_grid;

	gtk_init(&argc, &argv);

	window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
	gtk_window_set_title(window, "Button Grid Test");
	gtk_window_set_resizable(window, FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(window), 2);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(exit), NULL);

	frame = GTK_FRAME(gtk_frame_new(NULL));
	gtk_frame_set_shadow_type(frame, GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(frame));

	button_grid = button_grid_new();
	button_grid_set_value_pixbuf(button_grid, 0, gdk_pixbuf_new_from_file("0.png", NULL));
	button_grid_set_value_pixbuf(button_grid, 1, gdk_pixbuf_new_from_file("1.png", NULL));
	button_grid_set_dimensions(button_grid, 8, 8);
	g_signal_connect(G_OBJECT(button_grid), "button_pressed", G_CALLBACK(button_pressed_callback), NULL);
	gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET(button_grid));

	gtk_widget_show(GTK_WIDGET(button_grid));
	gtk_widget_show(GTK_WIDGET(frame));
	gtk_widget_show(GTK_WIDGET(window));

	gtk_main();
	return 0;
}

