//Using SDL, SDL_image, standard IO, vectors, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <iostream>

using namespace std;

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;


//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile( string path );

		#ifdef _SDL_TTF_H
		//Creates image from font string
		bool loadFromRenderedText( string textureText, SDL_Color textColor );
		#endif

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor( Uint8 red, Uint8 green, Uint8 blue );

		//Set blending
		void setBlendMode( SDL_BlendMode blending );

		//Set alpha modulation
		void setAlpha( Uint8 alpha );

		//Renders texture at given point
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

		//Gets image dimensions
		int getWidth();
		int getHeight();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
};

class LWindow
{
	public:
		//Initializes internals
		LWindow();

		//Creates window
		bool init();

		//Creates renderer from internal window
		SDL_Renderer* createRenderer();

		//Handles window events
		void handleEvent( SDL_Event& e );

		//Deallocates internals
		void free();

		//Window dimensions
		int getWidth();
		int getHeight();

		//Window focii
		bool hasMouseFocus();
		bool hasKeyboardFocus();
		bool isMinimized();

	private:
		//Window data
		SDL_Window* mWindow;

		//Window Dimensions
		int mWidth;
		int mHeight;

		//Window focus
		bool mMouseFocus;
		bool mKeyboardFocus;
		bool mFullScreen;
		bool mMinimized;
};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Our custom window
LWindow gWindow;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Scene textures
LTexture gSceneTexture;

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile( string path )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		cout << "Unable to load image %s! SDL_image Error: %s\n" <<  path.c_str() << IMG_GetError() << endl;
	}
	else
	{
		//Color key image
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			cout << "Unable to create texture from %s! SDL Error: %s\n" << path.c_str() << SDL_GetError() << endl;
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText( string textureText, SDL_Color textColor )
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
	if( textSurface != NULL )
	{
		//Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
		if( mTexture == NULL )
		{
			cout << "Unable to create texture from rendered text! SDL Error: %s\n" << SDL_GetError() << endl;
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface( textSurface );
	}
	else
	{
		cout << "Unable to render text surface! SDL_ttf Error: %s\n" << TTF_GetError() << endl;
	}


	//Return success
	return mTexture != NULL;
}
#endif

void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
	//Modulate texture rgb
	SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
	//Set blending function
	SDL_SetTextureBlendMode( mTexture, blending );
}

void LTexture::setAlpha( Uint8 alpha )
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

LWindow::LWindow()
{
	//Initialize non-existant window
	mWindow = NULL;
	mMouseFocus = false;
	mKeyboardFocus = false;
	mFullScreen = false;
	mMinimized = false;
	mWidth = 0;
	mHeight = 0;
}

bool LWindow::init()
{
	//Create window
	mWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );
	if( mWindow != NULL )
	{
		mMouseFocus = true;
		mKeyboardFocus = true;
		mWidth = SCREEN_WIDTH;
		mHeight = SCREEN_HEIGHT;
	}

	return mWindow != NULL;
}

