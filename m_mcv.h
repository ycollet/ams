#ifndef M_MCV_H
#define M_MCV_H

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
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "module.h"
#include "port.h"

#define MODULE_MCV_WIDTH                 90
#define MODULE_MCV_HEIGHT               100
#define MODULE_MCV_RESPONSE              32

class M_mcv : public Module
{
  Q_OBJECT

  private:
    float pitchbend;
    Port *port_note_out, *port_gate_out, *port_velocity_out;

  public: 
    int  pitch, channel;
    float freq[MAXPOLY], lastfreq[MAXPOLY];
                
  public:
    M_mcv(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~M_mcv();
    virtual void noteOnEvent(int osc);
    virtual void noteOffEvent(int osc);

  protected:
    virtual void paintEvent(QPaintEvent *ev);
  
  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
