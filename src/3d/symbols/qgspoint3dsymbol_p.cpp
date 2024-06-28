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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometry>
#include <Qt3DCore/QTransform>

typedef Qt3DRender::QAttribute Qt3DQAttribute;
typedef Qt3DRender::QBuffer Qt3DQBuffer;
typedef Qt3DRender::QGeometry Qt3DQGeometry;
#else
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>

typedef Qt3DCore::QAttribute Qt3DQAttribute;
typedef Qt3DCore::QBuffer Qt3DQBuffer;
typedef Qt3DCore::QGeometry Qt3DQGeometry;
#endif

#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>

#include <Qt3DExtras/QCylinderGeometry>
#include <Qt3DExtras/QConeGeometry>
#include <Qt3DExtras/QCuboidGeometry>
#include <Qt3DExtras/QPlaneGeometry>
#include <Qt3DExtras/QSphereGeometry>
#include <Qt3DExtras/QTorusGeometry>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QSceneLoader>
#include <Qt3DRender/QPaintedTextureImage>

#include <Qt3DRender/QMesh>

#include <Qt3DExtras/QExtrudedTextGeometry>

#include <QUrl>
#include <QVector3D>

#include "qgspoint3dsymbol.h"
#include "qgs3dmapsettings.h"

#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgs3dutils.h"
#include "qgsbillboardgeometry.h"
#include "qgspoint3dbillboardmaterial.h"
#include "qgssourcecache.h"

/// @cond PRIVATE


//* INSTANCED RENDERING *//


class QgsInstancedPoint3DSymbolHandler : public QgsFeature3DHandler
{
  public:
    QgsInstancedPoint3DSymbolHandler( const QgsPoint3DSymbol *symbol, const QgsFeatureIds &selectedIds )
      : mSymbol( static_cast< QgsPoint3DSymbol *>( symbol->clone() ) )
      , mSelectedIds( selectedIds ) {}

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames ) override;
    void processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:

    //! temporary data we will pass to the tessellator
    class PointData
    {
      public:

        PointData() = default;

        float length = 0.0f;
        float radius = 0.0f;
        float bottomRadius = 0.0f;
        float minorRadius = 0.0f;
        float topRadius = 0.0f;
        float size = 0.0f;
        QVector<QVector3D> positions;

        bool operator==( const PointData &other ) const
        {
          return ( length == other.length && radius == other.radius
                   && bottomRadius == other.bottomRadius && minorRadius == other.minorRadius
                   && topRadius == other.topRadius && size == other.size );
        }
    };

    //!< Create a material from a point symbol and context. This is call by makeEntity().
    static Qt3DRender::QMaterial *material( const QgsPoint3DSymbol *symbol, const QgsMaterialContext &materialContext );
    //!< Create a geometry renderer from a point symbol, position and shape. This is call by makeEntity().
    static Qt3DRender::QGeometryRenderer *renderer( const QgsPoint3DSymbol *symbol, const PointData &pointData );
    //!< Create a geometry from a point symbol and shape. This is call by renderer().
    static Qt3DQGeometry *symbolGeometry( const QgsPoint3DSymbol *symbol, const PointData &pointData );

    //!< Create one entity per PointData. This is called by finalize().
    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, QVector<PointData> &out, bool selected );

    // input specific for this class
    std::unique_ptr< QgsPoint3DSymbol > mSymbol;
    // inputs - generic
    QgsFeatureIds mSelectedIds;

    // outputs
    QVector<PointData> outNormal;  //!< Features that are not selected
    QVector<PointData> outSelected;  //!< Features that are selected
};

bool QgsInstancedPoint3DSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames )
{
  QSet<QString> attrs = mSymbol->dataDefinedProperties().referencedFields( context.expressionContext() );
  attributeNames.unite( attrs );
  return true;
}

