#include "background.h"

background::background()
{

}

void background::show()
{
    bg = get_screen();

    SDL_Flip(bg);
}
