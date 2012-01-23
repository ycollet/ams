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

#include "colorwidget.h"
#include "synthdata.h"
#include "prefwidget.h"



PrefWidget::PrefWidget(): vBox(this)
{
    setGeometry(0, 0, PREF_DEFAULT_WIDTH, PREF_DEFAULT_HEIGHT);
    vBox.setMargin(10);
    vBox.setSpacing(5);
    
    /*set default path values*/
    colorPath = QDir::homePath();
    patchPath = colorPath;

    recallColors();

    tabWidget = new QTabWidget();
    vBox.addWidget(tabWidget);

    QWidget *colorWidget = new QWidget();
    QVBoxLayout *colorBox = new QVBoxLayout(colorWidget);
    QWidget *midiWidget = new QWidget();
    QVBoxLayout *midiBox = new QVBoxLayout(midiWidget);
    QWidget *pathWidget = new QWidget();
    QVBoxLayout *pathBox = new QVBoxLayout(pathWidget);
    QWidget *editingWidget = new QWidget();
    QVBoxLayout *editingBox = new QVBoxLayout(editingWidget);

    tabWidget->addTab(colorWidget, tr("&Colors"));
    tabWidget->addTab(midiWidget, tr("&MIDI"));
    tabWidget->addTab(pathWidget, tr("&Paths"));
    tabWidget->addTab(editingWidget, tr("&Editing"));


    /*Color tab*/
    QGridLayout *colorLayout = new QGridLayout();
    colorBox->addLayout(colorLayout);

    // QLabel *label1 = new QLabel("Background Color", colorGridWidget);
    colorBackgroundLabel = new ColorWidget(tr("Background color"),
            colorLayout, colorBackground, 1, this,
            SLOT(colorBackgroundClicked()));

    colorModuleBackgroundLabel = new ColorWidget(tr("Module Background color"),
            colorLayout, colorModuleBackground, 3, this,
            SLOT(colorModuleBackgroundClicked()));

    colorModuleBorderLabel = new ColorWidget(tr("Module Border color"),
            colorLayout, colorModuleBorder, 5, this,
            SLOT(colorModuleBorderClicked()));

    colorModuleFontLabel = new ColorWidget(tr("Module font color"),
            colorLayout, colorModuleFont, 7, this,
            SLOT(colorModuleFontClicked()));

    colorCableLabel = new ColorWidget(tr("Cable color"),
            colorLayout, colorCable, 9, this,
            SLOT(colorCableClicked()));

    colorJackLabel = new ColorWidget(tr("Jack color"),
            colorLayout, colorJack, 11, this,
            SLOT(colorJackClicked()));

    QPushButton *defaultColorButton = new QPushButton(tr("&Default colors"));
    colorLayout->addWidget(defaultColorButton, 12, 1);
    QObject::connect(defaultColorButton, SIGNAL(clicked()), this,
            SLOT(defaultcolorClicked()));

    colorBox->addStretch();

    
    /*Midi tab*/
    QHBoxLayout *midiModeSelectorBox = new QHBoxLayout();
    midiBox->addLayout(midiModeSelectorBox);
    QLabel *midiModeLabel = new QLabel(tr("M&IDI controller mode: "));
    midiModeSelectorBox->addWidget(midiModeLabel);
    midiModeSelectorBox->addStretch();
    QStringList midiModeNames;
    midiModeNames << tr("Avoid parameter jumps");
    midiModeNames << tr("Init MIDI controller");
    midiModeNames << tr("Follow MIDI controller");
    midiModeComboBox = new QComboBox();
    midiModeSelectorBox->addWidget(midiModeComboBox);
    midiModeLabel->setBuddy(midiModeComboBox);
    //  new QWidget(midiModeSelectorBox);
    midiModeComboBox->addItems(midiModeNames);
    midiModeComboBox->setCurrentIndex(midiControllerMode);
    QObject::connect(midiModeComboBox, SIGNAL(highlighted(int)),
            this, SLOT(updateMidiMode(int)));                

    midiBox->addStretch();

    /*Path tab*/
    QHBoxLayout *colorPathBox = new QHBoxLayout();
    pathBox->addLayout(colorPathBox);
    QLabel *loadLabel = new QLabel(tr("&Color path:"));
    colorPathBox->addWidget(loadLabel);
    loadEdit = new QLineEdit();
    pathBox->addWidget(loadEdit);
    loadEdit->setText(colorPath);
    loadLabel->setBuddy(loadEdit);
    QPushButton *browseColorButton = new QPushButton(tr("&Browse..."));
    colorPathBox->addStretch();
    colorPathBox->addWidget(browseColorButton);

    pathBox->addSpacing(10);

    QHBoxLayout *patchPathBox = new QHBoxLayout();
    pathBox->addLayout(patchPathBox);
    QLabel *saveLabel = new QLabel(tr("&Patch path:"));
    patchPathBox->addWidget(saveLabel);
    saveEdit = new QLineEdit();
    pathBox->addWidget(saveEdit);
    saveEdit->setText(patchPath);
    saveLabel->setBuddy(saveEdit);
    QPushButton *browsePatchButton = new QPushButton(tr("Bro&wse..."));
    patchPathBox->addStretch();
    patchPathBox->addWidget(browsePatchButton);

    pathBox->addStretch();

    QObject::connect(browseColorButton, SIGNAL(clicked()),
            this, SLOT(browseColor()));
    QObject::connect(browsePatchButton, SIGNAL(clicked()),
            this, SLOT(browsePatch()));
    QObject::connect(loadEdit, SIGNAL(lostFocus()),
            this, SLOT(colorPathUpdate()));
    QObject::connect(loadEdit, SIGNAL(returnPressed()),
            this, SLOT(colorPathUpdate()));
    QObject::connect(saveEdit, SIGNAL(lostFocus()),
            this, SLOT(patchPathUpdate()));
    QObject::connect(saveEdit, SIGNAL(returnPressed()),
            this, SLOT(patchPathUpdate()));

    /*Editing tab*/
    QHBoxLayout *editingModeSelectorBox = new QHBoxLayout();
    editingBox->addLayout(editingModeSelectorBox);
    QLabel *editingModeLabel = new QLabel(tr("Box movement:"));
    editingModeSelectorBox->addWidget(editingModeLabel);
    editingModeSelectorBox->addStretch();
    QStringList editingModeNames;
    editingModeNames << tr("Standard");
    editingModeNames << tr("No topleft border");
    editingModeComboBox = new QComboBox();
    editingModeSelectorBox->addWidget(editingModeComboBox);
    editingModeLabel->setBuddy(midiModeComboBox);
    editingModeComboBox->addItems(editingModeNames);
    QObject::connect(editingModeComboBox, SIGNAL(highlighted(int)),
            this, SLOT(updateEditingMode(int)));                
    editingBox->addStretch();


    /*Button line*/
    QHBoxLayout *buttonContainer = new QHBoxLayout();
    buttonContainer->addStretch();
    vBox.addLayout(buttonContainer);
    QPushButton *applyButton = new QPushButton(tr("&Apply"));
    buttonContainer->addWidget(applyButton);
    QPushButton *okButton = new QPushButton(tr("OK"));
    buttonContainer->addWidget(okButton);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    buttonContainer->addWidget(cancelButton);

    QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(submitPref()));  
    QObject::connect(applyButton, SIGNAL(clicked()), this, SLOT(applyPref()));  
    QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));  
}


