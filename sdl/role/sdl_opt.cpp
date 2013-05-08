#include "sdl_opt.h"

#define SCREEN_WITH     600
#define SCREEN_HEIGHT   480
#define SCREEN_BPP  32

sdl_opt::sdl_opt()
{
    printf("%s\n", __func__);
    if (init_flag != 1) {
        int ret;
        ret = SDL_Init(SDL_INIT_EVERYTHING);
        if (ret < 0) {
            printf("SDL_Init error\n");
            return;
        }

        screen = SDL_SetVideoMode(SCREEN_WITH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);;

        SDL_WM_SetCaption("KOF97", NULL);
        if (ret < 0) {
            printf("SDL_WM_SetCaption\n");
            return;
        }
        init_flag = 1;
    } 
}

SDL_Surface* load_image(char* file)
{
    SDL_Surface* Image = NULL;
    SDL_Surface* optimizedImage = NULL;

    Image = IMG_Load("./person.png");
    //printf("%s\n", file);
    if (Image == NULL) {
        printf("IMG_Load error\n");
        return NULL;
    } else {
        optimizedImage = SDL_DisplayFormat(Image);
        SDL_FreeSurface(Image);
    }

    return optimizedImage;
}

SDL_Surface* sdl_opt::load_files(char* file)
//bool sdl_opt::load_files(char* file)
{
    unsigned long int colorkey;
    printf("%s\n", __func__);
    optim = load_image();
    if (screen == NULL) {
        printf("load_image error");
        return false;
    } else {
        printf("%s\n", __func__);
        colorkey = SDL_MapRGB(screen->format, 0x00, 0xFF, 0xFF);
        SDL_SetColorKey(screen, SDL_SRCCOLORKEY, colorkey);
    }

    return true;
}

SDL_Surface* sdl_opt::get_screen()
{
    return screen;
}
