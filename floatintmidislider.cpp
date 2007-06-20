#include <qslider.h>
#include <qlabel.h>
#include <stdio.h>
#include <math.h>
#include "intmidislider.h"
#include "synthdata.h"
#include "midiwidget.h"
#include "midiguicomponent.h"
#include "floatintmidislider.h"

FloatIntMidiSlider::FloatIntMidiSlider(Module *parentModule, float minValue, float maxValue, int pageStep, float value,
                                       Qt::Orientation orientation, QWidget * parent, const QString &name, 
                                       float *p_valueRef)
  : MidiSliderBase(parentModule, orientation, parent, name,
		   (int)minValue, (int)maxValue, pageStep, (int)value,
		   QString().sprintf("Min: %d", (int)minValue),
		   QString().sprintf("Max: %d", (int)maxValue)) {

  QString qs;

  componentType = GUIcomponentType_floatintslider;
  valueRef = p_valueRef;
  slider->setTickInterval((abs((int)maxValue) + abs((int)minValue)) / 10);
  slider->setTickPosition(QSlider::TicksBelow);
  slider->setFixedHeight(slider->sizeHint().height());
  QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)));
}


void FloatIntMidiSlider::setMidiValue(int value) {

  if (!controllerOK) {
    controllerOK = abs(getMidiValue() - value) < 4;
  }
  if (controllerOK) {
    if (midiSign == 1) {
      slider->setValue( slider->minimum() + rint(float(slider->maximum() - slider->minimum()) * (float)value / 127.0));
    } else {
      slider->setValue( slider->maximum() - rint(float(slider->maximum() - slider->minimum()) * (float)value / 127.0));
    }
  }
}

void FloatIntMidiSlider::updateValue(int value) {

  QString qs;
  
  *valueRef = value;
  qs.sprintf(" %d ", (int)*valueRef);
  valueLabel->setText(qs);
  emit valueChanged(value);
  emit guiComponentTouched();
}

int FloatIntMidiSlider::getMidiValue() {
  
  int x;
 
  x = rint(float(slider->value() -  slider->minimum())) * 127.0 / float(slider->maximum() - slider->minimum());
  return(x);
}
