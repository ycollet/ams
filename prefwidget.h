#ifndef PREFWIDGET_H
#define PREFWIDGET_H

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
#include <qcombobox.h>
#include <qstringlist.h>
#include <qlineedit.h>
#include <qfile.h>
#include <qtabwidget.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"

#define PREF_DEFAULT_WIDTH  300
#define PREF_DEFAULT_HEIGHT 300

class PrefWidget : public QVBox
{
  Q_OBJECT

  private:
    SynthData *synthdata;
    QTabWidget *tabWidget;
    QString loadPath, savePath;
    QLabel *colorBackgroundLabel, *colorModuleBackgroundLabel, *colorModuleBorderLabel, *colorModuleFontLabel;
    QLabel *colorCableLabel, *colorJackLabel;
    QColor colorBackground, colorModuleBackground, colorModuleBorder, colorModuleFont, colorPortFont1, colorPortFont2;
    QColor colorCable, colorJack;
    float drift_rate, drift_amp;
    float detune_rate, detune_amp;
    QComboBox *midiModeComboBox;
    QSlider *driftRateSlider, *driftAmpSlider;
    QLabel *driftAmpLabel, *driftRateLabel;
    QSlider *detuneRateSlider, *detuneAmpSlider;
    QLabel *detuneAmpLabel, *detuneRateLabel;
    QLineEdit *loadEdit, *saveEdit;
    int midiControllerMode;
   
  public:
    PrefWidget(SynthData *p_synthdata, QWidget* parent, const char *name=0);
    ~PrefWidget();

  signals:
    void prefChanged();

  public slots:
    void loadPref(QString config_fn);
    void savePref(QString config_fn);
    void submitPref();
    void applyPref();
    void refreshColors();
    void recallColors();
    void storeColors();
    void colorBackgroundClicked();
    void colorModuleBackgroundClicked();
    void colorModuleBorderClicked();
    void colorModuleFontClicked();
    void colorCableClicked();
    void colorJackClicked();
    void browseLoad();
    void browseSave();
    void updateMidiMode(int);
    void updateDriftAmp(int);
    void updateDriftRate(int);
    void updateDetuneAmp(int);
    void updateDetuneRate(int);
    void loadPathUpdate();
    void savePathUpdate();
};
  
#endif
