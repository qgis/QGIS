/***************************************************************************
  qgspoint3dsymbol_p.cpp
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

#include "qgspoint3dsymbol_p.h"

#include <memory>

#include "qgs3d.h"
#include "qgs3drendercontext.h"
#include "qgs3dutils.h"
#include "qgsapplication.h"
#include "qgsbillboardgeometry.h"
#include "qgsfeature3dhandler_p.h"
#include "qgsgeotransform.h"
#include "qgsgltf3dutils.h"
#include "qgslogger.h"
#include "qgsmaterial3dhandler.h"
#include "qgsmetalroughmaterial.h"
#include "qgsobj3dutils.h"
#include "qgsphongmaterial.h"
#include "qgsphongmaterialsettings.h"
#include "qgsphongtexturedmaterial.h"
#include "qgspoint3dbillboardmaterial.h"
#include "qgspoint3dsymbol.h"
#include "qgssourcecache.h"
#include "qgstexturematerial.h"
#include "qgsunlitmaterial.h"
#include "qgsvectorlayer.h"

#include <QFileInfo>
#include <QString>
#include <QUrl>
#include <QVector3D>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>
#include <Qt3DExtras/QConeGeometry>
#include <Qt3DExtras/QCuboidGeometry>
#include <Qt3DExtras/QCylinderGeometry>
#include <Qt3DExtras/QExtrudedTextGeometry>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QPlaneGeometry>
#include <Qt3DExtras/QSphereGeometry>
#include <Qt3DExtras/QTorusGeometry>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QPaintedTextureImage>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>

using namespace Qt::StringLiterals;

/// @cond PRIVATE


//* INSTANCED RENDERING *//


class QgsInstancedPoint3DSymbolHandler : public QgsFeature3DHandler
{
  public:
    QgsInstancedPoint3DSymbolHandler( const QgsPoint3DSymbol *symbol, const QgsFeatureIds &selectedIds )
      : mSymbol( static_cast<QgsPoint3DSymbol *>( symbol->clone() ) )
      , mSelectedIds( selectedIds )
    {}

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, const QgsBox3D &chunkExtent ) override;
    void processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:
    static QgsMaterial *material( const QgsPoint3DSymbol *symbol, const QgsMaterialContext &materialContext, bool hasDataDefinedScale, bool hasDataDefinedRotation );
    static Qt3DRender::QGeometryRenderer *renderer( const QgsPoint3DSymbol *symbol, const QVector<QVector3D> &positions, const QVector<QVector3D> &scales, const QVector<QVector4D> rotations );
    static Qt3DCore::QGeometry *symbolGeometry( const QgsPoint3DSymbol *symbol );

    //! temporary data we will pass to the tessellator
    struct PointData
    {
        QVector<QVector3D> positions; // contains triplets of float x,y,z for each point
        QVector<QVector3D> scales;
        QVector<QVector4D> rotations;
    };

    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected );

    // input specific for this class
    std::unique_ptr<QgsPoint3DSymbol> mSymbol;
    QVector3D mSymbolScale;
    QQuaternion mSymbolRotation;
    QVector3D mPointTranslation;
    // inputs - generic
    QgsFeatureIds mSelectedIds;
    // outputs
    PointData outNormal;   //!< Features that are not selected
    PointData outSelected; //!< Features that are selected
};


bool QgsInstancedPoint3DSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, const QgsBox3D &chunkExtent )
{
  mChunkOrigin = chunkExtent.center();
  mChunkExtent = chunkExtent;

  QSet<QString> attrs = mSymbol->dataDefinedProperties().referencedFields( context.expressionContext() );
  attributeNames.unite( attrs );
  attrs = mSymbol->materialSettings()->dataDefinedProperties().referencedFields( context.expressionContext() );
  attributeNames.unite( attrs );

  Qgs3DUtils::decomposeTransformMatrix( mSymbol->transform(), mPointTranslation, mSymbolRotation, mSymbolScale );

  return true;
}

