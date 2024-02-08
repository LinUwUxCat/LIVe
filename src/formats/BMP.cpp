#include "formats/BMP.h"

//Note : SDL has BMP support built-in.
//However with this, i don't get much info about the file (only what's in SDL_Surface) so maybe it will be replaced by my own parser.

SDL_Surface* BMP_GetSurface(char* filename){
    SDL_RWops* file = SDL_RWFromFile(filename, "rb");
    return SDL_LoadBMP_RW(file, SDL_TRUE); 
}