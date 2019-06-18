#ifndef KKWB_MEMCACHE_H
#define KKWB_MEMCACHE_H
#include <kknet/util/public.h>
#include <kknet/util/Mutex.h>
#include <map>
#include <memory>
#define MEMORY_CACHE_MAXSIZE 1024*1024*50 //最大缓存容量 50MB
#define MEMORY_CACHE_MAXFILESIZE 1024*1024 //最大文件大小 1MB
namespace kkwb
{
    struct CacheNode
    {
        CacheNode():next(0),last(0){}
        string url;
        string data;
        timespec time;
        uint64_t length;
        CacheNode* next;
        CacheNode* last;
    };
    class MemoryCache
    {
        public:
            static MemoryCache* memCache;
            MemoryCache():size_(0),head_(0),tail_(0)
            {

            }
            ~MemoryCache();
            static MemoryCache* getInstance();
            CacheNode* getNode(const string& key);
            //使用此方法前一定要用getNode判断一下
            string getData(const string& key);
            void pushNode(const string& key,CacheNode* node);
            bool deleteNode(const string& key);
            void lock()
            {
                mutex.lock();
            }
            void unlock()
            {
                mutex.unlock();
            }
        private:
            void removeSpace();
            static void init()
            {
                memCache = new MemoryCache();
            }
            bool deleteNode(CacheNode* node);
            static pthread_once_t once;
            typedef std::map<string,CacheNode*> KeyMap;
            CacheNode* head_;
            CacheNode* tail_;
            uint64_t size_;
            KeyMap keyMap_;
            kknet::MutexLock mutex;
            
            
    };
}

#endif