void QgsInstancedPoint3DSymbolHandler::processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context )
{
  PointData &out = mSelectedIds.contains( feature.id() ) ? outSelected : outNormal;

  if ( feature.geometry().isNull() )
    return;

  const QgsPropertyCollection &ddp = mSymbol->dataDefinedProperties();

  QgsVector3D translation = mPointTranslation;
  const bool hasDDTranslation = ddp.isActive( QgsAbstract3DSymbol::Property::TranslationX )
                                || ddp.isActive( QgsAbstract3DSymbol::Property::TranslationY )
                                || ddp.isActive( QgsAbstract3DSymbol::Property::TranslationZ );
  if ( hasDDTranslation )
  {
    const double translationX = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::TranslationX, context.expressionContext(), translation.x() );
    const double translationY = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::TranslationY, context.expressionContext(), translation.y() );
    const double translationZ = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::TranslationZ, context.expressionContext(), translation.z() );
    translation = QgsVector3D( translationX, translationY, translationZ );
  }

  const std::size_t oldSize = out.positions.size();
  Qgs3DUtils::extractPointPositions( feature, context, mChunkOrigin, mSymbol->altitudeClamping(), out.positions, translation );

  const std::size_t added = out.positions.size() - oldSize;

  const bool hasDDScale = ddp.isActive( QgsAbstract3DSymbol::Property::ScaleX ) || ddp.isActive( QgsAbstract3DSymbol::Property::ScaleY ) || ddp.isActive( QgsAbstract3DSymbol::Property::ScaleZ );

  if ( hasDDScale )
  {
    out.scales.resize( out.positions.size() );
    QVector3D *outScale = out.scales.data() + oldSize;

    const double scaleX = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::ScaleX, context.expressionContext(), mSymbolScale.x() );
    const double scaleY = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::ScaleY, context.expressionContext(), mSymbolScale.y() );
    const double scaleZ = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::ScaleZ, context.expressionContext(), mSymbolScale.z() );

    for ( std::size_t i = 0; i < added; ++i )
    {
      ( *outScale++ ) = QVector3D( static_cast< float >( scaleX ), static_cast< float >( scaleY ), static_cast< float >( scaleZ ) );
    }
  }

  const bool hasDDRotation = ddp.isActive( QgsAbstract3DSymbol::Property::RotationX )
                             || ddp.isActive( QgsAbstract3DSymbol::Property::RotationY )
                             || ddp.isActive( QgsAbstract3DSymbol::Property::RotationZ );
  if ( hasDDRotation )
  {
    out.rotations.resize( out.positions.size() );
    QVector4D *outRotation = out.rotations.data() + oldSize;

    // extract default rotation components from symbol rotation
    const QVector3D baseEuler = mSymbolRotation.toEulerAngles();

    const double rotationX = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::RotationX, context.expressionContext(), baseEuler.x() );
    const double rotationY = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::RotationY, context.expressionContext(), baseEuler.y() );
    const double rotationZ = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::RotationZ, context.expressionContext(), baseEuler.z() );

    //... and then re-calculate the rotation vector for this feature
    const QQuaternion finalQuat = QQuaternion::fromEulerAngles( static_cast< float >( rotationX ), static_cast< float >( rotationY ), static_cast< float >( rotationZ ) );
    const QVector4D finalVec4 = finalQuat.toVector4D();

    for ( std::size_t i = 0; i < added; ++i )
    {
      ( *outRotation++ ) = finalVec4;
    }
  }

  mFeatureCount++;
}

void QgsInstancedPoint3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
  makeEntity( parent, context, outSelected, true );

  auto updateZRangeFromPointData = [this]( const PointData &pointData ) {
    const QVector3D *scales = pointData.scales.empty() ? nullptr : pointData.scales.constData();
    for ( const QVector3D &pos : std::as_const( pointData.positions ) )
    {
      double minZ = 0;
      double maxZ = 0;

      // also account for the actual height of the objects themselves
      // NOTE -- these calculations are naive, and assume no rotation or scaling of the symbol!
      switch ( mSymbol->shape() )
      {
        case Qgis::Point3DShape::Cylinder:
        {
          const float length = mSymbol->shapeProperty( u"length"_s ).toFloat();
          minZ -= length * 0.5f;
          maxZ += length * 0.5f;
          break;
        }

        case Qgis::Point3DShape::Sphere:
        {
          const float radius = mSymbol->shapeProperty( u"radius"_s ).toFloat();
          minZ -= radius;
          maxZ += radius;
          break;
        }

        case Qgis::Point3DShape::Cone:
        {
          const float length = mSymbol->shapeProperty( u"length"_s ).toFloat();
          minZ -= length * 0.5f;
          maxZ += length * 0.5f;
          break;
        }

        case Qgis::Point3DShape::Cube:
        {
          const float size = mSymbol->shapeProperty( u"size"_s ).toFloat();
          minZ -= size * 0.5f;
          maxZ += size * 0.5f;
          break;
        }

        case Qgis::Point3DShape::Torus:
        {
          const float radius = mSymbol->shapeProperty( u"radius"_s ).toFloat();
          minZ -= radius;
          maxZ += radius;
          break;
        }

        case Qgis::Point3DShape::Plane:
        {
          // worst case scenario -- even though planes are usually rotated so that they are flat,
          // let's account for possible overridden rotation
          const float size = mSymbol->shapeProperty( u"size"_s ).toFloat();
          minZ -= size * 0.5f;
          maxZ += size * 0.5f;
          break;
        }

        case Qgis::Point3DShape::ExtrudedText:
        case Qgis::Point3DShape::Model:
        case Qgis::Point3DShape::Billboard:
          break;
      }

      if ( scales )
      {
        const double zScale = ( *scales++ )[2];
        minZ *= zScale;
        maxZ *= zScale;
      }

      // as we are relative to chunk center elevation we have to add mChunkOrigin.z()
      minZ += pos.z() + mChunkOrigin.z();
      maxZ += pos.z() + mChunkOrigin.z();

      if ( minZ < mZMin )
        mZMin = static_cast< float >( minZ );
      if ( maxZ > mZMax )
        mZMax = static_cast< float >( maxZ );
    }
  };

  updateZRangeFromPointData( outNormal );
  updateZRangeFromPointData( outSelected );
}

