
#include <windows.h>
#include <simple-collections.h>
#include <layout-containers.h>

typedef struct hbox_layout_container_data *HBoxLayoutContainerData_t;
typedef struct hbox_layout_container_child_data *HBoxLayoutContainerChildData_t;
typedef struct vbox_layout_container_data *VBoxLayoutContainerData_t;
typedef struct vbox_layout_container_child_data *VBoxLayoutContainerChildData_t;
typedef struct table_layout_container_data *TableLayoutContainerData_t;
typedef struct table_layout_container_row_data *TableLayoutContainerRowData_t;
typedef struct table_layout_container_cell_data *TableLayoutContainerCellData_t;

static LRESULT CALLBACK HBoxLayoutContainerWindowProc(HWND window_handle, UINT message_id, WPARAM wparam, LPARAM lparam);
static LRESULT CALLBACK VBoxLayoutContainerWindowProc(HWND window_handle, UINT message_id, WPARAM wparam, LPARAM lparam);
static LRESULT CALLBACK TableLayoutContainerWindowProc(HWND window_handle, UINT message_id, WPARAM wparam, LPARAM lparam);
static TableLayoutContainerCellData_t GetTableLayoutContainerCellData(TableLayoutContainerData_t container_data, int column_number, int row_number);

static struct hbox_layout_container_data
{
	PointerArray_t children;
};

static struct hbox_layout_container_child_data
{
	HWND child;
	int initial_width;
	int initial_height;
	int expand_weight;
	int sticky_sides;
	int padding_top;
	int padding_right;
	int padding_bottom;
	int padding_left;
};

static struct vbox_layout_container_data
{
	PointerArray_t children;
};

static struct vbox_layout_container_child_data
{
	HWND child;
	int initial_width;
	int initial_height;
	int expand_weight;
	int sticky_sides;
	int padding_top;
	int padding_right;
	int padding_bottom;
	int padding_left;
};

static struct table_layout_container_data
{
	PointerArray_t rows;
	int number_of_columns;
	int number_of_rows;
	int caret_column_number;
	int caret_row_number;
};

static struct table_layout_container_row_data
{
	PointerArray_t cells;
};

static struct table_layout_container_cell_data
{
	HWND child;
	int initial_width;
	int initial_height;
	int column_span;
	int row_span;
	int column_expand_weight;
	int row_expand_weight;
	int sticky_sides;
	int padding_top;
	int padding_right;
	int padding_bottom;
	int padding_left;
};

void InitLayoutContainers(void)
{
	{
		WNDCLASSEX window_class;
		window_class.cbSize = sizeof (WNDCLASSEX);
		window_class.style = CS_HREDRAW | CS_VREDRAW;
		window_class.lpfnWndProc = HBoxLayoutContainerWindowProc;
		window_class.cbClsExtra = 0;
		window_class.cbWndExtra = 0;
		window_class.hInstance = GetModuleHandle(NULL);
		window_class.hIcon = NULL;
		window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
		window_class.hbrBackground = NULL;
		window_class.lpszMenuName =  NULL;
		window_class.lpszClassName = HBOX_LAYOUT_CONTAINER;
		window_class.hIconSm = NULL;
		RegisterClassEx(&window_class);
	}

	{
		WNDCLASSEX window_class;
		window_class.cbSize = sizeof (WNDCLASSEX);
		window_class.style = CS_HREDRAW | CS_VREDRAW;
		window_class.lpfnWndProc = VBoxLayoutContainerWindowProc;
		window_class.cbClsExtra = 0;
		window_class.cbWndExtra = 0;
		window_class.hInstance = GetModuleHandle(NULL);
		window_class.hIcon = NULL;
		window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
		window_class.hbrBackground = NULL;
		window_class.lpszMenuName =  NULL;
		window_class.lpszClassName = VBOX_LAYOUT_CONTAINER;
		window_class.hIconSm = NULL;
		RegisterClassEx(&window_class);
	}

	{
		WNDCLASSEX window_class;
		window_class.cbSize = sizeof (WNDCLASSEX);
		window_class.style = CS_HREDRAW | CS_VREDRAW;
		window_class.lpfnWndProc = TableLayoutContainerWindowProc;
		window_class.cbClsExtra = 0;
		window_class.cbWndExtra = 0;
		window_class.hInstance = GetModuleHandle(NULL);
		window_class.hIcon = NULL;
		window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
		window_class.hbrBackground = NULL;
		window_class.lpszMenuName =  NULL;
		window_class.lpszClassName = TABLE_LAYOUT_CONTAINER;
		window_class.hIconSm = NULL;
		RegisterClassEx(&window_class);
	}
}

