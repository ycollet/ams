#ifndef ACTIONPARAMETER_H
#define ACTIONPARAMETER_H


#include <float.h>
#include <math.h>
#include <qstring.h>
#include <qobject.h>


#include "parameter.h"
/*! @file actionparameter.h
 *
 *  @brief
 *  Declaration of class ActionParameter
 *  @author Lukas Degener
 */




/*! @brief a Parameter representing an action.
 *
 * Not a real parameter, but implemented anyway. This defines an action which can be triggered if it is enabled
 */
class ActionParameter : public Parameter{

   Q_OBJECT

   private:
   /*! @brief true if Action can be triggered
    *
    * The action represented by this parameter can only be triggered if this value is true.
    * @see setEnabled(bool), enabled()
    */
   bool mEnabled;


   public:
   ActionParameter(QObject * parent=0, QString name="unnamed parameter",
                  QString descr="no description", bool p_enable=true);

   ~ActionParameter();
   public slots:
   /*! @brief trigger this action
    *
    * if mEnabled is true, this will emit triggered()
    */
   void trigger();

   /*! @brief get the actions enabled/disabled state
    *
    * @returns true if this action can be triggered
    * @see setEnabled(bool), mEnabled
    */
   bool enabled();

   /*! @brief set the actions enabled/disabled state
    *
    * @param val if this is set true, the action can be triggered
    * @see enabled(), mEnabled
    */
   void setEnabled(bool val);

   signals:
   /*! @brief emited when the action is triggered
    * 
    * This signal is emited whenever trigger() is called while mEnabled is true
    */
   void triggered();
   
   /*! @brief emited when action state changes
    *
    * this signal is emited whenever setEnabled(bool) is called
    */
   void stateChanged(bool val);
};

#endif
