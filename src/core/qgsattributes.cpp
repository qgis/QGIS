/***************************************************************************
  qgsattributes.cpp - QgsAttributes

 ---------------------
 begin                : 29.3.2017
 copyright            : (C) 2017 by Denis Rouzaud
 email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributes.h"
#include "qgis.h"


QgsAttributeMap QgsAttributes::toMap() const
{
  QgsAttributeMap map;
  for ( int idx = 0; idx < count(); ++idx )
  {
    const QVariant v = at( idx );
    if ( v.isValid() )
      map.insert( idx, v );
  }
  return map;
}

uint qHash( const QgsAttributes &attributes )
{
  if ( attributes.isEmpty() )
    return std::numeric_limits<uint>::max();
  else
    return qHash( attributes.at( 0 ) );
}
