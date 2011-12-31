#ifndef LAYOUT_CONTAINERS_INCLUDED
#define LAYOUT_CONTAINERS_INCLUDED

/*

User interface layout management for programs based on the raw Windows API.
No frameworks necessary!

Notes:

1.  Call InitLayoutContainers() at the start of your program, just like
InitCommonControls().

2.  Pass HBOX_LAYOUT_CONTAINER, VBOX_LAYOUT_CONTAINER, or
TABLE_LAYOUT_CONTAINER as the class name argument to CreateWindowEx() to create
the corresponding type of container.  You can then add conventional controls or
other containers as children.

3.  The size of a child when it is added to a container will be treated as the
child's natural size.

4.  Functions which set a container's child-specific properties apply to the
most recently added child.

5.  AddTableLayoutContainerChild() automatically takes into account column and
row spanning from previous children, so as to prevent collisions.

Planned, but not yet implemented:

1.  A choice of overflowing, clipping, or scrolling when the children don't fit
in the container.

2.  Control over minimum size and maximum size.

3.  Maybe support for a wrapping flow layout and an equal-sized grid layout?

*/

#ifdef __cplusplus
extern "C"
{
#endif

#define HBOX_LAYOUT_CONTAINER "HBoxLayoutContainer"
#define VBOX_LAYOUT_CONTAINER "VBoxLayoutContainer"
#define TABLE_LAYOUT_CONTAINER "TableLayoutContainer"

#define LAYOUT_CONTAINER_TOP 0x00000001
#define LAYOUT_CONTAINER_RIGHT 0x00000002
#define LAYOUT_CONTAINER_BOTTOM 0x00000004
#define LAYOUT_CONTAINER_LEFT 0x00000008
#define LAYOUT_CONTAINER_VERTICAL (LAYOUT_CONTAINER_TOP | LAYOUT_CONTAINER_BOTTOM)
#define LAYOUT_CONTAINER_HORIZONTAL (LAYOUT_CONTAINER_LEFT | LAYOUT_CONTAINER_RIGHT)
#define LAYOUT_CONTAINER_ALL (LAYOUT_CONTAINER_VERTICAL | LAYOUT_CONTAINER_HORIZONTAL)

void InitLayoutContainers(void);

void AddHBoxLayoutContainerChild(HWND container, HWND child);
void SetHBoxLayoutContainerChildExpand(HWND container, int weight);
void SetHBoxLayoutContainerChildSticky(HWND container, int sides);
void SetHBoxLayoutContainerChildPadding(HWND container, int top, int right, int bottom, int left);

void AddVBoxLayoutContainerChild(HWND container, HWND child);
void SetVBoxLayoutContainerChildExpand(HWND container, int weight);
void SetVBoxLayoutContainerChildSticky(HWND container, int sides);
void SetVBoxLayoutContainerChildPadding(HWND container, int top, int right, int bottom, int left);

void AddTableLayoutContainerRow(HWND container);
void AddTableLayoutContainerChild(HWND container, HWND child, int column_span, int row_span);
void SetTableLayoutContainerChildExpand(HWND container, int column_weight, int row_weight);
void SetTableLayoutContainerChildSticky(HWND container, int sides);
void SetTableLayoutContainerChildPadding(HWND container, int top, int right, int bottom, int left);

#ifdef __cplusplus
}
#endif

#endif