void QgsInstancedPoint3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected )
{
  if ( out.positions.isEmpty() )
  {
    return; // nothing to show - no need to create the entity
  }

  // build the default material
  QgsMaterialContext materialContext = QgsMaterialContext::fromRenderContext( context );
  materialContext.setIsSelected( selected );
  materialContext.setIsHighlighted( mHighlightingEnabled );
  QgsMaterial *mat = material( mSymbol.get(), materialContext, !out.scales.empty(), !out.rotations.empty() );
  if ( !mat )
    return;

  mat->addParameter( new Qt3DRender::QParameter( "symbolScale", mSymbolScale, mat ) );
  mat->addParameter( new Qt3DRender::QParameter( "symbolRotation", mSymbolRotation.toVector4D(), mat ) );

  // add transform (our geometry has coordinates relative to mChunkOrigin)
  QgsGeoTransform *tr = new QgsGeoTransform;
  tr->setGeoTranslation( mChunkOrigin );

  // build the entity
  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;
  entity->addComponent( renderer( mSymbol.get(), out.positions, out.scales, out.rotations ) );
  entity->addComponent( mat );
  entity->addComponent( tr );
  entity->setParent( parent );

  // cppcheck wrongly believes entity will leak
  // cppcheck-suppress memleak
}


QgsMaterial *QgsInstancedPoint3DSymbolHandler::material( const QgsPoint3DSymbol *symbol, const QgsMaterialContext &materialContext, bool hasDataDefinedScale, bool hasDataDefinedRotation )
{
  Qgis::InstancedMaterialFlags flags;
  if ( hasDataDefinedScale )
    flags |= Qgis::InstancedMaterialFlag::DataDefinedScale;
  if ( hasDataDefinedRotation )
    flags |= Qgis::InstancedMaterialFlag::DataDefinedRotation;

  const QString upAxis = symbol->shapeProperty( u"upAxis"_s ).toString();
  const QString forwardAxis = symbol->shapeProperty( u"forwardAxis"_s ).toString();

  const QMatrix4x4 meshTransform = Qgs3DUtils::axisTransformMatrix( !upAxis.isEmpty() ? upAxis : u"y"_s, !forwardAxis.isEmpty() ? forwardAxis : u"-z"_s );

  if ( materialContext.isHighlighted() )
  {
    QgsUnlitMaterial *mat = Qgs3D::createHighlightMaterial();
    mat->setInstancingEnabled( true, flags );
    mat->setInstancingMeshTransform( meshTransform );
    return mat;
  }

  const QgsAbstractMaterialSettings *settings = symbol->materialSettings();
  if ( const QgsAbstractMaterial3DHandler *handler = Qgs3D::handlerForMaterialSettings( settings ) )
  {
    return handler->toInstancedMaterial( settings, materialContext, flags, meshTransform );
  }

  return nullptr;
}

Qt3DRender::QGeometryRenderer *QgsInstancedPoint3DSymbolHandler::renderer(
  const QgsPoint3DSymbol *symbol, const QVector<QVector3D> &positions, const QVector<QVector3D> &scales, const QVector<QVector4D> rotations
)
{
  const std::size_t count = positions.count();
  const std::size_t byteCount = positions.count() * sizeof( QVector3D );
  QByteArray ba;
  ba.resize( byteCount );
  memcpy( ba.data(), positions.constData(), byteCount );

  Qt3DCore::QBuffer *instanceBuffer = new Qt3DCore::QBuffer();
  instanceBuffer->setData( ba );

  Qt3DCore::QAttribute *instanceTranslationAttribute = new Qt3DCore::QAttribute;
  instanceTranslationAttribute->setName( u"instanceTranslation"_s );
  instanceTranslationAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  instanceTranslationAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  instanceTranslationAttribute->setVertexSize( 3 );
  instanceTranslationAttribute->setByteOffset( 0 );
  instanceTranslationAttribute->setDivisor( 1 );
  instanceTranslationAttribute->setBuffer( instanceBuffer );
  instanceTranslationAttribute->setCount( count );
  instanceTranslationAttribute->setByteStride( 3 * sizeof( float ) );

  Qt3DCore::QGeometry *geometry = symbolGeometry( symbol );
  geometry->addAttribute( instanceTranslationAttribute );
  geometry->setBoundingVolumePositionAttribute( instanceTranslationAttribute );

  if ( !scales.empty() )
  {
    auto scaleBuffer = new Qt3DCore::QBuffer();
    auto instanceScaleAttribute = new Qt3DCore::QAttribute;
    instanceScaleAttribute->setName( u"instanceScale"_s );
    instanceScaleAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
    instanceScaleAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
    instanceScaleAttribute->setVertexSize( 3 );
    instanceScaleAttribute->setByteOffset( 0 );
    instanceScaleAttribute->setDivisor( 1 );
    instanceScaleAttribute->setByteStride( 3 * sizeof( float ) );
    QByteArray scaleBa;
    scaleBa.resize( byteCount );
    memcpy( scaleBa.data(), scales.constData(), byteCount );

    scaleBuffer->setData( scaleBa );
    instanceScaleAttribute->setCount( count );

    instanceScaleAttribute->setBuffer( scaleBuffer );
    geometry->addAttribute( instanceScaleAttribute );
  }

  if ( !rotations.empty() )
  {
    auto rotationBuffer = new Qt3DCore::QBuffer();
    auto instanceRotationAttribute = new Qt3DCore::QAttribute;
    instanceRotationAttribute->setName( u"instanceRotation"_s );
    instanceRotationAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
    instanceRotationAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
    instanceRotationAttribute->setVertexSize( 4 );
    instanceRotationAttribute->setByteOffset( 0 );
    instanceRotationAttribute->setDivisor( 1 );
    instanceRotationAttribute->setByteStride( 4 * sizeof( float ) );

    QByteArray rotationBa;
    const std::size_t rotationByteCount = positions.count() * sizeof( QVector4D );
    rotationBa.resize( rotationByteCount );
    memcpy( rotationBa.data(), rotations.constData(), rotationByteCount );
    rotationBuffer->setData( rotationBa );
    instanceRotationAttribute->setCount( count );

    instanceRotationAttribute->setBuffer( rotationBuffer );
    geometry->addAttribute( instanceRotationAttribute );
  }

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setGeometry( geometry );
  renderer->setInstanceCount( count );

  return renderer;
}

