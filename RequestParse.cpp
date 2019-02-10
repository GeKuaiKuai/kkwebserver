#include "RequestParse.h"
#include "functions.h"
#include "kknet/util/public.h"

using namespace kkwb;

void kkwb::versionParse(const char* buf,std::unique_ptr<Request>& request)
{
    int start = 0;
    int end = 0;
    
    //读取method
    while(buf[end]!=32) end++;
    request->method = string(buf+start,end-start);
    end++;
    start = end;

    //读取URL
    while(buf[end]!=32 && buf[end]!='?') end++;
    request->url = string(buf+start,end-start);
    end++;
    start = end;

    if(buf[end-1] == '?')
    {
        while(buf[end]!=32) end++;
        request->query = string(buf+start,end-start);
        end++;
        start = end;
    }

    //读取Version
    while(buf[end]!='\r' && buf[end]!='\n') end++;
    request->version = string(buf+start,end-start);
    end++;
    start = end;

}

bool kkwb::headParse(const char* buf,std::unique_ptr<Request>& request)
{
    if(buf[0] == '\n' || (buf[0] == '\r' && buf[1] == '\n'))
    {
        return false;
    }
        
    char key[KEY_LENGTH] = {0};
    char value[VALUE_LENGTH] = {0};
    int begin = 0;
    int end = 0;
    while(buf[end]!=':')
        end++;
    ::memcpy(key,buf+begin,end-begin);
    end+=2;
    begin = end;
    while(buf[end]!='\r' && buf[end]!='\n') end++;
    ::memcpy(value,buf+begin,end-begin);

    string keys = turnSmallLetter(key);
    string values = value;
    request->headMap[keys] = value;

    return true;


}

bool kkwb::headParse(const char* buf,string& keys,string& values)
{
    if(buf[0] == '\n' || (buf[0] == '\r' && buf[1] == '\n'))
        return false;
    char key[KEY_LENGTH] = {0};
    char value[VALUE_LENGTH] = {0};
    int begin = 0;
    int end = 0;
    while(buf[end]!=':')
        end++;
    ::memcpy(key,buf+begin,end-begin);
    end+=2;
    begin = end;
    while(buf[end]!='\r' && buf[end]!='\n') end++;
    ::memcpy(value,buf+begin,end-begin);
    keys = key;
    values = value;    
}