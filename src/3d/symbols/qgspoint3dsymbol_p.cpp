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

#include "qgs3d.h"
#include "qgs3drendercontext.h"
#include "qgs3dutils.h"
#include "qgsapplication.h"
#include "qgsbillboardgeometry.h"
#include "qgsfeature3dhandler_p.h"
#include "qgsgeotransform.h"
#include "qgshighlightmaterial.h"
#include "qgsmaterial3dhandler.h"
#include "qgspoint3dbillboardmaterial.h"
#include "qgspoint3dsymbol.h"
#include "qgssourcecache.h"
#include "qgsvectorlayer.h"

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
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QPaintedTextureImage>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QSceneLoader>
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
  std::unique_ptr<QgsMaterial> material;

  if ( materialContext.isHighlighted() )
  {
    material = std::make_unique<QgsHighlightMaterial>( Qgis::MaterialRenderingTechnique::InstancedPoints );
  }
  else
  {
    Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey;
    filterKey->setName( u"renderingStyle"_s );
    filterKey->setValue( "forward" );

    Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram;

    const QByteArray vertexShaderCode = Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/instanced.vert"_s ) );
    QStringList defines;
    if ( hasDataDefinedScale )
      defines << u"USE_INSTANCE_SCALE"_s;
    if ( hasDataDefinedRotation )
      defines << u"USE_INSTANCE_ROTATION"_s;

    const QByteArray finalVertexShaderCode = Qgs3DUtils::addDefinesToShaderCode( vertexShaderCode, defines );
    shaderProgram->setVertexShaderCode( finalVertexShaderCode );
    shaderProgram->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/phong.frag"_s ) ) );

    Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass;
    renderPass->setShaderProgram( shaderProgram );

    Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique;
    technique->addFilterKey( filterKey );
    technique->addRenderPass( renderPass );
    technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
    technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
    technique->graphicsApiFilter()->setMajorVersion( 3 );
    technique->graphicsApiFilter()->setMinorVersion( 2 );

    Qt3DRender::QEffect *effect = new Qt3DRender::QEffect;
    effect->addTechnique( technique );

    Qgs3D::addMaterialParametersToEffect( effect, symbol->materialSettings(), materialContext );

    material = std::make_unique<QgsMaterial>();
    material->setEffect( effect );
  }

  return material.release();
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
      Qt3DExtras::QSphereGeometry *g = new Qt3DExtras::QSphereGeometry;
      g->setRadius( radius );
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
    static void addSceneEntities(
      const Qgs3DRenderContext &context,
      const QVector<QVector3D> &positions,
      const QVector<QVector3D> &scales,
      const QVector<QQuaternion> &rotations,
      const QgsVector3D &chunkOrigin,
      const QgsPoint3DSymbol *symbol,
      Qt3DCore::QEntity *parent
    );
    static void addMeshEntities(
      const Qgs3DRenderContext &context,
      const QVector<QVector3D> &positions,
      const QVector<QVector3D> &scales,
      const QVector<QQuaternion> &rotations,
      const QgsVector3D &chunkOrigin,
      const QgsPoint3DSymbol *symbol,
      Qt3DCore::QEntity *parent,
      bool areSelected,
      bool areHighlighted
    );
    static QgsGeoTransform *transform( QVector3D position, const QMatrix4x4 &transform, const QgsVector3D &chunkOrigin );

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

  QVector3D scale = mSymbolScale;
  const bool hasDDScale = ddp.isActive( QgsAbstract3DSymbol::Property::ScaleX ) || ddp.isActive( QgsAbstract3DSymbol::Property::ScaleY ) || ddp.isActive( QgsAbstract3DSymbol::Property::ScaleZ );
  if ( hasDDScale )
  {
    const double scaleX = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::ScaleX, context.expressionContext(), mSymbolScale.x() );
    const double scaleY = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::ScaleY, context.expressionContext(), mSymbolScale.y() );
    const double scaleZ = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::ScaleZ, context.expressionContext(), mSymbolScale.z() );
    scale = QVector3D( static_cast< float >( scaleX ), static_cast< float >( scaleY ), static_cast< float >( scaleZ ) );
  }

  out.scales.resize( out.positions.size() );
  QVector3D *outScale = out.scales.data() + oldSize;
  for ( std::size_t i = 0; i < added; ++i )
  {
    ( *outScale++ ) = scale;
  }

  QQuaternion rotation = mSymbolRotation;
  const bool hasDDRotation = ddp.isActive( QgsAbstract3DSymbol::Property::RotationX )
                             || ddp.isActive( QgsAbstract3DSymbol::Property::RotationY )
                             || ddp.isActive( QgsAbstract3DSymbol::Property::RotationZ );
  if ( hasDDRotation )
  {
    // extract default rotation components from symbol rotation
    const QVector3D baseEuler = mSymbolRotation.toEulerAngles();

    const double rotationX = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::RotationX, context.expressionContext(), baseEuler.x() );
    const double rotationY = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::RotationY, context.expressionContext(), baseEuler.y() );
    const double rotationZ = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::RotationZ, context.expressionContext(), baseEuler.z() );

    //... and then re-calculate the rotation vector for this feature
    rotation = QQuaternion::fromEulerAngles( static_cast< float >( rotationX ), static_cast< float >( rotationY ), static_cast< float >( rotationZ ) );
  }

  out.rotations.resize( out.positions.size() );
  QQuaternion *outRotation = out.rotations.data() + oldSize;
  for ( std::size_t i = 0; i < added; ++i )
  {
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

  if ( selected )
  {
    addMeshEntities( context, out.positions, out.scales, out.rotations, mChunkOrigin, mSymbol.get(), parent, true, mHighlightingEnabled );
  }
  else
  {
    //  "overwriteMaterial" is a legacy setting indicating that non-embedded material should be used
    if ( mSymbol->shapeProperty( u"overwriteMaterial"_s ).toBool() || ( mSymbol->materialSettings() && mSymbol->materialSettings()->type() != "null"_L1 ) || mHighlightingEnabled )
    {
      addMeshEntities( context, out.positions, out.scales, out.rotations, mChunkOrigin, mSymbol.get(), parent, false, mHighlightingEnabled );
    }
    else
    {
      addSceneEntities( context, out.positions, out.scales, out.rotations, mChunkOrigin, mSymbol.get(), parent );
    }
  }
}

