#include "formats/BMP.h"

//Note : SDL has BMP support built-in.

SDL_Surface* BMP_GetSurface(char* filename){
    SDL_RWops* file = SDL_RWFromFile(filename, "rb");
    return SDL_LoadBMP_RW(file, SDL_TRUE); 
}