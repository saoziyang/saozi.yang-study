#pragma once
#include <string>
#include <SDL/SDL.h>
using namespace std;

class Role {
    private:
        string name;
        SDL_Surface* r;
        SDL_Rect clip;
        SDL_Rect rm[4];
        SDL_Event event;
    public:
        Role(string str);
        void handle_event(bool *qf);
        void set_clips();
        void show();
        void right_move();
        //void beat();
        //void jump();
};
