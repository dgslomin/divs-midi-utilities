
#include <gtk/gtk.h>
#include "button-grid.h"

static void button_grid_cclosure_marshal_VOID__UINT_UINT(GClosure *closure, GValue *return_value, guint n_param_values, const GValue *param_values, gpointer invocation_hint, gpointer marshal_data)
{
	typedef void (*GMarshalFunc_VOID__UINT_UINT)(gpointer data1, guint arg_1, guint arg_2, gpointer data2);
	register GMarshalFunc_VOID__UINT_UINT callback;
	register GCClosure *cc = (GCClosure*)(closure);
	register gpointer data1, data2;

	g_return_if_fail (n_param_values == 3);

	if (G_CCLOSURE_SWAP_DATA(closure))
	{
		data1 = closure->data;
		data2 = g_value_peek_pointer(param_values + 0);
	}
	else
	{
		data1 = g_value_peek_pointer(param_values + 0);
		data2 = closure->data;
	}

	callback = (GMarshalFunc_VOID__UINT_UINT)(marshal_data ? marshal_data : cc->callback); 
	callback(data1, g_value_get_uint(param_values + 1), g_value_get_uint(param_values + 2), data2);
}

#define BUTTON_GRID_GET_BUTTON_NUMBER(button_grid, column_number, row_number) ((button_grid->number_of_columns * row_number) + column_number)

static void button_grid_class_init(ButtonGridClass *button_grid_class)
{
	button_grid_class->button_pressed_signal = g_signal_new("button_pressed", G_TYPE_FROM_CLASS(button_grid_class), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION, 0, NULL, NULL, button_grid_cclosure_marshal_VOID__UINT_UINT, G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_UINT);
}

static gboolean button_grid_button_press_callback(GtkWidget *event_box, GdkEventButton *button_event, gpointer user_data)
{
	ButtonGrid *button_grid = BUTTON_GRID(g_object_get_qdata(G_OBJECT(event_box), g_quark_from_string("button_grid")));
	guint column_number = GPOINTER_TO_UINT(g_object_get_qdata(G_OBJECT(event_box), g_quark_from_string("column_number")));
	guint row_number = GPOINTER_TO_UINT(g_object_get_qdata(G_OBJECT(event_box), g_quark_from_string("row_number")));
	g_signal_emit(G_OBJECT(button_grid), BUTTON_GRID_CLASS(G_OBJECT_GET_CLASS(button_grid))->button_pressed_signal, 0, column_number, row_number);
	return TRUE;
}

static void button_grid_remove_child_callback(GtkWidget *event_box, gpointer user_data)
{
	gtk_container_remove(GTK_CONTAINER(user_data), event_box);
}

static void button_grid_do_layout(ButtonGrid *button_grid)
{
	GdkPixbuf *pixbuf;
	guint row_number;
	guint column_number;
	GtkEventBox *event_box;

	button_grid->button_values = (guint *)(realloc(button_grid->button_values, button_grid->number_of_rows * button_grid->number_of_columns * sizeof (guint)));
	button_grid->button_images = (GtkImage **)(realloc(button_grid->button_images, button_grid->number_of_rows * button_grid->number_of_columns * sizeof (GtkImage *)));
	button_grid->button_value_has_changed = (gboolean *)(realloc(button_grid->button_value_has_changed, button_grid->number_of_rows * button_grid->number_of_columns * sizeof (gboolean)));
	gtk_container_foreach(GTK_CONTAINER(button_grid), button_grid_remove_child_callback, button_grid);
	gtk_table_resize(GTK_TABLE(button_grid), button_grid->number_of_columns, button_grid->number_of_rows);

	if (button_grid->number_of_value_pixbufs > 0)
	{
		pixbuf = button_grid->value_pixbufs[0];
	}
	else
	{
		pixbuf = NULL;
	}

	for (row_number = 0; row_number < button_grid->number_of_rows; row_number++)
	{
		for (column_number = 0; column_number < button_grid->number_of_columns; column_number++)
		{
			guint button_number = BUTTON_GRID_GET_BUTTON_NUMBER(button_grid, column_number, row_number);
			button_grid->button_values[button_number] = 0;
			button_grid->button_images[button_number] = GTK_IMAGE(gtk_image_new_from_pixbuf(pixbuf));
			button_grid->button_value_has_changed[button_number] = FALSE;
			event_box = GTK_EVENT_BOX(gtk_event_box_new());
			g_object_set_qdata(G_OBJECT(event_box), g_quark_from_string("button_grid"), button_grid);
			g_object_set_qdata(G_OBJECT(event_box), g_quark_from_string("column_number"), GUINT_TO_POINTER(column_number));
			g_object_set_qdata(G_OBJECT(event_box), g_quark_from_string("row_number"), GUINT_TO_POINTER(row_number));
			g_signal_connect(G_OBJECT(event_box), "button_press_event", G_CALLBACK(button_grid_button_press_callback), NULL);
			gtk_container_add(GTK_CONTAINER(event_box), GTK_WIDGET(button_grid->button_images[button_number]));
			gtk_table_attach_defaults(GTK_TABLE(button_grid), GTK_WIDGET(event_box), column_number, column_number + 1, row_number, row_number + 1);
			gtk_widget_show(GTK_WIDGET(button_grid->button_images[button_number]));
			gtk_widget_show(GTK_WIDGET(event_box));
		}
	}
}