static LRESULT CALLBACK HBoxLayoutContainerWindowProc(HWND window_handle, UINT message_id, WPARAM wparam, LPARAM lparam)
{
	switch (message_id)
	{
		case WM_CREATE:
		{
			HBoxLayoutContainerData_t container_data = (HBoxLayoutContainerData_t)(malloc(sizeof (struct hbox_layout_container_data)));
			container_data->children = PointerArray_new(16);
			SetWindowLongPtr(window_handle, GWLP_USERDATA, (long)(container_data));
			break;
		}
		case WM_DESTROY:
		{
			HBoxLayoutContainerData_t container_data = (HBoxLayoutContainerData_t)(GetWindowLongPtr(container, GWLP_USERDATA));
			int child_number;

			for (child_number = PointerArray_getSize(container_data->children) - 1; child_number >= 0; child_number--)
			{
				free(PointerArray_get(container_data->children, child_number));
			}

			PointerArray_free(container_data->children);
			free(container_data);
			break;
		}
		case WM_SIZE:
		{
			HBoxLayoutContainerData_t container_data = (HBoxLayoutContainerData_t)(GetWindowLongPtr(container, GWLP_USERDATA));
			int child_number;
			int number_of_children = PointerArray_getSize(container_data->children);
			RECT client_rect;
			int available_width;
			int available_height;
			int total_expand_weight = 0;
			int total_fixed_width = 0;
			int cell_x;
			int cell_width;
			int x;
			int y;
			int width
			int height

			GetClientRect(window_handle, &client_rect);
			available_width = client_rect.right - client_rect.left;
			available_height = client_rect.bottom - client_rect.top;

			for (child_number = 0; child_number < number_of_children; child_number++)
			{
				HBoxLayoutContainerChildData_t child_data = (HBoxLayoutContainerChildData_t)(PointerArray_get(container_data->children, child_number));
				total_expand_weight += child_data->expand_weight;
				total_fixed_width += child_data->padding_left + ((child_data->expand_weight == 0) ? child_data->initial_width : 0) + child_data->padding_right;
			}

			cell_x = client_rect.left;

			for (child_number = 0; child_number < number_of_children; child_number++)
			{
				HBoxLayoutContainerChildData_t child_data = (HBoxLayoutContainerChildData_t)(PointerArray_get(container_data->children, child_number));

				if (child_data->expand_width == 0)
				{
					cell_width = child_data->padding_left + child_data->initial_width + child_data->padding_right;
					width = child_data->initial_width;
					x = child_data->padding_left;
				}
				else if (total_fixed_width < available_width)
				{
					cell_width = child_data->padding_left + ((available_width - total_fixed_width) * child_data->expand_weight / total_expand_weight) + child_data->padding_right;

					if ((child_data->sticky_sides & LAYOUT_CONTAINER_HORIZONTAL) == LAYOUT_CONTAINER_HORIZONTAL)
					{
						width = cell_width - (child_data->padding_left + child_data->padding_right);
						x = child_data->padding_left;
					}
					else if ((child_data->sticky_sides & LAYOUT_CONTAINER_HORIZONTAL) == LAYOUT_CONTAINER_LEFT)
					{
						width = child_data->initial_width; 
						x = child_data->padding_left;
					}
					else if ((child_data->sticky_sides & LAYOUT_CONTAINER_HORIZONTAL) == LAYOUT_CONTAINER_RIGHT)
					{
						width = child_data->initial_width; 
						x = cell_width - (child_data->initial_width + child_data->padding_right);
					}
					else
					{
						width = child_data->initial_width; 
						x = cell_width - ((child_data->padding_left + child_data->initial_width + child_data->padding_right) / 2);
					}
				}
				else
				{
					cell_width = child_data->padding_left + child_data->padding_right;
					width = 0;
					x = child_data->padding_left;
				}

				if ((child_data->sticky_sides & LAYOUT_CONTAINER_VERTICAL) == LAYOUT_CONTAINER_VERTICAL)
				{
					height = available_height - (child_data->padding_top + child_data->padding_bottom);
					y = child_data->padding_top;
				}
				else if ((child_data->sticky_sides & LAYOUT_CONTAINER_VERTICAL) == LAYOUT_CONTAINER_TOP)
				{
					height = child_data->initial_height; 
					y = child_data->padding_top;
				}
				else if ((child_data->sticky_sides & LAYOUT_CONTAINER_VERTICAL) == LAYOUT_CONTAINER_BOTTOM)
				{
					height = child_data->initial_height; 
					y = available_height - (child_data->initial_height + child_data->padding_bottom);
				}
				else
				{
					height = child_data->initial_height; 
					y = available_height - ((child_data->padding_top + child_data->initial_height + child_data->padding_bottom) / 2);
				}

				if (child_data != NULL) MoveWindow(child_data->window, cell_x + x, client_rect.top + y, width, height, TRUE);
				cell_x += cell_width;
			}

			break;
		}
		default:
		{
			break;
		}
	}

	return DefWindowProc(window_handle, message_id, wparam, lparam);
}

