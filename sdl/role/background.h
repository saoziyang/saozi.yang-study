#include "def_inc.h"
#include "sdl_opt.h"

class background : public sdl_opt 
{
    private:
        SDL_Surface* bg;
    public:
        background();
        void show();
};
