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
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qstringlist.h>
#include <qlineedit.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "midiwidget.h"
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
#include "guiwidget.h"

MidiWidget::MidiWidget(SynthData *p_synthdata, QWidget* parent, const char *name) 
                : QVBox(parent, name) {

  int l1;
  QString qs;

  setMargin(10);
  setSpacing(5);
  synthdata = p_synthdata;
  noteControllerEnabled = false;
  followConfig = false;
  followMidi = false;
  firstBindingMightHaveChanged = false;
  QSplitter *listViewBox = new QSplitter(this);
  setStretchFactor(listViewBox, 2);
  midiControllerListView = new QListView(listViewBox);
  midiControllerListView->setRootIsDecorated(true);
  midiControllerListView->addColumn("MIDI Controller / Parameter");
  midiControllerListView->addColumn("Module");
  midiControllerListView->setAllColumnsShowFocus(true);
  moduleListView = new QListView(listViewBox);
  moduleListView->setRootIsDecorated(true);
  moduleListView->addColumn("Module / Parameter"); 
  moduleListView->addColumn("MIDI Sign"); 
  moduleListView->setAllColumnsShowFocus(true);
  QObject::connect(moduleListView, SIGNAL(selectionChanged()), this, SLOT(guiControlChanged()));
  QObject::connect(midiControllerListView, SIGNAL(selectionChanged()), this, SLOT(midiControlChanged()));
  QVBox *controlFrame = new QVBox(this);
  controlFrame->setSpacing(5);
//  controlFrame->setMargin(5);
  setStretchFactor(controlFrame, 1);
  guiControlParent = new QVBox(controlFrame);
  guiControlParent->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  currentGUIcontrol = NULL;
  QHBox *checkbuttonBox = new QHBox(controlFrame);
  checkbuttonBox->setSpacing(10);
  checkbuttonBox->setMargin(5);
  QStrList *channelNames = new QStrList(true);
  channelNames->append("Omni");
  for (l1 = 1; l1 < 17; l1++) {
    qs.sprintf("%4d", l1);
    channelNames->append(qs);
  }
  QHBox *midiChannelBox = new QHBox(controlFrame);
  QLabel *channelText = new QLabel(midiChannelBox);
  channelText->setText("MIDI Channel:");
  QComboBox *comboBox = new QComboBox(midiChannelBox);
  new QWidget(midiChannelBox);
  new QWidget(midiChannelBox);
  new QWidget(midiChannelBox);
  comboBox->insertStrList(channelNames);
  comboBox->setFixedSize(comboBox->sizeHint());
  QObject::connect(comboBox, SIGNAL(highlighted(int)), this, SLOT(updateMidiChannel(int)));
  new QWidget(midiChannelBox);       
  QPushButton *addGuiButton = new QPushButton("Add to Parameter View", midiChannelBox);
  QObject::connect(addGuiButton, SIGNAL(clicked()), this, SLOT(addToParameterViewClicked()));
  new QWidget(midiChannelBox);       
  QHBox *buttonBox = new QHBox(controlFrame);
  buttonBox->setSpacing(5);
  buttonBox->setMargin(5);
  new QWidget(buttonBox);
  noteCheck = new QCheckBox("Enable note events", checkbuttonBox);
  noteCheck->setChecked(noteControllerEnabled);
  configCheck = new QCheckBox("Follow Configuration Dialog", checkbuttonBox);
  configCheck->setChecked(followConfig);
  midiCheck = new QCheckBox("Follow MIDI", checkbuttonBox);
  midiCheck->setChecked(followMidi);         
  QObject::connect(noteCheck, SIGNAL(clicked()), this, SLOT(noteControllerCheckToggle()));
  QObject::connect(configCheck, SIGNAL(clicked()), this, SLOT(configCheckToggle()));
  QObject::connect(midiCheck, SIGNAL(clicked()), this, SLOT(midiCheckToggle()));
  new QWidget(buttonBox);       
  QPushButton *bindButton = new QPushButton("Bind", buttonBox);
  new QWidget(buttonBox);       
  QPushButton *clearButton = new QPushButton("Clear Binding", buttonBox);
  new QWidget(buttonBox);      
  QPushButton *clearAllButton = new QPushButton("Clear All", buttonBox);
  new QWidget(buttonBox);
  QPushButton *midiSignButton = new QPushButton("Toggle MIDI Sign", buttonBox);
  new QWidget(buttonBox);
  QObject::connect(bindButton, SIGNAL(clicked()), this, SLOT(bindClicked()));
  QObject::connect(clearButton, SIGNAL(clicked()), this, SLOT(clearClicked()));
  QObject::connect(clearAllButton, SIGNAL(clicked()), this, SLOT(clearAllClicked()));
  QObject::connect(midiSignButton, SIGNAL(clicked()), this, SLOT(toggleMidiSign()));
}

MidiWidget::~MidiWidget() {
}

