#ifndef LADSPADIALOG_H
#define LADSPADIALOG_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlistbox.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <ladspa.h>
#include "synthdata.h"

class LadspaDialog : public QVBox
{
  Q_OBJECT

  private:
    SynthData *synthdata;
    QLabel *pluginLabel, *pluginMaker, *pluginCopyright;
    QLineEdit *searchLine;
    QCheckBox *extCtrlPortsCheck;
           
  public: 
    QListView *ladspaList;
    
  public:
    LadspaDialog(SynthData *p_synthdata, QWidget* parent=0, const char *name=0);
    ~LadspaDialog();
    
  signals:
    void createLadspaModule(int index, int n, bool poly, bool extCtrlPorts);  

  private slots:
    void insertClicked();
    void insertPolyClicked();
    void searchClicked();
    void pluginHighlighted();
};
  
#endif