void QgsInstancedPoint3DSymbolHandler::processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context )
{
  QVector<PointData> &out = mSelectedIds.contains( feature.id() ) ? outSelected : outNormal;

  if ( feature.geometry().isNull() )
    return;

  const QgsPropertyCollection &ddp = mSymbol->dataDefinedProperties();
  const bool hasDDLength = ddp.isActive( QgsAbstract3DSymbol::Property::Length );
  float length = mSymbol->shapeProperty( QStringLiteral( "length" ) ).toFloat();
  if ( hasDDLength )
  {
    length = static_cast<float>( ddp.valueAsDouble( QgsAbstract3DSymbol::Property::Length, context.expressionContext(), length ) );
  }

  const bool hasDDRadius = ddp.isActive( QgsAbstract3DSymbol::Property::Radius );
  float radius = mSymbol->shapeProperty( QStringLiteral( "radius" ) ).toFloat();
  if ( hasDDRadius )
  {
    radius = static_cast<float>( ddp.valueAsDouble( QgsAbstract3DSymbol::Property::Radius, context.expressionContext(), radius ) );
  }

  const bool hasDDBottomRadius = ddp.isActive( QgsAbstract3DSymbol::Property::BottomRadius );
  float bottomRadius = mSymbol->shapeProperty( QStringLiteral( "bottomRadius" ) ).toFloat();
  if ( hasDDBottomRadius )
  {
    bottomRadius = static_cast<float>( ddp.valueAsDouble( QgsAbstract3DSymbol::Property::BottomRadius, context.expressionContext(), bottomRadius ) );
  }

  const bool hasDDMinorRadius = ddp.isActive( QgsAbstract3DSymbol::Property::MinorRadius );
  float minorRadius = mSymbol->shapeProperty( QStringLiteral( "minorRadius" ) ).toFloat();
  if ( hasDDMinorRadius )
  {
    minorRadius = static_cast<float>( ddp.valueAsDouble( QgsAbstract3DSymbol::Property::MinorRadius, context.expressionContext(), minorRadius ) );
  }

  const bool hasDDTopRadius = ddp.isActive( QgsAbstract3DSymbol::Property::TopRadius );
  float topRadius = mSymbol->shapeProperty( QStringLiteral( "topRadius" ) ).toFloat();
  if ( hasDDTopRadius )
  {
    topRadius = static_cast<float>( ddp.valueAsDouble( QgsAbstract3DSymbol::Property::TopRadius, context.expressionContext(), topRadius ) );
  }

  const bool hasDDSize = ddp.isActive( QgsAbstract3DSymbol::Property::Size );
  float size = mSymbol->shapeProperty( QStringLiteral( "size" ) ).toFloat();
  if ( hasDDSize )
  {
    size = static_cast<float>( ddp.valueAsDouble( QgsAbstract3DSymbol::Property::Size, context.expressionContext(), size ) );
  }

  QVector<QVector3D> positions;
  Qgs3DUtils::extractPointPositions( feature, context.map(), mSymbol->altitudeClamping(), positions );

  PointData newPointData;
  newPointData.length = length;
  newPointData.radius = radius;
  newPointData.bottomRadius = bottomRadius;
  newPointData.minorRadius = minorRadius;
  newPointData.topRadius = topRadius;
  newPointData.size = size;

  PointData *existingPointData = std::find( out.begin(), out.end(), newPointData );
  if ( existingPointData == out.end() )
  {
    newPointData.positions = QVector<QVector3D> {positions[0]};
    out.append( newPointData );
  }
  else
  {
    existingPointData->positions.append( positions[0] );
  }

  mFeatureCount++;
}

