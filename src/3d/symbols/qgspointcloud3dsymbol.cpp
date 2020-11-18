/***************************************************************************
  qgspointcloud3dsymbol.h
  ------------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Nedjima Belgacem
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloud3dsymbol.h"


QgsPointCloud3DSymbol::QgsPointCloud3DSymbol()
  : QgsAbstract3DSymbol()
{

}

QgsPointCloud3DSymbol::~QgsPointCloud3DSymbol() {  }

QgsAbstract3DSymbol *QgsPointCloud3DSymbol::clone() const
{
  QgsPointCloud3DSymbol *result = new QgsPointCloud3DSymbol;
  result->mEnabled = mEnabled;
  result->mPointSize = mPointSize;
  copyBaseSettings( result );
  return result;
}

void QgsPointCloud3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )

  elem.setAttribute( QStringLiteral( "enabled" ), mEnabled );
  elem.setAttribute( QStringLiteral( "point-size" ), mPointSize );
}

void QgsPointCloud3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  mEnabled = elem.attribute( "enabled", QStringLiteral( "0" ) ).toInt();
  mPointSize = elem.attribute( "point-size", QStringLiteral( "5.0" ) ).toFloat();
}

void QgsPointCloud3DSymbol::setIsEnabled( bool enabled )
{
  mEnabled = enabled;
}

void QgsPointCloud3DSymbol::setPointSize( float size )
{
  mPointSize = size;
}