Qt3DCore::QGeometry *QgsInstancedPoint3DSymbolHandler::symbolGeometry( const QgsPoint3DSymbol *symbol )
{
  switch ( symbol->shape() )
  {
    case Qgis::Point3DShape::Cylinder:
    {
      const float radius = symbol->shapeProperty( u"radius"_s ).toFloat();
      const float length = symbol->shapeProperty( u"length"_s ).toFloat();
      Qt3DExtras::QCylinderGeometry *g = new Qt3DExtras::QCylinderGeometry;
      //g->setRings(2);  // how many vertices vertically
      //g->setSlices(8); // how many vertices on circumference
      g->setRadius( radius );
      g->setLength( length );
      return g;
    }

    case Qgis::Point3DShape::Sphere:
    {
      const float radius = symbol->shapeProperty( u"radius"_s ).toFloat();
      const int rings = symbol->shapeProperty( u"rings"_s ).toInt();
      const int slices = symbol->shapeProperty( u"slices"_s ).toInt();

      const bool tangents = symbol->materialSettings() && symbol->materialSettings()->requiresTangents();
      Qt3DExtras::QSphereGeometry *g = new Qt3DExtras::QSphereGeometry;
      g->setRadius( radius );
      g->setRings( rings );
      g->setSlices( slices );
      g->setGenerateTangents( tangents );
      return g;
    }

    case Qgis::Point3DShape::Cone:
    {
      const float length = symbol->shapeProperty( u"length"_s ).toFloat();
      const float bottomRadius = symbol->shapeProperty( u"bottomRadius"_s ).toFloat();
      const float topRadius = symbol->shapeProperty( u"topRadius"_s ).toFloat();

      Qt3DExtras::QConeGeometry *g = new Qt3DExtras::QConeGeometry;
      g->setLength( length );
      g->setBottomRadius( bottomRadius );
      g->setTopRadius( topRadius );
      //g->setHasBottomEndcap(hasBottomEndcap);
      //g->setHasTopEndcap(hasTopEndcap);
      return g;
    }

    case Qgis::Point3DShape::Cube:
    {
      const float size = symbol->shapeProperty( u"size"_s ).toFloat();
      Qt3DExtras::QCuboidGeometry *g = new Qt3DExtras::QCuboidGeometry;
      g->setXExtent( size );
      g->setYExtent( size );
      g->setZExtent( size );
      return g;
    }

    case Qgis::Point3DShape::Torus:
    {
      const float radius = symbol->shapeProperty( u"radius"_s ).toFloat();
      const float minorRadius = symbol->shapeProperty( u"minorRadius"_s ).toFloat();
      Qt3DExtras::QTorusGeometry *g = new Qt3DExtras::QTorusGeometry;
      g->setRadius( radius );
      g->setMinorRadius( minorRadius );
      return g;
    }

    case Qgis::Point3DShape::Plane:
    {
      const float size = symbol->shapeProperty( u"size"_s ).toFloat();
      Qt3DExtras::QPlaneGeometry *g = new Qt3DExtras::QPlaneGeometry;
      g->setWidth( size );
      g->setHeight( size );
      return g;
    }

    case Qgis::Point3DShape::ExtrudedText:
    {
      const float depth = symbol->shapeProperty( u"depth"_s ).toFloat();
      const QString text = symbol->shapeProperty( u"text"_s ).toString();
      Qt3DExtras::QExtrudedTextGeometry *g = new Qt3DExtras::QExtrudedTextGeometry;
      g->setDepth( depth );
      g->setText( text );
      return g;
    }

    case Qgis::Point3DShape::Model:
    case Qgis::Point3DShape::Billboard:
      break;
  }
  Q_ASSERT( false );
  return nullptr;
}

//* 3D MODEL RENDERING *//


