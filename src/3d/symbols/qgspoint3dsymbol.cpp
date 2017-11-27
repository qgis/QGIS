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

#include "qgsreadwritecontext.h"
#include "qgsxmlutils.h"

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
  shapePropertiesCopy["model"] = QVariant( context.pathResolver().writePath( shapePropertiesCopy["model"].toString() ) );

  QDomElement elemShapeProperties = doc.createElement( QStringLiteral( "shape-properties" ) );
  elemShapeProperties.appendChild( QgsXmlUtils::writeVariant( shapePropertiesCopy, doc ) );
  elem.appendChild( elemShapeProperties );

  QDomElement elemTransform = doc.createElement( QStringLiteral( "transform" ) );
  elemTransform.setAttribute( QStringLiteral( "matrix" ), Qgs3DUtils::matrix4x4toString( mTransform ) );
  elem.appendChild( elemTransform );
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
  mShapeProperties["model"] = QVariant( context.pathResolver().readPath( mShapeProperties["model"].toString() ) );

  QDomElement elemTransform = elem.firstChildElement( QStringLiteral( "transform" ) );
  mTransform = Qgs3DUtils::stringToMatrix4x4( elemTransform.attribute( QStringLiteral( "matrix" ) ) );
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
    default: Q_ASSERT( false ); return QString();
  }
}
