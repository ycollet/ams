#ifndef MODULARSYNTH_H
#define MODULARSYNTH_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlabel.h>
#include <qlist.h>
//#include <q3frame.h>
#include <qmessagebox.h>
#include <QMainWindow>
#include <qsocketnotifier.h>
#include <qpoint.h>
#include <qcolor.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <QMouseEvent>
#include <QTextStream>
#include <alsa/asoundlib.h>
#include "main.h"
#include "synthdata.h"
#include "module.h"
#include "port.h"
#include "midiwidget.h"
#include "msoptions.h"
#include "ladspadialog.h"

extern QTextStream StdErr;
extern QTextStream StdOut;

enum connectorStyleType {CONNECTOR_STRAIGHT, CONNECTOR_BEZIER};


class ModularSynth : public QWidget
{
  Q_OBJECT

    QMessageBox *aboutWidget;
    QList<Module*> listModule;
    QList<class TextEdit*> listTextEdit;
    connectorStyleType connectorStyle;    
    bool firstPort;
    Port *connectingPort[2];
    QSocketNotifier *seqNotifier;
    LadspaDialog *ladspaDialog;
    MidiWidget *midiWidget;
    class GuiWidget *guiWidget;
    class PrefWidget *prefWidget;
    bool loadingPatch;
    QString pcmname;
    int   fsamp;
    int   frsize;
    int   nfrags;
    int   ncapt;
    int   nplay;
    bool paintFastly;
    bool modified;
    double _zoomFactor;
    QPoint newBoxPos;
  int rcFd;
    void initPorts(Module *m);
    void initNewModule(Module *m);
    snd_pcm_t *open_pcm(bool openCapture);
    snd_seq_t *open_seq(); 
    int initSeqNotifier();  
    void newM_mix(int in_channels);
    void newM_function(int functionCount);
    void newM_stereomix(int in_channels);
    void newM_seq(int seqLen);
    void newM_vcorgan(int oscCount);
    void newM_dynamicwaves(int oscCount);
    void newM_ad(int outCount);
    void new_textEdit(int w, int h);
    void showContextMenu(QPoint pos);
    bool clearConfig(bool restart);
            
public:
  ModularSynth(QWidget* parent, const ModularSynthOptions&);
  ~ModularSynth();

    QMenu *contextMenu;
    void resize(void);
    int go(bool withJack);
    void setPaintFastly(bool v) {
      paintFastly = v;
    }
    void moveAllBoxes(const QPoint &delta);
    bool isModified();
    QString getLoadPath();
    void setLoadPath(const QString& sp);
    QString getSavePath();
    void setSavePath(const QString& sp);
    int getSynthDataPoly();

  protected:
    class QAbstractScrollArea *scrollArea() {
      return (QAbstractScrollArea *)parent();
    }
    void paintEvent(QPaintEvent *pe);
    virtual void mousePressEvent (QMouseEvent* );
    virtual void mouseReleaseEvent (QMouseEvent* );
    
  public slots: 
    void displayAbout();
    void displayMidiController();
    void displayParameterView();
    void displayPreferences();
    void displayLadspaPlugins();
    void midiAction(int fd);
    void startSynth();
    void stopSynth();
    void new_textEdit();
    void newM_delay();
    void newM_seq_8();
    void newM_seq_12();
    void newM_seq_16();
    void newM_seq_24();
    void newM_seq_32();
    void newM_env();
    void newM_vcenv();
    void newM_vcenv2();
    void newM_vcdoubledecay();
    void newM_vcpanning();
    void newM_advenv();
    void newM_mcv();
    void newM_advmcv();
    void newM_scmcv();
    void newM_scmcv(QString *p_scalaName);
    void newM_vco();
    void newM_vco2();
    void newM_vca_lin();
    void newM_vca_exp();
    void newM_vcf();
    void newM_lfo();
    void newM_mphlfo();
    void newM_noise();
    void newM_noise2();
    void newM_ringmod();
    void newM_inv();
    void newM_amp();
    void newM_ad_2();
    void newM_ad_4();
    void newM_ad_6();
    void newM_vquant();
    void newM_conv();
    void newM_cvs();
    void newM_slew();
    void newM_quantizer();
    void newM_scquantizer(QString *p_scalaName);
    void newM_scquantizer();
    void newM_mix_2();
    void newM_mix_4();
    void newM_mix_8();
    void newM_function_1();
    void newM_function_2();
    void newM_function_4();
    void newM_stereomix_2();
    void newM_stereomix_4();
    void newM_stereomix_8();
    void newM_ladspa(int p_ladspaDesFuncIndex, int n,
            bool p_newLadspaPoly, bool p_extCtrlPorts);
    void newM_pcmout();
    void newM_pcmin();
    void newM_wavout();
    void newM_sh();
    void newM_midiout();
    void newM_vcswitch();
    void newM_scope();
    void newM_spectrum();
    void newM_vcorgan_4();
    void newM_vcorgan_6();
    void newM_vcorgan_8();
    void newM_dynamicwaves_4();
    void newM_dynamicwaves_6();
    void newM_dynamicwaves_8();
    void resizeTextEdit(QPoint pos);
    void portSelected();
    void deleteModule();
    void deleteModule(Module *m);
    void deleteTextEdit();
    void deleteTextEdit(TextEdit *te);
    void clearConfig();
    void load(QTextStream&);
    void save(QTextStream&);
    void loadColors();
    void saveColors();
    void allVoicesOff();
    void cleanUpSynth();
    void refreshColors();
    void redrawPortConnections();
    void updateColors();
};

  
#endif
