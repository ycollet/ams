#ifndef INTPARAMETER_H
#define INTPARAMETER_H


#include <float.h>
#include <math.h>
#include <qstring.h>
#include <qobject.h>

#include <qlist.h>


#include "midicontroller.h"


#include "parameter.h"



/*! @file intparameter.h
 *
 *  @brief
 *  Declaration of class IntParameter
 *  @author Lukas Degener
 */

/*! @class IntParameter
 *
 * @brief A Parameter representing an int value
 *
 * An instance of this class represents an int value within certain bounds.
 *
 * Actualy there are two pairs of bounds:
 *
 * - The 'real' bounds are set duringe instantiation and are represented by the member fields mRealMax and mRealMin.
 *   Those two values should define the range within which this parameter does make sense in its application context.
 *   E.g. you would not want to allow negativ frequencies to an oscilator, etc. The real bounds can never be changed
 *   for as long as the IntParameter instance lives.
 *
 * - The 'user-defined' bounds are, as you would have guessed, defined by the user. they can be changed at any time.
 *   They are represented by the member fields mMin and mMax
 *
 * Each instance of IntParameter guaranties that the following is always true:
 *
 * <code>mRealMin <= mMin <= mValue <= mMax <= mRealMax</code
 *
 */
class IntParameter : public Parameter{

   Q_OBJECT

   private:
   /*! @brief the current value of the parameter
    *
    * @see setValue(const int & value), value()
    */
   int & mValue;
   int mLocalValue;

   /*! @brief the 'real' maximum of the parameter
    *
    * @see realMaxValue()
    */
   int mRealMax;

   /*! @brief the 'real' minimum of the parameter
    *
    * @see realMinValue()
    */
   int mRealMin;

   /*! @brief the 'user-defined' maximum of the parameter
    *
    * @see maxValue(), setMaxValue(const int &)
    */
   int mMax;

   /*! @brief the 'user-defined' minimum of the parameter
    *
    * @see maxValue(), setMinValue(const int &)
    */
   int mMin;

   public:
   IntParameter(QObject * parent=0, QString name="unnamed parameter",
                  QString descr="no description",
                  int p_min=INT_MIN, int p_max=INT_MAX, int p_value=0);

   /*! @brief alternative/old way
    *
    * This allows you to let the parameter operate on a given int you give it
    * it a pointer to.
    *
    * It is a quick and dirty hack to allow complex modules to work with int
    * arrays internaly and not care about parameter objects.
    *
    * @warning Please be very aware of the many risks this holds. Never delete the
    * value before deleting the parameter. do not use the int as an lvalue. NEVER
    * have more than one Parameter operate on the same value. etc.
    *
    * I have to admit that i implemented this to get all the modules converted
    * quickly. As soon as i find some time to think about a better way, this
    * method will disapear.
    *
    *@deprecated Don't rely on this method, it will disapear soon (hopefully;-))
    */
    IntParameter(QObject * parent=0, QString name="unnamed parameter",
                  QString descr="no description",
                  int p_min=INT_MIN, int p_max=INT_MAX, int * p_ref=0);


   ~IntParameter();

   /*! @brief set the parameters value
    *
    * sets the value of the parameter. If val is greater (less) than mMax (mMin), than
    * mValue will be set to mMax (mMin).
    *
    * @returns the new value of this parameter
    */
   int setValue(const int & val);

   /*! @brief set the user-defined minimum
    *
    * sets the user defined minimum of this parameter.
    *
    * - If val is greater (less) than mMax (mRealMin), than
    *   mMin will be set to mMaxV (mRealMin).
    *
    * - If after that mValue is greater (less) than mMax (mMin), than
    *   mValue will be set to mMax (mMin)
    *
    *  minValueChanged(int) and valueChanged(int) will be emmited accordingly
    *
    * @returns the new value of mMin
    */
  int setMinValue(const int & val);
  /*! @brief set the user-defined maximum
    *
    * sets the user defined maximum of this parameter.
    *
    * - If val is greater (less) than mRealMax (mMin), than
    *   mMax will be set to mRealMax (mMin).
    *
    * - If after that mValue is greater (less) than mMax (mMin), than
    *   mValue will be set to mMax (mMin)
    *
    *  maxValueChanged(int) and valueChanged(int) will be emmited accordingly
    *
    * @returns the new value of mMax
    */
  int setMaxValue(const int & val);

   /*! @brief const pointer to mValue
    */
   const int * valuePtr();
   operator const int &();
   const int & operator=(const int & orig);
   /*! @brief get the current value
    */
   int value();

   /*! @brief get the user defined minimum
    */
   int minValue();

   /*! @brief get the user defined maximum
    */
   int maxValue();

   /*! @brief get the user defined maximum
    */
   int realMinValue();

   /*! @brief get the real maximum
    */
   int realMaxValue();

   signals:
   /*! @brief emited if mValue changed
    */
   void valueChanged(int val);

   /*! @brief emmited if mMin changed
    */
   void minChanged(int val);

   /*! @brief emmited if mMax chaned
    */
   void maxChanged(int val);
};

#endif
