#ifndef PARAMETEREDITORFACTORY
#define PARAMETEREDITORFACTORY

#include <qwidget.h>
#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <iostream.h>

#include "parameter.h"
#include "floatparametereditor.h"

class ParameterEditorFactory{
   public:
   /*! Returns a StringList containing the classnames of all
    * subclasses of Parameter, for which an Editor widget is implemented
    */
   static QStringList list();

   /*! Returns true if an Editor widget can be created for a Parameter subclass
    * with the given name.
    */
   static bool editorAvailableFor(QString paramclass);

   /*! Returns true if an Editor widget can be created for the given Parameter
    */
   static bool editorAvailableFor(Parameter * param);

   /*! creates an editor widget that edits param, and is a child widget of parent*/
   static QWidget * createEditor(QWidget * parent=0,QString name="unnamed editor", Parameter * param=0);
};

#endif