void PrefWidget::loadPref(QString& line)
{
    int r, g, b;

    if (line.startsWith("ColorBackground")) {
        r = line.section(' ', 1, 1).toInt();
        g = line.section(' ', 2, 2).toInt();
        b = line.section(' ', 3, 3).toInt();
        synthdata->colorBackground = QColor(r, g, b);
    }        
    else if (line.startsWith("ColorModuleBackground")) {
        r = line.section(' ', 1, 1).toInt();
        g = line.section(' ', 2, 2).toInt();
        b = line.section(' ', 3, 3).toInt();
        synthdata->colorModuleBackground = QColor(r, g, b);
    }        
    else if (line.startsWith("ColorModuleBorder")) {
        r = line.section(' ', 1, 1).toInt();
        g = line.section(' ', 2, 2).toInt();
        b = line.section(' ', 3, 3).toInt();
        synthdata->colorModuleBorder = QColor(r, g, b);
    }        
    else if (line.startsWith("ColorModuleFont")) {
        r = line.section(' ', 1, 1).toInt();
        g = line.section(' ', 2, 2).toInt();
        b = line.section(' ', 3, 3).toInt();
        synthdata->colorModuleFont = QColor(r, g, b);
        synthdata->colorPortFont1 = QColor(r, g, b);
    }        
    else if (line.startsWith("ColorJack")) {
        r = line.section(' ', 1, 1).toInt();
        g = line.section(' ', 2, 2).toInt();
        b = line.section(' ', 3, 3).toInt();
        synthdata->colorJack = QColor(r, g, b);
    }        
    else if (line.startsWith("ColorCable")) {
        r = line.section(' ', 1, 1).toInt();
        g = line.section(' ', 2, 2).toInt();
        b = line.section(' ', 3, 3).toInt();
        synthdata->colorCable = QColor(r, g, b);
    }       
    else if (line.startsWith("MidiControllerMode")) {
        midiControllerMode = line.section(' ', 1, 1).toInt();
        synthdata->midiControllerMode = midiControllerMode;
    }       
    else if (line.startsWith("ColorPath")) {
        colorPath = line.section(' ', 1); 
        if (colorPath.isEmpty())
            colorPath = QDir::homePath();
        loadEdit->setText(colorPath);
        synthdata->colorPath = colorPath;
    }       
    else if (line.startsWith("PatchPath")) {
        patchPath = line.section(' ', 1); 
        if (patchPath.isEmpty())
            patchPath = QDir::homePath();
        saveEdit->setText(patchPath);
        synthdata->patchPath = patchPath;
    }       
    else if (line.startsWith("EditingFlags")) {
	synthdata->editingFlags.f = line.section(' ', 1).toInt();
	editingMode = synthdata->editingFlags.crossTopLeft();
    }       
}


