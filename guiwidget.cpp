#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h> 
#include <qsplitter.h>
#include <qlistbox.h> 
#include <qlabel.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "guiwidget.h"
#include "midicontroller.h"
#include "midicontrollerlist.h"
#include "midiguicomponent.h"
#include "module.h"
#include "midislider.h"
#include "intmidislider.h"
#include "floatintmidislider.h"
#include "midicombobox.h"
#include "midicheckbox.h"
#include "midipushbutton.h"

GuiWidget::GuiWidget(SynthData *p_synthdata, QWidget* parent, const char *name) 
                : QVBox(parent, name) {

  setGeometry(0, 0, GUI_DEFAULT_WIDTH, GUI_DEFAULT_HEIGHT);
  setMargin(10);
  setSpacing(5);
  synthdata = p_synthdata;
  QHBox *presetContainer = new QHBox(this);
  QHBox *presetNameContainer = new QHBox(this);
//  frameContainer = new QHBox(this);
  tabWidget = new QTabWidget(this);
  currentTab = NULL;
  currentGroupBox = NULL;
  currentTabIndex = 0;
  new QWidget(presetContainer);
  QPushButton *addPresetButton = new QPushButton("Add Preset", presetContainer);
  new QWidget(presetContainer);
  QObject::connect(addPresetButton, SIGNAL(clicked()), this, SLOT(addPreset()));  
  QPushButton *overwritePresetButton = new QPushButton("OverwritePreset", presetContainer);
  new QWidget(presetContainer);
  QObject::connect(overwritePresetButton, SIGNAL(clicked()), this, SLOT(overwritePreset()));  
  presetCountLabel = new QLabel(presetContainer);
  presetCountLabel->setText("Presets for this configuration: 0");
  new QWidget(presetContainer);
  QPushButton *decButton = new QPushButton("-1", presetContainer);  
  QObject::connect(decButton, SIGNAL(clicked()), this, SLOT(presetDec()));
  QPushButton *incButton = new QPushButton("+1", presetContainer);  
  QObject::connect(incButton, SIGNAL(clicked()), this, SLOT(presetInc()));
  new QWidget(presetNameContainer);
  presetLabel = new QLabel(presetNameContainer);
  new QWidget(presetNameContainer);
  presetName = new QLineEdit(presetNameContainer);
  new QWidget(presetNameContainer);
  presetLabel->setText("Preset 0 : ");
  setPresetCount(0);
  setCurrentPreset(0);
}

GuiWidget::~GuiWidget() {

}

int GuiWidget::addFrame(QString frameName) {
  
//  printf("Adding frame %s.\n", frameName.latin1());
  if (!currentTab) {
    return(-1);
  }
  frameNameList.append(frameName);
  QGroupBox *gbox = new QGroupBox(1, Qt::Horizontal, frameName, currentTab);
  QVBox *vbox = new QVBox(gbox, frameName);
  gbox->show();
  GuiFrame *guiFrame = new GuiFrame;
  guiFrame->frameBox = vbox;
  guiFrame->tabIndex = currentTabIndex;
  frameBoxList.append(guiFrame);
  currentGroupBox = vbox;
  vbox->show();
  new QWidget(currentGroupBox);
  return(0);  
}

int GuiWidget::setFrame(int index) {

//  fprintf(stderr, "Setting frame index %d.\n", index);  
  currentGroupBox = frameBoxList.at(index)->frameBox;
  currentGroupBox->show();
  return(0);  
}

int GuiWidget::addTab(QString tabName) {
  
//  printf("Adding tab %s.\n", tabName.latin1());
  tabNameList.append(tabName);
  currentTab = new QHBox(this, tabName);
  currentTabIndex = tabNameList.count() - 1;
  tabWidget->insertTab(currentTab, tabName);
  currentTab->show();
  tabList.append(currentTab);
  return(0);  
}

int GuiWidget::setTab(int index) {

//  fprintf(stderr, "Setting tab index %d.\n", index);  
  currentTab = tabList.at(index);
  currentTab->show();
  currentTabIndex = index;
  return(0);  
}

