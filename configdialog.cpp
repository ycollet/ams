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
#include "envelope.h"
#include "multi_envelope.h"

ConfigDialog::ConfigDialog(QObject *p_parentModule, QWidget* parent, const char *name)
: QVBox(parent, name) {


  parentModule =p_parentModule;
  headerBox = new QVBox(this);
  headerBox->setMinimumWidth(430);
  headerBox->setMargin(5);
  headerBox->setSpacing(5);
  tabWidget = new QTabWidget(this);

  QHBox *removeFrame = new QHBox(this);
  removeFrame->setMargin(10);
  new QWidget(removeFrame);
  removeButton = new QPushButton("Remove Module", removeFrame);
  new QWidget(removeFrame);
  QObject::connect(removeButton, SIGNAL(clicked()), this, SLOT(removeButtonClicked()));

  //synthdata = p_synthdata;
}

ConfigDialog::~ConfigDialog() {
}



void ConfigDialog::removeButtonClicked() {

  emit removeModuleClicked();
}


void ConfigDialog::addParameter(Parameter * param, const char * panelName){
   QString name;
   if(!panelName){
      name = "Main";
   }
   else{
      name = panelName;
   }
   ParameterPanel * panel = mPanels[name];
   if(!panel){
      panel = new ParameterPanel(tabWidget,name,false);
      tabWidget->addTab(panel,name);
      mPanels.replace(name,panel);
   }
   panel->addParameter(param);
}


/*
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

*/
