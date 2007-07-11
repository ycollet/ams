#include <stdio.h>
#include <qlabel.h>
#include <QGroupBox>
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

GuiWidget::GuiWidget(QWidget* parent, const char *name) 
  : QWidget(parent)
  , vLayout(this)
{
  setObjectName(name);

  setGeometry(0, 0, GUI_DEFAULT_WIDTH, GUI_DEFAULT_HEIGHT);
  

  QWidget *presetContainer = new QWidget();
  QHBoxLayout *presetContainerLayout = new QHBoxLayout(presetContainer);
  vLayout.addWidget(presetContainer);

  QWidget *presetNameContainer = new QWidget();
  QHBoxLayout *presetNameContainerLayout = new QHBoxLayout(presetNameContainer);
  vLayout.addWidget(presetNameContainer);

  tabWidget = new QTabWidget();
  vLayout.addWidget(tabWidget);
  currentTab = NULL;
  currentGroupBox = NULL;
  currentTabIndex = 0;

  QPushButton *addPresetButton = new QPushButton("Add Preset");
  presetContainerLayout->addWidget(addPresetButton);
  QObject::connect(addPresetButton, SIGNAL(clicked()), this, SLOT(addPreset()));  
  QPushButton *overwritePresetButton =
    new QPushButton("OverwritePreset");
  presetContainerLayout->addWidget(overwritePresetButton);
  QObject::connect(overwritePresetButton, SIGNAL(clicked()),
		   this, SLOT(overwritePreset()));  
  presetCountLabel = new QLabel();
  presetCountLabel->setText("Presets for this configuration: 0");
  presetContainerLayout->addWidget(presetCountLabel);
  QPushButton *decButton = new QPushButton("-1");
  presetContainerLayout->addWidget(decButton);
  QObject::connect(decButton, SIGNAL(clicked()), this, SLOT(presetDec()));
  QPushButton *incButton = new QPushButton("+1");
  presetContainerLayout->addWidget(incButton);
  QObject::connect(incButton, SIGNAL(clicked()), this, SLOT(presetInc()));

  presetLabel = new QLabel();
  presetNameContainerLayout->addWidget(presetLabel);
  presetName = new QLineEdit();
  presetNameContainerLayout->addWidget(presetName);
  presetLabel->setText("Preset 0 : ");

  setPresetCount(0);
  setCurrentPreset(0);
}

GuiWidget::~GuiWidget() {

}

int GuiWidget::addFrame(const QString &frameName)
{
//  printf("Adding frame %s.\n", frameName.latin1());
  if (!currentTab)
    return -1;

  frameNameList.append(frameName);
  QGroupBox *gbox = new QGroupBox(frameName);
  gbox->setObjectName(frameName);
  currentTab->addWidget(gbox);
  QVBoxLayout *vbox = new QVBoxLayout(gbox);

  GuiFrame *guiFrame = new GuiFrame;
  guiFrame->frameBox = vbox;
  guiFrame->tabIndex = currentTabIndex;
  frameBoxList.append(guiFrame);
  currentGroupBox = vbox;
  return(0);  
}

int GuiWidget::setFrame(int index) {

//  fprintf(stderr, "Setting frame index %d.\n", index);  
  currentGroupBox = frameBoxList.at(index)->frameBox;
  return(0);  
}

int GuiWidget::addTab(const QString &tabName) {
  
//  printf("Adding tab %s.\n", tabName.latin1());
  tabNameList.append(tabName);
  QWidget *tab = new QWidget();
  setObjectName(tabName);
  currentTab = new QHBoxLayout(tab);
  currentTabIndex = tabNameList.count() - 1;
  tabWidget->insertTab(-1, tab, tabName);
  tabList.append(currentTab);
  return(0);  
}

int GuiWidget::setTab(int index) {

//   fprintf(stderr, "Setting tab index %d.\n", index);  
  currentTab = tabList.at(index);
  currentTabIndex = index;
  return(0);  
}

