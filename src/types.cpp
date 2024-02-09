#include "types.h"

void ParamList::addParameter(char *parameter, char *value){
    this->parameters.push_back(parameter);
    this->values.push_back(value);
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
