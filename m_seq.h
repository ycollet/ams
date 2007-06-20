#ifndef M_SEQ_H
#define M_SEQ_H

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
#include <qtimer.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "module.h"
#include "port.h"

#define MODULE_SEQ_WIDTH                 90
#define MODULE_SEQ_HEIGHT               140
#define MODULE_SEQ_MAX_LEN               32

class M_seq : public Module
{
  Q_OBJECT

  private:
    Port *port_trigger, *port_trigger_out, *port_note_out, *port_gate_out, *port_velocity_out;
    float seq_gate, seq_freq, seq_velocity;
    int seq_pos, tick, osc, note_len, triggerCount;
    QTimer *timer; 

  public: 
    int pitch[MODULE_SEQ_MAX_LEN], velocity[MODULE_SEQ_MAX_LEN];
    int bpm, pitch_ofs, seqLen;
    float gate[MODULE_SEQ_MAX_LEN];
    bool updateTimerFlag, trigger, triggerOut;
    float **triggerData;
                
  public:
    M_seq(int p_seqLen, QWidget* parent=0, const char *name=0);
    ~M_seq();

  public slots:
    void generateCycle();
    void showConfigDialog();
    void nextStep();
    void updateTimer(int p_bpm);
};
  
#endif
