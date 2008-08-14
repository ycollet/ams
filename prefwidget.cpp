#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h> 
#include <qsplitter.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <QFileDialog>
#include <qradiobutton.h>
#include <qcolordialog.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qregexp.h> 
#include <qlayout.h> 
#include <QDir>
#include <QGridLayout>
#include <QTextStream>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "prefwidget.h"

class ColorWidget: public QWidget {
  QLabel label;
  QPushButton button;
public:
  ColorWidget(const char *l, QGridLayout *layout, QColor &color, int pos,
	      PrefWidget *pw, const char *slot)
    : label(l)
    , button("Change")
  {
    setAutoFillBackground(true);
    setPalette(QPalette(color, color));
    layout->addWidget(&label, pos - 1, 0);
    layout->addWidget(this, pos, 0);
    layout->addWidget(&button, pos, 1);
    connect(&button, SIGNAL(clicked()), pw, slot);
  }
};

PrefWidget::PrefWidget()
  : vBox(this)
{
  setGeometry(0, 0, PREF_DEFAULT_WIDTH, PREF_DEFAULT_HEIGHT);
  vBox.setMargin(10);
  vBox.setSpacing(5);
  
  recallColors();

  tabWidget = new QTabWidget();
  vBox.addWidget(tabWidget);
  QWidget *colorWidget = new QWidget();
  QVBoxLayout *colorBox = new QVBoxLayout(colorWidget);
  QWidget *midiWidget = new QWidget();
  QVBoxLayout *midiBox = new QVBoxLayout(midiWidget);
  QWidget *pathWidget = new QWidget();
  QVBoxLayout *pathBox = new QVBoxLayout(pathWidget);
  tabWidget->addTab(colorWidget, tr("Colors"));
  tabWidget->addTab(midiWidget, tr("MIDI"));
  tabWidget->addTab(pathWidget, tr("Paths"));

  QGridLayout *colorLayout = new QGridLayout();//colorGridWidget, 12, 2, 10);
  colorBox->addLayout(colorLayout);

  // QLabel *label1 = new QLabel("Background Color", colorGridWidget);
  colorBackgroundLabel =
    new ColorWidget("Background Color", colorLayout, colorBackground, 1,
		    this, SLOT(colorBackgroundClicked()));
  colorModuleBackgroundLabel =
    new ColorWidget("Module Background Color", colorLayout, colorModuleBackground, 3,
		    this, SLOT(colorModuleBackgroundClicked()));
  colorModuleBorderLabel =
    new ColorWidget("Module Border Color", colorLayout, colorModuleBorder, 5,
		    this, SLOT(colorModuleBorderClicked()));
  colorModuleFontLabel =
    new ColorWidget("Module Font Color", colorLayout, colorModuleFont, 7,
		    this, SLOT(colorModuleFontClicked()));
  colorCableLabel =
    new ColorWidget("Cable Color", colorLayout, colorCable, 9,
		    this, SLOT(colorCableClicked()));
  colorJackLabel =
    new ColorWidget("Jack Color", colorLayout, colorJack, 11,
		    this, SLOT(colorJackClicked()));

  QPushButton *defaultColorButton = new QPushButton("default colors");
  colorLayout->addWidget(defaultColorButton, 12, 1);
  QObject::connect(defaultColorButton, SIGNAL(clicked()), this, SLOT(defaultcolorClicked()));

  QHBoxLayout *midiModeSelectorBox = new QHBoxLayout();
  midiBox->addLayout(midiModeSelectorBox);
  //  new QWidget(midiModeSelectorBox);
  midiModeSelectorBox->addStretch();
  QLabel *midiModeLabel = new QLabel("MIDI Controller Mode: ");
  midiModeSelectorBox->addWidget(midiModeLabel);
  //  new QWidget(midiModeSelectorBox);
  QStringList midiModeNames;
  midiModeNames << "Avoid Parameter Jumps";
  midiModeNames << "Init MIDI Controller";
  midiModeNames << "Follow MIDI Controller";
  midiModeComboBox = new QComboBox();
    midiModeSelectorBox->addWidget(midiModeComboBox);
    //  new QWidget(midiModeSelectorBox);
  midiModeComboBox->addItems(midiModeNames);
  midiModeComboBox->setCurrentIndex(midiControllerMode);
  QObject::connect(midiModeComboBox, SIGNAL(highlighted(int)), this, SLOT(updateMidiMode(int)));                

  pathBox->addStretch();
  QHBoxLayout *loadPathBox = new QHBoxLayout();
  pathBox->addLayout(loadPathBox);
  QLabel *loadLabel = new QLabel(tr("Load Path:"));
  loadPathBox->addWidget(loadLabel);
  loadEdit = new QLineEdit();
  pathBox->addWidget(loadEdit);
  loadEdit->setText(loadPath);
  QPushButton *browseLoadButton = new QPushButton(tr("Browse"));
  loadPathBox->addStretch();
  loadPathBox->addWidget(browseLoadButton);

  pathBox->addStretch();
  QHBoxLayout *savePathBox = new QHBoxLayout();
  pathBox->addLayout(savePathBox);
  QLabel *saveLabel = new QLabel(tr("Save Path:"));
  savePathBox->addWidget(saveLabel);
  saveEdit = new QLineEdit();
  pathBox->addWidget(saveEdit);
  saveEdit->setText(savePath);
  QPushButton *browseSaveButton = new QPushButton(tr("Browse"));
  savePathBox->addStretch();
  savePathBox->addWidget(browseSaveButton);

  pathBox->addStretch();

  QObject::connect(browseLoadButton, SIGNAL(clicked()), this, SLOT(browseLoad()));
  QObject::connect(browseSaveButton, SIGNAL(clicked()), this, SLOT(browseSave()));
  QObject::connect(loadEdit, SIGNAL(lostFocus()), this, SLOT(loadPathUpdate()));
  QObject::connect(loadEdit, SIGNAL(returnPressed()), this, SLOT(loadPathUpdate()));
  QObject::connect(saveEdit, SIGNAL(lostFocus()), this, SLOT(savePathUpdate()));
  QObject::connect(saveEdit, SIGNAL(returnPressed()), this, SLOT(savePathUpdate()));

  QHBoxLayout *buttonContainer = new QHBoxLayout();
  vBox.addLayout(buttonContainer);
  buttonContainer->addStretch();
  QPushButton *cancelButton = new QPushButton(tr("Cancel"));
  buttonContainer->addWidget(cancelButton);
  buttonContainer->addStretch();
  QPushButton *applyButton = new QPushButton(tr("Apply"));
  buttonContainer->addWidget(applyButton);
  buttonContainer->addStretch();
  QPushButton *okButton = new QPushButton(tr("OK"));
  buttonContainer->addWidget(okButton);
  buttonContainer->addStretch();
  QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(submitPref()));  
  QObject::connect(applyButton, SIGNAL(clicked()), this, SLOT(applyPref()));  
  QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));  
}

