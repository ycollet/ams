#ifndef ENUMPARAMETEREDITOR_H
#define ENUMPARAMETEREDITOR_H

#include <qobject.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <iostream.h>
#include <qgrid.h>

#include "parameter.h"
#include "enumparameter.h"
#include "parametereditor.h"


/*! @file enumparametereditor.h
 *
 *  @brief
 *  Declaration of class EnumParameterEditor
 *  @author Lukas Degener
 */


/*! @class EnumParameterEditor
 *
 * @brief A widget for editing instances of the EnumParameter class.
 *
 * @todo: right now, there is no way to enable/disable certain items. i
 * have to find a way to visualize this adaequatly.
 */
class EnumParameterEditor : public ParameterEditor{

   Q_OBJECT

   private:

   EnumParameter * mParam;
   QPushButton * mHelpButton;
   QPushButton * mMidiButton;
   QLabel * mNameLabel;
   QComboBox * mComboBox;
   // All slots are private. No one should mess around with our
   // connections to the Parameter.
   private slots:
   // these slots are connected to the parameter instance
   void parameterDeleted();
   void selectionChanged(int key);
   void itemAdded(int key);
   void itemRemoved(int key,const QString &name);
   void itemStateChanged(int key);

   // these slots are connected to our own local gui components
   void helpButtonClicked();
   void midiButtonClicked();
   void itemHighlighted(int index);

   // There are _*NO*_ signals. Any information about whats happening to a parameter
   // should be fetched from the model (aka EnumParameter) itself.
   signals:
   void touched();
   public:
   EnumParameterEditor(EnumParameter * model, QWidget * parent = 0, const char * name = 0, WFlags f = 0);
   ~EnumParameterEditor();
   virtual Parameter * getParameter();
   private:
   void updateComboItems();
};

#endif