void AddHBoxLayoutContainerChild(HWND container, HWND child)
{
	HBoxLayoutContainerData_t container_data = (HBoxLayoutContainerData_t)(GetWindowLongPtr(container, GWLP_USERDATA));
	HBoxLayoutContainerChildData_t child_data = (HBoxLayoutContainerChildData_t)(malloc(sizeof (struct hbox_layout_container_child_data)));
	child_data->child = child;

	if (child != NULL)
	{
		RECT rect;

		GetWindowRect(child, &rect);
		child_data->initial_width = rect.right - rect.left;
		child_data->initial_height = rect.bottom - rect.top;
	}

	child_data->expand_weight = 0;
	child_data->sticky_sides = 0;
	child_data->padding_top = 0;
	child_data->padding_right = 0;
	child_data->padding_bottom = 0;
	child_data->padding_left = 0;
	PointerArray_add(container_data->children, child_data);
}

void SetHBoxLayoutContainerChildExpand(HWND container, int weight)
{
	HBoxLayoutContainerData_t container_data = (HBoxLayoutContainerData_t)(GetWindowLongPtr(container, GWLP_USERDATA));
	HBoxLayoutContainerChildData_t child_data = (HBoxLayoutContainerChildData_t)(PointerArray_get(container_data->children, PointerArray_getSize(container_data->children) - 1));
	child_data->expand_weight = weight;
}

void SetHBoxLayoutContainerChildSticky(HWND container, int sides)
{
	HBoxLayoutContainerData_t container_data = (HBoxLayoutContainerData_t)(GetWindowLongPtr(container, GWLP_USERDATA));
	HBoxLayoutContainerChildData_t child_data = (HBoxLayoutContainerChildData_t)(PointerArray_get(container_data->children, PointerArray_getSize(container_data->children) - 1));
	child_data->sticky_sides = sides;
}

void SetHBoxLayoutContainerChildPadding(HWND container, int top, int right, int bottom, int left)
{
	HBoxLayoutContainerData_t container_data = (HBoxLayoutContainerData_t)(GetWindowLongPtr(container, GWLP_USERDATA));
	HBoxLayoutContainerChildData_t child_data = (HBoxLayoutContainerChildData_t)(PointerArray_get(container_data->children, PointerArray_getSize(container_data->children) - 1));
	child_data->padding_top = top;
	child_data->padding_right = right;
	child_data->padding_bottom = bottom;
	child_data->padding_left = left;
}

