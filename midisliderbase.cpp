#include <QHBoxLayout>
#include <QVBoxLayout>
#include "midisliderbase.h"

/**
  *@author Karsten Wiese
  */

MidiSliderBase::MidiSliderBase(Module *parentModule,
			       Qt::Orientation orientation,
			       QWidget *parent, const QString &name,
			       int min, int max, int step, int value,
			       const QString &sMin,
			       const QString &sMax
			       )
  : MidiGUIcomponent(parentModule, parent, name)
{
  QVBoxLayout *sliderBox = new QVBoxLayout(this);
  sliderBox->setSpacing(0);
  sliderBox->setMargin(0);
  QLabel *nameLabel = new QLabel(name);
  sliderBox->addWidget(nameLabel, 0, Qt::AlignHCenter);
  QHBoxLayout *sliderLabels = new QHBoxLayout();
  sliderBox->addLayout(sliderLabels);
  minLabel = new QLabel(sMin);
  sliderLabels->addWidget(minLabel, 0);
  valueLabel = new QLabel();
  sliderLabels->addWidget(valueLabel, 0, Qt::AlignHCenter);
  maxLabel = new QLabel(sMax);
  sliderLabels->addWidget(maxLabel, 0, Qt::AlignRight);

  slider = new QSlider(orientation);
  slider->setMinimum(min);
  slider->setMaximum(max);
  slider->setPageStep(step);
  slider->setValue(value);
  sliderBox->addWidget(slider);
}