void MidiWidget::clearAllClicked() {

  int l1, l2;  
  Module *module;

  for (l1 = 0; l1 < synthdata->moduleList.count(); l1++) {
    module = (Module *)synthdata->moduleList.at(l1);
    for (l2 = 0; l2 < module->configDialog->midiGUIcomponentList.count(); l2++) {
      module->configDialog->midiGUIcomponentList.at(l2)->listViewItemList.clear();
      module->configDialog->midiGUIcomponentList.at(l2)->midiControllerList.clear();
    }
  }
  midiControllerList.setAutoDelete(true);
  midiControllerList.clear();  
  midiControllerListView->clear();  
}

int MidiWidget::addMidiController(MidiController *midiController) {

  QString qs;

  qs.sprintf("type: %d channel: %d param: %d", midiController->type,
               midiController->ch, midiController->param);
  midiController->setListViewItem(new QListViewItem(midiControllerListView, qs));
  midiControllerList.append(midiController);
  return(0);
}

int MidiWidget::addMidiGuiComponent(MidiController *midiController, QObject *midiGuiComponent) {

  ((MidiGUIcomponent *)midiGuiComponent)->listViewItemList.append(new QListViewItem(midiController->listViewItem, 
                                                                  ((MidiGUIcomponent *)midiGuiComponent)->name(), 
                                                                  ((Module *)((MidiGUIcomponent *)midiGuiComponent)->parentModule)->configDialog->caption()));
  return 0;                                                                  
}

int MidiWidget::deleteMidiGuiComponent(MidiController *midiController, QObject *midiGuiComponent) {

  int l1;
  MidiGUIcomponent *tmp;

  tmp = (MidiGUIcomponent *)midiGuiComponent;
  for (l1 = 0; l1 <  tmp->listViewItemList.count(); l1++) {
    if (midiController->listViewItem == tmp->listViewItemList.at(l1)->parent()) {
      tmp->listViewItemList.at(l1)->parent()->takeItem(tmp->listViewItemList.at(l1));
    }
  }
  return 0;
}

void MidiWidget::clearClicked() {

  int index, l1, l2, l3;
  QListViewItem *tmp;
  Module *module;

  if (!midiControllerListView->selectedItem() 
      || ((QObject *)midiControllerListView->selectedItem()->parent() == (QObject *)midiControllerListView)) {
    return;
  } else {
    for (l1 = 0; l1 < midiControllerList.count(); l1++) {
      if (midiControllerList.at(l1)->listViewItem == midiControllerListView->selectedItem()->parent()) {
        break;
      }
    }
    index = l1;
    for (l1 = 0; l1 < synthdata->moduleList.count(); l1++) {
      module = (Module *)synthdata->moduleList.at(l1);
      for (l2 = 0; l2 < module->configDialog->midiGUIcomponentList.count(); l2++) {
        for (l3 = 0; l3 < module->configDialog->midiGUIcomponentList.at(l2)->listViewItemList.count(); l3++) {
          tmp = module->configDialog->midiGUIcomponentList.at(l2)->listViewItemList.at(l3);
          if (tmp == midiControllerListView->selectedItem()) {
            module->configDialog->midiGUIcomponentList.at(l2)->disconnectController(midiControllerList.at(index));
            deleteMidiGuiComponent(midiControllerList.at(index), module->configDialog->midiGUIcomponentList.at(l2));
          }
        }
      }
    }
  }
  firstBindingMightHaveChanged = true;
}

