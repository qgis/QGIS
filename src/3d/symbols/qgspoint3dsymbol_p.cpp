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

QgsPoint3DSymbolEntity::QgsPoint3DSymbolEntity( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  if ( symbol.shape() == QgsPoint3DSymbol::Model )
  {
    QgsPoint3DSymbolModelEntityFactory::addEntitiesForSelectedPoints( map, layer, symbol, this );
    QgsPoint3DSymbolModelEntityFactory::addEntitiesForNotSelectedPoints( map, layer, symbol, this );
  }
  else
  {
    QgsPoint3DSymbolInstancedEntityFactory::addEntityForNotSelectedPoints( map, layer, symbol, this );
    QgsPoint3DSymbolInstancedEntityFactory::addEntityForSelectedPoints( map, layer, symbol, this );
  }
}

//* INSTANCED RENDERING *//

Qt3DRender::QMaterial *QgsPoint3DSymbolInstancedEntityFactory::material( const QgsPoint3DSymbol &symbol )
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

void QgsPoint3DSymbolInstancedEntityFactory::addEntityForSelectedPoints( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, QgsPoint3DSymbolEntity *parent )
{
  // build the default material
  Qt3DRender::QMaterial *mat = material( symbol );

  // update the material with selection colors
  Q_FOREACH ( Qt3DRender::QParameter *param, mat->effect()->parameters() )
  {
    if ( param->name() == QLatin1String( "kd" ) ) // diffuse
      param->setValue( map.selectionColor() );
    else if ( param->name() == QLatin1String( "ka" ) ) // ambient
      param->setValue( map.selectionColor().darker() );
  }

  // build the feature request to select features
  QgsFeatureRequest req;
  req.setDestinationCrs( map.crs(), map.transformContext() );
  req.setFilterFids( layer->selectedFeatureIds() );
  req.setNoAttributes();

  // build the entity
  QgsPoint3DSymbolInstancedEntityNode *entity = new QgsPoint3DSymbolInstancedEntityNode( map, layer, symbol, req );
  entity->addComponent( mat );
  entity->setParent( parent );
}

void QgsPoint3DSymbolInstancedEntityFactory::addEntityForNotSelectedPoints( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, QgsPoint3DSymbolEntity *parent )
{
  // build the default material
  Qt3DRender::QMaterial *mat = material( symbol );

  // build the feature request to select features
  QgsFeatureRequest req;
  req.setDestinationCrs( map.crs(), map.transformContext() );
  req.setNoAttributes();

  QgsFeatureIds notSelected = layer->allFeatureIds();
  notSelected.subtract( layer->selectedFeatureIds() );
  req.setFilterFids( notSelected );

  // build the entity
  QgsPoint3DSymbolInstancedEntityNode *entity = new QgsPoint3DSymbolInstancedEntityNode( map, layer, symbol, req );
  entity->addComponent( mat );
  entity->setParent( parent );
}

QgsPoint3DSymbolInstancedEntityNode::QgsPoint3DSymbolInstancedEntityNode( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, const QgsFeatureRequest &req, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  QList<QVector3D> pos = Qgs3DUtils::positions( map, layer, req, symbol.altitudeClamping() );
  addComponent( renderer( symbol, pos ) );
}

Qt3DRender::QGeometryRenderer *QgsPoint3DSymbolInstancedEntityNode::renderer( const QgsPoint3DSymbol &symbol, const QList<QVector3D> &positions ) const
{
  int count = positions.count();

  QByteArray ba;
  ba.resize( count * sizeof( QVector3D ) );
  QVector3D *posData = reinterpret_cast<QVector3D *>( ba.data() );
  for ( int j = 0; j < count; ++j )
  {
    *posData = positions[j];
    ++posData;
  }

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

Qt3DRender::QGeometry *QgsPoint3DSymbolInstancedEntityNode::symbolGeometry( QgsPoint3DSymbol::Shape shape, const QVariantMap &shapeProperties ) const
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

static Qt3DExtras::QPhongMaterial *phongMaterial( const QgsPoint3DSymbol &symbol )
{
  Qt3DExtras::QPhongMaterial *phong = new Qt3DExtras::QPhongMaterial;

  phong->setAmbient( symbol.material().ambient() );
  phong->setDiffuse( symbol.material().diffuse() );
  phong->setSpecular( symbol.material().specular() );
  phong->setShininess( symbol.material().shininess() );

  return phong;
}

void QgsPoint3DSymbolModelEntityFactory::addEntitiesForSelectedPoints( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, QgsPoint3DSymbolEntity *parent )
{
  QgsFeatureRequest req;
  req.setDestinationCrs( map.crs(), map.transformContext() );
  req.setNoAttributes();
  req.setFilterFids( layer->selectedFeatureIds() );

  addMeshEntities( map, layer, req, symbol, parent, true );
}



void QgsPoint3DSymbolModelEntityFactory::addEntitiesForNotSelectedPoints( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, QgsPoint3DSymbolEntity *parent )
{
  // build the feature request to select features
  QgsFeatureRequest req;
  req.setDestinationCrs( map.crs(), map.transformContext() );
  req.setNoAttributes();
  QgsFeatureIds notSelected = layer->allFeatureIds();
  notSelected.subtract( layer->selectedFeatureIds() );
  req.setFilterFids( notSelected );

  if ( symbol.shapeProperties()[QStringLiteral( "overwriteMaterial" )].toBool() )
  {
    addMeshEntities( map, layer, req, symbol, parent, false );
  }
  else
  {
    addSceneEntities( map, layer, req, symbol, parent );
  }
}

void QgsPoint3DSymbolModelEntityFactory::addSceneEntities( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsFeatureRequest &req, const QgsPoint3DSymbol &symbol, QgsPoint3DSymbolEntity *parent )
{
  QList<QVector3D> positions = Qgs3DUtils::positions( map, layer, req, symbol.altitudeClamping() );
  Q_FOREACH ( const QVector3D &position, positions )
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

void QgsPoint3DSymbolModelEntityFactory::addMeshEntities( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsFeatureRequest &req, const QgsPoint3DSymbol &symbol, QgsPoint3DSymbolEntity *parent, bool are_selected )
{
  // build the default material
  Qt3DExtras::QPhongMaterial *mat = phongMaterial( symbol );

  if ( are_selected )
  {
    mat->setDiffuse( map.selectionColor() );
    mat->setAmbient( map.selectionColor().darker() );
  }

  // get nodes
  QList<QVector3D> positions = Qgs3DUtils::positions( map, layer, req, symbol.altitudeClamping() );
  Q_FOREACH ( const QVector3D &position, positions )
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

Qt3DCore::QTransform *QgsPoint3DSymbolModelEntityFactory::transform( QVector3D position, const QgsPoint3DSymbol &symbol )
{
  Qt3DCore::QTransform *tr = new Qt3DCore::QTransform;
  tr->setMatrix( symbol.transform() );
  tr->setTranslation( position + tr->translation() );
  return tr;
}

/// @endcond
