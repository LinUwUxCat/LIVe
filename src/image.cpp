#include "image.h"
#include "formats/BMP.h"


SDL_Surface* ImageGetSurface(char* filename){
    //Switch on format...
    //SDL_Surface* s = SDL_CreateSurface(400, 300, SDL_BITSPERPIXEL(32));
    return BMP_GetSurface(filename);
}