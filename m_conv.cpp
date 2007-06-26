#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlabel.h>


#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qpainter.h>
//Added by qt3to4:

#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "m_conv.h"
#include "port.h"

M_conv::M_conv(QWidget* parent) 
  : Module(M_type_conv, 1, parent, "Converter")
{
  QString qs;

  convMode = 0;
  octave = 0;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_CONV_WIDTH, MODULE_CONV_HEIGHT);
  port_M_in = new Port("In", PORT_IN, 0, this); 
  port_M_in->move(0, 35);
  port_M_in->outTypeAcceptList.append(outType_audio);
  portList.append(port_M_in);
  port_out = new Port("Out", PORT_OUT, 0, this);          
  port_out->move(width() - port_out->width(), 55);
  port_out->outType = outType_audio;
  portList.append(port_out);

  QStringList *convModeNames = new QStringList();
  *convModeNames << "V/Octave --> Hz";
  *convModeNames << "V/Octave --> 0..1, 1=rate/2";
  *convModeNames << "V/Octave --> 0..1, 1=20000 Hz";
  configDialog->addComboBox(0, "Conversion Mode ", &convMode, convModeNames->count(), convModeNames);
  configDialog->addIntSlider(-3, 3, octave, "Octave Offset", &octave);
}

void M_conv::generateCycle() {

  int l1, l2;
  float offset;

  if (!cycleReady) {
    cycleProcessing = true;

    inData = port_M_in->getinputdata();

    offset = octave + 5.0313842;
    if (convMode == 0) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
          data[0][l1][l2] = exp(M_LN2 * (offset + inData[l1][l2]));
        }
      }
    } else if (convMode == 1) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
          data[0][l1][l2] = 2.0 * exp(M_LN2 * (offset + inData[l1][l2])) / synthdata->rate;
        }
      }
    } else if (convMode == 2) {
      for (l1 = 0; l1 < synthdata->poly; l1++) {
        for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
          data[0][l1][l2] = exp(M_LN2 * (offset + inData[l1][l2])) / 20000.0;
        }
      }
    } 
  }
  cycleProcessing = false;
  cycleReady = true;
}

