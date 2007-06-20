#ifndef MIDISLIDERBASE_H
#define MIDISLIDERBASE_H

#include <qslider.h>
#include <qlabel.h>
#include "synthdata.h"
#include "midiguicomponent.h"

/**
  *@author Karsten Wiese
  */

#define SLIDER_SCALE 16384.0

class MidiSliderBase: public MidiGUIcomponent {
Q_OBJECT

public:
  QSlider *slider;
  QLabel *valueLabel;
  QLabel *minLabel, *maxLabel;

public:
  MidiSliderBase(Module *parentModule, Qt::Orientation orientation,
		 QWidget * parent, const QString &name,
		 int min, int max, int step, int value,
		 const QString &sMin,
		 const QString &sMax
	  );

public slots:
  void updateSlider(int value) {
    slider->setValue(value);
  }
};

#endif