class QgsModelPoint3DSymbolHandler : public QgsFeature3DHandler
{
  public:
    QgsModelPoint3DSymbolHandler( const QgsPoint3DSymbol *symbol, const QgsFeatureIds &selectedIds )
      : mSymbol( static_cast<QgsPoint3DSymbol *>( symbol->clone() ) )
      , mSelectedIds( selectedIds )
    {}

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, const QgsBox3D &chunkExtent ) override;
    void processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:
    void addInstancedEntities(
      const QVector<QVector3D> &positions,
      const QVector<QVector3D> &scales,
      const QVector<QQuaternion> &rotations,
      const QgsVector3D &chunkOrigin,
      const QgsPoint3DSymbol *symbol,
      Qt3DCore::QEntity *parent,
      const QgsMaterialContext &materialContext,
      bool useEmbeddedTexture
    );

    //! temporary data we will pass to the tessellator
    struct PointData
    {
        QVector<QVector3D> positions; // contains triplets of float x,y,z for each point
        QVector<QVector3D> scales;
        QVector<QQuaternion> rotations;
    };

    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected );

    // input specific for this class
    std::unique_ptr<QgsPoint3DSymbol> mSymbol;

    QVector3D mSymbolScale;
    QQuaternion mSymbolRotation;
    QVector3D mPointTranslation;

    // inputs - generic
    QgsFeatureIds mSelectedIds;
    // outputs
    PointData outNormal;   //!< Features that are not selected
    PointData outSelected; //!< Features that are selected
};

bool QgsModelPoint3DSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, const QgsBox3D &chunkExtent )
{
  Q_UNUSED( context )
  Q_UNUSED( attributeNames )

  mChunkOrigin = chunkExtent.center();
  mChunkExtent = chunkExtent;

  QSet<QString> attrs = mSymbol->dataDefinedProperties().referencedFields( context.expressionContext() );
  attributeNames.unite( attrs );

  Qgs3DUtils::decomposeTransformMatrix( mSymbol->transform(), mPointTranslation, mSymbolRotation, mSymbolScale );
  return true;
}

void QgsModelPoint3DSymbolHandler::processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context )
{
  PointData &out = mSelectedIds.contains( feature.id() ) ? outSelected : outNormal;

  if ( feature.geometry().isNull() )
    return;

  const QgsPropertyCollection &ddp = mSymbol->dataDefinedProperties();

  QgsVector3D translation = mPointTranslation;
  const bool hasDDTranslation = ddp.isActive( QgsAbstract3DSymbol::Property::TranslationX )
                                || ddp.isActive( QgsAbstract3DSymbol::Property::TranslationY )
                                || ddp.isActive( QgsAbstract3DSymbol::Property::TranslationZ );
  if ( hasDDTranslation )
  {
    const double translationX = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::TranslationX, context.expressionContext(), translation.x() );
    const double translationY = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::TranslationY, context.expressionContext(), translation.y() );
    const double translationZ = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::TranslationZ, context.expressionContext(), translation.z() );
    translation = QgsVector3D( translationX, translationY, translationZ );
  }

  const std::size_t oldSize = out.positions.size();
  Qgs3DUtils::extractPointPositions( feature, context, mChunkOrigin, mSymbol->altitudeClamping(), out.positions, translation );
  const std::size_t added = out.positions.size() - oldSize;

  const bool hasDDScale = ddp.isActive( QgsAbstract3DSymbol::Property::ScaleX ) || ddp.isActive( QgsAbstract3DSymbol::Property::ScaleY ) || ddp.isActive( QgsAbstract3DSymbol::Property::ScaleZ );
  if ( hasDDScale )
  {
    out.scales.resize( out.positions.size() );
    QVector3D *outScale = out.scales.data() + oldSize;
    const double scaleX = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::ScaleX, context.expressionContext(), mSymbolScale.x() );
    const double scaleY = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::ScaleY, context.expressionContext(), mSymbolScale.y() );
    const double scaleZ = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::ScaleZ, context.expressionContext(), mSymbolScale.z() );
    const QVector3D scale( static_cast< float >( scaleX ), static_cast< float >( scaleY ), static_cast< float >( scaleZ ) );
    for ( std::size_t i = 0; i < added; ++i )
      ( *outScale++ ) = scale;
  }

  const bool hasDDRotation = ddp.isActive( QgsAbstract3DSymbol::Property::RotationX )
                             || ddp.isActive( QgsAbstract3DSymbol::Property::RotationY )
                             || ddp.isActive( QgsAbstract3DSymbol::Property::RotationZ );
  if ( hasDDRotation )
  {
    out.rotations.resize( out.positions.size() );
    QQuaternion *outRotation = out.rotations.data() + oldSize;
    const QVector3D baseEuler = mSymbolRotation.toEulerAngles();
    const double rotationX = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::RotationX, context.expressionContext(), baseEuler.x() );
    const double rotationY = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::RotationY, context.expressionContext(), baseEuler.y() );
    const double rotationZ = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::RotationZ, context.expressionContext(), baseEuler.z() );
    const QQuaternion rotation = QQuaternion::fromEulerAngles( static_cast< float >( rotationX ), static_cast< float >( rotationY ), static_cast< float >( rotationZ ) );
    for ( std::size_t i = 0; i < added; ++i )
      ( *outRotation++ ) = rotation;
  }

  mFeatureCount++;
}

