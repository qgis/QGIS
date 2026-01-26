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

#include "qgs3d.h"
#include "qgs3dexportobject.h"
#include "qgs3dsceneexporter.h"
#include "qgs3dutils.h"
#include "qgsmarkersymbol.h"
#include "qgsmaterialregistry.h"
#include "qgsreadwritecontext.h"
#include "qgssymbollayerutils.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerelevationproperties.h"
#include "qgsxmlutils.h"

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

  QDomElement elemDataProperties = doc.createElement( u"data"_s );
  elemDataProperties.setAttribute( u"alt-clamping"_s, Qgs3DUtils::altClampingToString( mAltClamping ) );
  elem.appendChild( elemDataProperties );

  elem.setAttribute( u"material_type"_s, mMaterialSettings->type() );
  QDomElement elemMaterial = doc.createElement( u"material"_s );
  mMaterialSettings->writeXml( elemMaterial, context );
  elem.appendChild( elemMaterial );

  elem.setAttribute( u"shape"_s, shapeToString( mShape ) );

  QVariantMap shapePropertiesCopy( mShapeProperties );
  shapePropertiesCopy[u"model"_s] = QVariant( context.pathResolver().writePath( shapePropertiesCopy[u"model"_s].toString() ) );

  QDomElement elemShapeProperties = doc.createElement( u"shape-properties"_s );
  elemShapeProperties.appendChild( QgsXmlUtils::writeVariant( shapePropertiesCopy, doc ) );
  elem.appendChild( elemShapeProperties );

  QDomElement elemTransform = doc.createElement( u"transform"_s );
  elemTransform.setAttribute( u"matrix"_s, Qgs3DUtils::matrix4x4toString( mTransform ) );
  elem.appendChild( elemTransform );

  if ( billboardSymbol() )
  {
    const QDomElement symbolElem = QgsSymbolLayerUtils::saveSymbol( u"symbol"_s, billboardSymbol(), doc, context );

    elem.appendChild( symbolElem );
  }
}

void QgsPoint3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  const QDomElement elemDataProperties = elem.firstChildElement( u"data"_s );
  mAltClamping = Qgs3DUtils::altClampingFromString( elemDataProperties.attribute( u"alt-clamping"_s ) );

  const QDomElement elemMaterial = elem.firstChildElement( u"material"_s );
  const QString materialType = elem.attribute( u"material_type"_s, u"phong"_s );
  mMaterialSettings.reset( Qgs3D::materialRegistry()->createMaterialSettings( materialType ) );
  if ( !mMaterialSettings )
    mMaterialSettings.reset( Qgs3D::materialRegistry()->createMaterialSettings( u"phong"_s ) );
  mMaterialSettings->readXml( elemMaterial, context );

  mShape = shapeFromString( elem.attribute( u"shape"_s ) );

  const QDomElement elemShapeProperties = elem.firstChildElement( u"shape-properties"_s );
  mShapeProperties = QgsXmlUtils::readVariant( elemShapeProperties.firstChildElement() ).toMap();
  mShapeProperties[u"model"_s] = QVariant( context.pathResolver().readPath( mShapeProperties[u"model"_s].toString() ) );

  const QDomElement elemTransform = elem.firstChildElement( u"transform"_s );
  mTransform = Qgs3DUtils::stringToMatrix4x4( elemTransform.attribute( u"matrix"_s ) );

  const QDomElement symbolElem = elem.firstChildElement( u"symbol"_s );

  setBillboardSymbol( QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( symbolElem, context ).release() );
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
  mShapeProperties[u"length"_s] = props->extrusionEnabled() ? static_cast<float>( props->extrusionHeight() ) : 0.0f;
}

Qgis::Point3DShape QgsPoint3DSymbol::shapeFromString( const QString &shape )
{
  if ( shape == "sphere"_L1 )
    return Qgis::Point3DShape::Sphere;
  else if ( shape == "cone"_L1 )
    return Qgis::Point3DShape::Cone;
  else if ( shape == "cube"_L1 )
    return Qgis::Point3DShape::Cube;
  else if ( shape == "torus"_L1 )
    return Qgis::Point3DShape::Torus;
  else if ( shape == "plane"_L1 )
    return Qgis::Point3DShape::Plane;
  else if ( shape == "extruded-text"_L1 )
    return Qgis::Point3DShape::ExtrudedText;
  else if ( shape == "model"_L1 )
    return Qgis::Point3DShape::Model;
  else if ( shape == "billboard"_L1 )
    return Qgis::Point3DShape::Billboard;
  else // "cylinder" (default)
    return Qgis::Point3DShape::Cylinder;
}

QString QgsPoint3DSymbol::shapeToString( Qgis::Point3DShape shape )
{
  switch ( shape )
  {
    case Qgis::Point3DShape::Cylinder:
      return u"cylinder"_s;
    case Qgis::Point3DShape::Sphere:
      return u"sphere"_s;
    case Qgis::Point3DShape::Cone:
      return u"cone"_s;
    case Qgis::Point3DShape::Cube:
      return u"cube"_s;
    case Qgis::Point3DShape::Torus:
      return u"torus"_s;
    case Qgis::Point3DShape::Plane:
      return u"plane"_s;
    case Qgis::Point3DShape::ExtrudedText:
      return u"extruded-text"_s;
    case Qgis::Point3DShape::Model:
      return u"model"_s;
    case Qgis::Point3DShape::Billboard:
      return u"billboard"_s;
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
      if ( property == "length"_L1 )
      {
        const float length = mShapeProperties.value( property ).toFloat();
        if ( length == 0 )
          return 10;
        return length;
      }
      else if ( property == "radius"_L1 )
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
      if ( property == "radius"_L1 )
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
      if ( property == "length"_L1 )
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
      if ( property == "size"_L1 )
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
      if ( property == "radius"_L1 )
      {
        const float radius = mShapeProperties.value( property ).toFloat();
        if ( radius == 0 )
          return 10;
        return radius;
      }
      else if ( property == "minorRadius"_L1 )
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
      if ( property == "size"_L1 )
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
      if ( property == "depth"_L1 )
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
