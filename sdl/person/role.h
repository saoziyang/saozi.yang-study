#pragma once
#include <string>
#include <SDL/SDL.h>
#include "timer.h"
using namespace std;

class Role : public Timer {
    private:
        enum {
            STOP,
            MOVE_R,
            MOVE_L,
            JUMP,
            DROP,
        };

        string name;
        int frame;
        int velocity;
        int height;
        int move_status;
        int direct;
        SDL_Surface* src;
        SDL_Surface* dest;
        SDL_Rect postion;
        SDL_Rect* cur_clip;
        SDL_Rect rm[4];
        SDL_Rect lm[4];
        SDL_Event* event;
    public:
        Role(string str);
        void handle_event();
        void set_clips();
        void set_event(SDL_Event* ev);
        void set_screen(SDL_Surface *s, SDL_Surface *d);
        void show();
        void move();
        void apply_surface(SDL_Surface* source, 
                 SDL_Surface* destination, SDL_Rect* cur);
        //SDL_Event* role_get_event() { return &event;}
        //void beat();
        //void jump();
};
