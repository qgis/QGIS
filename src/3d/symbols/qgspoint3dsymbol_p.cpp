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

#include "qgs3dutils.h"
#include "qgsapplication.h"
#include "qgsbillboardgeometry.h"
#include "qgsgeotransform.h"
#include "qgspoint3dbillboardmaterial.h"
#include "qgspoint3dsymbol.h"
#include "qgssourcecache.h"
#include "qgsvectorlayer.h"

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

/// @cond PRIVATE


//* INSTANCED RENDERING *//


class QgsInstancedPoint3DSymbolHandler : public QgsFeature3DHandler
{
  public:
    QgsInstancedPoint3DSymbolHandler( const QgsPoint3DSymbol *symbol, const QgsFeatureIds &selectedIds )
      : mSymbol( static_cast<QgsPoint3DSymbol *>( symbol->clone() ) )
      , mSelectedIds( selectedIds ) {}

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, const QgsVector3D &chunkOrigin ) override;
    void processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:
    static QgsMaterial *material( const QgsPoint3DSymbol *symbol, const QgsMaterialContext &materialContext );
    static Qt3DRender::QGeometryRenderer *renderer( const QgsPoint3DSymbol *symbol, const QVector<QVector3D> &positions );
    static Qt3DCore::QGeometry *symbolGeometry( const QgsPoint3DSymbol *symbol );

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

    //! origin (in the map coordinates) for output geometries (e.g. at the center of the chunk)
    QgsVector3D mChunkOrigin;

    // outputs
    PointData outNormal;   //!< Features that are not selected
    PointData outSelected; //!< Features that are selected
};


bool QgsInstancedPoint3DSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, const QgsVector3D &chunkOrigin )
{
  Q_UNUSED( context )
  Q_UNUSED( attributeNames )

  mChunkOrigin = chunkOrigin;

  return true;
}

void QgsInstancedPoint3DSymbolHandler::processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context )
{
  PointData &out = mSelectedIds.contains( feature.id() ) ? outSelected : outNormal;

  if ( feature.geometry().isNull() )
    return;

  Qgs3DUtils::extractPointPositions( feature, context, mChunkOrigin, mSymbol->altitudeClamping(), out.positions );
  mFeatureCount++;
}

void QgsInstancedPoint3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
  makeEntity( parent, context, outSelected, true );

  updateZRangeFromPositions( outNormal.positions );
  updateZRangeFromPositions( outSelected.positions );

  // the elevation offset is applied in the vertex shader so let's account for it as well
  const float symbolOffset = mSymbol->transform().data()[14];

  // also account for the actual height of the objects themselves
  // NOTE -- these calculations are naive, and assume no rotation or scaling of the symbol!
  switch ( mSymbol->shape() )
  {
    case Qgis::Point3DShape::Cylinder:
    {
      const float length = mSymbol->shapeProperty( u"length"_s ).toFloat();
      mZMin -= length * 0.5f;
      mZMax += length * 0.5f;
      break;
    }

    case Qgis::Point3DShape::Sphere:
    {
      const float radius = mSymbol->shapeProperty( u"radius"_s ).toFloat();
      mZMin -= radius;
      mZMax += radius;
      break;
    }

    case Qgis::Point3DShape::Cone:
    {
      const float length = mSymbol->shapeProperty( u"length"_s ).toFloat();
      mZMin -= length * 0.5f;
      mZMax += length * 0.5f;
      break;
    }

    case Qgis::Point3DShape::Cube:
    {
      const float size = mSymbol->shapeProperty( u"size"_s ).toFloat();
      mZMin -= size * 0.5f;
      mZMax += size * 0.5f;
      break;
    }

    case Qgis::Point3DShape::Torus:
    {
      const float radius = mSymbol->shapeProperty( u"radius"_s ).toFloat();
      mZMin -= radius;
      mZMax += radius;
      break;
    }

    case Qgis::Point3DShape::Plane:
    {
      // worst case scenario -- even though planes are usually rotated so that they are flat,
      // let's account for possible overridden rotation
      const float size = mSymbol->shapeProperty( u"size"_s ).toFloat();
      mZMin -= size * 0.5f;
      mZMax += size * 0.5f;
      break;
    }

    case Qgis::Point3DShape::ExtrudedText:
    case Qgis::Point3DShape::Model:
    case Qgis::Point3DShape::Billboard:
      break;
  }

  mZMin += symbolOffset;
  mZMax += symbolOffset;
}

void QgsInstancedPoint3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected )
{
  if ( out.positions.isEmpty() )
  {
    return; // nothing to show - no need to create the entity
  }

  // build the default material
  QgsMaterialContext materialContext;
  materialContext.setIsSelected( selected );
  materialContext.setSelectionColor( context.selectionColor() );
  QgsMaterial *mat = material( mSymbol.get(), materialContext );

  // add transform (our geometry has coordinates relative to mChunkOrigin)
  QgsGeoTransform *tr = new QgsGeoTransform;
  tr->setGeoTranslation( mChunkOrigin );

  // build the entity
  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;
  entity->addComponent( renderer( mSymbol.get(), out.positions ) );
  entity->addComponent( mat );
  entity->addComponent( tr );
  entity->setParent( parent );

  // cppcheck wrongly believes entity will leak
  // cppcheck-suppress memleak
}


