#include <qslider.h>
#include <qhbox.h>
#include <qlabel.h>
#include <stdio.h>
#include <math.h>
#include "intmidislider.h"
#include "synthdata.h"
#include "midiwidget.h"
#include "midiguicomponent.h"

IntMidiSlider::IntMidiSlider(QObject *parentModule, int minValue, int maxValue, int pageStep, int value,
                       QSlider::Orientation orientation, QWidget * parent, const char * name, 
                       SynthData *p_synthdata, int *p_valueRef)
           : MidiGUIcomponent(parentModule, p_synthdata, parent, name) {

  QString qs;

  componentType = GUIcomponentType_intslider;
  valueRef = p_valueRef;
  setSpacing(5);
  setMargin(5);
  new QWidget(this);
  QVBox *sliderBox = new QVBox(this);
  sliderBox->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
  QHBox *sliderNameBox = new QHBox(sliderBox);
  new QWidget(sliderNameBox);
  sliderNameBox->setMargin(5);
  QLabel *nameLabel = new QLabel(sliderNameBox);
  new QWidget(sliderNameBox);
  QHBox *sliderLabels = new QHBox(sliderBox);
  qs.sprintf("Min: %d", minValue); 
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
  qs.sprintf("Max: %d", maxValue);
  QLabel *maxLabel = new QLabel(qs, sliderLabels);
  if (name) {
    nameLabel->setFixedHeight(nameLabel->sizeHint().height());
  }
  nameLabel->setFixedHeight(valueLabel->sizeHint().height());
  valueLabel->setFixedHeight(valueLabel->sizeHint().height());
  minLabel->setFixedHeight(minLabel->sizeHint().height());
  maxLabel->setFixedHeight(maxLabel->sizeHint().height());
  slider = new QSlider(minValue, maxValue, pageStep, 
                       value, orientation, sliderBox);  
  slider->setTickInterval((abs(maxValue) + abs(minValue)) / 10);
  slider->setTickmarks(QSlider::Below);
  slider->setFixedHeight(slider->sizeHint().height());
  QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)));
  slider->setValue(value);
  qs.sprintf(" %d ", *valueRef);
  valueLabel->setText(qs);
}

IntMidiSlider::~IntMidiSlider(){
}

void IntMidiSlider::setMidiValue(int value) {

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

void IntMidiSlider::updateValue(int value) {

  QString qs;
  
  *valueRef = value;
  qs.sprintf(" %d ", *valueRef);
  valueLabel->setText(qs);
  emit valueChanged(value);
  emit guiComponentTouched();
}

void IntMidiSlider::updateSlider(int value) {

  slider->setValue(value);
}

int IntMidiSlider::getMidiValue() {
  
  int x;
 
  x = rint(float(slider->value() -  slider->minValue())) * 127.0 / float(slider->maxValue() - slider->minValue());
  return(x);
}
