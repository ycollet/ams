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
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qstringlist.h>
#include <qlineedit.h>
#include <QTreeWidget>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "midiwidget.h"
#include "midicontroller.h"
#include "midicontrollerlist.h"
#include "midiguicomponent.h"
#include "module.h"
#include "midislider.h"
#include "intmidislider.h"
#include "floatintmidislider.h"
#include "midicombobox.h"
#include "midicheckbox.h"
#include "midipushbutton.h"
#include "guiwidget.h"

#include <iostream>
//using namespace std;


int MidiControllerModel::rowCount(const QModelIndex &parent) const
{
  if (!parent.isValid())
    return midiControllerList.count();

  const MidiController *c = (const MidiController *)parent.internalPointer();
  if (!c)
    return midiControllerList.at(parent.row()).
      context->mgcList.count();

  return 0;
}

QVariant MidiControllerModel::data(const QModelIndex &index, int role) const
{
  if (index.isValid() && role == Qt::DisplayRole) {
    const MidiController *c = (const MidiController *)index.internalPointer();
    MidiGUIcomponent *mgc = NULL;
    if (c)
      mgc = c->context->mgcList.at(index.row());

    if (mgc) {
      if (index.column())
	return mgc->parentModule->configDialog->windowTitle();
      else
	return mgc->objectName();
    } else
      if (!c && !index.column()) {
	QString qs;
	c = &midiControllerList.at(index.row());
	return qs.sprintf("type: %d channel: %d param: %d",
			  c->type(), c->ch(), c->param());
      }
  }
  return QVariant();
}

QVariant MidiControllerModel::headerData(int section, Qt::Orientation orientation,
					 int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return section ? "Module" : "MIDI Controller / Parameter";
  
  return QVariant();
}

QModelIndex MidiControllerModel::index(int row, int column,
				       const QModelIndex &parent) const
{
  if (parent.isValid())
    return createIndex(row, column,
		       (void*)&midiControllerList.at(parent.row()));
  else
    return createIndex(row, column, (void*)NULL);
}

QModelIndex MidiControllerModel::parent(const QModelIndex &child) const
{
  if (child.isValid() && child.internalPointer()) {
    const MidiController *c = (const MidiController *)child.internalPointer();
    int row = c - midiControllerList.data();
    return index(row, 0);
  }
  return QModelIndex();
}

int MidiControllerModel::columnCount(const QModelIndex &/*parent*/) const
{
  return 2;
}

void MidiControllerModel::insert(QVector<MidiController>::iterator c,
				 MidiController &mc)
{
  int row = &*c - midiControllerList.data();
  beginInsertRows(QModelIndex(), row, row);
  QVector<MidiController>::iterator n = midiControllerList.insert(c, mc);
  n->context = new MidiControllerContext();
  endInsertRows();
}

int ModuleModel::rowCount(const QModelIndex &parent) const
{
  if (!parent.isValid())
    return list.count();

  Module *m = dynamic_cast<Module *>((QWidget *)parent.internalPointer());
  if (m)
    return m->configDialog->midiGUIcomponentList.count();

  return 0;
}

QVariant ModuleModel::data(const QModelIndex &index, int role) const
{
  if (index.isValid() && role == Qt::DisplayRole) {
    MidiGUIcomponent *mgc = dynamic_cast<MidiGUIcomponent *>((QWidget *)index.internalPointer());
    if (mgc) {
      if (index.column())
	return mgc->midiSign == 1 ? "1" : "-1";
      else
	return mgc->objectName();
    }
    Module *m = dynamic_cast<Module *>((QWidget *)index.internalPointer());
    if (m && !index.column())
      return m->configDialog->windowTitle();
  }
  return QVariant();
}

QVariant ModuleModel::headerData(int section, Qt::Orientation orientation,
				 int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return section ? "MIDI Sign" : "Module / Parameter";
  
  return QVariant();
}

QModelIndex ModuleModel::index(int row, int column, const QModelIndex &parent) const
{
  if (parent.isValid()) {
    Module *m = (Module *)parent.internalPointer();
    return createIndex(row, column, m->configDialog->midiGUIcomponentList.at(row));
  } else
    return createIndex(row, column, list.at(row));
}

QModelIndex ModuleModel::parent(const QModelIndex &child) const
{
  if (child.isValid()) {
    MidiGUIcomponent *mgc = dynamic_cast<MidiGUIcomponent *>((QWidget *)child.internalPointer());
    if (mgc) {
      Module *m = mgc->parentModule;
      int row = list.indexOf(m);
      if (row >= 0)
	return createIndex(row, 0, m);
    }
  }
  return QModelIndex();
}

int ModuleModel::columnCount(const QModelIndex &/*parent*/) const
{
  return 2;
}


