#include <qwidget.h>
#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <iostream.h>

#include "parameter.h"
#include "floatparametereditor.h"
#include "intparametereditor.h"
#include "enumparametereditor.h"
#include "boolparametereditor.h"
#include "actionparametereditor.h"
#include "parametereditorfactory.h"

QStringList ParameterEditorFactory::list(){
   QStringList sl;
   sl.append("FloatParameter");
   sl.append("IntParameter");
   sl.append("EnumParameter");
   sl.append("BoolParameter");
   sl.append("ActionParameter");
   //others will be added here as soon as they are implemented.
   return sl;
}

bool ParameterEditorFactory::editorAvailableFor(QString paramclass){
   if (paramclass=="FloatParameter") return true;
   if (paramclass=="IntParameter") return true;
   if (paramclass=="EnumParameter") return true;
   if (paramclass=="BoolParameter") return true;
   if (paramclass=="ActionParameter") return true;
   return false;
}

bool ParameterEditorFactory::editorAvailableFor(Parameter * param){
   if (!param) return false;
   if (param->isA("FloatParameter")) return true;
   if (param->isA("IntParameter")) return true;
   if (param->isA("EnumParameter")) return true;
   if (param->isA("BoolParameter")) return true;
   if (param->isA("ActionParameter")) return true;
   return false;
}
QWidget * ParameterEditorFactory::createEditor(QWidget * parent,QString name, Parameter * param){
   if (!param) return NULL;
   if (param->isA("FloatParameter")) return new FloatParameterEditor((FloatParameter *)param,parent,name);
   if (param->isA("IntParameter")) return new IntParameterEditor((IntParameter *)param,parent,name);
   if (param->isA("EnumParameter")) return new EnumParameterEditor((EnumParameter *)param,parent,name);
   if (param->isA("BoolParameter")) return new BoolParameterEditor((BoolParameter *)param,parent,name);
   if (param->isA("ActionParameter")) return new ActionParameterEditor((ActionParameter *)param,parent,name);
   return new QLabel("Sorry, not implemented yet!",parent);
}
