#ifndef M_FUNCTION_H
#define M_FUNCTION_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlabel.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qpointarray.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "module.h"
#include "port.h"
#include "function.h"

#define MODULE_FUNCTION_WIDTH                 85
#define MODULE_FUNCTION_HEIGHT                40
#define MODULE_FUNCTION_DEFAULT_POINTCOUNT     8
       
class M_function : public Module
{
  Q_OBJECT

  private:
    QList<Port> out_port_list;
    Port *port_in;
    float gainIn, gainOut;
    QPointArray *points[MAX_FUNCTIONS];
    int i[MAX_FUNCTIONS];
        
  public: 
    float **inData;
    int functionCount;
                            
  public:
    M_function(int p_functionCount, SynthData *p_synthdata, QWidget* parent=0, const char *name=0);
    ~M_function();

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