QVector3D stringToAxis( const QString &axis )
{
  if ( axis == "x"_L1 )
    return QVector3D( 1.0f, 0.0f, 0.0f );
  if ( axis == "-x"_L1 )
    return QVector3D( -1.0f, 0.0f, 0.0f );
  if ( axis == "y"_L1 )
    return QVector3D( 0.0f, 1.0f, 0.0f );
  if ( axis == "-y"_L1 )
    return QVector3D( 0.0f, -1.0f, 0.0f );
  if ( axis == "z"_L1 )
    return QVector3D( 0.0f, 0.0f, 1.0f );
  if ( axis == "-z"_L1 )
    return QVector3D( 0.0f, 0.0f, -1.0f );

  return QVector3D();
}

QMatrix4x4 createZUpTransform( const QString &upAxis, const QString &forwardAxis )
{
  QVector3D up = stringToAxis( upAxis );
  QVector3D forward = stringToAxis( forwardAxis );

  if ( up.isNull() || forward.isNull() || std::abs( QVector3D::dotProduct( up, forward ) ) > 1e-6f )
  {
    // no transform (identity matrix) on error
    return QMatrix4x4();
  }

  QVector3D right = QVector3D::crossProduct( forward, up ).normalized();
  return QMatrix4x4( right.x(), right.y(), right.z(), 0.0f, forward.x(), forward.y(), forward.z(), 0.0f, up.x(), up.y(), up.z(), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f );
}