void QgsInstancedPoint3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
  makeEntity( parent, context, outSelected, true );

  for ( PointData &pointData : outNormal )
  {
    updateZRangeFromPositions( pointData.positions );
  }

  for ( PointData &pointData : outSelected )
  {
    updateZRangeFromPositions( pointData.positions );
  }

  // the elevation offset is applied in the vertex shader so let's account for it as well
  const float symbolOffset = mSymbol->transform().data()[13];

  // also account for the actual height of the objects themselves
  // NOTE -- these calculations are naive, and assume no rotation or scaling of the symbol!
  switch ( mSymbol->shape() )
  {
    case Qgis::Point3DShape::Cylinder:
    {
      float lengthMax = 0.0;
      for ( const PointData &pointData : outNormal )
      {
        if ( pointData.length > lengthMax )
          lengthMax = pointData.length;
      }

      for ( const PointData &pointData : outSelected )
      {
        if ( pointData.length > lengthMax )
          lengthMax = pointData.length;
      }

      mZMin -= lengthMax * 0.5f;
      mZMax += lengthMax * 0.5f;
      break;
    }

    case Qgis::Point3DShape::Sphere:
    {
      float radiusMax = 0.0;
      for ( const PointData &pointData : outNormal )
      {
        if ( pointData.radius > radiusMax )
          radiusMax = pointData.radius;
      }

      for ( const PointData &pointData : outSelected )
      {
        if ( pointData.radius > radiusMax )
          radiusMax = pointData.radius;
      }

      mZMin -= radiusMax;
      mZMax += radiusMax;
      break;
    }

    case Qgis::Point3DShape::Cone:
    {
      float lengthMax = 0.0;
      for ( const PointData &pointData : outNormal )
      {
        if ( pointData.length > lengthMax )
          lengthMax = pointData.length;
      }

      for ( const PointData &pointData : outSelected )
      {
        if ( pointData.length > lengthMax )
          lengthMax = pointData.length;
      }

      mZMin -= lengthMax * 0.5f;
      mZMax += lengthMax * 0.5f;
      break;
    }

    case Qgis::Point3DShape::Cube:
    {
      float sizeMax = 0.0;
      for ( const PointData &pointData : outNormal )
      {
        if ( pointData.size > sizeMax )
          sizeMax = pointData.size;
      }

      for ( const PointData &pointData : outSelected )
      {
        if ( pointData.size > sizeMax )
          sizeMax = pointData.size;
      }

      mZMin -= sizeMax * 0.5f;
      mZMax += sizeMax * 0.5f;
      break;
    }

    case Qgis::Point3DShape::Torus:
    {
      float radiusMax = 0.0;
      for ( const PointData &pointData : outNormal )
      {
        if ( pointData.radius > radiusMax )
          radiusMax = pointData.radius;
      }

      for ( const PointData &pointData : outSelected )
      {
        if ( pointData.radius > radiusMax )
          radiusMax = pointData.radius;
      }

      mZMin -= radiusMax;
      mZMax += radiusMax;
      break;
    }

    case Qgis::Point3DShape::Plane:
    {
      // worst case scenario -- even though planes are usually rotated so that they are flat,
      // let's account for possible overridden rotation
      float sizeMax = 0.0;
      for ( const PointData &pointData : outNormal )
      {
        if ( pointData.size > sizeMax )
          sizeMax = pointData.size;
      }

      for ( const PointData &pointData : outSelected )
      {
        if ( pointData.size > sizeMax )
          sizeMax = pointData.size;
      }

      mZMin -= sizeMax * 0.5f;
      mZMax += sizeMax * 0.5f;
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

void QgsInstancedPoint3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, QVector<PointData> &out, bool selected )
{
  // build the default material
  QgsMaterialContext materialContext;
  materialContext.setIsSelected( selected );
  materialContext.setSelectionColor( context.map().selectionColor() );
  Qt3DRender::QMaterial *mat = material( mSymbol.get(), materialContext );

  // build the entity
  for ( const PointData *pointData = out.cbegin(), *end = out.cend(); pointData != end; ++pointData )
  {
    Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;
    entity->addComponent( renderer( mSymbol.get(), *pointData ) );
    entity->addComponent( mat );
    entity->setParent( parent );
  }

// cppcheck wrongly believes entity will leak
// cppcheck-suppress memleak
}



Qt3DRender::QMaterial *QgsInstancedPoint3DSymbolHandler::material( const QgsPoint3DSymbol *symbol, const QgsMaterialContext &materialContext )
{
  Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey;
  filterKey->setName( QStringLiteral( "renderingStyle" ) );
  filterKey->setValue( "forward" );

  Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram;
  shaderProgram->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/instanced.vert" ) ) ) );
  shaderProgram->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/phongConstant.frag" ) ) ) );

  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass;
  renderPass->setShaderProgram( shaderProgram );

  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique;
  technique->addFilterKey( filterKey );
  technique->addRenderPass( renderPass );
  technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
  technique->graphicsApiFilter()->setMajorVersion( 3 );
  technique->graphicsApiFilter()->setMinorVersion( 2 );

  const QMatrix4x4 transformMatrix = symbol->transform();
  QMatrix3x3 normalMatrix = transformMatrix.normalMatrix();  // transponed inverse of 3x3 sub-matrix

  // QMatrix3x3 is not supported for passing to shaders, so we pass QMatrix4x4
  float *n = normalMatrix.data();
  const QMatrix4x4 normalMatrix4(
    n[0], n[3], n[6], 0,
    n[1], n[4], n[7], 0,
    n[2], n[5], n[8], 0,
    0, 0, 0, 0 );

  Qt3DRender::QParameter *paramInst = new Qt3DRender::QParameter;
  paramInst->setName( QStringLiteral( "inst" ) );
  paramInst->setValue( transformMatrix );

  Qt3DRender::QParameter *paramInstNormal = new Qt3DRender::QParameter;
  paramInstNormal->setName( QStringLiteral( "instNormal" ) );
  paramInstNormal->setValue( normalMatrix4 );

  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect;
  effect->addTechnique( technique );
  effect->addParameter( paramInst );
  effect->addParameter( paramInstNormal );

  symbol->materialSettings()->addParametersToEffect( effect, materialContext );

  Qt3DRender::QMaterial *material = new Qt3DRender::QMaterial;
  material->setEffect( effect );

  return material;
}

Qt3DRender::QGeometryRenderer *QgsInstancedPoint3DSymbolHandler::renderer( const QgsPoint3DSymbol *symbol, const PointData &pointData )
{
  const QVector<QVector3D> positions = pointData.positions;
  const int count = positions.count();
  const int byteCount = static_cast<int>( count * sizeof( QVector3D ) );
  QByteArray ba;
  ba.resize( byteCount );
  memcpy( ba.data(), positions.constData(), byteCount );

  Qt3DQBuffer *instanceBuffer = new Qt3DQBuffer();
  instanceBuffer->setData( ba );

  Qt3DQAttribute *instanceDataAttribute = new Qt3DQAttribute;
  instanceDataAttribute->setName( QStringLiteral( "pos" ) );
  instanceDataAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  instanceDataAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  instanceDataAttribute->setVertexSize( 3 );
  instanceDataAttribute->setByteOffset( 0 );
  instanceDataAttribute->setDivisor( 1 );
  instanceDataAttribute->setBuffer( instanceBuffer );
  instanceDataAttribute->setCount( count );
  instanceDataAttribute->setByteStride( 3 * sizeof( float ) );

  Qt3DQGeometry *geometry = symbolGeometry( symbol, pointData );
  geometry->addAttribute( instanceDataAttribute );
  geometry->setBoundingVolumePositionAttribute( instanceDataAttribute );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setGeometry( geometry );
  renderer->setInstanceCount( count );

  return renderer;
}

Qt3DQGeometry *QgsInstancedPoint3DSymbolHandler::symbolGeometry( const QgsPoint3DSymbol *symbol, const PointData &pointData )
{
  switch ( symbol->shape() )
  {
    case Qgis::Point3DShape::Cylinder:
    {
      Qt3DExtras::QCylinderGeometry *geometry = new Qt3DExtras::QCylinderGeometry;
      // geometry->setRings(2);  // how many vertices vertically
      // geometry->setSlices(8); // how many vertices on circumference
      geometry->setRadius( pointData.radius );
      geometry->setLength( pointData.length );
      return geometry;
    }

    case Qgis::Point3DShape::Sphere:
    {
      Qt3DExtras::QSphereGeometry *geometry = new Qt3DExtras::QSphereGeometry;
      geometry->setRadius( pointData.radius );
      return geometry;
    }

    case Qgis::Point3DShape::Cone:
    {
      Qt3DExtras::QConeGeometry *geometry = new Qt3DExtras::QConeGeometry;
      geometry->setLength( pointData.length );
      geometry->setBottomRadius( pointData.bottomRadius );
      geometry->setTopRadius( pointData.topRadius );
      // geometry->setHasBottomEndcap(hasBottomEndcap);
      // geometry->setHasTopEndcap(hasTopEndcap);
      return geometry;
    }

    case Qgis::Point3DShape::Cube:
    {
      Qt3DExtras::QCuboidGeometry *geometry = new Qt3DExtras::QCuboidGeometry;
      geometry->setXExtent( pointData.size );
      geometry->setYExtent( pointData.size );
      geometry->setZExtent( pointData.size );
      return geometry;
    }

    case Qgis::Point3DShape::Torus:
    {
      Qt3DExtras::QTorusGeometry *geometry = new Qt3DExtras::QTorusGeometry;
      geometry->setRadius( pointData.radius );
      geometry->setMinorRadius( pointData.minorRadius );
      return geometry;
    }

    case Qgis::Point3DShape::Plane:
    {
      Qt3DExtras::QPlaneGeometry *geometry = new Qt3DExtras::QPlaneGeometry;
      geometry->setWidth( pointData.size );
      geometry->setHeight( pointData.size );
      return geometry;
    }

    case Qgis::Point3DShape::ExtrudedText:
    {
      const float depth = symbol->shapeProperty( QStringLiteral( "depth" ) ).toFloat();
      const QString text = symbol->shapeProperty( QStringLiteral( "text" ) ).toString();
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
      : mSymbol( static_cast< QgsPoint3DSymbol * >( symbol->clone() ) )
      , mSelectedIds( selectedIds ) {}

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames ) override;
    void processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:

    static void addSceneEntities( const Qgs3DMapSettings &map, const QVector<QVector3D> &positions, const QgsPoint3DSymbol *symbol, Qt3DCore::QEntity *parent );
    static void addMeshEntities( const Qgs3DMapSettings &map, const QVector<QVector3D> &positions, const QgsPoint3DSymbol *symbol, Qt3DCore::QEntity *parent, bool are_selected );
    static Qt3DCore::QTransform *transform( QVector3D position, const QgsPoint3DSymbol *symbol );

    //! temporary data we will pass to the tessellator
    struct PointData
    {
      QVector<QVector3D> positions;  // contains triplets of float x,y,z for each point
    };

    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected );

    // input specific for this class
    std::unique_ptr< QgsPoint3DSymbol > mSymbol;
    // inputs - generic
    QgsFeatureIds mSelectedIds;

    // outputs
    PointData outNormal;  //!< Features that are not selected
    PointData outSelected;  //!< Features that are selected
};

bool QgsModelPoint3DSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames )
{
  Q_UNUSED( context )
  Q_UNUSED( attributeNames )
  return true;
}

