#include "formats/DDS.h"
#include <cstdio>

//Function created with the help of multiple resources on Microsoft's website. 
//Helpful guide to MS formats:
// DWORD -> Uint32
// UINT -> Uint32
SDL_Surface* DDS_GetSurfaceAndMetadata(char* filename, ParamList* Metadata){

    FILE* f = fopen(filename, "rb");
    char* dwMagic = (char*)SDL_malloc(4);
    fread(dwMagic, 1, 4, f);
    if (SDL_strncmp(dwMagic, "DDS ", 4)) return NULL;

    //DDS HEADER
    Uint32 dwSize = 0;
    fread(&dwSize, 1, 4, f);
    if (dwSize != 124) return NULL;

    Uint32 dwFlags = 0; //Flags are here just to confirm which other header va&r contains valid data. However almost all them are required lol
    fread(&dwFlags, 1, 4, f); 

    Uint32 h = 0;
    fread(&h, 1, 4, f);
    Uint32 w = 0;
    fread(&w, 1, 4, f); 

    if (h%4!=0 || w%4!=0)return NULL;

    Uint32 dwPitchOrLinearSize = 0;
    fread(&dwPitchOrLinearSize, 1, 4, f);

    Uint32 dwDepth = 0;
    fread(&dwDepth, 1, 4, f);

    Uint32 dwMipMapCount = 0;
    fread(&dwMipMapCount, 1, 4, f);

    fseek(f, 4*11, SEEK_CUR); //Reserved1 - Unused

    DDS_PIXELFORMAT ddspf;
    fread(&ddspf, sizeof(DDS_PIXELFORMAT), 1, f);

    Uint32 dwCaps = 0;
    fread(&dwCaps, 1, 4, f);

    Uint32 dwCaps2 = 0;
    fread(&dwCaps2, 1, 4, f);

    Uint32 dwCaps3 = 0; //Unused
    fread(&dwCaps3, 1, 4, f);

    Uint32 dwCaps4 = 0; //Unused
    fread(&dwCaps4, 1, 4, f);

    fseek(f, 4, SEEK_CUR); //Reserved2 - Unused
    //END DDS HEADER

    //DDS DXT10 HEADER
    bool isDX10 = !SDL_strncmp((char*)&(ddspf.dwFourCC), "DX10", 4);
    DXGI_FORMAT dx10DxgiFormat;
    D3D10_RESOURCE_DIMENSION dx10ResourceDimension;
    Uint32 dx10MiscFlag;
    Uint32 dx10ArraySize;
    Uint32 dx10MiscFlags2; //Glad to know microsoft's naming is as consistent as mine
    if (isDX10){
        fread(&dx10DxgiFormat, sizeof(DXGI_FORMAT), 1, f);
        fread(&dx10ResourceDimension, sizeof(D3D10_RESOURCE_DIMENSION), 1, f);
        fread(&dx10MiscFlag, 1, 4, f);
        fread(&dx10ArraySize, 1, 4, f);
        fread(&dx10MiscFlags2, 1, 4, f);
    }
    //END DDS DXT10 HEADER

    bool isCompressedData = ddspf.dwFlags & 0x4;
    if (!isCompressedData) return NULL; //Weirdly enough, uncompressed data isn't supported

    Metadata->addParameter("Size", "%dx%d", w, h);
    Metadata->addParameter("Linear Size", "%d", dwPitchOrLinearSize);
    Metadata->addParameter("Depth", "%d", dwDepth);
    Metadata->addParameter("MipMaps", "%d", dwMipMapCount);

    Uint8* pixels;
    SDL_PixelFormatEnum pxfm;

    char fourCCsafe[5] = {' ',' ',' ',' ','\0'};
    SDL_memcpy(fourCCsafe, &(ddspf.dwFourCC), 4);
    int DDSPitch = 1;

    if (!SDL_strncmp((char*)&(ddspf.dwFourCC), "DXT1", 4) || (isDX10 && (dx10DxgiFormat == DXGI_FORMAT_BC1_UNORM || dx10DxgiFormat == DXGI_FORMAT_BC1_UNORM_SRGB))){
        //DXT1 / BC1
        DDSPitch = SDL_max(1, ((w+3)/4)) * 8;
        pixels = DDS_BC1GetPixels(f, w, h, DDSPitch);
        pxfm = SDL_PIXELFORMAT_RGB565;

        Metadata->addParameter("Compression Format", "%s / %s", fourCCsafe, "BC1");
    }

    
    Metadata->addParameter("Pixel Format", "%s", SDL_GetPixelFormatName(pxfm));
    fclose(f);
    SDL_Surface* s = SDL_CreateSurfaceFrom((void*)pixels, w, h, DDSPitch, pxfm);
    return s;
}

Uint8* DDS_BC1GetPixels(FILE* f, Uint32 w, Uint32 h, int DDSPitch){
    Uint16* pixels = (Uint16*)SDL_malloc(h*DDSPitch);
    for (int i = 0; i < h; i+=4){
        for (int k = 0; k < w; k+=4){
            Uint16 colors[4] = {0,0,0,0};
            fread(&colors[0], 2, 1, f);
            fread(&colors[1], 2, 1, f);
            colors[2] = DDS_DivideRGB565(colors[0], colors[1], 2, 1, 3); // Note : According to the MS docs, this is not the same here if the file has alpha.
            colors[3] = DDS_DivideRGB565(colors[0], colors[1], 1, 2, 3); // However, using dwFlags & 1 to check if alpha is present didn't prove successful for some reason.
            for (int j = 0; j < 4; j++){                                 // It ended up being 1 when sometimes no alpha was on the file, so i'm confused.
                int row = fgetc(f);
                pixels[i*w+k+j*w+3] = colors[(row >> 6) & 3];
                pixels[i*w+k+j*w+2] = colors[(row >> 4) & 3];
                pixels[i*w+k+j*w+1] = colors[(row >> 2) & 3];
                pixels[i*w+k+j*w+0] = colors[row & 3];
            }
        }
    }
    return (Uint8*)pixels;
}

//The output may not be right. It looks different in gimp. But that's as close as i can get i think.
//I use RGB565 to display in the end but maybe gimp converts that before doing the divisions? i don't know.
Uint16 DDS_DivideRGB565(Uint16 color_0, Uint16 color_1, float d0, float d1, float dv){ 
    Uint16 c = d0/dv*(color_0 & 0b11111) + d1/dv*(color_1 & 0b11111);
    c += (Uint16)(d0/dv*((color_0 >> 5) & 0b111111) + d1/dv*((color_1 >> 5) & 0b111111)) << 5;
    c += (Uint16)(d0/dv*((color_0 >> 11) & 0b11111) + d1/dv*((color_1 >> 11) & 0b11111)) << 11;
    return c;
}