#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "timer.h"
#include "role.h"

#define SCREEN_WITH     600
#define SCREEN_HEIGHT   480
#define SCREEN_BPP  32
SDL_Surface *screen = NULL;
SDL_Surface *person = NULL;
SDL_Event event;

bool init_sdl_lib()
{
    printf("%s\n", __func__);
    SDL_Init(SDL_INIT_EVERYTHING);

    //设置屏幕模式
    screen = SDL_SetVideoMode(SCREEN_WITH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);

    SDL_WM_SetCaption("Role Action", NULL);

    return true;
}

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


bool load_files()
{
    unsigned long int colorkey;

    person = load_image("person.png");
    if (person == NULL) {
        printf("load_image error\n");
        return 0;
    } else {
        colorkey = SDL_MapRGB(person->format, 0x00, 0xFF, 0xFF);
        SDL_SetColorKey(person, SDL_SRCCOLORKEY, colorkey);
    }


    return 1;
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
    //Timer fps;
    //Background bg;

    init_sdl_lib();
    load_files();
    role.set_screen(person, screen);
    role.set_clips();
    role.set_event(&event);

    SDL_FillRect(screen, &screen->clip_rect, 
            SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF));

    SDL_Flip(screen);

    while (quit != true) {
        role.start();
        while (SDL_PollEvent(&event)) {
        //if (SDL_PollEvent(role.role_get_event())) {
            role.handle_event();

            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        role.move();
        SDL_FillRect(screen, &screen->clip_rect, 
                SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF));

        role.show();

        SDL_Flip(screen);
            //bg.show(); 
        if (role.get_ticks() < 1000 / 20) {
            SDL_Delay((1000/20) - role.get_ticks());
        }
    }

    clean_up();
    return 0;
}
