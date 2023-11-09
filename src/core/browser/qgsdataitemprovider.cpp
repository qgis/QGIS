/***************************************************************************
  qgsdataitemprovider.cpp
  --------------------------------------
  Date                 : March 2015
  Copyright            : (C) 2015 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
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
