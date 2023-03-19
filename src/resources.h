/*
 * File with gobal resources
 */


#ifndef RESOURCES_H
#define RESOURCES_H

#include <QtGlobal>


/* hande Qt::endl introduced in Qt 5.14*/
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
#define QT_ENDL Qt::endl
#else
#define QT_ENDL endl
#endif


#endif