bool PrefWidget::loadPref(QString config_fn)
{
  QFile f(config_fn);
  if (!f.open( QIODevice::ReadOnly )) {
    QMessageBox::information(this, "AlsaModularSynth",
            tr("Could not open file."));
    return false;
  }

  return loadPref(f.handle());
}

bool PrefWidget::loadPref(int rcFd)
{
  QString qs, qs2;
  int r,g,b;

  QTextStream rctext(fdopen(rcFd, "r"));
    QRegExp sep(" ");
    while (!rctext.atEnd()) {
      qs = rctext.readLine(); 
      if (qs.contains("ColorBackground")) {
        qs2 = qs.section(sep, 1, 1); 
        r = qs2.toInt();
        qs2 = qs.section(sep, 2, 2); 
        g = qs2.toInt();
        qs2 = qs.section(sep, 3, 3); 
        b = qs2.toInt();
        synthdata->colorBackground = QColor(r, g, b);
      }        
      else if (qs.contains("ColorModuleBackground")) {
        qs2 = qs.section(sep, 1, 1); 
        r = qs2.toInt();
        qs2 = qs.section(sep, 2, 2); 
        g = qs2.toInt();
        qs2 = qs.section(sep, 3, 3); 
        b = qs2.toInt();
        synthdata->colorModuleBackground = QColor(r, g, b);
      }        
      else if (qs.contains("ColorModuleBorder")) {
        qs2 = qs.section(sep, 1, 1); 
        r = qs2.toInt();
        qs2 = qs.section(sep, 2, 2); 
        g = qs2.toInt();
        qs2 = qs.section(sep, 3, 3); 
        b = qs2.toInt();
        synthdata->colorModuleBorder = QColor(r, g, b);
      }        
      else if (qs.contains("ColorModuleFont")) {
        qs2 = qs.section(sep, 1, 1); 
        r = qs2.toInt();
        qs2 = qs.section(sep, 2, 2); 
        g = qs2.toInt();
        qs2 = qs.section(sep, 3, 3); 
        b = qs2.toInt();
        synthdata->colorModuleFont = QColor(r, g, b);
        synthdata->colorPortFont1 = QColor(r, g, b);
      }        
      else if (qs.contains("ColorJack")) {
        qs2 = qs.section(sep, 1, 1); 
        r = qs2.toInt();
        qs2 = qs.section(sep, 2, 2); 
        g = qs2.toInt();
        qs2 = qs.section(sep, 3, 3); 
        b = qs2.toInt();
        synthdata->colorJack = QColor(r, g, b);
      }        
      else if (qs.contains("ColorCable")) {
        qs2 = qs.section(sep, 1, 1); 
        r = qs2.toInt();
        qs2 = qs.section(sep, 2, 2); 
        g = qs2.toInt();
        qs2 = qs.section(sep, 3, 3); 
        b = qs2.toInt();
        synthdata->colorCable = QColor(r, g, b);
      }       
      else if (qs.contains("MidiControllerMode")) {
        qs2 = qs.section(sep, 1, 1); 
        midiControllerMode = qs2.toInt();
        synthdata->midiControllerMode = midiControllerMode;
      }       
      else if (qs.contains("LoadPath")) {
        loadPath = qs.section(sep, 1, 1); 
        if (loadPath.isEmpty())
            loadPath = QDir::homePath();
        loadEdit->setText(loadPath);
        synthdata->loadPath = loadPath;
      }       
      else if (qs.contains("SavePath")) {
        savePath = qs.section(sep, 1, 1); 
        if (savePath.isEmpty())
            savePath = QDir::homePath();
        saveEdit->setText(savePath);
        synthdata->savePath = savePath;
      }       
    }   
    //  }        
  StdErr << "loadPath: " << synthdata->loadPath << ", savePath: " <<
    synthdata->savePath <<  endl;
  recallColors();
  refreshColors();
  return true;
}