void QgsModelPoint3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
  makeEntity( parent, context, outSelected, true );

  updateZRangeFromPositions( outNormal.positions );
  updateZRangeFromPositions( outSelected.positions );

  // the elevation offset is applied separately in QTransform added to sub-entities
  const float symbolHeight = mSymbol->transform().data()[14];
  // as we are relative to chunk center elevation we have to add mChunkOrigin.z()
  mZMin += static_cast<float>( symbolHeight + mChunkOrigin.z() );
  mZMax += static_cast<float>( symbolHeight + mChunkOrigin.z() );
}

void QgsModelPoint3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected )
{
  if ( out.positions.isEmpty() )
  {
    return; // nothing to show - no need to create the entity
  }

  const QgsAbstractMaterialSettings *settings = mSymbol->materialSettings();
  const bool useEmbeddedTexture = !mSymbol->shapeProperty( u"overwriteMaterial"_s ).toBool() && ( !settings || settings->type() == "null"_L1 );

  QgsMaterialContext materialContext = QgsMaterialContext::fromRenderContext( context );
  materialContext.setIsSelected( selected );
  materialContext.setIsHighlighted( mHighlightingEnabled );

  addInstancedEntities( out.positions, out.scales, out.rotations, mChunkOrigin, mSymbol.get(), parent, materialContext, useEmbeddedTexture );
}

