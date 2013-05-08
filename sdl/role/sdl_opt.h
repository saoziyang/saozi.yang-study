#pragma once
#include "def_inc.h"

class sdl_opt {
    private:
        int init_flag;
        SDL_Surface* screen;
        SDL_Surface* optim;
    public:
        sdl_opt();
        bool load_files(char* file);
        SDL_Surface* get_screen();
        SDL_Surface* load_image(char* file);
        //virtual ~sdl_opt();
};
