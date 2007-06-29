#ifndef M_MCV_H
#define M_MCV_H

#include "module.h"


#define MODULE_MCV_WIDTH                 90
#define MODULE_MCV_HEIGHT               120

class M_mcv : public Module
{
    float pitchbend;
    Port *port_note_out, *port_gate_out, *port_velocity_out, *port_trig_out;

  public: 
    int  pitch, channel;
    float freq[MAXPOLY];
                
  public:
    M_mcv(QWidget* parent=0);

    void generateCycle();
};
  
#endif
