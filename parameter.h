#ifndef PARAMETER_H
#define PARAMETER_H

#include <float.h>


#include <qstring.h>
#include <qobject.h>
#include <qlist.h>
#include <qintdict.h>
#include <qdict.h>
#include <qmap.h>

/*! @file parameter.h
 *
 *  @brief
 *  Declaration of class Parameter
 *  @author Lukas Degener
 */


/*! @class Parameter
 *
 * @brief Superclass for all classes representing parameters
 *
 * Instances of subclasses to Parameter represent a parameter of certain type, together with all meta information
 * necessary to display, edit, or make otherwise use of that particular parameter. A Parameter is logicaly unique,
 * meaning that it would make no sens (and might be rather difficult) to have more than one Parameter instances
 * representing the same logical parameter. There might however be several objects that display or edit the same
 * Parameter instance.Have a look at the several subclasses and there corresponding parameter editors and midi controllers
 *
 * Parameter is a general purpose class, therefor it does not contain any midi or gui aspects.The same should be true for
 * any subclasses
 *
 * Within ams Parameters are typicaly used to represent aspects of a Module, which can be edited in real time, eg. via
 * corresponding ParameterEditor objects, or via Midi, by creating a MidiController instance for a Parameter instance.
 * Have a look at the ParameterEditorFactory and MidiControllerFactory classes
 *
 * There might however be other uses of the Parameter class within ams
 *
 * @note Creating an instance of this class is impossible, Better find a subclass that fits your needs.
 *
 * @note the parameter should be given a meaningfull name, since Gui components might use QObject::name() for labels etc.
 *
 * @note in ams, the parent object of a Parameter instance would typicaly be the Module it belongs to.
 */
class Parameter: public QObject{

   Q_OBJECT



   private:
   static QDict<Parameter> mInstances;
   static QMap<Parameter*,QString> mNames;
   static void registerInstance(Parameter * par);
   static void unregisterInstance(Parameter * par);


   public:
   static Parameter * instance(const QString & longname);


   /*! @brief The Parameter's description
    *
    * a string describing the parameter.
    * usefull e.g. for tooltips etc.
    */
   QString mDescription;


   protected:

   Parameter(QObject * parent=0, QString name="unnamed parameter", QString descr="no description");

   public:
   ~Parameter();

   /*! @brief parameter's long name
    *
    * returns a String of the form "<parent name>:<param name>"
    * this combination should be unique for each parameter, it is used to
    * identifie a parameter.
    */
   virtual QString longName();

   /*! get a description of this parameter
    *
    * @returns a String describing the parameter
    * @see setDescription(QString), mDescription
    */
   virtual QString description();

   /*! @brief change the parameters description
    *
    * sets the parameter's description to descr
    *
    * @todo right now, there is now signal emited. maybe this should be added.
    */
   virtual void setDescription(QString descr);
   public slots:
   void touchedByGui();
   signals:


};

#endif
