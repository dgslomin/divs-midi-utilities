
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <gtk/gtk.h>
#include <midifile.h>

typedef struct
{
	MidiFile_t midi_file;
	GtkWindow *window;
	GtkVBox *vbox;
	GtkMenuBar *menu_bar;
	GtkMenuItem *file_menu_item;
	GtkMenu *file_menu;
	GtkMenuItem *exit_menu_item;
	GtkScrolledWindow *scrolled_window;
	GtkTextView *text_view;
	GtkStatusbar *statusbar;
}
Application;

static gchar *_midi_note_number_to_string(int note_number)
{
	return g_strdup_printf("%s%d", "C-\0C#\0D-\0D#\0E-\0F-\0F#\0G-\0G#\0A-\0A#\0B-\0" + ((note_number % 12) * 3), note_number / 12);
}

static gboolean _text_view_click_handler(GtkWidget *widget, GdkEvent *event)
{
	GtkTextView *text_view = GTK_TEXT_VIEW(widget);

	if (event->type == GDK_BUTTON_RELEASE)
	{
		GdkEventButton *button_event = GDK_EVENT_BUTTON(event);

		if (button_event->button == 1)
		{
			GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(text_view);
			gint x;
			gint y;
			GtkTextIter text_iter;

			gtk_text_view_window_to_buffer_coords(text_view, GTK_TEXT_WINDOW_WIDGET, button_event->x, button_event->y, &x, &y);
			gtk_text_view_get_iter_at_location(text_view, &text_iter, x, y);

			{
				GSList *text_tag_list = gtk_text_iter_get_tags(&text_iter);
				GSList *text_tag_list_item;

				for (text_tag_list_item = text_tag_list; text_tag_list_item != NULL; text_tag_list_item = text_tag_list_item->next)
				{
					GtkTextTag *text_tag = GTK_TEXT_TAG(text_tag_list_item->data);

					if (g_object_get_data(G_OBJECT(text_tag), "column_type") != NULL)
					{
						gtk_text_buffer_place_cursor(text_buffer);
						break;
					}
				}

				if (text_tag_list != NULL) g_slist_free(text_tag_list);
			}

			return TRUE;
		}
	}

	return FALSE;
}

