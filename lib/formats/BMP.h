#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "types.h"

SDL_Surface* BMP_GetSurface(char* filename);
void BMP_GetMetadata(SDL_Surface* Surface, ParamList* Metadata);