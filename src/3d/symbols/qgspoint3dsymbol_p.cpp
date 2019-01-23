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

#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
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

#include <Qt3DRender/QMesh>

#if QT_VERSION >= 0x050900
#include <Qt3DExtras/QExtrudedTextGeometry>
#endif

#include <QUrl>
#include <QVector3D>

#include "qgspoint3dsymbol.h"
#include "qgs3dmapsettings.h"

#include "qgsvectorlayer.h"
#include "qgspoint.h"
#include "qgs3dutils.h"

/// @cond PRIVATE


//* INSTANCED RENDERING *//


class QgsInstancedPoint3DSymbolHandler : public QgsFeature3DHandler
{
  public:
    QgsInstancedPoint3DSymbolHandler( const QgsPoint3DSymbol &symbol, const QgsFeatureIds &selectedIds )
      : mSymbol( symbol ), mSelectedIds( selectedIds ) {}

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames ) override;
    void processFeature( QgsFeature &feature, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:

    static Qt3DRender::QMaterial *material( const QgsPoint3DSymbol &symbol );
    static Qt3DRender::QGeometryRenderer *renderer( const QgsPoint3DSymbol &symbol, const QVector<QVector3D> &positions );
    static Qt3DRender::QGeometry *symbolGeometry( QgsPoint3DSymbol::Shape shape, const QVariantMap &shapeProperties );

    //! temporary data we will pass to the tessellator
    struct PointData
    {
      QVector<QVector3D> positions;  // contains triplets of float x,y,z for each point
    };

    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected );

    // input specific for this class
    const QgsPoint3DSymbol &mSymbol;
    // inputs - generic
    QgsFeatureIds mSelectedIds;

    // outputs
    PointData outNormal;  //!< Features that are not selected
    PointData outSelected;  //!< Features that are selected
};


bool QgsInstancedPoint3DSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames )
{
  Q_UNUSED( context );
  Q_UNUSED( attributeNames );
  return true;
}

void QgsInstancedPoint3DSymbolHandler::processFeature( QgsFeature &feature, const Qgs3DRenderContext &context )
{
  PointData &out = mSelectedIds.contains( feature.id() ) ? outSelected : outNormal;

  if ( feature.geometry().isNull() )
    return;

  Qgs3DUtils::extractPointPositions( feature, context.map(), mSymbol.altitudeClamping(), out.positions );
}

void QgsInstancedPoint3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
  makeEntity( parent, context, outSelected, true );
}

void QgsInstancedPoint3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected )
{
  // build the default material
  Qt3DRender::QMaterial *mat = material( mSymbol );

  if ( selected )
  {
    // update the material with selection colors
    for ( Qt3DRender::QParameter *param : mat->effect()->parameters() )
    {
      if ( param->name() == QLatin1String( "kd" ) ) // diffuse
        param->setValue( context.map().selectionColor() );
      else if ( param->name() == QLatin1String( "ka" ) ) // ambient
        param->setValue( context.map().selectionColor().darker() );
    }
  }

  // build the entity
  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;
  entity->addComponent( renderer( mSymbol, out.positions ) );
  entity->addComponent( mat );
  entity->setParent( parent );
}



