
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

typedef struct Application *Application_t;
typedef struct MainWindow *MainWindow_t;

Application_t Application_new(void);
void Application_free(Application_t application);
void Application_addMainWindow(Application_t application, MainWindow_t main_window);
void Application_removeMainWindow(Application_t application, MainWindow_t main_window);

MainWindow_t MainWindow_new(Application_t application);
void MainWindow_free(MainWindow_t main_window);
void MainWindow_handleDestroy(GtkWidget *window, gpointer user_data);
void MainWindow_handleFileMenuExitMenuItem(gpointer user_data);
void MainWindow_handleFileMenuNewMenuItem(gpointer user_data);

struct Application
{
	GList *main_window_list;
};

struct MainWindow
{
	Application_t application;
	GtkWidget *window;
	GtkAccelGroup *accel_group;
	GtkWidget *vbox;
	GtkWidget *menu_bar;
	GtkWidget *menu_bar_file_menu_item;
	GtkWidget *menu_bar_edit_menu_item;
	GtkWidget *menu_bar_column_menu_item;
	GtkWidget *menu_bar_help_menu_item;
	GtkWidget *file_menu;
	GtkWidget *file_menu_new_menu_item;
	GtkWidget *file_menu_open_menu_item;
	GtkWidget *file_menu_save_menu_item;
	GtkWidget *file_menu_save_as_menu_item;
	GtkWidget *file_menu_close_menu_item;
	GtkWidget *file_menu_exit_menu_item;
	GtkWidget *edit_menu;
	GtkWidget *edit_menu_undo_menu_item;
	GtkWidget *edit_menu_redo_menu_item;
	GtkWidget *edit_menu_cut_menu_item;
	GtkWidget *edit_menu_copy_menu_item;
	GtkWidget *edit_menu_paste_menu_item;
	GtkWidget *column_menu;
	GtkWidget *column_menu_add_menu_item;
	GtkWidget *column_menu_clone_menu_item;
	GtkWidget *column_menu_remove_menu_item;
	GtkWidget *column_menu_move_left_menu_item;
	GtkWidget *column_menu_move_right_menu_item;
	GtkWidget *column_menu_properties_menu_item;
	GtkWidget *help_menu;
	GtkWidget *help_menu_about_menu_item;
	GtkWidget *matrix;
};

int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);

	{
		Application_t application = Application_new();
		Application_addMainWindow(application, MainWindow_new(application));
		gtk_main();
		Application_free(application);
	}

	return 0;
}

Application_t Application_new(void)
{
	Application_t application = (Application_t)(malloc(sizeof(struct Application)));
	application->main_window_list = NULL;
	return application;
}

void Application_free(Application_t application)
{
	while (application->main_window_list != NULL)
	{
		MainWindow_t main_window = (MainWindow_t)(application->main_window_list->data);
		application->main_window_list = g_list_delete_link(application->main_window_list, application->main_window_list);
		MainWindow_free(main_window);
	}

	g_list_free(application->main_window_list);
	free(application);
}

void Application_addMainWindow(Application_t application, MainWindow_t main_window)
{
	application->main_window_list = g_list_append(application->main_window_list, main_window);
}

void Application_removeMainWindow(Application_t application, MainWindow_t main_window)
{
	application->main_window_list = g_list_remove(application->main_window_list, main_window);
	if (application->main_window_list == NULL) gtk_main_quit();
}

