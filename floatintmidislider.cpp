#include <qslider.h>
#include <qhbox.h>
#include <qlabel.h>
#include <stdio.h>
#include <math.h>
#include "intmidislider.h"
#include "synthdata.h"
#include "midiwidget.h"
#include "midiguicomponent.h"

FloatIntMidiSlider::FloatIntMidiSlider(QObject *parentModule, float minValue, float maxValue, int pageStep, float value,
                                       QSlider::Orientation orientation, QWidget * parent, const char * name, 
                                       SynthData *p_synthdata, float *p_valueRef)
           : MidiGUIcomponent(parentModule, p_synthdata, parent, name) {

  QString qs;

  componentType = GUIcomponentType_floatintslider;
  valueRef = p_valueRef;
  setSpacing(1);
  new QWidget(this);
  QVBox *sliderBox = new QVBox(this);
  sliderBox->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
  QHBox *sliderNameBox = new QHBox(sliderBox);
  new QWidget(sliderNameBox);
  sliderNameBox->setMargin(1);
  QLabel *nameLabel = new QLabel(sliderNameBox);
  new QWidget(sliderNameBox);
  QHBox *sliderLabels = new QHBox(sliderBox);
  qs.sprintf("Min: %d", (int)minValue); 
  QLabel *minLabel = new QLabel(qs, sliderLabels);
  new QWidget(sliderLabels);
  valueLabel = new QLabel("  ", sliderLabels);
  if (name) {
    qs.sprintf(" %s ", name);
  } else {
    qs.sprintf("  ");
  }
  nameLabel->setText(qs);
  new QWidget(sliderLabels);
  qs.sprintf("Max: %d", (int)maxValue);
  QLabel *maxLabel = new QLabel(qs, sliderLabels);
  if (name) {
    nameLabel->setFixedHeight(nameLabel->sizeHint().height());
  }
  nameLabel->setFixedHeight(valueLabel->sizeHint().height());
  valueLabel->setFixedHeight(valueLabel->sizeHint().height());
  minLabel->setFixedHeight(minLabel->sizeHint().height());
  maxLabel->setFixedHeight(maxLabel->sizeHint().height());
  slider = new QSlider((int)minValue, (int)maxValue, pageStep, 
                       (int)value, orientation, sliderBox);  
  slider->setTickInterval((abs((int)maxValue) + abs((int)minValue)) / 10);
  slider->setTickmarks(QSlider::Below);
  slider->setFixedHeight(slider->sizeHint().height());
  QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)));
  slider->setValue((int)value);
  qs.sprintf(" %d ", (int)*valueRef);
  valueLabel->setText(qs);
}

FloatIntMidiSlider::~FloatIntMidiSlider(){
}

void FloatIntMidiSlider::setMidiValue(int value) {

  if (!controllerOK) {
    controllerOK = abs(getMidiValue() - value) < 4;
  }
  if (controllerOK) {

    if (midiSign == 1) {
      slider->setValue( slider->minValue() + rint(float(slider->maxValue() - slider->minValue()) * (float)value / 127.0));
    } else {
      slider->setValue( slider->maxValue() - rint(float(slider->maxValue() - slider->minValue()) * (float)value / 127.0));
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

void FloatIntMidiSlider::updateSlider(int value) {

  slider->setValue(value);
}

int FloatIntMidiSlider::getMidiValue() {
  
  int x;
 
  x = rint(float(slider->value() -  slider->minValue())) * 127.0 / float(slider->maxValue() - slider->minValue());
  return(x);
}
