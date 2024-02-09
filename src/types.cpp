#include "types.h"
#include <cstdio>
#include <cstdarg>

void ParamList::addParameter(char *parameter, char *value, ...){
    this->parameters.push_back(parameter);
    va_list args;
    va_start(args, value);
    char* val;
    vasprintf(&val, value, args);
    va_end(args);
    this->values.push_back(val);
}

void ParamList::clear(){
    this->parameters.clear();
    this->values.clear();
}

int ParamList::length(){
    if (this->parameters.size() != this->values.size()){
        return -1; //What
    } else return this->parameters.size();
}
