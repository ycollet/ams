#ifndef ACTIONPARAMETEREDITOR_H
#define ACTIONPARAMETEREDITOR_H

#include <qhbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>

#include "actionparameter.h"
#include "parametereditor.h"
/*! @file actionparametereditor.h
 *
 *  @brief
 *  Declaration of class ActionParameterEditor
 *  @author Lukas Degener
 */

/*! @class ActionParameterEditor
 *
 *  @brief A GUI Widget to trigger an ActionParameter
 *
 */
class ActionParameterEditor : public ParameterEditor{

   Q_OBJECT

   private:

   ActionParameter * mParam;

   bool mAnimate;

   QCheckBox * mCheckBox;
   QPushButton * mHelpButton;
   QPushButton * mMidiButton;
   QPushButton * mTriggerButton;

   private slots:
   // these slots are connected to the parameter
   void parameterDeleted();
   void triggered();
   void stateChanged(bool enabled);


   // these slots are connected to our own local gui components
   void checkBoxToggled(bool val);
   void helpButtonClicked();
   void midiButtonClicked();
   void triggerButtonClicked();
   public:
   ActionParameterEditor(ActionParameter * model, QWidget * parent = 0, const char * name = 0, WFlags f = 0);
   ~ActionParameterEditor();
   virtual Parameter * getParameter();
};
#endif