void QgsModelPoint3DSymbolHandler::processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context )
{
  PointData &out = mSelectedIds.contains( feature.id() ) ? outSelected : outNormal;

  if ( feature.geometry().isNull() )
    return;

  Qgs3DUtils::extractPointPositions( feature, context.map(), mSymbol->altitudeClamping(), out.positions );
  mFeatureCount++;
}

void QgsModelPoint3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
  makeEntity( parent, context, outSelected, true );

  updateZRangeFromPositions( outNormal.positions );
  updateZRangeFromPositions( outSelected.positions );

  // the elevation offset is applied separately in QTransform added to sub-entities
  const float symbolHeight = mSymbol->transform().data()[13];
  mZMin += symbolHeight;
  mZMax += symbolHeight;
}

void QgsModelPoint3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected )
{
  if ( selected )
  {
    addMeshEntities( context.map(), out.positions, mSymbol.get(), parent, true );
  }
  else
  {
    //  "overwriteMaterial" is a legacy setting indicating that non-embedded material should be used
    if ( mSymbol->shapeProperty( QStringLiteral( "overwriteMaterial" ) ).toBool()
         || ( mSymbol->materialSettings() && mSymbol->materialSettings()->type() != QLatin1String( "null" ) ) )
    {
      addMeshEntities( context.map(), out.positions, mSymbol.get(), parent, false );
    }
    else
    {
      addSceneEntities( context.map(), out.positions, mSymbol.get(), parent );
    }
  }
}



