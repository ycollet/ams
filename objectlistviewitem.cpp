#include <qobject.h>
#include <qlistview.h>


#include "objectlistviewitem.h"

ObjectListViewItem::ObjectListViewItem (QListView * parent, QObject * o = 0)
:QListViewItem(parent){
   object=o;
}
ObjectListViewItem::ObjectListViewItem (QListViewItem * parent, QObject * o = 0)
:QListViewItem(parent){
   object=o;
}

