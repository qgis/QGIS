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
#include "qgs3d.h"
#include "qgsmaterialregistry.h"
#include "qgs3dexportobject.h"
#include "qgs3dsceneexporter.h"
#include "qgsmarkersymbol.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerelevationproperties.h"

QgsAbstract3DSymbol *QgsPoint3DSymbol::clone() const
{
  return new QgsPoint3DSymbol( *this );
}

QgsAbstract3DSymbol *QgsPoint3DSymbol::create()
{
  return new QgsPoint3DSymbol();
}

QgsPoint3DSymbol::QgsPoint3DSymbol()
  : mMaterialSettings( std::make_unique<QgsPhongMaterialSettings>() )
{
  setBillboardSymbol( static_cast<QgsMarkerSymbol *>( QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ) ) );

  // our built-in 3D geometries (e.g. cylinder, plane, ...) assume Y axis going "up",
  // let's rotate them by default so that their Z axis goes "up" (like the rest of the scene)
  mTransform.rotate( QQuaternion::fromAxisAndAngle( QVector3D( 1, 0, 0 ), 90 ) );
}

QgsPoint3DSymbol::QgsPoint3DSymbol( const QgsPoint3DSymbol &other )
  : mAltClamping( other.altitudeClamping() )
  , mMaterialSettings( other.materialSettings() ? other.materialSettings()->clone() : nullptr )
  , mShape( other.shape() )
  , mShapeProperties( other.shapeProperties() )
  , mTransform( other.transform() )
  , mBillboardSymbol( other.billboardSymbol() ? other.billboardSymbol()->clone() : nullptr )
{
  setDataDefinedProperties( other.dataDefinedProperties() );
}

QgsPoint3DSymbol::~QgsPoint3DSymbol() = default;

void QgsPoint3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  QDomDocument doc = elem.ownerDocument();

  QDomElement elemDataProperties = doc.createElement( QStringLiteral( "data" ) );
  elemDataProperties.setAttribute( QStringLiteral( "alt-clamping" ), Qgs3DUtils::altClampingToString( mAltClamping ) );
  elem.appendChild( elemDataProperties );

  elem.setAttribute( QStringLiteral( "material_type" ), mMaterialSettings->type() );
  QDomElement elemMaterial = doc.createElement( QStringLiteral( "material" ) );
  mMaterialSettings->writeXml( elemMaterial, context );
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

  if ( billboardSymbol() )
  {
    const QDomElement symbolElem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "symbol" ), billboardSymbol(), doc, context );

    elem.appendChild( symbolElem );
  }
}

void QgsPoint3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  const QDomElement elemDataProperties = elem.firstChildElement( QStringLiteral( "data" ) );
  mAltClamping = Qgs3DUtils::altClampingFromString( elemDataProperties.attribute( QStringLiteral( "alt-clamping" ) ) );

  const QDomElement elemMaterial = elem.firstChildElement( QStringLiteral( "material" ) );
  const QString materialType = elem.attribute( QStringLiteral( "material_type" ), QStringLiteral( "phong" ) );
  mMaterialSettings.reset( Qgs3D::materialRegistry()->createMaterialSettings( materialType ) );
  if ( !mMaterialSettings )
    mMaterialSettings.reset( Qgs3D::materialRegistry()->createMaterialSettings( QStringLiteral( "phong" ) ) );
  mMaterialSettings->readXml( elemMaterial, context );

  mShape = shapeFromString( elem.attribute( QStringLiteral( "shape" ) ) );

  const QDomElement elemShapeProperties = elem.firstChildElement( QStringLiteral( "shape-properties" ) );
  mShapeProperties = QgsXmlUtils::readVariant( elemShapeProperties.firstChildElement() ).toMap();
  mShapeProperties[QStringLiteral( "model" )] = QVariant( context.pathResolver().readPath( mShapeProperties[QStringLiteral( "model" )].toString() ) );

  const QDomElement elemTransform = elem.firstChildElement( QStringLiteral( "transform" ) );
  mTransform = Qgs3DUtils::stringToMatrix4x4( elemTransform.attribute( QStringLiteral( "matrix" ) ) );

  const QDomElement symbolElem = elem.firstChildElement( QStringLiteral( "symbol" ) );

  setBillboardSymbol( QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( symbolElem, context ) );
}

