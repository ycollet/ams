#ifndef MIDIWIDGET_H
#define MIDIWIDGET_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlistview.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <alsa/asoundlib.h>
#include "midicontroller.h"
#include "midicontrollerlist.h"
#include "synthdata.h"
#include "module.h"
#include "midiguicomponent.h"

class MidiWidget : public QVBox
{
  Q_OBJECT

  private:
   QCheckBox *noteCheck, *configCheck, *midiCheck;
   QListView *midiControllerListView, *moduleListView;
   SynthData *synthdata;
   QVBox *currentGUIcontrol, *guiControlParent;
   QLabel *valueLabel, *minLabel, *maxLabel;
   QSlider *slider;
   QCheckBox *logCheck, *currentCheck;
   QPushButton *currentPushButton;
   QString currentFrameName, currentTabName;
   QComboBox *comboBox;
   float minValue, maxValue, value, initial_min, initial_max;
   MidiGUIcomponent *midiGUIcomponent;
   bool firstBindingMightHaveChanged;
    
  public: 
    MidiControllerList midiControllerList;
    bool noteControllerEnabled, followConfig, followMidi;
    
  public:
    MidiWidget(SynthData *p_synthdata, QWidget* parent, const char *name=0);
    ~MidiWidget();
    int addMidiController(MidiController *midiController);
    int addMidiGuiComponent(MidiController *midiController, QObject *midiGuiComponent);
    int deleteMidiGuiComponent(MidiController *midiController, QObject *midiGuiComponent);
    MidiController *getSelectedController();
    int setSelectedController(MidiController *midiController);
    int findFirstBinding(MidiController *midiController);

  signals:
    void updateMIDIController();     

  public slots: 
    void clearAllClicked();
    void clearClicked();
    void bindClicked();
    void addToParameterViewClicked();
    void updateModuleList();
    void addModule(Module *m);
    void deleteModule(Module *m);
    void noteControllerCheckToggle();
    void configCheckToggle();
    void midiCheckToggle();
    void toggleMidiSign();
    void guiControlChanged();
    void midiControlChanged();
    void updateSliderValue(int p_value);
    void updateIntSliderValue(int p_value);
    void updateComboBox(int p_value);
    void updateCheckBox(bool on);
    void setLogCheck(bool);
    void setLogMode(bool on);
    void setNewMin();
    void setNewMax();
    void setInitialMinMax();
    void updateGuiComponent();
    void updateMidiChannel(int index);
};
  
#endif
