#ifndef M_MIDIOUT_H
#define M_MIDIOUT_H

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
#include <qtimer.h>
#include <qdialog.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "module.h"
#include "port.h"

#define MODULE_MIDIOUT_WIDTH                 90
#define MODULE_MIDIOUT_HEIGHT               100

class M_midiout : public Module
{
  Q_OBJECT

  private:
    float mixer_gain[2], triggerLevel; 
    int midiMode, offset[2], controller[2], channel;
    Port *port_in[2], *port_M_trigger;
    bool trigger[MAXPOLY];    
    int triggeredNote[2][MAXPOLY], lastmididata[2][MAXPOLY];
    
  public: 
    float **inData[2], **triggerData;
                            
  public:
    M_midiout(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~M_midiout();

  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
