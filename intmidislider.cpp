#include <qslider.h>

#include <qlabel.h>
#include <stdio.h>
#include <math.h>
#include "intmidislider.h"
#include "synthdata.h"
#include "midiwidget.h"
#include "midiguicomponent.h"

IntMidiSlider::IntMidiSlider(Module *parentModule, int minValue, int maxValue, int pageStep, int value,
                       Qt::Orientation orientation, QWidget * parent, const QString &name, 
                       int *p_valueRef)
  : MidiSliderBase(parentModule, orientation, parent, name,
		   minValue, maxValue, pageStep, value,
		   QString().sprintf("Min: %d", minValue),
		   QString().sprintf("Max: %d", maxValue))
{

  QString qs;

  componentType = GUIcomponentType_intslider;
  valueRef = p_valueRef;
//   setSpacing(1);
//   new QWidget(this);
//   Q3VBox *sliderBox = new Q3VBox(this);
//   sliderBox->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
//   Q3HBox *sliderNameBox = new Q3HBox(sliderBox);
//   new QWidget(sliderNameBox);
//   sliderNameBox->setMargin(1);
//   QLabel *nameLabel = new QLabel(sliderNameBox);
//   new QWidget(sliderNameBox);
//   Q3HBox *sliderLabels = new Q3HBox(sliderBox);
//   qs.sprintf("Min: %d", minValue); 
//   QLabel *minLabel = new QLabel(qs, sliderLabels);
//   new QWidget(sliderLabels);
//   valueLabel = new QLabel("  ", sliderLabels);
//   if (name) {
//     qs.sprintf(" %s ", name);
//   } else {
//     qs.sprintf("  ");
//   }
//   nameLabel->setText(qs);
//   new QWidget(sliderLabels);
//   qs.sprintf("Max: %d", maxValue);
//   QLabel *maxLabel = new QLabel(qs, sliderLabels);
//   if (name) {
//     nameLabel->setFixedHeight(nameLabel->sizeHint().height());
//   }
//   nameLabel->setFixedHeight(valueLabel->sizeHint().height());
//   valueLabel->setFixedHeight(valueLabel->sizeHint().height());
//   minLabel->setFixedHeight(minLabel->sizeHint().height());
//   maxLabel->setFixedHeight(maxLabel->sizeHint().height());
//   slider = new QSlider(minValue, maxValue, pageStep, 
//                        value, orientation, sliderBox);  
  slider->setTickInterval((abs(maxValue) + abs(minValue)) / 10);
  slider->setTickPosition(QSlider::TicksBelow);
  slider->setFixedHeight(slider->sizeHint().height());
  QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)));
  slider->setValue(value);
  qs.sprintf(" %d ", *valueRef);
  valueLabel->setText(qs);
}


void IntMidiSlider::setMidiValue(int value) {

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

void IntMidiSlider::updateValue(int value) {

  QString qs;
  
  *valueRef = value;
  qs.sprintf(" %d ", *valueRef);
  valueLabel->setText(qs);
  emit valueChanged(value);
  emit guiComponentTouched();
}

int IntMidiSlider::getMidiValue() {
  
  int x;
 
  x = rint(float(slider->value() -  slider->minimum())) * 127.0 / float(slider->maximum() - slider->minimum());
  return(x);
}
