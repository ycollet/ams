#ifndef MIDIBINDING_H
#define MIDIBINDING_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include "midicontroller.h"
#include "parameter.h"

/*! @file midibinding.h
 *
 *  @brief
 *  Declaration of class MidiBinding
 *  @author Lukas Degener
 */

/*! @class MidiBinding
 * @brief Midi controller binding of a Parameter
 *
 * You cannot instantiate this class, it is abstract (whats the term for this in c++? ;-) ). 
 * If you want to create a MidiBinding, create an instance of one
 * of the subclasses
 *
 * An instance of a subclass represents a binding of certain midi controller events
 * to an instance of a Parameter subclass. For each implemented subclasses of Parameter there are is a subclass of
 * MidiBinding.
 *
 * The Parameter given as the first argument to the constructor will be the parent of the binding. A call to
 * connectToController(MidiController *) will make the Binding listen to a particular class of midi events.
 * if the mAutoDelete member is true (the default), the MidiBinding will perform Seppuko if the bound controller is
 * destroyed. I made this the default behaviour, since a MidiBinding without a Controller connected to it does not
 * make much sense anyway ;-)
 *
 * So you can think of this Object as a kind of mapping from midi values to parameter values.
 * An arbitrary number of such mappings can be applied to a single kind of midi event represented by a MidiController
 * instance, and there might be an arbitrary number of mappings on one single Parameter object.
 *
 * The way midi values (0, ... ,127) are mapped into parameter space (depends on the kind of Parameter) is set by calling
 * setMapType(CtrlMapType). Have a look at the CtrlMapType enum, to get an impression what kinds of mapping are
 * possible.
 *
 *
 * @note not all MidiBinding subclasses support all types of mapping. call mapTypes() or mapTypeNames() to find out
 *       which mapTypes are supported.
 *
 * @note i have yet to find a way to let the user change the map type in an intuitiv way. i just don't know where to
 *       put the relevant gui components right now.
 *
 *
 */
class MidiBinding : public QObject  {

  Q_OBJECT
   public:
  /*! @brief  enum of all types of Controller-mappings
   *
   * if setMidiValue(int) is called on some subclass, the member mMapType will be checked to determine
   * how to set the connected Parameters value. To consider what exactly has to be done is a job that is left
   * to the subclass. The values of this enum can be seen as a hint on what kind of mapping is desired
   */
   enum CtrlMapType {
      None=0,            /*!< undefined, aka nothing, nihil, nada, nüscht, n.a., etc.*/
      LinearMap=1,       /*!< The controller value is maped linear upon the
                          *  range in which the parameter is defined.
                          *  E.g. This is the typical default mapping for FloatMidiBinding and
                          *  IntMidiBinding.
                          *  This map type has a directional effect on the parameter, meaning
                          *  that if mSign is negativ,
                          *  the change in the parameter will be complementary to that of the controller.
                          */
      LogarithmicMap=2,  /*!< As above, but the mapping is logarithmic.
                          */
      ModuloMap=4,       /*!< If the connected Parameter that has a discret set n of possible
                          *  values, the mapping of the controller value is done so that
                          *  the controller value is aequivalent to the resulting parameter value
                          *  in modulo n. The behaviour of a "continous" Binding (like FloatMidiBinding)
                          *  would be undefined.
                          *  This type of mapping can e.g. be used to 'directly' manipulate a EnumParameter with
                          *  programm change controller events.
                          *  This map type has a directional effect on the parameter, meaning that if mSign is negativ,
                          *  the change in the parameter will be complementary to that of the controller.
                          */
      OnToggleMap=8,     /*!< A change to the Parameter will only accure if the midi value is equal
                          *  or above the the value of mOnThreshold. This is usefull if you have
                          *  those 'momentary' controller buttons (like on my Kurzweil PC88) or a
                          *  sustain pedal connected that causes the events, and don't want to get any
                          *  change if you release the button/pedal. As the name suggest, there
                          *  should be some 'toggle' effect to the parameter's value.
                          *  E.g. FloatMidiBinding and
                          *  IntMidiBinding will toggle between the parameters min and max values.
                          */
      OnOffToggleMap=16, /*!< Same as OnToggleMap, but the 'release' is also regarded.(midi values below mOffValue)
                          *  The behaviour of this map type typicaly is directional, meaning that
                          *  if mSign is negative, the effect on the parameter is vice versa. Yeah, i
                          *  know, in many cases the same effect could be achieved with linear
                          *  or Logarithmic map. Just let me have it. Maybe it will be usefull in
                          *  some futur binding types.
                          */
      OnCycleMap=32,     /*!< As the name suggests, this is ment to 'cycle' through the parameter's
                          *  set of defined values. Therefor i would make no sense with "continous"
                          *  Bindings. If the midi value is greater or equal than mOnThreshold,
                          *  the parameter is incremented (or decremented if mSign < 0) in a way that
                          *  makes sense to that kind of parameter. E.g. you could use two momentary
                          *  buttons/foot switches, one with positiv mSign, the other with negativ mSign
                          *  to cycle up and down through the items of an EnumParameter.
                          */
      OnOffCycleMap=64   /*!< As above, but also midi values below mOffValue will in addition decrement
                          *  the parameter (or increment if mSign < 0 ). I havn't yet found a situation
                          *  where this makes sense, but included this type for the sake of completness.
                          */
   };


    /*! @brief the Threshold above which a midi value is considered ON
     */
    int mOnThreshold;

