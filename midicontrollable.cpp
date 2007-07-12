#include "midicontrollable.h"
#include "midiwidget.h"
#include "midislider.h"
#include <math.h>


QString MidiControllableBase::temp;


void MidiControllableBase::updateMGCs(MidiGUIcomponent *sender)
{
  for (typeof(mcws.constBegin()) mcw = mcws.constBegin();
       mcw != mcws.constEnd();  mcw++)
    if (*mcw != sender)
      (*mcw)->mcAbleChanged();

  if (sender)
    synthdata->midiWidget->guiComponentTouched(*this);
}


void MidiControllableBase::connectToController(MidiControllerKey midiController)
{
  if (!midiControllerList.contains(midiController)) {
    midiControllerList.append(midiController);
    synthdata->midiWidget->addMidiControllable(midiController, this);
  }
}
   
void MidiControllableBase::disconnectController(MidiControllerKey midiController)
{
  midiControllerList.removeAll(midiController);
  synthdata->midiWidget->removeMidiControllable(midiController, this);
}


void MidiControllableDoOnce::updateMGCs(MidiGUIcomponent */*sender*/)
{
  trigger();
}

bool MidiControllableDoOnce::setMidiValueRT(int val0to127)
{
  if (!midiSign)
    val0to127 = 127 - val0to127;

  if (val0to127 > std::max(95, lastVal)) {
    lastVal = 127;
    return true;
  }

  if (val0to127 < std::min(32, lastVal))
    lastVal = val0to127;

  return false;
}

int MidiControllableDoOnce::getMidiValue()
{
  return 0;
}

float MidiControllableFloat::maybeLog(float v)
{
  float r;

  if (isLog) {
    if (v < 1e-4)
      v = 1e-4;
    r = logf(v);
  } else
    r = v;

  return r;
}

int MidiControllableFloat::sliderMin()
{
  return (int)(SLIDER_SCALE * maybeLog(varMin));
}

int MidiControllableFloat::sliderMax()
{
  return (int)(SLIDER_SCALE * maybeLog(varMax));
}

int MidiControllableFloat::sliderVal()
{
  return (int)(SLIDER_SCALE * maybeLog(value));
}

int MidiControllableFloat::sliderStep()
{
  return 0;
}

void MidiControllableFloat::setSliderVal(int val, MidiSliderBase *sender)
{
  float v = (float)val / SLIDER_SCALE;

  if (isLog)
    v = expf(v);

  value = v;
  updateMGCs(sender);
}

void MidiControllableFloat::setLog(bool log)
{
  isLog = log;
  updateFloatMGCs();
  updateMGCs(NULL);
}

void MidiControllableFloat::setNewMin(int min)
{
  varMin = (float)min / SLIDER_SCALE;
  if (isLog)
    varMin = expf(varMin);

  updateFloatMGCs();
  updateMGCs(NULL);
}

void MidiControllableFloat::setNewMax(int max)
{
  varMax = (float)max / SLIDER_SCALE;
  if (isLog)
    varMax = expf(varMax);

  updateFloatMGCs();
  updateMGCs(NULL);
}

void MidiControllableFloat::setNewMin()
{
  varMin = value;
  updateFloatMGCs();
  updateMGCs(NULL);
}

void MidiControllableFloat::setNewMax()
{
  varMax = value;
  updateFloatMGCs();
  updateMGCs(NULL);
}

void MidiControllableFloat::resetMinMax()
{
  varMin = min;
  varMax = max;
  updateFloatMGCs();
  updateMGCs(NULL);
}

void MidiControllableFloat::updateFloatMGCs()
{
  for (typeof(mcws.constBegin()) mcw = mcws.constBegin();
       mcw != mcws.constEnd();  mcw++) {
    MidiSlider * s = dynamic_cast<MidiSlider *>(*mcw);
    if (s)
      s->minMaxChanged();
  }
}

bool MidiControllableFloat::setMidiValueRT(int val0to127)
{
  float tick = (varMax - varMin) / 127;
  if (!midiSign)
    val0to127 = 127 - val0to127;
  value = varMin + tick * val0to127;

  return true;
}

int MidiControllableFloat::getMidiValue()
{
  return 0;
}

const QString &MidiControllableFloat::minString()
{
  return temp.setNum(varMin, 'g', 3);
}
const QString &MidiControllableFloat::maxString()
{
  return temp.setNum(varMax, 'g', 3);
}
const QString &MidiControllableFloat::valString()
{
  return temp.setNum(value, 'g', 3);
}