MainWindow_t MainWindow_new(Application_t application)
{
	MainWindow_t main_window = (MainWindow_t)(malloc(sizeof(struct MainWindow)));
	main_window->application = application;

	main_window->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(main_window->window), "Matrix");
	gtk_window_set_default_size(GTK_WINDOW(main_window->window), 200, 300);
	g_signal_connect(G_OBJECT(main_window->window), "destroy", G_CALLBACK(MainWindow_handleDestroy), main_window);

	main_window->accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(main_window->window), main_window->accel_group);

	main_window->vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(main_window->window), main_window->vbox);

	main_window->menu_bar = gtk_menu_bar_new();
	gtk_box_pack_start(GTK_BOX(main_window->vbox), main_window->menu_bar, FALSE, FALSE, 2);

	main_window->menu_bar_file_menu_item = gtk_menu_item_new_with_mnemonic("_File");
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->menu_bar), main_window->menu_bar_file_menu_item);

	main_window->menu_bar_edit_menu_item = gtk_menu_item_new_with_mnemonic("_Edit");
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->menu_bar), main_window->menu_bar_edit_menu_item);

	main_window->menu_bar_column_menu_item = gtk_menu_item_new_with_mnemonic("_Column");
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->menu_bar), main_window->menu_bar_column_menu_item);

	main_window->menu_bar_help_menu_item = gtk_menu_item_new_with_mnemonic("_Help");
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->menu_bar), main_window->menu_bar_help_menu_item);

	main_window->file_menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(main_window->menu_bar_file_menu_item), main_window->file_menu);

	main_window->file_menu_new_menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW, main_window->accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->file_menu), main_window->file_menu_new_menu_item);
	g_signal_connect_swapped(G_OBJECT(main_window->file_menu_new_menu_item), "activate", G_CALLBACK(MainWindow_handleFileMenuNewMenuItem), main_window);

	main_window->file_menu_open_menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, main_window->accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->file_menu), main_window->file_menu_open_menu_item);

	main_window->file_menu_save_menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE, main_window->accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->file_menu), main_window->file_menu_save_menu_item);

	main_window->file_menu_save_as_menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE_AS, main_window->accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->file_menu), main_window->file_menu_save_as_menu_item);

	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->file_menu), gtk_separator_menu_item_new());

	main_window->file_menu_close_menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_CLOSE, main_window->accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->file_menu), main_window->file_menu_close_menu_item);

	main_window->file_menu_exit_menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, main_window->accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->file_menu), main_window->file_menu_exit_menu_item);
	g_signal_connect_swapped(G_OBJECT(main_window->file_menu_exit_menu_item), "activate", G_CALLBACK(MainWindow_handleFileMenuExitMenuItem), main_window);

	main_window->edit_menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(main_window->menu_bar_edit_menu_item), main_window->edit_menu);

	main_window->edit_menu_undo_menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_UNDO, main_window->accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->edit_menu), main_window->edit_menu_undo_menu_item);

	main_window->edit_menu_redo_menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_REDO, main_window->accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->edit_menu), main_window->edit_menu_redo_menu_item);

	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->edit_menu), gtk_separator_menu_item_new());

	main_window->edit_menu_cut_menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_CUT, main_window->accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->edit_menu), main_window->edit_menu_cut_menu_item);

	main_window->edit_menu_copy_menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY, main_window->accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->edit_menu), main_window->edit_menu_copy_menu_item);

	main_window->edit_menu_paste_menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE, main_window->accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->edit_menu), main_window->edit_menu_paste_menu_item);

	main_window->column_menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(main_window->menu_bar_column_menu_item), main_window->column_menu);

	main_window->column_menu_add_menu_item = gtk_menu_item_new_with_mnemonic("_Add");
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->column_menu), main_window->column_menu_add_menu_item);

	main_window->column_menu_clone_menu_item = gtk_menu_item_new_with_mnemonic("_Clone");
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->column_menu), main_window->column_menu_clone_menu_item);

	main_window->column_menu_remove_menu_item = gtk_menu_item_new_with_mnemonic("Remo_ve");
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->column_menu), main_window->column_menu_remove_menu_item);

	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->column_menu), gtk_separator_menu_item_new());

	main_window->column_menu_move_left_menu_item = gtk_menu_item_new_with_mnemonic("Move _Left");
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->column_menu), main_window->column_menu_move_left_menu_item);

	main_window->column_menu_move_right_menu_item = gtk_menu_item_new_with_mnemonic("Move _Right");
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->column_menu), main_window->column_menu_move_right_menu_item);

	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->column_menu), gtk_separator_menu_item_new());

	main_window->column_menu_properties_menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_PROPERTIES, main_window->accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->column_menu), main_window->column_menu_properties_menu_item);

	main_window->help_menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(main_window->menu_bar_help_menu_item), main_window->help_menu);

	main_window->help_menu_about_menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, main_window->accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(main_window->help_menu), main_window->help_menu_about_menu_item);

	main_window->matrix = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(main_window->vbox), main_window->matrix);
#if 0
	g_signal_connect(G_OBJECT(main_window->matrix), "configure_event", G_CALLBACK(MainWindow_handleMatrixConfigureEvent), main_window);
	g_signal_connect(G_OBJECT(main_window->matrix), "expose_event", G_CALLBACK(MainWindow_handleMatrixExposeEvent), main_window);
#endif

	gtk_widget_show_all(main_window->window);

	return main_window;
}

void MainWindow_free(MainWindow_t main_window)
{
	free(main_window);
}

void MainWindow_handleDestroy(GtkWidget *widget, gpointer user_data)
{
	MainWindow_t main_window = (MainWindow_t)(user_data);
	Application_removeMainWindow(main_window->application, main_window);
	MainWindow_free(main_window);
}

void MainWindow_handleFileMenuNewMenuItem(gpointer user_data)
{
	MainWindow_t main_window = (MainWindow_t)(user_data);
	Application_addMainWindow(main_window->application, MainWindow_new(main_window->application));
}

void MainWindow_handleFileMenuExitMenuItem(gpointer user_data)
{
	MainWindow_t main_window = (MainWindow_t)(user_data);
	Application_removeMainWindow(main_window->application, main_window);
	MainWindow_free(main_window);
}