void MidiWidget::addToParameterViewClicked() {

  QString qs, qs2, qs3;
  bool ok, success, foundFrameName, foundTabName;
  int l1, l2, frameIndex, tabIndex;
  QListViewItem *tmp;
  Module *module;

  if (((GuiWidget *)synthdata->guiWidget)->presetCount > 0) {
    qs.sprintf("This will erase all presets for this configuration. Continue ?");
    QMessageBox questionContinue("AlsaModularSynth", qs, QMessageBox::NoIcon,
                                 QMessageBox::Yes | QMessageBox::Default, QMessageBox::No  | QMessageBox::Escape, QMessageBox::NoButton);
    if (questionContinue.exec() == QMessageBox::No) {
      return;
    }
  }
  qs = QInputDialog::getText("AlsaModularSynth", "Add this parameter to frame:", QLineEdit::Normal, currentFrameName, &ok, this);
  currentFrameName = qs;
  if (qs.isEmpty()) {
    return;
  }
  foundFrameName = false;
  frameIndex = 0;
  if ((l1 =((GuiWidget *)synthdata->guiWidget)->frameNameList.findIndex(qs.stripWhiteSpace())) >= 0) {
    foundFrameName = true;
    frameIndex = l1;
  }
  if (!foundFrameName) {
    qs2.sprintf("Frame %s does not exist. Create ?", qs.latin1()); 
    QMessageBox question("AlsaModularSynth", qs2, QMessageBox::NoIcon, QMessageBox::Yes | QMessageBox::Default,
                         QMessageBox::No  | QMessageBox::Escape, QMessageBox::NoButton);
    if (question.exec() == QMessageBox::Yes) {
      qs3 = QInputDialog::getText("AlsaModularSynth", "Add frame to tab:", QLineEdit::Normal, currentTabName, &ok, this);
      currentTabName = qs3;
      foundTabName = false;
      tabIndex = 0;
      if ((l1 =((GuiWidget *)synthdata->guiWidget)->tabNameList.findIndex(qs3.stripWhiteSpace())) >= 0) {
        foundTabName = true;
        tabIndex = l1;
        ((GuiWidget *)synthdata->guiWidget)->setTab(tabIndex);
      } else {
        qs2.sprintf("Tab %s does not exist. Create ?", qs3.latin1());
        QMessageBox question("AlsaModularSynth", qs2, QMessageBox::NoIcon, QMessageBox::Yes | QMessageBox::Default,
                             QMessageBox::No  | QMessageBox::Escape, QMessageBox::NoButton);
        if (question.exec() == QMessageBox::Yes) {
        printf("Creating tab %s.\n", qs3.latin1());
          ((GuiWidget *)synthdata->guiWidget)->addTab(qs3.stripWhiteSpace());
        } else {
          return;
        }
      }
      printf("Creating frame %s.\n", qs.latin1());
      ((GuiWidget *)synthdata->guiWidget)->addFrame(qs.stripWhiteSpace());
    } else {
      return;
    }
  } else {
    ((GuiWidget *)synthdata->guiWidget)->setFrame(frameIndex);
  }
  success = false;
  for (l1 = 0; l1 < synthdata->moduleList.count(); l1++) {
    module = (Module *)synthdata->moduleList.at(l1);
    for (l2 = 0; l2 < module->configDialog->midiGUIcomponentList.count(); l2++) {
      tmp = module->configDialog->midiGUIcomponentList.at(l2)->listViewItem;
      if (tmp == moduleListView->selectedItem()) {
        success = true;
        break;
      }
    }  
    if (success) {
      break;
    }
  }  
  if (success) {
    qs2.sprintf("%s  ID %d", moduleListView->selectedItem()->text(0).latin1(), module->moduleID);
    qs = QInputDialog::getText("AlsaModularSynth", "Parameter name:", QLineEdit::Normal, qs2, &ok, this);
    ((GuiWidget *)synthdata->guiWidget)->addParameter(module->configDialog->midiGUIcomponentList.at(l2), qs);
  } else {
    return;
  }  
}

void MidiWidget::bindClicked() {

  int index, l1, l2, l3;
  QListViewItem *tmp;
  Module *module;
  bool success;

  if (!midiControllerListView->selectedItem() || !moduleListView->selectedItem()) {
    return;
  } else {
    success = false;
    for (l1 = 0; l1 < midiControllerList.count(); l1++) {
      if (midiControllerList.at(l1)->listViewItem == midiControllerListView->selectedItem()) {
        success = true;
        break;
      }
    }
    if (!success) {
      return;
    }
    index = l1;
    success = false;
    for (l1 = 0; l1 < synthdata->moduleList.count(); l1++) {
      module = (Module *)synthdata->moduleList.at(l1);
      for (l2 = 0; l2 < module->configDialog->midiGUIcomponentList.count(); l2++) {
        tmp = module->configDialog->midiGUIcomponentList.at(l2)->listViewItem;
        if (tmp == moduleListView->selectedItem()) {
          success = true;
          break;
        }
      }
      if (success) {
        break;
      }
    }
    if (success) {
      module->configDialog->midiGUIcomponentList.at(l2)->connectToController(midiControllerList.at(index));
    }
  }
  firstBindingMightHaveChanged = true;
}

void MidiWidget::noteControllerCheckToggle() {

  noteControllerEnabled = noteCheck->isChecked();  
}

void MidiWidget::configCheckToggle() {

  followConfig = configCheck->isChecked();  
}

void MidiWidget::midiCheckToggle() {

  followMidi = midiCheck->isChecked();  
}

MidiController *MidiWidget::getSelectedController() {
  
  int l1, ch, param, type, a, b;
  QString qs, qsm;

  if (!midiControllerListView->selectedItem()) {
    return(NULL);
  } else {
    qs = midiControllerListView->selectedItem()->text(0);
    a = qs.find(":") + 2;
    b = qs.find("channel") - 1;
    qsm = qs.mid(a, b - a);
    type = qsm.toInt();
//    fprintf(stderr, "%s\n", qsm.latin1()); 
    a = qs.find(":", b) + 2;
    b = qs.find("param") - 1;
    qsm = qs.mid(a, b - a);
    ch = qsm.toInt();
//    fprintf(stderr, "%s\n", qsm.latin1()); 
    a = qs.find(":", b) + 2;
    qsm = qs.mid(a);
    param = qsm.toInt();
//    fprintf(stderr, "%s\n", qsm.latin1()); 
    for (l1 = 0; l1 < midiControllerList.count(); l1++) {
      if ((midiControllerList.at(l1)->type == type)
       && (midiControllerList.at(l1)->ch == ch)
       && (midiControllerList.at(l1)->param == param)) {
        break;
      }
    }
    if (l1 < midiControllerList.count()) {
      return(midiControllerList.at(l1));
    } else {
      return(NULL);
    }
  }
}