Qt3DRender::QMaterial *QgsInstancedPoint3DSymbolHandler::material( const QgsPoint3DSymbol &symbol )
{
  Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey;
  filterKey->setName( QStringLiteral( "renderingStyle" ) );
  filterKey->setValue( "forward" );

  // the fragment shader implements a simplified version of phong shading that uses hardcoded light
  // (instead of whatever light we have defined in the scene)
  // TODO: use phong shading that respects lights from the scene
  Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram;
  shaderProgram->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/instanced.vert" ) ) ) );
  shaderProgram->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/instanced.frag" ) ) ) );

  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass;
  renderPass->setShaderProgram( shaderProgram );

  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique;
  technique->addFilterKey( filterKey );
  technique->addRenderPass( renderPass );
  technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
  technique->graphicsApiFilter()->setMajorVersion( 3 );
  technique->graphicsApiFilter()->setMinorVersion( 2 );

  Qt3DRender::QParameter *ambientParameter = new Qt3DRender::QParameter( QStringLiteral( "ka" ), QColor::fromRgbF( 0.05f, 0.05f, 0.05f, 1.0f ) );
  Qt3DRender::QParameter *diffuseParameter = new Qt3DRender::QParameter( QStringLiteral( "kd" ), QColor::fromRgbF( 0.7f, 0.7f, 0.7f, 1.0f ) );
  Qt3DRender::QParameter *specularParameter = new Qt3DRender::QParameter( QStringLiteral( "ks" ), QColor::fromRgbF( 0.01f, 0.01f, 0.01f, 1.0f ) );
  Qt3DRender::QParameter *shininessParameter = new Qt3DRender::QParameter( QStringLiteral( "shininess" ), 150.0f );

  diffuseParameter->setValue( symbol.material().diffuse() );
  ambientParameter->setValue( symbol.material().ambient() );
  specularParameter->setValue( symbol.material().specular() );
  shininessParameter->setValue( symbol.material().shininess() );

  QMatrix4x4 transformMatrix = symbol.transform();
  QMatrix3x3 normalMatrix = transformMatrix.normalMatrix();  // transponed inverse of 3x3 sub-matrix

  // QMatrix3x3 is not supported for passing to shaders, so we pass QMatrix4x4
  float *n = normalMatrix.data();
  QMatrix4x4 normalMatrix4(
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

  effect->addParameter( ambientParameter );
  effect->addParameter( diffuseParameter );
  effect->addParameter( specularParameter );
  effect->addParameter( shininessParameter );

  Qt3DRender::QMaterial *material = new Qt3DRender::QMaterial;
  material->setEffect( effect );

  return material;
}

Qt3DRender::QGeometryRenderer *QgsInstancedPoint3DSymbolHandler::renderer( const QgsPoint3DSymbol &symbol, const QVector<QVector3D> &positions )
{
  int count = positions.count();
  int byteCount = positions.count() * sizeof( QVector3D );
  QByteArray ba;
  ba.resize( byteCount );
  memcpy( ba.data(), positions.constData(), byteCount );

  Qt3DRender::QBuffer *instanceBuffer = new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer );
  instanceBuffer->setData( ba );

  Qt3DRender::QAttribute *instanceDataAttribute = new Qt3DRender::QAttribute;
  instanceDataAttribute->setName( QStringLiteral( "pos" ) );
  instanceDataAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  instanceDataAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  instanceDataAttribute->setVertexSize( 3 );
  instanceDataAttribute->setDivisor( 1 );
  instanceDataAttribute->setBuffer( instanceBuffer );
  instanceDataAttribute->setCount( count );
  instanceDataAttribute->setByteStride( 3 * sizeof( float ) );

  Qt3DRender::QGeometry *geometry = symbolGeometry( symbol.shape(), symbol.shapeProperties() );
  geometry->addAttribute( instanceDataAttribute );
  geometry->setBoundingVolumePositionAttribute( instanceDataAttribute );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setGeometry( geometry );
  renderer->setInstanceCount( count );

  return renderer;
}

