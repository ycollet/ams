#ifndef FLOATPARAMETER_H
#define FLOATPARAMETER_H


#include <float.h>
#include <math.h>
#include <qstring.h>
#include <qobject.h>

#include <qlist.h>


#include "midicontroller.h"

#include "parameter.h"

/*! @file floatparameter.h
 *
 *  @brief
 *  Declaration of class FloatParameter
 *  @author Lukas Degener
 */

/*! @class FloatParameter
 *
 * @brief A Parameter representing a float value
 *
 * An instance of this class represents a float value within certain bounds.
 *
 * Actualy there are two pairs of bounds:
 *
 * - The 'real' bounds are set duringe instantiation and are represented by the member fields mRealMax and mRealMin.
 *   Those two values should define the range within which this parameter does make sense in its application context.
 *   E.g. you would not want to allow negativ frequencies to an oscilator, etc. The real bounds can never be changed
 *   for as long as the FloatParameter instance lives.
 *
 * - The 'user-defined' bounds are, as you would have guessed, defined by the user. they can be changed at any time.
 *   They are represented by the member fields mMin and mMax
 *
 * Each instance of FloatParameter guaranties that the following is always true:
 *
 * <code>mRealMin <= mMin <= mValue <= mMax <= mRealMax</code
 *
 */


class FloatParameter : public Parameter{

   Q_OBJECT

   private:
   /*! @brief the current value of the parameter
    *
    * @see setValue(const float & value), value()
    */
   float  & mValue;
   float mLocalValue;
   /*! @brief the 'real' maximum of the parameter
    *
    * @see realMaxValue()
    */
   float mRealMax;

   /*! @brief the 'real' minimum of the parameter
    *
    * @see realMinValue()
    */
   float mRealMin;

   /*! @brief the 'user-defined' maximum of the parameter
    *
    * @see maxValue(), setMaxValue(const float &)
    */
   float mMax;

   /*! @brief the 'user-defined' minimum of the parameter
    *
    * @see maxValue(), setMaxValue(const float &)
    */
   float mMin;

   public:
   /*! @brief recommended/new way
    */
   FloatParameter(QObject * parent=0, QString name="unnamed parameter",
                  QString descr="no description",
                  float p_min=FLT_MIN, float p_max=FLT_MAX, float p_value=0);

   /*! @brief alternative/old way
    *
    * This allows you to let the parameter operate on a given float you give it
    * it a pointer to.
    *
    * It is a quick and dirty hack to allow complex modules to work with float
    * arrays internaly and not care about parameter objects.
    *
    * @warning Please be very aware of the many risks this holds. Never delete the
    * value before deleting the parameter. do not use the float as an lvalue. NEVER
    * have more than one Parameter operate on the same value. etc.
    *
    * I have to admit that i implemented this to get all the modules converted 
    * quickly. As soon as i find some time to think about a better way, this
    * method will disapear.
    *
    *@deprecated Don't rely on this method, it will disapear soon (hopefully;-))
    */
   FloatParameter(QObject * parent=0, QString name="unnamed parameter",
                  QString descr="no description",
                  float p_min=FLT_MIN, float p_max=FLT_MAX, float * p_ref=0);

   ~FloatParameter();

   /*! @brief set the parameters value
    *
    * sets the value of the parameter. If val is greater (less) than mMax (mMin), than
    * mValue will be set to mMax (mMin).
    *
    * @returns the new value of this parameter
    */
   float setValue(const float & val);

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
    *  minValueChanged(float) and valueChanged(float) will be emmited accordingly
    *
    * @returns the new value of mMin
    */
   float setMinValue(const float & val);

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
    *  maxValueChanged(float) and valueChanged(float) will be emmited accordingly
    *
    * @returns the new value of mMax
    */
   float setMaxValue(const float & val);

   /*! @brief const pointer to mValue
    */
   const float * valuePtr();

   operator const float &();
   const float & operator=(const float & orig);

   /*! @brief get the current value
    */
   float value();

   /*! @brief get the user defined minimum
    */
   float minValue();

   /*! @brief get the user defined maximum
    */
   float maxValue();

   /*! @brief get the real minimum
    */
   float realMinValue();

   /*! @brief get the real maximum
    */
   float realMaxValue();

   signals:

   /*! @brief emited if mValue changed
    */
   void valueChanged(float val);

   /*! @brief emmited if mMin changed
    */
   void minChanged(float val);

   /*! @brief emmited if mMax chaned
    */
   void maxChanged(float val);
};

#endif