MidiWidget::MidiWidget(QWidget* parent, const char *name) 
  : QWidget(parent)
  , vbox(this)
  , midiControllerModel(midiControllerList)
  , selectedControl(-1)
  , midiGUIcomponent(NULL)
{
  setObjectName(name);
  int l1;
  QString qs;

  vbox.setMargin(10);
  vbox.setSpacing(5);
  
  noteControllerEnabled = false;
  followConfig = false;
  followMidi = false;
  firstBindingMightHaveChanged = false;
  QSplitter *listViewBox = new QSplitter();
  vbox.addWidget(listViewBox, 2);
  midiControllerListView = new QTreeView(listViewBox);
  midiControllerListView->setModel(&midiControllerModel);
  midiControllerListView->setAllColumnsShowFocus(true);
  moduleListView = new QTreeView(listViewBox);
  moduleListView->setModel(&moduleModel);
  moduleListView->setAllColumnsShowFocus(true);
  QObject::connect(moduleListView->selectionModel(),
		   SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
		   this,
		   SLOT(guiControlChanged(const QItemSelection &, const QItemSelection &)));
  QObject::connect(midiControllerListView->selectionModel(),
		   SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
		   this,
		   SLOT(midiControlChanged(const QItemSelection &, const QItemSelection &)));
  QVBoxLayout *controlFrame = new QVBoxLayout();
  controlFrame->setSpacing(5);
//  controlFrame->setMargin(5);
  vbox.addLayout(controlFrame);
  guiControlParent = new QFrame(); // QVBoxLayout
  controlFrame->addWidget(guiControlParent);
  guiControlParent->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  currentGUIcontrol = new QVBoxLayout(guiControlParent);
  currentGUIcontrol->setMargin(5);
  //  currentGUIcontrol = NULL;
  QHBoxLayout *checkbuttonBox = new QHBoxLayout();
  controlFrame->addLayout(checkbuttonBox);
  checkbuttonBox->setSpacing(10);
  checkbuttonBox->setMargin(5);
  QStringList channelNames;
  channelNames << "Omni";
  for (l1 = 1; l1 < 17; l1++) {
    qs.sprintf("%4d", l1);
    channelNames << qs;
  }
  QHBoxLayout *midiChannelBox = new QHBoxLayout();
  controlFrame->addLayout(midiChannelBox);
  QLabel *channelText = new QLabel();
  midiChannelBox->addWidget(channelText);
  channelText->setText("MIDI Channel:");
  QComboBox *comboBox = new QComboBox();
  midiChannelBox->addWidget(comboBox);
  midiChannelBox->addStretch();
  midiChannelBox->addStretch();
  midiChannelBox->addStretch();
  comboBox->addItems(channelNames);
  comboBox->setFixedSize(comboBox->sizeHint());
  QObject::connect(comboBox, SIGNAL(highlighted(int)), this, SLOT(updateMidiChannel(int)));
  midiChannelBox->addStretch();
  addGuiButton = new QPushButton("Add to Parameter View");
  addGuiButton->setEnabled(false);
  midiChannelBox->addWidget(addGuiButton);
  QObject::connect(addGuiButton, SIGNAL(clicked()), this, SLOT(addToParameterViewClicked()));
  midiChannelBox->addStretch();
  QHBoxLayout *buttonBox = new QHBoxLayout();
  controlFrame->addLayout(buttonBox);
  buttonBox->setSpacing(5);
  buttonBox->setMargin(5);
  buttonBox->addStretch();
  noteCheck = new QCheckBox("Enable note events");
  checkbuttonBox->addWidget(noteCheck);
  noteCheck->setChecked(noteControllerEnabled);
  configCheck = new QCheckBox("Follow Configuration Dialog");
  checkbuttonBox->addWidget(configCheck);
  configCheck->setChecked(followConfig);
  midiCheck = new QCheckBox("Follow MIDI");
  checkbuttonBox->addWidget(midiCheck);
  midiCheck->setChecked(followMidi);         
  QObject::connect(noteCheck, SIGNAL(clicked()), this, SLOT(noteControllerCheckToggle()));
  QObject::connect(configCheck, SIGNAL(clicked()), this, SLOT(configCheckToggle()));
  QObject::connect(midiCheck, SIGNAL(clicked()), this, SLOT(midiCheckToggle()));
  buttonBox->addStretch();
  bindButton = new QPushButton("Bind");
  bindButton->setEnabled(false);
  buttonBox->addWidget(bindButton);
  buttonBox->addStretch();
  clearButton = new QPushButton("Clear Binding");
  buttonBox->addWidget(clearButton);
  clearButton->setEnabled(false);
  buttonBox->addStretch();
  clearAllButton = new QPushButton("Clear All");
  buttonBox->addWidget(clearAllButton);
  buttonBox->addStretch();
  midiSignButton = new QPushButton("Toggle MIDI Sign");
  buttonBox->addWidget(midiSignButton);
  midiSignButton->setEnabled(false);
  buttonBox->addStretch();
  QObject::connect(bindButton, SIGNAL(clicked()), this, SLOT(bindClicked()));
  QObject::connect(clearButton, SIGNAL(clicked()), this, SLOT(clearClicked()));
  QObject::connect(clearAllButton, SIGNAL(clicked()), this, SLOT(clearAllClicked()));
  QObject::connect(midiSignButton, SIGNAL(clicked()), this, SLOT(toggleMidiSign()));
}

