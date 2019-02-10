#ifndef KKWB_REQUESTPARSE_H
#define KKWB_REQUESTPARSE_H

#include "kknet/util/public.h"
#include <memory>
#include <map>
#include <vector>

#define METHOD_LENGTH 5
#define URL_LENGTH 255
#define VERSION_LENGTH 15
#define HOST_LENGTH 32
#define RESPONSE_LENGTH 65536
#define RESPONSE_LINE_LENGTH 1024*8
#define KEY_LENGTH 128
#define VALUE_LENGTH 4096



namespace kkwb
{

    struct Request
    {
        std::string method;
        std::string url;
        std::string version;
        std::string query;
        std::string body;
        std::string fileurl;
        std::map<string,string> headMap;

    };

    

    void versionParse(const char* buf,std::unique_ptr<Request>& request);
    bool headParse(const char* buf,std::unique_ptr<Request>& request);
    bool headParse(const char* buf,string& key,string& value);
    

}
#endif