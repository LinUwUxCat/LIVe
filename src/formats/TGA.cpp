#include "formats/TGA.h"
#include "utils.h"
#include <cstdio>

SDL_Surface* TGA_GetSurfaceAndMetadata(char* filename, ParamList* Metadata){
    FILE* f = fopen(filename, "rb");
    if (fseek(f, -26, SEEK_END)) return NULL; //File size < 26 bytes?

    int extOffset = 0;
    int devOffset = 0;
    fread(&extOffset, 1, 4, f);
    fread(&devOffset, 1, 4, f);

    // Version Check
    char* sig = (char*)SDL_malloc(18*sizeof(char));
    int version = 1;
    fread(sig, 1, 18, f);
    if (!SDL_strcmp(sig, "TRUEVISION-XFILE.")){ //Version 1 has no real signature so...
        version = 2;
    }
    Metadata->addParameter("TGA Version", "%d", version);
    fseek(f, 0, SEEK_SET);
    
    //Header
    int IDLen = fgetc(f);
    Metadata->addParameter("Image ID field Length", "%d", IDLen);
    bool hasColorMap = (bool)fgetc(f);
    Metadata->addParameter("Has Colormap", "%s", hasColorMap?"true":"false");
    int imgType = fgetc(f);
    Metadata->addParameter("Image Type", "%d", imgType);
    if (hasColorMap){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "NotImplemented : Implement TGA ColorMap!!");
        return NULL;
    }
    fseek(f, 5, SEEK_CUR);
    fseek(f, 4, SEEK_CUR); //Always 0 unless your file sucks
    int w = 0;
    int h = 0;
    fread(&w, 1, 2, f);
    fread(&h, 1, 2, f);
    Metadata->addParameter("Size", "%dx%d", w, h);
    int bpp = fgetc(f);
    Metadata->addParameter("Bits per pixel", "%d", bpp);
    int desc = fgetc(f); //TODO : use the alpha stuff in here
    bool RtL = (bool)((desc >> 4) & 1);
    bool TtB = (bool)((desc >> 5) & 1);
    Metadata->addParameter("Image Origin", "%s %s", (TtB?"Top":"Bottom"), (RtL?"Right":"Left"));

    //Image/Color Data
    if (IDLen!=0){
        char* IDInfo = (char*)SDL_malloc(IDLen);
        Metadata->addParameter("Image ID", IDInfo);
    }

    //TODO ! Read color map data

    int BytesPerPixel = ((bpp+8-1)&-8)/8;   //Round to next multiple of 8, then divide by 8 to get Bytes per pixel.
                                            //Doing just /8 would work for 8/16/24/32 bpp but 15bpp exists.

    Uint8* pixels = (Uint8*)SDL_malloc(w*h*BytesPerPixel); 

    for (Uint64 i=0; i<w*h*BytesPerPixel; i++)pixels[i] = fgetc(f);

    //Extension area. This is optional. Only here if version = 2
    if (version ==2 && extOffset != 0){
        fseek(f, extOffset, SEEK_SET);
    }

    //Developer area. This is related to the software ID in the extension area. Optional. Only here if version = 2
    if (version == 2 && devOffset != 0){
        Metadata->addParameter("Developer area", "UNKNOWN"); //Depends
    }
    
    Metadata->addParameter("Pixel Format", (char*)SDL_GetPixelFormatName(TGA_GetPixelFormat(bpp)));
    
    fclose(f);
    SDL_Surface* s = SDL_CreateSurfaceFrom((void*)pixels, w, h, w*BytesPerPixel, TGA_GetPixelFormat(bpp));
    if (!TtB) FlipSurfaceVertical(s);
    if (RtL) FlipSurfaceHorizontal(s);
    return s;
}

//Gets the correct pixel format according to the TGA spec. However this should also use the alpha stuff from desc.
SDL_PixelFormatEnum TGA_GetPixelFormat(int BitsPerPixel){
    switch (BitsPerPixel){
        case 8 : return SDL_PIXELFORMAT_RGB332;
        case 15: return SDL_PIXELFORMAT_XRGB1555;
        case 16: return SDL_PIXELFORMAT_ARGB1555;
        case 24: return SDL_PIXELFORMAT_BGR24;
        case 32: return SDL_PIXELFORMAT_ARGB8888;
        default: return SDL_PIXELFORMAT_UNKNOWN;
    }
}