void QgsModelPoint3DSymbolHandler::addInstancedEntities(
  const QVector<QVector3D> &positions,
  const QVector<QVector3D> &scales,
  const QVector<QQuaternion> &rotations,
  const QgsVector3D &chunkOrigin,
  const QgsPoint3DSymbol *symbol,
  Qt3DCore::QEntity *parent,
  const QgsMaterialContext &materialContext,
  bool useEmbeddedTexture
)
{
  const QString source = QgsApplication::sourceCache()->localFilePath( symbol->shapeProperty( u"model"_s ).toString() );

  std::vector<QgsMeshNodeData> meshes;

  const QString suffix = QFileInfo( source ).suffix().toLower();
  if ( suffix == "gltf"_L1 || suffix == "glb"_L1 )
  {
    QStringList gltfErrors;
    for ( QgsMeshNodeData &m : QgsGltf3DUtils::buildGltfGeometries( source, materialContext, &gltfErrors, parent ) )
      meshes.push_back( std::move( m ) );
    if ( !gltfErrors.isEmpty() )
      QgsDebugError( u"GLTF instancing errors for '%1': %2"_s.arg( source, gltfErrors.join( ", "_L1 ) ) );
  }
  else if ( suffix == "obj"_L1 )
  {
    for ( QgsMeshNodeData &m : QgsObj3DUtils::buildObjGeometries( source, materialContext ) )
      meshes.push_back( std::move( m ) );
  }
  else
  {
    QgsDebugError( u"Unsupported model file suffix '%1' for source: %2"_s.arg( suffix, source ) );
    return;
  }

  if ( meshes.empty() )
  {
    QgsDebugMsgLevel( u"No meshes loaded for model symbol source: %1"_s.arg( source ), 2 );
    return;
  }

  const int count = positions.size();

  QByteArray translationData( reinterpret_cast<const char *>( positions.constData() ), static_cast<qsizetype>( count * sizeof( QVector3D ) ) );

  const QString upAxis = symbol->shapeProperty( u"upAxis"_s ).toString();
  const QString forwardAxis = symbol->shapeProperty( u"forwardAxis"_s ).toString();

  Qgis::InstancedMaterialFlags instancedFlags;
  if ( !scales.empty() )
    instancedFlags |= Qgis::InstancedMaterialFlag::DataDefinedScale;
  if ( !rotations.empty() )
    instancedFlags |= Qgis::InstancedMaterialFlag::DataDefinedRotation;

  const QMatrix4x4 meshTransform = Qgs3DUtils::axisTransformMatrix( upAxis, forwardAxis );

  Qt3DCore::QBuffer *translationBufferData = new Qt3DCore::QBuffer( parent );
  translationBufferData->setData( translationData );

  Qt3DCore::QBuffer *scaleBufferData = nullptr;
  if ( !scales.empty() )
  {
    QByteArray scaleData( reinterpret_cast<const char *>( scales.constData() ), static_cast<qsizetype>( count * sizeof( QVector3D ) ) );
    scaleBufferData = new Qt3DCore::QBuffer( parent );
    scaleBufferData->setData( scaleData );
  }

  Qt3DCore::QBuffer *rotationBufferData = nullptr;
  if ( !rotations.empty() )
  {
    QVector<QVector4D> rotationVectors;
    rotationVectors.reserve( count );
    for ( const QQuaternion &q : rotations )
      rotationVectors.append( q.toVector4D() );
    QByteArray rotationData( reinterpret_cast<const char *>( rotationVectors.constData() ), static_cast<qsizetype>( count * sizeof( QVector4D ) ) );
    rotationBufferData = new Qt3DCore::QBuffer( parent );
    rotationBufferData->setData( rotationData );
  }

  for ( QgsMeshNodeData &mesh : meshes )
  {
    Qt3DCore::QGeometry *geom = mesh.geometry.release();

    Qt3DCore::QAttribute *translationAttribute = new Qt3DCore::QAttribute;
    translationAttribute->setName( u"instanceTranslation"_s );
    translationAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
    translationAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
    translationAttribute->setVertexSize( 3 );
    translationAttribute->setByteOffset( 0 );
    translationAttribute->setByteStride( 3 * sizeof( float ) );
    translationAttribute->setDivisor( 1 );
    translationAttribute->setCount( static_cast<uint>( count ) );
    translationAttribute->setBuffer( translationBufferData );
    geom->addAttribute( translationAttribute );
    geom->setBoundingVolumePositionAttribute( translationAttribute );

    if ( scaleBufferData )
    {
      Qt3DCore::QAttribute *scaleAttribute = new Qt3DCore::QAttribute;
      scaleAttribute->setName( u"instanceScale"_s );
      scaleAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
      scaleAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
      scaleAttribute->setVertexSize( 3 );
      scaleAttribute->setByteOffset( 0 );
      scaleAttribute->setByteStride( 3 * sizeof( float ) );
      scaleAttribute->setDivisor( 1 );
      scaleAttribute->setCount( static_cast<uint>( count ) );
      scaleAttribute->setBuffer( scaleBufferData );
      geom->addAttribute( scaleAttribute );
    }

    if ( rotationBufferData )
    {
      Qt3DCore::QAttribute *rotationAttribute = new Qt3DCore::QAttribute;
      rotationAttribute->setName( u"instanceRotation"_s );
      rotationAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
      rotationAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
      rotationAttribute->setVertexSize( 4 );
      rotationAttribute->setByteOffset( 0 );
      rotationAttribute->setByteStride( 4 * sizeof( float ) );
      rotationAttribute->setDivisor( 1 );
      rotationAttribute->setCount( static_cast<uint>( count ) );
      rotationAttribute->setBuffer( rotationBufferData );
      geom->addAttribute( rotationAttribute );
    }

    const QMatrix4x4 meshTransformCombined = meshTransform * mesh.meshTransform;

    QgsMaterial *mat = nullptr;
    if ( materialContext.isHighlighted() )
    {
      QgsUnlitMaterial *highlightMaterial = Qgs3D::createHighlightMaterial();
      highlightMaterial->setInstancingEnabled( true, instancedFlags );
      highlightMaterial->setInstancingMeshTransform( meshTransformCombined );
      mat = highlightMaterial;
    }
    else if ( useEmbeddedTexture )
    {
      if ( QgsPhongTexturedMaterial *phongTexMat = qobject_cast<QgsPhongTexturedMaterial *>( mesh.material.get() ) )
      {
        phongTexMat->setInstancingEnabled( true, instancedFlags );
        phongTexMat->setInstancingMeshTransform( meshTransformCombined );
      }
      else if ( QgsPhongMaterial *phongMat = qobject_cast<QgsPhongMaterial *>( mesh.material.get() ) )
      {
        phongMat->setInstancingEnabled( true, instancedFlags );
        phongMat->setInstancingMeshTransform( meshTransformCombined );
      }
      else if ( QgsMetalRoughMaterial *pbrMat = qobject_cast<QgsMetalRoughMaterial *>( mesh.material.get() ) )
      {
        pbrMat->setInstancingEnabled( true, instancedFlags );
        pbrMat->setInstancingMeshTransform( meshTransformCombined );
      }
      else if ( QgsTextureMaterial *gltfTexMat = qobject_cast<QgsTextureMaterial *>( mesh.material.get() ) )
      {
        gltfTexMat->setInstancingEnabled( true, instancedFlags );
        gltfTexMat->setInstancingMeshTransform( meshTransformCombined );
      }
      mat = mesh.material.release();
    }
    else
    {
      const QgsAbstractMaterialSettings *settings = symbol->materialSettings();
      if ( const QgsAbstractMaterial3DHandler *handler = Qgs3D::handlerForMaterialSettings( settings ) )
        mat = handler->toInstancedMaterial( settings, materialContext, instancedFlags, meshTransformCombined );
    }

    if ( !mat )
    {
      QgsMetalRoughMaterial *metal = new QgsMetalRoughMaterial();
      metal->setInstancingEnabled( true, instancedFlags );
      metal->setInstancingMeshTransform( meshTransformCombined );
      mat = metal;
    }

    mat->addParameter( new Qt3DRender::QParameter( "symbolScale", mSymbolScale, mat ) );
    mat->addParameter( new Qt3DRender::QParameter( "symbolRotation", mSymbolRotation.toVector4D(), mat ) );

    Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
    renderer->setGeometry( geom );
    renderer->setInstanceCount( static_cast<int>( count ) );

    QgsGeoTransform *geoTransform = new QgsGeoTransform;
    geoTransform->setGeoTranslation( chunkOrigin );

    Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;
    entity->addComponent( renderer );
    entity->addComponent( mat );
    entity->addComponent( geoTransform );
    entity->setParent( parent );

    // cppcheck wrongly believes entity will leak
    // cppcheck-suppress memleak
  }
}

