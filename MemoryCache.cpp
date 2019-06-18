#include "MemoryCache.h"
using namespace kkwb;
MemoryCache::~MemoryCache()
{
    CacheNode* walk = head_;
    CacheNode* temp=0;
    while(walk)
    {
        temp = walk;
        walk = walk->next;
        delete temp;
    }
}
MemoryCache* MemoryCache::memCache = 0;
pthread_once_t MemoryCache::once = PTHREAD_ONCE_INIT;

MemoryCache* MemoryCache::getInstance()
{
    pthread_once(&once,MemoryCache::init);
    return memCache;
}

CacheNode* MemoryCache::getNode(const string& key)
{
    CacheNode* node =  keyMap_[key];
    
    if(node)
    {
        if(node != head_)
        {
            CacheNode* node_next = node->next;
            node->next = head_;
            node->last->next = node_next;
            if(node_next)
                node_next->last = node->last;
            else
            {
                tail_ = node->last;
                
            }
            node->last = 0;
            head_->last = node;
            head_ = node;
        }
    }
    return node;
}

string MemoryCache::getData(const string& key)
{
    string result;
    CacheNode* node = getNode(key);
    return node->data;
}

void MemoryCache::pushNode(const string& key,CacheNode* node)
{
    if(keyMap_[key] != 0)
    {
        deleteNode(keyMap_[key]);
    }

    if(head_)
    {
        node->next = head_;
        head_->last = node;
    }
    else
    {
        tail_ = node;
        
    }
    keyMap_[key] = node;
    head_ = node;
    size_ += node->length;
    removeSpace();
}

bool MemoryCache::deleteNode(const string& key)
{
    CacheNode* node  = keyMap_[key];
    if(node == 0)
        return false;
    if(node == head_)
    {
        head_ = node->next;
        if(head_ == 0)
        {
            tail_ = 0;
        }
        else
        {
            head_->last = 0;
        }
    }
    else if(node == tail_)
    {
        tail_ = node->last;
        if(tail_ == 0)
        {
            head_ = 0;
        }
        else
        {
            tail_->next = 0;
        }
    }
    else
    {
        node->last->next = node->next;
        node->next->last = node->last;
    }
    size_ -= node->length;
    keyMap_.erase(key);
    delete node;
    return true;
}

void MemoryCache::removeSpace()
{
    while(size_ > MEMORY_CACHE_MAXSIZE)
    {
        deleteNode(tail_);
    }
}

bool MemoryCache::deleteNode(CacheNode* node)
{
    if(node == 0)
        return false;
    if(node == head_)
    {
        head_ = node->next;
        if(head_ == 0)
        {
            tail_ = 0;
        }
        else
        {
            head_->last = 0;
        }
    }
    else if(node == tail_)
    {
        tail_ = node->last;
        if(tail_ == 0)
        {
            head_ = 0;
        }
        else
        {
            tail_->next = 0;
        }
    }
    else
    {
        node->last->next = node->next;
        node->next->last = node->last;
    }

    size_ -= node->length;
    keyMap_.erase(node->url);
    delete node;
    return true;              
}