#ifndef M_VCENV2_H
#define M_VCENV2_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlabel.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "module.h"
#include "port.h"

#define MODULE_VCENV2_WIDTH                110
#define MODULE_VCENV2_HEIGHT               175

class M_vcenv2 : public Module
{
  Q_OBJECT

  private:
    Port *port_M_gate, *port_M_retrigger, *port_M_attack, *port_M_decay, *port_M_sustain, *port_M_release, *port_out;
    
  public: 
    float **gateData, **retriggerData, **attackData, **decayData, **sustainData, **releaseData;        
    float a0, d0, s0, r0, aGain, dGain, sGain, rGain;
    float e[MAXPOLY];
    int state[MAXPOLY];
    bool noteActive[MAXPOLY], gate[MAXPOLY], retrigger[MAXPOLY];
                                    
  public:
    M_vcenv2(QWidget* parent=0, const char *name=0);
    ~M_vcenv2();

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
