#ifndef KKWB_RESPONSE_H
#define KKWB_RESPONSE_H
#include <map>
#include <vector>
#include "kknet/util/public.h"

#define HTTP_VERSION "HTTP/1.1"
#define HTTP_STATE_OK "200 OK"
#define HTTP_STATE_NOTFOUND "404 Not Found"
#define HTTP_STATE_FORBIDDEN "403 Forbidden"
#define HTTP_301 "301 Moved Permanently"
#define HTTP_302 "302 Move temporarily"

#define HTTP_CONTENT_TYPE "Content-Type"
#define HTTP_SERVER "Server"
#define HTTP_SERVER_NAME "KK WebServer 0.1"
#define HTTP_LENGTH "Content-Length"

#define HTTP_ERROR_MESS1 "<html><head><title>"
#define HTTP_ERROR_MESS2 "</title></head><body bgcolor=\"white\"><center><h1>"
#define HTTP_ERROR_MESS3 "</h1></center><hr><center>"
#define HTTP_ERROR_MESS4 "</center></body></html>"

namespace kkwb
{
    class Response
    {
        public:
            typedef std::map<string,std::vector<string>> HeadMap;
            void setVersion(const string& version = HTTP_VERSION);
            void setState(const string& state = HTTP_STATE_OK);
            const string& getState()
            {
                return state_;
            }
            void addHead(const string& key,const string& value);
            void updateHead(const string& key,const string& value);
            void deleteHead(const string& key);
            const string& getVersion() const;
            const string& getState() const;
            const HeadMap& getHeads() const
            {
                return heads_;
            }
        private:
            string version_;
            string state_;
            HeadMap heads_;
    };
}



#endif