#ifndef MIDICONTROLCENTER_H
#define MIDICONTROLCENTER_H

class MidiController;
class MidiControllerList;
class MidiBinding;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qptrdict.h>
#include <qlist.h>

#include "midicontroller.h"
#include "midicontrollerlist.h"
#include "midibinding.h"
#include "parameter.h"
#include "objectlistviewitem.h"

/*! @file midicontrolcenter.h
 *
 *  @brief
 *  Declaration of class MidiControlCenter
 *  @author Lukas Degener, Matthias Nagorni
 */

/*! @class MidiControlCenter
 *
 *  @brief replaces MidiWidget
 *
 *  @note i have changed this to be a Singleton Class. There will never be more than
 *        one instance of it.
 */
class MidiControlCenter : public QVBox{

   Q_OBJECT

   private:
   static MidiControlCenter * mInstance;
   QCheckBox *mNoteCheckBox, *mConfigCheckBox, *mMidiCheckBox;
   QListView *mControllerListView, *mParameterListView;
   QVBox *mEditorBox;
   QWidget *mCurrentEditor;
   QPushButton *mClearButton,*mBindButton;
   QComboBox *mBindingCombo;
   QCheckBox *mSignCheckBox;

   /*! @brief to quicklyfind listviewitems
    */
   QPtrDict<ObjectListViewItem> mListViewItems;

   MidiControllerList mControllers;

   Parameter * mSelectedParameter;
   MidiBinding * mSelectedBinding;
   MidiController * mSelectedController;

   MidiControlCenter(QWidget* parent=0, const char *name=0);
   public:
   static MidiControlCenter * getInstance();
   bool noteEventsEnabled();
   bool followMidiEnabled();
   bool followConfigEnabled();


    ~MidiControlCenter();

   /*! @brief get the selected Parameter
    *
    * @returns the selected Parameter or NULL if the selection is empty
    */
   Parameter * selectedParameter();

   /*! @brief get the selected MidiBinding
    *
    * @returns the selected Binding or NULL if no Binding is selected
    */
   MidiBinding * selectedBinding();

   /*! @brief get the selected MidiController
    *
    * @returns the selected MidiController or NULL if the selection is empty
    */
   MidiController * selectedController();
   
   /*! @warning this should be const. but i couldn't find a const function
    * to traverse a QPtrList.
    */
   MidiControllerList * controllers();
   public slots:

   /*! @brief register a parameter
    *
    * If a parameter should be managed by the controlcenter, it typicaly is registered right after
    * creation. The control center will take care of unregistering if the parameter is destroyed.
    *
    * After registration, the parameter will appear in the right list view, as a child of a note representing
    * its parent (typicaly the module it belongs to). If that parent note does not exist yet, it will be created.
    */
   void registerParameter(Parameter * newParam);

   /*! @brief register a controller.
    *
    * this will only register the controller if there is no aequivalint
    * controller already registered.
    *
    * This method will return false if it rejects the controller.
    *
    */
   bool registerController(MidiController * newCtrl);

   /*! @brief select a binding
    *
    * if @c binding is registered, it will be highlighted.
    */
   void selectBinding(MidiBinding * binding);

   /*! @brief select a parameter
    *
    * if @c param is a registered Parameter, it will become highlighted.
    */
   void selectParameter(Parameter * param);

   /*! @brief select a controller
    *
    * if @c ctrl is a registered Controller, it will become highlighted.
    */
   void selectController(MidiController * param);

   private slots:

   /*! @brief connected to Parameter::destroyed(QObject *)
    */
   void parameterDestroyed(QObject *);

   /*! @brief connected to MidiBinding::destroyed(QObject *)
    */
   void bindingDestroyed(QObject *);

   /*! @brief connected to MidiController::destroyed(QObject *)
    */
   void controllerDestroyed(QObject *);

   /*! @brief connected to MidiController::bindingAdded(MidiController*,MidiBinding*);
    */
   void bindingAdded(MidiController * ctrl, MidiBinding * binding);

   /*! @brief connected to MidiController::bindingRemoved(MidiController*,MidiBinding*);
    */
   void bindingRemoved(MidiController * ctrl, MidiBinding * binding);

   void noteCheckBoxToggled(bool val);
   void configCheckBoxToggled(bool val);
   void midiCheckBoxToggled(bool val);

   void controllerListViewSelectionChanged( QListViewItem * item);
   void parameterListViewSelectionChanged( QListViewItem * item);

   void clearButtonClicked();
   void bindButtonClicked();
   
   void mapTypeChanged(const QString &name);
   void midiSignChanged(int sign);
   void panelNameChanged(int,const QString &);
   signals:


};

#endif