MidiWidget::~MidiWidget() {
}

void MidiWidget::clearAllClicked() {

  int l1, l2;  
  Module *module;

  for (l1 = 0; l1 < synthdata->moduleList.count(); l1++) {
    module = synthdata->moduleList.at(l1);
    for (l2 = 0; l2 < module->configDialog->midiGUIcomponentList.count(); l2++)
      module->configDialog->midiGUIcomponentList.at(l2)->midiControllerList.clear();
  }
  midiControllerList.resize(0);
  midiControllerListView->reset();
}

void MidiWidget::addMidiController(MidiControllerKey mck)
{
  MidiController mc(mck.getKey());
  typeof(midiControllerList.end()) c(midiControllerList.end());
  if (!midiControllerList.empty()) {
    c = qLowerBound(midiControllerList.begin(), midiControllerList.end(), mck);
    if (c != midiControllerList.end()) {
//       std::cout << (int)c->type() << "==" << (int)mc.type() << "&&" << (int)c->ch() << " == " << (int)mc.ch() << " && " << (int)c->param() << "=="<< (int)mc.param() << std::endl;
      if (*c == mc) {
// 	std::cout  << &*c - midiControllerList.data() << std::endl;
	return;
      }
    }
  }
//   std::cout  << (int)mc.type() << " " << (int)mc.ch() << " " << (int)mc.param() << std::endl;
  midiControllerModel.insert(c, mc);
  midiControllerListView->resizeColumnToContents(0);
}

void MidiWidget::addMidiGuiComponent(MidiControllerKey mck,
				     MidiGUIcomponent *midiGuiComponent)
{
  typeof(midiControllerList.constEnd()) c
    = qBinaryFind(midiControllerList.constBegin(),
		  midiControllerList.constEnd(), mck);
  if (c == midiControllerList.end()) {
    std::cerr  << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
    return;
  }

  int row = &*c - midiControllerList.data();
  int childRow = c->context->mgcList.count();
  midiControllerModel.beginInsertRows(midiControllerModel.index(row, 0),
				      childRow, childRow);
  QObject::connect(c->context, SIGNAL(midiValueChanged(int)),
		   midiGuiComponent, SLOT(midiValueChanged(int))); 
  c->context->mgcList.append(midiGuiComponent);
  midiControllerModel.endInsertRows();  
  moduleListView->resizeColumnToContents(0);
}

void MidiWidget::removeMidiGuiComponent(MidiControllerKey mck, MidiGUIcomponent *midiGuiComponent)
{
  typeof(midiControllerList.constEnd()) c
    = qBinaryFind(midiControllerList.constBegin(),
		  midiControllerList.constEnd(), mck);
  if (c == midiControllerList.end()) {
    std::cerr  << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
    return;
  }

  int row = &*c - midiControllerList.data();
  int childRow = c->context->mgcList.indexOf(midiGuiComponent);
  if (childRow != -1) {
    midiControllerModel.beginRemoveRows(midiControllerModel.index(row, 0),
					childRow, childRow);
    QObject::disconnect(c->context, SIGNAL(midiValueChanged(int)),
			midiGuiComponent, SLOT(midiValueChanged(int)));
    c->context->mgcList.removeAll(midiGuiComponent);
    midiControllerModel.endRemoveRows();  
  }
}

void MidiWidget::clearClicked()
{
  if (selectedController.isValid() && selectedControl != -1) {
    typeof(midiControllerList.constEnd()) c
      = qBinaryFind(midiControllerList.constBegin(),
		    midiControllerList.constEnd(), selectedController);
    if (c == midiControllerList.end()) {
      std::cerr  << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
      return;
    }

    MidiGUIcomponent *mgc = c->context->mgcList.at(selectedControl);
    mgc->disconnectController(selectedController);
  }
  firstBindingMightHaveChanged = true;
}

