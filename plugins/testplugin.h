#ifndef TESTMODULE_H
#define TESTMODULE_H
#include "plugin.h"
#include "qobject.h"
#include "interfaces/observer.h"
#include "interfaces/subject.h"
class TestPlugin
:public QObject, 
 public Plugin,
 public observer<string>,
 public observer<int>,
 public subject<string>,
 public subject<int>
 {
  Q_OBJECT

  public:
    virtual void thatSpecialFunction();
  TestPlugin();
  virtual ~TestPlugin();
  static PluginInfo  info(){
    PluginInfo foobar;
    foobar.className="TestPlugin";
    foobar.version="0.1";
    foobar.world="AMS-2";
    foobar.synopsis="QObject";
    foobar.author="Lukas Degener";
    foobar.licens="GPL";
    foobar.description="A totaly pointless example plugin without any functionality.";
    foobar.provides.insert("TestPlugin");
    foobar.provides.insert("QObject");
    foobar.provides.insert("observer<int>");
    foobar.provides.insert("observer<string>");
    foobar.provides.insert("subject<int>");
    foobar.provides.insert("subject<string>");
    foobar.requires.insert("Plugin");
    return foobar;
  };

  virtual void onNotify(const string &e){
    cout<<upiid()<<": I was notified. The event was a string: "<<e<<endl;
    cout<<upiid()<<": I will now send the integer 42 to my observers"<<endl;
    subject<int>::notify((int)42);
  };

  virtual void onNotify(const int &e){
    cout<<upiid()<<": I was notified. The event was an int: "<<e<<endl;
    cout<<upiid()<<": if the int is NOT 42, i will send the string \"foobar\" to my observers"<<endl;
    if(e!=42) subject<string>::notify((string)"foobar");
  };
  

};


#endif
