#ifndef CAPTURE_H
#define CAPTURE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qobject.h>
#include <qthread.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"

#define CAPTURE_GAIN 32767.0 // TODO This should depend on bits per sample

class Capture : public QThread
{
  private:
    SynthData *synthdata;
    unsigned char *periodBuf;
  
  public:
    Capture(SynthData *p_synthdata);
    ~Capture();
    virtual void run();                    
};
  
#endif
