#ifndef ENUMPARAMETER_H
#define ENUMPARAMETER_H


#include <float.h>
#include <math.h>
#include <qstring.h>
#include <qobject.h>

#include <qlist.h>
#include <qvaluelist.h>
#include <qmap.h>


#include "parameter.h"


/*! @file enumparameter.h
 *
 *  @brief
 *  Declaration of class EnumParameter
 *  @author Lukas Degener
 */

/*! @class EnumParameter
 *
 * @brief A Parameter representing a enum-like value
 *
 * An instance of this class represents a finit set of items, of which one and only one at time
 * is the 'selected item'.
 *
 * Each item has
 * - a unique key of type <code>int</code>,
 * - a name of type <code>QString</code>,
 * - and a On/Off state, that determines if this item can be selected.
 *
 * The items are ordered by there keys.
 * Key and name of an item can not be changed, as long as it exists. The state of an item can be changed at any time.
 * (if the selected Item is disables, the first enabled item becomes selected. If an EnumParameter object has no
 * enabled Items, its value is undefined.
 *
 * items can be added or removed to the EnumParameter at any time.
 *
 * @note No item can have the key -1 since this value is reserved, to represent <code>undefined</code>
 */
class EnumParameter : public Parameter{

   Q_OBJECT

   private:
   /*! @brief the key of the selected item
    *
    * @see selectItem(int), selectItem(QString), selectedItem()
    */
   int & mCurrentKey;
   int mLocalValue;

   /*! @brief a map containing the item names
    *
    * I don't now if it is wise to use a QMap for this. Some expert advice would be apreciated.
    * @see itemName()
    */
   QMap<int,QString> mItemNames;

   /*! @brief a map containing the item on/off states
    *
    * I don't now if it is wise to use a QMap for this. Some expert advice would be apreciated.
    * @see itemEnabled()
    */
   QMap<int,bool> mItemStates;


   public:
   EnumParameter(QObject * parent=0, QString name="unnamed parameter", QString descr="no description");


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
    EnumParameter(QObject * parent, QString name, QString descr, int *p_ref);


   ~EnumParameter();

   /*! @brief get the selected item
    *
    * @returns the key of the currently selected item, or -1 if there are no enabled items
    */
   int selectedItem();

   /*! @brief valueRef ;-)
    *
    * @returns a const pointer to the mCurrentKey member
    */
   const int * valuePtr();

   operator const int &();
   const int & operator=(const int & orig);

   /*! @brief find the key for an item name
    *
    * @returns the key of the first item with the given name, or -1 if none is found.
    */
   int itemKey(const QString& name);

   /*! @brief get the name for an item
    *
    * @returns the name of an item or an empty string if there is key doe not exist
    */
   QString itemName(const int key);

   /*! @brief get the state of an item
    *
    * @returns true if key exists, and the corresponding item is enabled, false otherwise
    */
   bool itemEnabled(const int key);

   /*! @brief get enabled Items
    *
    * @returns a sorted list of the keys of all enabled items
    *
    * @note any later change to this parameter might invalidate the returned list.
    */
   QValueList<int> enabledItems();



   public slots:

   /*! @brief add an item
    *
    * use this method to add items to the list.
    *
    * @param key the key you whish to associate with the new item. it must be unique and it must not be -1.
    *        If there is already an item associated to key, or if key is -1, this method does nothing.
    *
    * @param name should be something sensible, it might be displayed to the user by some gui component. Item names
    *        don't have to be unique, however this is recomended.
    *
    * @param enabled the initial On/Off state for the new item
    *
    * if the item is succsessfully added, the itemAdded(int) signal is emitted
    */
   void addItem(const int key, const QString& name, bool enabled=true);

   /*! @brief remove an item
    *
    * removes the item associated with key. If key does not exist, nothing happens.
    *
    * if the item is succsessfully removed, the itemRemoved(int,QString) signal is emitted
    */
   void removeItem(const int key);

   /*! @brief remove an item
    *
    * aequivaluent to <code>removeItem( itemKey( name ) );</code>
    */
   void removeItem(const QString& name);

   /*! @brief set the state of an item
    *
    * if an item is associated with key, its On/Off state is set to val and the itemStateChanged(int) signal
    * is emited.
    */
   void setItemEnabled(const int key, bool val);

   /*! @brief select an item
    *
    * if an item is associated with key, and this item is enabled, it becomes the selected item, and selectionChanged(int)
    * is emited
    */
   void selectItem(const int & key);

   /*! @brief select an item
    *
    * aequivaluent to <code>selectItem( itemKey( name ) );</code>
    */
   void selectItem(const QString&);

   signals:

   /*! @brief emited if the value of mCurrentKey changes
    */
   void selectionChanged(int key);

   /*! @brief emited if an item was added
    */
   void itemAdded(int key);

   /*! @brief emited if an item was removed
    */
   void itemRemoved(int key,const QString& name);

   /*! @brief emited if the state of an item changed
    */
   void itemStateChanged(int key);
};

#endif
