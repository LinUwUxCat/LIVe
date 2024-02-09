#pragma once

#include <vector>

class ParamList{
    public:
        std::vector<char*> parameters;
        std::vector<char*> values;
        void addParameter(char* parameter, char* value);
        int length();
        void clear();
};