#ifndef M_SCMCV_H
#define M_SCMCV_H

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
#include <qfiledialog.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "module.h"
#include "port.h"

#define MODULE_SCMCV_WIDTH                 90
#define MODULE_SCMCV_HEIGHT               120
#define MODULE_SCMCV_RESPONSE              32

class M_scmcv : public Module
{
  Q_OBJECT

  private:
    float pitchbend;
    float scale_lut[128]; 
    bool scale_lut_isRatio[128];
    int scale_lut_length;
    float scale_notes[128];
    Port *port_note_out, *port_gate_out, *port_velocity_out, *port_trig_out;
    QFileDialog *fileDialog;
    
  private:
    void calcScale();   

  public: 
    int base, lastbase, channel;
    QString sclname, dirpath;
    float freq[MAXPOLY], lastfreq[MAXPOLY], trig[MAXPOLY];
                
  public:
    M_scmcv(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0, QString *p_sclname = 0);
    ~M_scmcv();
    virtual void noteOnEvent(int osc);
    virtual void noteOffEvent(int osc);

  public slots:
    void generateCycle();
    void showConfigDialog();
    void loadScale(const QString &p_sclname);
    void openBrowser();
};
  
#endif
