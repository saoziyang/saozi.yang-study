#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>


#define SCREEN_WIDTH   640 
#define SCREEN_HEIGHT  480 
#define SCREEN_BPP  32

SDL_Surface *screen = NULL;
SDL_Surface *background = NULL;
SDL_Surface *message = NULL;


SDL_Surface* load_image(char* filename)
{
    SDL_Surface* loadImage = NULL; 
    SDL_Surface* optimizedImage = NULL;

    loadImage = SDL_LoadBMP(filename);
    if (loadImage == NULL) {
        printf("SDL_LoadBMP ERROR");
        return NULL;
    } else {
        optimizedImage = SDL_DisplayFormat(loadImage);
        SDL_FreeSurface(loadImage);
    }

    return optimizedImage;
}

void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination )
{
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;

    SDL_BlitSurface(source, NULL, destination, &offset);
    return;
}

//int main(int argc, char* argv[])
int main()
{
        int ret = 0;

    ret = SDL_Init(SDL_INIT_EVERYTHING);
    if (ret < 0) {
        printf("SDL INIT ERROR\n");
        return 1;
    }

    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
    if (screen == NULL) {
        printf("SDL_SetVideoMode ERROR");
        return 1;
    }

    SDL_WM_SetCaption("background", NULL);

    message = load_image("hello.bmp");
    if (message == NULL) {
        printf("load_image error\n");
        return 1;
    }
    background = load_image("hello.bmp");
    if (background == NULL) {
        printf("load_image error\n");
        return 1;
    }

    //hello = SDL_LoadBMP("hello.bmp");

    //SDL_BlitSurface(hello, NULL, screen, NULL);
    //SDL_Flip(screen);
    apply_surface( 0, 0, background, screen );
    apply_surface( 147, 0, background, screen ); 
    apply_surface( 0, 105, background, screen ); 
    apply_surface( 147, 105, background, screen );
    apply_surface( 70, 50, message, screen ); 

    SDL_Flip(screen);

    SDL_Delay(2000);

    //SDL_FreeSurface(hello);
    SDL_FreeSurface(message); 
    SDL_FreeSurface(background);

    SDL_Quit(); 

    return 0;
}