void PrefWidget::savePref(int rcFd)
{
  QString qs;

//   QFile f(config_fn);
//   if (!f.open( QIODevice::WriteOnly )) {
//     QMessageBox::information( this, "AlsaModularSynth", "Could not open file.");
//   } else {
  if (ftruncate(rcFd, 0)) {
    StdErr << "Ooops in " << __FUNCTION__ << " at " << __LINE__ << endl;
    exit(-1);
  }
  lseek(rcFd, 0, SEEK_SET);
  QTextStream rctext(fdopen(rcFd, "w"));
    rctext << "ColorBackground " << synthdata->colorBackground.red() << " " << synthdata->colorBackground.green() << " " << synthdata->colorBackground.blue() << "\n";
    rctext << "ColorModuleBackground " << synthdata->colorModuleBackground.red() << " " << synthdata->colorModuleBackground.green() << " " << synthdata->colorModuleBackground.blue() << "\n";
    rctext << "ColorModuleBorder " << synthdata->colorModuleBorder.red() << " " << synthdata->colorModuleBorder.green() << " " << synthdata->colorModuleBorder.blue() << "\n";
    rctext << "ColorModuleFont " << synthdata->colorModuleFont.red() << " " << synthdata->colorModuleFont.green() << " " << synthdata->colorModuleFont.blue() << "\n";
    rctext << "ColorJack " << synthdata->colorJack.red() << " " << synthdata->colorJack.green() << " " << synthdata->colorJack.blue() << "\n";
    rctext << "ColorCable " << synthdata->colorCable.red() << " " << synthdata->colorCable.green() << " " << synthdata->colorCable.blue() << "\n";
    rctext << "MidiControllerMode " << synthdata->midiControllerMode << "\n";
    rctext << "LoadPath " << synthdata->loadPath << "\n";
    rctext << "SavePath " << synthdata->savePath << "\n";
    //    f.close();
    //  }       
}                             

void PrefWidget::submitPref()
{
  applyPref();
  close();
}

void PrefWidget::applyPref() {

  storeColors();
  emit prefChanged();
}

