#ifndef INTMIDIBINDING_H
#define INTMIDIBINDING_H

#include "qobject.h"
#include "midibinding.h"
#include "midicontroller.h"
#include "intparameter.h"


/*! @file intmidibinding.h
 *
 *  @brief
 *  Declaration of class IntMidiBinding
 *  @author Lukas Degener
 */

/*! @class IntMidiBinding
 *
 * @brief A Midi binding to a IntParameter
 *
 * An instance of this class represents a map of a certain kind of midi events to
 * a int value represented by an instance of IntParameter
 *
 *
 * the CtrlMapTypes supported by this class are
 *
 * @arg @c MidiBinding::LinearMap      Midi values are mapped linear into the range defined by
 *                                        IntParameter::mMin and IntParameter::mMax.
 *
 * @arg @c MidiBinding::LogarithmicMap as above, but the mapping is logharithmic
 *
 * @arg @c MidiBinding::OnToggleMap    A midi value that is equal greater than mOnThreshold will make the
 *                                        Parameter toggle between IntParameter::mMin and IntParameter::mMax
 *
 * @arg @c MidiBinding::OnOffToggleMap A midi value that is equal greater than mOnThreshold will set the
 *                                        Parameter to IntParameter::mMax, a value below mOffThreshold will set it
 *                                        to IntParameter::mMin. or vice versa if mSign is negative
 *
 * @arg @c MidiBinding::OnCycleMa p    A midi value that is equal greater than mOnThreshold will increment the
 *                                        parameter by 1 or decrement it if mSign<0
 */
class IntMidiBinding: public MidiBinding{
   Q_OBJECT
private:
   IntParameter * mParameter;

public:
   IntMidiBinding(IntParameter * param, MidiController * ctrl,bool autoDelete=true, const char * name = 0);
   ~IntMidiBinding();

   /*!
    * @returns <code>LinearMap|LogarithmicMap|OnToggleMap|OnOffToggleMap|OnCycleMap</code> per definition
    */
   virtual int mapTypes();

   /*!
    * @returns <code>LinearMap</code> per definition
    */
   virtual CtrlMapType defaultMapType();
private slots:

public slots:
   virtual void setMidiValue(int value);
};
#endif