void MidiWidget::updateModuleList() {

  int l1, l2;
  Module *module;
  QListViewItem *listViewItem, *guiItem;
  QString qs;

  moduleListView->clear();
  for (l1 = 0; l1 < synthdata->moduleList.count(); l1++) {
    module = (Module *)synthdata->moduleList.at(l1);
    listViewItem = new QListViewItem(moduleListView, module->configDialog->caption());
    for (l2 = 0; l2 < module->configDialog->midiGUIcomponentList.count(); l2++) {
       qs = (module->configDialog->midiGUIcomponentList.at(l2)->midiSign == 1) ? "1" : "-1";
       guiItem = new QListViewItem(listViewItem, module->configDialog->midiGUIcomponentList.at(l2)->name(), qs);
       module->configDialog->midiGUIcomponentList.at(l1)->listViewItem = guiItem;
    }
  }
}

void MidiWidget::addModule(Module *m) {

  int l1;
  QListViewItem *listViewItem, *guiItem;
  QString qs;

  if (m->configDialog->midiGUIcomponentList.count()) {
    listViewItem = new QListViewItem(moduleListView, m->configDialog->caption());
    m->listViewItem = listViewItem;
    for (l1 = 0; l1 < m->configDialog->midiGUIcomponentList.count(); l1++) {
      qs = (m->configDialog->midiGUIcomponentList.at(l1)->midiSign == 1) ? "1" : "-1";
      guiItem = new QListViewItem(listViewItem, m->configDialog->midiGUIcomponentList.at(l1)->name(), qs);
      m->configDialog->midiGUIcomponentList.at(l1)->listViewItem = guiItem;
    }  
  } else {
    m->listViewItem = NULL;
  }
}  

void MidiWidget::deleteModule(Module *m) {

  int l1, l2;
  Module *module;
  QListViewItem *listViewItem;
  QListViewItemIterator it(moduleListView);

  if (!m->listViewItem) {
    return;
  }
  if (moduleListView->selectedItem()) {
    if (moduleListView->selectedItem() == m->listViewItem) {
      moduleListView->setSelected(moduleListView->selectedItem(), false);
    }
    if (moduleListView->selectedItem()->parent() == m->listViewItem) {
      moduleListView->setSelected(moduleListView->selectedItem(), false);
    }
  }
  for (l1 = 0; l1 < m->configDialog->midiGUIcomponentList.count(); l1++) {
    for (l2 = 0; l2 < midiControllerList.count(); l2++) {
      deleteMidiGuiComponent(midiControllerList.at(l2), m->configDialog->midiGUIcomponentList.at(l1));
    }
  }
  while (it.current()) {
    if ((it.current()->isSelectable()) && (it.current() == m->listViewItem)) {
      break;
    }
    ++it;
  }
  moduleListView->takeItem(it.current());
  firstBindingMightHaveChanged = true;
}  

void MidiWidget::toggleMidiSign() {

  int midiSign, l1, l2;
  QListViewItem *tmp;
  Module *module;
  bool success;
  QString qs;

  if (!moduleListView->selectedItem()) {
    return;
  } else {
    success = false;
    for (l1 = 0; l1 < synthdata->moduleList.count(); l1++) {
      module = (Module *)synthdata->moduleList.at(l1);
      for (l2 = 0; l2 < module->configDialog->midiGUIcomponentList.count(); l2++) {
        tmp = module->configDialog->midiGUIcomponentList.at(l2)->listViewItem;
        if (tmp == moduleListView->selectedItem()) {
          success = true;
          break;
        }
      }
      if (success) {
        break;
      }
    }
    if (success) {
      midiSign = module->configDialog->midiGUIcomponentList.at(l2)->midiSign;
      module->configDialog->midiGUIcomponentList.at(l2)->midiSign = (midiSign == 1) ? 0 : 1;
      qs = (midiSign == 1 ) ? "-1" : "1";
      moduleListView->selectedItem()->setText(1, qs);
    }
  }
}