void PrefWidget::savePref(QTextStream& rctext)
{
    rctext << "ColorBackground "
        << synthdata->colorBackground.red() << " "
        << synthdata->colorBackground.green() << " "
        << synthdata->colorBackground.blue() << "\n";
    rctext << "ColorModuleBackground "
        << synthdata->colorModuleBackground.red() << " "
        << synthdata->colorModuleBackground.green() << " "
        << synthdata->colorModuleBackground.blue() << "\n";
    rctext << "ColorModuleBorder "
        << synthdata->colorModuleBorder.red() << " "
        << synthdata->colorModuleBorder.green() << " "
        << synthdata->colorModuleBorder.blue() << "\n";
    rctext << "ColorModuleFont "
        << synthdata->colorModuleFont.red() << " "
        << synthdata->colorModuleFont.green() << " "
        << synthdata->colorModuleFont.blue() << "\n";
    rctext << "ColorJack "
        << synthdata->colorJack.red() << " "
        << synthdata->colorJack.green() << " "
        << synthdata->colorJack.blue() << "\n";
    rctext << "ColorCable "
        << synthdata->colorCable.red() << " "
        << synthdata->colorCable.green() << " "
        << synthdata->colorCable.blue() << "\n";
    rctext << "MidiControllerMode " << synthdata->midiControllerMode << "\n";
    rctext << "ColorPath " << synthdata->colorPath << "\n";
    rctext << "PatchPath " << synthdata->patchPath << "\n";
    rctext << "EditingFlags " << synthdata->editingFlags.f << "\n";
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
  colorModuleBackgroundLabel->setPalette(QPalette(
              colorModuleBackground, colorModuleBackground));  
  colorModuleBorderLabel->setPalette(QPalette(
              colorModuleBorder, colorModuleBorder));  
  colorModuleFontLabel->setPalette(QPalette(
              colorModuleFont, colorModuleFont));  
  colorCableLabel->setPalette(QPalette(colorCable, colorCable));  
  colorJackLabel->setPalette(QPalette(colorJack, colorJack));  
  midiModeComboBox->setCurrentIndex(midiControllerMode);
  editingModeComboBox->setCurrentIndex(editingMode);
  loadEdit->setText(colorPath);
  saveEdit->setText(patchPath);
}

void PrefWidget::recallColors() {

  colorBackground = synthdata->colorBackground;
  colorModuleBackground = synthdata->colorModuleBackground; 
  colorModuleBorder = synthdata->colorModuleBorder;
  colorModuleFont = synthdata->colorModuleFont;
  colorCable = synthdata->colorCable;
  colorJack = synthdata->colorJack;
  midiControllerMode = synthdata->midiControllerMode;
  editingMode = synthdata->editingFlags.crossTopLeft();
  colorPath = synthdata->colorPath;
  patchPath = synthdata->patchPath;
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
  synthdata->editingFlags.setCrossTopLeft(editingMode);
  synthdata->colorPath = colorPath;
  synthdata->patchPath = patchPath;
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

void PrefWidget::updateEditingMode(int mode)
{
  editingMode = mode;
}

void PrefWidget::browseColor() {

  QString qs;

  qs = QFileDialog::getExistingDirectory(this, tr("Choose color path"),
          colorPath);
  if (qs.isEmpty())
    return;

  colorPath = qs;
  loadEdit->setText(colorPath);
}

void PrefWidget::browsePatch() {

  QString qs;

  qs = QFileDialog::getExistingDirectory(this, tr("Choose patch path"),
          patchPath);
  if (qs.isEmpty())
    return;

  patchPath = qs;
  saveEdit->setText(patchPath);
}

void PrefWidget::colorPathUpdate() {

  colorPath = loadEdit->text();
}
  
void PrefWidget::patchPathUpdate() {

  patchPath = saveEdit->text();
}
