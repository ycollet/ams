#ifndef ENVELOPE_H
#define ENVELOPE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qlabel.h>
#include <qsizepolicy.h>
#include <qsize.h>
#include "synthdata.h"

#define ENVELOPE_MINIMUM_WIDTH        100
#define ENVELOPE_MINIMUM_HEIGHT        50
#define SUSTAIN_LEN                   0.5

class Envelope : public QWidget
{
  Q_OBJECT

  private:
    SynthData *synthdata;
    float *delayRef, *attackRef, *holdRef, *decayRef, *sustainRef, *releaseRef;

  protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent (QResizeEvent* );            
    
  public:
    Envelope(float *p_delayRef, float *p_attackRef, float *p_holdRef,
             float *p_decayRef, float *p_sustainRef, float *p_releaseRef,
             QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~Envelope();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;
    int setDelay(float p_delay);

  public slots: 
    void updateEnvelope(int value);
};
  
#endif