void MidiWidget::guiControlChanged() {

  int l1, l2;
  QListViewItem *tmp;
  Module *module;
  bool success;
  QString qs;
  float log_val;
  int min, max, val;

  if (currentGUIcontrol) {
    switch (midiGUIcomponent->componentType) {
      case GUIcomponentType_slider: 
        QObject::disconnect(logCheck, SIGNAL(toggled(bool)), 
                         this, SLOT(setLogMode(bool)));
        QObject::disconnect(slider, SIGNAL(valueChanged(int)), 
                        (MidiSlider *)midiGUIcomponent, SLOT(updateSlider(int)));
        QObject::disconnect((MidiSlider *)midiGUIcomponent, SIGNAL(valueChanged(int)),
                         this, SLOT(updateSliderValue(int)));
        break;
      case GUIcomponentType_intslider:
        QObject::disconnect(slider, SIGNAL(valueChanged(int)), 
                            (IntMidiSlider *)midiGUIcomponent, SLOT(updateSlider(int)));
        QObject::disconnect((IntMidiSlider *)midiGUIcomponent, SIGNAL(valueChanged(int)),
                            this, SLOT(updateIntSliderValue(int)));
        break;
      case GUIcomponentType_floatintslider:
        QObject::disconnect(slider, SIGNAL(valueChanged(int)), 
                            (FloatIntMidiSlider *)midiGUIcomponent, SLOT(updateSlider(int)));
        QObject::disconnect((FloatIntMidiSlider *)midiGUIcomponent, SIGNAL(valueChanged(int)),
                            this, SLOT(updateIntSliderValue(int)));
        break;
      case GUIcomponentType_checkbox:
        QObject::disconnect(currentCheck, SIGNAL(toggled(bool)), 
                            (MidiCheckBox *)midiGUIcomponent, SLOT(updateCheck(bool)));
        QObject::disconnect(((MidiCheckBox *)midiGUIcomponent)->checkBox, SIGNAL(toggled(bool)),
                            this, SLOT(updateCheckBox(bool)));
        break;
      case GUIcomponentType_pushbutton:
         QObject::disconnect(currentPushButton, SIGNAL(clicked()), 
                          (MidiPushButton *)midiGUIcomponent, SLOT(buttonClicked()));
         break;
      case GUIcomponentType_combobox:
        QObject::disconnect(comboBox, SIGNAL(highlighted(int)), 
                            (MidiComboBox *)midiGUIcomponent, SLOT(updateValue(int)));
        QObject::disconnect(((MidiComboBox *)midiGUIcomponent)->comboBox, SIGNAL(highlighted(int)),
                            this, SLOT(updateComboBox(int)));
        break;
      default:
        break;
    }
    delete currentGUIcontrol;
    currentGUIcontrol = NULL;
  }  
  success = false;
  for (l1 = 0; l1 < synthdata->moduleList.count(); l1++) {
    module = (Module *)synthdata->moduleList.at(l1);
    for (l2 = 0; l2 < module->configDialog->midiGUIcomponentList.count(); l2++) {
      tmp = module->configDialog->midiGUIcomponentList.at(l2)->listViewItem;
      if (tmp == moduleListView->selectedItem()) {
        success = true;
        break;
      }
    }  
    if (success) {
      break;
    }
  }  
  if (success) {
    currentGUIcontrol = new QVBox(guiControlParent);
    currentGUIcontrol->setMargin(5);
    midiGUIcomponent = module->configDialog->midiGUIcomponentList.at(l2); 
  } else {
    return;
  }  
  switch (midiGUIcomponent->componentType) {
    case GUIcomponentType_slider: {
        minValue = ((MidiSlider *)midiGUIcomponent)->min;
        maxValue = ((MidiSlider *)midiGUIcomponent)->max;
        initial_min = ((MidiSlider *)midiGUIcomponent)->initial_min;
        initial_max = ((MidiSlider *)midiGUIcomponent)->initial_max;
        value = ((MidiSlider *)midiGUIcomponent)->getValue();
        QHBox *sliderNameBox = new QHBox(currentGUIcontrol);
        sliderNameBox->setMargin(5);
        new QWidget(sliderNameBox);
        QHBox *minMaxLogButtonsBox = new QHBox(currentGUIcontrol);
        minMaxLogButtonsBox->setMargin(5);
        logCheck = new QCheckBox("Log", minMaxLogButtonsBox);
        QObject::connect(logCheck, SIGNAL(toggled(bool)), 
                         this, SLOT(setLogMode(bool)));
        new QWidget(minMaxLogButtonsBox);
        QPushButton *newMinButton = new QPushButton("Set Min", minMaxLogButtonsBox);
        QObject::connect(newMinButton, SIGNAL(clicked()), 
                         this, SLOT(setNewMin()));
        new QWidget(minMaxLogButtonsBox);
        QPushButton *newMaxButton = new QPushButton("Set Max", minMaxLogButtonsBox);
        QObject::connect(newMaxButton, SIGNAL(clicked()), 
                         this, SLOT(setNewMax()));
        new QWidget(minMaxLogButtonsBox);
        QPushButton *resetMinMaxButton = new QPushButton("Reset Min/Max", minMaxLogButtonsBox);
        QObject::connect(resetMinMaxButton, SIGNAL(clicked()), 
                         this, SLOT(setInitialMinMax()));
        QLabel *nameLabel = new QLabel(sliderNameBox);
        new QWidget(sliderNameBox);
        QHBox *sliderLabels = new QHBox(currentGUIcontrol);
        qs.sprintf("Min: %7.3f", minValue); 
        minLabel = new QLabel(qs, sliderLabels);
        new QWidget(sliderLabels);
        qs.sprintf(" %7.3f ", value);
        valueLabel = new QLabel(qs, sliderLabels);
        nameLabel->setText(moduleListView->selectedItem()->text(0));
        new QWidget(sliderLabels);
        qs.sprintf("Max: %7.3f", maxValue);
        maxLabel = new QLabel(qs, sliderLabels);
        nameLabel->setFixedHeight(nameLabel->sizeHint().height());
        valueLabel->setFixedHeight(valueLabel->sizeHint().height());
        minLabel->setFixedHeight(minLabel->sizeHint().height());
        maxLabel->setFixedHeight(maxLabel->sizeHint().height());
        slider = new QSlider(int(SLIDER_SCALE * minValue), int(SLIDER_SCALE * maxValue), 0, 
                             int(SLIDER_SCALE * value), QSlider::Horizontal, currentGUIcontrol);  
        slider->setFixedHeight(slider->sizeHint().height());
        QObject::connect(slider, SIGNAL(valueChanged(int)), 
                        (MidiSlider *)midiGUIcomponent, SLOT(updateSlider(int)));
        QObject::connect((MidiSlider *)midiGUIcomponent, SIGNAL(valueChanged(int)),
                         this, SLOT(updateSliderValue(int)));
        setLogCheck(((MidiSlider *)midiGUIcomponent)->isLog);
        }
      break;
    case GUIcomponentType_intslider: {
        min = ((IntMidiSlider *)midiGUIcomponent)->slider->minValue();
        max = ((IntMidiSlider *)midiGUIcomponent)->slider->maxValue();
        val = ((IntMidiSlider *)midiGUIcomponent)->slider->value();
        QHBox *sliderNameBox = new QHBox(currentGUIcontrol);
        sliderNameBox->setMargin(5);
        new QWidget(sliderNameBox);
        QLabel *nameLabel = new QLabel(sliderNameBox);
        new QWidget(sliderNameBox);
        QHBox *sliderLabels = new QHBox(currentGUIcontrol);
        qs.sprintf("Min: %d", min); 
        minLabel = new QLabel(qs, sliderLabels);
        new QWidget(sliderLabels);
        qs.sprintf(" %d ", val);
        valueLabel = new QLabel(qs, sliderLabels);
        nameLabel->setText(moduleListView->selectedItem()->text(0));
        new QWidget(sliderLabels);
        qs.sprintf("Max: %d", max);
        maxLabel = new QLabel(qs, sliderLabels);
        nameLabel->setFixedHeight(nameLabel->sizeHint().height());
        valueLabel->setFixedHeight(valueLabel->sizeHint().height());
        minLabel->setFixedHeight(minLabel->sizeHint().height());
        maxLabel->setFixedHeight(maxLabel->sizeHint().height());
        slider = new QSlider(min, max, 0, val, QSlider::Horizontal, currentGUIcontrol);  
        slider->setTickInterval((abs(max) + abs(min)) / 10);
        slider->setTickmarks(QSlider::Below);
        slider->setFixedHeight(slider->sizeHint().height());
        QObject::connect(slider, SIGNAL(valueChanged(int)), 
                         (IntMidiSlider *)midiGUIcomponent, SLOT(updateSlider(int)));
        QObject::connect((IntMidiSlider *)midiGUIcomponent, SIGNAL(valueChanged(int)),
                         this, SLOT(updateIntSliderValue(int)));
        }
      break;
    case GUIcomponentType_floatintslider: {
        min = ((FloatIntMidiSlider *)midiGUIcomponent)->slider->minValue();
        max = ((FloatIntMidiSlider *)midiGUIcomponent)->slider->maxValue();
        val = ((FloatIntMidiSlider *)midiGUIcomponent)->slider->value();
        QHBox *sliderNameBox = new QHBox(currentGUIcontrol);
        sliderNameBox->setMargin(5);
        new QWidget(sliderNameBox);
        QLabel *nameLabel = new QLabel(sliderNameBox);
        new QWidget(sliderNameBox);
        QHBox *sliderLabels = new QHBox(currentGUIcontrol);
        qs.sprintf("Min: %d", min); 
        minLabel = new QLabel(qs, sliderLabels);
        new QWidget(sliderLabels);
        qs.sprintf(" %d ", val);
        valueLabel = new QLabel(qs, sliderLabels);
        nameLabel->setText(moduleListView->selectedItem()->text(0));
        new QWidget(sliderLabels);
        qs.sprintf("Max: %d", max);
        maxLabel = new QLabel(qs, sliderLabels);
        nameLabel->setFixedHeight(nameLabel->sizeHint().height());
        valueLabel->setFixedHeight(valueLabel->sizeHint().height());
        minLabel->setFixedHeight(minLabel->sizeHint().height());
        maxLabel->setFixedHeight(maxLabel->sizeHint().height());
        slider = new QSlider(min, max, 0, val, QSlider::Horizontal, currentGUIcontrol);  
        slider->setTickInterval((abs(max) + abs(min)) / 10);
        slider->setTickmarks(QSlider::Below);
        slider->setFixedHeight(slider->sizeHint().height());
        QObject::connect(slider, SIGNAL(valueChanged(int)), 
                         (FloatIntMidiSlider *)midiGUIcomponent, SLOT(updateSlider(int)));
        QObject::connect((FloatIntMidiSlider *)midiGUIcomponent, SIGNAL(valueChanged(int)),
                         this, SLOT(updateIntSliderValue(int)));
        }
      break;
    case GUIcomponentType_checkbox: {
        QHBox *checkNameBox = new QHBox(currentGUIcontrol);
        new QWidget(checkNameBox);
        checkNameBox->setMargin(5);
        QLabel *nameLabel = new QLabel(checkNameBox);
        new QWidget(checkNameBox);
        nameLabel->setText(moduleListView->selectedItem()->text(0));
        nameLabel->setFixedHeight(nameLabel->sizeHint().height());
        QHBox *checkHBox = new QHBox(currentGUIcontrol);
        new QWidget(checkHBox);
        currentCheck = new QCheckBox(checkHBox);
        new QWidget(checkHBox);
        currentCheck->setChecked(((MidiCheckBox *)midiGUIcomponent)->checkBox->isChecked());
        currentCheck->setFixedSize(currentCheck->sizeHint());
        QObject::connect(currentCheck, SIGNAL(toggled(bool)), 
                         (MidiCheckBox *)midiGUIcomponent, SLOT(updateCheck(bool)));
        QObject::connect(((MidiCheckBox *)midiGUIcomponent)->checkBox, SIGNAL(toggled(bool)),
                         this, SLOT(updateCheckBox(bool)));
        }
      break;
    case GUIcomponentType_pushbutton: {
         QHBox *pushbuttonBox = new QHBox(currentGUIcontrol);
         new QWidget(pushbuttonBox);
         currentPushButton = new QPushButton(midiGUIcomponent->name(), pushbuttonBox);
         new QWidget(pushbuttonBox);
         QObject::connect(currentPushButton, SIGNAL(clicked()), 
                          (MidiPushButton *)midiGUIcomponent, SLOT(buttonClicked()));
        }
      break;
    case GUIcomponentType_combobox: {
        QHBox *comboNameBox = new QHBox(currentGUIcontrol);
        new QWidget(comboNameBox);
        comboNameBox->setMargin(5);
        QLabel *nameLabel = new QLabel(comboNameBox);
        new QWidget(comboNameBox);
        nameLabel->setText(moduleListView->selectedItem()->text(0));
        nameLabel->setFixedHeight(nameLabel->sizeHint().height());
        QHBox *comboHBox = new QHBox(currentGUIcontrol);
        new QWidget(comboHBox);
        comboBox = new QComboBox(comboHBox);
        new QWidget(comboHBox);
        for (l1 = 0; l1 < ((MidiComboBox *)midiGUIcomponent)->comboBox->count(); l1++) {
          comboBox->insertItem(((MidiComboBox *)midiGUIcomponent)->comboBox->text(l1));
        }
        comboBox->setCurrentItem(((MidiComboBox *)midiGUIcomponent)->comboBox->currentItem());
        comboBox->setFixedSize(comboBox->sizeHint());
        QObject::connect(comboBox, SIGNAL(highlighted(int)), 
                         (MidiComboBox *)midiGUIcomponent, SLOT(updateValue(int)));
        QObject::connect(((MidiComboBox *)midiGUIcomponent)->comboBox, SIGNAL(highlighted(int)),
                         this, SLOT(updateComboBox(int)));
        }
      break;
    default:
      break;
  }
  currentGUIcontrol->show();
  firstBindingMightHaveChanged = true;
}