void QgsModelPoint3DSymbolHandler::addSceneEntities( const Qgs3DMapSettings &map, const QVector<QVector3D> &positions, const QgsPoint3DSymbol *symbol, Qt3DCore::QEntity *parent )
{
  Q_UNUSED( map )
  for ( const QVector3D &position : positions )
  {
    const QString source = QgsApplication::sourceCache()->localFilePath( symbol->shapeProperty( QStringLiteral( "model" ) ).toString() );
    // if the source is remote, the Qgs3DMapScene will take care of refreshing this 3D symbol when the source is fetched
    if ( !source.isEmpty() )
    {
      // build the entity
      Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

      const QUrl url = QUrl::fromLocalFile( source );
      Qt3DRender::QSceneLoader *modelLoader = new Qt3DRender::QSceneLoader;
      modelLoader->setSource( url );

      entity->addComponent( modelLoader );
      entity->addComponent( transform( position, symbol ) );
      entity->setParent( parent );

// cppcheck wrongly believes entity will leak
// cppcheck-suppress memleak
    }
  }
}

void QgsModelPoint3DSymbolHandler::addMeshEntities( const Qgs3DMapSettings &map, const QVector<QVector3D> &positions, const QgsPoint3DSymbol *symbol, Qt3DCore::QEntity *parent, bool are_selected )
{
  if ( positions.empty() )
    return;

  // build the default material
  QgsMaterialContext materialContext;
  materialContext.setIsSelected( are_selected );
  materialContext.setSelectionColor( map.selectionColor() );
  Qt3DRender::QMaterial *mat = symbol->materialSettings()->toMaterial( QgsMaterialSettingsRenderingTechnique::Triangles, materialContext );

  // get nodes
  for ( const QVector3D &position : positions )
  {
    const QString source = QgsApplication::sourceCache()->localFilePath( symbol->shapeProperty( QStringLiteral( "model" ) ).toString() );
    if ( !source.isEmpty() )
    {
      // build the entity
      Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

      const QUrl url = QUrl::fromLocalFile( source );
      Qt3DRender::QMesh *mesh = new Qt3DRender::QMesh;
      mesh->setSource( url );

      entity->addComponent( mesh );
      entity->addComponent( mat );
      entity->addComponent( transform( position, symbol ) );
      entity->setParent( parent );

// cppcheck wrongly believes entity will leak
// cppcheck-suppress memleak
    }
  }
}