void MidiWidget::addToParameterViewClicked() {

  QString qs, qs2, qs3;
  bool ok, foundFrameName, foundTabName;
  int l1, frameIndex, tabIndex;

  if (!midiGUIcomponent)
    return;

  if (synthdata->guiWidget->presetCount > 0) {
    qs.sprintf("This will erase all presets for this configuration. Continue ?");
    QMessageBox questionContinue("AlsaModularSynth", qs, QMessageBox::NoIcon,
                                 QMessageBox::Yes | QMessageBox::Default, QMessageBox::No  | QMessageBox::Escape, QMessageBox::NoButton);
    if (questionContinue.exec() == QMessageBox::No) {
      return;
    }
  }
  qs = QInputDialog::getText(this, "AlsaModularSynth", "Add this parameter to frame:", QLineEdit::Normal, currentFrameName, &ok);
  currentFrameName = qs;
  if (qs.isEmpty()) {
    return;
  }
  foundFrameName = false;
  frameIndex = 0;
  if ((l1 =synthdata->guiWidget->frameNameList.indexOf(qs.trimmed())) >= 0) {
    foundFrameName = true;
    frameIndex = l1;
  }
  if (!foundFrameName) {
    qs2 = "Frame " + qs + " does not exist. Create ?"; 
    QMessageBox question("AlsaModularSynth", qs2, QMessageBox::NoIcon, QMessageBox::Yes | QMessageBox::Default,
                         QMessageBox::No  | QMessageBox::Escape, QMessageBox::NoButton);
    if (question.exec() == QMessageBox::Yes) {
      qs3 = QInputDialog::getText(this, "AlsaModularSynth", "Add frame to tab:", QLineEdit::Normal, currentTabName, &ok);
      currentTabName = qs3;
      foundTabName = false;
      tabIndex = 0;
      if ((l1 =synthdata->guiWidget->tabNameList.indexOf(qs3.trimmed())) >= 0) {
        foundTabName = true;
        tabIndex = l1;
        synthdata->guiWidget->setTab(tabIndex);
      } else {
        qs2 = "Tab " +qs3 + " does not exist. Create ?";
        QMessageBox question("AlsaModularSynth", qs2, QMessageBox::NoIcon, QMessageBox::Yes | QMessageBox::Default,
                             QMessageBox::No  | QMessageBox::Escape, QMessageBox::NoButton);
        if (question.exec() == QMessageBox::Yes) {
	  //printf("Creating tab %s.\n", qs3.latin1());
          synthdata->guiWidget->addTab(qs3.trimmed());
        } else {
          return;
        }
      }
      //      printf("Creating frame %s.\n", qs.latin1());
      synthdata->guiWidget->addFrame(qs.trimmed());
    } else {
      return;
    }
  } else
    synthdata->guiWidget->setFrame(frameIndex);

  //  qs2.sprintf("%s  ID %d", midiGUIcomponent->objectName(), module->moduleID);
  qs2 = midiGUIcomponent->objectName() + "  ID " +
    QString().setNum(midiGUIcomponent->parentModule->moduleID);
  qs = QInputDialog::getText(this, "AlsaModularSynth", "Parameter name:", QLineEdit::Normal, qs2, &ok);
  synthdata->guiWidget->addParameter(midiGUIcomponent, qs);
}

void MidiWidget::bindClicked()
{
  if (midiGUIcomponent && selectedController.isValid() && selectedControl == -1)
    midiGUIcomponent->connectToController(selectedController);

  firstBindingMightHaveChanged = true;
}

void MidiWidget::noteControllerCheckToggle() {

  noteControllerEnabled = noteCheck->isChecked();  
}

void MidiWidget::configCheckToggle() {

  followConfig = configCheck->isChecked();  
}

void MidiWidget::midiCheckToggle() {

  followMidi = midiCheck->isChecked();  
}

void MidiWidget::addModule(Module *m)
{
  if (!m->configDialog->midiGUIcomponentList.count())
    return;

  int row = moduleModel.list.count();

  moduleModel.beginInsertRows(QModelIndex(), row, row);
  moduleModel.list.append(m);
  moduleModel.endInsertRows();
}  

void MidiWidget::removeModule(Module *m)
{
  synthdata->moduleList.removeAll(m);
  synthdata->decModuleCount();

  int row = moduleModel.list.indexOf(m);
  if (row == -1)
    return;

  moduleModel.beginRemoveRows(QModelIndex(), row, row);
  moduleModel.list.removeAll(m);
  moduleModel.endRemoveRows();
}  

void MidiWidget::toggleMidiSign()
{
  if (midiGUIcomponent == NULL)
    return;
  midiGUIcomponent->midiSign = (midiGUIcomponent->midiSign == 1) ? 0 : 1;
  Module *m = midiGUIcomponent->parentModule;
  QModelIndex mMi =
    moduleModel.index(moduleModel.list.indexOf(m), 0);
  QModelIndex mgcMi =
    moduleModel.index(m->configDialog->
		      midiGUIcomponentList.indexOf(midiGUIcomponent), 1, mMi);
  emit moduleModel.dataChanged(mgcMi, mgcMi);
}

