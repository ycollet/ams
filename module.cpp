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
#include "main.h"


Module::Module(int p_outPortCount, QWidget* parent, const char *name, SynthData *p_synthdata, 
           M_typeEnum p_M_type) : QWidget(parent, name) {

  int l1, l2;

  cycleReady = false;
  cycleProcessing = false;
  outPortCount = p_outPortCount;
  synthdata = p_synthdata;
  synthdata->incModuleCount();
  moduleID = synthdata->getModuleID();
  colorBackground = synthdata->colorModuleBackground;
  colorBorder = synthdata->colorModuleBorder;
  colorFont = synthdata->colorModuleFont;
  synthdata->moduleList.append(this);
  setPalette(QPalette(colorBackground, colorBackground));
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_DEFAULT_HEIGHT);
  drag = false;
  configDialog = new ConfigDialog((QObject *)this, NULL, name, p_synthdata);
  QObject::connect(configDialog, SIGNAL(removeModuleClicked()), this, SLOT(removeThisModule()));
  data = (float ***)malloc(outPortCount * sizeof(float **));
  for (l1 = 0; l1 < outPortCount; ++l1) {
    data[l1] = (float **)malloc(synthdata->poly * sizeof(float *));
    for (l2 = 0; l2 < synthdata->poly; ++l2) {                           // TODO Caution, if poly is changed
      data[l1][l2] = (float *)malloc(synthdata->periodsize * sizeof(float));
      memset(data[l1][l2], 0, synthdata->periodsize * sizeof(float));
    }
  }
}

Module::~Module() {

  int l1, l2;
  
  for (l1 = 0; l1 < portList.count(); ++l1) {
    for (l2 = 0; l2 < portList.at(l1)->connectedPortList.count(); ++l2) {
      portList.at(l1)->connectedPortList.at(l2)->connectedPortList.removeRef(portList.at(l1)); 
      portList.at(l1)->connectedPortList.clear();
    }
  }
  delete(configDialog);  
  synthdata->moduleList.removeRef(this);
  synthdata->decModuleCount();
  for (l1 = 0; l1 < outPortCount; ++l1) {
    for (l2 = 0; l2 < synthdata->poly; ++l2) {
      free(data[l1][l2]);
    }
    free(data[l1]);
  }
  free(data);
}

