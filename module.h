#ifndef MODULE_H
#define MODULE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qstring.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qpoint.h>
#include <qlist.h>
#include <QMouseEvent>
#include <QPaintEvent>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "configdialog.h"
#include "port.h"
#include "box.h"

#define MODULE_DEFAULT_WIDTH                 50
#define MODULE_DEFAULT_HEIGHT               100 
#define MODULE_NEW_X                         50
#define MODULE_NEW_Y                         50

enum M_typeEnum {
  M_type_custom,
  M_type_vco,
  M_type_vca,
  M_type_lfo,
  M_type_delay,
  M_type_ringmod,
  M_type_ladspa,
  M_type_pcmout,
  M_type_mix,
  M_type_vcf,
  M_type_mcv,		// 10
  M_type_env,
  M_type_seq,
  M_type_inv,
  M_type_noise,
  M_type_slew,
  M_type_quantizer,
  M_type_pcmin,
  M_type_cvs, 
  M_type_sh,
  M_type_vcorgan,	// 20
  M_type_dynamicwaves,
  M_type_advenv,
  M_type_wavout, 
  M_type_scope,
  M_type_spectrum,
  M_type_vcswitch,
  M_type_jackin,
  M_type_jackout,
  M_type_midiout,
  M_type_scmcv,		// 30
  M_type_scquantizer,
  M_type_stereomix,
  M_type_conv, 
  M_type_vcenv,
  M_type_advmcv,
  M_type_function,
  M_type_vcpanning,
  M_type_vcenv2,
  M_type_vcdoubledecay,
  M_type_vquant,
  M_type_amp,
  M_type_ad,
  M_type_mphlfo,
  M_type_noise2,
  M_type_vco2
};

// types jackin and jackout kept to ensure existing patches will load. FA

class Module : public Box
{
  Q_OBJECT

  int connections;
    
  public: 
    ConfigDialog *configDialog;
    float ***data; 
    bool cycleReady;
    M_typeEnum M_type;  
    QList<Port*> portList;
    QList<class MidiControllableBase *> midiControllables;
    int moduleID, outPortCount;
//    class QTreeWidgetItem *listViewItem;
    QColor colorBackground, colorBorder, colorFont;
  static int portmemAllocated;

  public:
    Module(M_typeEnum M_type, int outPortCount, QWidget* parent, const QString &name);
    virtual  ~Module();
  int connected() {
    return connections;
  }
  void incConnections();
  void decConnections();
  virtual void mcAbleChanged(MidiControllableBase *) {}
    int save(FILE *f);
    virtual int saveConnections(FILE *f); 
    virtual int saveParameters(FILE *f);
    virtual int saveBindings(FILE *f); 
    virtual int load(FILE *f);
    void getColors(void);

  float **getData(int index) {
    if (!cycleReady) {
      generateCycle();
      cycleReady = true;
    }
    return data[index];
  }


protected:
  virtual void generateCycle() = 0;

  void portMemAlloc(int outPortCount, bool poly);
  virtual void paintEvent(QPaintEvent *ev);
  virtual void mousePressEvent (QMouseEvent* );

  signals:
    void dragged(QPoint pos);
    void removeModule();
                        
  public slots: 
    virtual void removeThisModule();
};
  
#endif
