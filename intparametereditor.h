#ifndef INTPARAMETEREDITOR_H
#define INTPARAMETEREDITOR_H

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

#include "parameter.h"
#include "intparameter.h"
#include "parametereditor.h"

#define SLIDER_SCALE 16384.0

/*! @file intparametereditor.h
 *
 *  @brief
 *  Declaration of class IntParameterEditor
 *  @author Lukas Degener
 */


/*! @class IntParameterEditor
 *
 * @brief A Gui Widget for editing an instance of IntParameter
 */
class IntParameterEditor : public ParameterEditor{

   Q_OBJECT

   private:
   int mStep; //not used right now
   IntParameter * mParam;
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
   void valueChanged(int val);
   void minChanged(int val);
   void maxChanged(int val);
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
   // should be fetched from the model (aka IntParameter) itself.
   signals:
   void touched();
   public:
   IntParameterEditor(IntParameter * model, QWidget * parent = 0, const char * name = 0, WFlags f = 0);
   ~IntParameterEditor();
   virtual Parameter * getParameter();
};

#endif
