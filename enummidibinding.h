#ifndef ENUMMIDIBINDING_H
#define ENUMMIDIBINDING_H

#include "qobject.h"
#include "qvaluelist.h"
#include "midibinding.h"
#include "enumparameter.h"
#include "midicontroller.h"


/*! @file enummidibinding.h
 *
 *  @brief
 *  Declaration of class EnumMidiBinding
 *  @author Lukas Degener
 */

/*! @class EnumMidiBinding
 *
 * @brief A Midi binding to a EnumParameter
 *
 * An instance of this class represents a map of a certain kind of midi events to
 * a enum-like value represented by an instance of EnumParameter
 *
 *
 * the CtrlMapTypes supported by this class are
 *
 * @arg @c MidiBinding::ModuloMap  if there are n enabled items in the EnumParameter, then calling
 *                                       setMidiValue(int) will select the i-th enabled item such that
 *                                       the midi value is aequivalent to i in modulo n
 *
 *
 * @arg @c MidiBinding::OnCycleMap if setMidiValue is called with a value equal or greater than
 *                                    MidiBinding::mOnThreshold, the parameter's next enabled item will be
 *                                    selected if mSign is positiv, or the previous, if mSign is negativ.
 *                                    if there is no next (previous) enabled item then the first (last) enabled item
 *                                    will be selected
 *
 */
class EnumMidiBinding: public MidiBinding{
   Q_OBJECT
private:
   /*! @brief pointer to the EnumParameter edited by this object
    */
   EnumParameter * mParameter;

   /*! @brief the currently enabled items
    *
    * this list is updated whenever an item state changes or an item is removed/added to the parameter
    * so it should always be in-sync with the parameter.
    */
   QValueList<int> mEnabledItems;
public:
   EnumMidiBinding(EnumParameter * param,MidiController * ctrl,bool autoDelet=true, const char * name = 0);


   ~EnumMidiBinding();


    /*!
    * @returns OnCycleMap|ModuloMap per definition
    */
   virtual int mapTypes();

   /*! @brief
    * @returns ModuloMap per definition
    */
   virtual CtrlMapType defaultMapType();
private slots:
   // these slots are connected to the parameter instance

   /*! @brief connected to EnumParameter::itemAdded(int)
    */
   void itemAdded(int key);

   /*! @brief connected to EnumParameter::itemRemoved(int,QString)
    */
   void itemRemoved(int key, QString name);

   /*! @brief connected to EnumParameter::itemStateChanged(int)
    */
   void itemStateChanged(int key);

public slots:
   virtual void setMidiValue(int value);
};
#endif
