#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qstring.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <QPaintEvent>
#include <QMouseEvent>
#include "midicombobox.h"
#include "midicheckbox.h"
#include "modularsynth.h"
#include "synthdata.h"
#include "configdialog.h"
#include "intmidislider.h"
#include "midicontrollable.h"
#include "midislider.h"
#include "module.h"
#include "main.h"

int Module::portmemAllocated;

Module::Module(M_typeEnum M_type, int outPortCount, QWidget* parent, const QString &name)
  : Box(parent, name)
  , connections(0)
  , M_type(M_type)
  , outPortCount(outPortCount)
{
  cycleReady = false;
  
  synthdata->incModuleCount();
  moduleID = synthdata->getModuleID();
  getColors();
//   colorBackground = synthdata->colorModuleBackground;
//   colorBorder = synthdata->colorModuleBorder;
//   colorFont = synthdata->colorModuleFont;
  synthdata->moduleList.append(this);
//   setPalette(QPalette(colorBackground, colorBackground));
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_DEFAULT_WIDTH, MODULE_DEFAULT_HEIGHT);

  configDialog = new ConfigDialog(*this);
  configDialog->setWindowTitle(name + " ID " + QString::number(moduleID));
  QObject::connect(configDialog, SIGNAL(removeModuleClicked()), this, SLOT(removeThisModule()));
  portMemAlloc(outPortCount);
}

void Module::portMemAlloc(int outPortCount)
{
  this->outPortCount = outPortCount;
  data = (float ***)malloc(outPortCount * sizeof(float **));
  for (int l1 = 0; l1 < outPortCount; ++l1) {
    data[l1] = (float **)malloc(synthdata->poly * sizeof(float *));
    for (int l2 = 0; l2 < synthdata->poly; ++l2) {                           // TODO Caution, if poly is changed
      data[l1][l2] = (float *)malloc(synthdata->periodsize * sizeof(float));
      portmemAllocated += synthdata->periodsize * sizeof(float);
      memset(data[l1][l2], 0, synthdata->periodsize * sizeof(float));
    }
  }
}

Module::~Module()
{
  int l1, l2;

  delete configDialog;  

  synthdata->midiWidget->removeModule(this);

  qDeleteAll(midiControllables.begin(), midiControllables.end());

  for (l1 = 0; l1 < portList.count(); ++l1) {
    Port *port = portList.at(l1);
    QList<Port *> &connected = port->connectedPortList;
    for (l2 = 0; l2 < connected.count(); ++l2)
      connected.at(l2)->connectedPortList.removeAll(port); 

    connected.clear();
  }

  for (l1 = 0; l1 < outPortCount; ++l1) {
    for (l2 = 0; l2 < synthdata->poly; ++l2) {
      free(data[l1][l2]);
      portmemAllocated -= synthdata->periodsize * sizeof(float);
    }
    free(data[l1]);
  }
  free(data);
}

void Module::paintEvent(QPaintEvent *)
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
  p.setFont(synthdata->bigFont);
  p.drawText(10, 20, objectName());
  p.setFont(synthdata->smallFont); 
  qs.sprintf("ID %d", moduleID);
  p.drawText(10, 32, qs);
}

void Module::mousePressEvent(QMouseEvent *ev)
{
  Box::mousePressEvent(ev);
  switch (ev->button()) {
  case Qt::RightButton:
    configDialog->show();
    configDialog->raise();
  default:
    break;
  }
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
              port[0]->module->moduleID, port[1]->module->moduleID, 
              port[0]->jackColor.red(), port[0]->jackColor.green(), port[0]->jackColor.blue(),
              port[0]->cableColor.red(), port[0]->cableColor.green(), port[0]->cableColor.blue());
    }   
  }    
  return 0; 
}

int Module::saveParameters(FILE *f)
{
  int l1, l2, l3;

  for (l1 = 0; l1 < configDialog->midiSliderList.count(); ++l1) {
    MidiControllableFloat &mcAbleF = dynamic_cast<MidiControllableFloat &>(configDialog->midiSliderList.at(l1)->mcAble);
    fprintf(f, "FSlider %d %d %d %d %d %d %d\n", moduleID, l1,
	    mcAbleF.sliderVal(),
	    mcAbleF.isLog,
	    mcAbleF.sliderMin(),
	    mcAbleF.sliderMax(),
	    mcAbleF.midiSign);
  }  
  for (l1 = 0; l1 < configDialog->intMidiSliderList.count(); ++l1)
    fprintf(f, "ISlider %d %d %d %d\n", moduleID, l1, 
	    configDialog->intMidiSliderList.at(l1)->mcAble.sliderVal(),
	    configDialog->intMidiSliderList.at(l1)->mcAble.midiSign);

  for (l1 = 0; l1 < configDialog->floatIntMidiSliderList.count(); ++l1)
    fprintf(f, "LSlider %d %d %d %d\n", moduleID, l1, 
	    configDialog->floatIntMidiSliderList.at(l1)->mcAble.sliderVal(),
	    configDialog->floatIntMidiSliderList.at(l1)->mcAble.midiSign);

  for (l1 = 0; l1 < configDialog->midiComboBoxList.count(); ++l1)
    fprintf(f, "ComboBox %d %d %d %d\n", moduleID, l1, 
	    configDialog->midiComboBoxList.at(l1)->mcAble.getValue(),
	    configDialog->midiComboBoxList.at(l1)->mcAble.midiSign);

  for (l1 = 0; l1 < configDialog->midiCheckBoxList.count(); ++l1)
    fprintf(f, "CheckBox %d %d %d %d\n", moduleID, l1, 
	    configDialog->midiCheckBoxList.at(l1)->mcAble.getValue(),
	    configDialog->midiCheckBoxList.at(l1)->mcAble.midiSign);

  for (l1 = 0; l1 < configDialog->functionList.count(); ++l1) {
    fprintf(f, "Function %d %d %d %d\n", moduleID, l1, 
	    configDialog->functionList.at(l1)->functionCount,
	    configDialog->functionList.at(l1)->pointCount);
    for (l2 = 0; l2 < configDialog->functionList.at(l1)->functionCount; l2++)
      for (l3 = 0; l3 < configDialog->functionList.at(l1)->pointCount; l3++) {
        fprintf(f, "Point %d %d %d %d %d %d\n", moduleID, l1, l2, l3,
		configDialog->functionList.at(l1)->getPoint(l2, l3).x(),
		configDialog->functionList.at(l1)->getPoint(l2, l3).y());
      }
       
  }
  return 0;
}

