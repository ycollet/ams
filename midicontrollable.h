#ifndef MIDICONTROLLABLE_H
#define MIDICONTROLLABLE_H

#include <QString>
#include "midisliderbase.h"
#include "midicontroller.h"
#include "module.h"

#define SLIDER_SCALE 16384.0


class MidiControllableBase {
protected:
  static QString temp;

public:
  class Module &module; 
  QString name;
  int midiControllableListIndex;
  QList<class MidiControllerKey> midiControllerList;
  QList<class MCed *> mcws;
  bool midiSign; 
  
  MidiControllableBase(Module &module, const QString &name)
    : module(module)
    , name(name)
    , midiSign(true)
    {
      midiControllableListIndex = module.midiControllables.count();
      module.midiControllables.append(this);
    }

  virtual ~MidiControllableBase() {
    while (midiControllerList.count())
      disconnectController(midiControllerList.at(0));
  }

  void updateMGCs(MidiGUIcomponent *sender);

  virtual int getValue() { return *(int*)0; }
  virtual int getMin() { return *(int*)0; }
  virtual int getMax() { return *(int*)0; }

  virtual void setMidiValueRT(int ) = 0;
  virtual void setValue(int value) = 0;
  virtual int getMidiValue() = 0;

  virtual int sliderMin() { return 0;}
  virtual int sliderMax() { return 0;}
  virtual int sliderVal() { return 0;}
  virtual int sliderStep() { return 0;}

  virtual void setSliderVal(int , MidiSliderBase *) {};

  virtual const QString &minString() { return *(QString*)NULL;}
  virtual const QString &maxString() { return *(QString*)NULL;}
  virtual const QString &valString() { return *(QString*)NULL;}

  void connectTo(MCed *mcw) {
    mcws.append(mcw);
  }

  void disconnect(MCed *mcw) {
    mcws.removeAll(mcw);
  }

  void connectToController(MidiControllerKey midiController);
  void disconnectController(MidiControllerKey midiController);
};


class MidiControllableDoOnce: public MidiControllableBase {
public:
  void (Module::*doOnce)(void);

  MidiControllableDoOnce(Module &module, const QString &name, void (Module::*doOnce)(void))
    : MidiControllableBase(module, name)
    , doOnce(doOnce)
  {}

  virtual void setMidiValueRT(int );
  virtual void setValue(int value);
  virtual int getMidiValue();
};


template <typename t> class MidiControllable: public MidiControllableBase {
protected:
  t &value;
  const t min, max;

  static t round(float f);

public:
  MidiControllable(Module &module, const QString &name, t &value, t min, t max)
    : MidiControllableBase(module, name)
    , value(value)
    , min(min)
    , max(max)
  {}

  void setVal(int val, MidiGUIcomponent *sender) {
    value = val;
    updateMGCs(sender);
  }

  int toInt(t _t) {return (int)_t;}
  t toType(float i) {return (t)i;}

  virtual int getValue() { return toInt(value); }
  virtual int getMin() { return toInt(min); }
  virtual int getMax() { return toInt(max); }

  virtual void setMidiValueRT(int val0to127) {
    float tick = (float)(max - min) / 127;
    if (!midiSign)
      val0to127 = 127 - val0to127;
    value = min + round(tick * val0to127);
  }
  virtual void setValue(int /*value*/) {}
  virtual int getMidiValue() { return 0;}

  virtual int sliderMin() { return getMin(); }
  virtual int sliderMax() { return getMax(); }
  virtual int sliderVal() { return getValue(); }
  virtual int sliderStep() { return 1; }

  virtual void setSliderVal(int val, MidiSliderBase *sender) {
    setVal(val, sender);
 }

  virtual const QString &minString() { return temp.setNum(toInt(min)); }
  virtual const QString &maxString() { return temp.setNum(toInt(max)); }
  virtual const QString &valString() { return temp.setNum(toInt(value)); }
};
template <>
inline float MidiControllable<float>::round(float f) {
  return f;
}
template <>
inline int MidiControllable<int>::round(float f) {
  return (int)(0.5 + f);
}

class MidiControllableFloat: public MidiControllable<float> {
public:
  bool isLog;
  float varMin, varMax;
  MidiControllableFloat(Module &module, const QString &name, float &value, float min, float max, bool isLog = false)
    : MidiControllable<float>(module, name, value, min, max)
    , isLog(isLog)
  {
    resetMinMax();
  }

  operator float() {return value;}

  float maybeLog(float );

  void setLog(bool );
  void setNewMin(int min);
  void setNewMax(int max);
  void setNewMin();
  void setNewMax();
  void resetMinMax();
  void updateFloatMGCs();

  virtual void setMidiValueRT(int );
  virtual void setValue(int value);
  virtual int getMidiValue();

  virtual int sliderMin();
  virtual int sliderMax();
  virtual int sliderVal();
  virtual int sliderStep();

  virtual void setSliderVal(int , MidiSliderBase *);

  virtual const QString &minString();
  virtual const QString &maxString();
  virtual const QString &valString();
};


class MidiControllableNames: public MidiControllable<int> {
public:
  QStringList itemNames;

  MidiControllableNames(Module &module, const QString &name, int &value, const QStringList &itemNames)
    : MidiControllable<int>(module, name, value, 0, itemNames.count() - 1)
    , itemNames(itemNames)
  {}

};

#endif