void MidiWidget::guiControlChanged(const QItemSelection &selected,
				   const QItemSelection &/*deselected*/)
{
  int l1;
  Module *module;
  bool success = false;
  QString qs;
  int min, max, val;

  if (midiGUIcomponent) {
    QObjectList del = currentGUIcontrol->children();
    QObjectList::const_iterator deli = del.constBegin();
    while (deli != del.constEnd()) {
      std::cout << (*deli)->metaObject()->className() << std::endl;
      delete *deli;
      ++deli;
    }
    del = guiControlParent->children();
    deli = del.constBegin();
    while (deli != del.constEnd()) {
      std::cout << (*deli)->metaObject()->className() << std::endl;
      if (*deli != currentGUIcontrol) 
	delete *deli;
      ++deli;
    }
    midiGUIcomponent = NULL;
  }  

//   std::cout << __FUNCTION__ << ":" << __LINE__ << std::endl;
  if (selected.indexes().count() > 0) {
    const QModelIndex mi = selected.indexes().at(0);
//     std::cout << __FUNCTION__ << ":" << __LINE__ << ":" << selected.indexes().count() << std::endl;
    MidiGUIcomponent *mgc = dynamic_cast<MidiGUIcomponent *>((QWidget *)mi.internalPointer());
    if (mgc) {
//   std::cout << __FUNCTION__ << ":" << __LINE__ << std::endl;
      midiGUIcomponent = mgc;
      module = mgc->parentModule;
      success = true;
    }
  }
  midiSignButton->setEnabled(success);
  addGuiButton->setEnabled(success);
  if (!success)
    return;

  QLabel *nameLabel = new QLabel(midiGUIcomponent->objectName());
  switch (midiGUIcomponent->componentType) {
    case GUIcomponentType_slider: {
        minValue = ((MidiSlider *)midiGUIcomponent)->min;
        maxValue = ((MidiSlider *)midiGUIcomponent)->max;
        initial_min = ((MidiSlider *)midiGUIcomponent)->initial_min;
        initial_max = ((MidiSlider *)midiGUIcomponent)->initial_max;
        value = ((MidiSlider *)midiGUIcomponent)->getValue();
        QHBoxLayout *sliderNameBox = new QHBoxLayout();
	currentGUIcontrol->addLayout(sliderNameBox);
        sliderNameBox->setMargin(5);
        sliderNameBox->addStretch();
        QHBoxLayout *minMaxLogButtonsBox = new QHBoxLayout();
	currentGUIcontrol->addLayout(minMaxLogButtonsBox);
        minMaxLogButtonsBox->setMargin(5);
        logCheck = new QCheckBox("Log");
	minMaxLogButtonsBox->addWidget(logCheck);
        QObject::connect(logCheck, SIGNAL(toggled(bool)), 
                         this, SLOT(setLogMode(bool)));
        minMaxLogButtonsBox->addStretch();
        QPushButton *newMinButton = new QPushButton("Set Min");
	minMaxLogButtonsBox->addWidget(newMinButton);
        QObject::connect(newMinButton, SIGNAL(clicked()), 
                         this, SLOT(setNewMin()));
        minMaxLogButtonsBox->addStretch();
        QPushButton *newMaxButton = new QPushButton("Set Max");
	minMaxLogButtonsBox->addWidget(newMaxButton);
        QObject::connect(newMaxButton, SIGNAL(clicked()), 
                         this, SLOT(setNewMax()));
        minMaxLogButtonsBox->addStretch();
        QPushButton *resetMinMaxButton = new QPushButton("Reset Min/Max");
	minMaxLogButtonsBox->addWidget(resetMinMaxButton);
        QObject::connect(resetMinMaxButton, SIGNAL(clicked()), 
                         this, SLOT(setInitialMinMax()));

	sliderNameBox->addWidget(nameLabel);
        sliderNameBox->addStretch();
        QHBoxLayout *sliderLabels = new QHBoxLayout();
	currentGUIcontrol->addLayout(sliderLabels);
        qs.sprintf("Min: %7.3f", minValue); 
        minLabel = new QLabel(qs);
	sliderLabels->addWidget(minLabel);
        sliderLabels->addStretch();
        qs.sprintf(" %7.3f ", value);
        valueLabel = new QLabel(qs);
	sliderLabels->addWidget(valueLabel);

        sliderLabels->addStretch();
        qs.sprintf("Max: %7.3f", maxValue);
        maxLabel = new QLabel(qs);
	sliderLabels->addWidget(maxLabel);
        nameLabel->setFixedHeight(nameLabel->sizeHint().height());
        valueLabel->setFixedHeight(valueLabel->sizeHint().height());
        minLabel->setFixedHeight(minLabel->sizeHint().height());
        maxLabel->setFixedHeight(maxLabel->sizeHint().height());
        slider = new QSlider(Qt::Horizontal);
	slider->setRange(int(SLIDER_SCALE * minValue), int(SLIDER_SCALE * maxValue));
	slider->setPageStep(0);
	slider->setValue(int(SLIDER_SCALE * value));
//         slider = new QSlider(int(SLIDER_SCALE * minValue), int(SLIDER_SCALE * maxValue), 0, 
//                              int(SLIDER_SCALE * value), Qt::Horizontal);
	currentGUIcontrol->addWidget(slider);
        slider->setFixedHeight(slider->sizeHint().height());
        QObject::connect(slider, SIGNAL(valueChanged(int)), 
                        (MidiSlider *)midiGUIcomponent, SLOT(updateSlider(int)));
        QObject::connect((MidiSlider *)midiGUIcomponent, SIGNAL(valueChanged(int)),
                         this, SLOT(updateSliderValue(int)));
        setLogCheck(((MidiSlider *)midiGUIcomponent)->isLog);
        }
      break;
    case GUIcomponentType_intslider: {
        min = ((IntMidiSlider *)midiGUIcomponent)->slider->minimum();
        max = ((IntMidiSlider *)midiGUIcomponent)->slider->maximum();
        val = ((IntMidiSlider *)midiGUIcomponent)->slider->value();
        QHBoxLayout *sliderNameBox = new QHBoxLayout();
	currentGUIcontrol->addLayout(sliderNameBox);
        sliderNameBox->setMargin(5);
        sliderNameBox->addStretch();

	sliderNameBox->addWidget(nameLabel);
        sliderNameBox->addStretch();
        QHBoxLayout *sliderLabels = new QHBoxLayout();
	currentGUIcontrol->addLayout(sliderLabels);
        qs.sprintf("Min: %d", min); 
        minLabel = new QLabel(qs);
	sliderLabels->addWidget(minLabel);
        sliderLabels->addStretch();
        qs.sprintf(" %d ", val);
        valueLabel = new QLabel(qs);
	sliderLabels->addWidget(valueLabel);

//         nameLabel->setText(moduleListView->selectedItem()->text(0));
        sliderLabels->addStretch();
        qs.sprintf("Max: %d", max);
        maxLabel = new QLabel(qs);
	sliderLabels->addWidget(maxLabel);
        nameLabel->setFixedHeight(nameLabel->sizeHint().height());
        valueLabel->setFixedHeight(valueLabel->sizeHint().height());
        minLabel->setFixedHeight(minLabel->sizeHint().height());
        maxLabel->setFixedHeight(maxLabel->sizeHint().height());
        slider = new QSlider(Qt::Horizontal);
	slider->setRange(min, max);
	slider->setPageStep(0);
	slider->setValue(val);

	currentGUIcontrol->addWidget(slider);
        slider->setTickInterval((abs(max) + abs(min)) / 10);
        slider->setTickPosition(QSlider::TicksBelow);
        slider->setFixedHeight(slider->sizeHint().height());
        QObject::connect(slider, SIGNAL(valueChanged(int)), 
                         (IntMidiSlider *)midiGUIcomponent, SLOT(updateSlider(int)));
        QObject::connect((IntMidiSlider *)midiGUIcomponent, SIGNAL(valueChanged(int)),
                         this, SLOT(updateIntSliderValue(int)));
        }
      break;
    case GUIcomponentType_floatintslider: {
        min = ((FloatIntMidiSlider *)midiGUIcomponent)->slider->minimum();
        max = ((FloatIntMidiSlider *)midiGUIcomponent)->slider->maximum();
        val = ((FloatIntMidiSlider *)midiGUIcomponent)->slider->value();
        QHBoxLayout *sliderNameBox = new QHBoxLayout();
	currentGUIcontrol->addLayout(sliderNameBox);
        sliderNameBox->setMargin(5);
        sliderNameBox->addStretch();

	sliderNameBox->addWidget(nameLabel);
        sliderNameBox->addStretch();
        QHBoxLayout *sliderLabels = new QHBoxLayout();
	currentGUIcontrol->addLayout(sliderLabels);
        qs.sprintf("Min: %d", min); 
        minLabel = new QLabel(qs);
	sliderLabels->addWidget(minLabel);
        sliderLabels->addStretch();
        qs.sprintf(" %d ", val);
        valueLabel = new QLabel(qs);
	sliderLabels->addWidget(valueLabel);

//         nameLabel->setText(moduleListView->selectedItem()->text(0));
        sliderLabels->addStretch();
        qs.sprintf("Max: %d", max);
        maxLabel = new QLabel(qs);
	sliderLabels->addWidget(maxLabel);
        nameLabel->setFixedHeight(nameLabel->sizeHint().height());
        valueLabel->setFixedHeight(valueLabel->sizeHint().height());
        minLabel->setFixedHeight(minLabel->sizeHint().height());
        maxLabel->setFixedHeight(maxLabel->sizeHint().height());
        slider = new QSlider(Qt::Horizontal);
	slider->setRange(min, max);
	slider->setPageStep(0);
	slider->setValue(val);

	currentGUIcontrol->addWidget(slider);
        slider->setTickInterval((abs(max) + abs(min)) / 10);
        slider->setTickPosition(QSlider::TicksBelow);
        slider->setFixedHeight(slider->sizeHint().height());
        QObject::connect(slider, SIGNAL(valueChanged(int)), 
                         (FloatIntMidiSlider *)midiGUIcomponent, SLOT(updateSlider(int)));
        QObject::connect((FloatIntMidiSlider *)midiGUIcomponent, SIGNAL(valueChanged(int)),
                         this, SLOT(updateIntSliderValue(int)));
        }
      break;
    case GUIcomponentType_checkbox: {
        QHBoxLayout *checkNameBox = new QHBoxLayout();
	currentGUIcontrol->addLayout(checkNameBox);
        checkNameBox->addStretch();
        checkNameBox->setMargin(5);

	checkNameBox->addWidget(nameLabel);
        checkNameBox->addStretch();

//         nameLabel->setText(moduleListView->selectedItem()->text(0));
        nameLabel->setFixedHeight(nameLabel->sizeHint().height());
        QHBoxLayout *checkHBox = new QHBoxLayout();
	currentGUIcontrol->addLayout(checkHBox);
        checkHBox->addStretch();
        currentCheck = new QCheckBox();
	checkHBox->addWidget(currentCheck);
        checkHBox->addStretch();
        currentCheck->setChecked(((MidiCheckBox *)midiGUIcomponent)->checkBox->isChecked());
        currentCheck->setFixedSize(currentCheck->sizeHint());
        QObject::connect(currentCheck, SIGNAL(toggled(bool)), 
                         (MidiCheckBox *)midiGUIcomponent, SLOT(updateCheck(bool)));
        QObject::connect(((MidiCheckBox *)midiGUIcomponent)->checkBox, SIGNAL(toggled(bool)),
                         this, SLOT(updateCheckBox(bool)));
        }
      break;
    case GUIcomponentType_pushbutton: {
         QHBoxLayout *pushbuttonBox = new QHBoxLayout();
	 currentGUIcontrol->addLayout(pushbuttonBox);
         pushbuttonBox->addStretch();
	 QPushButton *currentPushButton =
	   new QPushButton(midiGUIcomponent->objectName());
	 pushbuttonBox->addWidget(currentPushButton);
         pushbuttonBox->addStretch();
         QObject::connect(currentPushButton, SIGNAL(clicked()), 
                          (MidiPushButton *)midiGUIcomponent, SLOT(buttonClicked()));
        }
      break;
    case GUIcomponentType_combobox: {
        QHBoxLayout *comboNameBox = new QHBoxLayout();
	currentGUIcontrol->addLayout(comboNameBox);
        comboNameBox->addStretch();
        comboNameBox->setMargin(5);

	comboNameBox->addWidget(nameLabel);
        comboNameBox->addStretch();
        nameLabel->setText(midiGUIcomponent->objectName());
//         nameLabel->setText(moduleListView->selectedItem()->text(0));
        nameLabel->setFixedHeight(nameLabel->sizeHint().height());
        QHBoxLayout *comboHBox = new QHBoxLayout();
	currentGUIcontrol->addLayout(comboHBox);
        comboHBox->addStretch();
        comboBox = new QComboBox();
	comboHBox->addWidget(comboBox);
        comboHBox->addStretch();
        for (l1 = 0; l1 < ((MidiComboBox *)midiGUIcomponent)->comboBox->count(); l1++)
          comboBox->insertItem(l1, ((MidiComboBox *)midiGUIcomponent)->comboBox->itemText(l1));

        comboBox->setCurrentIndex(((MidiComboBox *)midiGUIcomponent)->comboBox->currentIndex());
        comboBox->setFixedSize(comboBox->sizeHint());
        QObject::connect(comboBox, SIGNAL(highlighted(int)), 
                         (MidiComboBox *)midiGUIcomponent, SLOT(updateValue(int)));
        QObject::connect(((MidiComboBox *)midiGUIcomponent)->comboBox, SIGNAL(highlighted(int)),
                         this, SLOT(updateComboBox(int)));
        }
      break;
    default:
      break;
  }

  firstBindingMightHaveChanged = true;
}