QgsMaterial *QgsInstancedPoint3DSymbolHandler::material( const QgsPoint3DSymbol *symbol, const QgsMaterialContext &materialContext )
{
  Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey;
  filterKey->setName( u"renderingStyle"_s );
  filterKey->setValue( "forward" );

  Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram;
  shaderProgram->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/instanced.vert"_s ) ) );
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

  const QMatrix4x4 tempTransformMatrix = symbol->transform();
  // our built-in 3D geometries (e.g. cylinder, plane, ...) assume Y axis going "up",
  // let's rotate them by default so that their Z axis goes "up" (like the rest of the scene)
  QMatrix4x4 id;
  id.rotate( QQuaternion::fromAxisAndAngle( QVector3D( 1, 0, 0 ), 90 ) );
  const QMatrix4x4 transformMatrix = tempTransformMatrix * id;

  // transponed inverse of 3x3 sub-matrix
  QMatrix3x3 normalMatrix = transformMatrix.normalMatrix();

  // QMatrix3x3 is not supported for passing to shaders, so we pass QMatrix4x4
  float *n = normalMatrix.data();
  const QMatrix4x4 normalMatrix4(
    n[0], n[3], n[6], 0,
    n[1], n[4], n[7], 0,
    n[2], n[5], n[8], 0,
    0, 0, 0, 0
  );

  Qt3DRender::QParameter *paramInst = new Qt3DRender::QParameter;
  paramInst->setName( u"inst"_s );
  paramInst->setValue( transformMatrix );

  Qt3DRender::QParameter *paramInstNormal = new Qt3DRender::QParameter;
  paramInstNormal->setName( u"instNormal"_s );
  paramInstNormal->setValue( normalMatrix4 );

  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect;
  effect->addTechnique( technique );
  effect->addParameter( paramInst );
  effect->addParameter( paramInstNormal );

  symbol->materialSettings()->addParametersToEffect( effect, materialContext );

  QgsMaterial *material = new QgsMaterial;
  material->setEffect( effect );

  return material;
}

Qt3DRender::QGeometryRenderer *QgsInstancedPoint3DSymbolHandler::renderer( const QgsPoint3DSymbol *symbol, const QVector<QVector3D> &positions )
{
  const int count = positions.count();
  const int byteCount = positions.count() * sizeof( QVector3D );
  QByteArray ba;
  ba.resize( byteCount );
  memcpy( ba.data(), positions.constData(), byteCount );

  Qt3DCore::QBuffer *instanceBuffer = new Qt3DCore::QBuffer();
  instanceBuffer->setData( ba );

  Qt3DCore::QAttribute *instanceDataAttribute = new Qt3DCore::QAttribute;
  instanceDataAttribute->setName( u"pos"_s );
  instanceDataAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  instanceDataAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  instanceDataAttribute->setVertexSize( 3 );
  instanceDataAttribute->setByteOffset( 0 );
  instanceDataAttribute->setDivisor( 1 );
  instanceDataAttribute->setBuffer( instanceBuffer );
  instanceDataAttribute->setCount( count );
  instanceDataAttribute->setByteStride( 3 * sizeof( float ) );

  Qt3DCore::QGeometry *geometry = symbolGeometry( symbol );
  geometry->addAttribute( instanceDataAttribute );
  geometry->setBoundingVolumePositionAttribute( instanceDataAttribute );

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
      , mSelectedIds( selectedIds ) {}

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, const QgsVector3D &chunkOrigin ) override;
    void processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:
    static void addSceneEntities( const Qgs3DRenderContext &context, const QVector<QVector3D> &positions, const QgsVector3D &chunkOrigin, const QgsPoint3DSymbol *symbol, Qt3DCore::QEntity *parent );
    static void addMeshEntities( const Qgs3DRenderContext &context, const QVector<QVector3D> &positions, const QgsVector3D &chunkOrigin, const QgsPoint3DSymbol *symbol, Qt3DCore::QEntity *parent, bool are_selected );
    static QgsGeoTransform *transform( QVector3D position, const QgsPoint3DSymbol *symbol, const QgsVector3D &chunkOrigin );

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

    //! origin (in the map coordinates) for output geometries (e.g. at the center of the chunk)
    QgsVector3D mChunkOrigin;

    // outputs
    PointData outNormal;   //!< Features that are not selected
    PointData outSelected; //!< Features that are selected
};

bool QgsModelPoint3DSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, const QgsVector3D &chunkOrigin )
{
  Q_UNUSED( context )
  Q_UNUSED( attributeNames )

  mChunkOrigin = chunkOrigin;

  return true;
}