void MidiWidget::midiControlChanged() {
  
  firstBindingMightHaveChanged = true;
}

void MidiWidget::updateSliderValue(int p_value) {

  QString qs;
  float log_val;
  
  if (logCheck->isChecked()) {
    value = exp((float)p_value / SLIDER_SCALE);
  } else {
    value = (float)p_value / SLIDER_SCALE;
  }
  qs.sprintf(" %7.3f ", value);
  valueLabel->setText(qs);
  slider->setValue(p_value);
}

void MidiWidget::updateIntSliderValue(int p_value) {

  QString qs;
  
  qs.sprintf(" %d ", p_value);
  valueLabel->setText(qs);
  slider->setValue(p_value);
}

void MidiWidget::setLogCheck(bool on) {

  logCheck->setChecked(on);
  setLogMode(on);
}

void MidiWidget::setLogMode(bool on) {
   
  float log_min, log_val, val;

  val = value;
  ((MidiSlider *)midiGUIcomponent)->setLogMode(on);
  if (on) {
    log_min = (minValue > 1e-4) ? minValue : 1e-4;
    log_val = (val > 1e-4) ? val : 1e-4;
    slider->setMinValue(int(SLIDER_SCALE * log(log_min)));
    ((MidiSlider *)midiGUIcomponent)->slider->setMinValue(int(SLIDER_SCALE * log(log_min)));
    slider->setMaxValue(int(SLIDER_SCALE * log(maxValue)));
    ((MidiSlider *)midiGUIcomponent)->slider->setMaxValue(int(SLIDER_SCALE * log(maxValue)));
    slider->setValue(int(SLIDER_SCALE * log(log_val)));
  } else {
    slider->setMinValue(int(SLIDER_SCALE * minValue));
    ((MidiSlider *)midiGUIcomponent)->slider->setMinValue(int(SLIDER_SCALE * minValue));
    slider->setMaxValue(int(SLIDER_SCALE * maxValue));  
    ((MidiSlider *)midiGUIcomponent)->slider->setMaxValue(int(SLIDER_SCALE * maxValue));
    slider->setValue(int(SLIDER_SCALE * val));   
  }
}