QList<Qgis::GeometryType> QgsPoint3DSymbol::compatibleGeometryTypes() const
{
  return QList<Qgis::GeometryType>() << Qgis::GeometryType::Point;
}

void QgsPoint3DSymbol::setDefaultPropertiesFromLayer( const QgsVectorLayer *layer )
{
  const QgsVectorLayerElevationProperties *props = qgis::down_cast<const QgsVectorLayerElevationProperties *>( const_cast<QgsVectorLayer *>( layer )->elevationProperties() );

  mAltClamping = props->clamping();
  mTransform.data()[13] = static_cast<float>( props->zOffset() );
  mShapeProperties[QStringLiteral( "length" )] = props->extrusionEnabled() ? static_cast<float>( props->extrusionHeight() ) : 0.0f;
}

Qgis::Point3DShape QgsPoint3DSymbol::shapeFromString( const QString &shape )
{
  if ( shape == QStringLiteral( "sphere" ) )
    return Qgis::Point3DShape::Sphere;
  else if ( shape == QLatin1String( "cone" ) )
    return Qgis::Point3DShape::Cone;
  else if ( shape == QLatin1String( "cube" ) )
    return Qgis::Point3DShape::Cube;
  else if ( shape == QLatin1String( "torus" ) )
    return Qgis::Point3DShape::Torus;
  else if ( shape == QLatin1String( "plane" ) )
    return Qgis::Point3DShape::Plane;
  else if ( shape == QLatin1String( "extruded-text" ) )
    return Qgis::Point3DShape::ExtrudedText;
  else if ( shape == QLatin1String( "model" ) )
    return Qgis::Point3DShape::Model;
  else if ( shape == QLatin1String( "billboard" ) )
    return Qgis::Point3DShape::Billboard;
  else // "cylinder" (default)
    return Qgis::Point3DShape::Cylinder;
}

QString QgsPoint3DSymbol::shapeToString( Qgis::Point3DShape shape )
{
  switch ( shape )
  {
    case Qgis::Point3DShape::Cylinder:
      return QStringLiteral( "cylinder" );
    case Qgis::Point3DShape::Sphere:
      return QStringLiteral( "sphere" );
    case Qgis::Point3DShape::Cone:
      return QStringLiteral( "cone" );
    case Qgis::Point3DShape::Cube:
      return QStringLiteral( "cube" );
    case Qgis::Point3DShape::Torus:
      return QStringLiteral( "torus" );
    case Qgis::Point3DShape::Plane:
      return QStringLiteral( "plane" );
    case Qgis::Point3DShape::ExtrudedText:
      return QStringLiteral( "extruded-text" );
    case Qgis::Point3DShape::Model:
      return QStringLiteral( "model" );
    case Qgis::Point3DShape::Billboard:
      return QStringLiteral( "billboard" );
    default:
      Q_ASSERT( false );
      return QString();
  }
}