int Module::saveBindings(FILE *f)
{
  int l1, l2;

  for (l1 = 0; l1 < configDialog->midiSliderList.count(); ++l1) {
     for (l2 = 0; l2 < configDialog->midiSliderList.at(l1)->mcAble.midiControllerList.count(); ++l2) {
       fprintf(f, "FSMIDI %d %d %d %d %d\n", moduleID, l1,
	       configDialog->midiSliderList.at(l1)->mcAble.midiControllerList.at(l2).type(),
	       configDialog->midiSliderList.at(l1)->mcAble.midiControllerList.at(l2).ch(),  
	       configDialog->midiSliderList.at(l1)->mcAble.midiControllerList.at(l2).param());
     }
  }  
  for (l1 = 0; l1 < configDialog->intMidiSliderList.count(); ++l1) {
    for (l2 = 0; l2 < configDialog->intMidiSliderList.at(l1)->mcAble.midiControllerList.count(); ++l2) {
      fprintf(f, "ISMIDI %d %d %d %d %d\n", moduleID, l1,
	      configDialog->intMidiSliderList.at(l1)->mcAble.midiControllerList.at(l2).type(),
	      configDialog->intMidiSliderList.at(l1)->mcAble.midiControllerList.at(l2).ch(),  
	      configDialog->intMidiSliderList.at(l1)->mcAble.midiControllerList.at(l2).param());
    }
  }  
  for (l1 = 0; l1 < configDialog->floatIntMidiSliderList.count(); ++l1) {
    for (l2 = 0; l2 < configDialog->floatIntMidiSliderList.at(l1)->mcAble.midiControllerList.count(); ++l2) {
      fprintf(f, "LSMIDI %d %d %d %d %d\n", moduleID, l1,
	      configDialog->floatIntMidiSliderList.at(l1)->mcAble.midiControllerList.at(l2).type(),
	      configDialog->floatIntMidiSliderList.at(l1)->mcAble.midiControllerList.at(l2).ch(),  
	      configDialog->floatIntMidiSliderList.at(l1)->mcAble.midiControllerList.at(l2).param());
    }
  }  
  for (l1 = 0; l1 < configDialog->midiComboBoxList.count(); ++l1) {
    for (l2 = 0; l2 < configDialog->midiComboBoxList.at(l1)->mcAble.midiControllerList.count(); ++l2) {
      fprintf(f, "CMIDI %d %d %d %d %d\n", moduleID, l1,
	      configDialog->midiComboBoxList.at(l1)->mcAble.midiControllerList.at(l2).type(),
	      configDialog->midiComboBoxList.at(l1)->mcAble.midiControllerList.at(l2).ch(),    
	      configDialog->midiComboBoxList.at(l1)->mcAble.midiControllerList.at(l2).param());
    }
  }  
  for (l1 = 0; l1 < configDialog->midiCheckBoxList.count(); ++l1) {
    for (l2 = 0; l2 < configDialog->midiCheckBoxList.at(l1)->mcAble.midiControllerList.count(); ++l2) {
      fprintf(f, "TMIDI %d %d %d %d %d\n", moduleID, l1,
	      configDialog->midiCheckBoxList.at(l1)->mcAble.midiControllerList.at(l2).type(),
	      configDialog->midiCheckBoxList.at(l1)->mcAble.midiControllerList.at(l2).ch(),    
	      configDialog->midiCheckBoxList.at(l1)->mcAble.midiControllerList.at(l2).param());
    }
  }
  return 0;
}

int Module::load(FILE *)
{
  return 0;
}

void Module::getColors(void)
{
  QColor alphaBack(synthdata->colorModuleBackground);
  alphaBack.setAlpha(203);
  setPalette(QPalette(alphaBack, alphaBack));
  colorBorder = synthdata->colorModuleBorder;
  colorFont = synthdata->colorModuleFont;
}

void Module::incConnections()
{
  if (connections++ == 0) {
    configDialog->removeButtonShow(false);
    synthdata->midiWidget->setActiveMidiControllers();
  }
}

void Module::decConnections()
{
  if (--connections == 0) {
    synthdata->midiWidget->setActiveMidiControllers();
    configDialog->removeButtonShow(true);
  }
}
