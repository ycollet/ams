#ifndef ENVELOPE_H
#define ENVELOPE_H

#include <qwidget.h>
#include <qsizepolicy.h>
#include <qsize.h>
#include <QPaintEvent>
#include "synthdata.h"

#define ENVELOPE_MINIMUM_WIDTH        100
#define ENVELOPE_MINIMUM_HEIGHT        50
#define SUSTAIN_LEN                   0.5

class Envelope : public QWidget
{
  Q_OBJECT

  private:
    float *delayRef, *attackRef, *holdRef, *decayRef, *sustainRef, *releaseRef;

  protected:
    virtual void paintEvent(QPaintEvent *);
    
  public:
    Envelope(float *p_delayRef, float *p_attackRef, float *p_holdRef,
             float *p_decayRef, float *p_sustainRef, float *p_releaseRef,
             QWidget* parent=0, const char *name=0);
    ~Envelope() {};
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;
    int setDelay(float p_delay);

  public slots: 
    void updateEnvelope(int value);
};
  
#endif
