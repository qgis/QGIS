/***************************************************************************
                          qgsgeometryoptions.cpp
                             -------------------
    begin                : Aug 23, 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometryoptions.h"

#include "qgsxmlutils.h"

bool QgsGeometryOptions::removeDuplicateNodes() const
{
  return mRemoveDuplicateNodes;
}

void QgsGeometryOptions::setRemoveDuplicateNodes( bool value )
{
  mRemoveDuplicateNodes = value;
}

double QgsGeometryOptions::geometryPrecision() const
{
  return mGeometryPrecision;
}

void QgsGeometryOptions::setGeometryPrecision( double value )
{
  mGeometryPrecision = value;
}

bool QgsGeometryOptions::isActive() const
{
  return mGeometryPrecision != 0.0 || mRemoveDuplicateNodes;
}

void QgsGeometryOptions::apply( QgsGeometry &geometry ) const
{
  if ( mGeometryPrecision != 0.0 )
    geometry = geometry.snappedToGrid( mGeometryPrecision, mGeometryPrecision );

  if ( mRemoveDuplicateNodes )
    geometry.removeDuplicateNodes();
}

QStringList QgsGeometryOptions::geometryChecks() const
{
  return mGeometryChecks;
}

void QgsGeometryOptions::setGeometryChecks( const QStringList &geometryChecks )
{
  mGeometryChecks = geometryChecks;
}

QVariantMap QgsGeometryOptions::checkConfiguration( const QString &checkId ) const
{
  return mCheckConfiguration.value( checkId ).toMap();
}

void QgsGeometryOptions::setCheckConfiguration( const QString &checkId, const QVariantMap &checkConfiguration )
{
  mCheckConfiguration[checkId] = checkConfiguration;
}

void QgsGeometryOptions::writeXml( QDomNode &node ) const
{
  QDomElement geometryOptionsElement = node.ownerDocument().createElement( QStringLiteral( "geometryOptions" ) );
  node.appendChild( geometryOptionsElement );

  geometryOptionsElement.setAttribute( QStringLiteral( "removeDuplicateNodes" ), mRemoveDuplicateNodes ? 1 : 0 );
  geometryOptionsElement.setAttribute( QStringLiteral( "geometryPrecision" ), mGeometryPrecision );
}

void QgsGeometryOptions::readXml( const QDomNode &node )
{
  QDomElement geometryOptionsElement = node.toElement();
  setGeometryPrecision( geometryOptionsElement.attribute( QStringLiteral( "geometryPrecision" ),  QStringLiteral( "0.0" ) ).toDouble() );
  setRemoveDuplicateNodes( geometryOptionsElement.attribute( QStringLiteral( "removeDuplicateNodes" ),  QStringLiteral( "0" ) ).toInt() == 1 );
}