void MidiWidget::setNewMin() {
  
  QString qs;

  minValue = value;
  ((MidiSlider *)midiGUIcomponent)->min = value;
  qs.sprintf("Min: %7.3f", minValue);
  minLabel->setText(qs);
  ((MidiSlider *)midiGUIcomponent)->minLabel->setText(qs);
  if (logCheck->isChecked()) {
    slider->setMinValue(int(SLIDER_SCALE * log(minValue)));
    ((MidiSlider *)midiGUIcomponent)->slider->setMinValue(int(SLIDER_SCALE * log(minValue)));
    slider->setValue(int(SLIDER_SCALE * log(value)));
  } else {
    slider->setMinValue(int(SLIDER_SCALE * minValue));
    ((MidiSlider *)midiGUIcomponent)->slider->setMinValue(int(SLIDER_SCALE * minValue));
    slider->setValue(int(SLIDER_SCALE * value));
  }
  midiGUIcomponent->controllerOK = false;
}

void MidiWidget::setNewMax() {        
 
  QString qs;

  maxValue = value;
  ((MidiSlider *)midiGUIcomponent)->max = value;
  qs.sprintf("Min: %7.3f", maxValue); 
  maxLabel->setText(qs);
  ((MidiSlider *)midiGUIcomponent)->maxLabel->setText(qs);
  if (logCheck->isChecked()) {
    slider->setMaxValue(int(SLIDER_SCALE * log(maxValue)));
    ((MidiSlider *)midiGUIcomponent)->slider->setMaxValue(int(SLIDER_SCALE * log(maxValue)));
    slider->setValue(int(SLIDER_SCALE * log(value)));
  } else {
    slider->setMaxValue(int(SLIDER_SCALE * maxValue));
    ((MidiSlider *)midiGUIcomponent)->slider->setMaxValue(int(SLIDER_SCALE * maxValue));
    slider->setValue(int(SLIDER_SCALE * value));
  }
  midiGUIcomponent->controllerOK = false;
}  

