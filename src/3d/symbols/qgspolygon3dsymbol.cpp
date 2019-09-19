/***************************************************************************
  qgspolygon3dsymbol.cpp
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

#include "qgspolygon3dsymbol.h"

#include "qgs3dutils.h"
#include "qgssymbollayerutils.h"

QgsAbstract3DSymbol *QgsPolygon3DSymbol::clone() const
{
  return new QgsPolygon3DSymbol( *this );
}

void QgsPolygon3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )

  QDomDocument doc = elem.ownerDocument();

  QDomElement elemDataProperties = doc.createElement( QStringLiteral( "data" ) );
  elemDataProperties.setAttribute( QStringLiteral( "alt-clamping" ), Qgs3DUtils::altClampingToString( mAltClamping ) );
  elemDataProperties.setAttribute( QStringLiteral( "alt-binding" ), Qgs3DUtils::altBindingToString( mAltBinding ) );
  elemDataProperties.setAttribute( QStringLiteral( "height" ), mHeight );
  elemDataProperties.setAttribute( QStringLiteral( "extrusion-height" ), mExtrusionHeight );
  elemDataProperties.setAttribute( QStringLiteral( "culling-mode" ), Qgs3DUtils::cullingModeToString( mCullingMode ) );
  elemDataProperties.setAttribute( QStringLiteral( "invert-normals" ), mInvertNormals ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elemDataProperties.setAttribute( QStringLiteral( "add-back-faces" ), mAddBackFaces ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elem.appendChild( elemDataProperties );

  QDomElement elemMaterial = doc.createElement( QStringLiteral( "material" ) );
  mMaterial.writeXml( elemMaterial );
  elem.appendChild( elemMaterial );

  QDomElement elemDDP = doc.createElement( QStringLiteral( "data-defined-properties" ) );
  mDataDefinedProperties.writeXml( elemDDP, propertyDefinitions() );
  elem.appendChild( elemDDP );

  QDomElement elemEdges = doc.createElement( QStringLiteral( "edges" ) );
  elemEdges.setAttribute( QStringLiteral( "enabled" ), mEdgesEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elemEdges.setAttribute( QStringLiteral( "width" ), mEdgeWidth );
  elemEdges.setAttribute( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( mEdgeColor ) );
  elem.appendChild( elemEdges );
}

void QgsPolygon3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  QDomElement elemDataProperties = elem.firstChildElement( QStringLiteral( "data" ) );
  mAltClamping = Qgs3DUtils::altClampingFromString( elemDataProperties.attribute( QStringLiteral( "alt-clamping" ) ) );
  mAltBinding = Qgs3DUtils::altBindingFromString( elemDataProperties.attribute( QStringLiteral( "alt-binding" ) ) );
  mHeight = elemDataProperties.attribute( QStringLiteral( "height" ) ).toFloat();
  mExtrusionHeight = elemDataProperties.attribute( QStringLiteral( "extrusion-height" ) ).toFloat();
  mCullingMode = Qgs3DUtils::cullingModeFromString( elemDataProperties.attribute( QStringLiteral( "culling-mode" ) ) );
  mInvertNormals = elemDataProperties.attribute( QStringLiteral( "invert-normals" ) ).toInt();
  mAddBackFaces = elemDataProperties.attribute( QStringLiteral( "add-back-faces" ) ).toInt();

  QDomElement elemMaterial = elem.firstChildElement( QStringLiteral( "material" ) );
  mMaterial.readXml( elemMaterial );

  QDomElement elemDDP = elem.firstChildElement( QStringLiteral( "data-defined-properties" ) );
  if ( !elemDDP.isNull() )
    mDataDefinedProperties.readXml( elemDDP, propertyDefinitions() );

  QDomElement elemEdges = elem.firstChildElement( QStringLiteral( "edges" ) );
  if ( !elemEdges.isNull() )
  {
    mEdgesEnabled = elemEdges.attribute( QStringLiteral( "enabled" ) ).toInt();
    mEdgeWidth = elemEdges.attribute( QStringLiteral( "width" ) ).toFloat();
    mEdgeColor = QgsSymbolLayerUtils::decodeColor( elemEdges.attribute( QStringLiteral( "color" ) ) );
  }
}
