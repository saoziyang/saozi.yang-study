#include "def_inc.h"
#include "sdl_opt.h"
#include "background.h"

int main()
{
    //class sdl_opt opt();
    background bg;

    bg.load_files("background.png");
    printf("-----\n");

    bg.show();

    while (1) {
        ;
    }
    return 0;
}
