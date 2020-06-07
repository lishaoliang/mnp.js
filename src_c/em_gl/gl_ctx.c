#include "gl_ctx.h"
#include "mem/klb_mem.h"
#include "em_util/em_log.h"
#include <SDL.h>
#include "SDL/SDL_opengl.h"

#include "stdlib.h"

static SDL_Surface* g_screen_surface = NULL;


typedef struct emgl_ctx_t_
{
    SDL_Surface* p_screen_surface;
}emgl_ctx_t;


int emgl_ctx_init(int w, int h, int bpp)
{
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Surface* p_screen = SDL_SetVideoMode(w, h, bpp, SDL_OPENGL);
    if (NULL == p_screen)
    {
        LOG_S("Unable to set video mode: %s\n", SDL_GetError());
        return 1;
    }

    g_screen_surface = p_screen;

    return 0;
}

emgl_ctx_t* emgl_ctx_create()
{
    emgl_ctx_t* p_ctx = KLB_MALLOC(emgl_ctx_t, 1, 0);
    KLB_MEMSET(p_ctx, 0, sizeof(emgl_ctx_t));

    p_ctx->p_screen_surface = g_screen_surface;

    return p_ctx;
}

void emgl_ctx_destroy(emgl_ctx_t* p_ctx)
{
    assert(NULL != p_ctx);

    KLB_FREE(p_ctx);
}

void emgl_ctx_viewport(emgl_ctx_t* p_ctx, int x, int y, int w, int h)
{
    assert(NULL != p_ctx);
    glViewport(x, y, w, h);
}

void emgl_ctx_begin(emgl_ctx_t* p_ctx)
{
    assert(NULL != p_ctx);

    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
}

void emgl_ctx_end(emgl_ctx_t* p_ctx)
{
    assert(NULL != p_ctx);

    SDL_GL_SwapBuffers();
}
