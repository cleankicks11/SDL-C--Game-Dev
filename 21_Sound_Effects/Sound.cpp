#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
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

		//Loads image at specified path
		bool loadFromFile( string path );

		#ifdef _SDL_TTF_H
		//Creates image from font string
		bool loadFromRenderedText( string textureText, SDL_Color textColor );
		#endif

		//Deallocates texture
		void free();

		//set color modulation
		void setColor( Uint8 red, Uint8 green, Uint8 blue );

		//Set Blending
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

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Scene textures
LTexture gPromptTexture;

//The music that will be played
Mix_Music *gMusic = NULL;

//Sound effects that will be used
Mix_Chunk *gScratch = NULL;
Mix_Chunk *gHigh = NULL;
Mix_Chunk *gMedium = NULL;
Mix_Chunk *gLow = NULL;

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
		cout << "Unable to load image %s! SDL_image Error: %s\n" << path.c_str() << IMG_GetError() << endl;
	}
	else
	{
		//Color key image
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

		//Create texture from surface pixels
	newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			cout << "Unable to create texture from %s! SDL Error %s\n" << path.c_str() << SDL_GetError() << endl;
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
	}
		cout << "Unable to render text Surface! SDL_ttf Error: %s\n" << TTF_GetError() << endl;
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

	//Set clip rending dimensions
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

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
	{
		cout << "SDL could not initialize! SDL Error: %s\n" << SDL_GetError() << endl;
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
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			cout << "Window could not be created! SDL Error %s\n" << SDL_GetError() << endl;
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			if( gRenderer == NULL )
			{
				cout << "Renderer could not Created! SDL_Image Error: %s\n" << SDL_GetError() << endl;
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
					cout << "SDL_Image could not intialize! SDL_image Error: %s\n" << IMG_GetError() << endl;
					success = false;
				}

				//Initialize SDL_mixer
				if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
				{
					cout << "SDL_mixer could not initialize! SDL_mixer Error: %s\n" << Mix_GetError() << endl;
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

	//load prompt texture
	if( !gPromptTexture.loadFromFile( "prompt.png" ) )
	{
		cout << "failed to load prompt texture!\n" << endl;
		success = false;
	}

	//Load music
	gMusic = Mix_LoadMUS( "beat.wav" );
	if( gMusic == NULL )
	{
		cout << "Failed to load beat music! SDL_mixer Error: %s\n" << Mix_GetError() << endl;
		success = false;
	}

	//Load sound effects
	gScratch = Mix_LoadWAV( "scratch.wav" );
	if( gScratch == NULL )
	{
		cout << "Failed to load scratch sound effects! SDL_mixer Error: %s/n" << Mix_GetError() << endl;
		success = false;
	}

	gHigh = Mix_LoadWAV( "high.wav" );
	if( gHigh == NULL )
	{
		cout << "failed to load high sound effect! SDL_mixer Error: %s\n" << Mix_GetError() << endl;
		success = false;
	}

	gMedium = Mix_LoadWAV( "medium.wav" );
	if( gMedium == NULL )
	{
		cout << "Failed to load medium sound effect! SDL_mixer Error: %s\n" << Mix_GetError() << endl;
		success = false;
	}

	gLow = Mix_LoadWAV( "low.wav" );
	if( gLow == NULL )
	{
		cout << "Failed to load low sound effect! SDL_mixer Error: %s/n" << Mix_GetError() << endl;
		success = false;
	}

	return success;
}

void close()
{
	//Free loaded image
	gPromptTexture.free();

	//Free the sound effects
	Mix_FreeChunk( gScratch );
	Mix_FreeChunk( gHigh );
	Mix_FreeChunk( gMedium );
	Mix_FreeChunk( gLow );
	gScratch = NULL;
	gHigh = NULL;
	gMedium = NULL;
	gLow = NULL;

	//Free the music
	Mix_FreeMusic( gMusic );
	gMusic = NULL;

	//Destroy window
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	Mix_Quit();
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
					//Handle key press
					else if( e.type == SDL_KEYDOWN )
					{
						switch( e.key.keysym.sym )
						{
							//Play High sound
							case SDLK_1:
							Mix_PlayChannel( -1, gHigh, 0 );
							break;

							//Play medium sound
							case SDLK_2:
							Mix_PlayChannel( -1, gMedium, 0 );
							break;

							//Play low sound
							case SDLK_3:
							Mix_PlayChannel( -1, gLow, 0 );
							break;

							//Play scratch sound
							case SDLK_4:
							Mix_PlayChannel( -1, gScratch, 0 );
							break;

							case SDLK_9:
							//If there is no music
							if( Mix_PlayingMusic() == 0 )
							{
								//Play the music
								Mix_PlayMusic( gMusic, -1 );
							}
								//If music is being played
							else
							{
								//If the music is paused
							if( Mix_PausedMusic() == 1 )
							{
							//Resume the music
							Mix_ResumeMusic();
							}
							//Music is playing
							else
							{
							//Pause the music
							Mix_PauseMusic();
							}
						}
						break;

						case SDLK_0:
						//Stop the music
						Mix_HaltMusic();
						break;
					}
				}
			}

			//Clear screen
			SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

			SDL_RenderClear( gRenderer );

			//Render prompt
			gPromptTexture.render( 0, 0 );

				//Update screen
				SDL_RenderPresent( gRenderer );
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}
