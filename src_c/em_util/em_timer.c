#include "em_util/em_timer.h"
#include "SDL/SDL.h"
#include "emscripten/emscripten.h"


void em_sleep(uint32_t ms)
{
    emscripten_sleep(ms);
}

// SDL_Init(0);
uint32_t em_get_ticks()
{
    return SDL_GetTicks();
}
