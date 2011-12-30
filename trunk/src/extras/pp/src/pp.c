
#include <windows.h>
#include <SDL.h>
#include <SDL_framerate.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <midifile.h>

#define ANCHOR_TOP_ROW 0x0
#define ANCHOR_CENTER_ROW 0x1
#define ANCHOR_BOTTOM_ROW 0x2
#define ANCHOR_LEFT_COLUMN 0x0
#define ANCHOR_CENTER_COLUMN 0x4
#define ANCHOR_RIGHT_COLUMN 0x8

typedef enum
{
	ANCHOR_NW = (ANCHOR_TOP_ROW | ANCHOR_LEFT_COLUMN),
	ANCHOR_N = (ANCHOR_TOP_ROW | ANCHOR_CENTER_COLUMN),
	ANCHOR_NE = (ANCHOR_TOP_ROW | ANCHOR_RIGHT_COLUMN),
	ANCHOR_W = (ANCHOR_CENTER_ROW | ANCHOR_LEFT_COLUMN),
	ANCHOR_CENTER = (ANCHOR_CENTER_ROW | ANCHOR_CENTER_COLUMN),
	ANCHOR_E = (ANCHOR_CENTER_ROW | ANCHOR_RIGHT_COLUMN),
	ANCHOR_SW = (ANCHOR_BOTTOM_ROW | ANCHOR_LEFT_COLUMN),
	ANCHOR_S = (ANCHOR_BOTTOM_ROW | ANCHOR_CENTER_COLUMN),
	ANCHOR_SE = (ANCHOR_BOTTOM_ROW | ANCHOR_RIGHT_COLUMN)
}
ANCHOR;

static void blit(SDL_Surface *src, SDL_Surface *dst, Sint16 x, Sint16 y, ANCHOR anchor)
{
	SDL_Rect dstrect;

	if (anchor & ANCHOR_RIGHT_COLUMN)
	{
		dstrect.x = x - src->w;
	}
	else if (anchor & ANCHOR_CENTER_COLUMN)
	{
		dstrect.x = x - (src->w / 2);
	}
	else
	{
		dstrect.x = x;
	}

	if (anchor & ANCHOR_BOTTOM_ROW)
	{
		dstrect.y = y - src->h;
	}
	else if (anchor & ANCHOR_CENTER_ROW)
	{
		dstrect.y = y - (src->h / 2);
	}
	else
	{
		dstrect.y = y;
	}

	SDL_BlitSurface(src, NULL, dst, &dstrect);
}

static void clear(SDL_Surface *dst)
{
	SDL_FillRect(dst, NULL, 0xFF000000);
}

static SDL_Surface *render_text(TTF_Font *font, const char *text, Uint8 r, Uint8 g, Uint8 b)
{
	SDL_Color color;
	color.r = r;
	color.g = g;
	color.b = b;
	return TTF_RenderText_Blended(font, text, color);
}

