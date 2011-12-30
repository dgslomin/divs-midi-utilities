#ifndef BUTTON_GRID_INCLUDED
#define BUTTON_GRID_INCLUDED

G_BEGIN_DECLS

typedef struct _ButtonGridClass ButtonGridClass;
typedef struct _ButtonGrid ButtonGrid;

struct _ButtonGridClass
{
	GtkTableClass parent_class;
	/* private */
	guint button_pressed_signal;
};

struct _ButtonGrid
{
	GtkTable parent;
	/* private */
	guint number_of_value_pixbufs;
	GdkPixbuf **value_pixbufs;
	guint number_of_columns;
	guint number_of_rows;
	guint *button_values;
	GtkImage **button_images;
	gint *button_value_has_changed;
	gint some_button_value_has_changed;
};

GType button_grid_get_type(void);
ButtonGrid *button_grid_new(void);
void button_grid_set_value_pixbuf(ButtonGrid *button_grid, int value, GdkPixbuf *pixbuf);
void button_grid_set_dimensions(ButtonGrid *button_grid, guint number_of_columns, guint number_of_rows);
guint button_grid_get_number_of_columns(ButtonGrid *button_grid);
guint button_grid_get_number_of_rows(ButtonGrid *button_grid);
void button_grid_set_button_value(ButtonGrid *button_grid, guint column_number, guint row_number, guint value);
guint button_grid_get_button_value(ButtonGrid *button_grid, guint column_number, guint row_number);

#define BUTTON_GRID_TYPE (button_grid_get_type())
#define BUTTON_GRID(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), BUTTON_GRID_TYPE, ButtonGrid))
#define BUTTON_GRID_CLASS(class_) (G_TYPE_CHECK_CLASS_CAST((class_), BUTTON_GRID_TYPE, ButtonGridClass))
#define IS_BUTTON_GRID(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), BUTTON_GRID_TYPE))
#define IS_BUTTON_GRID_CLASS(class_) (G_TYPE_CHECK_CLASS_TYPE((class_), BUTTON_GRID_TYPE))

G_END_DECLS

#endif