static gboolean button_grid_idle_callback(gpointer user_data)
{
	ButtonGrid *button_grid = BUTTON_GRID(user_data);

	if (g_atomic_int_compare_and_exchange(&(button_grid->some_button_value_has_changed), 1, 0))
	{
		guint row_number;
		guint column_number;

		for (row_number = 0; row_number < button_grid->number_of_rows; row_number++)
		{
			for (column_number = 0; column_number < button_grid->number_of_columns; column_number++)
			{
				guint button_number = BUTTON_GRID_GET_BUTTON_NUMBER(button_grid, column_number, row_number);

				if (g_atomic_int_compare_and_exchange(&(button_grid->button_value_has_changed[button_number]), 1, 0))
				{
					guint value = button_grid->button_values[button_number];
					GdkPixbuf *pixbuf;

					if (value < button_grid->number_of_value_pixbufs)
					{
						pixbuf = button_grid->value_pixbufs[value];
					}
					else
					{
						pixbuf = NULL;
					}

					gtk_image_set_from_pixbuf(button_grid->button_images[button_number], pixbuf);
				}
			}
		}
	}

	return TRUE;
}

static void button_grid_init(ButtonGrid *button_grid)
{
	button_grid->number_of_value_pixbufs = 0;
	button_grid->value_pixbufs = NULL;
	button_grid->number_of_columns = 1;
	button_grid->number_of_rows = 1;
	button_grid->button_values = NULL;
	button_grid->button_images = NULL;
	button_grid->button_value_has_changed = NULL;
	g_atomic_int_set(&(button_grid->some_button_value_has_changed), 0);
	button_grid_do_layout(button_grid);
	g_idle_add(button_grid_idle_callback, button_grid);
}

GType button_grid_get_type(void)
{
	static GType type = 0;

	if (type == 0)
	{
		static const GTypeInfo type_info = {sizeof (ButtonGridClass), NULL, NULL, (GClassInitFunc)(button_grid_class_init), NULL, NULL, sizeof (ButtonGrid), 0, (GInstanceInitFunc)(button_grid_init)};
		type = g_type_register_static(GTK_TYPE_TABLE, "ButtonGridType", &type_info, 0);
	}

	return type;
}

ButtonGrid *button_grid_new(void)
{
	return BUTTON_GRID(g_object_new(BUTTON_GRID_TYPE, NULL));
}

void button_grid_set_value_pixbuf(ButtonGrid *button_grid, int value, GdkPixbuf *pixbuf)
{
	guint row_number;
	guint column_number;

	if (value >= button_grid->number_of_value_pixbufs)
	{
		button_grid->value_pixbufs = (GdkPixbuf **)(realloc(button_grid->value_pixbufs, (value + 1) * sizeof (GdkPixbuf *)));
		memset(button_grid->value_pixbufs + button_grid->number_of_value_pixbufs, 0, (value - button_grid->number_of_value_pixbufs + 1) * sizeof (GdkPixbuf *));
		button_grid->number_of_value_pixbufs = value + 1;
	}

	button_grid->value_pixbufs[value] = pixbuf;

	for (row_number = 0; row_number < button_grid->number_of_rows; row_number++)
	{
		for (column_number = 0; column_number < button_grid->number_of_columns; column_number++)
		{
			if (button_grid->button_values[BUTTON_GRID_GET_BUTTON_NUMBER(button_grid, column_number, row_number)] == value)
			{
				gtk_image_set_from_pixbuf(button_grid->button_images[BUTTON_GRID_GET_BUTTON_NUMBER(button_grid, column_number, row_number)], pixbuf);
			}
		}
	}
}

void button_grid_set_dimensions(ButtonGrid *button_grid, guint number_of_columns, guint number_of_rows)
{
	button_grid->number_of_columns = number_of_columns;
	button_grid->number_of_rows = number_of_rows;
	button_grid_do_layout(button_grid);
}

guint button_grid_get_number_of_columns(ButtonGrid *button_grid)
{
	return button_grid->number_of_columns;
}

guint button_grid_get_number_of_rows(ButtonGrid *button_grid)
{
	return button_grid->number_of_rows;
}

void button_grid_set_button_value(ButtonGrid *button_grid, guint column_number, guint row_number, guint value)
{
	guint button_number = BUTTON_GRID_GET_BUTTON_NUMBER(button_grid, column_number, row_number);
	button_grid->button_values[button_number] = value;
	g_atomic_int_set(&(button_grid->button_value_has_changed[button_number]), 1);
	g_atomic_int_set(&(button_grid->some_button_value_has_changed), 1);
}

guint button_grid_get_button_value(ButtonGrid *button_grid, guint column_number, guint row_number)
{
	return button_grid->button_values[BUTTON_GRID_GET_BUTTON_NUMBER(button_grid, column_number, row_number)];
}

