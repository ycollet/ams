#ifndef M_CVS_H
#define M_CVS_H

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

#define MODULE_CVS_WIDTH                 90
#define MODULE_CVS_HEIGHT               120
#define MODULE_CVS_CV_COUNT               4

class M_cvs : public Module
{
  Q_OBJECT

  private:
    Port *port_cv_out[MODULE_CVS_CV_COUNT];
    //float cv[MODULE_CVS_CV_COUNT], cv_fine[MODULE_CVS_CV_COUNT];
    FloatParameter *cv[MODULE_CVS_CV_COUNT], *cv_fine[MODULE_CVS_CV_COUNT];

  public:
    M_cvs(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~M_cvs();

  protected:
    virtual void paintEvent(QPaintEvent *ev);

  public slots:
    void generateCycle();
    void showConfigDialog();
};

#endif
