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
#include <qstrlist.h>
#include <qscrollview.h>
#include <qsizepolicy.h>
#include <qlineedit.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "configdialog.h"
#include "midislider.h"
#include "intmidislider.h"
#include "midicombobox.h"
#include "envelope.h"
#include "multi_envelope.h"
#include "scopescreen.h"
#include "spectrumscreen.h"

ConfigDialog::ConfigDialog(QObject *p_parentModule, QWidget* parent, const char *name, SynthData *p_synthdata) 
                : QVBox(parent, name) {

  parentModule = p_parentModule;
//  scroll = new QScrollView(this);
  tabWidget = NULL;
//  configBox = new QVBox(scroll->viewport());
  configBox = new QVBox(this);
  configBox->setMinimumWidth(430);
  configBox->setMargin(5);
  configBox->setSpacing(5);
//  scroll->setMinimumWidth(460);
//  scroll->addChild(configBox);
//  scroll->setResizePolicy(QScrollView::AutoOneFit);
  QHBox *removeFrame = new QHBox(this);
  removeFrame->setMargin(10);
  new QWidget(removeFrame);
  removeButton = new QPushButton("Remove Module", removeFrame);
  new QWidget(removeFrame);
  QObject::connect(removeButton, SIGNAL(clicked()), this, SLOT(removeButtonClicked()));
  synthdata = p_synthdata;
}

ConfigDialog::~ConfigDialog() {
}

int ConfigDialog::addSlider(float minValue, float maxValue, float value, const char * name, float *valueRef, bool isLog, QWidget *parent) {

  MidiSlider *midiSlider;
  
  if (!parent) {
    midiSlider = new MidiSlider(parentModule, minValue, maxValue, 0, value, 
                                          QSlider::Horizontal, configBox, name, synthdata, valueRef, isLog);
  } else {
    midiSlider = new MidiSlider(parentModule, minValue, maxValue, 0, value, 
                                          QSlider::Horizontal, parent, name, synthdata, valueRef, isLog);
  }
  midiSliderList.append(midiSlider);
  midiGUIcomponentList.append((MidiGUIcomponent *)midiSlider);
  return(0);
}

int ConfigDialog::addIntSlider(int minValue, int maxValue, int value, const char * name, int *valueRef, QWidget *parent) {
 
  IntMidiSlider *intMidiSlider;

  if (!parent) {
    intMidiSlider = new IntMidiSlider(parentModule, minValue, maxValue, 0, value, 
                                            QSlider::Horizontal, configBox, name, synthdata, valueRef);
  } else {
    intMidiSlider = new IntMidiSlider(parentModule, minValue, maxValue, 0, value, 
                                              QSlider::Horizontal, parent, name, synthdata, valueRef);
  }
  intMidiSliderList.append(intMidiSlider);
  midiGUIcomponentList.append((MidiGUIcomponent *)intMidiSlider);
  return(0);
}

int ConfigDialog::addComboBox(int value, const char * name, int *valueRef, int itemCount, QStrList *itemNames, QWidget *parent) {
 
  MidiComboBox *midiComboBox;

  if (!parent) {
    midiComboBox = new MidiComboBox(parentModule, value, configBox, name, synthdata, valueRef, itemNames);
  } else {
    midiComboBox = new MidiComboBox(parentModule, value, parent, name, synthdata, valueRef, itemNames);
  }
  midiComboBoxList.append(midiComboBox);
  midiGUIcomponentList.append((MidiGUIcomponent *)midiComboBox);
  return(0);
}

int ConfigDialog::addCheckBox(float value, const char * name, float *valueRef, QWidget *parent) {
  
  MidiCheckBox *midiCheckBox;

  if (!parent) {
    midiCheckBox = new MidiCheckBox(parentModule, value, configBox, name, synthdata, valueRef);
  } else {
    midiCheckBox = new MidiCheckBox(parentModule, value, parent, name, synthdata, valueRef);
  }
  midiCheckBoxList.append(midiCheckBox);
  midiGUIcomponentList.append((MidiGUIcomponent *)midiCheckBox);
  return(0);
}

