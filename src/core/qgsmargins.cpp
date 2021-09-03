/***************************************************************************
  qgsmargins.cpp
  --------------
  Date                 : January 2017
  Copyright            : (C) 2017 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmargins.h"

QString QgsMargins::toString() const
{
  if ( isNull() )
    return QString();
  else
    return QStringLiteral( "%1,%2,%3,%4" ).arg( qgsDoubleToString( mLeft ), qgsDoubleToString( mTop ),
           qgsDoubleToString( mRight ), qgsDoubleToString( mBottom ) );
}

QgsMargins QgsMargins::fromString( const QString &string )
{
  const QStringList margins = string.split( ',' );
  if ( margins.count() != 4 )
    return QgsMargins();

  return QgsMargins( margins.at( 0 ).toDouble(),
                     margins.at( 1 ).toDouble(),
                     margins.at( 2 ).toDouble(),
                     margins.at( 3 ).toDouble() );
}