// --------------

//* BILLBOARD RENDERING *//

class QgsPoint3DBillboardSymbolHandler : public QgsFeature3DHandler
{
  public:
    QgsPoint3DBillboardSymbolHandler( const QgsPoint3DSymbol *symbol, const QgsFeatureIds &selectedIds )
      : mSymbol( static_cast<QgsPoint3DSymbol *>( symbol->clone() ) )
      , mSelectedIds( selectedIds )
    {}

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, const QgsBox3D &chunkExtent ) override;
    void processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:
    //! temporary data we will pass to the tessellator
    struct PointData
    {
        QVector<QVector3D> positions; // contains triplets of float x,y,z for each point
    };

    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected );

    // input specific for this class
    std::unique_ptr<QgsPoint3DSymbol> mSymbol;
    // inputs - generic
    QgsFeatureIds mSelectedIds;
    // outputs
    PointData outNormal;   //!< Features that are not selected
    PointData outSelected; //!< Features that are selected
};

bool QgsPoint3DBillboardSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, const QgsBox3D &chunkExtent )
{
  Q_UNUSED( context )
  Q_UNUSED( attributeNames )

  mChunkOrigin = chunkExtent.center();
  mChunkExtent = chunkExtent;

  return true;
}

void QgsPoint3DBillboardSymbolHandler::processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context )
{
  PointData &out = mSelectedIds.contains( feature.id() ) ? outSelected : outNormal;

  if ( feature.geometry().isNull() )
    return;

  Qgs3DUtils::extractPointPositions( feature, context, mChunkOrigin, mSymbol->altitudeClamping(), out.positions );
  mFeatureCount++;
}

void QgsPoint3DBillboardSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
  makeEntity( parent, context, outSelected, true );

  updateZRangeFromPositions( outNormal.positions );
  updateZRangeFromPositions( outSelected.positions );

  // the elevation offset is applied externally through a QTransform of QEntity so let's account for it
  const float billboardHeight = mSymbol->billboardHeight();
  // as we are relative to chunk center elevation we have to add mChunkOrigin.z()
  mZMin += static_cast<float>( billboardHeight + mChunkOrigin.z() );
  mZMax += static_cast<float>( billboardHeight + mChunkOrigin.z() );
}

void QgsPoint3DBillboardSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected )
{
  if ( out.positions.isEmpty() )
  {
    return; // nothing to show - no need to create the entity
  }

  // Billboard Geometry
  QgsBillboardGeometry *billboardGeometry = new QgsBillboardGeometry();
  billboardGeometry->setPositions( out.positions );

  // Billboard Geometry Renderer
  Qt3DRender::QGeometryRenderer *billboardGeometryRenderer = new Qt3DRender::QGeometryRenderer;
  billboardGeometryRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::Points );
  billboardGeometryRenderer->setGeometry( billboardGeometry );
  billboardGeometryRenderer->setVertexCount( billboardGeometry->count() );

  // Billboard Material
  QgsPoint3DBillboardMaterial *billboardMaterial = new QgsPoint3DBillboardMaterial();
  QgsMarkerSymbol *symbol = mSymbol->billboardSymbol();

  if ( symbol )
  {
    billboardMaterial->setTexture2DFromSymbol( symbol, context, selected );
  }
  else
  {
    billboardMaterial->useDefaultSymbol( context, selected );
  }

  // Billboard Transform
  QgsGeoTransform *billboardTransform = new QgsGeoTransform;
  billboardTransform->setGeoTranslation( mChunkOrigin + QgsVector3D( 0, 0, mSymbol->billboardHeight() ) );

  // Build the entity
  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

  entity->addComponent( billboardMaterial );
  entity->addComponent( billboardTransform );
  entity->addComponent( billboardGeometryRenderer );
  entity->setParent( parent );

  // cppcheck wrongly believes entity will leak
  // cppcheck-suppress memleak
}


namespace Qgs3DSymbolImpl
{

  QgsFeature3DHandler *handlerForPoint3DSymbol( const QgsVectorLayer *layer, const QgsAbstract3DSymbol *symbol )
  {
    const QgsPoint3DSymbol *pointSymbol = dynamic_cast<const QgsPoint3DSymbol *>( symbol );
    if ( !pointSymbol )
      return nullptr;

    if ( pointSymbol->shape() == Qgis::Point3DShape::Model )
      return new QgsModelPoint3DSymbolHandler( pointSymbol, layer->selectedFeatureIds() );
    // Add proper handler for billboard
    else if ( pointSymbol->shape() == Qgis::Point3DShape::Billboard )
      return new QgsPoint3DBillboardSymbolHandler( pointSymbol, layer->selectedFeatureIds() );
    else
      return new QgsInstancedPoint3DSymbolHandler( pointSymbol, layer->selectedFeatureIds() );
  }
} // namespace Qgs3DSymbolImpl

/// @endcond
