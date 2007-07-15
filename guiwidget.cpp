#include <stdio.h>
#include <qlabel.h>
#include <QGroupBox>
#include <qspinbox.h>
#include <QTextStream>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "guiwidget.h"
#include "midicontroller.h"
#include "midicontrollerlist.h"
#include "midiguicomponent.h"
#include "module.h"
#include "midislider.h"
#include "intmidislider.h"
#include "midicombobox.h"
#include "midicheckbox.h"
#include "midipushbutton.h"

GuiWidget::GuiWidget(QWidget* parent, const char *name) 
  : QWidget(parent)
  , vLayout(this)
{
  setObjectName(name);

  setGeometry(0, 0, GUI_DEFAULT_WIDTH, GUI_DEFAULT_HEIGHT);
  

  QWidget *presetContainer = new QWidget();
  QHBoxLayout *presetContainerLayout = new QHBoxLayout(presetContainer);
  vLayout.addWidget(presetContainer);

  QWidget *presetNameContainer = new QWidget();
  QHBoxLayout *presetNameContainerLayout = new QHBoxLayout(presetNameContainer);
  vLayout.addWidget(presetNameContainer);

  tabWidget = new QTabWidget();
  vLayout.addWidget(tabWidget);
  currentTab = NULL;
  currentGroupBox = NULL;
  currentTabIndex = 0;

  QPushButton *addPresetButton = new QPushButton("Add Preset");
  presetContainerLayout->addWidget(addPresetButton);
  QObject::connect(addPresetButton, SIGNAL(clicked()), this, SLOT(addPreset()));  
  QPushButton *overwritePresetButton =
    new QPushButton("OverwritePreset");
  presetContainerLayout->addWidget(overwritePresetButton);
  QObject::connect(overwritePresetButton, SIGNAL(clicked()),
		   this, SLOT(overwritePreset()));  
  presetCountLabel = new QLabel();
  presetCountLabel->setText("Presets for this configuration: 0");
  presetContainerLayout->addWidget(presetCountLabel);
  QPushButton *decButton = new QPushButton("-1");
  presetContainerLayout->addWidget(decButton);
  QObject::connect(decButton, SIGNAL(clicked()), this, SLOT(presetDec()));
  QPushButton *incButton = new QPushButton("+1");
  presetContainerLayout->addWidget(incButton);
  QObject::connect(incButton, SIGNAL(clicked()), this, SLOT(presetInc()));

  presetLabel = new QLabel();
  presetNameContainerLayout->addWidget(presetLabel);
  presetName = new QLineEdit();
  presetNameContainerLayout->addWidget(presetName);

  setPresetCount(0);
  setCurrentPreset(0);
}

int GuiWidget::addFrame(const QString &frameName)
{
//  printf("Adding frame %s.\n", frameName.latin1());
  if (!currentTab)
    return -1;

  frameNameList.append(frameName);
  QGroupBox *gbox = new QGroupBox(frameName);
  gbox->setObjectName(frameName);
  currentTab->addWidget(gbox);
  QVBoxLayout *vbox = new QVBoxLayout(gbox);

  GuiFrame *guiFrame = new GuiFrame;
  guiFrame->frameBox = vbox;
  guiFrame->tabIndex = currentTabIndex;
  frameBoxList.append(guiFrame);
  currentGroupBox = vbox;
  return(0);  
}

int GuiWidget::setFrame(int index) {

//  fprintf(stderr, "Setting frame index %d.\n", index);  
  currentGroupBox = frameBoxList.at(index)->frameBox;
  return(0);  
}

int GuiWidget::addTab(const QString &tabName) {
  
//  printf("Adding tab %s.\n", tabName.latin1());
  tabNameList.append(tabName);
  QWidget *tab = new QWidget();
  setObjectName(tabName);
  currentTab = new QHBoxLayout(tab);
  currentTabIndex = tabNameList.count() - 1;
  tabWidget->insertTab(-1, tab, tabName);
  tabList.append(currentTab);
  return(0);  
}

int GuiWidget::setTab(int index) {

//   fprintf(stderr, "Setting tab index %d.\n", index);  
  currentTab = tabList.at(index);
  currentTabIndex = index;
  return(0);  
}

