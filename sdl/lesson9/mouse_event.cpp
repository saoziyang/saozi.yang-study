#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>


#define SCREEN_WIDTH   640 
#define SCREEN_HEIGHT  480 
#define SCREEN_BPP  32

SDL_Surface *screen = NULL;
SDL_Surface *buttonSheet = NULL;
TTF_Font *font = NULL;
SDL_Event event;
SDL_Rect clips[4];
const int CLIP_MOUSEOVER = 0;
const int CLIP_MOUSEOUT = 1;
const int CLIP_MOUSEDOWN = 2;
const int CLIP_MOUSEUP = 3;


class Button
{
    private:
        SDL_Rect box;
        SDL_Rect* clip;
    public:
        Button(int x, int y, int w, int h);
        void hand_events();
        void show();
};

Button::Button(int x, int y, int w, int h)
{
    box.x = x;
    box.y = y;
    box.w = w;
    box.h = h;

    clip = &clips[CLIP_MOUSEOUT];
}

void Button::hand_events()
{
    int x = 0, y = 0;
    //鼠标移动
    if (event.type == SDL_MOUSEMOTION) {
        x = event.motion.x;
        y = event.motion.y;
        if ((x > box.x) && (x < box.x + box.w) &&
            (y > box.y) && (y < box.y + box.h)) {
            clip = &clips[CLIP_MOUSEOVER];
        } else {
            clip = &clips[CLIP_MOUSEOUT];
        }
    }

    //鼠标左键按下
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            x = event.button.x;
            y = event.button.y;

            if ((x > box.x) && (x < box.x + box.w) &&
                (y > box.y) && (y < box.y + box.h)) {
                clip = &clips[CLIP_MOUSEDOWN];
            }
        }
    }

    //鼠标左键抬起
    if (event.type == SDL_MOUSEBUTTONUP) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            x = event.button.x;
            y = event.button.y;

            if ((x > box.x) && (x < box.x + box.w) &&
                (y > box.y) && (y < box.y + box.h)) {
                clip = &clips[CLIP_MOUSEUP];
            }
        }
    }
}

void apply_surface( int x, int y, SDL_Surface* source, 
        SDL_Surface* destination, SDL_Rect* clip)
{
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;

    SDL_BlitSurface(source, clip, destination, &offset);
    return;
}

void Button::show()
{
    //Show the button
    apply_surface(box.x, box.y, buttonSheet, screen, clip);
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


bool init()
{
    int ret = 0;

    ret = SDL_Init(SDL_INIT_EVERYTHING);
    if (ret < 0) {
        printf("SDL INIT ERROR\n");
        return 0;
    }

    ret = TTF_Init();
    if (ret < 0) {
        printf("TTY_Init\n");
        return 0;
    }

    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
    if (screen == NULL) {
        printf("SDL_SetVideoMode ERROR");
        return 0;
    }

    SDL_WM_SetCaption("message", NULL);

    return 1;
}

void clean_up()
{
    SDL_FreeSurface(buttonSheet);
    TTF_CloseFont(font);

    SDL_Quit(); 
}

bool load_files()
{
    unsigned long int colorkey;

    buttonSheet = load_image( "button.png");
    if (buttonSheet == NULL) {
        printf("load_image error\n");
        return false;
    }

    return 1;
}

void set_clips()
{
    //Clip the sprite sheet
    clips[ CLIP_MOUSEOVER ].x = 0;
    clips[ CLIP_MOUSEOVER ].y = 0;
    clips[ CLIP_MOUSEOVER ].w = 320;
    clips[ CLIP_MOUSEOVER ].h = 240;

    clips[ CLIP_MOUSEOUT ].x = 320;
    clips[ CLIP_MOUSEOUT ].y = 0;
    clips[ CLIP_MOUSEOUT ].w = 320;
    clips[ CLIP_MOUSEOUT ].h = 240;

    clips[ CLIP_MOUSEDOWN ].x = 0;
    clips[ CLIP_MOUSEDOWN ].y = 240;
    clips[ CLIP_MOUSEDOWN ].w = 320;
    clips[ CLIP_MOUSEDOWN ].h = 240;

    clips[ CLIP_MOUSEUP ].x = 320;
    clips[ CLIP_MOUSEUP ].y = 240;
    clips[ CLIP_MOUSEUP ].w = 320;
    clips[ CLIP_MOUSEUP ].h = 240;
}


//int main(int argc, char* argv[])
int main()
{
    int ret = 0;
    bool quit = false;

    ret = init();
    if (ret == 0) {
        printf("init error\n");
        return 1;
    }

    ret = load_files(); 
    if (ret == 0) {
        printf("load files error\n");
        return 1;
    }

    //Clip the sprite sheet
    set_clips();

    Button myButton( 170, 120, 320, 240 );


    //SDL_Delay(2000);
    while (quit == false) {
        if (SDL_PollEvent(&event)) {

            myButton.hand_events();

            if (event.type == SDL_QUIT) {
               quit = true; 
            }
        }

        SDL_FillRect(screen, &screen->clip_rect, 
                SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF));
        myButton.show();
        SDL_Flip(screen);
    }

    clean_up();
    return 0;
}
