#ifndef BOOLPARAMETEREDITOR_H
#define BOOLPARAMETEREDITOR_H

#include <qhbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include "parametereditor.h"
#include "boolparameter.h"
/*! @file boolparametereditor.h
 *
 *  @brief
 *  Declaration of class BoolParameterEditor
 *  @author Lukas Degener
 */


/*! @class BoolParameterEditor
 *
 * @brief A Gui Widget for editing an instance of BoolParameter
 */
class BoolParameterEditor :public ParameterEditor{
   
   Q_OBJECT

   private:
   BoolParameter * mParam;

   QCheckBox * mCheckBox;
   QPushButton * mHelpButton;
   QPushButton * mMidiButton;

   private slots:
   // these slots are connected to the parameter
   void parameterDeleted();
   void toggled(bool val);

   // these slots are connected to our own local gui components
   void checkBoxToggled(bool val);
   void helpButtonClicked();
   void midiButtonClicked();
   public:
   BoolParameterEditor(BoolParameter * model, QWidget * parent = 0, const char * name = 0, WFlags f = 0);
   ~BoolParameterEditor();
   virtual Parameter * getParameter();
};
#endif
