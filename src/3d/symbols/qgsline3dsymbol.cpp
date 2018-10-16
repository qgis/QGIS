/***************************************************************************
  qgsline3dsymbol.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsline3dsymbol.h"

#include "qgs3dutils.h"

QgsAbstract3DSymbol *QgsLine3DSymbol::clone() const
{
  return new QgsLine3DSymbol( *this );
}

void QgsLine3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context );

  QDomDocument doc = elem.ownerDocument();

  QDomElement elemDataProperties = doc.createElement( QStringLiteral( "data" ) );
  elemDataProperties.setAttribute( QStringLiteral( "alt-clamping" ), Qgs3DUtils::altClampingToString( mAltClamping ) );
  elemDataProperties.setAttribute( QStringLiteral( "alt-binding" ), Qgs3DUtils::altBindingToString( mAltBinding ) );
  elemDataProperties.setAttribute( QStringLiteral( "height" ), mHeight );
  elemDataProperties.setAttribute( QStringLiteral( "extrusion-height" ), mExtrusionHeight );
  elemDataProperties.setAttribute( QStringLiteral( "simple-lines" ), mRenderAsSimpleLines ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elemDataProperties.setAttribute( QStringLiteral( "width" ), mWidth );
  elem.appendChild( elemDataProperties );

  QDomElement elemMaterial = doc.createElement( QStringLiteral( "material" ) );
  mMaterial.writeXml( elemMaterial );
  elem.appendChild( elemMaterial );
}

void QgsLine3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  QDomElement elemDataProperties = elem.firstChildElement( QStringLiteral( "data" ) );
  mAltClamping = Qgs3DUtils::altClampingFromString( elemDataProperties.attribute( QStringLiteral( "alt-clamping" ) ) );
  mAltBinding = Qgs3DUtils::altBindingFromString( elemDataProperties.attribute( QStringLiteral( "alt-binding" ) ) );
  mHeight = elemDataProperties.attribute( QStringLiteral( "height" ) ).toFloat();
  mExtrusionHeight = elemDataProperties.attribute( QStringLiteral( "extrusion-height" ) ).toFloat();
  mWidth = elemDataProperties.attribute( QStringLiteral( "width" ) ).toFloat();
  mRenderAsSimpleLines = elemDataProperties.attribute( QStringLiteral( "simple-lines" ), QStringLiteral( "0" ) ).toInt();

  QDomElement elemMaterial = elem.firstChildElement( QStringLiteral( "material" ) );
  mMaterial.readXml( elemMaterial );
}