static LRESULT CALLBACK VBoxLayoutContainerWindowProc(HWND window_handle, UINT message_id, WPARAM wparam, LPARAM lparam)
{
	switch (message_id)
	{
		case WM_CREATE:
		{
			VBoxLayoutContainerData_t container_data = (VBoxLayoutContainerData_t)(malloc(sizeof (struct vbox_layout_container_data)));
			container_data->children = PointerArray_new(16);
			SetWindowLongPtr(window_handle, GWLP_USERDATA, (long)(container_data));
			break;
		}
		case WM_DESTROY:
		{
			VBoxLayoutContainerData_t container_data = (VBoxLayoutContainerData_t)(GetWindowLongPtr(container, GWLP_USERDATA));
			int child_number;

			for (child_number = PointerArray_getSize(container_data->children) - 1; child_number >= 0; child_number--)
			{
				free(PointerArray_get(container_data->children, child_number));
			}

			PointerArray_free(container_data->children);
			free(container_data);
			break;
		}
		case WM_SIZE:
		{
			VBoxLayoutContainerData_t container_data = (VBoxLayoutContainerData_t)(GetWindowLongPtr(container, GWLP_USERDATA));
			RECT client_rect;
			GetClientRect(window_handle, &client_rect);
			break;
		}
		default:
		{
			break;
		}
	}

	return DefWindowProc(window_handle, message_id, wparam, lparam);
}

void AddVBoxLayoutContainerChild(HWND container, HWND child)
{
	VBoxLayoutContainerData_t container_data = (VBoxLayoutContainerData_t)(GetWindowLongPtr(container, GWLP_USERDATA));
	VBoxLayoutContainerChildData_t child_data = (VBoxLayoutContainerChildData_t)(malloc(sizeof (struct vbox_layout_container_child_data)));
	child_data->child = child;

	if (child != NULL)
	{
		RECT rect;

		GetWindowRect(child, &rect);
		child_data->initial_width = rect.right - rect.left;
		child_data->initial_height = rect.bottom - rect.top;
	}

	child_data->expand_weight = 0;
	child_data->sticky_sides = 0;
	child_data->padding_top = 0;
	child_data->padding_right = 0;
	child_data->padding_bottom = 0;
	child_data->padding_left = 0;
	PointerArray_add(container_data->children, child_data);
}

void SetVBoxLayoutContainerChildExpand(HWND container, int weight)
{
	VBoxLayoutContainerData_t container_data = (VBoxLayoutContainerData_t)(GetWindowLongPtr(container, GWLP_USERDATA));
	VBoxLayoutContainerChildData_t child_data = (VBoxLayoutContainerChildData_t)(PointerArray_get(container_data->children, PointerArray_getSize(container_data->children) - 1));
	child_data->expand_weight = weight;
}

void SetVBoxLayoutContainerChildSticky(HWND container, int sides)
{
	VBoxLayoutContainerData_t container_data = (VBoxLayoutContainerData_t)(GetWindowLongPtr(container, GWLP_USERDATA));
	VBoxLayoutContainerChildData_t child_data = (VBoxLayoutContainerChildData_t)(PointerArray_get(container_data->children, PointerArray_getSize(container_data->children) - 1));
	child_data->sticky_sides = 0;
}

void SetVBoxLayoutContainerChildPadding(HWND container, int top, int right, int bottom, int left)
{
	VBoxLayoutContainerData_t container_data = (VBoxLayoutContainerData_t)(GetWindowLongPtr(container, GWLP_USERDATA));
	VBoxLayoutContainerChildData_t child_data = (VBoxLayoutContainerChildData_t)(PointerArray_get(container_data->children, PointerArray_getSize(container_data->children) - 1));
	child_data->padding_top = 0;
	child_data->padding_right = 0;
	child_data->padding_bottom = 0;
	child_data->padding_left = 0;
}

