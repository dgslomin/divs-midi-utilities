
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR command_line, int show)
{
	SDL_Surface *surface;
	int should_shutdown = 0;
	int getting_brighter = 1;
	GLfloat brightness = 0.0;

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	surface = SDL_SetVideoMode(1024, 768, 32, SDL_OPENGL | SDL_FULLSCREEN);
	SDL_ShowCursor(SDL_DISABLE);

	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

	while (!should_shutdown)
	{
		SDL_Event event;

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYUP:
				{
					if ((event.key.keysym.sym == SDLK_ESCAPE) || ((event.key.keysym.sym == SDLK_F4) && ((event.key.keysym.mod & KMOD_ALT) != 0)))
					{
						should_shutdown = 1;
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

		if (getting_brighter)
		{
			brightness += 0.01;

			if (brightness > 1.0)
			{
				brightness = 1.0;
				getting_brighter = 0;
			}
		}
		else
		{
			brightness -= 0.01;

			if (brightness < 0.0)
			{
				brightness = 0.0;
				getting_brighter = 1;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);
		glColor3f(brightness, brightness, 0.5);

		glBegin(GL_POLYGON);
		glVertex3f(brightness, brightness, 0.0);
		glVertex3f(1.0 - brightness, brightness, 0.0);
		glVertex3f(1.0 - brightness, 1.0 - brightness, 0.0);
		glVertex3f(brightness, 1.0 - brightness, 0.0);
		glEnd();

		glFlush();
		SDL_GL_SwapBuffers();
	}

	SDL_FreeSurface(surface);
	SDL_Quit();
	return 0;
}

