
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR command_line, int show)
{
	SDL_Surface *surface;
	int should_shutdown = 0;
	Uint32 refresh_rate = 30;
	Uint32 refresh_interval = 1000 / refresh_rate;
	Uint32 previous_refresh_time = 0;
	int right_pressed = 0;
	int left_pressed = 0;
	int up_pressed = 0;
	int down_pressed = 0;
	int pgdn_pressed = 0;
	int pgup_pressed = 0;
	float initial_x = 0.5;
	float initial_y = 0.5;
	float initial_z = 2.0;
	float x = initial_x;
	float y = initial_y;
	float z = initial_z;
	float delta = 0.01;

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	surface = SDL_SetVideoMode(1024, 768, 32, SDL_OPENGL | SDL_FULLSCREEN);
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
						case SDLK_r:
						{
							x = initial_x;
							y = initial_y;
							z = initial_z;
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

		if (right_pressed) x += delta;
		if (left_pressed) x -= delta;
		if (up_pressed) y += delta;
		if (down_pressed) y -= delta;
		if (pgdn_pressed) z += delta;
		if (pgup_pressed) z -= delta;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(90.0, 1024.0 / 768.0, 0, 1000.0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(x, y, z, 0.5, 0.5, 0.5, 0.0, 1.0, 0.0);

		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);

		/* back face - red */

		glColor3f(1.0, 0.0, 0.0);

		glBegin(GL_POLYGON);
		glVertex3f(1.0, 1.0, 0.0);
		glVertex3f(0.0, 1.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(1.0, 0.0, 0.0);
		glEnd();

		/* front face - green */

		glColor3f(0.0, 1.0, 0.0);

		glBegin(GL_POLYGON);
		glVertex3f(1.0, 1.0, 1.0);
		glVertex3f(0.0, 1.0, 1.0);
		glVertex3f(0.0, 0.0, 1.0);
		glVertex3f(1.0, 0.0, 1.0);
		glEnd();

		/* left face - blue */

		glColor3f(0.0, 0.0, 1.0);

		glBegin(GL_POLYGON);
		glVertex3f(0.0, 1.0, 1.0);
		glVertex3f(0.0, 0.0, 1.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 1.0, 0.0);
		glEnd();

		/* right face - yellow */

		glColor3f(1.0, 1.0, 0.0);

		glBegin(GL_POLYGON);
		glVertex3f(1.0, 1.0, 1.0);
		glVertex3f(1.0, 0.0, 1.0);
		glVertex3f(1.0, 0.0, 0.0);
		glVertex3f(1.0, 1.0, 0.0);
		glEnd();

		/* bottom face - cyan */

		glColor3f(0.0, 1.0, 1.0);

		glBegin(GL_POLYGON);
		glVertex3f(1.0, 0.0, 1.0);
		glVertex3f(0.0, 0.0, 1.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(1.0, 0.0, 0.0);
		glEnd();

		/* top face - magenta */

		glColor3f(1.0, 0.0, 1.0);

		glBegin(GL_POLYGON);
		glVertex3f(1.0, 1.0, 1.0);
		glVertex3f(0.0, 1.0, 1.0);
		glVertex3f(0.0, 1.0, 0.0);
		glVertex3f(1.0, 1.0, 0.0);
		glEnd();

		glFlush();
		SDL_GL_SwapBuffers();

		current_time = SDL_GetTicks();
		if ((current_time - previous_refresh_time) < refresh_interval) SDL_Delay(refresh_interval - (current_time - previous_refresh_time));
		previous_refresh_time = current_time;
	}

	SDL_FreeSurface(surface);
	SDL_Quit();
	return 0;
}