void MidiWidget::midiControlChanged(const QItemSelection &selected,
				    const QItemSelection &deselected)
{
  selectedController = MidiControllerKey();
  selectedControl = -1;
  bool bindEnable = false,
    clearEnable = false;

  if (selected.indexes().count() > 0) {
    const QModelIndex mi = selected.indexes().at(0);
    std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << ":" << selected.indexes().count() << ":" << deselected.indexes().count() << std::endl;
    const MidiController *mc = (const MidiController *)mi.internalPointer();
    if (mc) {
      selectedController = ((const MidiController *)mi.internalPointer())->getKey();
      selectedControl = mi.row();
      clearEnable = true;
    } else {
      selectedController = midiControllerList.at(mi.row()).getKey();
      bindEnable = true;
    }
  }
  bindButton->setEnabled(bindEnable);
  clearButton->setEnabled(clearEnable);
  firstBindingMightHaveChanged = true;
}

void MidiWidget::updateSliderValue(int p_value) {

  QString qs;
  
  if (logCheck->isChecked()) {
    value = exp((float)p_value / SLIDER_SCALE);
  } else {
    value = (float)p_value / SLIDER_SCALE;
  }
  qs.sprintf(" %7.3f ", value);
  valueLabel->setText(qs);
  slider->setValue(p_value);
}

