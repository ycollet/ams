#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qstring.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "configdialog.h"
#include "module.h"

Module::Module(int p_outPortCount, QWidget* parent, const char *name, SynthData *p_synthdata, 
           M_typeEnum p_M_type) : QWidget(parent, name) {

  int l1, l2;

  cycleReady = false;
  cycleProcessing = false;
  outPortCount = p_outPortCount;
  synthdata = p_synthdata;
  synthdata->incModuleCount();
  moduleID = synthdata->getModuleID();
  synthdata->moduleList.append(this);
  setPalette(QPalette(QColor(77, 70, 64), QColor(77, 70, 64)));
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_DEFAULT_HEIGHT);
  drag = false;
  configDialog = new ConfigDialog((QObject *)this, NULL, name);
  QObject::connect(configDialog, SIGNAL(removeModuleClicked()), this, SLOT(removeThisModule()));
  data = (float ***)malloc(outPortCount * sizeof(float **));
  lastdata = (float ***)malloc(outPortCount * sizeof(float **));
  for (l1 = 0; l1 < outPortCount; l1++) {
    data[l1] = (float **)malloc(synthdata->poly * sizeof(float *));
    lastdata[l1] = (float **)malloc(synthdata->poly * sizeof(float *));
    for (l2 = 0; l2 < synthdata->poly; l2++) {                           // TODO Caution, if poly is changed
      data[l1][l2] = (float *)malloc(synthdata->periodsize * sizeof(float));
      memset(data[l1][l2], 0, synthdata->periodsize * sizeof(float));
      lastdata[l1][l2] = (float *)malloc(synthdata->periodsize * sizeof(float));
      memset(lastdata[l1][l2], 0, synthdata->periodsize * sizeof(float));
    }
  }
}

Module::~Module() {

  int l1, l2;
  
  for (l1 = 0; l1 < portList.count(); l1++) {
    for (l2 = 0; l2 < portList.at(l1)->connectedPortList.count(); l2++) {
      portList.at(l1)->connectedPortList.at(l2)->connectedPortList.removeRef(portList.at(l1)); 
      portList.at(l1)->connectedPortList.clear();
    }
  }
  delete(configDialog);  
  synthdata->moduleList.removeRef(this);
  synthdata->decModuleCount();
  for (l1 = 0; l1 < outPortCount; l1++) {
    for (l2 = 0; l2 < synthdata->poly; l2++) {
      free(data[l1][l2]);
      free(lastdata[l1][l2]);
    }
    free(data[l1]);
    free(lastdata[l1]);
  }
  free(data);
  free(lastdata);
}

void Module::paintEvent(QPaintEvent *ev) {
  
  QPainter p(this);
  int l1;

  for (l1 = 0; l1 < 4; l1++) { 
    p.setPen(QColor(10 + 10*l1, 10 + 10*l1, 10 * l1));
    p.drawRect(l1, l1, width()-2*l1, height()-2*l1);
  }
}

void Module::noteOnEvent(int osc) {
}

void Module::noteOffEvent(int osc) {
}

void Module::mousePressEvent(QMouseEvent *ev) {
  
  switch (ev->button()) {
  case Qt::LeftButton:   
    raise();
    drag = true;
    mousePressPos = ev->pos();
    break;
  case Qt::RightButton:
    break;
  case Qt::MidButton:
    break;
  default:
    break;
  }
}  

void Module::mouseReleaseEvent(QMouseEvent *ev) {
  
  switch (ev->button()) {
  case Qt::LeftButton:   
    drag = false;
    break;
  case Qt::RightButton:
    configDialog->show();
    configDialog->raise();
    break;
  case Qt::MidButton:
    break;
  default:
    break;
  }
}  
   
void Module::mouseMoveEvent(QMouseEvent *ev) {

  if (drag) {
    emit dragged(ev->pos());
  }
}  
            
void Module::generateCycle() {
}

void Module::showConfigDialog() {
}

QPoint Module::getMousePressPos() {

  return(mousePressPos);
}

void Module::removeThisModule() {

  emit removeModule();
}
