#include <qslider.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <stdio.h>
#include <math.h>
#include "midislider.h"
#include "synthdata.h"
#include "midiwidget.h"
#include "midiguicomponent.h"

MidiSlider::MidiSlider(QObject *parentModule, float minValue, float maxValue, float pageStep, float value,
                       QSlider::Orientation orientation, QWidget * parent, const char * name, 
                       SynthData *p_synthdata, float *p_valueRef, bool p_isLog)
           : MidiGUIcomponent(parentModule, p_synthdata, parent, name) {

  QString qs;

  componentType = GUIcomponentType_slider;
  valueRef = p_valueRef;
  min = minValue;
  max = maxValue;
  initial_min = minValue;
  initial_max = maxValue;
  isLog = p_isLog;
  isMaster = true;
  setSpacing(1);
//  setMargin(1);
  new QWidget(this);
  QVBox *sliderBox = new QVBox(this);
  sliderBox->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
  QHBox *sliderNameBox = new QHBox(sliderBox);
  sliderNameBox->setMargin(1);
  new QWidget(sliderNameBox);
  QLabel *nameLabel = new QLabel(sliderNameBox);
  new QWidget(sliderNameBox);
  QHBox *sliderLabels = new QHBox(sliderBox);
  qs.sprintf("Min: %7.3f", minValue); 
  minLabel = new QLabel(qs, sliderLabels);
  new QWidget(sliderLabels);
  valueLabel = new QLabel("  ", sliderLabels);
  if (name) {
    qs.sprintf(" %s ", name);
  } else {
    qs.sprintf("  ");
  }
  nameLabel->setText(qs);
  new QWidget(sliderLabels);
  qs.sprintf("Max: %7.3f", maxValue);
  maxLabel = new QLabel(qs, sliderLabels);
  if (name) {
    nameLabel->setFixedHeight(nameLabel->sizeHint().height());
  }
  nameLabel->setFixedHeight(valueLabel->sizeHint().height());
  valueLabel->setFixedHeight(valueLabel->sizeHint().height());
  minLabel->setFixedHeight(minLabel->sizeHint().height());
  maxLabel->setFixedHeight(maxLabel->sizeHint().height());
  slider = new QSlider(int(SLIDER_SCALE * minValue), int(SLIDER_SCALE * maxValue), int(SLIDER_SCALE * pageStep), 
                       int(SLIDER_SCALE * value), orientation, sliderBox);  
  slider->setFixedHeight(slider->sizeHint().height());
  QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)));
  slider->setValue(int(SLIDER_SCALE * value));
  setLogMode(isLog);
  qs.sprintf(" %7.3f ", *valueRef);
  valueLabel->setText(qs);
}

MidiSlider::~MidiSlider(){
}

void MidiSlider::setMidiValue(int value) {

  if (!controllerOK) {
    controllerOK = abs(getMidiValue() - value) < 4;
  }
  if (controllerOK) {
    if (midiSign == 1) {
      slider->setValue( slider->minValue() + int(float(slider->maxValue() - slider->minValue()) * value / 127.0));
    } else {
      slider->setValue( slider->maxValue() - int(float(slider->maxValue() - slider->minValue()) * value / 127.0));
    }
  }
}

void MidiSlider::updateValue(int value) {

  QString qs;
  
  if (isLog) {
    *valueRef = exp((float)value / SLIDER_SCALE);
  } else {
    *valueRef = (float)value / SLIDER_SCALE;
  }
  qs.sprintf(" %7.3f ", *valueRef);
  valueLabel->setText(qs);
  emit valueChanged(value);
  emit guiComponentTouched();
}

void MidiSlider::updateSlider(int value) {

  slider->setValue(value);
}

int MidiSlider::getMidiValue() {
  
  float x;
 
  x = float(slider->value() -  slider->minValue()) * 127.0 / float(slider->maxValue() - slider->minValue());
  return(int(rint(x)));
}

void MidiSlider::setLogMode(bool on) {
  
  float log_min, log_val, val;

//  fprintf(stderr, "MidiSlider::setLogMode(%d)\n", on);
  if (isMaster && (isLog != on)) {
//    fprintf(stderr, "emit logModeChanged(%d)\n", on);
    emit logModeChanged(on);
  }
  isLog = on;
  val = *valueRef;
//  fprintf(stderr, "val: %f\n", val);
  if (isLog) {
    log_min = (min > 0) ? min : max / 10000.0;
    log_val = (val > 0 ) ? val : max / 10000.0;
    if (log_min < 0.001) log_min = 0.001;
    slider->setMinValue(int(SLIDER_SCALE * log(log_min)));
    slider->setMaxValue(int(SLIDER_SCALE * log(max)));
    slider->setValue(int(SLIDER_SCALE * log(log_val)));
  } else {
    slider->setMinValue(int(SLIDER_SCALE * min));
    slider->setMaxValue(int(SLIDER_SCALE * max));
    slider->setValue(int(SLIDER_SCALE * val));
  }
//  fprintf(stderr, "end of setLogMode\n");
}

void MidiSlider::setNewMin() {
  
  float val;
  QString qs;

  val = *valueRef;
  min = val;
  qs.sprintf("Min: %7.3f", min); 
  minLabel->setText(qs);
  if (isLog) {
    slider->setMinValue(int(SLIDER_SCALE * log(min)));
    slider->setValue(int(SLIDER_SCALE * log(val)));
  } else {
    slider->setMinValue(int(SLIDER_SCALE * min));
    slider->setValue(int(SLIDER_SCALE * val));
  }
  controllerOK = false;
}

void MidiSlider::setNewMax() {        
 
  float val;  
  QString qs;

  val = *valueRef;
  max = val;
  qs.sprintf("Max: %7.3f", max);      
  maxLabel->setText(qs);          
  if (isLog) {
    slider->setMaxValue(int(SLIDER_SCALE * log(max)));    
    slider->setValue(int(SLIDER_SCALE * log(val)));    
  } else {
    slider->setMaxValue(int(SLIDER_SCALE * max));
    slider->setValue(int(SLIDER_SCALE * val));
  }
  controllerOK = false;
}  

void MidiSlider::setNewMin(int p_min) {
  
  QString qs;

  min = (isLog) ? exp((double)p_min / SLIDER_SCALE) : (double)p_min / SLIDER_SCALE;
  qs.sprintf("Min: %7.3f", min); 
  minLabel->setText(qs);
  slider->setMinValue(int(p_min));
  controllerOK = false;
}

void MidiSlider::setNewMax(int p_max) {        
 
  QString qs;

  max = (isLog) ? exp((double)p_max / SLIDER_SCALE) : (double)p_max / SLIDER_SCALE;
  qs.sprintf("Max: %7.3f", max);      
  maxLabel->setText(qs);          
  slider->setMaxValue(int(p_max));
  controllerOK = false;
}  

void MidiSlider::setInitialMinMax() {        

  float val;   
  QString qs;

  val = *valueRef;
  min = initial_min;
  max = initial_max;
  qs.sprintf("Min: %7.3f", min);      
  minLabel->setText(qs);          
  qs.sprintf("Max: %7.3f", max);
  maxLabel->setText(qs);        
  slider->setMinValue(int(SLIDER_SCALE * min));  
  slider->setMaxValue(int(SLIDER_SCALE * max));
  slider->setValue(int(SLIDER_SCALE * val));
  controllerOK = false;
}  

float MidiSlider::getValue() {

  return(*valueRef);
}