static LRESULT CALLBACK TableLayoutContainerWindowProc(HWND window_handle, UINT message_id, WPARAM wparam, LPARAM lparam)
{
	switch (message_id)
	{
		case WM_CREATE:
		{
			TableLayoutContainerData_t container_data = (TableLayoutContainerData_t)(malloc(sizeof (struct table_layout_container_data)));
			container_data->rows = PointerArray_new(16);
			container_data->number_of_columns = 0;
			container_data->number_of_rows = 0;
			container_data->caret_column_number = 0;
			container_data->caret_row_number = -1;
			SetWindowLongPtr(window_handle, GWLP_USERDATA, (long)(container_data));
			break;
		}
		case WM_DESTROY:
		{
			TableLayoutContainerData_t container_data = (TableLayoutContainerData_t)(GetWindowLongPtr(container, GWLP_USERDATA));
			int row_number;

			for (row_number = PointerArray_getSize(container_data->rows) - 1; row_number >= 0; row_number--)
			{
				TableLayoutContainerRowData_t row_data = (TableLayoutContainerRowData_t)(PointerArray_get(container_data->rows, row_number));
				int cell_number;

				for (cell_number = PointerArray_getSize(row_data->cells) - 1; cell_number >= 0; cell_number--)
				{
					free(PointerArray_get(row_data->cells, cell_number));
				}

				free(row_data);
			}

			PointerArray_free(container_data->rows);
			free(container_data);
			break;
		}
		case WM_SIZE:
		{
			TableLayoutContainerData_t container_data = (TableLayoutContainerData_t)(GetWindowLongPtr(container, GWLP_USERDATA));
			RECT client_rect;
			GetClientRect(window_handle, &client_rect);
			break;
		}
		default:
		{
			break;
		}
	}

	return DefWindowProc(window_handle, message_id, wparam, lparam);
}

void AddTableLayoutContainerRow(HWND container)
{
	TableLayoutContainerData_t container_data = (TableLayoutContainerData_t)(GetWindowLongPtr(container, GWLP_USERDATA));
	container_data->caret_column_number = 0;
	(container_data->caret_row_number)++;
}

void AddTableLayoutContainerChild(HWND container, HWND child, int column_span, int row_span)
{
	TableLayoutContainerData_t container_data = (TableLayoutContainerData_t)(GetWindowLongPtr(container, GWLP_USERDATA));
	int column_offset;
	int row_offset;

	if (container_data->caret_row_number < 0) AddTableLayoutContainerRow(container);
	if (column_span < 1) column_span = 1;
	if (row_span < 1) row_span = 1;

	while (1)
	{
		int collision = 0;

		for (row_offset = 0; row_offset < row_span; row_offset++)
		{
			for (column_offset = 0; column_offset < column_span; column_offset++)
			{
				TableLayoutContainerCellData_t cell_data = GetTableLayoutContainerCellData(container_data, container_data->caret_column_number + column_offset, container_data->caret_row_number + row_offset);

				if (cell_data->column_span > 0)
				{
					collision = 1;
					break;
				}
			}

			if (collision) break;
		}

		if (!collision) break;
		(container_data->caret_column_number)++;
	}

	{
		TableLayoutContainerCellData_t cell_data = GetTableLayoutContainerCellData(container_data, container_data->caret_column_number, container_data->caret_row_number);
		cell_data->child = child;

		if (child != NULL)
		{
			RECT rect;

			GetWindowRect(child, &rect);
			cell_data->initial_width = rect.right - rect.left;
			cell_data->initial_height = rect.bottom - rect.top;
		}
	}

	for (row_offset = 0; row_offset < row_span; row_offset++)
	{
		for (column_offset = 0; column_offset < column_span; column_offset++)
		{
			TableLayoutContainerCellData_t cell_data = GetTableLayoutContainerCellData(container_data, container_data->caret_column_number + column_offset, container_data->caret_row_number + row_offset);
			cell_data->column_span = column_span - column_offset;
			cell_data->row_span = row_span - row_offset;
		}
	}
}

