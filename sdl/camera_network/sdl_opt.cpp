#include "sdl_opt.h"

#define SCREEN_WITH     600
#define SCREEN_HEIGHT   480
//#define SCREEN_BPP  32
#define SCREEN_BPP  16

sdl_opt::sdl_opt()
{
    //if (init_flag != 1) {
        int ret;
        printf("%s\n", __func__);
        ret = SDL_Init(SDL_INIT_EVERYTHING);
        if (ret < 0) {
            printf("SDL_Init error\n");
            return;
        }

        //screen = SDL_SetVideoMode(SCREEN_WITH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);;
        screen = SDL_SetVideoMode(SCREEN_WITH, SCREEN_HEIGHT, 
                            SCREEN_BPP, SDL_SWSURFACE | SDL_DOUBLEBUF);

        SDL_WM_SetCaption("Camera Capture", NULL);
        if (ret < 0) {
            printf("SDL_WM_SetCaption\n");
            return;
        }

        //overlay = SDL_CreateYUVOverlay(640, 480, SDL_YVYU_OVERLAY, screen);
        //overlay = SDL_CreateYUVOverlay(640, 480, SDL_YUY2_OVERLAY, screen);
        overlay = SDL_CreateYUVOverlay(640, 480, SDL_IYUV_OVERLAY, screen);
        if ( overlay == NULL ) {
            return;
        }

        init_flag = 1;
    //} 
}

SDL_Surface* sdl_opt::load_image(char* file)
{
    SDL_Surface* Image = NULL;
    SDL_Surface* optimizedImage = NULL;

    Image = IMG_Load(file);
    if (Image == NULL) {
        printf("IMG_Load error\n");
        return NULL;
    } else {
        optimizedImage = SDL_DisplayFormat(Image);
        SDL_FreeSurface(Image);
    }

    optim = optimizedImage;
    return optimizedImage;
}

SDL_Surface* sdl_opt::load_files(char* file)
{
    unsigned long int colorkey;
    printf("%s\n", __func__);
    optim = load_image(file);
    if (optim == NULL) {
        printf("load_image error");
        return false;
    } else {
        colorkey = SDL_MapRGB(optim->format, 0x00, 0xFF, 0xFF);
        SDL_SetColorKey(optim, SDL_SRCCOLORKEY, colorkey);
    }

    return optim;
}

void sdl_opt::ov_sdl_display(unsigned char* p)
{
    SDL_Rect rect;
    unsigned char* dst = NULL;

    dst = overlay->pixels[0];

    //SDL_LockSurface(screen); 
    //SDL_LockYUVOverlay(overlay);
    //memcpy(dst, p, 640*480*2);
    //overlay->pixels[0] = p;
    memcpy(overlay->pixels[0], p, 640*480);
    memcpy(overlay->pixels[1], p+640*480, 640*480/2);
    memcpy(overlay->pixels[2], p+640*480 + 640*480/2, 640*480/2);
    //SDL_UnlockYUVOverlay(overlay);
    //SDL_UnlockSurface(screen); 

    rect.x = 0;
    rect.y = 0;
    rect.w = 640;
    rect.h = 480;

    SDL_DisplayYUVOverlay(overlay,&rect);
}

SDL_Surface* sdl_opt::get_screen()
{
    return screen;
}

SDL_Event* sdl_opt::get_event()
{
    return &event;
}

void sdl_opt::update_screen()
{
    SDL_Flip(this->screen);
}
