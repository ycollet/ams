#ifndef ACTIONMIDIBINDING_H
#define ACTIONMIDIBINDING_H

#include "qobject.h"
#include "midibinding.h"
#include "actionparameter.h"
#include "midicontroller.h"

/*! @file actionmidibinding.h
 *
 *  @brief
 *  Declaration of class ActionMidiBinding
 *  @author Lukas Degener
 */

/*! @class ActionMidiBinding
 *  @brief represents a binding of certain midi controller events to an instance of class ActionParameter.
 */
class ActionMidiBinding: public MidiBinding{
   Q_OBJECT
private:
   /*! a pointer to the ActionParameter object */
   ActionParameter * mParameter;

public:
   ActionMidiBinding(ActionParameter * param, MidiController * ctrl,bool autoDelete=true, const char * name = 0);
   ~ActionMidiBinding();

   /*! @brief valid mappings for this binding
    *
    * This method will return an int consisting of all mappings that are valid for this binding ORed together into an int
    * @returns MidiBinding::OnOffToggle|MidiBinding::OnToggle by definition
    * @see MidiBinding:CtrlMapType
    */
   virtual int mapTypes();

   /*! @brief the default mapping used by this binding.
    *  @returns MidiBinding::OnOffToggle by definition.
    */
   virtual CtrlMapType defaultMapType();

private slots:


public slots:

   /*! @brief manipulate the connected IntParameter
    *
    * the behabiour of this method depends on the current CtrlMapType. If it is <code>OnToggle</code>, the connected ActionParameter
    * will be triggerd if <code>value</code> is greater or equal than MidiBinding::OnThreshold. If it is <code>OnOffToggle</code>,
    * it will be triggered if the above applies, or if <code>value</code> is less than MidiBinding::OffThreshold.
    *
    * Triggered means, that ActionParameter::trigger() will be called.
    */
   virtual void setMidiValue(int value);
};
#endif