Qt3DCore::QTransform *QgsModelPoint3DSymbolHandler::transform( QVector3D position, const QgsPoint3DSymbol *symbol )
{
  Qt3DCore::QTransform *tr = new Qt3DCore::QTransform;
  tr->setMatrix( symbol->transform() );
  tr->setTranslation( position + tr->translation() );
  return tr;
}

// --------------

//* BILLBOARD RENDERING *//

class QgsPoint3DBillboardSymbolHandler : public QgsFeature3DHandler
{
  public:
    QgsPoint3DBillboardSymbolHandler( const QgsPoint3DSymbol *symbol, const QgsFeatureIds &selectedIds )
      : mSymbol( static_cast< QgsPoint3DSymbol * >( symbol->clone() ) )
      , mSelectedIds( selectedIds ) {}

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames ) override;
    void processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:

    //! temporary data we will pass to the tessellator
    struct PointData
    {
      QVector<QVector3D> positions;  // contains triplets of float x,y,z for each point
    };

    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected );

    // input specific for this class
    std::unique_ptr< QgsPoint3DSymbol > mSymbol;
    // inputs - generic
    QgsFeatureIds mSelectedIds;

    // outputs
    PointData outNormal;  //!< Features that are not selected
    PointData outSelected;  //!< Features that are selected
};

bool QgsPoint3DBillboardSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames )
{
  Q_UNUSED( context )
  Q_UNUSED( attributeNames )
  return true;
}

void QgsPoint3DBillboardSymbolHandler::processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context )
{
  PointData &out = mSelectedIds.contains( feature.id() ) ? outSelected : outNormal;

  if ( feature.geometry().isNull() )
    return;

  Qgs3DUtils::extractPointPositions( feature, context.map(), mSymbol->altitudeClamping(), out.positions );
  mFeatureCount++;
}

void QgsPoint3DBillboardSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
  makeEntity( parent, context, outSelected, true );

  updateZRangeFromPositions( outNormal.positions );
  updateZRangeFromPositions( outSelected.positions );

  // the elevation offset is applied externally through a QTransform of QEntity so let's account for it
  const float billboardHeight = mSymbol->transform().data()[13];
  mZMin += billboardHeight;
  mZMax += billboardHeight;
}

void QgsPoint3DBillboardSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected )
{
  // Billboard Geometry
  QgsBillboardGeometry *billboardGeometry = new QgsBillboardGeometry();
  billboardGeometry->setPoints( out.positions );

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
    billboardMaterial->setTexture2DFromSymbol( symbol, context.map(), selected );
  }
  else
  {
    billboardMaterial->useDefaultSymbol( context.map(), selected );
  }

  // Billboard Transform
  Qt3DCore::QTransform *billboardTransform = new Qt3DCore::QTransform();
  billboardTransform->setMatrix( mSymbol->billboardTransform() );

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
    const QgsPoint3DSymbol *pointSymbol = dynamic_cast< const QgsPoint3DSymbol * >( symbol );
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
}

/// @endcond
