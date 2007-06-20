#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

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
#include <QVBoxLayout>
#include <qtabwidget.h>
#include <qlineedit.h>
#include <QPointF>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "envelope.h"
#include "function.h"

/** configuration dialog for each module
 *
 */ 
class ConfigDialog : public QWidget
{
  Q_OBJECT

  QVBoxLayout *configBox;
  int addStretch;
  QPushButton *removeButton;
  QHBoxLayout *removeFrame;
    
public: 
  QList<class MidiSlider*> midiSliderList; 
  QList<class IntMidiSlider*> intMidiSliderList; 
  QList<class FloatIntMidiSlider*> floatIntMidiSliderList; 
  QList<class MidiComboBox*> midiComboBoxList;
  QList<class MidiCheckBox*> midiCheckBoxList;
  QList<class MidiPushButton*> midiPushButtonList;
  QList<class Envelope*> envelopeList;
  QList<class ScopeScreen*> scopeScreenList;
#ifdef OUTDATED_CODE
  QList<class SpectrumScreen*> spectrumScreenList;
#endif
  QList<class MultiEnvelope*> multiEnvelopeList;
  QList<class Function*> functionList;
  QList<class MidiGUIcomponent*> midiGUIcomponentList;
  QList<class QLineEdit*> lineEditList;
  QList<class QLabel*> labelList;
  Module *parentModule;
  QTabWidget *tabWidget;

protected:
  void insertWidget(QBoxLayout *layout, QWidget *widget, int stretch = 0, Qt::Alignment alignment = 0);
     
public:
  ConfigDialog(Module *p_parentModule, QWidget* parent=0, const char *name=0);
  ~ConfigDialog();
  void setAddStretch(int v) {
    addStretch = v;
  }
  void removeButtonShow(bool show);
  int addSlider(float minValue, float maxValue, float value, const QString &name, float *valueRef, bool isLog=false, QBoxLayout *layout = 0);
  int addIntSlider(int minValue, int maxValue, int value, const QString &name, int *valueRef, QBoxLayout *layout = NULL);
  int addFloatIntSlider(float minValue, float maxValue, float value, const QString &name, float *valueRef, QBoxLayout *layout = NULL);
  int addComboBox(int value, const QString &name, int *valueRef, int itemCount, QStringList *itemNames, QBoxLayout *layout = NULL);
  int addCheckBox(float value, const QString &name, float *valueRef, QBoxLayout *layout = NULL);
  int addPushButton(const QString &name, QBoxLayout *layout = NULL);
  int addEnvelope(float *delayRef, float *attackRef, float *holdRef, 
		  float *decayRef, float *sustainRef, float *releaseRef, QBoxLayout *layout = NULL);
  int addMultiEnvelope(int envCount, float *timeScaleRef, float *attackRef, float *sustainRef, float *releaseRef, QBoxLayout *layout = NULL);
  int addFunction(int p_functionCount, int *p_mode, int *p_editIndex, tFunction &, int p_pointCount, QBoxLayout *layout = NULL);
  int addLabel(QString label, QBoxLayout *layout = NULL);
  int addScopeScreen(float *timeScaleRef, int *modeRef, int *edgeRef, int *triggerModeRef, 
		     float *triggerThrsRef, float *zoomRef, QBoxLayout *layout = NULL);
#ifdef OUTDATED_CODE
  int addSpectrumScreen(QWidget *parent=0);
#endif
  int addTab(QWidget *tabPage, const QString &tabLabel) {
    return tabWidget->addTab(tabPage, tabLabel);
  }
  QVBoxLayout *addVBoxTab(const char *tabLabel);
  QVBoxLayout *addVBoxTab(const QString &tabLabel);
  QHBoxLayout *addHBox(QBoxLayout *layout = NULL);
  QVBoxLayout *addVBox(QBoxLayout *layout = NULL);
  int addLineEdit(const char *lineName, QBoxLayout *layout = NULL);
  int initTabWidget();
    
signals:
  void removeModuleClicked();
                
public slots: 
  void removeButtonClicked();
};
  
#endif
