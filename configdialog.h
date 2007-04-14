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
#include <qvbox.h>
#include <qhbox.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qstrlist.h>
#include <qscrollview.h>
#include <qtabwidget.h>
#include <qlineedit.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "midislider.h"
#include "intmidislider.h"
#include "floatintmidislider.h"
#include "midicombobox.h"
#include "midicheckbox.h"
#include "midipushbutton.h"
#include "midiguicomponent.h"
#include "envelope.h"
#include "multi_envelope.h"
#include "scopescreen.h"
#include "function.h"
/** configuration dialog for each module
 *
 */ 
class ConfigDialog : public QVBox
{
  Q_OBJECT

  private:
    SynthData *synthdata;
    QVBox *configBox;
    QScrollView *scroll;
    
  public: 
    QList<MidiSlider> midiSliderList; 
    QList<IntMidiSlider> intMidiSliderList; 
    QList<FloatIntMidiSlider> floatIntMidiSliderList; 
    QList<MidiComboBox> midiComboBoxList;
    QList<MidiCheckBox> midiCheckBoxList;
    QList<MidiPushButton> midiPushButtonList;
    QList<Envelope> envelopeList;
    QList<ScopeScreen> scopeScreenList;
#ifdef OUTDATED_CODE
    QList<SpectrumScreen> spectrumScreenList;
#endif
    QList<MultiEnvelope> multiEnvelopeList;
    QList<Function> functionList;
    QList<MidiGUIcomponent> midiGUIcomponentList;
    QList<QLineEdit> lineEditList;
    QList<QHBox> hboxList;
    QList<QVBox> vboxList;
    QList<QLabel> labelList;
    QPushButton *removeButton;
    QObject *parentModule;
    QTabWidget *tabWidget;
            
  public:
    ConfigDialog(QObject *p_parentModule, QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0);
    ~ConfigDialog();
    int addSlider(float minValue, float maxValue, float value, const char *name, float *valueRef, bool isLog=false, QWidget *parent=0);
    int addIntSlider(int minValue, int maxValue, int value, const char *name, int *valueRef, QWidget *parent=0);
    int addFloatIntSlider(float minValue, float maxValue, float value, const char *name, float *valueRef, QWidget *parent=0);
    int addComboBox(int value, const char * name, int *valueRef, int itemCount, QStrList *itemNames, QWidget *parent=0);
    int addCheckBox(float value, const char * name, float *valueRef, QWidget *parent=0);
    int addPushButton(const char * name, QWidget *parent=0);
    int addEnvelope(float *delayRef, float *attackRef, float *holdRef, 
                    float *decayRef, float *sustainRef, float *releaseRef, QWidget *parent=0);
    int addMultiEnvelope(int envCount, float *timeScaleRef, float *attackRef, float *sustainRef, float *releaseRef, QWidget *parent=0);
    int addFunction(int p_functionCount, int *p_mode, int *p_editIndex, QPointArray *p_points[], int p_pointCount, SynthData *p_synthdata, QWidget *parent=0);
    int addLabel(QString label, QWidget *parent=0);
    int addScopeScreen(float *timeScaleRef, int *modeRef, int *edgeRef, int *triggerModeRef, 
                       float *triggerThrsRef, float *zoomRef, QWidget *parent=0);
#ifdef OUTDATED_CODE
    int addSpectrumScreen(QWidget *parent=0);
#endif
    int addTab(QWidget *tabPage, QString tabLabel);
    QHBox *addHBox(QWidget *parent=0);
    QVBox *addVBox(QWidget *parent=0);
    int addLineEdit(QString lineName, QWidget *parent=0);
    int initTabWidget();
    
  signals:
    void removeModuleClicked();
                
  public slots: 
    void removeButtonClicked();
};
  
#endif
