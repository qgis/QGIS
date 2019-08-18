/***************************************************************************
  qgspoint3dsymbol.cpp
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

#include "qgspoint3dsymbol.h"

#include "qgs3dutils.h"
#include "qgsreadwritecontext.h"
#include "qgsxmlutils.h"
#include "qgssymbollayerutils.h"

QgsAbstract3DSymbol *QgsPoint3DSymbol::clone() const
{
  return new QgsPoint3DSymbol( *this );
}

void QgsPoint3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  QDomDocument doc = elem.ownerDocument();

  QDomElement elemDataProperties = doc.createElement( QStringLiteral( "data" ) );
  elemDataProperties.setAttribute( QStringLiteral( "alt-clamping" ), Qgs3DUtils::altClampingToString( mAltClamping ) );
  elem.appendChild( elemDataProperties );

  QDomElement elemMaterial = doc.createElement( QStringLiteral( "material" ) );
  mMaterial.writeXml( elemMaterial );
  elem.appendChild( elemMaterial );

  elem.setAttribute( QStringLiteral( "shape" ), shapeToString( mShape ) );

  QVariantMap shapePropertiesCopy( mShapeProperties );
  shapePropertiesCopy[QStringLiteral( "model" )] = QVariant( context.pathResolver().writePath( shapePropertiesCopy[QStringLiteral( "model" )].toString() ) );

  QDomElement elemShapeProperties = doc.createElement( QStringLiteral( "shape-properties" ) );
  elemShapeProperties.appendChild( QgsXmlUtils::writeVariant( shapePropertiesCopy, doc ) );
  elem.appendChild( elemShapeProperties );

  QDomElement elemTransform = doc.createElement( QStringLiteral( "transform" ) );
  elemTransform.setAttribute( QStringLiteral( "matrix" ), Qgs3DUtils::matrix4x4toString( mTransform ) );
  elem.appendChild( elemTransform );

  elem.setAttribute( QStringLiteral( "billboard-symbol" ), QgsSymbolLayerUtils::symbolProperties( billboardSymbol() ) );
}

void QgsPoint3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  QDomElement elemDataProperties = elem.firstChildElement( QStringLiteral( "data" ) );
  mAltClamping = Qgs3DUtils::altClampingFromString( elemDataProperties.attribute( QStringLiteral( "alt-clamping" ) ) );

  QDomElement elemMaterial = elem.firstChildElement( QStringLiteral( "material" ) );
  mMaterial.readXml( elemMaterial );

  mShape = shapeFromString( elem.attribute( QStringLiteral( "shape" ) ) );

  QDomElement elemShapeProperties = elem.firstChildElement( QStringLiteral( "shape-properties" ) );
  mShapeProperties = QgsXmlUtils::readVariant( elemShapeProperties.firstChildElement() ).toMap();
  mShapeProperties[QStringLiteral( "model" )] = QVariant( context.pathResolver().readPath( mShapeProperties[QStringLiteral( "model" )].toString() ) );

  QDomElement elemTransform = elem.firstChildElement( QStringLiteral( "transform" ) );
  mTransform = Qgs3DUtils::stringToMatrix4x4( elemTransform.attribute( QStringLiteral( "matrix" ) ) );

  QDomDocument doc( QStringLiteral( "symbol" ) );
  doc.setContent( elem.attribute( QStringLiteral( "billboard-symbol" ) ) );
  QDomElement symbolElem = doc.firstChildElement( QStringLiteral( "symbol" ) );

  mBillboardSymbol = QgsSymbolLayerUtils::loadSymbol< QgsMarkerSymbol >( symbolElem, QgsReadWriteContext() );
}

QgsPoint3DSymbol::Shape QgsPoint3DSymbol::shapeFromString( const QString &shape )
{
  if ( shape ==  QStringLiteral( "sphere" ) )
    return Sphere;
  else if ( shape == QStringLiteral( "cone" ) )
    return Cone;
  else if ( shape == QStringLiteral( "cube" ) )
    return Cube;
  else if ( shape == QStringLiteral( "torus" ) )
    return Torus;
  else if ( shape == QStringLiteral( "plane" ) )
    return Plane;
  else if ( shape == QStringLiteral( "extruded-text" ) )
    return ExtrudedText;
  else if ( shape == QStringLiteral( "model" ) )
    return Model;
  else if ( shape == QStringLiteral( "billboard" ) )
    return Billboard;
  else   // "cylinder" (default)
    return Cylinder;
}

QString QgsPoint3DSymbol::shapeToString( QgsPoint3DSymbol::Shape shape )
{
  switch ( shape )
  {
    case Cylinder: return QStringLiteral( "cylinder" );
    case Sphere: return QStringLiteral( "sphere" );
    case Cone: return QStringLiteral( "cone" );
    case Cube: return QStringLiteral( "cube" );
    case Torus: return QStringLiteral( "torus" );
    case Plane: return QStringLiteral( "plane" );
    case ExtrudedText: return QStringLiteral( "extruded-text" );
    case Model: return QStringLiteral( "model" );
    case Billboard: return QStringLiteral( "billboard" );
    default: Q_ASSERT( false ); return QString();
  }
}

void QgsPoint3DSymbol::setBillboardTransform( float height )
{
  QQuaternion rot( QQuaternion::fromEulerAngles( 0, 0, 0 ) );
  QVector3D sca( 1, 1, 1 );
  QVector3D tra( 0, height, 0 );

  QMatrix4x4 tr;
  tr.translate( tra );
  tr.scale( sca );
  tr.rotate( rot );

  mBillboardTransform = tr;
}
