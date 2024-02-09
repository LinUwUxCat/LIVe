#include "image.h"
#include "formats/BMP.h"


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
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "The %s format is not supported", ext);
        return NULL;
    }    
}