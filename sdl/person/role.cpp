#include "role.h"

Role::Role(string str)
{
    name = str;
}

void Role::handle_event(bool *qf)
{
    while (SDL_PollEvent(&event)){

        if (event.type == SDL_QUIT) {
            *qf = true;
            return;
        }

        Uint8 *keystates = SDL_GetKeyState(NULL);

        if (keystates[SDLK_UP]) {
            printf("%s jump\n", __func__);
        }

        if (keystates[SDLK_DOWN]) {
            printf("%s squat\n", __func__);
        }

        if (keystates[SDLK_RIGHT]) {
            printf("%s move right\n", __func__);
        }

        if (keystates[SDLK_LEFT]) {
            printf("%s move left\n", __func__);
        }
    }
}

void Role::right_move()
{

}
