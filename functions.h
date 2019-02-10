#ifndef KKWB_FUNCTIONS_H
#define KKWB_FUNCTIONS_H
#include <unistd.h>
#include "kknet/util/public.h"
#include <sstream>

namespace kkwb
{
    void addLine(char* buf,const char* data,size_t size,int &index);
    void addWord(char* buf,const char* data,size_t size,int &index,bool first);
    bool isHaveFile(const string& url);
    bool isDIR(const string& url);
    string getExtendName(const string& url);
    string getMIME(const string& extendName);
    string turnSmallLetter(const string& letter);

    template <class Type> 
    Type stringToNum(const string& str)
    { 
        std::istringstream iss(str); 
        Type num; 
        iss >> num; 
        return num;     
    } 

    void show(const string& str);


}


#endif