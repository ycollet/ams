#ifndef M_SCQUANTIZER_H
#define M_SCQUANTIZER_H

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
#include <qfile.h>
#include <QFileDialog>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "module.h"
#include "port.h"

#define MODULE_SCQUANTIZER_WIDTH                 90
#define MODULE_SCQUANTIZER_HEIGHT               140

class M_scquantizer : public Module
{
  Q_OBJECT

  private:
    Port *port_M_in, *port_M_trigger, *port_M_transpose, *port_out, *port_trigger_out;
    int trigCount[MAXPOLY];
    bool trigger[MAXPOLY];
    int quantum;

    float scale_lut[128];
    bool scale_lut_isRatio[128];
    int scale_lut_length;
    float scale_notes[128];
    float qsig[MAXPOLY];
    QFileDialog *fileDialog;
                           
  private:
    void calcScale();
                                  
  public: 
    int base, lastbase;
    QString sclname, dirpath;
    float **inData, **triggerData, **transposeData;       
                            
  public:
    M_scquantizer(QWidget* parent=0, const char *name=0, QString *p_sclname = 0);
    ~M_scquantizer();

  public slots:
    void generateCycle();
    void showConfigDialog();
    void loadScale(const QString &p_sclname);
    void openBrowser();
};
  
#endif
