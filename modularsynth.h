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
#include <qframe.h>
#include <qmessagebox.h>
#include <qsocketnotifier.h>
#include <qpopupmenu.h>
#include <qscrollview.h>
#include <qlistview.h>
#include <qpoint.h>
#include <alsa/asoundlib.h>
#include "synth.h"
#include "capture.h"
#include "synthdata.h"
#include "module.h"
#include "port.h"
#include "m_vco.h"
#include "m_vca.h"
#include "m_delay.h"
#include "m_seq.h"
#include "m_env.h"
#include "m_advenv.h"
#include "m_mcv.h"
#include "m_ringmod.h"
#include "m_mix.h"
#include "m_out.h"
#include "m_jackout.h"
#include "m_jackin.h"
#include "m_wavout.h"
#include "m_in.h"
#include "m_lfo.h"
#include "m_noise.h"
#include "m_ladspa.h"
#include "m_vcf.h"
#include "m_inv.h"
#include "m_cvs.h"
#include "m_vcswitch.h"
#include "m_slew.h"
#include "m_sh.h"
#include "m_midiout.h"
#include "m_scope.h"
#include "m_spectrum.h"
#include "m_vcorgan.h"
#include "m_dynamicwaves.h"
#include "m_quantizer.h"
#include "midiwidget.h"
#include "textedit.h"
#include "ladspadialog.h"

#define DEFAULT_PCMNAME  "plughw:0,0"
#define DEFAULT_WIDTH             750
#define DEFAULT_HEIGHT            550
#define SYNTH_DEFAULT_WIDTH       750
#define SYNTH_DEFAULT_HEIGHT      500
#define SYNTH_MINIMUM_WIDTH       200
#define SYNTH_MINIMUM_HEIGHT      150

enum connectorStyleType {CONNECTOR_STRAIGHT, CONNECTOR_BEZIER};

class ModularSynth : public QScrollView
{
  Q_OBJECT

  private:
    Synth *synth;
    Capture *capture;
    QMessageBox *aboutWidget;
    QList<Module> listModule;
    QList<TextEdit> listTextEdit;
    QString PCMname, presetPath, savePath;
    connectorStyleType connectorStyle;    
    bool firstPort;
    Port *connectingPort[2];
    QSocketNotifier *seqNotifier;
    LadspaDialog *ladspaDialog;
    MidiWidget *midiWidget;
    bool loadingPatch;

  public:
    SynthData *synthdata;
    
  private:
    void initPorts(Module *m);
    void initNewModule(Module *m);
    snd_pcm_t *open_pcm(bool openCapture);
    snd_seq_t *open_seq(); 
    int initSeqNotifier();  
    void newM_mix(int in_channels);
    void newM_seq(int seqLen);
    void newM_vcorgan(int oscCount);
    void newM_dynamicwaves(int oscCount);
    void new_textEdit(int x, int y, int w, int h);
            
  public:
    ModularSynth(int poly, int periodsize, QWidget* parent=0, const char *name=0);
    ~ModularSynth();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;
    int go(bool withJack);
    int setPeriodsize(int periodsize);
    int setPeriods(int periods);
    int setRate(int rate);
    int setChannels(int channels);
    int setPCMname(QString name);
    int setPresetPath(QString name);
    int setSavePath(QString name);
    
  protected:
    void viewportPaintEvent(QPaintEvent *pe);
    virtual void mousePressEvent (QMouseEvent* );
    virtual void mouseReleaseEvent (QMouseEvent* );
    
  public slots: 
    void displayAbout();
    void displayMidiController();
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
    void newM_advenv();
    void newM_mcv();
    void newM_vco();
    void newM_vca();
    void newM_vcf();
    void newM_lfo();
    void newM_noise();
    void newM_ringmod();
    void newM_inv();
    void newM_cvs();
    void newM_slew();
    void newM_quantizer();
    void newM_mix_2();
    void newM_mix_4();
    void newM_mix_8();
    void newM_ladspa(int p_ladspaDesFuncIndex, int n, bool p_newLadspaPoly);
    void newM_out();
    void newM_jackout();
    void newM_jackin();
    void newM_wavout();
    void newM_in();
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
    void moveModule(QPoint pos);
    void moveTextEdit(QPoint pos);
    void resizeTextEdit(QPoint pos);
    void portSelected();
    void updatePortConnections();
    void deleteModule();
    void deleteModule(Module *m);
    void deleteTextEdit();
    void deleteTextEdit(TextEdit *te);
    void clearConfig();
    void load(QString *presetName);
    void load();
    void save();
    void allVoicesOff();
    void cleanUpSynth();
};
  
#endif
