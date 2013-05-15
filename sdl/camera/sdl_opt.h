#pragma once
#include "def_inc.h"

class sdl_opt {
    private:
        int init_flag;
        SDL_Surface* screen;
        SDL_Surface* optim;
        SDL_Event event;
        SDL_Overlay *overlay;
    public:

        sdl_opt();
        SDL_Surface* load_files(char* file);
        SDL_Surface* get_screen();
        SDL_Event* get_event();
        SDL_Surface* load_image(char* file);
        void update_screen();
        void ov_sdl_display(unsigned char* p);
        //virtual ~sdl_opt();
};