void QgsModelPoint3DSymbolHandler::addSceneEntities(
  const Qgs3DRenderContext &context,
  const QVector<QVector3D> &positions,
  const QVector<QVector3D> &scales,
  const QVector<QQuaternion> &rotations,
  const QgsVector3D &chunkOrigin,
  const QgsPoint3DSymbol *symbol,
  Qt3DCore::QEntity *parent
)
{
  Q_UNUSED( context );
  const QString source = QgsApplication::sourceCache()->localFilePath( symbol->shapeProperty( u"model"_s ).toString() );
  // if the source is remote, the Qgs3DMapScene will take care of refreshing this 3D symbol when the source is fetched

  const QString upAxis = symbol->shapeProperty( u"upAxis"_s ).toString();
  const QString forwardAxis = symbol->shapeProperty( u"forwardAxis"_s ).toString();
  const QMatrix4x4 zUpMatrix = createZUpTransform( upAxis, forwardAxis );

  if ( !source.isEmpty() )
  {
    int index = 0;
    for ( const QVector3D &position : positions )
    {
      // build the entity
      Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

      const QUrl url = QUrl::fromLocalFile( source );
      Qt3DRender::QSceneLoader *modelLoader = new Qt3DRender::QSceneLoader;
      modelLoader->setSource( url );

      QMatrix4x4 entityTransform;
      entityTransform.scale( scales.at( index ) );
      entityTransform.rotate( rotations.at( index ) );
      entityTransform *= zUpMatrix;

      entity->addComponent( modelLoader );
      entity->addComponent( transform( position, entityTransform, chunkOrigin ) );
      entity->setParent( parent );

      // cppcheck wrongly believes entity will leak
      // cppcheck-suppress memleak
      index++;
    }
  }
  else
  {
    QgsDebugMsgLevel( u"File '%1' is not accessible!"_s.arg( symbol->shapeProperty( u"model"_s ).toString() ), 1 );
  }
}

void QgsModelPoint3DSymbolHandler::addMeshEntities(
  const Qgs3DRenderContext &context,
  const QVector<QVector3D> &positions,
  const QVector<QVector3D> &scales,
  const QVector<QQuaternion> &rotations,
  const QgsVector3D &chunkOrigin,
  const QgsPoint3DSymbol *symbol,
  Qt3DCore::QEntity *parent,
  bool areSelected,
  bool areHighlighted
)
{
  if ( positions.empty() )
    return;

  const QString upAxis = symbol->shapeProperty( u"upAxis"_s ).toString();
  const QString forwardAxis = symbol->shapeProperty( u"forwardAxis"_s ).toString();
  const QMatrix4x4 zUpMatrix = createZUpTransform( upAxis, forwardAxis );

  const QString source = QgsApplication::sourceCache()->localFilePath( symbol->shapeProperty( u"model"_s ).toString() );
  if ( !source.isEmpty() )
  {
    // build the default material
    QgsMaterialContext materialContext = QgsMaterialContext::fromRenderContext( context );
    materialContext.setIsSelected( areSelected );
    materialContext.setIsHighlighted( areHighlighted );

    QgsMaterial *mat = Qgs3D::toMaterial( symbol->materialSettings(), Qgis::MaterialRenderingTechnique::Triangles, materialContext );
    if ( !mat )
      return;

    const QUrl url = QUrl::fromLocalFile( source );

    // get nodes
    int index = 0;
    for ( const QVector3D &position : positions )
    {
      // build the entity
      Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

      Qt3DRender::QMesh *mesh = new Qt3DRender::QMesh;
      mesh->setSource( url );

      entity->addComponent( mesh );
      entity->addComponent( mat );

      QMatrix4x4 entityTransform;
      entityTransform.scale( scales.at( index ) );
      entityTransform.rotate( rotations.at( index ) );
      entityTransform *= zUpMatrix;

      entity->addComponent( transform( position, entityTransform, chunkOrigin ) );
      entity->setParent( parent );

      // cppcheck wrongly believes entity will leak
      // cppcheck-suppress memleak
      index++;
    }
  }
  else
  {
    QgsDebugMsgLevel( u"File '%1' is not accessible!"_s.arg( symbol->shapeProperty( u"model"_s ).toString() ), 1 );
  }
}

QgsGeoTransform *QgsModelPoint3DSymbolHandler::transform( QVector3D position, const QMatrix4x4 &transform, const QgsVector3D &chunkOrigin )
{
  // position is relative to chunkOrigin
  QgsGeoTransform *tr = new QgsGeoTransform;
  tr->setMatrix( transform );
  tr->setGeoTranslation( chunkOrigin + position + tr->translation() );
  return tr;
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
