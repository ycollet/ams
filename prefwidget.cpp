#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h> 
#include <qsplitter.h>
#include <qlistbox.h> 
#include <qlabel.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qspinbox.h>
#include <qfiledialog.h>
#include <qradiobutton.h>
#include <qcolordialog.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qregexp.h> 
#include <qlayout.h> 
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "prefwidget.h"

PrefWidget::PrefWidget(SynthData *p_synthdata, QWidget* parent, const char *name) 
                : QVBox(parent, name) {

  setGeometry(0, 0, PREF_DEFAULT_WIDTH, PREF_DEFAULT_HEIGHT);
  setMargin(10);
  setSpacing(5);
  synthdata = p_synthdata;
  updateColors();

  QWidget *colorGridWidget = new QWidget(this);
  QGridLayout *colorLayout = new QGridLayout(colorGridWidget, 12, 2, 10);

  QLabel *label1 = new QLabel("Background Color", colorGridWidget);
  colorBackgroundLabel = new QLabel(colorGridWidget);
  colorBackgroundLabel->setPalette(QPalette(colorBackground, colorBackground));  
  QPushButton *changeColorBackgroundButton = new QPushButton("Change", colorGridWidget);
  colorLayout->addWidget(label1, 0, 0);
  colorLayout->addWidget(colorBackgroundLabel, 1, 0);
  colorLayout->addWidget(changeColorBackgroundButton, 1, 1);

  QLabel *label2 = new QLabel("Module Background Color", colorGridWidget);
  colorModuleBackgroundLabel = new QLabel(colorGridWidget);
  colorModuleBackgroundLabel->setPalette(QPalette(colorModuleBackground, colorModuleBackground));  
  QPushButton *changeColorModuleBackgroundButton = new QPushButton("Change", colorGridWidget);
  colorLayout->addWidget(label2, 2, 0);
  colorLayout->addWidget(colorModuleBackgroundLabel, 3, 0);
  colorLayout->addWidget(changeColorModuleBackgroundButton, 3, 1);

  QLabel *label3 = new QLabel("Module Border Color", colorGridWidget);
  colorModuleBorderLabel = new QLabel(colorGridWidget);
  colorModuleBorderLabel->setPalette(QPalette(colorModuleBorder, colorModuleBorder));  
  QPushButton *changeColorModuleBorderButton = new QPushButton("Change", colorGridWidget);
  colorLayout->addWidget(label3, 4, 0);
  colorLayout->addWidget(colorModuleBorderLabel, 5, 0);
  colorLayout->addWidget(changeColorModuleBorderButton, 5, 1);

  QLabel *label4 = new QLabel("Module Font Color", colorGridWidget);
  colorModuleFontLabel = new QLabel(colorGridWidget);
  colorModuleFontLabel->setPalette(QPalette(colorModuleFont, colorModuleFont));  
  QPushButton *changeColorModuleFontButton = new QPushButton("Change", colorGridWidget);
  colorLayout->addWidget(label4, 6, 0);
  colorLayout->addWidget(colorModuleFontLabel, 7, 0);
  colorLayout->addWidget(changeColorModuleFontButton, 7, 1);

  QLabel *label5 = new QLabel("Cable Color", colorGridWidget);
  colorCableLabel = new QLabel(colorGridWidget);
  colorCableLabel->setPalette(QPalette(colorCable, colorCable));  
  QPushButton *changeColorCableButton = new QPushButton("Change", colorGridWidget);
  colorLayout->addWidget(label5, 8, 0);
  colorLayout->addWidget(colorCableLabel, 9, 0);
  colorLayout->addWidget(changeColorCableButton, 9, 1);
  
  QLabel *label6 = new QLabel("Jack Color", colorGridWidget);
  colorJackLabel = new QLabel(colorGridWidget);
  colorJackLabel->setPalette(QPalette(colorJack, colorJack));  
  QPushButton *changeColorJackButton = new QPushButton("Change", colorGridWidget);
  colorLayout->addWidget(label6, 10, 0);
  colorLayout->addWidget(colorJackLabel, 11, 0);
  colorLayout->addWidget(changeColorJackButton, 11, 1);

  QObject::connect(changeColorBackgroundButton, SIGNAL(clicked()),this, SLOT(colorBackgroundClicked()));
  QObject::connect(changeColorModuleBackgroundButton, SIGNAL(clicked()),this, SLOT(colorModuleBackgroundClicked()));
  QObject::connect(changeColorModuleBorderButton, SIGNAL(clicked()), this, SLOT(colorModuleBorderClicked()));
  QObject::connect(changeColorModuleFontButton, SIGNAL(clicked()), this, SLOT(colorModuleFontClicked()));
  QObject::connect(changeColorCableButton, SIGNAL(clicked()), this, SLOT(colorCableClicked()));
  QObject::connect(changeColorJackButton, SIGNAL(clicked()), this, SLOT(colorJackClicked()));
  
  QHBox *buttonContainer = new QHBox(this);
  new QWidget(buttonContainer);
  QPushButton *cancelButton = new QPushButton("Cancel", buttonContainer);
  new QWidget(buttonContainer);
  QPushButton *applyButton = new QPushButton("Apply", buttonContainer);
  new QWidget(buttonContainer);
  QObject::connect(applyButton, SIGNAL(clicked()), this, SLOT(submitPref()));  
  QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));  
}

