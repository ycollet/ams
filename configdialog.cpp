#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlabel.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qsizepolicy.h>
#include <qlineedit.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "configdialog.h"
#include "midislider.h"
#include "intmidislider.h"
#include "floatintmidislider.h"
#include "midicheckbox.h"
#include "midicombobox.h"
#include "midipushbutton.h"
#include "envelope.h"
#include "multi_envelope.h"
#include "scopescreen.h"
#include "spectrumscreen.h"
#include "function.h"

ConfigDialog::ConfigDialog(Module *p_parentModule, QWidget* parent, const char *name) 
  : QWidget(parent)
  , addStretch(1)
  , removeButton(new QPushButton("Remove Module"))
  , removeFrame(new QHBoxLayout())
{
  setObjectName(name);
  QVBoxLayout *vBox = new QVBoxLayout();
  setLayout(vBox);
  vBox->setMargin(0);
  vBox->setSpacing(0);
  configBox = new QVBoxLayout();
  configBox->setMargin(5);
  configBox->setSpacing(5);
  vBox->addLayout(configBox);

  parentModule = p_parentModule;

  tabWidget = NULL;
  removeFrame->addStretch();
  removeFrame->addWidget(removeButton);
  removeFrame->addStretch();
  vBox->addLayout(removeFrame);
  removeButtonShow(true);
  QObject::connect(removeButton, SIGNAL(clicked()), this, SLOT(removeButtonClicked()));
}

ConfigDialog::~ConfigDialog() {
  MidiGUIcomponent *mgc;
  for (int i = 0; i < midiGUIcomponentList.count(); ++i)
    delete midiGUIcomponentList.at(i);
}

int ConfigDialog::addSlider(float minValue, float maxValue, float value, const QString &name, float *valueRef, bool isLog, QBoxLayout *layout) {

  MidiSlider *midiSlider;
  
  midiSlider = new MidiSlider(parentModule, minValue, maxValue, 0, value, 
                              Qt::Horizontal, NULL, name, valueRef, isLog);
  insertWidget(layout, midiSlider);

  midiSliderList.append(midiSlider);
  midiSlider->midiGUIcomponentListIndex = midiGUIcomponentList.count();
  midiGUIcomponentList.append((MidiGUIcomponent *)midiSlider);
  return(0);
}

int ConfigDialog::addFloatIntSlider(float minValue, float maxValue, float value, const QString &name, float *valueRef, QBoxLayout *layout) {

  FloatIntMidiSlider *floatIntMidiSlider;

  floatIntMidiSlider = new FloatIntMidiSlider(parentModule, minValue, maxValue, 0, value, 
					      Qt::Horizontal, NULL, name, valueRef);
  insertWidget(layout, floatIntMidiSlider);
  floatIntMidiSliderList.append(floatIntMidiSlider);
  floatIntMidiSlider->midiGUIcomponentListIndex = midiGUIcomponentList.count();
  midiGUIcomponentList.append((MidiGUIcomponent *)floatIntMidiSlider);
  return(0);
}

int ConfigDialog::addIntSlider(int minValue, int maxValue, int value, const QString &name, int *valueRef, QBoxLayout *layout) {
 
  IntMidiSlider *intMidiSlider;

  intMidiSlider = new IntMidiSlider(parentModule, minValue, maxValue, 0, value, 
				    Qt::Horizontal, NULL, name, valueRef);
  insertWidget(layout, intMidiSlider);
  intMidiSliderList.append(intMidiSlider);
  intMidiSlider->midiGUIcomponentListIndex = midiGUIcomponentList.count();
  midiGUIcomponentList.append((MidiGUIcomponent *)intMidiSlider);
  return(0);
}

int ConfigDialog::addComboBox(int value, const QString &name, int *valueRef, int /*itemCount*/, QStringList *itemNames, QBoxLayout *layout) {
 
  MidiComboBox *midiComboBox;

  midiComboBox = new MidiComboBox(parentModule, value, NULL, name, valueRef, itemNames);
  insertWidget(layout, midiComboBox, 0, Qt::AlignCenter);

  midiComboBoxList.append(midiComboBox);
  midiComboBox->midiGUIcomponentListIndex = midiGUIcomponentList.count();
  midiGUIcomponentList.append((MidiGUIcomponent *)midiComboBox);
  return(0);
}

int ConfigDialog::addCheckBox(float value, const QString &name, float *valueRef, QBoxLayout *layout) {
  
  MidiCheckBox *midiCheckBox;

  midiCheckBox = new MidiCheckBox(parentModule, value, NULL, name, valueRef);
  insertWidget(layout, midiCheckBox, 0, Qt::AlignCenter);

  midiCheckBoxList.append(midiCheckBox);
  midiCheckBox->midiGUIcomponentListIndex = midiGUIcomponentList.count();
  midiGUIcomponentList.append((MidiGUIcomponent *)midiCheckBox);
  return(0);
}

