#ifndef GUIWIDGET_H
#define GUIWIDGET_H

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
#include <qgroupbox.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qstringlist.h>
#include <qtabwidget.h>
#include <alsa/asoundlib.h>
#include "midicontroller.h"
#include "midicontrollerlist.h"
#include "synthdata.h"
#include "module.h"
#include "midiguicomponent.h"

#define GUI_DEFAULT_WIDTH   300
#define  GUI_DEFAULT_HEIGHT 200
#define MAX_PRESETS         128

class GuiWidget : public QVBox
{
  Q_OBJECT
  
  private:
    SynthData *synthdata;
//    QHBox *frameContainer;
    QHBox *currentGroupBox;
    QTabWidget *tabWidget;
    QHBox *currentTab;
    int currentPreset;
    QLabel *presetLabel, *presetCountLabel;
   
  public: 
    int presetCount;
    QStringList frameNameList;
    QList<QVBox> frameBoxList;
    QStringList tabNameList;
    QList<QHBox> tabList;
    QList<MidiGUIcomponent> parameterList;
    QValueList<int> presetList[MAX_PRESETS];
    
  public:
    GuiWidget(SynthData *p_synthdata, QWidget* parent, const char *name=0);
    ~GuiWidget();
    int addFrame(QString frameName);
    int setFrame(int index);
    int addTab(QString tabName);
    int setTab(int index);
    int setPresetCount(int count);
    int setCurrentPreset(int presetNum);
    int addParameter(MidiGUIcomponent *midiGUIcomponent, QString parameterName);
    
  public slots:
    void presetDec();
    void presetInc();  
    void addPreset();
    void overwritePreset();
    void clearPresets();
    void clearGui();
};
  
#endif
