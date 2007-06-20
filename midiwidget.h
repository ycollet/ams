#ifndef MIDIWIDGET_H
#define MIDIWIDGET_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <QAbstractListModel>
#include <QItemSelectionModel>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlabel.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <alsa/asoundlib.h>
#include "midicontroller.h"
#include "synthdata.h"
#include "module.h"
#include "midiguicomponent.h"

class MidiControllerModel : public QAbstractItemModel
{
Q_OBJECT

friend class MidiWidget;

  QVector<MidiController> &midiControllerList;

public:
  MidiControllerModel(QVector<MidiController> &midiControllerList, QObject *parent = 0)
    : QAbstractItemModel(parent)
    , midiControllerList(midiControllerList)
  {}

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation,
		      int role = Qt::DisplayRole) const;
  QModelIndex index(int row, int column,
		    const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &child) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;

  void insert(QVector<MidiController>::iterator c, MidiController &mc);
};

class ModuleModel : public QAbstractItemModel
{
Q_OBJECT

friend class MidiWidget;

  QList<class Module *> list;

public:
  ModuleModel(QObject *parent = 0)
    : QAbstractItemModel(parent) {}

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation,
		      int role = Qt::DisplayRole) const;
  QModelIndex index(int row, int column,
		    const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &child) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
};

class MidiWidget : public QWidget
{
Q_OBJECT

  friend class MidiControllerModel;

private:
  QCheckBox *noteCheck, *configCheck, *midiCheck;
  class QTreeView *midiControllerListView, *moduleListView;
  QVBoxLayout vbox;
  MidiControllerModel midiControllerModel;
  ModuleModel moduleModel;
  MidiControllerKey selectedController;
  int selectedControl;
  QVBoxLayout *currentGUIcontrol;
  QFrame *guiControlParent;
  QLabel *valueLabel, *minLabel, *maxLabel;
  QSlider *slider;
  QCheckBox *logCheck, *currentCheck;
  QString currentFrameName, currentTabName;
  class QComboBox *comboBox;
  float minValue, maxValue, value, initial_min, initial_max;
  MidiGUIcomponent *midiGUIcomponent;
  bool firstBindingMightHaveChanged; 
  QVector<MidiController> midiControllerList;
  QPushButton *addGuiButton;
  QPushButton *bindButton;
  QPushButton *clearButton;
  QPushButton *clearAllButton;
  QPushButton *midiSignButton;

  void deleteMidiGuiComponent();

public: 
   bool noteControllerEnabled, followConfig, followMidi;
    
public:
  MidiWidget(QWidget* parent, const char *name=0);
  ~MidiWidget();
  void addMidiController(MidiControllerKey midiController);
  const MidiController *midiController(MidiControllerKey midiController) {
    typeof(midiControllerList.constEnd()) c =
      qBinaryFind(midiControllerList.constBegin(), midiControllerList.constEnd(), midiController);
    return c == midiControllerList.constEnd() ? NULL : &*c;
  }
    void addMidiGuiComponent(MidiControllerKey midiController, MidiGUIcomponent *midiGuiComponent);
    void removeMidiGuiComponent(MidiControllerKey midiController, MidiGUIcomponent *midiGuiComponent);
    const MidiControllerKey getSelectedController() {
	    return selectedController;
    }
    void setSelectedController(MidiControllerKey midiController);

    void addModule(Module *m);
    void removeModule(Module *m);

  public slots: 
    void clearAllClicked();
    void clearClicked();
    void bindClicked();
    void addToParameterViewClicked();
    void noteControllerCheckToggle();
    void configCheckToggle();
    void midiCheckToggle();
    void toggleMidiSign();
    void guiControlChanged(const QItemSelection &selected,
			   const QItemSelection &deselected);
    void midiControlChanged(const QItemSelection &selected,
			    const QItemSelection &deselected);
    void updateSliderValue(int p_value);
    void updateIntSliderValue(int p_value);
    void updateComboBox(int p_value);
    void updateCheckBox(bool on);
    void setLogCheck(bool);
    void setLogMode(bool on);
    void setNewMin();
    void setNewMax();
    void setInitialMinMax();
    void updateGuiComponent();
    void updateMidiChannel(int index);
};
  
#endif
