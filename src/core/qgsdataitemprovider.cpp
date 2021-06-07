/***************************************************************************
  qgsdataitemprovider.cpp
  --------------------------------------
  Date                 : March 2015
  Copyright            : (C) 2015 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsdataitemprovider.h"

// no implementation currently

QVector<QgsDataItem *> QgsDataItemProvider::createDataItems( const QString &path, QgsDataItem *parentItem )
{
  Q_UNUSED( path )
  Q_UNUSED( parentItem )
  return QVector<QgsDataItem *>();
}

bool QgsDataItemProvider::handlesDirectoryPath( const QString & )
{
  return false;
}
