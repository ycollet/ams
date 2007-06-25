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
#include <qtimer.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "midislider.h"
#include "midicombobox.h"
#include "midipushbutton.h"
#include "m_scope.h"
#include "module.h"
#include "scopescreen.h"
#include "port.h"


M_scope::M_scope(QWidget* parent) 
  : Module(M_type_scope, 0, parent, "Scope")
{
  QString qs;
  QHBoxLayout *hbox;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_SCOPE_WIDTH, MODULE_SCOPE_HEIGHT);
  gain = 0.5;
  mixer_gain[0] = 0.5;
  mixer_gain[1] = 0.5;
  agc = 0;
  port_in[0] = new Port("In 0", PORT_IN, 0, this);          
  port_in[0]->move(0, 35);
  port_in[0]->outTypeAcceptList.append(outType_audio);
  portList.append(port_in[0]);
  port_in[1] = new Port("In 1", PORT_IN, 1, this);          
  port_in[1]->move(0, 55);
  port_in[1]->outTypeAcceptList.append(outType_audio);
  portList.append(port_in[1]);

  configDialog->initTabWidget();
  mode = 0;
  edge = 0;
  triggerMode = 1;
  triggerThrs = 0;
  zoom = 1;
  timeScale = 100;
  configDialog->setAddStretch(-1);
  QVBoxLayout *scopeTab = configDialog->addVBoxTab("Scope");
  configDialog->setAddStretch(-1);
  //  scopeTab->setMinimumHeight(200);
  configDialog->addScopeScreen(&timeScale, &mode, &edge, &triggerMode, 
                               &triggerThrs, &zoom, scopeTab);

  configDialog->addSlider(10, 1000, timeScale, "Time Scale", &timeScale, false, scopeTab);
  QObject::connect(configDialog->midiSliderList.at(0), SIGNAL(valueChanged(int)),
                   this, SLOT(updateTimeScale(int)));
  configDialog->addSlider(0.1, 10, zoom, "Gain", &zoom, false, scopeTab);
  QObject::connect(configDialog->midiSliderList.at(1), SIGNAL(valueChanged(int)),
                   this, SLOT(updateZoom(int)));
  QVBoxLayout *triggerTab = configDialog->addVBoxTab("Trigger");
  configDialog->setAddStretch(1);
  hbox = configDialog->addHBox(triggerTab);
  configDialog->setAddStretch(0);
  QStringList triggerModeNames;
  triggerModeNames << "Continuous" << "Triggered" << "Single";
  configDialog->addComboBox(triggerMode, "Refresh Mode", &triggerMode, triggerModeNames.count(), &triggerModeNames, hbox);
  QObject::connect(configDialog->midiComboBoxList.at(0)->comboBox, SIGNAL(highlighted(int)),
                   this, SLOT(updateTriggerMode(int)));
  QStringList edgeNames;
  edgeNames << "Rising" << "Falling";
  configDialog->addComboBox(edge, "Trigger Edge", &edge, edgeNames.count(), &edgeNames, hbox);
  QObject::connect(configDialog->midiComboBoxList.at(1)->comboBox, SIGNAL(highlighted(int)),
                   this, SLOT(updateEdge(int)));
  configDialog->setAddStretch(1);
  configDialog->addSlider(-1, 1, triggerThrs, "Trigger Level", &triggerThrs, false, triggerTab);
  QObject::connect(configDialog->midiSliderList.at(2), SIGNAL(valueChanged(int)),
                   this, SLOT(updateTriggerThrs(int)));
  configDialog->addPushButton("Trigger", triggerTab);
  QObject::connect(configDialog->midiPushButtonList.at(0), SIGNAL(clicked()),
                   configDialog->scopeScreenList.at(0), SLOT(singleShot()));

  floatdata = (float *)malloc(2 * synthdata->periodsize * sizeof(float));
  memset(floatdata, 0, 2 * synthdata->periodsize * sizeof(float));
  configDialog->scopeScreenList.at(0)->writeofs = 0;
  timer = new QTimer(this);   
  QObject::connect(timer, SIGNAL(timeout()),
                   this, SLOT(timerProc()));
  timer->setSingleShot(true);
  timer->start((int)(timeScale));
  updateTriggerMode(1);
}

M_scope::~M_scope()
{
  synthdata->scopeModuleList.removeAll(this);
  free(floatdata);
}

int M_scope::setGain(float p_gain) {
  gain = p_gain;
  return(0);
}

float M_scope::getGain() {
  return(gain);
}

void M_scope::generateCycle()
{
  int l1, l2, l3, ofs;
  float max, mixgain, wavgain;
  float *scopedata, **indata;

  wavgain = 32767.0 / synthdata->poly;
  memset(floatdata, 0, 2 * synthdata->cyclesize * sizeof(float));

  for (l1 = 0; l1 < 2; ++l1)
  {
      indata = port_in [l1]->getinputdata ();
      mixgain = gain * mixer_gain[l1];
      for (l2 = 0; l2 < synthdata->cyclesize; ++l2)
      {
          for (l3 = 0; l3 < synthdata->poly; ++l3)
          {
              floatdata[2 * l2 + l1] += mixgain * indata[l3][l2]; 
          }
      }
      if (agc)
      {
          max = 0.0f;
          for (l2 = 0; l2 < synthdata->cyclesize; ++l2)
          {
            if (max < fabs(floatdata[2 * l2 + l1])) max = fabs(floatdata[2 * l2 + l1]);
          }
          if (max > 0.9f)
          {
	      max = 0.9f / max;
              for (l2 = 0; l2 < synthdata->cyclesize; ++l2) floatdata[2 * l2 + l1] *= max;
  	  }
      }
  }

  scopedata = configDialog->scopeScreenList.at(0)->scopedata;
  ofs = configDialog->scopeScreenList.at(0)->writeofs;
  for (l1 = 0; l1 < synthdata->cyclesize; ++l1) {   
    scopedata[2 * ofs] = wavgain * floatdata[2 * l1];
    scopedata[2 * ofs + 1] = wavgain * floatdata[2 * l1 + 1];
    ofs++;
    if (ofs >= SCOPE_BUFSIZE >> 1) {
      ofs -= SCOPE_BUFSIZE >> 1;
    }
  }   
  configDialog->scopeScreenList.at(0)->writeofs = ofs;
}

void M_scope::showConfigDialog() {
}

void M_scope::timerProc() {          
 
  if (triggerMode < 2) {
    timer->setSingleShot(true);
    timer->start((int)(timeScale));
  }
  configDialog->scopeScreenList.at(0)->refreshScope();
}

void M_scope::updateTimeScale(int) {

  configDialog->scopeScreenList.at(0)->setTimeScale(timeScale);
}

void M_scope::updateZoom(int) {

  configDialog->scopeScreenList.at(0)->setZoom(zoom);
}

void M_scope::updateTriggerThrs(int) {

  configDialog->scopeScreenList.at(0)->setTriggerThrs(triggerThrs);
}

void M_scope::updateEdge(int) {

  configDialog->scopeScreenList.at(0)->setEdge((edgeType)edge);
}

void M_scope::updateTriggerMode(int) {

  configDialog->scopeScreenList.at(0)->setTriggerMode((triggerModeType)triggerMode);
  if (triggerMode < 2) {
    timer->setSingleShot(true);
    timer->start((int)(timeScale));
  }
}