int main(int argc, char **argv)
{
	Application application;

	g_thread_init(NULL);
	gtk_init(&argc, &argv);

	application.midi_file = MidiFile_load(argv[1]);

	application.window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
	gtk_window_set_default_size(application.window, 640, 480);
	gtk_window_set_title(application.window, "Sequencer");
	g_signal_connect(G_OBJECT(application.window), "destroy", G_CALLBACK(exit), NULL);

	application.vbox = GTK_VBOX(gtk_vbox_new(FALSE, 0));
	gtk_container_add(GTK_CONTAINER(application.window), GTK_WIDGET(application.vbox));

	application.menu_bar = GTK_MENU_BAR(gtk_menu_bar_new());
	gtk_box_pack_start(GTK_BOX(application.vbox), GTK_WIDGET(application.menu_bar), FALSE, FALSE, 0);

	application.file_menu_item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic("_File"));
	gtk_menu_shell_append(GTK_MENU_SHELL(application.menu_bar), GTK_WIDGET(application.file_menu_item));

	application.file_menu = GTK_MENU(gtk_menu_new());
	gtk_menu_item_set_submenu(application.file_menu_item, GTK_WIDGET(application.file_menu));

	application.exit_menu_item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic("E_xit"));
	gtk_menu_shell_append(GTK_MENU_SHELL(application.file_menu), GTK_WIDGET(application.exit_menu_item));

	application.scrolled_window = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));
	gtk_box_pack_start(GTK_BOX(application.vbox), GTK_WIDGET(application.scrolled_window), TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy(application.scrolled_window, GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

	application.text_view = GTK_TEXT_VIEW(gtk_text_view_new());
	gtk_container_add(GTK_CONTAINER(application.scrolled_window), GTK_WIDGET(application.text_view));
	gtk_text_view_set_editable(application.text_view, FALSE);

	{
		PangoContext *pango_context;
		PangoLayout *pango_layout;
		PangoTabArray *pango_tab_array;
		int column_width;

		pango_context = gtk_widget_get_pango_context(GTK_WIDGET(application.text_view));
		pango_layout = pango_layout_new(pango_context);
		pango_layout_set_text(pango_layout, "WWWWWWWW", -1);
		pango_layout_get_size(pango_layout, &column_width, NULL);
		pango_tab_array = pango_tab_array_new_with_positions(6, FALSE, PANGO_TAB_LEFT, 0, PANGO_TAB_LEFT, column_width, PANGO_TAB_LEFT, column_width * 2, PANGO_TAB_LEFT, column_width * 3, PANGO_TAB_LEFT, column_width * 4, PANGO_TAB_LEFT, column_width * 5);
		gtk_text_view_set_tabs(application.text_view, pango_tab_array);
		pango_tab_array_free(pango_tab_array);
	}

	{
		GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(application.text_view);
		GtkTextIter text_iter;
		float previous_displayed_beat = 0.0;
		MidiFileEvent_t midi_file_event;

		gtk_text_buffer_get_iter_at_offset(text_buffer, &text_iter, 0);

		for (midi_file_event = MidiFile_getFirstEvent(application.midi_file); midi_file_event != NULL; midi_file_event = MidiFileEvent_getNextEventInFile(midi_file_event))
		{
			if (MidiFileEvent_isNoteStartEvent(midi_file_event) && (MidiFileTrack_getNumber(MidiFileEvent_getTrack(midi_file_event)) == 1))
			{
				float beat = MidiFile_getBeatFromTick(application.midi_file, MidiFileEvent_getTick(midi_file_event));

				while (previous_displayed_beat <= beat)
				{
					gchar *text = g_strdup_printf("%.3f\t\t\t\t\t\n", previous_displayed_beat);
					gtk_text_buffer_insert(text_buffer, &text_iter, text, -1);
					g_free(text);
					previous_displayed_beat += 0.25;
				}

				{
					gchar *text = g_strdup_printf("%.3f\t", beat);
					GtkTextTag *text_tag = gtk_text_buffer_create_tag(text_buffer, NULL, NULL);
					g_object_set_data(G_OBJECT(text_tag), "column_type", "beat");
					g_object_set_data(G_OBJECT(text_tag), "midi_file_event", midi_file_event);
					gtk_text_buffer_insert_with_tags(text_buffer, &text_iter, text, -1, text_tag, NULL);
					g_free(text);
				}

				{
					GtkTextTag *text_tag = gtk_text_buffer_create_tag(text_buffer, NULL, NULL);
					g_object_set_data(G_OBJECT(text_tag), "column_type", "event_type");
					g_object_set_data(G_OBJECT(text_tag), "midi_file_event", midi_file_event);
					gtk_text_buffer_insert_with_tags(text_buffer, &text_iter, "NoteStart\t", -1, text_tag, NULL);
				}

				{
					gchar *text = g_strdup_printf("%d\t", MidiFileTrack_getNumber(MidiFileEvent_getTrack(midi_file_event)));
					GtkTextTag *text_tag = gtk_text_buffer_create_tag(text_buffer, NULL, NULL);
					g_object_set_data(G_OBJECT(text_tag), "column_type", "track");
					g_object_set_data(G_OBJECT(text_tag), "midi_file_event", midi_file_event);
					gtk_text_buffer_insert_with_tags(text_buffer, &text_iter, text, -1, text_tag, NULL);
					g_free(text);
				}

				{
					gchar *text = g_strdup_printf("%d\t", MidiFileNoteStartEvent_getChannel(midi_file_event) + 1);
					GtkTextTag *text_tag = gtk_text_buffer_create_tag(text_buffer, NULL, NULL);
					g_object_set_data(G_OBJECT(text_tag), "column_type", "channel");
					g_object_set_data(G_OBJECT(text_tag), "midi_file_event", midi_file_event);
					gtk_text_buffer_insert_with_tags(text_buffer, &text_iter, text, -1, text_tag, NULL);
					g_free(text);
				}

				{
					gchar *note_string = _midi_note_number_to_string(MidiFileNoteStartEvent_getNote(midi_file_event));
					gchar *text = g_strdup_printf("%s\t", note_string);
					GtkTextTag *text_tag = gtk_text_buffer_create_tag(text_buffer, NULL, NULL);
					g_object_set_data(G_OBJECT(text_tag), "column_type", "note");
					g_object_set_data(G_OBJECT(text_tag), "midi_file_event", midi_file_event);
					gtk_text_buffer_insert_with_tags(text_buffer, &text_iter, text, -1, text_tag, NULL);
					g_free(text);
					g_free(note_string);
				}

				{
					gchar *text = g_strdup_printf("%d\n", MidiFileNoteStartEvent_getVelocity(midi_file_event));
					GtkTextTag *text_tag = gtk_text_buffer_create_tag(text_buffer, NULL, NULL);
					g_object_set_data(G_OBJECT(text_tag), "column_type", "velocity");
					g_object_set_data(G_OBJECT(text_tag), "midi_file_event", midi_file_event);
					gtk_text_buffer_insert_with_tags(text_buffer, &text_iter, text, -1, text_tag, NULL);
					g_free(text);
				}
			}
		}
	}

	g_signal_connect(application.text_view, "event-after", G_CALLBACK(_text_view_click_handler), NULL);

	application.statusbar = GTK_STATUSBAR(gtk_statusbar_new());
	gtk_box_pack_start(GTK_BOX(application.vbox), GTK_WIDGET(application.statusbar), FALSE, FALSE, 0);

	gtk_widget_show_all(GTK_WIDGET(application.window));

	gtk_main();
	return 0;
}

