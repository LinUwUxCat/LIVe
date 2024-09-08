#include "formats/TGA.h"
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

    //Color Map Specification
    int colorMapFirstEntryIndex = 0;
    fread(&colorMapFirstEntryIndex, 1, 2, f);
    if (hasColorMap) Metadata->addParameter("ColorMap First Entry", "%d", colorMapFirstEntryIndex);
    int colorMapLen = 0;
    fread(&colorMapLen, 1, 2, f);
    if (hasColorMap) Metadata->addParameter("ColorMap Length", "%d", colorMapLen);
    int colorMapEntrySize = fgetc(f);
    if (hasColorMap) Metadata->addParameter("ColorMap Entry Size", "%d", colorMapEntrySize);

    //Image Specification
    fseek(f, 4, SEEK_CUR); //Lower left position of the image on a screen having an origin on the lower left. Basically some sort of offset that i've never seen being used and that is ignored here either way
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
        fread(IDInfo, 1, IDLen, f);
        Metadata->addParameter("Image ID", IDInfo);
    }

    int BytesPerPixel = ((bpp+8-1)&-8)/8;   //Round to next multiple of 8, then divide by 8 to get Bytes per pixel.
                                            //Doing just /8 would work for 8/16/24/32 bpp but 15bpp exists.
    Uint8* pixels = (Uint8*)SDL_malloc(w*h*BytesPerPixel); 

    if (hasColorMap){
        int BytesPerEntry = ((colorMapEntrySize+8-1)&-8)/8;
        Uint8* colorMapData = (Uint8*)SDL_malloc(colorMapLen*BytesPerEntry);
        fread(colorMapData, 1, colorMapLen*BytesPerEntry, f);
        for (Uint64 i=0; i<w*h; i+=BytesPerPixel){
            int colorNumber = 0;
            fread(&colorNumber, 1, BytesPerPixel, f);
            if (colorNumber >= colorMapLen) colorNumber = 0; // Corrupted file?
            for (int j = 0; j < BytesPerEntry; j++) pixels[i+j] = colorMapData[colorNumber*BytesPerEntry+j];
        }
    } else {
        //Image Data
        for (Uint64 i=0; i<w*h*BytesPerPixel; i++)pixels[i] = fgetc(f);
    }


    //Extension area. This is optional. Only here if version = 2
    if (version == 2 && extOffset != 0){
        fseek(f, extOffset, SEEK_SET);
        int extSize = 495;
        fread(&extSize, 1, 2, f);
        if (extSize == 495){
            //Author Name
            char* authorName = (char*)SDL_malloc(41); 
            fread(authorName, 1, 41, f);
            Metadata->addParameter("Author Name", authorName);
            //Author Comments
            for (int i=0; i<4; i++){
                char* commentLine = (char*)SDL_malloc(81);
                fread(commentLine, 1, 81, f);
                Metadata->addParameter((char*)(i==0?"Author Comments":""), commentLine);
            }
            //DateTime
            int month,day,year,hour,minute,second;
            fread(&month,1,2,f);
            fread(&day,1,2,f);
            fread(&year,1,2,f);
            fread(&hour,1,2,f);
            fread(&minute,1,2,f);
            fread(&second,1,2,f);
            Metadata->addParameter("Date and Time", "%d/%d/%d %d:%d:%d", day, month, year, hour, minute, second);
            //There is more information available. However it is not pertinent for now, and besides i have yet to find a TGA file that has some.


        } else Metadata->addParameter("Extension area", "UNKNOWN");
    }

    //Developer area. This is related to the software ID in the extension area. Optional. Only here if version = 2
    if (version == 2 && devOffset != 0){
        Metadata->addParameter("Developer area", "UNKNOWN"); //Depends
    }
    
    Metadata->addParameter("Pixel Format", (char*)SDL_GetPixelFormatName(TGA_GetPixelFormat(hasColorMap?colorMapEntrySize:bpp)));
    
    fclose(f);
    SDL_Surface* s = SDL_CreateSurfaceFrom(w, h, TGA_GetPixelFormat(bpp), (void*)pixels, w*BytesPerPixel);
    if (!TtB) SDL_FlipSurface(s, SDL_FLIP_VERTICAL); // FlipSurfaceVertical(s);
    if (RtL) SDL_FlipSurface(s, SDL_FLIP_HORIZONTAL); //FlipSurfaceHorizontal(s);
    return s;
}

//Gets the correct pixel format according to the TGA spec. However this should also use the alpha stuff from desc.
SDL_PixelFormat TGA_GetPixelFormat(int BitsPerPixel){
    switch (BitsPerPixel){
        case 8 : return SDL_PIXELFORMAT_RGB332;
        case 15: return SDL_PIXELFORMAT_XRGB1555;
        case 16: return SDL_PIXELFORMAT_ARGB1555;
        case 24: return SDL_PIXELFORMAT_BGR24;
        case 32: return SDL_PIXELFORMAT_ARGB8888;
        default: return SDL_PIXELFORMAT_UNKNOWN;
    }
}