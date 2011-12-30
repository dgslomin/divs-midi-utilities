#ifndef SEQER_INCLUDED
#define SEQER_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct SeqerApplication *SeqerApplication_t;
typedef struct SeqerWindow *SeqerWindow_t;

typedef enum
{
	SEQER_WINDOW_TYPE_PIANO_ROLL,
	SEQER_WINDOW_TYPE_EVENT_LIST
}
SeqerWindowType_t;

typedef enum
{
	SEQER_PIANO_ROLL_MODE_SINGLE,
	SEQER_PIANO_ROLL_MODE_OVERLAID,
	SEQER_PIANO_ROLL_MODE_STACKED
}
SeqerPianoRollMode_t;

#define SEQER_COLOR_SELECTED_FOREGROUND 0x1000000
#define SEQER_COLOR_SELECTED_BACKGROUND 0x1000001

SeqerApplication_t SeqerApplication_new(void);
void SeqerApplication_free(SeqerApplication_t application);
int SeqerApplication_getNumberOfWindows(SeqerApplication_t application);
SeqerWindow_t SeqerApplication_getWindow(SeqerApplication_t application, int window_number);
void SeqerApplication_setPreferredMidiFileDivisionType(SeqerApplication_t application, MidiFileDivisionType_t preferred_midi_file_division_type);
MidiFileDivisionType_t SeqerApplication_getPreferredMidiFileDivisionType(SeqerApplication_t application);
void SeqerApplication_setPreferredMidiFileResolution(SeqerApplication_t application, int preferred_midi_file_resolution);
int SeqerApplication_getPreferredMidiFileResolution(SeqerApplication_t application);
void SeqerApplication_setPreferredWindowType(SeqerApplication_t application, SeqerWindowType_t preferred_window_type);
SeqerWindowType_t SeqerApplication_getPreferredWindowType(SeqerApplication_t application);
void SeqerApplication_setPreferredPianoRollMode(SeqerApplication_t application, SeqerPianoRollMode_t preferred_piano_roll_mode);
SeqerPianoRollMode_t SeqerApplication_getPreferredPianoRollMode(SeqerApplication_t application);
void SeqerApplication_setDrawLineCallback(SeqerApplication_t application, void (*draw_line_callback)(SeqerWindow_t window, int x1, int y1, int x2, int y2, int color));
void SeqerApplication_setDrawRectangleCallback(SeqerApplication_t application, void (*draw_rectangle_callback)(SeqerWindow_t window, int x, int y, int width, int height, int color, int fill_color));
void SeqerApplication_setDrawCircleCallback(SeqerApplication_t application, void (*draw_circle_callback)(SeqerWindow_t window, int x, int y, int diameter, int color, int fill_color));
void SeqerApplication_setMeasureStringCallback(SeqerApplication_t application, void (*measure_string_callback)(SeqerWindow_t window, char *string, int *width_out, int *height_out));
void SeqerApplication_setDrawStringCallback(SeqerApplication_t application, void (*draw_string_callback)(SeqerWindow_t window, int x, int y, char *string, int color));
void SeqerApplication_setFilenameChangeCallback(SeqerApplication_t application, void (*filename_change_callback)(SeqerWindow_t window));
void SeqerApplication_setRedrawCallback(SeqerApplication_t application, void (*redraw_callback)(SeqerWindow_t window));
void SeqerApplication_setConfigureHorizontalScrollbarCallback(SeqerApplication_t application, void (*configure_horizontal_scrollbar_callback)(SeqerWindow_t window, int max, int line, int page, int current));
void SeqerApplication_setConfigureVerticalScrollbarCallback(SeqerApplication_t application, void (*configure_vertical_scrollbar_callback)(SeqerWindow_t window, int max, int line, int page, int current));
void SeqerApplication_setUserData(SeqerApplication_t application, void *user_data);
void *SeqerApplication_getUserData(SeqerApplication_t application);

SeqerWindow_t SeqerWindow_new(SeqerApplication_t application, SeqerWindow_t base_window);
void SeqerWindow_free(SeqerWindow_t window);
SeqerApplication_t SeqerWindow_getApplication(SeqerWindow_t window);
int SeqerWindow_shouldPromptForSave(SeqerWindow_t window);
int SeqerWindow_shouldPromptForFilename(SeqerWindow_t window);
void SeqerWindow_newMidiFile(SeqerWindow_t window);
int SeqerWindow_loadMidiFile(SeqerWindow_t window, char *filename);
int SeqerWindow_saveMidiFile(SeqerWindow_t window, char *filename);
char *SeqerWindow_getFilename(SeqerWindow_t window);
int SeqerWindow_getModified(SeqerWindow_t window);
MidiFile_t SeqerWindow_getMidiFile(SeqerWindow_t window);
void SeqerWindow_setSize(SeqerWindow_t window, int width, int height);
int SeqerWindow_getWidth(SeqerWindow_t window);
int SeqerWindow_getHeight(SeqerWindow_t window);
void SeqerWindow_scrollHorizontal(SeqerWindow_t window, int position);
void SeqerWindow_scrollVertical(SeqerWindow_t window, int position);
void SeqerWindow_paint(SeqerWindow_t window);
void SeqerWindow_mouseDown(SeqerWindow_t window, int button_number, int x, int y);
void SeqerWindow_mouseUp(SeqerWindow_t window, int button_number, int x, int y);
void SeqerWindow_mouseMove(SeqerWindow_t window, int x, int y);
void SeqerWindow_setUserData(SeqerWindow_t window, void *user_data);
void *SeqerWindow_getUserData(SeqerWindow_t window);
SeqerWindowType_t SeqerWindow_getType(SeqerWindow_t window);
void SeqerWindow_setType(SeqerWindow_t window, SeqerWindowType_t type);

void SeqerPianoRollWindow_setMode(SeqerWindow_t window, SeqerPianoRollMode_t mode);
SeqerPianoRollMode_t SeqerPianoRollWindow_getMode(SeqerWindow_t window);

#ifdef __cplusplus
}
#endif

#endif
