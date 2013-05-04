#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>


#define SCREEN_WIDTH   640 
#define SCREEN_HEIGHT  480 
#define SCREEN_BPP  32

SDL_Surface *screen = NULL;
SDL_Surface *background = NULL;
SDL_Rect clips[4];
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

void apply_surface( int x, int y, SDL_Surface* source, 
        SDL_Surface* destination, SDL_Rect* clip)
{
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;

    SDL_BlitSurface(source, clip, destination, &offset);
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

void clean_up()
{
    SDL_FreeSurface(background);

    SDL_Quit(); 
}

bool load_files()
{
    unsigned long int colorkey;

    background = load_image("dots.png");
    if (background == NULL) {
        printf("load_image error\n");
        return 0;
    } else {
        colorkey = SDL_MapRGB(background->format, 0x00, 0xFF, 0xFF);
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

    SDL_FillRect(screen, &screen->clip_rect, SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF));

    clips[0].x = 0;
    clips[0].y = 0;
    clips[0].w = 100;
    clips[0].h = 100;

    clips[1].x = 100;
    clips[1].y = 0;
    clips[1].w = 100;
    clips[1].h = 100;

    clips[2].x = 0;
    clips[2].y = 100;
    clips[2].w = 100;
    clips[2].h = 100;

    clips[3].x = 100;
    clips[3].y = 100;
    clips[3].w = 100;
    clips[3].h = 100;

    apply_surface( 0, 0, background, screen, &clips[0] );
    apply_surface( 540, 0, background, screen, &clips[1] );
    apply_surface( 0, 380, background, screen, &clips[2] );
    apply_surface( 540, 380, background, screen, &clips[3] );

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
