#ifndef PARAMETEREDITOR_H
#define PARAMETEREDITOR_H

#include "parameter.h"
#include <qhbox.h>
#include <qwidget.h>
/*! @brief just a tagging interface :-)
 */
class ParameterEditor :public QHBox{
   Q_OBJECT
   public:
   ParameterEditor(QWidget *parent=0, const char * name=0, unsigned int flags=0);
   virtual Parameter * getParameter()=0;
};

#endif