Qt3DRender::QGeometry *QgsInstancedPoint3DSymbolHandler::symbolGeometry( QgsPoint3DSymbol::Shape shape, const QVariantMap &shapeProperties )
{
  switch ( shape )
  {
    case QgsPoint3DSymbol::Cylinder:
    {
      float radius = shapeProperties[QStringLiteral( "radius" )].toFloat();
      float length = shapeProperties[QStringLiteral( "length" )].toFloat();
      Qt3DExtras::QCylinderGeometry *g = new Qt3DExtras::QCylinderGeometry;
      //g->setRings(2);  // how many vertices vertically
      //g->setSlices(8); // how many vertices on circumference
      g->setRadius( radius ? radius : 10 );
      g->setLength( length ? length : 10 );
      return g;
    }

    case QgsPoint3DSymbol::Sphere:
    {
      float radius = shapeProperties[QStringLiteral( "radius" )].toFloat();
      Qt3DExtras::QSphereGeometry *g = new Qt3DExtras::QSphereGeometry;
      g->setRadius( radius ? radius : 10 );
      return g;
    }

    case QgsPoint3DSymbol::Cone:
    {
      float length = shapeProperties[QStringLiteral( "length" )].toFloat();
      float bottomRadius = shapeProperties[QStringLiteral( "bottomRadius" )].toFloat();
      float topRadius = shapeProperties[QStringLiteral( "topRadius" )].toFloat();
      Qt3DExtras::QConeGeometry *g = new Qt3DExtras::QConeGeometry;
      g->setLength( length ? length : 10 );
      g->setBottomRadius( bottomRadius );
      g->setTopRadius( topRadius );
      //g->setHasBottomEndcap(hasBottomEndcap);
      //g->setHasTopEndcap(hasTopEndcap);
      return g;
    }

    case QgsPoint3DSymbol::Cube:
    {
      float size = shapeProperties[QStringLiteral( "size" )].toFloat();
      Qt3DExtras::QCuboidGeometry *g = new Qt3DExtras::QCuboidGeometry;
      g->setXExtent( size ? size : 10 );
      g->setYExtent( size ? size : 10 );
      g->setZExtent( size ? size : 10 );
      return g;
    }

    case QgsPoint3DSymbol::Torus:
    {
      float radius = shapeProperties[QStringLiteral( "radius" )].toFloat();
      float minorRadius = shapeProperties[QStringLiteral( "minorRadius" )].toFloat();
      Qt3DExtras::QTorusGeometry *g = new Qt3DExtras::QTorusGeometry;
      g->setRadius( radius ? radius : 10 );
      g->setMinorRadius( minorRadius ? minorRadius : 5 );
      return g;
    }

    case QgsPoint3DSymbol::Plane:
    {
      float size = shapeProperties[QStringLiteral( "size" )].toFloat();
      Qt3DExtras::QPlaneGeometry *g = new Qt3DExtras::QPlaneGeometry;
      g->setWidth( size ? size : 10 );
      g->setHeight( size ? size : 10 );
      return g;
    }

#if QT_VERSION >= 0x050900
    case QgsPoint3DSymbol::ExtrudedText:
    {
      float depth = shapeProperties[QStringLiteral( "depth" )].toFloat();
      QString text = shapeProperties[QStringLiteral( "text" )].toString();
      Qt3DExtras::QExtrudedTextGeometry *g = new Qt3DExtras::QExtrudedTextGeometry;
      g->setDepth( depth ? depth : 1 );
      g->setText( text );
      return g;
    }
#endif

    default:
      Q_ASSERT( false );
      return nullptr;
  }
}

//* 3D MODEL RENDERING *//


class QgsModelPoint3DSymbolHandler : public QgsFeature3DHandler
{
  public:
    QgsModelPoint3DSymbolHandler( const QgsPoint3DSymbol &symbol, const QgsFeatureIds &selectedIds )
      : mSymbol( symbol ), mSelectedIds( selectedIds ) {}

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames ) override;
    void processFeature( QgsFeature &feature, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:

    static void addSceneEntities( const Qgs3DMapSettings &map, const QVector<QVector3D> &positions, const QgsPoint3DSymbol &symbol, Qt3DCore::QEntity *parent );
    static void addMeshEntities( const Qgs3DMapSettings &map, const QVector<QVector3D> &positions, const QgsPoint3DSymbol &symbol, Qt3DCore::QEntity *parent, bool are_selected );
    static Qt3DCore::QTransform *transform( QVector3D position, const QgsPoint3DSymbol &symbol );

    //! temporary data we will pass to the tessellator
    struct PointData
    {
      QVector<QVector3D> positions;  // contains triplets of float x,y,z for each point
    };

    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected );

    // input specific for this class
    const QgsPoint3DSymbol &mSymbol;
    // inputs - generic
    QgsFeatureIds mSelectedIds;

    // outputs
    PointData outNormal;  //!< Features that are not selected
    PointData outSelected;  //!< Features that are selected
};