QVariant QgsPoint3DSymbol::shapeProperty( const QString &property ) const
{
  switch ( mShape )
  {
    case Qgis::Point3DShape::Cylinder:
    {
      if ( property == QLatin1String( "length" ) )
      {
        const float length = mShapeProperties.value( property ).toFloat();
        if ( length == 0 )
          return 10;
        return length;
      }
      else if ( property == QLatin1String( "radius" ) )
      {
        const float radius = mShapeProperties.value( property ).toFloat();
        if ( radius == 0 )
          return 10;
        return radius;
      }
      break;
    }
    case Qgis::Point3DShape::Sphere:
    {
      if ( property == QLatin1String( "radius" ) )
      {
        const float radius = mShapeProperties.value( property ).toFloat();
        if ( radius == 0 )
          return 10;
        return radius;
      }
      break;
    }
    case Qgis::Point3DShape::Cone:
    {
      if ( property == QLatin1String( "length" ) )
      {
        const float length = mShapeProperties.value( property ).toFloat();
        if ( length == 0 )
          return 10;
        return length;
      }
      break;
    }
    case Qgis::Point3DShape::Cube:
    {
      if ( property == QLatin1String( "size" ) )
      {
        const float size = mShapeProperties.value( property ).toFloat();
        if ( size == 0 )
          return 10;
        return size;
      }
      break;
    }
    case Qgis::Point3DShape::Torus:
    {
      if ( property == QLatin1String( "radius" ) )
      {
        const float radius = mShapeProperties.value( property ).toFloat();
        if ( radius == 0 )
          return 10;
        return radius;
      }
      else if ( property == QLatin1String( "minorRadius" ) )
      {
        const float minorRadius = mShapeProperties.value( property ).toFloat();
        if ( minorRadius == 0 )
          return 5;
        return minorRadius;
      }
      break;
    }
    case Qgis::Point3DShape::Plane:
    {
      if ( property == QLatin1String( "size" ) )
      {
        const float size = mShapeProperties.value( property ).toFloat();
        if ( size == 0 )
          return 10;
        return size;
      }
      break;
    }
    case Qgis::Point3DShape::ExtrudedText:
    {
      if ( property == QLatin1String( "depth" ) )
      {
        const float depth = mShapeProperties.value( property ).toFloat();
        if ( depth == 0 )
          return 1;
        return depth;
      }
      break;
    }

    case Qgis::Point3DShape::Model:
    case Qgis::Point3DShape::Billboard:
      break;
  }
  return mShapeProperties.value( property );
}

float QgsPoint3DSymbol::billboardHeight() const
{
  return mTransform.data()[14];
}

QgsAbstractMaterialSettings *QgsPoint3DSymbol::materialSettings() const
{
  return mMaterialSettings.get();
}

void QgsPoint3DSymbol::setMaterialSettings( QgsAbstractMaterialSettings *materialSettings )
{
  if ( materialSettings == mMaterialSettings.get() )
    return;

  mMaterialSettings.reset( materialSettings );
}

bool QgsPoint3DSymbol::exportGeometries( Qgs3DSceneExporter *exporter, Qt3DCore::QEntity *entity, const QString &objectNamePrefix ) const
{
  if ( shape() == Qgis::Point3DShape::Model )
  {
    Qt3DRender::QSceneLoader *sceneLoader = entity->findChild<Qt3DRender::QSceneLoader *>();
    if ( sceneLoader )
    {
      const QVector<Qgs3DExportObject *> objects = exporter->processSceneLoaderGeometries( sceneLoader, objectNamePrefix );
      for ( Qgs3DExportObject *obj : objects )
      {
        obj->setSmoothEdges( exporter->smoothEdges() );
        obj->setupMaterial( materialSettings() );
      }
      exporter->mObjects << objects;
    }
    else
    {
      const QList<Qt3DRender::QMesh *> meshes = entity->findChildren<Qt3DRender::QMesh *>();
      for ( Qt3DRender::QMesh *mesh : meshes )
      {
        Qgs3DExportObject *object = exporter->processGeometryRenderer( mesh, objectNamePrefix );
        if ( !object )
          continue;
        object->setSmoothEdges( exporter->smoothEdges() );
        object->setupMaterial( materialSettings() );
        exporter->mObjects << object;
      }
    }
    return true;
  }
  else if ( shape() == Qgis::Point3DShape::Billboard )
  {
    Qgs3DExportObject *obj = exporter->processPoints( entity, objectNamePrefix );
    if ( obj )
    {
      exporter->mObjects << obj;
      return true;
    }
  }
  else
  {
    const QVector<Qgs3DExportObject *> objects = exporter->processInstancedPointGeometry( entity, objectNamePrefix );
    for ( Qgs3DExportObject *obj : objects )
    {
      obj->setupMaterial( materialSettings() );
      exporter->mObjects << obj;
    }
    return true;
  }
  return false;
}

QgsMarkerSymbol *QgsPoint3DSymbol::billboardSymbol() const
{
  return mBillboardSymbol.get();
}

void QgsPoint3DSymbol::setBillboardSymbol( QgsMarkerSymbol *symbol )
{
  mBillboardSymbol.reset( symbol );
}
