#ifndef M_LADSPA_H
#define M_LADSPA_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlabel.h>
#include <qlist.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <alsa/asoundlib.h>
#include <ladspa.h>
#include "synthdata.h"
#include "module.h"
#include "port.h"

#define MODULE_LADSPA_WIDTH                220
#define MODULE_LADSPA_HEIGHT                40
#define MAX_AUDIOPORTS                       8     // TODO audioports should be allocated dynamically
#define MAX_OUTPORTS                        16
#define MAX_CONTROLPORTS                    48     // TODO controlports should be allocated dynamically

class M_ladspa : public Module
{
  Q_OBJECT

  private:
    QList<Port> in_port_list, out_port_list;
    QList<Port> in_ctrl_port_list, out_ctrl_port_list;
    Module *in_M[MAX_AUDIOPORTS], *out_M[MAX_AUDIOPORTS];
    Module *in_M_ctrl[MAX_CONTROLPORTS], *out_M_ctrl[MAX_CONTROLPORTS];
    const LADSPA_Descriptor *ladspa_dsc;
    LADSPA_Handle ladspa_handle[MAXPOLY];
    LADSPA_Data *ladspaDataIn[MAX_AUDIOPORTS][MAXPOLY], *ladspaDataOut[MAX_AUDIOPORTS][MAXPOLY];
    LADSPA_Data control_data[MAX_CONTROLPORTS], control_gui[MAX_CONTROLPORTS], control_out[MAX_CONTROLPORTS];
    LADSPA_Data control_data_min[MAX_CONTROLPORTS], control_data_max[MAX_CONTROLPORTS];
    bool controlPortRate[MAX_CONTROLPORTS];
    int ladspa_audio_in_count, ladspa_audio_out_count;
    int ladspa_ctrl_in_count, ladspa_ctrl_out_count;
    int ctrl_index[MAX_CONTROLPORTS], audio_index[MAX_CONTROLPORTS];
    float rate_factor;
    
  public: 
    float **inData[MAX_AUDIOPORTS];
    float **inData_ctrl[MAX_CONTROLPORTS];
    int ladspaDesFuncIndex, n;
    bool isPoly, hasExtCtrlPorts;
    QString pluginName;
    
  public:
    M_ladspa(QWidget* parent=0, const char *name=0, SynthData *p_synthdata=0, int p_ladspaDesFuncIndex=0, int p_n=0, bool poly = false, bool extCtrlPorts = false);
    ~M_ladspa();

  protected:
    virtual void paintEvent(QPaintEvent *ev);
  
  public slots:
    void generateCycle();
    void showConfigDialog();
};
  
#endif
