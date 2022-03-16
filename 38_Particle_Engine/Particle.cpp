#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Particle count
const int TOTAL_PARTICLES = 20;

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

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
};

class Particle
{
	public:
		//Initialize position and animation
		Particle( int x, int y );

		//Shows the particle
		void render();

		//Checks if particle is dead
		bool isDead();

	private:
		//Offsets
		int mPosX, mPosY;

		//Current frame of animation
		int mFrame;

		//Type of particle
		LTexture *mTexture;
};

//The dot that will move
class Dot
{
	public:
		//The dimensions of the dot
		static const int DOT_WIDTH = 20;
		static const int DOT_HEIGHT = 20;

		//Maximum axis velocity of the dot
		static const int DOT_VEL = 10;

		//Initializes the variable and allocates particles
		Dot();

		//Deallocates particles
		~Dot();

		//Takes key presses and adjusts the dots velocity
		void handleEvent( SDL_Event& e );

		//Moves the dot
		void move();

		//Shows the dot on the screen
		void render();

	private:
		//The particles
		Particle* particles[ TOTAL_PARTICLES ];

		//Shows the particles
		void renderParticles();

		//The X and Y offsets of the dot
		int mPosX, mPosY;

		//The velocity of the dot
		int mVelX, mVelY;
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
LTexture gDotTexture;
LTexture gRedTexture;
LTexture gGreenTexture;
LTexture gBlueTexture;
LTexture gShimmerTexture;

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
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB ( loadedSurface->format, 0, 0xFF, 0xFF ) );

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			cout << "Unable to create texture from %s! SDL Error: % s\n" << path.c_str() << SDL_GetError() << endl;
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

Particle::Particle( int x, int y )
{
	//Set offsets
	mPosX = x - 5 + ( rand() % 25 );
	mPosY = y - 5 + ( rand() % 25 );

	//Initialize animation
	mFrame = rand() % 5;

	//SEt type
	switch( rand() % 3 )
	{
		case 0: mTexture = &gRedTexture; break;
		case 1: mTexture = &gGreenTexture; break;
		case 2: mTexture = &gBlueTexture; break;
	}
}

void Particle::render()
{
	//Show image
		mTexture->render( mPosX, mPosY );
	//Show shimmer
	if( mFrame % 2 == 0)
	{
		gShimmerTexture.render( mPosX, mPosY );
	}

	//Animate
	mFrame++;
}

bool Particle::isDead()
{
	return mFrame > 100;
}

Dot::Dot()
{
	//Initialize teh offsets
	mPosX = 0;
	mPosY = 0;

	//Initialize the velocity
	mVelX = 0;
	mVelY = 0;

	//Initialize particles
	for( int i = 0; i < TOTAL_PARTICLES; ++i )
	{
		particles[ i ] = new Particle( mPosX, mPosY );
	}
}

Dot::~Dot()
{
	//Delete particles
	for( int i = 0; i < TOTAL_PARTICLES; ++i )
	{
		delete particles[ i ];
	}
}

void Dot::handleEvent( SDL_Event& e )
{
	//If a ket was pressed
		if( e.type == SDL_KEYDOWN && e.key.repeat == 0 )
	{
		//Adjust the velocity
		switch( e.key.keysym.sym )
		{
			case SDLK_UP: mVelY -= DOT_VEL; break;
			case SDLK_DOWN: mVelY += DOT_VEL; break;
			case SDLK_LEFT: mVelX -= DOT_VEL; break;
			case SDLK_RIGHT: mVelX += DOT_VEL; break;
		}
	}
	//If a key was released
	else if( e.type == SDL_KEYUP && e.key.repeat == 0 )
	{
		//Adjust the velocity
		switch( e.key.keysym.sym )
		{
			case SDLK_UP: mVelY += DOT_VEL; break;
			case SDLK_DOWN: mVelY -= DOT_VEL; break;
			case SDLK_LEFT: mVelX += DOT_VEL; break;
			case SDLK_RIGHT: mVelX -= DOT_VEL; break;
		}
	}
}

void Dot::move()
{
	//move the dot left or right
	mPosX += mVelX;

	//If the dot went too far to the left or right
	if( ( mPosX < 0 ) || (mPosX + DOT_WIDTH > SCREEN_WIDTH ) )
	{
		//Move back
		mPosX -= mVelX;
	}

	//Move the dot up or down
	mPosY += mVelY;

	//If the dot went too far up or down
	if( ( mPosY < 0 ) || (mPosY + DOT_HEIGHT > SCREEN_HEIGHT ) )
	{
		//Move back
		mPosY -= mVelY;
	}
}

void Dot::render()
{
	//Show dot
	gDotTexture.render( mPosX, mPosY );

	//show particles on top of dot
	renderParticles();
}

void Dot::renderParticles()
{
	//Go through paricles
	for( int i = 0; i < TOTAL_PARTICLES; ++i )
	{
		//Delete and replace dead paricles
		if( particles[ i ]->isDead() )
		{
			delete particles[ i ];
			particles[ i ] = new Particle( mPosX, mPosY );
		}
	}

	//show particles
	for( int i = 0; i < TOTAL_PARTICLES; ++i )
	{
		particles[ i ]->render();
	}
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

	//load dot texture
	if( !gDotTexture.loadFromFile( "dot.bmp" ) )
	{
		cout << "Failed to load dot texture!\n" << endl;
		success = false;
	}

	//Load red texture
	if( !gRedTexture.loadFromFile( "red.bmp" ) )
	{
		cout << "failed to load red texture!\n" << endl;
		success = false;
	}

	//Load green texture
	if( !gGreenTexture.loadFromFile( "green.bmp" ) )
	{
		cout << "Failed to load green texture!\n" << endl;
		success = false;
	}

	//Load blue texture
	if( !gBlueTexture.loadFromFile( "blue.bmp" ) )
	{
		cout << "failed to load blue texture!\n" << endl;
		success = false;
	}

	//Load shimmer texture
	if( !gShimmerTexture.loadFromFile( "shimmer.bmp" ) )
	{
		cout << "Failed to load shimmer texture!\n" << endl;
		success = false;
	}

	//Set texture transperancy
	gRedTexture.setAlpha( 192 );
	gGreenTexture.setAlpha( 192 );
	gBlueTexture.setAlpha( 192 );
	gShimmerTexture.setAlpha( 192 );

	return success;
}

void close()
{
	//Free loaded images
	gDotTexture.free();
	gRedTexture.free();
	gGreenTexture.free();
	gBlueTexture.free();
	gShimmerTexture.free();

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
		//Main loop flag
		bool quit = false;

		//Event handler
		SDL_Event e;

		//The dot that will be moving
		Dot dot;

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

				//Handle input for dot
				dot.handleEvent( e );

				//Move the dot
				dot.move();

				//Clear screen
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
				SDL_RenderClear( gRenderer );

				//Redner objects
				dot.render();

				//Update screen
				SDL_RenderPresent( gRenderer );
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;

}

