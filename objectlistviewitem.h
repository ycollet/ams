#ifndef OBJECTLISTVIEWITEM_H
#define OBJECTLISTVIEWITEM_H

#include <qobject.h>
#include <qlistview.h>


class ObjectListViewItem : public QListViewItem{

   public:
   QObject * object;
   ObjectListViewItem (QListView * parent, QObject * o = 0);
   ObjectListViewItem (QListViewItem * parent, QObject * o = 0);
};

#endif