void MidiWidget::updateIntSliderValue(int p_value) {

  QString qs;
  
  qs.sprintf(" %d ", p_value);
  valueLabel->setText(qs);
  slider->setValue(p_value);
}

void MidiWidget::setLogCheck(bool on) {

  logCheck->setChecked(on);
  setLogMode(on);
}

void MidiWidget::setLogMode(bool on) {
   
  float log_min, log_val, val;

  val = value;
  ((MidiSlider *)midiGUIcomponent)->setLogMode(on);
  if (on) {
    log_min = (minValue > 1e-4) ? minValue : 1e-4;
    log_val = (val > 1e-4) ? val : 1e-4;
    slider->setMinimum(int(SLIDER_SCALE * log(log_min)));
    ((MidiSlider *)midiGUIcomponent)->slider->setMinimum(int(SLIDER_SCALE * log(log_min)));
    slider->setMaximum(int(SLIDER_SCALE * log(maxValue)));
    ((MidiSlider *)midiGUIcomponent)->slider->setMaximum(int(SLIDER_SCALE * log(maxValue)));
    slider->setValue(int(SLIDER_SCALE * log(log_val)));
  } else {
    slider->setMinimum(int(SLIDER_SCALE * minValue));
    ((MidiSlider *)midiGUIcomponent)->slider->setMinimum(int(SLIDER_SCALE * minValue));
    slider->setMaximum(int(SLIDER_SCALE * maxValue));  
    ((MidiSlider *)midiGUIcomponent)->slider->setMaximum(int(SLIDER_SCALE * maxValue));
    slider->setValue(int(SLIDER_SCALE * val));   
  }
}