void Module::paintEvent(QPaintEvent *ev)
{
  QPainter p(this);
  QString  qs;

  p.setPen(colorBorder);
  for (int i = 0; i < 4; ++i)
  { 
      p.setPen(colorBorder.light(100 + 15 * i));
      p.drawRect(i, i, width() - 2 * i, height() - 2 * i);
  }
  p.setPen(colorFont);
  p.setFont(QFont("Helvetica", 10));
  p.drawText(10, 20, name ());
  p.setFont(QFont("Helvetica", 8)); 
  qs.sprintf("ID %d", moduleID);
  p.drawText(10, 32, qs);
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

int Module::save(FILE *f) {

  saveConnections(f);
  saveParameters(f);
  saveBindings(f);
  return 0;
}

int Module::saveConnections(FILE *f) {

  Port *port[2];
  int l1;

  for (l1 = 0; l1 < portList.count(); ++l1) {
    port[0] = portList.at(l1);
    if ((port[0]->dir == PORT_IN) && port[0]->connectedPortList.count()) {
      port[1] = port[0]->connectedPortList.at(0);
      fprintf(f, "ColorP %d %d %d %d %d %d %d %d %d %d\n", port[0]->index, port[1]->index,
              ((Module *)port[0]->parentModule)->moduleID, ((Module *)port[1]->parentModule)->moduleID, 
              port[0]->jackColor.red(), port[0]->jackColor.green(), port[0]->jackColor.blue(),
              port[0]->cableColor.red(), port[0]->cableColor.green(), port[0]->cableColor.blue());
    }   
  }    
  return 0; 
}

int Module::saveParameters(FILE *f) {

  int l1, l2, l3;

  for (l1 = 0; l1 < configDialog->midiSliderList.count(); ++l1) {
    fprintf(f, "FSlider %d %d %d %d %d %d %d\n", moduleID, l1,   
                configDialog->midiSliderList.at(l1)->slider->value(),
                configDialog->midiSliderList.at(l1)->isLog,
                configDialog->midiSliderList.at(l1)->slider->minValue(),
                configDialog->midiSliderList.at(l1)->slider->maxValue(),
                configDialog->midiSliderList.at(l1)->midiSign);
  }  
  for (l1 = 0; l1 < configDialog->intMidiSliderList.count(); ++l1) {
    fprintf(f, "ISlider %d %d %d %d\n", moduleID, l1, 
                configDialog->intMidiSliderList.at(l1)->slider->value(),
                configDialog->intMidiSliderList.at(l1)->midiSign);
  }  
  for (l1 = 0; l1 < configDialog->floatIntMidiSliderList.count(); ++l1) {
    fprintf(f, "LSlider %d %d %d %d\n", moduleID, l1, 
                configDialog->floatIntMidiSliderList.at(l1)->slider->value(),
                configDialog->floatIntMidiSliderList.at(l1)->midiSign);
  }  
  for (l1 = 0; l1 < configDialog->midiComboBoxList.count(); ++l1) {
    fprintf(f, "ComboBox %d %d %d %d\n", moduleID, l1, 
                configDialog->midiComboBoxList.at(l1)->comboBox->currentItem(),
                configDialog->midiComboBoxList.at(l1)->midiSign);
  }  
  for (l1 = 0; l1 < configDialog->midiCheckBoxList.count(); ++l1) {
    fprintf(f, "CheckBox %d %d %d %d\n", moduleID, l1, 
                (int)(configDialog->midiCheckBoxList.at(l1)->checkBox->isChecked()),
                configDialog->midiCheckBoxList.at(l1)->midiSign);
  }
  for (l1 = 0; l1 < configDialog->functionList.count(); ++l1) {
    fprintf(f, "Function %d %d %d %d\n", moduleID, l1, 
                configDialog->functionList.at(l1)->functionCount,
                configDialog->functionList.at(l1)->pointCount);
    for (l2 = 0; l2 < configDialog->functionList.at(l1)->functionCount; l2++) {
      for (l3 = 0; l3 < configDialog->functionList.at(l1)->pointCount; l3++) {
        fprintf(f, "Point %d %d %d %d %d %d\n", moduleID, l1, l2, l3,
                    configDialog->functionList.at(l1)->getPoint(l2, l3).x(),
                    configDialog->functionList.at(l1)->getPoint(l2, l3).y());
      }
    }                
  }
  return 0;
}

int Module::saveBindings(FILE *f) {

  int l1, l2;

  for (l1 = 0; l1 < configDialog->midiSliderList.count(); ++l1) {
     for (l2 = 0; l2 < configDialog->midiSliderList.at(l1)->midiControllerList.count(); ++l2) {
       fprintf(f, "FSMIDI %d %d %d %d %d\n", moduleID, l1,
                  configDialog->midiSliderList.at(l1)->midiControllerList.at(l2)->type,
                  configDialog->midiSliderList.at(l1)->midiControllerList.at(l2)->ch,  
                  configDialog->midiSliderList.at(l1)->midiControllerList.at(l2)->param);
     }
  }  
  for (l1 = 0; l1 < configDialog->intMidiSliderList.count(); ++l1) {
    for (l2 = 0; l2 < configDialog->intMidiSliderList.at(l1)->midiControllerList.count(); ++l2) {
      fprintf(f, "ISMIDI %d %d %d %d %d\n", moduleID, l1,
                  configDialog->intMidiSliderList.at(l1)->midiControllerList.at(l2)->type,
                  configDialog->intMidiSliderList.at(l1)->midiControllerList.at(l2)->ch,  
                  configDialog->intMidiSliderList.at(l1)->midiControllerList.at(l2)->param);
    }
  }  
  for (l1 = 0; l1 < configDialog->floatIntMidiSliderList.count(); ++l1) {
    for (l2 = 0; l2 < configDialog->floatIntMidiSliderList.at(l1)->midiControllerList.count(); ++l2) {
      fprintf(f, "LSMIDI %d %d %d %d %d\n", moduleID, l1,
                  configDialog->floatIntMidiSliderList.at(l1)->midiControllerList.at(l2)->type,
                  configDialog->floatIntMidiSliderList.at(l1)->midiControllerList.at(l2)->ch,  
                  configDialog->floatIntMidiSliderList.at(l1)->midiControllerList.at(l2)->param);
    }
  }  
  for (l1 = 0; l1 < configDialog->midiComboBoxList.count(); ++l1) {
    for (l2 = 0; l2 < configDialog->midiComboBoxList.at(l1)->midiControllerList.count(); ++l2) {
      fprintf(f, "CMIDI %d %d %d %d %d\n", moduleID, l1,
                  configDialog->midiComboBoxList.at(l1)->midiControllerList.at(l2)->type,
                  configDialog->midiComboBoxList.at(l1)->midiControllerList.at(l2)->ch,    
                  configDialog->midiComboBoxList.at(l1)->midiControllerList.at(l2)->param);
    }
  }  
  for (l1 = 0; l1 < configDialog->midiCheckBoxList.count(); ++l1) {
    for (l2 = 0; l2 < configDialog->midiCheckBoxList.at(l1)->midiControllerList.count(); ++l2) {
      fprintf(f, "TMIDI %d %d %d %d %d\n", moduleID, l1,
                  configDialog->midiCheckBoxList.at(l1)->midiControllerList.at(l2)->type,
                  configDialog->midiCheckBoxList.at(l1)->midiControllerList.at(l2)->ch,    
                  configDialog->midiCheckBoxList.at(l1)->midiControllerList.at(l2)->param);
    }
  }
  return 0;
}

int Module::load(FILE *f) {

  return 0;
}