int GuiWidget::addParameter(MidiControllableBase *mcAble, const QString &parameterName)
{
  if (!currentGroupBox)
    return -1;

  clearPresets();

  MidiGUIcomponent *mgc = dynamic_cast<MidiGUIcomponent *>(mcAble->mcws.at(0))->createTwin();
  currentGroupBox->addWidget(mgc);
  currentGroupBox->addStretch(100);
  parameterList.append(mcAble);
  mgcs.append(mgc);
  mgc->nameLabel.setText(parameterName);

  currentGroupBox->parentWidget()->show();

  return 0;
}

int GuiWidget::setPresetCount(int count) {

  QString qs;

  presetCount = count;
  qs.sprintf("Presets for this configuration: %d", presetCount);
  presetCountLabel->setText(qs);
  return 0;
}

int GuiWidget::setCurrentPreset(int presetNum, bool rt)
{
  int index, value;

  if (presetCount == 0)
    currentPreset = 0;

  if (presetNum < 0 || presetNum >= presetCount)
    return -1;

  currentPreset = presetNum;

  for (index = 0; index < presetList[currentPreset].count(); index++) {
    value = presetList[currentPreset][index];
     //!!    parameterList.at(index)->invalidateController();
    if (rt) {
      parameterList.at(index)->setValRT(value);
      synthdata->mcSet.put(parameterList.at(index));
    } else {
      //      StdOut << currentPreset << " " << parameterList.at(index)->name << " " << index << " " << value << " " << endl;
      parameterList.at(index)->setVal(value, NULL);
    }
  }

  if (!rt)
    setCurrentPresetText();

  return 0;
}

void GuiWidget::setCurrentPresetText()
{
  QString qs;
  QTextStream ts(&qs);

  ts << "Preset " << currentPreset << ":";
  presetLabel->setText(qs);

  for (QStringList::Iterator it = presetNameList.begin(); it != presetNameList.end(); it++) {
    qs = (*it).mid(0, 3);
    if (qs.toInt() == currentPreset) {
      qs = (*it).mid(3);
      presetName->setText(qs);
    }
  }
}

void GuiWidget::presetDec() {

  if (currentPreset > 0) {
    setCurrentPreset(currentPreset - 1);
  }
}

void GuiWidget::presetInc() {

  if (currentPreset < presetCount - 1) {
    setCurrentPreset(currentPreset + 1);
  }
}

void GuiWidget::addPreset() {
 
  QString qs;

  if (presetCount) {
    setPresetCount(presetCount + 1);
    setCurrentPreset(presetCount - 1);
  }
  qs.sprintf("%3d", currentPreset);
  presetNameList.append(qs+presetName->text());
  overwritePreset();
}

void GuiWidget::overwritePreset()
{
  int l1, value;
  QString qs;

  if (!presetCount)
    setPresetCount(presetCount + 1);

  presetList[currentPreset].clear();
  for (l1 = 0; l1 < parameterList.count(); l1++) {
    value = parameterList.at(l1)->sliderVal();
    presetList[currentPreset].append(value);
  } 
  for (QStringList::Iterator it = presetNameList.begin(); it != presetNameList.end(); it++) {
    qs = (*it).mid(0, 3);
    if (qs.toInt() == currentPreset) {
      qs.sprintf("%3d", currentPreset);
      *it = qs+presetName->text();
    }
  }
}

void GuiWidget::clearPresets() {

  int l1; 

  for (l1 = 0; l1 < MAX_PRESETS; l1++) {
    presetList[l1].clear();
  }
  presetNameList.clear();
  presetName->setText(" ");
  setPresetCount(0);
}

void GuiWidget::clearGui() {

  delete tabWidget;
  frameBoxList.clear();
  tabList.clear();
  parameterList.clear();
  presetNameList.clear();
  presetName->setText(" ");
  frameNameList.clear();
  tabNameList.clear();
  tabWidget = new QTabWidget();
  vLayout.addWidget(tabWidget);
  tabWidget->show();
  setPresetCount(0);
  setCurrentPreset(0);
}

void GuiWidget::refreshGui() {

  tabWidget->hide();
  tabWidget->show();
}

void GuiWidget::remove(MidiControllableBase *mcAble)
{
  int index = parameterList.indexOf(mcAble);
  if (index < 0)
    return;

  MidiGUIcomponent *mgc = mgcs.takeAt(index);
  delete mgc;

  for (int ps = 0; ps < presetCount; ps++)
    presetList[ps].removeAt(index);

  parameterList.removeAt(index);
}
