#ifndef BOOLPARAMETER_H
#define BOOLPARAMETER_H


#include <float.h>
#include <math.h>
#include <qstring.h>
#include <qobject.h>


#include "parameter.h"

/*! @file boolparameter.h
 *
 *  @brief
 *  Declaration of class BoolParameter
 *  @author Lukas Degener
 */


/*! @class BoolParameter
 *
 * @brief A Parameter representing a boolean value
 *
 * An instance of this class represents a boolean value.
 * When toggled the signal toggled(bool) is emited
 */
class BoolParameter : public Parameter{

   Q_OBJECT

   private:
   /*! @brief the current state of the Parameter
    *
    * Set this value with setOn(bool).
    * To get the current value, use isOn().
    */
   bool & mValue;
   bool mLocalValue;

   public:
   BoolParameter(QObject * parent=0, QString name="unnamed parameter",
                  QString descr="no description", bool p_value=false);

   
   /*! @brief alternative/old way
    *
    * This allows you to let the parameter operate on a given bool you give it
    * it a pointer to.
    *
    * It is a quick and dirty hack to allow complex modules to work with bool
    * arrays internaly and not care about parameter objects.
    *
    * @warning Please be very aware of the many risks this holds. Never delete the
    * value before deleting the parameter. do not use the bool as an lvalue. NEVER
    * have more than one Parameter operate on the same value. etc.
    *
    * I have to admit that i implemented this to get all the modules converted
    * quickly. As soon as i find some time to think about a better way, this
    * method will disapear.
    *
    *@deprecated Don't rely on this method, it will disapear soon (hopefully;-))
    */
    BoolParameter(QObject * parent=0, QString name="unnamed parameter",
                  QString descr="no description", bool * p_ref=0);

   ~BoolParameter();
   /*! @brief sets the current value to val
    *
    * @see isOn(), mValue
    */
   void setOn(const bool & val);

   /*! @brief returns a const pointer to the parameters value
    */
   const bool * valuePtr();
   operator const bool & ();
   const bool & operator=(const bool & orig);

   /*! @brief returns the current state of this parameter
    *
    * @see setOn(bool), mValue
    */
   bool isOn();

   signals:
   /*! @brief emited whenever setOn is called.
    */
   void toggled(bool val);
};

#endif
