#include "plugin.h"

map<string,maker_t*> Plugin::create;
map<string,PluginInfo> Plugin::infos;

int Plugin::mLastUPIId=-1;

Plugin::Plugin()
{
  mLastUPIId++;
  mUPIId=mLastUPIId;
}

Plugin::~Plugin()
{
  cout<<"Plugin::~Plugin():  hi! i am the destructor of the plugin class. glad that you call me"<<endl;
}

int Plugin::upiid(){
  return mUPIId;
}
const string & Plugin::pluginKey(){
  return mPluginKey;
}
void Plugin::setKey(const string & key){
  if(mPluginKey.empty())mPluginKey=key;
}






