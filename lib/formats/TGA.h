#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "types.h"

SDL_Surface* TGA_GetSurfaceAndMetadata(char* filename, ParamList* Metadata);
SDL_PixelFormatEnum TGA_GetPixelFormat(int BitsPerPixel);