void SetTableLayoutContainerChildExpand(HWND container, int column_weight, int row_weight)
{
	TableLayoutContainerData_t container_data = (TableLayoutContainerData_t)(GetWindowLongPtr(container, GWLP_USERDATA));
	TableLayoutContainerCellData_t cell_data = GetTableLayoutContainerCellData(container_data, container_data->caret_column_number, container_data->caret_row_number);
	cell_data->column_expand_weight = column_weight;
	cell_data->row_expand_weight = row_weight;
}

void SetTableLayoutContainerChildSticky(HWND container, int sides)
{
	TableLayoutContainerData_t container_data = (TableLayoutContainerData_t)(GetWindowLongPtr(container, GWLP_USERDATA));
	TableLayoutContainerCellData_t cell_data = GetTableLayoutContainerCellData(container_data, container_data->caret_column_number, container_data->caret_row_number);
	cell_data->sticky_sides = sides;
}

void SetTableLayoutContainerChildPadding(HWND container, int top, int right, int bottom, int left)
{
	TableLayoutContainerData_t container_data = (TableLayoutContainerData_t)(GetWindowLongPtr(container, GWLP_USERDATA));
	TableLayoutContainerCellData_t cell_data = GetTableLayoutContainerCellData(container_data, container_data->caret_column_number, container_data->caret_row_number);
	cell_data->padding_top = top;
	cell_data->padding_right = right;
	cell_data->padding_bottom = bottom;
	cell_data->padding_left = left;
}

static TableLayoutContainerCellData_t GetTableLayoutContainerCellData(TableLayoutContainerData_t container_data, int column_number, int row_number)
{
	int other_row_number;
	int other_column_number;

	if (container_data->number_of_columns <= column_number)
	{
		for (other_row_number = 0; other_row_number < container_data->number_of_rows; other_row_number++)
		{
			TableLayoutContainerRowData_t row_data = (TableLayoutContainerRowData_t)(PointerArray_get(container_data->rows, other_row_number));

			for (other_column_number = container_data->number_of_columns; other_column_number <= column_number; other_column_number++)
			{
				TableLayoutContainerCellData_t cell_data = (TableLayoutContainerCellData_t)(malloc(sizeof (struct table_layout_container_cell_data)));
				cell_data->child = NULL;
				cell_data->initial_width = 0;
				cell_data->initial_height = 0;
				cell_data->column_span = 0;
				cell_data->row_span = 0;
				cell_data->column_expand_weight = 0;
				cell_data->row_expand_weight = 0;
				cell_data->sticky_sides = 0;
				cell_data->padding_top = 0;
				cell_data->padding_right = 0;
				cell_data->padding_bottom = 0;
				cell_data->padding_left = 0;
				PointerArray_add(row_data->cells, cell_data);
			}
		}

		container_data->number_of_columns = column_number + 1;
	}

	if (container_data->number_of_rows <= row_number)
	{
		for (other_row_number = container_data->number_of_rows; other_row_number <= row_number; other_row_number++)
		{
			TableLayoutContainerRowData_t row_data = (TableLayoutContainerRowData_t)(malloc(sizeof (struct table_layout_container_row_data)));
			row_data->cells = PointerArray_new(16);
			PointerArray_add(container_data->rows, row_data);

			for (other_column_number = 0; other_column_number < container_data->number_of_columns; other_column_number++)
			{
				TableLayoutContainerCellData_t cell_data = (TableLayoutContainerCellData_t)(malloc(sizeof (struct table_layout_container_cell_data)));
				cell_data->child = NULL;
				cell_data->initial_width = 0;
				cell_data->initial_height = 0;
				cell_data->column_span = 0;
				cell_data->row_span = 0;
				cell_data->column_expand_weight = 0;
				cell_data->row_expand_weight = 0;
				cell_data->sticky_sides = 0;
				cell_data->padding_top = 0;
				cell_data->padding_right = 0;
				cell_data->padding_bottom = 0;
				cell_data->padding_left = 0;
				PointerArray_add(row_data->cells, cell_data);
			}
		}

		container_data->number_of_rows = row_number + 1;
	}

	return (TableLayoutContainerCellData_t)(PointerArray_get((TableLayoutContainerRowData_t)(PointerArray_get(container_data->rows, row_number)), column_number));
}