int GuiWidget::addParameter(MidiGUIcomponent *midiGUIcomponent, QString parameterName) {

  float minValue, maxValue, value, initial_min, initial_max;
  int l1;
  QString qs;
  bool isLog;
  float min, max, val;

  if (!currentGroupBox) {
    return(-1);
  }
  clearPresets();
//  fprintf(stderr, "addParameter %s\n", parameterName.latin1());
  switch (midiGUIcomponent->componentType) {
    case GUIcomponentType_slider: {
        minValue = ((MidiSlider *)midiGUIcomponent)->min;
        maxValue = ((MidiSlider *)midiGUIcomponent)->max;
        isLog = ((MidiSlider *)midiGUIcomponent)->isLog;
//        fprintf(stderr, "parameterName %s, isLog: %d\n", parameterName.latin1(), isLog);
        value = ((MidiSlider *)midiGUIcomponent)->getValue();
        MidiSlider *slider = new MidiSlider(midiGUIcomponent->parentModule, minValue, maxValue, 0, value,
                                            QSlider::Horizontal, currentGroupBox, parameterName,
                                            midiGUIcomponent->synthdata, ((MidiSlider *)midiGUIcomponent)->valueRef, isLog);
        slider->midiGUIcomponentListIndex = midiGUIcomponent->midiGUIcomponentListIndex;
        slider->isMaster = false;
        parameterList.append((MidiGUIcomponent *)slider);
        QObject::connect(slider, SIGNAL(valueChanged(int)), (MidiSlider *)midiGUIcomponent, SLOT(updateSlider(int)));
        QObject::connect((MidiSlider *)midiGUIcomponent, SIGNAL(valueChanged(int)),
                         slider, SLOT(updateSlider(int)));
//        QObject::connect(slider, SIGNAL(logModeChanged(bool)), (MidiSlider *)midiGUIcomponent, SLOT(setLogMode(bool)));
        QObject::connect((MidiSlider *)midiGUIcomponent, SIGNAL(logModeChanged(bool)),
                         slider, SLOT(setLogMode(bool)));
        slider->show();
      }
      break;
    case GUIcomponentType_intslider: {
        min = ((IntMidiSlider *)midiGUIcomponent)->slider->minValue();
        max = ((IntMidiSlider *)midiGUIcomponent)->slider->maxValue();
        val = ((IntMidiSlider *)midiGUIcomponent)->slider->value();
        IntMidiSlider *slider = new IntMidiSlider(midiGUIcomponent->parentModule, min, max, 0, val,
                                            QSlider::Horizontal, currentGroupBox, parameterName,
                                            midiGUIcomponent->synthdata, ((IntMidiSlider *)midiGUIcomponent)->valueRef);
        slider->midiGUIcomponentListIndex = midiGUIcomponent->midiGUIcomponentListIndex;
        parameterList.append((MidiGUIcomponent *)slider);
        QObject::connect(slider, SIGNAL(valueChanged(int)), (IntMidiSlider *)midiGUIcomponent, SLOT(updateSlider(int)));
        QObject::connect((IntMidiSlider *)midiGUIcomponent, SIGNAL(valueChanged(int)),
                         slider, SLOT(updateSlider(int)));
        slider->show();
      }
      break;
    case GUIcomponentType_floatintslider: {
        min = ((FloatIntMidiSlider *)midiGUIcomponent)->slider->minValue();
        max = ((FloatIntMidiSlider *)midiGUIcomponent)->slider->maxValue();
        val = ((FloatIntMidiSlider *)midiGUIcomponent)->slider->value();
        FloatIntMidiSlider *slider = new FloatIntMidiSlider(midiGUIcomponent->parentModule, min, max, 0, val,
                                            QSlider::Horizontal, currentGroupBox, parameterName,
                                            midiGUIcomponent->synthdata, ((FloatIntMidiSlider *)midiGUIcomponent)->valueRef);
        slider->midiGUIcomponentListIndex = midiGUIcomponent->midiGUIcomponentListIndex;
        parameterList.append((MidiGUIcomponent *)slider);
        QObject::connect(slider, SIGNAL(valueChanged(int)), (FloatIntMidiSlider *)midiGUIcomponent, SLOT(updateSlider(int)));
        QObject::connect((FloatIntMidiSlider *)midiGUIcomponent, SIGNAL(valueChanged(int)),
                         slider, SLOT(updateSlider(int)));
        slider->show();
      }
      break;
    case GUIcomponentType_checkbox: {
      }
      break;
    case GUIcomponentType_pushbutton: {
      }
      break;
    case GUIcomponentType_combobox: {
        QStrList *comboNames = new QStrList(true);
        for (l1 = 0; l1 < ((MidiComboBox *)midiGUIcomponent)->comboBox->count(); l1++) {
          comboNames->append(((MidiComboBox *)midiGUIcomponent)->comboBox->text(l1));
        }
        MidiComboBox *midiComboBox = new MidiComboBox(midiGUIcomponent->parentModule,  ((MidiComboBox *)midiGUIcomponent)->comboBox->currentItem(), 
                                                      currentGroupBox, parameterName, midiGUIcomponent->synthdata, 
                                                      ((MidiComboBox *)midiGUIcomponent)->valueRef, comboNames);
        midiComboBox->midiGUIcomponentListIndex = midiGUIcomponent->midiGUIcomponentListIndex;
        parameterList.append((MidiGUIcomponent *)midiComboBox);
        QObject::connect(midiComboBox->comboBox, SIGNAL(highlighted(int)), (MidiComboBox *)midiGUIcomponent, SLOT(updateValue(int)));
        QObject::connect(((MidiComboBox *)midiGUIcomponent)->comboBox, SIGNAL(highlighted(int)), midiComboBox, SLOT(updateValue(int)));
        midiComboBox->show();
      }
      break;
    default:
      break;
  }
  new QWidget(currentGroupBox);
  currentGroupBox->show();
  return(0);
}

