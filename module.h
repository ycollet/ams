#ifndef MODULE_H
#define MODULE_H

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
#include <qsize.h>
#include <qsizepolicy.h>
#include <qpoint.h>
#include <qlist.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "configdialog.h"
#include "port.h"
#include "floatparameter.h"
#include "intparameter.h"
#include "enumparameter.h"
#include "actionparameter.h"
#include "boolparameter.h"

#define MODULE_DEFAULT_WIDTH                 50
#define MODULE_DEFAULT_HEIGHT               100 
#define MODULE_NEW_X                         50
#define MODULE_NEW_Y                         50

enum M_typeEnum { M_type_custom, M_type_vco, M_type_vca, M_type_lfo, M_type_delay, M_type_ringmod, 
                  M_type_ladspa, M_type_out, M_type_mix, M_type_vcf, M_type_mcv, M_type_env, M_type_seq, 
                  M_type_inv, M_type_noise, M_type_slew, M_type_quantizer, M_type_in, M_type_cvs, 
                  M_type_sh, M_type_vcorgan, M_type_dynamicwaves, M_type_advenv, M_type_wavout, 
                  M_type_scope, M_type_spectrum, M_type_vcswitch, M_type_jackin, M_type_jackout,
                  M_type_midiout };

class Module : public QWidget
{
  Q_OBJECT

  private:
    bool drag;
    QPoint mousePressPos;
    
  public: 
    ConfigDialog *configDialog;
    float ***data, ***lastdata; 
    bool cycleReady, cycleProcessing;
    M_typeEnum M_type;  
    QList<Port> portList;
    int moduleID, outPortCount;
    QListViewItem *listViewItem;
    


  protected:
    SynthData *synthdata;

  public:
    Module(int p_outPortCount, QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0, 
           M_typeEnum p_M_type=M_type_custom);
    virtual  ~Module();
    QPoint getMousePressPos();
    virtual void noteOnEvent(int osc);
    virtual void noteOffEvent(int osc); 
    
  protected:
    virtual void paintEvent(QPaintEvent *ev);
    virtual void mousePressEvent (QMouseEvent* );
    virtual void mouseReleaseEvent (QMouseEvent* );
    virtual void mouseMoveEvent (QMouseEvent* );

  signals:
    void dragged(QPoint pos);
    void removeModule();
                        
  public slots: 
    virtual void generateCycle();
    virtual void showConfigDialog();
    virtual void removeThisModule();
};
  
#endif
