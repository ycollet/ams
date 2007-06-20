#ifndef MULTI_ENVELOPE_H
#define MULTI_ENVELOPE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qsizepolicy.h>
#include <qsize.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <QResizeEvent>
#include "synthdata.h"

#define MULTI_ENVELOPE_MINIMUM_WIDTH        100
#define MULTI_ENVELOPE_MINIMUM_HEIGHT        50
#define SUSTAIN_LEN                         0.5

class MultiEnvelope : public QWidget
{
  Q_OBJECT

    int envCount;
    float *timeScaleRef, *attackRef, *sustainRef, *releaseRef;
    QColor colorTable[8];
    
  protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent (QResizeEvent* );            
    
  public:
    MultiEnvelope(int p_envCount, float *p_timeScaleRef, float *p_attackRef, float *p_sustainRef, float *p_releaseRef,
             QWidget* parent=0, const char *name=0);
    ~MultiEnvelope();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;

  public slots: 
    void updateMultiEnvelope(int value);
};
  
#endif
