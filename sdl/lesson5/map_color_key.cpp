#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>


#define SCREEN_WIDTH   640 
#define SCREEN_HEIGHT  480 
#define SCREEN_BPP  32

SDL_Surface *screen = NULL;
SDL_Surface *background = NULL;
SDL_Surface *frontground = NULL;
SDL_Event event;


SDL_Surface* load_image(char* filename)
{
    SDL_Surface* loadImage = NULL; 
    SDL_Surface* optimizedImage = NULL;

    //loadImage = SDL_LoadBMP(filename);
    loadImage = IMG_Load(filename);
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

bool init()
{
    int ret = 0;

    ret = SDL_Init(SDL_INIT_EVERYTHING);
    if (ret < 0) {
        printf("SDL INIT ERROR\n");
        return 0;
    }

    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
    if (screen == NULL) {
        printf("SDL_SetVideoMode ERROR");
        return 0;
    }

    SDL_WM_SetCaption("background", NULL);

    return 1;
}

bool clean_up()
{
    SDL_FreeSurface(frontground);

    SDL_FreeSurface(background);

    SDL_Quit(); 
}

bool load_files()
{
    unsigned long int colorkey;
    frontground = load_image("foo.png");
    if (frontground == NULL) {
        printf("load_image error\n");
        return 0;
    } else {
        colorkey = SDL_MapRGB(frontground->format, 0x00, 0xFF, 0xFF);
        SDL_SetColorKey(frontground, SDL_SRCCOLORKEY, colorkey);
    }

    background = load_image("background.png");
    if (background == NULL) {
        printf("load_image error\n");
        return 0;
    } else {
        //colorkey = SDL_MapRGB(background->format, 0xFF, 0xFF, 0xFF);
        colorkey = SDL_MapRGB(background->format, 0x00, 0x00, 0x00);
        SDL_SetColorKey(background, SDL_SRCCOLORKEY, colorkey);
    }


    return 1;
}

//int main(int argc, char* argv[])
int main()
{
    int ret = 0;
    bool quit = false;

    ret = init();
    if (ret == 0) {
        printf("init error\n");
        return 1;
    }

    ret = load_files(); 
    if (ret == 0) {
        printf("load files error\n");
        return 1;
    }

    apply_surface( 0, 0, background, screen );
    apply_surface( 140, 160, frontground, screen );

    SDL_Flip(screen);

    //SDL_Delay(2000);
    while (quit == false) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
               quit = true; 
            }
        }
    }

    clean_up();
    return 0;
}
