/***************************************************************************
  qgsmesh3dsymbol.cpp
  -------------------
  Date                 : January 2019
  Copyright            : (C) 2019 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmesh3dsymbol.h"

#include "qgs3dutils.h"

QgsAbstract3DSymbol *QgsMesh3DSymbol::clone() const
{
  return new QgsMesh3DSymbol( *this );
}

void QgsMesh3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )

  QDomDocument doc = elem.ownerDocument();

  QDomElement elemDataProperties = doc.createElement( QStringLiteral( "data" ) );
  elemDataProperties.setAttribute( QStringLiteral( "alt-clamping" ), Qgs3DUtils::altClampingToString( mAltClamping ) );
  elemDataProperties.setAttribute( QStringLiteral( "height" ), mHeight );
  elemDataProperties.setAttribute( QStringLiteral( "add-back-faces" ), mAddBackFaces ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elem.appendChild( elemDataProperties );

  QDomElement elemMaterial = doc.createElement( QStringLiteral( "material" ) );
  mMaterial.writeXml( elemMaterial );
  elem.appendChild( elemMaterial );

  QDomElement elemDDP = doc.createElement( QStringLiteral( "data-defined-properties" ) );
  mDataDefinedProperties.writeXml( elemDDP, propertyDefinitions() );
  elem.appendChild( elemDDP );
}

void QgsMesh3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  QDomElement elemDataProperties = elem.firstChildElement( QStringLiteral( "data" ) );
  mAltClamping = Qgs3DUtils::altClampingFromString( elemDataProperties.attribute( QStringLiteral( "alt-clamping" ) ) );
  mHeight = elemDataProperties.attribute( QStringLiteral( "height" ) ).toFloat();
  mAddBackFaces = elemDataProperties.attribute( QStringLiteral( "add-back-faces" ) ).toInt();

  QDomElement elemMaterial = elem.firstChildElement( QStringLiteral( "material" ) );
  mMaterial.readXml( elemMaterial );

  QDomElement elemDDP = elem.firstChildElement( QStringLiteral( "data-defined-properties" ) );
  if ( !elemDDP.isNull() )
    mDataDefinedProperties.readXml( elemDDP, propertyDefinitions() );
}
