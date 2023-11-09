/***************************************************************************
    qgswfsutils.cpp
    ---------------------
    begin                : March 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswfsutils.h"

#include <QStringList>

QString QgsWFSUtils::removeNamespacePrefix( const QString &tname )
{
  QString name( tname );
  if ( name.contains( ':' ) )
  {
    const QStringList splitList = name.split( ':' );
    if ( splitList.size() > 1 )
    {
      name = splitList.at( 1 );
    }
  }
  return name;
}

QString QgsWFSUtils::nameSpacePrefix( const QString &tname )
{
  const QStringList splitList = tname.split( ':' );
  if ( splitList.size() < 2 )
  {
    return QString();
  }
  return splitList.at( 0 );
}

