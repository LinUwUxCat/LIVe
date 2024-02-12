#pragma once

#include <vector>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

class ParamList{
    public:
        std::vector<char*> parameters;
        std::vector<char*> values;
        void addParameter(char* parameter, char* value, ...);
        int length();
        void clear();
};

struct Color24{
    Uint8 R;
    Uint8 G;
    Uint8 B;
};