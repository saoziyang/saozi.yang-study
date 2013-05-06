#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>
#include "timer.h"
#include "role.h"

#define SCREEN_WITH     600
#define SCREEN_HEIGHT   480
#define SCREEN_BPP  32
SDL_Surface *screen = NULL;

bool init_sdl_lib()
{
    printf("%s\n", __func__);
    SDL_Init(SDL_INIT_EVERYTHING);

    //设置屏幕模式
    screen = SDL_SetVideoMode(SCREEN_WITH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);

    SDL_WM_SetCaption("Role Action", NULL);

    return true;
}

void clean_up()
{
    printf("%s\n", __func__);
    SDL_Quit();
    return;
}

int main()
{
    static bool quit = false;
    Role role("saozi");
    Timer fps;
    //Background bg;

    init_sdl_lib();

    while (quit != true) {
        //fps.start();
        role.handle_event(&quit);
        //bg.show(); 
        //role.show();
    }

    clean_up();
    return 0;
}