int GuiWidget::addParameter(MidiControllableBase *mca, const QString &parameterName) {

//   float minValue, maxValue, value;
//   int l1;
//   QString qs;
//   bool isLog;
//   float min, max, val;

//   if (!currentGroupBox) {
//     return(-1);
//   }
//   clearPresets();
// //  fprintf(stderr, "addParameter %s\n", parameterName.latin1());
//   switch (midiGUIcomponent->componentType) {
//     case GUIcomponentType_slider: {
//         minValue = ((MidiSlider *)midiGUIcomponent)->min;
//         maxValue = ((MidiSlider *)midiGUIcomponent)->max;
//         isLog = ((MidiSlider *)midiGUIcomponent)->isLog;
// //        fprintf(stderr, "parameterName %s, isLog: %d\n", parameterName.latin1(), isLog);
//         value = ((MidiSlider *)midiGUIcomponent)->getValue();
//         MidiSlider *slider = new MidiSlider(midiGUIcomponent->parentModule, minValue, maxValue, 0, value,
//                                             Qt::Horizontal, NULL, parameterName,
//                                             ((MidiSlider *)midiGUIcomponent)->valueRef, isLog);
// 	currentGroupBox->addWidget(slider);
//         slider->midiGUIcomponentListIndex = midiGUIcomponent->midiGUIcomponentListIndex;
//         slider->isMaster = false;
//         parameterList.append((MidiGUIcomponent *)slider);
//         QObject::connect(slider, SIGNAL(valueChanged(int)), (MidiSlider *)midiGUIcomponent, SLOT(updateSlider(int)));
//         QObject::connect((MidiSlider *)midiGUIcomponent, SIGNAL(valueChanged(int)),
//                          slider, SLOT(updateSlider(int)));
// //        QObject::connect(slider, SIGNAL(logModeChanged(bool)), (MidiSlider *)midiGUIcomponent, SLOT(setLogMode(bool)));
//         QObject::connect((MidiSlider *)midiGUIcomponent, SIGNAL(logModeChanged(bool)),
//                          slider, SLOT(setLogMode(bool)));
//         QObject::connect(slider, SIGNAL(sigResetController()), (MidiSlider *)midiGUIcomponent, SLOT(resetControllerOK()));                          
// //         slider->show();
//       }
//       break;
//     case GUIcomponentType_intslider: {
//       IntMidiSlider *master = (IntMidiSlider *)midiGUIcomponent;
//         int min = master->slider->minimum();
//         int max = master->slider->maximum();
//         int val = master->slider->value();
//         IntMidiSlider *slider =
// 	  new IntMidiSlider(midiGUIcomponent->parentModule, min, max, 0, val,
// 			    Qt::Horizontal, NULL, parameterName,
// 			    master->valueRef);
// 	currentGroupBox->addWidget(slider);
//         slider->midiGUIcomponentListIndex = midiGUIcomponent->midiGUIcomponentListIndex;
//         parameterList.append((MidiGUIcomponent *)slider);
//         QObject::connect(slider, SIGNAL(valueChanged(int)), (IntMidiSlider *)midiGUIcomponent, SLOT(updateSlider(int)));
//         QObject::connect((IntMidiSlider *)midiGUIcomponent, SIGNAL(valueChanged(int)),
//                          slider, SLOT(updateSlider(int)));
//         QObject::connect(slider, SIGNAL(sigResetController()), (IntMidiSlider *)midiGUIcomponent, SLOT(resetControllerOK()));                          
// //         slider->show();
//       }
//       break;
//     case GUIcomponentType_floatintslider: {
//         min = ((FloatIntMidiSlider *)midiGUIcomponent)->slider->minimum();
//         max = ((FloatIntMidiSlider *)midiGUIcomponent)->slider->maximum();
//         val = ((FloatIntMidiSlider *)midiGUIcomponent)->slider->value();
//         FloatIntMidiSlider *slider = new FloatIntMidiSlider(midiGUIcomponent->parentModule, min, max, 0, val,
//                                             Qt::Horizontal, NULL, parameterName,
//                                             ((FloatIntMidiSlider *)midiGUIcomponent)->valueRef);
// 	currentGroupBox->addWidget(slider);
//         slider->midiGUIcomponentListIndex = midiGUIcomponent->midiGUIcomponentListIndex;
//         parameterList.append((MidiGUIcomponent *)slider);
//         QObject::connect(slider, SIGNAL(valueChanged(int)), (FloatIntMidiSlider *)midiGUIcomponent, SLOT(updateSlider(int)));
//         QObject::connect((FloatIntMidiSlider *)midiGUIcomponent, SIGNAL(valueChanged(int)),
//                          slider, SLOT(updateSlider(int)));
//         QObject::connect(slider, SIGNAL(sigResetController()), (FloatIntMidiSlider *)midiGUIcomponent, SLOT(resetControllerOK()));                          
// //         slider->show();
//       }
//       break;
//     case GUIcomponentType_checkbox: {
//       }
//       break;
//     case GUIcomponentType_pushbutton: {
//       }
//       break;
//     case GUIcomponentType_combobox: {
//         QStringList *comboNames = new QStringList();
//         for (l1 = 0; l1 < ((MidiComboBox *)midiGUIcomponent)->comboBox->count(); l1++) {
//           *comboNames << ((MidiComboBox *)midiGUIcomponent)->comboBox->itemText(l1);
//         }
//         MidiComboBox *midiComboBox = new MidiComboBox(midiGUIcomponent->parentModule,  ((MidiComboBox *)midiGUIcomponent)->comboBox->currentIndex(), 
//                                                       NULL, parameterName, 
//                                                       ((MidiComboBox *)midiGUIcomponent)->valueRef, comboNames);
// 	currentGroupBox->addWidget(midiComboBox);
//         midiComboBox->midiGUIcomponentListIndex = midiGUIcomponent->midiGUIcomponentListIndex;
//         parameterList.append((MidiGUIcomponent *)midiComboBox);
//         QObject::connect(midiComboBox->comboBox, SIGNAL(highlighted(int)), (MidiComboBox *)midiGUIcomponent, SLOT(updateValue(int)));
//         QObject::connect(((MidiComboBox *)midiGUIcomponent)->comboBox, SIGNAL(highlighted(int)), midiComboBox, SLOT(updateValue(int)));
//         QObject::connect(midiComboBox, SIGNAL(sigResetController()), (MidiComboBox *)midiGUIcomponent, SLOT(resetControllerOK()));                          
// //         midiComboBox->show();
//       }
//       break;
//     default:
//       break;
//   }

//   currentGroupBox->parentWidget()->show();
  return(0);
}