    /*! @brief the Threshold below which a midi value is considered OFF
     */
    int mOffThreshold;

    /*! @brief the bindings 'sign'
     *
     * if mType is directional, this value determines the direction.
     *
     * - > 0 the changes on the parameter will 'in the same direction' as those in the midi value
     * - < 0 the changes on the parameter will 'in the oposing direction' as those in the midi value
     */
    int mSign;

    /*! @brief internal flag to avoid value jumps after loading of a patch
     *
     * Ok, i understand this now a little better, after Mathias explained the problem:
     *
     * Imagine you load a patch with some midi bindings to certain parameters. Now, there will propably be
     * some values to that parameters that were saved together with this patch. But now, that you load
     * the patch, the sliders, or whatever creates the corresponding midi events will propably be at
     * some arbitrary position. so if you move that dial/slider/whatever, there will be an ugly jump
     * in the parameter values.
     *
     * A similar problem would arise if the param group / snapshot feature is implemented (which will happen soon)
     *
     * Or even simpler, you edit the connected parameter with a gui editor :-)
     *
     * In the past, the workaround for this prob has been to ignore midi values until they come 'close' to
     * a value that would be mapped on the parameter value that was saved with the patch. That's what this
     * flag was for.
     *
     * Now the big question:
     * - Do we realy need this?
     * - If yes, for which binding / map types ?
     * - Since this would involve getting the 'current' midi value: What semantics should getMidiValue have?
     *
     * right now, this prob is simply ignored. controllerOK is always true.
     *
     * @warning Don't lean heavily on this member, as it might disapear or be renamed. or its semantics might change.
     *
     * @todo think about/discuss this problem. Find a solution. Fix it already!
     *
     * @remark Think i have an idea how to solve this:
     * The key problem is, that we can not inverse the mapping in all cases. But what we could do, is
     * 'simulate' the effect of the mapping for a given midi value. Then this value could be compared with the
     * current parameter value and if desired, 'jumps' could be prohibited. The controllerOK flag would be replaced by
     * something like <code>bool mIgnoreJumps</code>. If this flag was true, the 'simulation' and the test mentioned above
     * would be run before any change to the parameter
     *
     * @remark Another, quick&dirty workaround would be to save the last recieved midi value with this binding, but
     * i don't like this very much.
     */
    bool controllerOK;

    bool mAutoDelete;
  protected:
   /*! @brief the type of mapping used by this MidiBinding
    *
    * @see CtrlMapType, setMapType(CtrlMapType), getMapType()
    */
   CtrlMapType mMapType;

  /*! @brief the MidiController this MidiBinding listens to.
   */
  MidiController * mController;


  protected:
    MidiBinding(Parameter * param, MidiController * ctrl,
                bool autoDelete = true, const char * name = 0);
  public:
    ~MidiBinding();

   /*! @brief get the controller
    */
   MidiController * controller();

    /*! @brief the Mapping currently used
     *
     * @returns the type of mapping this binding currently uses to modify the parameter
     * @see mMapType, CtrlMapType, setMapType(CtrlMapType), mapTypes(), mapTypeNames(), defaultMapType()
     */
    CtrlMapType mapType();
    QString mapTypeName(CtrlMapType maptype);
    /*! @brief valid map types
     *
     * @returns a list of CtrlMapType values that this Binding subclass supports
     *            the values are ORed together into an int
     * @see mMapType, CtrlMapType, setMapType(CtrlMapType), mapTypeNames(), defaultMapType()
     */
    virtual int mapTypes()=0;

    virtual CtrlMapType defaultMapType();

    /*! @brief names of valid map types
     *
     * @returns a list of QStrings, that represent the mapping types this Binding supports
     * @see mMapType, CtrlMapType, setMapType(CtrlMapType), mapTypes(), mapType(), defaultMapType()
     */
    QStringList mapTypeNames();

    /*! @brief get bindings 'sign'
     *
     * @see mSign
     */
    int midiSign();
  private slots:
    /*! @brief connected to MidiController::destroyed()
     *
     * if the mAutoDelete is true, this will let this MidiBinding find to a quick and painless end >:-)
     *
     * Actualy i use QTs defferedDelete() method.
     */
  void controllerDestroyed();

  public slots:
    /*! @brief set the controller
     */
    void setController(MidiController * ctrl);


    /*! @brief map a value into parameter space
     *
     * @param  value typicaly the value of a recieved controller event. It should be between 0 and 127, otherwise
     *               strange things could happen. :-)
     */
    virtual void setMidiValue(int value)=0;

    /*! @brief set the map type
     *
     * @see mMapType, CtrlMapType, mapType(), mapTypes(), mapTypeNames(), defaultMapType()
     */
    void setMapType(CtrlMapType maptype);

    /*! @brief set the map type
     *
     * @param maptypename should be a string from the list of valid map type names returned by mapTypeNames()
     * @see mMapType, CtrlMapType, mapType(), mapTypes(), mapTypeNames(), defaultMapType()
     */
    void setMapType(const QString & maptypename);

    /*! @brief set the bindings 'sign'
     *
     * @see mSign
     */
    void setMidiSign(int sign);
    void setMidiSign(bool negative);
    /*! @brief toggle the bindings 'sign'
     *
     * @see mSign
     */
    void toggleMidiSign();

  signals:

    /*! @brief called when mSign was changed */
    void midiSignChanged(int);

    /*! @brief called when the map type was changed */
    void mapTypeChanged(CtrlMapType);
    void mapTypeChanged(const QString&);
};

#endif