int ConfigDialog::addPushButton(const QString &name, QBoxLayout *layout) {
  
  MidiPushButton *midiPushButton;

  midiPushButton = new MidiPushButton(parentModule, name);
  insertWidget(layout, midiPushButton, 0, Qt::AlignCenter);

  midiPushButtonList.append(midiPushButton);
  midiPushButton->midiGUIcomponentListIndex = midiGUIcomponentList.count();
  midiGUIcomponentList.append((MidiGUIcomponent *)midiPushButton);
  return(0);
}

int ConfigDialog::addEnvelope(float *delayRef, float *attackRef, float *holdRef, 
			      float *decayRef, float *sustainRef, float *releaseRef, QBoxLayout *layout) {

  Envelope *envelope;

  envelope = new Envelope(delayRef, attackRef, holdRef, decayRef, sustainRef, releaseRef, 
			  NULL, "Envelope");
  insertWidget(layout, envelope, 100);

  envelopeList.append(envelope);
  return(0);
}

int ConfigDialog::addMultiEnvelope(int envCount, float *timeScaleRef, float *attackRef, float *sustainRef, float *releaseRef, QBoxLayout *layout) {

  MultiEnvelope *envelope;

  envelope = new MultiEnvelope(envCount, timeScaleRef, attackRef, sustainRef, releaseRef, 
			       NULL, "Multi Envelope");
  insertWidget(layout, envelope, 100);

  multiEnvelopeList.append(envelope);
  return(0);
}

int ConfigDialog::addLabel(QString label, QBoxLayout *layout) {

  QLabel *configLabel;

  configLabel = new QLabel();
  insertWidget(layout, configLabel);

  configLabel->setAlignment(/*Qt::WordBreak |*/ Qt::AlignLeft);
  configLabel->setText(label);
  labelList.append(configLabel);
  return(0);
}

void ConfigDialog::removeButtonClicked() {

  emit removeModuleClicked();
}

int ConfigDialog::initTabWidget() {

  tabWidget = new QTabWidget();
  configBox->insertWidget(configBox->count() - 1, tabWidget);
  return 0;
}

QHBoxLayout *ConfigDialog::addHBox(QBoxLayout *layout)
{
  QHBoxLayout *hbox;

  if (!layout)
    layout = configBox;

  hbox = new QHBoxLayout();
  layout->addLayout(hbox);
  if (addStretch > 0)
    layout->addStretch(addStretch);

  return hbox;
}

QVBoxLayout *ConfigDialog::addVBox(QBoxLayout *layout)
{
  QVBoxLayout *vbox;

  if (!layout)
    layout = configBox;

  vbox = new QVBoxLayout();
  layout->addLayout(vbox);
  if (addStretch > 0)
    layout->addStretch(addStretch);

  return vbox;
}

QVBoxLayout *ConfigDialog::addVBoxTab(const char *tabLabel)
{
  return addVBoxTab(QString(tabLabel));
}


QVBoxLayout *ConfigDialog::addVBoxTab(const QString &tabLabel)
{
  QWidget *w = new QWidget();
  tabWidget->addTab(w, tabLabel);
  return new QVBoxLayout(w);
}

int ConfigDialog::addLineEdit(const char *Name, QBoxLayout *layout) {

  QLineEdit *lineEdit;

  QHBoxLayout *line = addHBox(layout);

//   line->setSpacing(5);
//   line->setMargin(10);
  QLabel *nameLabel = new QLabel();
  nameLabel->setText(Name);
  line->addWidget(nameLabel);
  lineEdit = new QLineEdit();
  lineEditList.append(lineEdit);
  line->addWidget(lineEdit);

  return 0;
}

int ConfigDialog::addScopeScreen(float *timeScaleRef, int *modeRef, int *edgeRef, int *triggerModeRef, 
                                 float *triggerThrsRef, float *zoomRef, QBoxLayout *layout) {

  ScopeScreen *scopeScreen;

  scopeScreen = new ScopeScreen();
  insertWidget(layout, scopeScreen, 1);

  scopeScreenList.append(scopeScreen);
  return(0);
}

#ifdef OUTDATED_CODE
int ConfigDialog::addSpectrumScreen(QBoxLayout *layout) {

  SpectrumScreen *spectrumScreen;

  if (!parent) {
    spectrumScreen = new SpectrumScreen(NULL, "Spectrum");
  } else {
    spectrumScreen = new SpectrumScreen(parent, "Spectrum");
  }
  spectrumScreenList.append(spectrumScreen);
  return(0);
}
#endif

int ConfigDialog::addFunction(int p_functionCount, int *p_mode, int *p_editIndex, tFunction &point, int p_pointCount, QBoxLayout *layout) {

  Function *function;

  function = new Function(p_functionCount, p_mode, p_editIndex, point, p_pointCount, NULL);
  insertWidget(layout, function, 1);

  functionList.append(function);
  return(0);
}

void ConfigDialog::insertWidget(QBoxLayout *layout, QWidget *widget,
				int stretch, Qt::Alignment alignment)
{
  if (!layout)
    layout = configBox;

  layout->addWidget(widget, stretch, alignment);
  if (addStretch > 0)
    layout->addStretch(addStretch);
}

void ConfigDialog::removeButtonShow(bool show)
{
  removeFrame->setEnabled(show);
  removeFrame->setMargin(show ? 5 : 0);
  removeButton->setVisible(show);
}
