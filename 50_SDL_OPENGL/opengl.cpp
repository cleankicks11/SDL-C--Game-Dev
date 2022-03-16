#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glut.h>
//#include <OpenGL/gl.h>
//#include <OpenGL/glu.h>
//#include <GLUT/glut.h>
#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

////Starts up SDL, creates window, and initializes OpenGL
bool init();

//Initializes matrices and clear color
bool initGL();

//Input handler
void handleKeys( unsigned char key, int x, int y);

//PerFrame update
void update();

//Remder quad to the screen
void render();

//Frees media and shuts down sDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//OpenGL context
SDL_GLContext gContext;

//Render flag
bool gRenderQuad = true;

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		cout << "SDL could not initialize! SDL Error: %s\n" << SDL_GetError() << endl;
		success = false;
	}
	else
	{
		//Use OpenGL 2.1
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );

		//Create window
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			cout << "Window could not be created! SDL Error: %s\n" << SDL_GetError() << endl;
			success = false;
		}
		else
		{
			//Create context
			gContext = SDL_GL_CreateContext( gWindow );
			if( gContext == NULL )
			{
				cout << "OpenGL context could not be created! SDL Error: %s\n" << SDL_GetError() << endl;
				success = false;
			}
			else
			{
				//Use Vsync
				if( SDL_GL_SetSwapInterval( 1 ) < 0 )
				{
					cout << "Warning: Unable to set VSync! SDL Error: %s\n" << SDL_GetError() << endl;
				}

				//Initialize OPENGL
				if( !initGL() )
				{
					cout << "unable to initialize OpenGL!\n" << endl;
					success = false;
				}
			}
		}
	}
	return success;
}

bool initGL()
{
	bool success = true;
	GLenum error = GL_NO_ERROR;

	//Initialize Projection Matrix
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	//Check for error
	error = glGetError();
	if( error != GL_NO_ERROR )
	{
		cout << "Error intializing OpenGL! %s\n" << gluErrorString( error ) << endl;
		success = false;
	}

	//Initialize modelview matrix
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	//Check error
	error = glGetError();
	if ( error != GL_NO_ERROR )
	{
		cout << "Error initializing OpenGL! %s\n" << gluErrorString( error ) << endl;
		success = false;
	}

	//Initialize clear error
	glClearColor( 0.f, 0.f, 0.f, 1.f );

	//check for error
	error = glGetError();
	if( error != GL_NO_ERROR )
	{
		cout << "Error initializing OpenGL! %s\n" << gluErrorString( error ) << endl;
		success = false;
	}

	return success;
}

void handleKeys( unsigned char key, int x, int y )
{
	//Toggle quad
	if( key == 'q' )
	{
		gRenderQuad = !gRenderQuad;
	}
}

void update()
{
	//No per frame update needed
}

void render()
{
	//Clear color buffer
	glClear( GL_COLOR_BUFFER_BIT );

	//Render quad
	if( gRenderQuad )
	{
		glBegin( GL_QUADS );
			glVertex2f( -0.5f, -0.5f );
			glVertex2f( 0.5f, -0.5f );
			glVertex2f( 0.5f, 0.5f );
			glVertex2f( -0.5f, 0.5f );
		glEnd();
	}
}

void close()
{
	//Destroy window
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;

	//Quit SDL subsystmes
	SDL_Quit();
}

int main()
{
	//Start up SDL and create window
	if( !init() )
	{
		cout << "Failed to initialize!\n" << endl;
	}
	else
	{
		//Main loop flag
		bool quit = false;

		//Event handler
		SDL_Event e;

		//Enable text input
		SDL_StartTextInput();

		//While application is running
		while( !quit )
		{
			//Handle events on queue
			while( SDL_PollEvent( &e ) != 0 )
			{
				//user request quit
				if( e.type == SDL_QUIT )
				{
					quit = true;
				}
				//Handle keypress with current mouse position

				else if( e.type == SDL_TEXTINPUT )
				{
					int x = 0, y = 0;
					SDL_GetMouseState( &x, &y );
					handleKeys( e.text.text[ 0 ], x, y );
				}
			}
			//Render quad
			render();

			//Update screen
			SDL_GL_SwapWindow( gWindow );

		}
		//Disable text input
		SDL_StopTextInput();
	}
	//Free resources and close SDL
	close();

	return 0;
}