void QgsModelPoint3DSymbolHandler::processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context )
{
  PointData &out = mSelectedIds.contains( feature.id() ) ? outSelected : outNormal;

  if ( feature.geometry().isNull() )
    return;

  Qgs3DUtils::extractPointPositions( feature, context, mChunkOrigin, mSymbol->altitudeClamping(), out.positions );
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
  mZMin += symbolHeight;
  mZMax += symbolHeight;
}

void QgsModelPoint3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected )
{
  if ( out.positions.isEmpty() )
  {
    return; // nothing to show - no need to create the entity
  }

  if ( selected )
  {
    addMeshEntities( context, out.positions, mChunkOrigin, mSymbol.get(), parent, true );
  }
  else
  {
    //  "overwriteMaterial" is a legacy setting indicating that non-embedded material should be used
    if ( mSymbol->shapeProperty( u"overwriteMaterial"_s ).toBool()
         || ( mSymbol->materialSettings() && mSymbol->materialSettings()->type() != "null"_L1 ) )
    {
      addMeshEntities( context, out.positions, mChunkOrigin, mSymbol.get(), parent, false );
    }
    else
    {
      addSceneEntities( context, out.positions, mChunkOrigin, mSymbol.get(), parent );
    }
  }
}


void QgsModelPoint3DSymbolHandler::addSceneEntities( const Qgs3DRenderContext &context, const QVector<QVector3D> &positions, const QgsVector3D &chunkOrigin, const QgsPoint3DSymbol *symbol, Qt3DCore::QEntity *parent )
{
  Q_UNUSED( context );
  const QString source = QgsApplication::sourceCache()->localFilePath( symbol->shapeProperty( u"model"_s ).toString() );
  // if the source is remote, the Qgs3DMapScene will take care of refreshing this 3D symbol when the source is fetched
  if ( !source.isEmpty() )
  {
    for ( const QVector3D &position : positions )
    {
      // build the entity
      Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

      const QUrl url = QUrl::fromLocalFile( source );
      Qt3DRender::QSceneLoader *modelLoader = new Qt3DRender::QSceneLoader;
      modelLoader->setSource( url );

      entity->addComponent( modelLoader );
      entity->addComponent( transform( position, symbol, chunkOrigin ) );
      entity->setParent( parent );

      // cppcheck wrongly believes entity will leak
      // cppcheck-suppress memleak
    }
  }
  else
  {
    QgsDebugMsgLevel( u"File '%1' is not accessible!"_s.arg( symbol->shapeProperty( u"model"_s ).toString() ), 1 );
  }
}

void QgsModelPoint3DSymbolHandler::addMeshEntities( const Qgs3DRenderContext &context, const QVector<QVector3D> &positions, const QgsVector3D &chunkOrigin, const QgsPoint3DSymbol *symbol, Qt3DCore::QEntity *parent, bool are_selected )
{
  if ( positions.empty() )
    return;

  const QString source = QgsApplication::sourceCache()->localFilePath( symbol->shapeProperty( u"model"_s ).toString() );
  if ( !source.isEmpty() )
  {
    // build the default material
    QgsMaterialContext materialContext;
    materialContext.setIsSelected( are_selected );
    materialContext.setSelectionColor( context.selectionColor() );
    QgsMaterial *mat = symbol->materialSettings()->toMaterial( QgsMaterialSettingsRenderingTechnique::Triangles, materialContext );

    const QUrl url = QUrl::fromLocalFile( source );

    // get nodes
    for ( const QVector3D &position : positions )
    {
      // build the entity
      Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

      Qt3DRender::QMesh *mesh = new Qt3DRender::QMesh;
      mesh->setSource( url );

      entity->addComponent( mesh );
      entity->addComponent( mat );
      entity->addComponent( transform( position, symbol, chunkOrigin ) );
      entity->setParent( parent );

      // cppcheck wrongly believes entity will leak
      // cppcheck-suppress memleak
    }
  }
  else
  {
    QgsDebugMsgLevel( u"File '%1' is not accessible!"_s.arg( symbol->shapeProperty( u"model"_s ).toString() ), 1 );
  }
}

QgsGeoTransform *QgsModelPoint3DSymbolHandler::transform( QVector3D position, const QgsPoint3DSymbol *symbol, const QgsVector3D &chunkOrigin )
{
  // position is relative to chunkOrigin
  QgsGeoTransform *tr = new QgsGeoTransform;
  tr->setMatrix( symbol->transform() );
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
      , mSelectedIds( selectedIds ) {}

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, const QgsVector3D &chunkOrigin ) override;
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

    //! origin (in the map coordinates) for output geometries (e.g. at the center of the chunk)
    QgsVector3D mChunkOrigin;

    // outputs
    PointData outNormal;   //!< Features that are not selected
    PointData outSelected; //!< Features that are selected
};

bool QgsPoint3DBillboardSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, const QgsVector3D &chunkOrigin )
{
  Q_UNUSED( context )
  Q_UNUSED( attributeNames )

  mChunkOrigin = chunkOrigin;

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
  mZMin += billboardHeight;
  mZMax += billboardHeight;
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

  QgsFeature3DHandler *handlerForPoint3DSymbol( QgsVectorLayer *layer, const QgsAbstract3DSymbol *symbol )
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
