
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <gtk/gtk.h>
#include "button-grid.h"
#include "scheduler.h"

typedef struct
{
	GtkWindow *window;
	GtkVBox *vbox;
	GtkFrame *frame;
	ButtonGrid *button_grid;
	GtkHButtonBox *button_box;
	GtkToggleButton *swing_button;
	GtkToggleButton *mute_button;
	GtkButton *clear_button;
	HMIDIOUT midi_out;
	guint column_number;
	guint sub_column_number;
	Scheduler *scheduler;
}
Application;

static void scheduler_callback(gpointer user_data)
{
	Application *application = (Application *)(user_data);
	gboolean swing = gtk_toggle_button_get_active(application->swing_button);
	gboolean mute = gtk_toggle_button_get_active(application->mute_button);

	if ((application->sub_column_number == 0) || (application->sub_column_number == (swing ? 4 : 3)))
	{
		guint row_number;

		for (row_number = 0; row_number < button_grid_get_number_of_rows(application->button_grid); row_number++)
		{
			button_grid_set_button_value(BUTTON_GRID(application->button_grid), application->column_number, row_number, button_grid_get_button_value(BUTTON_GRID(application->button_grid), application->column_number, row_number) & ~2);
		}

		application->column_number = (application->column_number + 1) % button_grid_get_number_of_columns(application->button_grid);

		for (row_number = 0; row_number < button_grid_get_number_of_rows(application->button_grid); row_number++)
		{
			guint value = button_grid_get_button_value(BUTTON_GRID(application->button_grid), application->column_number, row_number);
			button_grid_set_button_value(BUTTON_GRID(application->button_grid), application->column_number, row_number, value | 2);

			if (!mute && (value & 1))
			{
				BYTE row_note_number[] = {75, 70, 56, 39, 49, 46, 42, 37, 38, 36};

				union
				{
					DWORD dwData;
					BYTE bData[4];
				}
				u;

				u.bData[0] = 0x90 | 9;
				u.bData[1] = row_note_number[row_number];
				u.bData[2] = 63;
				u.bData[3] = 0;
				midiOutShortMsg(application->midi_out, u.dwData);
			}
		}
	}

	application->sub_column_number = (application->sub_column_number + 1) % 6;
}

static void button_grid_button_pressed_callback(GtkWidget *button_grid, guint column_number, guint row_number, gpointer user_data)
{
	button_grid_set_button_value(BUTTON_GRID(button_grid), column_number, row_number, button_grid_get_button_value(BUTTON_GRID(button_grid), column_number, row_number) ^ 1);
}

static void clear_button_clicked_callback(GtkButton *clear_button, gpointer user_data)
{
	ButtonGrid *button_grid = BUTTON_GRID(user_data);
	guint column_number;
	guint row_number;

	for (column_number = 0; column_number < button_grid_get_number_of_columns(button_grid); column_number++)
	{
		for (row_number = 0; row_number < button_grid_get_number_of_rows(button_grid); row_number++)
		{
			button_grid_set_button_value(button_grid, column_number, row_number, button_grid_get_button_value(button_grid, column_number, row_number) & 2);
		}
	}
}

int main(int argc, char **argv)
{
	Application application;

	g_thread_init(NULL);
	gtk_init(&argc, &argv);

	application.window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
	gtk_window_set_title(application.window, "Sequencer #1");
	gtk_window_set_resizable(application.window, FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(application.window), 2);
	g_signal_connect(G_OBJECT(application.window), "destroy", G_CALLBACK(exit), NULL);

	application.vbox = GTK_VBOX(gtk_vbox_new(FALSE, 4));
	gtk_container_add(GTK_CONTAINER(application.window), GTK_WIDGET(application.vbox));

	application.frame = GTK_FRAME(gtk_frame_new(NULL));
	gtk_frame_set_shadow_type(application.frame, GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(application.vbox), GTK_WIDGET(application.frame));

	application.button_grid = button_grid_new();
	button_grid_set_value_pixbuf(application.button_grid, 0, gdk_pixbuf_new_from_file("0.png", NULL));
	button_grid_set_value_pixbuf(application.button_grid, 1, gdk_pixbuf_new_from_file("1.png", NULL));
	button_grid_set_value_pixbuf(application.button_grid, 2, gdk_pixbuf_new_from_file("1.png", NULL));
	button_grid_set_value_pixbuf(application.button_grid, 3, gdk_pixbuf_new_from_file("1.png", NULL));
	button_grid_set_dimensions(application.button_grid, 16, 10);
	g_signal_connect(G_OBJECT(application.button_grid), "button_pressed", G_CALLBACK(button_grid_button_pressed_callback), NULL);
	gtk_container_add(GTK_CONTAINER(application.frame), GTK_WIDGET(application.button_grid));

	application.button_box = GTK_HBUTTON_BOX(gtk_hbutton_box_new());
	gtk_button_box_set_layout(GTK_BUTTON_BOX(application.button_box), GTK_BUTTONBOX_END);
	gtk_box_set_spacing(GTK_BOX(application.button_box), 4);
	gtk_container_add(GTK_CONTAINER(application.vbox), GTK_WIDGET(application.button_box));

	application.swing_button = GTK_TOGGLE_BUTTON(gtk_toggle_button_new_with_mnemonic("S_wing"));
	gtk_container_add(GTK_CONTAINER(application.button_box), GTK_WIDGET(application.swing_button));

	application.mute_button = GTK_TOGGLE_BUTTON(gtk_toggle_button_new_with_mnemonic("_Mute"));
	gtk_container_add(GTK_CONTAINER(application.button_box), GTK_WIDGET(application.mute_button));

	application.clear_button = GTK_BUTTON(gtk_button_new_with_mnemonic("_Clear"));
	g_signal_connect(G_OBJECT(application.clear_button), "clicked", G_CALLBACK(clear_button_clicked_callback), application.button_grid);
	gtk_container_add(GTK_CONTAINER(application.button_box), GTK_WIDGET(application.clear_button));

	gtk_widget_show(GTK_WIDGET(application.clear_button));
	gtk_widget_show(GTK_WIDGET(application.mute_button));
	gtk_widget_show(GTK_WIDGET(application.swing_button));
	gtk_widget_show(GTK_WIDGET(application.button_box));
	gtk_widget_show(GTK_WIDGET(application.button_grid));
	gtk_widget_show(GTK_WIDGET(application.frame));
	gtk_widget_show(GTK_WIDGET(application.vbox));
	gtk_widget_show(GTK_WIDGET(application.window));

	midiOutOpen(&(application.midi_out), MIDI_MAPPER, 0, 0, 0);

	application.column_number = button_grid_get_number_of_columns(application.button_grid) - 1;
	application.sub_column_number = 0;

	application.scheduler = scheduler_new(G_USEC_PER_SEC / 24, scheduler_callback, &application);
	gtk_main();

	midiOutClose(application.midi_out);

	return 0;
}

