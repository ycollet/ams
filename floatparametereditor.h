#ifndef FLOATPARAMETEREDITOR_H
#define FLOATPARAMETEREDITOR_H

#include <qobject.h>
#include <qvbox.h>
#include <qslider.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpopupmenu.h>
#include <iostream.h>
#include <qgrid.h>
#include "parametereditor.h"
#include "parameter.h"
#include "floatparameter.h"

#define SLIDER_SCALE 16384.0


/*! @file floatparametereditor.h
 *
 *  @brief
 *  Declaration of class FloatParameterEditor
 *  @author Lukas Degener
 */


/*! @class FloatParameterEditor
 *
 * @brief A Gui Widget for editing an instance of FloatParameter
 */

class FloatParameterEditor :  public ParameterEditor{

   Q_OBJECT

   private:
   float mStep; //not used right now
   FloatParameter * mParam;
   QPushButton * mMinButton;
   QPushButton * mMaxButton;
   QPushButton * mResetMinButton;
   QPushButton * mResetMaxButton;
   QPushButton * mHelpButton;

   QPushButton * mMidiButton;
   QLabel * mMinLabel;
   QLabel * mMaxLabel;
   QLabel * mValueLabel;
   QLabel * mNameLabel;
   QHBox * mHBox;
   QHBox * mMinHBox;
   QHBox * mMaxHBox;
   QGrid * mGrid;
   QSlider * mSlider;

   // All slots are private. No one should mess around with our
   // connections to the Parameter.
   private slots:
   // these slots are connected to the parameter instance
   void valueChanged(float val);
   void minChanged(float val);
   void maxChanged(float val);
   void parameterDeleted();

   // these slots are connected to our own local gui components
   void sliderMoved(int val);
   void maxButtonClicked();
   void minButtonClicked();
   void resetMaxButtonClicked();
   void resetMinButtonClicked();
   void helpButtonClicked();

   void midiButtonClicked();

   // There are no signals. Any information about whats happening to a parameter
   // should be fetched from the model (aka FloatParameter) itself.
   signals:
   void touched();
   public:
   FloatParameterEditor(FloatParameter * model, QWidget * parent = 0, const char * name = 0, WFlags f = 0);
   ~FloatParameterEditor();
   virtual Parameter * getParameter();
};

#endif
