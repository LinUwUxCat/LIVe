#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "formats/BMP.h"
#include "types.h"


SDL_Surface* ImageGetSurface(char* filename, ParamList* Metadata);
Color24 ImageRGB565To888(Uint16 RGB565);