void PrefWidget::refreshColors() {

  colorBackgroundLabel->setPalette(QPalette(colorBackground, colorBackground));  
  colorModuleBackgroundLabel->setPalette(QPalette(colorModuleBackground, colorModuleBackground));  
  colorModuleBorderLabel->setPalette(QPalette(colorModuleBorder, colorModuleBorder));  
  colorModuleFontLabel->setPalette(QPalette(colorModuleFont, colorModuleFont));  
  colorCableLabel->setPalette(QPalette(colorCable, colorCable));  
  colorJackLabel->setPalette(QPalette(colorJack, colorJack));  
  midiModeComboBox->setCurrentIndex(midiControllerMode);
  loadEdit->setText(loadPath);
  saveEdit->setText(savePath);
}

void PrefWidget::recallColors() {

  colorBackground = synthdata->colorBackground;
  colorModuleBackground = synthdata->colorModuleBackground; 
  colorModuleBorder = synthdata->colorModuleBorder;
  colorModuleFont = synthdata->colorModuleFont;
  colorCable = synthdata->colorCable;
  colorJack = synthdata->colorJack;
  midiControllerMode = synthdata->midiControllerMode;
  loadPath = synthdata->loadPath;
  savePath = synthdata->savePath;
}
void PrefWidget::defaultcolorClicked()
{
  colorBackground = QColor(COLOR_MAINWIN_BG);
  colorModuleBackground = QColor(COLOR_MODULE_BG);
  colorModuleBorder = QColor(195, 195, 195);
  colorModuleFont = QColor(255, 255, 255);
  colorCable = QColor(180, 180, 180);
  colorJack = QColor(250, 200, 50);
  refreshColors();
}
void PrefWidget::storeColors() {

  synthdata->colorBackground = colorBackground;
  synthdata->colorModuleBackground = colorModuleBackground;
  synthdata->colorModuleBorder = colorModuleBorder;
  synthdata->colorModuleFont = colorModuleFont;
  synthdata->colorPortFont1 = colorModuleFont;
  synthdata->colorCable = colorCable;
  synthdata->colorJack = colorJack;
  synthdata->midiControllerMode = midiControllerMode;
  synthdata->loadPath = loadPath;
  synthdata->savePath = savePath;
}

void PrefWidget::colorBackgroundClicked() {

  QColor tmp;
  
  tmp = QColorDialog::getColor(colorBackground);
  if (tmp.isValid()) {
    colorBackground = tmp;
    refreshColors();
  }
}

void PrefWidget::colorModuleBackgroundClicked() {

  QColor tmp;

  tmp = QColorDialog::getColor(colorModuleBackground);
  if (tmp.isValid()) {
    colorModuleBackground = tmp;
    refreshColors();
  }
}

void PrefWidget::colorModuleBorderClicked() {

  QColor tmp;

  tmp = QColorDialog::getColor(colorModuleBorder);
  if (tmp.isValid()) {       
    colorModuleBorder = tmp;
    refreshColors();
  }
}

void PrefWidget::colorModuleFontClicked() {

  QColor tmp;

  tmp = QColorDialog::getColor(colorModuleFont);
  if (tmp.isValid()) {       
    colorModuleFont = tmp;    
    refreshColors();
  }
}

void PrefWidget::colorCableClicked() {

  QColor tmp;

  tmp = QColorDialog::getColor(synthdata->colorCable);
  if (tmp.isValid()) {       
    colorCable = tmp;    
    refreshColors();
  }
}

void PrefWidget::colorJackClicked() {

  QColor tmp;

  tmp = QColorDialog::getColor(synthdata->colorJack);
  if (tmp.isValid()) {       
    colorJack = tmp;    
    refreshColors();
  }
}

void PrefWidget::updateMidiMode(int mode) {

  midiControllerMode = mode;
}

void PrefWidget::browseLoad() {

  QString qs;

  qs = QFileDialog::getExistingDirectory(this, tr("Choose Load Path"), loadPath);
  if (qs.isEmpty())
    return;

  loadPath = qs;
  loadEdit->setText(loadPath);
}

void PrefWidget::browseSave() {

  QString qs;

  qs = QFileDialog::getExistingDirectory(this, tr("Choose Save Path"), savePath);
  if (qs.isEmpty())
    return;

  savePath = qs;
  saveEdit->setText(savePath);
}

void PrefWidget::loadPathUpdate() {

  loadPath = loadEdit->text();
}
  
void PrefWidget::savePathUpdate() {

  savePath = saveEdit->text();
}