int GuiWidget::setPresetCount(int count) {

  QString qs;

  presetCount = count;
  qs.sprintf("Presets for this configuration: %d", presetCount);
  presetCountLabel->setText(qs);
  return 0;
}

int GuiWidget::setCurrentPreset(int presetNum) {

//   QString qs;
//   int index, value;

//   if (presetCount == 0) {
//     currentPreset = 0;
//   }
//   if ((presetNum < 0) || (presetNum > presetCount - 1)) {
//     return(-1);
//   }
//   currentPreset = presetNum;
//   qs.sprintf("Preset %d : ", currentPreset);
//   presetLabel->setText(qs);
//   index = 0;
//   for (index = 0; index < presetList[currentPreset].count(); index++) {
//     value = presetList[currentPreset][index];
//     parameterList.at(index)->invalidateController();
//     switch(parameterList.at(index)->componentType) {  
//       case GUIcomponentType_slider:
//         ((MidiSlider *)parameterList.at(index))->updateValue(value);
//         break;
//       case GUIcomponentType_intslider:
//         ((IntMidiSlider *)parameterList.at(index))->updateValue(value);
//         break;
//       case GUIcomponentType_floatintslider:
//         ((FloatIntMidiSlider *)parameterList.at(index))->updateValue(value);
//         break;
//       case GUIcomponentType_combobox:
//         ((MidiComboBox *)parameterList.at(index))->updateValue(value);
//     default:
//         break;
//     }
//   }
//   for (QStringList::Iterator it = presetNameList.begin(); it != presetNameList.end(); it++) {
//     qs = (*it).mid(0, 3);
//     if (qs.toInt() == currentPreset) {
//       qs = (*it).mid(3);
//       presetName->setText(qs);
//     }
//   }
//   emit updateMIDIController();
  return 0;
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

// !!  int l1, value;
//   QString qs;

//   if (!presetCount) {
//     setPresetCount(presetCount + 1);
//   }
//   presetList[currentPreset].clear();
//   for (l1 = 0; l1 < parameterList.count(); l1++) {
//     switch(parameterList.at(l1)->componentType) {
//       case GUIcomponentType_slider:
//         value = ((MidiSlider *)parameterList.at(l1))->slider->value();
//         break;
//       case GUIcomponentType_intslider:
//         value = ((IntMidiSlider *)parameterList.at(l1))->slider->value();
//         break;
//       case GUIcomponentType_floatintslider:
//         value = ((IntMidiSlider *)parameterList.at(l1))->slider->value();
//         break;
//       case GUIcomponentType_combobox:
//         value = ((MidiComboBox *)parameterList.at(l1))->comboBox->currentIndex();
//         break;
//       default:
//         value = 0;
//     }
//     presetList[currentPreset].append(value);
//   } 
//   for (QStringList::Iterator it = presetNameList.begin(); it != presetNameList.end(); it++) {
//     qs = (*it).mid(0, 3);
//     if (qs.toInt() == currentPreset) {
//       qs.sprintf("%3d", currentPreset);
//       *it = qs+presetName->text();
//     }
//   }
}

void GuiWidget::clearPresets() {

  int l1; 

  for (l1 = 0; l1 < MAX_PRESETS; l1++) {
    presetList[l1].clear();
  }
  presetNameList.clear();
  presetName->setText(" ");
  setPresetCount(0);
}

void GuiWidget::clearGui() {

  delete tabWidget;
  frameBoxList.clear();
  tabList.clear();
  parameterList.clear();
  presetNameList.clear();
  presetName->setText(" ");
  frameNameList.clear();
  tabNameList.clear();
  tabWidget = new QTabWidget();
  vLayout.addWidget(tabWidget);
  tabWidget->show();
  setPresetCount(0);
  setCurrentPreset(0);
}

void GuiWidget::refreshGui() {

  tabWidget->hide();
  tabWidget->show();
}
