#ifndef BOOLMIDIBINDING_H
#define BOOLMIDIBINDING_H

#include "qobject.h"
#include "midibinding.h"
#include "boolparameter.h"
#include "midicontroller.h"

/*! @file boolmidibinding.h
 *
 *  @brief
 *  Declaration of class BoolMidiBinding
 *  @author Lukas Degener
 */


/*! @class BoolMidiBinding
 *
 * @brief A Midi binding to a BoolParameter
 *
 * An instance of this class represents a map of a certain kind of midi events to
 * a boolean value represented by an instance of BoolParameter
 *
 *
 * the CtrlMapTypes supported by this class are
 *
 * @arg @c MidiBinding::OnToggleMap  the parameter value will be toggled whenever setMidiValue(int) is called with an
 *                                      argument equal or greater than MidiBinding::mOnThreshold
 *
 * @arg @c MidiBinding::OnOffToggleMap the parameter value will be set to <code>true</code> whenever
 *                                        setMidiValue(int) is called with an argument equal or greater than
 *                                        MidiBinding::mOnThreshold, it will be set to <code>false</code>
 *                                        whenever the argument is less than mOffThreshold, or vice versa if
 *                                        MidiBinding::mSign is negativ.
 */
class BoolMidiBinding: public MidiBinding{
   Q_OBJECT
private:
   /*! @brief a pointer to the BoolParameter object that is edited by this object
    */
   BoolParameter * mParameter;

public:
   BoolMidiBinding(BoolParameter * param,MidiController * ctrl,bool autoDelete=true, const char * name = 0);
   ~BoolMidiBinding();

   /*!
    * @returns OnToggleMap|OnOffToggleMap per definition
    */
   virtual int mapTypes();

   /*!
    * @returns OnOffToggleMap per definition
    */
   virtual CtrlMapType defaultMapType();
private slots:

public slots:
   virtual void setMidiValue(int value);

};
#endif
