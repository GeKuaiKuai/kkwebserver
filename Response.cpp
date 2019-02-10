#include "Response.h"
#include "kknet/util/public.h"
#include "functions.h"
using namespace kkwb;

void Response::setVersion(const string& version)
{
    this->version_ = version;
}

void Response::setState(const string& state)
{
    this->state_ = state;
}

void Response::addHead(const string& key,const string& value)
{
    string skey = turnSmallLetter(key);
    heads_[skey].push_back(value);
}

void Response::updateHead(const string& key,const string& value)
{
    string skey = turnSmallLetter(key);
    heads_[skey].clear();
    heads_[skey].push_back(value);
}

void Response::deleteHead(const string& key)
{
    string skey = turnSmallLetter(key);
    heads_[skey].clear();
}


const string& Response::getVersion() const
{
    return this->version_;
}

const string& Response::getState() const
{
    return this->state_;
}