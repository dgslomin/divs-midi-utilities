
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <gtk/gtk.h>

static void destroy(GtkWidget *widget, gpointer user_data)
{
	gtk_main_quit();
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR command_line, int show_state)
{
	gtk_init(&__argc, &__argv);

	{
		GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);

		{
			GtkWidget *vbox = gtk_vbox_new(TRUE, 2);
			gtk_container_set_border_width(GTK_CONTAINER(vbox), 2);

			{
				GtkWidget *file_chooser_button = gtk_file_chooser_button_new("Select Folder...", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
				gtk_container_add(GTK_CONTAINER(vbox), file_chooser_button);
				gtk_widget_show(file_chooser_button);
			}

			{
				GtkWidget *file_chooser_button = gtk_file_chooser_button_new("Select Folder...", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
				gtk_container_add(GTK_CONTAINER(vbox), file_chooser_button);
				gtk_widget_show(file_chooser_button);
			}

			gtk_container_add(GTK_CONTAINER(window), vbox);
			gtk_widget_show(vbox);
		}

		gtk_widget_show(window);
	}

	gtk_main();
	return 0;
}