PrefWidget::~PrefWidget() {

}

void PrefWidget::loadPref(QString config_fn) {

  QString qs, qs2;
  int r,g,b;

  QFile f(config_fn);
  if (!f.open( IO_ReadOnly )) {
    QMessageBox::information( this, "AlsaModularSynth", "Could not open file.");
  } else {
    QTextStream rctext(&f);
    QRegExp sep(" ");
    while (!rctext.atEnd()) {
      qs = rctext.readLine(); 
      if (qs.contains("ColorBackground", false)) {
        qs2 = qs.section(sep, 1, 1); 
        r = qs2.toInt();
        qs2 = qs.section(sep, 2, 2); 
        g = qs2.toInt();
        qs2 = qs.section(sep, 3, 3); 
        b = qs2.toInt();
        synthdata->colorBackground = QColor(r, g, b);
      }        
      if (qs.contains("ColorModuleBackground", false)) {
        qs2 = qs.section(sep, 1, 1); 
        r = qs2.toInt();
        qs2 = qs.section(sep, 2, 2); 
        g = qs2.toInt();
        qs2 = qs.section(sep, 3, 3); 
        b = qs2.toInt();
        synthdata->colorModuleBackground = QColor(r, g, b);
      }        
      if (qs.contains("ColorModuleBorder", false)) {
        qs2 = qs.section(sep, 1, 1); 
        r = qs2.toInt();
        qs2 = qs.section(sep, 2, 2); 
        g = qs2.toInt();
        qs2 = qs.section(sep, 3, 3); 
        b = qs2.toInt();
        synthdata->colorModuleBorder = QColor(r, g, b);
      }        
      if (qs.contains("ColorModuleFont", false)) {
        qs2 = qs.section(sep, 1, 1); 
        r = qs2.toInt();
        qs2 = qs.section(sep, 2, 2); 
        g = qs2.toInt();
        qs2 = qs.section(sep, 3, 3); 
        b = qs2.toInt();
        synthdata->colorModuleFont = QColor(r, g, b);
        synthdata->colorPortFont1 = QColor(r, g, b);
      }        
      if (qs.contains("ColorJack", false)) {
        qs2 = qs.section(sep, 1, 1); 
        r = qs2.toInt();
        qs2 = qs.section(sep, 2, 2); 
        g = qs2.toInt();
        qs2 = qs.section(sep, 3, 3); 
        b = qs2.toInt();
        synthdata->colorJack = QColor(r, g, b);
      }        
      if (qs.contains("ColorCable", false)) {
        qs2 = qs.section(sep, 1, 1); 
        r = qs2.toInt();
        qs2 = qs.section(sep, 2, 2); 
        g = qs2.toInt();
        qs2 = qs.section(sep, 3, 3); 
        b = qs2.toInt();
        synthdata->colorCable = QColor(r, g, b);
      }       
    }   
    f.close();
  }        
  updateColors();
  refreshColors();
}

