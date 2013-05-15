#include "timer.h"
#include <SDL/SDL.h>

Timer::Timer()
{
    printf("%s\n", __func__);
    startTicks = false;
    pausedTicks = false;
    paused = false;
    started = false;
}

void Timer::start()
{
    started = true;
    paused = false;

    startTicks = SDL_GetTicks();
}

void Timer::stop()
{
    started = false;
    paused = false;
}

void Timer::pause()
{
    if ((started == true) && (paused == false)) {
        paused = true;
        pausedTicks = SDL_GetTicks() - startTicks;
    }
}

void Timer::unpause()
{
    if (paused == true) {
        paused = false;
        startTicks = SDL_GetTicks() - pausedTicks;
        pausedTicks = 0;
    }
}

int Timer::get_ticks()
{
    if (started == true) {
        if (paused == true) {
            return pausedTicks;
        } else {
            return SDL_GetTicks() - startTicks;
        }
    }
    return 0;
}

bool Timer::is_started()
{
    return started; 
}
bool Timer::is_paused()
{ 
    return paused; 
}
