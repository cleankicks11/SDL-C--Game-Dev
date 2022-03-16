#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string>
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

		//Loads image specific path
		bool loadFromFile( string path );

		#ifdef _SDL_TTF_H
		//Create image from font string
		bool loadFromRenderedText( string textureText, SDL_Color textColor );
		#endif

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor( Uint8 red, Uint8 green, Uint8 blue );

		//set blending
		void setBlendMode( SDL_BlendMode blending );

		//Set alpha modulation
		void setAlpha( Uint8 alpha );

		//Render texture at given point
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

		//Gets image dimensions
		int getWidth();
		int getHeight();

		//Pixel Manipulators
		bool lockTexture();
		bool unlockTexture();
		void* getPixels();
		int getPitch();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;
		void* mPixels;
		int mPitch;

		//Image dimensions
		int mWidth;
		int mHeight;
};

//Starts SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Scene texture
LTexture gFooTexture;

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
	mPixels = NULL;
	mPitch = 0;
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
		cout << "Unable to load image %s! SDL_image Error: %s\n" << path.c_str() << IMG_GetError() << endl;
	}
	else
	{
		//Convert surface to display format
		SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat ( loadedSurface, SDL_GetWindowPixelFormat( gWindow ), NULL );
		if( formattedSurface == NULL )
		{
			cout << "Unable to convert loaded surface to display format! SDL Error: %s\n" << SDL_GetError() << endl;
		}
		else
		{

			//Create blank streamable texture
			newTexture = SDL_CreateTexture( gRenderer, SDL_GetWindowPixelFormat( gWindow ), SDL_TEXTUREACCESS_STREAMING, formattedSurface->w, formattedSurface->h );
			if( newTexture == NULL )
			{
				cout << "Unable to create blank texture! SDL Error: %s\n" << SDL_GetError() << endl;
			}
			else
			{

				//Lock texture for manipulation
				SDL_LockTexture( newTexture, NULL, &mPixels, &mPitch );

			//Copy loaded/formatted surface pixels
			memcpy( mPixels, formattedSurface->pixels, formattedSurface->pitch * formattedSurface->h );

			//Unlock texture to update
			SDL_UnlockTexture( newTexture );
			mPixels = NULL;

			//Get image dimensions
			mWidth = formattedSurface->w;
			mHeight = formattedSurface->h;
		}
		//Get rid of old loaded surface
		SDL_FreeSurface( formattedSurface );
	}

	//Get rid of old loaded surface
	SDL_FreeSurface( loadedSurface );

	}

	//Return Success
	mTexture = newTexture;
	return mTexture != NULL;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText( string textureText, SDL_Color textColor)
{
	//Get rid of  preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
	if( textSurface != NULL )
	{
		//Create texture from surface pixels
		mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
		if( mTexture == NULL )
		{
			cout << "Unable to create texture from rendered text! SDL Error: %s\n" << SDL_GetError << endl;
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
		mPixels = NULL;
		mPitch = 0;
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
	//modulate texture alpha
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

bool LTexture::lockTexture()
{
	bool success = true;

	//Texture is already locked
	if( mPixels != NULL )
	{
		cout << "Texture is already locked!\n" << endl;
		success = false;
	}
	//Lock texture
	else
	{
		if( SDL_LockTexture( mTexture, NULL, &mPixels, &mPitch ) != 0 )
		{
			cout << "Unable to lock texture! %s\n" << SDL_GetError() << endl;
			success = false;
		}
	}

	return success;
}

bool LTexture::unlockTexture()
{
	bool success = true;

	//Texture is not locked
	if( mPixels == NULL )
	{
		cout << "Texture is not locked!\n" << endl;
		success = false;
	}
	//Unlock texture
	else
	{
		SDL_UnlockTexture( mTexture );
		mPixels = NULL;
		mPitch = 0;
	}

	return success;
}

void* LTexture::getPixels()
{
	return mPixels;
}

int LTexture::getPitch()
{
	return mPitch;
}

bool init()
{
	//Initializatio flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		cout << "SDL could not initialize! SDL Error: %s\n" << SDL_GetError() << endl;
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			cout << "Warning: linear texture filtering not enabled!" << endl;
		}

		//Create window
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			cout << "Window could not be created! SDL Error: %s\n" << SDL_GetError << endl;
			success = false;
		}
		else
		{
			//Create render for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			if( gRenderer == NULL )
			{
				cout << "Renderer could not be created! SDL Error: %s\n" << SDL_GetError() << endl;
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
					cout << "SDL_image could not be initialized! SDL_image Error: %s\n" << IMG_GetError() << endl;
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loadingsucces flag
	bool success = true;

	//load foo texture
	if( !gFooTexture.loadFromFile( "foo.png" ) )
	{
		cout << "Failed to load corner texture!\n" << endl;
		success = false;
	}
	else
	{
		//Lock texture
		if( !gFooTexture.lockTexture() )
		{
			cout << "Unable to lock Foo' texture!\n" << endl;
		}
		//Manual color key
		else
		{
			//Allocate format from window
			Uint32 format = SDL_GetWindowPixelFormat( gWindow );
			SDL_PixelFormat* mappingFormat = SDL_AllocFormat ( format );

			//Get pixel data
			Uint32* pixels = (Uint32*)gFooTexture.getPixels();
			int pixelCount = ( gFooTexture.getPitch() / 4 ) * gFooTexture.getHeight();

			//Map colors
			Uint32 colorKey = SDL_MapRGB( mappingFormat, 0, 0xFF, 0xFF );
			Uint32 transparent = SDL_MapRGBA( mappingFormat, 0xFF, 0xFF, 0xFF, 0x00 );

			//Color key pixels
			for( int i = 0; i < pixelCount; ++i )
			{
				if( pixels[ i ] == colorKey )
				{
					pixels[ i ] = transparent;
				}
			}

			//Unlock texture
			gFooTexture.unlockTexture();

			//Free format
			SDL_FreeFormat( mappingFormat );

		}
	}

	return success;
}

void close()
{
	//Free loaded images
	gFooTexture.free();

	//Destroy window
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quir SDL subsystems
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
		//load media
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
				while( SDL_PollEvent( &e ) != 0)
				{
					//User request squit
					if( e.type == SDL_QUIT )
					{
						quit = true;
					}

				}

				//Clear screen
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
				SDL_RenderClear( gRenderer );

				//Redner stick figure
				gFooTexture.render( ( SCREEN_WIDTH - gFooTexture.getWidth() ) / 2, ( SCREEN_HEIGHT - gFooTexture.getHeight() ) / 2 );

				//Update screen
				SDL_RenderPresent( gRenderer );
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;

}