void PrefWidget::savePref(QString config_fn) {

  QString qs;

  QFile f(config_fn);
  if (!f.open( IO_WriteOnly )) {
    QMessageBox::information( this, "AlsaModularSynth", "Could not open file.");
  } else {
    QTextStream rctext(&f);
    rctext << "ColorBackground " << synthdata->colorBackground.red() << " " << synthdata->colorBackground.green() << " " << synthdata->colorBackground.blue() << "\n";
    rctext << "ColorModuleBackground " << synthdata->colorModuleBackground.red() << " " << synthdata->colorModuleBackground.green() << " " << synthdata->colorModuleBackground.blue() << "\n";
    rctext << "ColorModuleBorder " << synthdata->colorModuleBorder.red() << " " << synthdata->colorModuleBorder.green() << " " << synthdata->colorModuleBorder.blue() << "\n";
    rctext << "ColorModuleFont " << synthdata->colorModuleFont.red() << " " << synthdata->colorModuleFont.green() << " " << synthdata->colorModuleFont.blue() << "\n";
    rctext << "ColorJack " << synthdata->colorJack.red() << " " << synthdata->colorJack.green() << " " << synthdata->colorJack.blue() << "\n";
    rctext << "ColorCable " << synthdata->colorCable.red() << " " << synthdata->colorCable.green() << " " << synthdata->colorCable.blue() << "\n";
    f.close();
  }       
}                             

void PrefWidget::submitPref() {

  emit prefChanged();
  close();
}

void PrefWidget::refreshColors() {

  colorBackgroundLabel->setPalette(QPalette(colorBackground, colorBackground));  
  colorModuleBackgroundLabel->setPalette(QPalette(colorModuleBackground, colorModuleBackground));  
  colorModuleBorderLabel->setPalette(QPalette(colorModuleBorder, colorModuleBorder));  
  colorModuleFontLabel->setPalette(QPalette(colorModuleFont, colorModuleFont));  
  colorCableLabel->setPalette(QPalette(colorCable, colorCable));  
  colorJackLabel->setPalette(QPalette(colorJack, colorJack));  
}

void PrefWidget::updateColors() {

  colorBackground = synthdata->colorBackground;
  colorModuleBackground = synthdata->colorModuleBackground; 
  colorModuleBorder = synthdata->colorModuleBorder;
  colorModuleFont = synthdata->colorModuleFont;
  colorCable = synthdata->colorCable;
  colorJack = synthdata->colorJack;
}

void PrefWidget::colorBackgroundClicked() {

  QColor tmp;
  
  tmp = QColorDialog::getColor(colorBackground);
  if (tmp.isValid()) {
    colorBackground = tmp;
    synthdata->colorBackground = tmp;
    refreshColors();
  }
}

void PrefWidget::colorModuleBackgroundClicked() {

  int l1;
  QColor tmp;

  tmp = QColorDialog::getColor(colorModuleBackground);
  if (tmp.isValid()) {
    synthdata->colorModuleBackground = tmp;
    updateColors();
    refreshColors();
  }
}

void PrefWidget::colorModuleBorderClicked() {

  QColor tmp;

  tmp = QColorDialog::getColor(colorModuleBorder);
  if (tmp.isValid()) {       
    synthdata->colorModuleBorder = tmp;
    updateColors();
    refreshColors();
  }
}

void PrefWidget::colorModuleFontClicked() {

  QColor tmp;

  tmp = QColorDialog::getColor(colorModuleFont);
  if (tmp.isValid()) {       
    synthdata->colorModuleFont = tmp;    
    updateColors();
    refreshColors();
  }
}

void PrefWidget::colorCableClicked() {

  QColor tmp;

  tmp = QColorDialog::getColor(synthdata->colorCable);
  if (tmp.isValid()) {       
    synthdata->colorCable = tmp;    
    updateColors();
    refreshColors();
  }
}

void PrefWidget::colorJackClicked() {

  QColor tmp;

  tmp = QColorDialog::getColor(synthdata->colorJack);
  if (tmp.isValid()) {       
    synthdata->colorJack = tmp;    
    updateColors();
    refreshColors();
  }
}