int GuiWidget::setPresetCount(int count) {

  QString qs;

  presetCount = count;
  qs.sprintf("Presets for this configuration: %d", presetCount);
  presetCountLabel->setText(qs);
}

int GuiWidget::setCurrentPreset(int presetNum) {

  QString qs;
  QValueList<int>::iterator it;
  int index, value;

  if (presetCount == 0) {
    currentPreset = 0;
  }
  if ((presetNum < 0) || (presetNum > presetCount - 1)) {
    return(-1);
  }
  currentPreset = presetNum;
  qs.sprintf("Preset %d : ", currentPreset);
  presetLabel->setText(qs);
  index = 0;
  for (it = presetList[currentPreset].begin(); it != presetList[currentPreset].end(); it++) {
    value = *it;
    switch(parameterList.at(index)->componentType) {  
      case GUIcomponentType_slider:
        ((MidiSlider *)parameterList.at(index))->updateValue(value);
        break;
      case GUIcomponentType_intslider:
        ((IntMidiSlider *)parameterList.at(index))->updateValue(value);
        break;
      case GUIcomponentType_floatintslider:
        ((FloatIntMidiSlider *)parameterList.at(index))->updateValue(value);
        break;
      case GUIcomponentType_combobox:
        ((MidiComboBox *)parameterList.at(index))->updateValue(value);
        break;
    }
    index++;
  }
  for (QStringList::Iterator it = presetNameList.begin(); it != presetNameList.end(); it++) {
    qs = (*it).mid(0, 3);
    if (qs.toInt() == currentPreset) {
      qs = (*it).mid(3);
      presetName->setText(qs);
    }
  }
}

void GuiWidget::presetDec() {

  if (currentPreset > 0) {
    setCurrentPreset(currentPreset - 1);
  }
}

void GuiWidget::presetInc() {

  if (currentPreset < presetCount - 1) {
    setCurrentPreset(currentPreset + 1);
  }
}

void GuiWidget::addPreset() {
 
  QString qs;

  if (presetCount) {
    setPresetCount(presetCount + 1);
    setCurrentPreset(presetCount - 1);
  }
  qs.sprintf("%3d", currentPreset);
  presetNameList.append(qs+presetName->text());
  overwritePreset();
}

void GuiWidget::overwritePreset() {

  int l1, value;
  QString qs;

  if (!presetCount) {
    setPresetCount(presetCount + 1);
  }
  presetList[currentPreset].clear();
  for (l1 = 0; l1 < parameterList.count(); l1++) {
    switch(parameterList.at(l1)->componentType) {
      case GUIcomponentType_slider:
        value = ((MidiSlider *)parameterList.at(l1))->slider->value();
        break;
      case GUIcomponentType_intslider:
        value = ((IntMidiSlider *)parameterList.at(l1))->slider->value();
        break;
      case GUIcomponentType_floatintslider:
        value = ((IntMidiSlider *)parameterList.at(l1))->slider->value();
        break;
      case GUIcomponentType_combobox:
        value = ((MidiComboBox *)parameterList.at(l1))->comboBox->currentItem();
        break;
      default:
        value = 0;
    }
    presetList[currentPreset].append(value);
  } 
  for (QStringList::Iterator it = presetNameList.begin(); it != presetNameList.end(); it++) {
    qs = (*it).mid(0, 3);
    if (qs.toInt() == currentPreset) {
      qs.sprintf("%3d", currentPreset);
      *it = qs+presetName->text();
    }
  }
}

void GuiWidget::clearPresets() {

  int l1; 

  for (l1 = 0; l1 < MAX_PRESETS; l1++) {
    presetList[l1].clear();
  }
  presetNameList.clear();
  setPresetCount(0);
}

void GuiWidget::clearGui() {

  delete(tabWidget);
  frameBoxList.clear();
  tabList.clear();
  parameterList.clear();
  presetNameList.clear();
  frameNameList.clear();
  tabNameList.clear();
  tabWidget = new QTabWidget(this);
  tabWidget->show();
  setPresetCount(0);
  setCurrentPreset(0);
}

void GuiWidget::refreshGui() {

  tabWidget->hide();
  tabWidget->show();
}
