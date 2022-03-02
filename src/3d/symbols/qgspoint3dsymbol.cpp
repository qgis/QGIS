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
  : mMaterial( std::make_unique< QgsPhongMaterialSettings >() )
{
  setBillboardSymbol( static_cast<QgsMarkerSymbol *>( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) ) );
}

QgsPoint3DSymbol::QgsPoint3DSymbol( const QgsPoint3DSymbol &other )
  : mAltClamping( other.altitudeClamping() )
  , mMaterial( other.material() ? other.material()->clone() : nullptr )
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

  elem.setAttribute( QStringLiteral( "material_type" ), mMaterial->type() );
  QDomElement elemMaterial = doc.createElement( QStringLiteral( "material" ) );
  mMaterial->writeXml( elemMaterial, context );
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
  mMaterial.reset( Qgs3D::materialRegistry()->createMaterialSettings( materialType ) );
  if ( !mMaterial )
    mMaterial.reset( Qgs3D::materialRegistry()->createMaterialSettings( QStringLiteral( "phong" ) ) );
  mMaterial->readXml( elemMaterial, context );

  mShape = shapeFromString( elem.attribute( QStringLiteral( "shape" ) ) );

  const QDomElement elemShapeProperties = elem.firstChildElement( QStringLiteral( "shape-properties" ) );
  mShapeProperties = QgsXmlUtils::readVariant( elemShapeProperties.firstChildElement() ).toMap();
  mShapeProperties[QStringLiteral( "model" )] = QVariant( context.pathResolver().readPath( mShapeProperties[QStringLiteral( "model" )].toString() ) );

  const QDomElement elemTransform = elem.firstChildElement( QStringLiteral( "transform" ) );
  mTransform = Qgs3DUtils::stringToMatrix4x4( elemTransform.attribute( QStringLiteral( "matrix" ) ) );

  const QDomElement symbolElem = elem.firstChildElement( QStringLiteral( "symbol" ) );

  setBillboardSymbol( QgsSymbolLayerUtils::loadSymbol< QgsMarkerSymbol >( symbolElem, context ) );
}

QList<QgsWkbTypes::GeometryType> QgsPoint3DSymbol::compatibleGeometryTypes() const
{
  return QList< QgsWkbTypes::GeometryType >() << QgsWkbTypes::PointGeometry;
}

void QgsPoint3DSymbol::setDefaultPropertiesFromLayer( const QgsVectorLayer *layer )
{
  const QgsVectorLayerElevationProperties *props = qgis::down_cast< const QgsVectorLayerElevationProperties * >( const_cast< QgsVectorLayer *>( layer )->elevationProperties() );

  mAltClamping = props->clamping();
  mTransform.data()[13] = static_cast< float >( props->zOffset() );
  mShapeProperties[QStringLiteral( "length" )] = props->extrusionEnabled() ? static_cast< float>( props->extrusionHeight() ) : 0.0f;
}

QgsPoint3DSymbol::Shape QgsPoint3DSymbol::shapeFromString( const QString &shape )
{
  if ( shape ==  QStringLiteral( "sphere" ) )
    return Sphere;
  else if ( shape == QLatin1String( "cone" ) )
    return Cone;
  else if ( shape == QLatin1String( "cube" ) )
    return Cube;
  else if ( shape == QLatin1String( "torus" ) )
    return Torus;
  else if ( shape == QLatin1String( "plane" ) )
    return Plane;
  else if ( shape == QLatin1String( "extruded-text" ) )
    return ExtrudedText;
  else if ( shape == QLatin1String( "model" ) )
    return Model;
  else if ( shape == QLatin1String( "billboard" ) )
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

QMatrix4x4 QgsPoint3DSymbol::billboardTransform() const
{
  QMatrix4x4 billboardTransformMatrix;
  billboardTransformMatrix.translate( QVector3D( 0, mTransform.data()[13], 0 ) );

  return billboardTransformMatrix;
}

QgsAbstractMaterialSettings *QgsPoint3DSymbol::material() const
{
  return mMaterial.get();
}

void QgsPoint3DSymbol::setMaterial( QgsAbstractMaterialSettings *material )
{
  if ( material == mMaterial.get() )
    return;

  mMaterial.reset( material );
}

bool QgsPoint3DSymbol::exportGeometries( Qgs3DSceneExporter *exporter, Qt3DCore::QEntity *entity, const QString &objectNamePrefix ) const
{
  if ( shape() == QgsPoint3DSymbol::Model )
  {
    Qt3DRender::QSceneLoader *sceneLoader = entity->findChild<Qt3DRender::QSceneLoader *>();
    if ( sceneLoader != nullptr )
    {
      const QVector<Qgs3DExportObject *> objects = exporter->processSceneLoaderGeometries( sceneLoader, objectNamePrefix );
      for ( Qgs3DExportObject *obj : objects )
      {
        obj->setSmoothEdges( exporter->smoothEdges() );
        obj->setupMaterial( material() );
      }
      exporter->mObjects << objects;
    }
    else
    {
      const QList<Qt3DRender::QMesh *> meshes = entity->findChildren<Qt3DRender::QMesh *>();
      for ( Qt3DRender::QMesh *mesh : meshes )
      {
        Qgs3DExportObject *object = exporter->processGeometryRenderer( mesh, objectNamePrefix );
        if ( object == nullptr ) continue;
        object->setSmoothEdges( exporter->smoothEdges() );
        object->setupMaterial( material() );
        exporter->mObjects << object;
      }
    }
    return true;
  }
  else if ( shape() == QgsPoint3DSymbol::Billboard )
  {
    Qgs3DExportObject *obj = exporter->processPoints( entity, objectNamePrefix );
    if ( obj != nullptr ) exporter->mObjects << obj;
    if ( obj != nullptr ) return true;
  }
  else
  {
    const QVector<Qgs3DExportObject *> objects = exporter->processInstancedPointGeometry( entity, objectNamePrefix );
    for ( Qgs3DExportObject *obj : objects )
    {
      obj->setupMaterial( material() );
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