SDL_Renderer* LWindow::createRenderer()
{
	return SDL_CreateRenderer( mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
}

void LWindow::handleEvent( SDL_Event& e )
{
	//Window event occured
	if( e.type == SDL_WINDOWEVENT )
	{
		//Caption update flag
		bool updateCaption = false;

		switch( e.window.event )
		{

			//Get new dimensions and repaint on window size change
			case SDL_WINDOWEVENT_SIZE_CHANGED:
			mWidth = e.window.data1;
			mHeight = e.window.data2;
			SDL_RenderPresent( gRenderer );
			break;

			//Repaint on exposure
			case SDL_WINDOWEVENT_EXPOSED:
			SDL_RenderPresent( gRenderer );
			break;

			//Mouse entered window
			case SDL_WINDOWEVENT_ENTER:
			mMouseFocus = true;
			updateCaption = true;
			break;

			//Mouse left window
			case SDL_WINDOWEVENT_LEAVE:
			mMouseFocus = false;
			updateCaption = true;
			break;

			//Window has keyboard focus
			case SDL_WINDOWEVENT_FOCUS_GAINED:
			mKeyboardFocus = true;
			updateCaption = true;
			break;

			//Window lost keyboard focus
			case SDL_WINDOWEVENT_FOCUS_LOST:
			mKeyboardFocus = false;
			updateCaption = true;
			break;

			//Window minimized
			case SDL_WINDOWEVENT_MINIMIZED:
			mMinimized = true;
			break;

			//Window maximized
			case SDL_WINDOWEVENT_MAXIMIZED:
			mMinimized = false;
			break;

			//Window restored
			case SDL_WINDOWEVENT_RESTORED:
			mMinimized = false;
			break;
		}

		//Update window caption
		if( updateCaption )
		{
			stringstream caption;
			caption << "SDL Tutorial - MouseFocus: " << ( ( mMouseFocus ) ? "On" : "Off" ) << " KeyboardFocus:" << ( ( mKeyboardFocus ) ? "On" : "Off" );
			SDL_SetWindowTitle( mWindow, caption.str().c_str() );
		}
	}

	//Enter exit full screen on return key
	else if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN )
	{
		if( mFullScreen )
		{
			SDL_SetWindowFullscreen( mWindow, SDL_FALSE );
			mFullScreen = false;
		}
		else
		{
			SDL_SetWindowFullscreen( mWindow, SDL_TRUE );
			mFullScreen = true;
			mMinimized = false;
		}
	}
}

void LWindow::free()
{
	if( mWindow != NULL )
	{
		SDL_DestroyWindow( mWindow );
	}

	mMouseFocus = false;
	mKeyboardFocus = false;
	mWidth = 0;
	mHeight = 0;
}

int LWindow::getWidth()
{
	return mWidth;
}

int LWindow::getHeight()
{
	return mHeight;
}

bool LWindow::hasKeyboardFocus()
{
	return mKeyboardFocus;
}

bool LWindow::isMinimized()
{
	return mMinimized;
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		cout << "SDL could not initialize! SDL Error: %s\n" << SDL_GetError() << endl;;
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			cout << "Warning: Linear texture filtering not enabled!" << endl;
		}

		//Create window
		if( !gWindow.init() )
		{
			cout << "Window could not be created! SDL Error: %s\n" << SDL_GetError() << endl;
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = gWindow.createRenderer();
			if( gRenderer == NULL )
			{
				cout << "Renderer could not be created! SDL Error: %s\n" << SDL_GetError() << endl;;
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					cout << "SDL_image could not initialize! SDL_image Error: %s\n" << IMG_GetError() << endl;
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//load scene texture
	if( !gSceneTexture.loadFromFile( "window.png" ) )
	{
		cout << "Failed to load window texture!\n" << endl;
		success = false;
	}

	return success;
}

void close()
{
	//Free loaded images
	gSceneTexture.free();

	//Destroy window
	SDL_DestroyRenderer( gRenderer );
	gWindow.free();

	//Quit SDL subsystems
	IMG_Quit();
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
		//Load media
		if( !loadMedia() )
		{
			cout << "Failed to load media!\n" << endl;
		}
		else
		{
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//While application is running
			while( !quit )
			{
				//Handle events on queue
				while( SDL_PollEvent( &e ) != 0 )
				{
					//User requests quit
					if( e.type == SDL_QUIT )
					{
						quit = true;
					}

					//Handle window events
					gWindow.handleEvent( e );
				}

				//Only draw whennot minimized
				if( !gWindow.isMinimized() )
				{

					//Clear screen
					SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

					SDL_RenderClear( gRenderer );

					//Render text textures
					gSceneTexture.render ( ( gWindow.getWidth() - gSceneTexture.getWidth() ) / 2, ( gWindow.getHeight() - gSceneTexture.getHeight() ) / 2 );


					//Update screen
					SDL_RenderPresent( gRenderer );
				}
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}