void MidiWidget::setNewMin() {
  
  QString qs;

  minValue = value;
  ((MidiSlider *)midiGUIcomponent)->min = value;
  qs.sprintf("Min: %7.3f", minValue);
  minLabel->setText(qs);
  ((MidiSlider *)midiGUIcomponent)->minLabel->setText(qs);
  if (logCheck->isChecked()) {
    slider->setMinimum(int(SLIDER_SCALE * log(minValue)));
    ((MidiSlider *)midiGUIcomponent)->slider->setMinimum(int(SLIDER_SCALE * log(minValue)));
    slider->setValue(int(SLIDER_SCALE * log(value)));
  } else {
    slider->setMinimum(int(SLIDER_SCALE * minValue));
    ((MidiSlider *)midiGUIcomponent)->slider->setMinimum(int(SLIDER_SCALE * minValue));
    slider->setValue(int(SLIDER_SCALE * value));
  }
  midiGUIcomponent->controllerOK = false;
}

void MidiWidget::setNewMax() {        
 
  QString qs;

  maxValue = value;
  ((MidiSlider *)midiGUIcomponent)->max = value;
  qs.sprintf("Min: %7.3f", maxValue); 
  maxLabel->setText(qs);
  ((MidiSlider *)midiGUIcomponent)->maxLabel->setText(qs);
  if (logCheck->isChecked()) {
    slider->setMaximum(int(SLIDER_SCALE * log(maxValue)));
    ((MidiSlider *)midiGUIcomponent)->slider->setMaximum(int(SLIDER_SCALE * log(maxValue)));
    slider->setValue(int(SLIDER_SCALE * log(value)));
  } else {
    slider->setMaximum(int(SLIDER_SCALE * maxValue));
    ((MidiSlider *)midiGUIcomponent)->slider->setMaximum(int(SLIDER_SCALE * maxValue));
    slider->setValue(int(SLIDER_SCALE * value));
  }
  midiGUIcomponent->controllerOK = false;
}  

void MidiWidget::setInitialMinMax() {

  QString qs;

  logCheck->setChecked(false);
  minValue = initial_min;
  ((MidiSlider *)midiGUIcomponent)->min = initial_min;
  maxValue = initial_max;
  ((MidiSlider *)midiGUIcomponent)->max = initial_max;
  qs.sprintf("Min: %7.3f", minValue);
  minLabel->setText(qs);
  ((MidiSlider *)midiGUIcomponent)->minLabel->setText(qs);
  slider->setMinimum(int(SLIDER_SCALE * minValue));
  ((MidiSlider *)midiGUIcomponent)->slider->setMinimum(int(SLIDER_SCALE * minValue));
  qs.sprintf("Min: %7.3f", maxValue); 
  maxLabel->setText(qs);
  ((MidiSlider *)midiGUIcomponent)->maxLabel->setText(qs);
  slider->setMaximum(int(SLIDER_SCALE * maxValue));
  ((MidiSlider *)midiGUIcomponent)->slider->setMaximum(int(SLIDER_SCALE * maxValue));
  slider->setValue(int(SLIDER_SCALE * value));
  midiGUIcomponent->controllerOK = false;
}  

void MidiWidget::updateComboBox(int p_value) {

  comboBox->setCurrentIndex(p_value);  
}

void MidiWidget::updateCheckBox(bool on) {

  currentCheck->setChecked(on);  
}

void MidiWidget::setSelectedController(MidiControllerKey mck)
{
  if (!(mck == selectedController)) {
    typeof(midiControllerList.constEnd()) c
      = qBinaryFind(midiControllerList.constBegin(),
		    midiControllerList.constEnd(), mck);
    if (c == midiControllerList.end()) {
      std::cerr  << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
      return;
    }
    midiControllerListView->selectionModel()->
      select(midiControllerModel.
	     index(&*c - midiControllerList.data(), 0),
	     QItemSelectionModel::ClearAndSelect);
  }
}

void MidiWidget::updateGuiComponent() {

  MidiGUIcomponent *guiComponent;

  if (followConfig) {
    guiComponent = (MidiGUIcomponent *)sender();
//     if (guiComponent->listViewItem) {
// //       if (moduleListView->currentItem() != guiComponent->listViewItem) {
// //         moduleListView->setSelected(guiComponent->listViewItem, true);
// //         moduleListView->ensureItemVisible(guiComponent->listViewItem);
// //       }
//     }
  }
}

void MidiWidget::updateMidiChannel(int index) {

  synthdata->midiChannel = index - 1;
}
