#include "functions.h"

#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>


namespace kkwb
{
    void addLine(char* buf,const char* data,size_t size,int &index)
    {
        if(size!=0)
        {
            ::memcpy(buf+index,data,size);
            index+=size;
        }
        ::memcpy(buf+index,"\r\n",2);
        index+=2;
    }

    void addWord(char* buf,const char* data,size_t size,int &index,bool first)
    {
        if(!first)
        {
            ::memcpy(buf+index," ",1);
            index+=1;
        }
        ::memcpy(buf+index,data,size);
        index+=size;
    }

    bool isHaveFile(const string& url)
    {
        int fd = ::open(url.c_str(),O_RDONLY|O_CLOEXEC);
        if(fd!=-1)
        {
            ::close(fd);
            return true;
        }
        else
            return false;
    }


    bool isDIR(const string& url)
    {
        int fd = ::open(url.c_str(),O_RDONLY|O_CLOEXEC);
        struct stat buf;
        ::fstat(fd,&buf);
        if(fd!=-1)
        {
            ::close(fd);
        }

        return S_ISDIR(buf.st_mode);
            
    }

    string getExtendName(const string& url)
    {
        char buf[20] = {};
        int pos = 0;
        for(auto iterater = url.cend();iterater!=url.cbegin();iterater--)
        {
            if(*iterater == '.')
            {
                pos = iterater - url.cbegin();
                break;
            }
        }
        if(pos)
            return string(url.c_str()+pos);
        else
            return string("");
    }

    string getMIME(const string& extendName)
    {
        if(extendName == ".htm" || extendName == ".html")
        {
            return "text/html";
        
        }
        else if(extendName == ".xml")
        {
            return "text/xml";
        }
        else if(extendName == ".txt")
        {
            return "text/plain";
        }
        else if(extendName == ".png")
        {
            return "image/png";
        }
        else if(extendName == ".gif")
        {
            return "image/gif";
        }
        else if(extendName == ".jpg")
        {
            return "image/jpeg";
        }
        else if(extendName == ".js")
        {
            return "application/javascript";
        }
        else if(extendName == ".css")
        {
            return "text/css";
        }
        else if(extendName == ".ico")
        {
            return "image/x-icon";
        }
        else
        {
            return "none";
        }
    }

    string turnSmallLetter(const string& letter)
    {
        string str = letter;
        for(auto &ch:str)
        {
            if(ch>='A' && ch <= 'Z')
            {
                ch -= 'A'-'a';
            }
        }
        return str;
    }

    void show(const string& str)
    {
        printf("%s\n",str.c_str());

    }
}


