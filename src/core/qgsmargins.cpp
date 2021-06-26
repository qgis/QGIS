/***************************************************************************
  qgsmargins.cpp
  --------------
  Date                 : January 2017
  Copyright            : (C) 2020 by Wang Peng
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
  QStringList margins = string.split( ',' );
  if ( margins.count() != 4 )
    return QgsMargins();

  return QgsMargins( margins.at( 0 ).toDouble(),
                     margins.at( 1 ).toDouble(),
                     margins.at( 2 ).toDouble(),
                     margins.at( 3 ).toDouble() );
}
