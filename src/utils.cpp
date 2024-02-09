#include "utils.h"

//Copied and adjusted from https://github.com/libsdl-org/SDL/blob/1143bdc35130e68c90c9b1a3a2069399b7f6143a/src/video/SDL_surface.c#L1115
void FlipSurfaceHorizontal(SDL_Surface* surface){
    Uint8 *row, *a, *b, *tmp;
    int i, j, bpp;

    if (surface->h <= 0) {
        return;
    }

    if (surface->w <= 1) {
        return;
    }

    bpp = surface->format->BytesPerPixel;
    row = (Uint8 *)surface->pixels;
    tmp = (Uint8*)SDL_malloc(surface->pitch);
    for (i = surface->h; i--; ) {
        a = row;
        b = a + (surface->w - 1) * bpp;
        for (j = surface->w / 2; j--; ) {
            SDL_memcpy(tmp, a, bpp);
            SDL_memcpy(a, b, bpp);
            SDL_memcpy(b, tmp, bpp);
            a += bpp;
            b -= bpp;
        }
        row += surface->pitch;
    }
    SDL_free(tmp);
}

//Copied and adjusted from https://github.com/libsdl-org/SDL/blob/1143bdc35130e68c90c9b1a3a2069399b7f6143a/src/video/SDL_surface.c#L1153
void FlipSurfaceVertical(SDL_Surface* surface){
    Uint8 *a, *b, *tmp;
    int i;

    if (surface->h <= 1) {
        return;
    }

    a = (Uint8 *)surface->pixels;
    b = a + (surface->h - 1) * surface->pitch;
    tmp = (Uint8 *)SDL_malloc(surface->pitch);
    for (i = surface->h / 2; i--; ) {
        SDL_memcpy(tmp, a, surface->pitch);
        SDL_memcpy(a, b, surface->pitch);
        SDL_memcpy(b, tmp, surface->pitch);
        a += surface->pitch;
        b -= surface->pitch;
    }
    SDL_free(tmp);
    return;
}