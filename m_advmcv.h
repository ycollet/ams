#ifndef M_ADVMCV_H
#define M_ADVMCV_H

#include "module.h"


#define MODULE_ADVMCV_WIDTH                140
#define MODULE_ADVMCV_HEIGHT               240
#define MODULE_ADVMCV_CONTROLLER_PORTS       4

class M_advmcv : public Module
{
    float pitchbend;
    Port *port_note_out, *port_gate_out, *port_velocity_out, *port_trig_out;
    Port *port_aftertouch_out, *port_pitchbend_out, *port_controller_out[MODULE_ADVMCV_CONTROLLER_PORTS];

  public: 
    int  pitch, channel, controller_num[MODULE_ADVMCV_CONTROLLER_PORTS];
    float freq[MAXPOLY], trig[MAXPOLY], aftertouch_cv[MAXPOLY], pitchbend_cv[MAXPOLY];
    float controller_cv[MODULE_ADVMCV_CONTROLLER_PORTS][MAXPOLY];
                
  public:
    M_advmcv(QWidget* parent=0);
    ~M_advmcv();
    virtual void noteOnEvent(int osc);

    void aftertouchEvent(int channel, int value, int note = 0);
    void controllerEvent(int channel, int controlNum, int value, bool is14bit =false);
    void pitchbendEvent(int channel, int value);
    
    void generateCycle();
};
  
#endif