int ConfigDialog::addPushButton(const char * name, QWidget *parent) {
  
  MidiPushButton *midiPushButton;

  if (!parent) {
    midiPushButton = new MidiPushButton(parentModule, configBox, name, synthdata);
  } else {
    midiPushButton = new MidiPushButton(parentModule, parent, name, synthdata);
  }
  midiPushButtonList.append(midiPushButton);
  midiGUIcomponentList.append((MidiGUIcomponent *)midiPushButton);
  return(0);
}

int ConfigDialog::addEnvelope(float *delayRef, float *attackRef, float *holdRef, 
                               float *decayRef, float *sustainRef, float *releaseRef, QWidget *parent) {

  Envelope *envelope;

  if (!parent) {
    envelope = new Envelope(delayRef, attackRef, holdRef, decayRef, sustainRef, releaseRef, 
                                    configBox, "Envelope", synthdata);
  } else {
    envelope = new Envelope(delayRef, attackRef, holdRef, decayRef, sustainRef, releaseRef, 
                                    parent, "Envelope", synthdata);
  }
  envelopeList.append(envelope);
  return(0);
}

int ConfigDialog::addMultiEnvelope(int envCount, float *timeScaleRef, float *attackRef, float *sustainRef, float *releaseRef, QWidget *parent) {

  MultiEnvelope *envelope;

  if (!parent) {
    envelope = new MultiEnvelope(envCount, timeScaleRef, attackRef, sustainRef, releaseRef, 
                                    configBox, "Multi Envelope", synthdata);
  } else {
    envelope = new MultiEnvelope(envCount, timeScaleRef, attackRef, sustainRef, releaseRef, 
                                    parent, "Multi Envelope", synthdata);
  }
  multiEnvelopeList.append(envelope);
  return(0);
}

int ConfigDialog::addLabel(QString label, QWidget *parent) {

  QLabel *configLabel;

  if (!parent) {
    configLabel = new QLabel(configBox);
  } else {
    configLabel = new QLabel(parent); 
  }
  configLabel->setAlignment(Qt::WordBreak | Qt::AlignLeft);
  configLabel->setText(label);
  labelList.append(configLabel);
}

void ConfigDialog::removeButtonClicked() {

  emit removeModuleClicked();
}

int ConfigDialog::addTab(QWidget *tabPage, QString tabLabel) {

  tabWidget->insertTab(tabPage, tabLabel);
}

int ConfigDialog::initTabWidget() {

  tabWidget = new QTabWidget(configBox);
}

QHBox *ConfigDialog::addHBox(QWidget *parent) {

  QHBox *hbox;

  if (!parent) {
    hbox = new QHBox(configBox);
  } else {  
    hbox = new QHBox(parent);
  }
  hboxList.append(hbox);   
  return(hbox);
}

QVBox *ConfigDialog::addVBox(QWidget *parent) {

  QVBox *vbox;

  if (!parent) {
    vbox = new QVBox(configBox);
  } else {  
    vbox = new QVBox(parent);
  }
  vboxList.append(vbox);   
  return(vbox);
}

int ConfigDialog::addLineEdit(QString lineName, QWidget *parent) {

  QLineEdit *lineEdit;
  QHBox *line;

  if (!parent) {
    line = new QHBox(configBox);
  } else {
    line = new QHBox(parent);
  }
  line->setSpacing(5);
  line->setMargin(10);
  QLabel *nameLabel = new QLabel(line);
  nameLabel->setText(lineName);
  lineEdit = new QLineEdit(line);
  lineEditList.append(lineEdit);
}

int ConfigDialog::addScopeScreen(float *timeScaleRef, int *modeRef, int *edgeRef, int *triggerModeRef, 
                                 float *triggerThrsRef, float *zoomRef, QWidget *parent) {

  ScopeScreen *scopeScreen;

  if (!parent) {
    scopeScreen = new ScopeScreen(configBox, "Scope", synthdata);
  } else {
    scopeScreen = new ScopeScreen(parent, "Scope", synthdata);
  }
  scopeScreenList.append(scopeScreen);
  return(0);
}

int ConfigDialog::addSpectrumScreen(QWidget *parent) {

  SpectrumScreen *spectrumScreen;

  if (!parent) {
    spectrumScreen = new SpectrumScreen(configBox, "Spectrum", synthdata);
  } else {
    spectrumScreen = new SpectrumScreen(parent, "Spectrum", synthdata);
  }
  spectrumScreenList.append(spectrumScreen);
  return(0);
}
