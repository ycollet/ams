#ifndef FLOATMIDIBINDING_H
#define FLOATMIDIBINDING_H

#include "qobject.h"
#include "midibinding.h"
#include "floatparameter.h"
#include "midicontroller.h"

/*! @file floatmidibinding.h
 *
 *  @brief
 *  Declaration of class FloatMidiBinding
 *  @author Lukas Degener
 */

/*! @class FloatMidiBinding
 *
 * @brief A Midi binding to a FloatParameter
 *
 * An instance of this class represents a map of a certain kind of midi events to
 * a float value represented by an instance of FloatParameter
 *
 *
 * the CtrlMapTypes supported by this class are
 *
 * @arg @c MidiBinding::LinearMap      Midi values are mapped linear into the range defined by
 *                                        FloatParameter::mMin and FloatParameter::mMax.
 *
 * @arg @c MidiBinding::LogarithmicMap as above, but the mapping is logharithmic
 *
 * @arg @c MidiBinding::OnToggleMap    A midi value that is equal greater than mOnThreshold will make the
 *                                        Parameter toggle between FloatParameter::mMin and FloatParameter::mMax
 *
 * @arg @c MidiBinding::OnOffToggleMap A midi value that is equal greater than mOnThreshold will set the
 *                                        Parameter to FloatParameter::mMax, a value below mOffThreshold will set it
 *                                        to FloatParameter::mMin. or vice versa if mSign is negative
 */
class FloatMidiBinding: public MidiBinding{
   Q_OBJECT
private:
   FloatParameter * mParameter;

public:
   FloatMidiBinding(FloatParameter * param, MidiController * ctrl, bool autoDelete=true, const char * name = 0);
   ~FloatMidiBinding();

   /*!
    * @returns <code>LinearMap|LogarithmicMap|OnToggleMap|OnOffToggleMap</code> per definition
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
