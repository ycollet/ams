#ifndef M_ADVENV_H
#define M_ADVENV_H

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

#define MODULE_ADVENV_WIDTH                120
#define MODULE_ADVENV_HEIGHT               120
#define ADVENVELOPE_RESPONSE               256

class M_advenv : public Module
{
  Q_OBJECT

  private:
    float attack[8], sustain, release[5];
    float e_noteOff[MAXPOLY], de[MAXPOLY], de_release[MAXPOLY];
    Module *in_M_gate, *in_M_retrigger;
    Port *port_gate, *port_retrigger, *port_inverse_out, *port_gain_out;

  public: 
    float e[MAXPOLY];
    float timeScale;
    bool noteActive[MAXPOLY], gate[MAXPOLY], retrigger[MAXPOLY];
    int noteOnOfs[MAXPOLY];
    int noteOffOfs[MAXPOLY];
    float **gateData, **retriggerData;
                
  public:
    M_advenv(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~M_advenv();
    virtual void noteOnEvent(int osc);
    virtual void noteOffEvent(int osc);

  protected:
    virtual void paintEvent(QPaintEvent *ev);
  
  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
