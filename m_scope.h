#ifndef M_SCOPE_H
#define M_SCOPE_H

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
#include <qtimer.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "module.h"
#include "port.h"

#define MODULE_SCOPE_WIDTH                 90
#define MODULE_SCOPE_HEIGHT                80

class M_scope : public Module
{
  Q_OBJECT

  private:
    int mode, edge, triggerMode;
    float zoom, timeScale, triggerThrs;
    long wavDataSize;
    float gain;
    float mixer_gain[2]; 
    int agc;
    Port *port_in[2];
    float *floatdata;
    QTimer *timer;
    
  public: 
    float **inData[2];
                            
  public:
    M_scope(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~M_scope();
    int setGain(float p_gain);
    float getGain();

  public slots:
    void generateCycle();
    void showConfigDialog();
    void timerProc();
    void updateTimeScale(int val);
    void updateZoom(int val);
    void updateTriggerThrs(int val);
    void updateTriggerMode(int val);
    void updateEdge(int val);
};
  
#endif
