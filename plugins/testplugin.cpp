#include "testplugin.h"
#include <qstring.h>
#include <qobject.h>

TestPlugin::TestPlugin()
:Plugin(),QObject(),observer<int>(),observer<string>(),subject<int>(),subject<string>()
{
  observer<string>::setDebug(QString("observer<string> ")+QString::number(upiid()));
  observer<int>::setDebug(QString(("observer<int> "))+QString::number(upiid()));
  subject<string>::setDebug(QString("subject<string> ")+QString::number(upiid()));
  subject<int>::setDebug(QString("subject<int> ")+QString::number(upiid()));

  cout<<upiid()<<" TestPlugin::TestPlugin(): i have been constructed :-) "<<endl;
}
TestPlugin::~TestPlugin(){
  cout<<upiid()<<" TestPlugin::~TestPlugin(): i have been destroyed :-) "<<endl;
}

void TestPlugin::thatSpecialFunction(){
  cout<<upiid()<<"I Am Unique. period."<<endl;
}
PLUGIN_AUTOLOAD(TestPlugin)