void MidiWidget::setInitialMinMax() {

  QString qs;

  logCheck->setChecked(false);
  minValue = initial_min;
  ((MidiSlider *)midiGUIcomponent)->min = initial_min;
  maxValue = initial_max;
  ((MidiSlider *)midiGUIcomponent)->max = initial_max;
  qs.sprintf("Min: %7.3f", minValue);
  minLabel->setText(qs);
  ((MidiSlider *)midiGUIcomponent)->minLabel->setText(qs);
  slider->setMinValue(int(SLIDER_SCALE * minValue));
  ((MidiSlider *)midiGUIcomponent)->slider->setMinValue(int(SLIDER_SCALE * minValue));
  qs.sprintf("Min: %7.3f", maxValue); 
  maxLabel->setText(qs);
  ((MidiSlider *)midiGUIcomponent)->maxLabel->setText(qs);
  slider->setMaxValue(int(SLIDER_SCALE * maxValue));
  ((MidiSlider *)midiGUIcomponent)->slider->setMaxValue(int(SLIDER_SCALE * maxValue));
  slider->setValue(int(SLIDER_SCALE * value));
  midiGUIcomponent->controllerOK = false;
}  

void MidiWidget::updateComboBox(int p_value) {

  comboBox->setCurrentItem(p_value);  
}

void MidiWidget::updateCheckBox(bool on) {

  currentCheck->setChecked(on);  
}

int MidiWidget::setSelectedController(MidiController *midiController) {

  if (midiControllerListView->currentItem() != midiController->listViewItem) {
    midiControllerListView->setSelected(midiController->listViewItem, true);
    midiControllerListView->ensureItemVisible(midiController->listViewItem);
    findFirstBinding(midiController);
  } else {
    if (firstBindingMightHaveChanged) {
      findFirstBinding(midiController);
      firstBindingMightHaveChanged = false; 
    }
  }
  return 0;
}

int MidiWidget::findFirstBinding(MidiController *midiController) {
  
  int l1, l2, l3;
  QListViewItem *tmp;
  Module *module;

  for (l1 = 0; l1 < synthdata->moduleList.count(); l1++) {
    module = (Module *)synthdata->moduleList.at(l1);
    for (l2 = 0; l2 < module->configDialog->midiGUIcomponentList.count(); l2++) {
      if (module->configDialog->midiGUIcomponentList.at(l2)->midiControllerList.contains(midiController)) {
        tmp = module->configDialog->midiGUIcomponentList.at(l2)->listViewItem;
        moduleListView->setSelected(tmp, true);
        moduleListView->ensureItemVisible(tmp);
        return(0);
      }
    }
  }
  return 0;
}

void MidiWidget::updateGuiComponent() {

  MidiGUIcomponent *guiComponent;

  if (followConfig) {
    guiComponent = (MidiGUIcomponent *)sender();
    if (guiComponent->listViewItem) {
      if (moduleListView->currentItem() != guiComponent->listViewItem) {
        moduleListView->setSelected(guiComponent->listViewItem, true);
        moduleListView->ensureItemVisible(guiComponent->listViewItem);
      }
    }
  }
}

void MidiWidget::updateMidiChannel(int index) {

  synthdata->midiChannel = index - 1;
}
