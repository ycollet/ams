#include "plugins/plugin.h" //this is all we know about plugins.
#include <stdio.h>
#include <iostream.h>
#include <qdir.h>
#include <qstringlist.h>
#include <dlfcn.h>
#include <qobject.h>
#include <boost/weak_ptr.hpp>
//#include "testplugin.h"
#include "interfaces/observer.h"
#include "interfaces/subject.h"

#include <vector>
void test_subject_observer(string pluginkey){
  cout<<"i will now try to create to instances of the plugin with the"<<endl;
  cout<<"pluginkey \""<<pluginkey<<"\"";
  
  Plugin * tp1 = Plugin::create[pluginkey]();
  Plugin * tp2 = Plugin::create[pluginkey]();
  //((TestPlugin *)tp1)->thatSpecialFunction();
  cout<<"done!"<<endl;

  cout<<"The instance IDs are "<<tp1->upiid()<<" and "<<tp2->upiid()<<endl;
  cout<<"I will now try to connect the two instances..."<<endl;
  subject<int> * si = dynamic_cast<subject<int> *>(tp1);
  if(!si) exit(-1);
  observer<int> *oi = dynamic_cast<observer<int> *>(tp2);
  if(!oi) exit(-1);
  subject<string> * ss = dynamic_cast<subject<string> *>(tp2);
  if(!ss) exit(-1);
  observer<string> * os = dynamic_cast<observer<string> *>(tp1);
  if(!os) exit(-1);
  oi->attach(si);
  os->attach(ss);
  cout<<"done."<<endl;
  cout<<"Next i will call notify(23) on the first one. "<<endl;
  
  si->notify(23);
  cout<<"you should have seen output from the onNotify methods of both plugins."<<endl<<endl;

  cout<<"Now i will delete the second plugin, pretending i didn't know it was a"<<endl;
  cout<<"TestPlugin, and that it was actualy attached to other plugin."<<endl;
  delete oi;
  cout<<"You should have seen output from the destructor(s)"<<endl;
  cout<<"Finaly i will call notify on the first one as before."<<endl;
  cout<<"This time, you shouldn't see any output from the onNotify methods."<<endl;
  si->notify(23);
  cout<<"i will now connect the remaining plugin to itself."<<endl;
  oi = dynamic_cast<observer<int> *>(tp1);
  si->attach(oi);
  cout<<"and another notify. let's see what happens..."<<endl;
  si->notify(23);
  cout<<"That's it. I will now delete also the first instance...";
  delete tp1;
  cout<<"done."<<endl;
}

int main(char ** argv,int argc){
  //get a list of all *.so files in the current directory
  QDir dir("plugins/.libs");
  QPtrList<void> handles;
  QStringList qsl = dir.entryList("*.so");
  void * dlib;
  //try to dlopen each entry. That should fill our regestry.
  for (QStringList::iterator it = qsl.begin(); it != qsl.end(); ++it ){
    cout<<"trying to dlopen "<<*it<<"...";
    dlib=dlopen(dir.absFilePath(*it),RTLD_NOW);
    if(!dlib){
      cout<<dlerror()<<endl;
    }
    else{
      cout<<"success!"<<endl;
      handles.append(dlib);
    }
  }

  //now we try to instanciate and destroy all found plugin classes
  //to simulate the way this will usualy take place, we will not to
  //the obvisous, but first get a list of registered class names,
  //and then look up the creator functions for this names, and use them.
  vector<string> classNames;
  for(map<string,maker_t*>::iterator it=Plugin::create.begin();it!=Plugin::create.end();++it){
    classNames.push_back((*it).first);
  }
   //weak_ptr<Plugin> observer;

  QObject *fakeParent=new QObject();
  for (vector<string>::iterator it = classNames.begin(); it != classNames.end(); ++it ){
    
    PluginInfo & pi = Plugin::infos[*it];
    cout<<"For Key \""<<*it<<"\":"<<endl;
    cout<<"  Version: "<<pi.version<<endl;
    cout<<"  Author: "<<pi.author<<endl;
    cout<<"  Licens: "<<pi.licens<<endl;
    cout<<"  Class name: "<<pi.className<<endl;
    cout<<"  Synopsis: "<<pi.synopsis<<endl;
    cout<<"  World: "<<pi.world<<endl;
    cout<<"  provides: "<<endl;
    for(set<string>::iterator jt=pi.provides.begin();jt!=pi.provides.end();++jt){
    	cout<<"    "<<*jt<<endl;
    }
    cout<<"  requires: "<<endl;
    for(set<string>::iterator jt=pi.requires.begin();jt!=pi.requires.end();++jt){
    	cout<<"    "<<*jt<<endl;
    }
    
    cout<<"I will check now, if this plugin is suitible for my"<<endl;
    cout<<"little observer/subject test...(lazy test for now)"<<endl;
    if(pi.provides.find("TestPlugin")!=pi.provides.end()){
      cout<<"Yes, the plugin provides interface \"TestPlugin\", i'm running the test now."<<endl;
      test_subject_observer(*it);
    }
    else{
      cout<<"No, sorry. I will skip the test."<<endl;
    }
    
    cout<<"Instanciating plugin with key \""<<*it<<"\""<<endl;
    Plugin * mod=Plugin::create[*it]();
    cout<<"instance reports:"<<endl;
    cout<<"  UPIId = "<<mod->upiid()<<endl;
    cout<<"  PluginKey = "<<mod->pluginKey()<<endl;
    cout<<"--------------------------"<<endl<<endl;
      
    cout<<"Deleting it again...";
    delete mod;
    cout<<"done."<<endl;
    cout<<"you should have seen messages produced by all involved destructors."<<endl;
  }
  //close the libraries
  cout<<"closing libs"<<endl;
  for(int i=0; i<handles.count(); i++){
            dlclose(handles.at(i));
  }
  exit(0);
}
