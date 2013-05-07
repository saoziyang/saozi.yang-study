#include "role.h"

Role::Role(string str)
{
    name = str;
    postion.x = 0;
    postion.y = 275;
    velocity = 0;
    frame = 0;
    move_status = STOP;
    direct = 0;
}

void Role::handle_event()
{
    //while (SDL_PollEvent(&event)) {

        if (event->type == SDL_KEYDOWN) {
            switch(event->key.keysym.sym) 
            {
                case SDLK_RIGHT:
                    printf("right key down\n");
                    velocity += 16;
                    move_status = MOVE_R;
                    direct = 0;
                    break;
                case SDLK_LEFT:
                    velocity -= 16;
                    move_status = MOVE_L;
                    direct = 1;
                    break;
                case SDLK_UP:
                    height -= 16;
                    move_status = JUMP;
                    break;
                default:
                    break;
            }
        } else if (event->type == SDL_KEYUP) {
            switch (event->key.keysym.sym)
            {
                case SDLK_RIGHT:
                    printf("right key up\n");
                    velocity -= 16;
                    move_status = STOP;
                    break;
                case SDLK_LEFT:
                    velocity += 16;
                    move_status = STOP;
                    break;

                case SDLK_UP:
                    velocity += 16;
                    postion.y = 10;
                    move_status = DROP;
                    break;

                default:
                    break;
            }
        }
    //}
}

void Role::set_event(SDL_Event* ev)
{
    event = ev;
}
void Role::set_clips()
{
    for (int i = 0; i < 4; i++) {
        rm[i].x = i*64;
        rm[i].y = 0;
        rm[i].w = 64;
        rm[i].h = 205;
    }

    for (int i = 0; i < 4; i++) {
        lm[i].x = i*64;
        lm[i].y = 205;
        lm[i].w = 64;
        lm[i].h = 205;
    }
}

void Role::move()
{
    printf("x:%x\ty:%x\n", postion.x, postion.y);

    if (move_status == MOVE_R) {
        postion.x += velocity;
    } else if (move_status == MOVE_L) {
        postion.x += velocity;
    } else if (move_status == JUMP) {
        if (direct == 0) {
            postion.x += 16;
        } else {
            postion.x -= 16;
        }
        postion.y += height;
    } else if (move_status == DROP) {
        if (direct == 0) {
            postion.x += 16;
        } else {
            postion.x -= 16;
        }
        postion.y += height;
    }

    if (postion.x < 0) {
        postion.x = 0;
    } else if (postion.x > 536) {
        postion.x = 536;
    } 

    if (postion.y < 50) {
        move_status = DROP;
        height = 16;
    }

     if (postion.y > 275) {
        move_status = STOP;
        height = 0;
    }

    //if (postion.y < 50) {
    //    printf("================\n");
    //    move_status = DROP;
    //    //postion.y -= velocity;
    //    postion.y = 275;
    //} else if (postion.y > 275) {
    //    move_status = STOP;
    //    height -= 16;
    //}
}

void Role::apply_surface(SDL_Surface* source, 
        SDL_Surface* destination, SDL_Rect* cur)
{
    SDL_Rect offset;
    offset.x = postion.x;
    offset.y = postion.y;

    SDL_BlitSurface(source, cur, destination, &offset);
    return;
}

void Role::set_screen(SDL_Surface *s, SDL_Surface *d)
{
    src = s;
    dest = d;
}

void Role::show()
{
    if (move_status == MOVE_R) {
        if (frame >= 4) {
            frame = 0;
        }
        apply_surface(src, dest, &rm[frame]); 
        frame++;
    } else if (move_status == MOVE_L) {
        if (frame >= 4) {
            frame = 0;
        }
        apply_surface(src, dest, &lm[frame]); 
        frame++;
    } else if (move_status == JUMP) {
        if (frame >= 4) {
            frame = 0;
        }
        apply_surface(src, dest, &lm[frame]); 
        frame++;
    } else if (move_status == STOP) {

        if (direct == 0) {
            apply_surface(src, dest, &rm[0]); 
        } else if (direct == 1) {
            apply_surface(src, dest, &lm[0]); 
        }
    } else if (move_status == DROP) {
        if (frame >= 4) {
            frame = 0;
        }
        frame++;
        if (direct == 0) {
            apply_surface(src, dest, &rm[frame]); 
        } else if (direct == 1) {
            apply_surface(src, dest, &lm[frame]); 
        }
    }

}