bool QgsModelPoint3DSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames )
{
  Q_UNUSED( context );
  Q_UNUSED( attributeNames );
  return true;
}

void QgsModelPoint3DSymbolHandler::processFeature( QgsFeature &feature, const Qgs3DRenderContext &context )
{
  PointData &out = mSelectedIds.contains( feature.id() ) ? outSelected : outNormal;

  if ( feature.geometry().isNull() )
    return;

  Qgs3DUtils::extractPointPositions( feature, context.map(), mSymbol.altitudeClamping(), out.positions );
}

void QgsModelPoint3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
  makeEntity( parent, context, outSelected, true );
}

void QgsModelPoint3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected )
{
  if ( selected )
  {
    addMeshEntities( context.map(), out.positions, mSymbol, parent, true );
  }
  else
  {
    if ( mSymbol.shapeProperties()[QStringLiteral( "overwriteMaterial" )].toBool() )
    {
      addMeshEntities( context.map(), out.positions, mSymbol, parent, false );
    }
    else
    {
      addSceneEntities( context.map(), out.positions, mSymbol, parent );
    }
  }
}



void QgsModelPoint3DSymbolHandler::addSceneEntities( const Qgs3DMapSettings &map, const QVector<QVector3D> &positions, const QgsPoint3DSymbol &symbol, Qt3DCore::QEntity *parent )
{
  Q_UNUSED( map );
  for ( const QVector3D &position : positions )
  {
    // build the entity
    Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

    QUrl url = QUrl::fromLocalFile( symbol.shapeProperties()[QStringLiteral( "model" )].toString() );
    Qt3DRender::QSceneLoader *modelLoader = new Qt3DRender::QSceneLoader;
    modelLoader->setSource( url );

    entity->addComponent( modelLoader );
    entity->addComponent( transform( position, symbol ) );
    entity->setParent( parent );
  }
}

void QgsModelPoint3DSymbolHandler::addMeshEntities( const Qgs3DMapSettings &map, const QVector<QVector3D> &positions, const QgsPoint3DSymbol &symbol, Qt3DCore::QEntity *parent, bool are_selected )
{
  // build the default material
  Qt3DExtras::QPhongMaterial *mat = Qgs3DUtils::phongMaterial( symbol.material() );

  if ( are_selected )
  {
    mat->setDiffuse( map.selectionColor() );
    mat->setAmbient( map.selectionColor().darker() );
  }

  // get nodes
  for ( const QVector3D &position : positions )
  {
    // build the entity
    Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

    QUrl url = QUrl::fromLocalFile( symbol.shapeProperties()[QStringLiteral( "model" )].toString() );
    Qt3DRender::QMesh *mesh = new Qt3DRender::QMesh;
    mesh->setSource( url );

    entity->addComponent( mesh );
    entity->addComponent( mat );
    entity->addComponent( transform( position, symbol ) );
    entity->setParent( parent );
  }
}

Qt3DCore::QTransform *QgsModelPoint3DSymbolHandler::transform( QVector3D position, const QgsPoint3DSymbol &symbol )
{
  Qt3DCore::QTransform *tr = new Qt3DCore::QTransform;
  tr->setMatrix( symbol.transform() );
  tr->setTranslation( position + tr->translation() );
  return tr;
}

// --------------


namespace Qgs3DSymbolImpl
{

  QgsFeature3DHandler *handlerForPoint3DSymbol( QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol )
  {
    if ( symbol.shape() == QgsPoint3DSymbol::Model )
      return new QgsModelPoint3DSymbolHandler( symbol, layer->selectedFeatureIds() );
    else
      return new QgsInstancedPoint3DSymbolHandler( symbol, layer->selectedFeatureIds() );
  }

  Qt3DCore::QEntity *entityForPoint3DSymbol( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol )
  {
    QgsFeature3DHandler *handler = handlerForPoint3DSymbol( layer, symbol );
    Qt3DCore::QEntity *e = entityFromHandler( handler, map, layer );
    delete handler;
    return e;
  }
}

/// @endcond