static void render_text_onto_surface(TTF_Font *font, const char *text, Uint8 r, Uint8 g, Uint8 b, SDL_Surface *dst, Sint16 x, Sint16 y, ANCHOR anchor)
{
	SDL_Surface *text_surface = render_text(font, text, r, g, b);
	blit(text_surface, dst, x, y, anchor);
	SDL_FreeSurface(text_surface);
}

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR command_line, int show)
{
	MidiFile_t midi_file;
	FPSmanager framerate_manager;
	TTF_Font *font;
	SDL_Surface *screen;
	int should_shutdown = 0;
	float lookahead = 5.0;

	int right_pressed = 0;
	int left_pressed = 0;
	int up_pressed = 0;
	int down_pressed = 0;
	int pgdn_pressed = 0;
	int pgup_pressed = 0;

	midi_file = MidiFile_load("song.mid");

	if (midi_file == NULL)
	{
		MessageBox(NULL, "Cannot load MIDI file.", "Piano Protagonist", MB_OK | MB_ICONERROR);
		exit(1);
	}

	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_initFramerate(&framerate_manager);
	SDL_setFramerate(&framerate_manager, 30);

	TTF_Init();
	font = TTF_OpenFont("c:/windows/fonts/benguiab.ttf", 24);

	if (font == NULL)
	{
		MessageBox(NULL, "Cannot open font.", "Piano Protagonist", MB_OK | MB_ICONERROR);
		exit(1);
	}

	screen = SDL_SetVideoMode(1024, 768, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
	SDL_ShowCursor(SDL_DISABLE);

	while (!should_shutdown)
	{
		SDL_Event event;
		Uint32 current_time;

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
				{
					switch (event.key.keysym.sym)
					{
						case SDLK_RIGHT:
						{
							right_pressed = 1;
							break;
						}
						case SDLK_LEFT:
						{
							left_pressed = 1;
							break;
						}
						case SDLK_UP:
						{
							up_pressed = 1;
							break;
						}
						case SDLK_DOWN:
						{
							down_pressed = 1;
							break;
						}
						case SDLK_PAGEDOWN:
						{
							pgdn_pressed = 1;
							break;
						}
						case SDLK_PAGEUP:
						{
							pgup_pressed = 1;
							break;
						}
					}

					break;
				}
				case SDL_KEYUP:
				{
					switch (event.key.keysym.sym)
					{
						case SDLK_RIGHT:
						{
							right_pressed = 0;
							break;
						}
						case SDLK_LEFT:
						{
							left_pressed = 0;
							break;
						}
						case SDLK_UP:
						{
							up_pressed = 0;
							break;
						}
						case SDLK_DOWN:
						{
							down_pressed = 0;
							break;
						}
						case SDLK_PAGEDOWN:
						{
							pgdn_pressed = 0;
							break;
						}
						case SDLK_PAGEUP:
						{
							pgup_pressed = 0;
							break;
						}
						case SDLK_ESCAPE:
						{
							should_shutdown = 1;
							break;
						}
						case SDLK_F4:
						{
							if (event.key.keysym.mod & KMOD_ALT) should_shutdown = 1;
							break;
						}
						case SDLK_c:
						{
							if (event.key.keysym.mod & KMOD_CTRL) should_shutdown = 1;
							break;
						}
						default:
						{
							break;
						}
					}

					break;
				}
				case SDL_QUIT:
				{
					should_shutdown = 1;
					break;
				}
				default:
				{
					break;
				}
			}
		}

		if (up_pressed) lookahead *= 1.1;
		if (down_pressed) lookahead /= 1.1;

		clear(screen);

		{
			Sint16 note;

			for (note = 0; note < 128; note++)
			{
				int piano_palette[] = { 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1 };

				if (piano_palette[note % 12])
				{
					boxRGBA(screen, (Sint16)((8 * note) + 1), 0, (Sint16)((8 * (note + 1)) - 1), 767, 31, 31, 31, 255);
				}
			}
		}

		{
			float now = (float)(SDL_GetTicks()) / 1000.0;
			MidiFileEvent_t midi_file_event;

			for (midi_file_event = MidiFile_getFirstEvent(midi_file); !should_shutdown && (midi_file_event != NULL); midi_file_event = MidiFileEvent_getNextEventInFile(midi_file_event))
			{
				if (MidiFileEvent_isNoteStartEvent(midi_file_event))
				{
					float midi_file_event_time = MidiFile_getTimeFromTick(midi_file, MidiFileEvent_getTick(midi_file_event));

					if ((midi_file_event_time >= now) && (midi_file_event_time < (now + lookahead)))
					{
						int midi_file_event_note = MidiFileNoteStartEvent_getNote(midi_file_event);
						int midi_file_event_track_number = MidiFileTrack_getNumber(MidiFileEvent_getTrack(midi_file_event));
						Sint16 x = (8 * midi_file_event_note) + 4;
						Sint16 y = 768 - (Sint16)(768.0 * ((midi_file_event_time - now) / lookahead));
						int palette[] = { 4, 6, 2, 3, 1 };
						Uint8 r = (palette[midi_file_event_track_number % 5] & 4) * 255;
						Uint8 g = (palette[midi_file_event_track_number % 5] & 2) * 255;
						Uint8 b = (palette[midi_file_event_track_number % 5] & 1) * 255;
						filledCircleRGBA(screen, x, y, 3, r, g, b, 255);
					}
				}
			}
		}

		render_text_onto_surface(font, "Piano Protagonist", 0, 0, 0, screen, 1005, 9, ANCHOR_NE);
		render_text_onto_surface(font, "Piano Protagonist", 255, 255, 210, screen, 1004, 10, ANCHOR_NE);

		SDL_Flip(screen);
		SDL_framerateDelay(&framerate_manager);
	}

	SDL_FreeSurface(screen);
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
	MidiFile_free(midi_file);
	return 0;
}

