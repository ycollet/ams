#include <qslider.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qstrlist.h>
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

  QString qs, qs1, qs2, qs3, qs4;
  int m1, m2, l1;

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
  comboFlag = false;
  mode = 0;
  QStrList *paramNames = new QStrList(true);
  if (name) {
    qs.sprintf(" %s ", name);
    if ((qs.contains("0:", false) || qs.contains(":0")) && (qs.contains("1:", false) || qs.contains(":1")) && qs.contains('\n')) {
      comboFlag = true;
      mode = 1;
    }
    if (comboFlag) {
      if (mode == 1) {
        for (l1 = 0; l1 < 64; l1++) {
          qs1.sprintf("%d:", l1);
          qs2.sprintf("%d:", l1 + 1);
          if ((m1 = qs.find(qs1)) >= 0) {
            m2 = qs.find('\n', m1 + 3);
            if (m2 < 0) {
              m2 = qs.length() - 1;
            }
            qs3 = qs.mid(m1+qs1.length(), m2-m1-qs1.length());
            qs4 = qs3.simplifyWhiteSpace();
            paramNames->append(qs4);
            continue;
          }
          qs1.sprintf(":%d", l1);
          qs2.sprintf(":%d", l1 + 1);
          if ((m1 = qs.find(qs1)) >= 0) {
            if ((m2 = qs.findRev('\n', m1 - 3)) >= 0) {
              qs3 = qs.mid(m2 + 1, m1-m2-1);
              qs4 = qs3.simplifyWhiteSpace();
              paramNames->append(qs4);
            }
          }
        }  
      }
    }
  } else {
    qs.sprintf("  ");
  }
  if (comboFlag) {
    m1 = qs.find('\n');
    qs1 = qs.left(m1 - 1);
    nameLabel->setText(qs1);
  } else {
    nameLabel->setText(qs);
  }  
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
  if (name && comboFlag) {
    combobox = new QComboBox(sliderBox);
    combobox->insertStrList(paramNames);
    combobox->setCurrentItem(0);
    QObject::connect(combobox, SIGNAL(highlighted(int)), this, SLOT(updateValue(int)));
  } 
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
      if (comboFlag) {
        combobox->setCurrentItem(value);
      }  
      slider->setValue( slider->minValue() + rint(float(slider->maxValue() - slider->minValue()) * (float)value / 127.0));
    } else {
      if (comboFlag) {
        combobox->setCurrentItem(value);
      }   
      slider->setValue( slider->maxValue() - rint(float(slider->maxValue() - slider->minValue()) * (float)value / 127.0));
    }
  }
}

void FloatIntMidiSlider::updateValue(int value) {

  QString qs;

  updateSlider(value);  
  *valueRef = value;
  qs.sprintf(" %d ", (int)*valueRef);
  valueLabel->setText(qs);
  emit valueChanged(value);
  emit guiComponentTouched();
}

void FloatIntMidiSlider::updateSlider(int value) {

  if (comboFlag) {  
    combobox->setCurrentItem(value);
  }
  slider->setValue(value);
}

int FloatIntMidiSlider::getMidiValue() {
  
  int x;
 
  x = rint(float(slider->value() -  slider->minValue())) * 127.0 / float(slider->maxValue() - slider->minValue());
  return(x);
}
