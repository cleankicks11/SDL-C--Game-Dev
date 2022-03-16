#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_FPS = 60;

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

		//Creates blank texture
		bool createBlank( int width, int height, SDL_TextureAccess = SDL_TEXTUREACCESS_STREAMING );

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

		//Set self as render target
		void setAsRenderTarget();

		//Gets image dimensions
		int getWidth();
		int getHeight();

		//Pixel Manipulators
		bool lockTexture();
		bool unlockTexture();
		void* getPixels();
		void copyPixels( void* pixels );
		int getPitch();
		Uint32 getPixel32( unsigned int x, unsigned int y );

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

//our worker function
int producer( void* data );
int consumer( void* data );
void produce();
void consume();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Scene texture
LTexture gSplashTexture;

//The protective mutex
SDL_mutex* gBufferLock = NULL;

//The conditions
SDL_cond* gCanProduce = NULL;
SDL_cond* gCanConsume = NULL;

//The 'data buffer'
int gData = -1;

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
		SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat ( loadedSurface, SDL_PIXELFORMAT_RGBA8888, NULL );
		if( formattedSurface == NULL )
		{
			cout << "Unable to convert loaded surface to display format! SDL Error: %s\n" << SDL_GetError() << endl;
		}
		else
		{

			//Create blank streamable texture
			newTexture = SDL_CreateTexture( gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, formattedSurface->w, formattedSurface->h );
			if( newTexture == NULL )
			{
				cout << "Unable to create blank texture! SDL Error: %s\n" << SDL_GetError() << endl;
			}
			else
			{

				//Enable blending on texture
				SDL_SetTextureBlendMode( newTexture, SDL_BLENDMODE_BLEND );

				//Lock texture for manipulation
				SDL_LockTexture( newTexture, &formattedSurface->clip_rect, &mPixels, &mPitch );

				//Copy loaded/formatted surface pixels
				memcpy( mPixels, formattedSurface->pixels, formattedSurface->pitch * formattedSurface->h );

				//Get image dimensions
				mWidth = formattedSurface->w;
				mHeight = formattedSurface->h;

				//Get pixel data in editable format
				Uint32* pixels = (Uint32*)mPixels;
				int pixelCount = ( mPitch / 4 ) * mHeight;

				//Map colors
				Uint32 colorKey = SDL_MapRGB ( formattedSurface->format, 0, 0xFF, 0xFF );
				Uint32 transparent = SDL_MapRGBA ( formattedSurface->format, 0x00, 0xFF, 0xFF, 0x00 );

				//Color key pixels
				for( int i = 0; i < pixelCount; ++i )
				{
					if( pixels[ i ] == colorKey )
					{
						pixels[ i ] = transparent;
					}
				}

			//Unlock texture to update
			SDL_UnlockTexture( newTexture );
			mPixels = NULL;

		}
		//Get rid of old formatted surface
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

bool LTexture::createBlank( int width, int height, SDL_TextureAccess access )
{
	//Create uninitialized texture
	mTexture = SDL_CreateTexture( gRenderer, SDL_PIXELFORMAT_RGBA8888, access, width, height );
	if( mTexture == NULL )
	{
		cout << "Unable to create blank texture! SDL Error: %s\n" << SDL_GetError() << endl;
	}
	else
	{
		mWidth = width;
		mHeight = height;
	}

	return mTexture != NULL;
}

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

void LTexture::setAsRenderTarget()
{
	//Make self render target
	SDL_SetRenderTarget( gRenderer, mTexture );
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

void LTexture::copyPixels( void* pixels )
{
	//Texture is locked
	if( mPixels != NULL )
	{
		//Copy to locked pixels
		memcpy( mPixels, pixels, mPitch * mHeight );
	}
}

int LTexture::getPitch()
{
	return mPitch;
}

Uint32 LTexture::getPixel32( unsigned int x, unsigned int y )
{
	//Convert the pixels to bit
	Uint32 *pixels = (Uint32*)mPixels;

	//Get the pixel requested
	return pixels[ ( y * ( mPitch / 4 ) ) + x ];
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
	//Create the mutex
	gBufferLock = SDL_CreateMutex();

	//Create conditions
	gCanProduce = SDL_CreateCond();
	gCanConsume = SDL_CreateCond();

	//Loading success flag
	bool success = true;

	//load splash texture
	if( !gSplashTexture.loadFromFile( "splash.png" ) )
	{
		cout << "Failed to load splash texture!\n" << endl;
		success = false;
	}


	return success;
}

void close()
{
	//Free loaded images
	gSplashTexture.free();

	//Destroy the mutex
	SDL_DestroyMutex( gBufferLock );
	gBufferLock = NULL;

	//Destroy conditions
	SDL_DestroyCond( gCanProduce );
	SDL_DestroyCond( gCanConsume );
	gCanProduce = NULL;
	gCanConsume = NULL;

	//Destroy window
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quir SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int producer( void* data )
{
	//Print incoming data
	cout << "\nProducer started...\n" << endl;

	//Seed thread random
	srand( SDL_GetTicks() );

	//Produce
	for( int i = 0; i < 5; ++i )
	{
		//Wait
		SDL_Delay( rand() % 1000 );

		//Produce
		produce();
	}

	cout << "\nProducer finished!\n" << endl;

	return 0;

}

int consumer( void *data )
{
	cout << "\nConsumer started...\n" << endl;

	//Seed thread random
	srand( SDL_GetTicks() );

	for( int i = 0; i < 5; ++i )
	{
		//Wait
		SDL_Delay( rand() % 1000 );

		//Consume
		consume();
	}

	cout << "\nConsumer finished!\n" << endl;

	return 0;

}

void produce()
{
	//Lock
	SDL_LockMutex( gBufferLock );

	//If the buffer is full
	if( gData != -1 )
	{
		//Wait for buffer to be cleared
		cout << "\nProducer encountered full buffer, waiting for consumer to empty buffer...\n" << endl;
		SDL_CondWait( gCanProduce, gBufferLock );
	}

	//Fill and show buffer
	gData = rand() % 255;
	cout << "\nProduced %d\n" << gData << endl;

	//Unlock
	SDL_UnlockMutex( gBufferLock );

	//Signal consumer
	SDL_CondSignal( gCanConsume );
}

void consume()
{
	//Lock
	SDL_LockMutex( gBufferLock );

	//If the buffer is empty
	if( gData == -1 )
	{
		//Wait for buffer to be filled
		cout << "\nConsumer encountered empty buffer, waiting for producer to fill buffer...\n" << endl;
		SDL_CondWait( gCanConsume, gBufferLock );
	}

	//Show and empty buffer
	cout << "\nConsumed %d\n" << gData << endl;
	gData = -1;

	//Unlock
	SDL_UnlockMutex( gBufferLock );

	//Signal producer
	SDL_CondSignal( gCanProduce );

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

			//Run the thread
			SDL_Thread* producerThread = SDL_CreateThread( producer, "Producer", NULL );
			SDL_Thread* consumerThread = SDL_CreateThread( consumer, "Consumer", NULL );

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

				//Render splash
				gSplashTexture.render( 0, 0 );

				//Update screen
				SDL_RenderPresent( gRenderer );
			}

			//Wait for threads to finish
			SDL_WaitThread( consumerThread, NULL );
			SDL_WaitThread( producerThread, NULL );
		}
	}

	//Free resources and close SDL
	close();

	return 0;

}

