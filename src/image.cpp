#include "image.h"
#include "formats/BMP.h"
#include "formats/TGA.h"
#include "formats/DDS.h"

SDL_Surface* ImageGetSurface(char* filename, ParamList* Metadata = NULL){
    char* ext = SDL_strrchr(filename, '.');
    if (ext == NULL){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not detect format from file!");
        return NULL;
    }
    if (!SDL_strcasecmp(ext, ".bmp")){
        SDL_Surface* s = BMP_GetSurface(filename);
        BMP_GetMetadata(s, Metadata);
        return s;
    } else if (!SDL_strcasecmp(ext, ".tga")){
        return TGA_GetSurfaceAndMetadata(filename, Metadata);
    } else if (!SDL_strcasecmp(ext, ".dds")){
        return DDS_GetSurfaceAndMetadata(filename, Metadata);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "The %s format is not supported", ext);
        return NULL;
    }    
}

Color24 ImageRGB565To888(Uint16 RGB565){
    Color24 c;
    c.R = ((RGB565 & 0b11111) * 527 + 23) >> 6;
    c.G = (((RGB565 >> 5) & 0b111111) * 259 + 33) >> 6;
    c.B = (((RGB565 >> 11) & 0b11111) * 527 + 23) >> 6;
    return c;
}