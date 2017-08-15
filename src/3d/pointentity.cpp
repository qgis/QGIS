#include "pointentity.h"

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
#include "map3d.h"
#include "terraingenerator.h"

#include "qgsvectorlayer.h"
#include "qgspoint.h"
#include "utils.h"


PointEntity::PointEntity( const Map3D &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  if ( symbol.shapeProperties["shape"].toString() == "model" )
  {
    Model3DPointEntityFactory::addEntitiesForSelectedPoints( map, layer, symbol, this );
    Model3DPointEntityFactory::addEntitiesForNotSelectedPoints( map, layer, symbol, this );
  }
  else
  {
    InstancedPointEntityFactory::addEntityForNotSelectedPoints( map, layer, symbol, this );
    InstancedPointEntityFactory::addEntityForSelectedPoints( map, layer, symbol, this );
  }
}

//* INSTANCED RENDERING *//

Qt3DRender::QMaterial *InstancedPointEntityFactory::material( const QgsPoint3DSymbol &symbol )
{
  Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey;
  filterKey->setName( "renderingStyle" );
  filterKey->setValue( "forward" );

  // the fragment shader implements a simplified version of phong shading that uses hardcoded light
  // (instead of whatever light we have defined in the scene)
  // TODO: use phong shading that respects lights from the scene
  Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram;
  shaderProgram->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( "qrc:/shaders/instanced.vert" ) ) );
  shaderProgram->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( "qrc:/shaders/instanced.frag" ) ) );

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

  diffuseParameter->setValue( symbol.material.diffuse() );
  ambientParameter->setValue( symbol.material.ambient() );
  specularParameter->setValue( symbol.material.specular() );
  shininessParameter->setValue( symbol.material.shininess() );

  QMatrix4x4 transformMatrix = symbol.transform;
  QMatrix3x3 normalMatrix = transformMatrix.normalMatrix();  // transponed inverse of 3x3 sub-matrix

  // QMatrix3x3 is not supported for passing to shaders, so we pass QMatrix4x4
  float *n = normalMatrix.data();
  QMatrix4x4 normalMatrix4(
    n[0], n[3], n[6], 0,
    n[1], n[4], n[7], 0,
    n[2], n[5], n[8], 0,
    0, 0, 0, 0 );

  Qt3DRender::QParameter *paramInst = new Qt3DRender::QParameter;
  paramInst->setName( "inst" );
  paramInst->setValue( transformMatrix );

  Qt3DRender::QParameter *paramInstNormal = new Qt3DRender::QParameter;
  paramInstNormal->setName( "instNormal" );
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

void InstancedPointEntityFactory::addEntityForSelectedPoints( const Map3D &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, PointEntity *parent )
{
  // build the default material
  Qt3DRender::QMaterial *mat = material( symbol );

  // update the material with selection colors
  Q_FOREACH ( Qt3DRender::QParameter *param, mat->effect()->parameters() )
  {
    if ( param->name() == "kd" ) // diffuse
      param->setValue( map.selectionColor() );
    else if ( param->name() == "ka" ) // ambient
      param->setValue( map.selectionColor().darker() );
  }

  // build the feature request to select features
  QgsFeatureRequest req;
  req.setDestinationCrs( map.crs );
  req.setFilterFids( layer->selectedFeatureIds() );

  // build the entity
  InstancedPointEntityNode *entity = new InstancedPointEntityNode( map, layer, symbol, req );
  entity->addComponent( mat );
  entity->setParent( parent );
}

void InstancedPointEntityFactory::addEntityForNotSelectedPoints( const Map3D &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, PointEntity *parent )
{
  // build the default material
  Qt3DRender::QMaterial *mat = material( symbol );

  // build the feature request to select features
  QgsFeatureRequest req;
  req.setDestinationCrs( map.crs );

  QgsFeatureIds notSelected = layer->allFeatureIds();
  notSelected.subtract( layer->selectedFeatureIds() );
  req.setFilterFids( notSelected );

  // build the entity
  InstancedPointEntityNode *entity = new InstancedPointEntityNode( map, layer, symbol, req );
  entity->addComponent( mat );
  entity->setParent( parent );
}

InstancedPointEntityNode::InstancedPointEntityNode( const Map3D &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, const QgsFeatureRequest &req, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  QList<QVector3D> pos = Utils::positions( map, layer, req );
  addComponent( renderer( symbol, pos ) );
}

Qt3DRender::QGeometryRenderer *InstancedPointEntityNode::renderer( const QgsPoint3DSymbol &symbol, const QList<QVector3D> &positions ) const
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
  instanceDataAttribute->setName( "pos" );
  instanceDataAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  instanceDataAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  instanceDataAttribute->setVertexSize( 3 );
  instanceDataAttribute->setDivisor( 1 );
  instanceDataAttribute->setBuffer( instanceBuffer );
  instanceDataAttribute->setCount( count );
  instanceDataAttribute->setByteStride( 3 * sizeof( float ) );

  Qt3DRender::QGeometry *geometry = nullptr;
  QString shape = symbol.shapeProperties["shape"].toString();
  if ( shape == "sphere" )
  {
    float radius = symbol.shapeProperties["radius"].toFloat();
    Qt3DExtras::QSphereGeometry *g = new Qt3DExtras::QSphereGeometry;
    g->setRadius( radius ? radius : 10 );
    geometry = g;
  }
  else if ( shape == "cone" )
  {
    float length = symbol.shapeProperties["length"].toFloat();
    float bottomRadius = symbol.shapeProperties["bottomRadius"].toFloat();
    float topRadius = symbol.shapeProperties["topRadius"].toFloat();
    Qt3DExtras::QConeGeometry *g = new Qt3DExtras::QConeGeometry;
    g->setLength( length ? length : 10 );
    g->setBottomRadius( bottomRadius );
    g->setTopRadius( topRadius );
    //g->setHasBottomEndcap(hasBottomEndcap);
    //g->setHasTopEndcap(hasTopEndcap);
    geometry = g;
  }
  else if ( shape == "cube" )
  {
    float size = symbol.shapeProperties["size"].toFloat();
    Qt3DExtras::QCuboidGeometry *g = new Qt3DExtras::QCuboidGeometry;
    g->setXExtent( size ? size : 10 );
    g->setYExtent( size ? size : 10 );
    g->setZExtent( size ? size : 10 );
    geometry = g;
  }
  else if ( shape == "torus" )
  {
    float radius = symbol.shapeProperties["radius"].toFloat();
    float minorRadius = symbol.shapeProperties["minorRadius"].toFloat();
    Qt3DExtras::QTorusGeometry *g = new Qt3DExtras::QTorusGeometry;
    g->setRadius( radius ? radius : 10 );
    g->setMinorRadius( minorRadius ? minorRadius : 5 );
    geometry = g;
  }
  else if ( shape == "plane" )
  {
    float size = symbol.shapeProperties["size"].toFloat();
    Qt3DExtras::QPlaneGeometry *g = new Qt3DExtras::QPlaneGeometry;
    g->setWidth( size ? size : 10 );
    g->setHeight( size ? size : 10 );
    geometry = g;
  }
#if QT_VERSION >= 0x050900
  else if ( shape == "extrudedText" )
  {
    float depth = symbol.shapeProperties["depth"].toFloat();
    QString text = symbol.shapeProperties["text"].toString();
    Qt3DExtras::QExtrudedTextGeometry *g = new Qt3DExtras::QExtrudedTextGeometry;
    g->setDepth( depth ? depth : 1 );
    g->setText( text );
    geometry = g;
  }
#endif
  else  // shape == "cylinder" or anything else
  {
    float radius = symbol.shapeProperties["radius"].toFloat();
    float length = symbol.shapeProperties["length"].toFloat();
    Qt3DExtras::QCylinderGeometry *g = new Qt3DExtras::QCylinderGeometry;
    //g->setRings(2);  // how many vertices vertically
    //g->setSlices(8); // how many vertices on circumference
    g->setRadius( radius ? radius : 10 );
    g->setLength( length ? length : 10 );
    geometry = g;
  }

  geometry->addAttribute( instanceDataAttribute );
  geometry->setBoundingVolumePositionAttribute( instanceDataAttribute );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setGeometry( geometry );
  renderer->setInstanceCount( count );

  return renderer;
}

//* 3D MODEL RENDERING *//

static Qt3DExtras::QPhongMaterial *phongMaterial( const QgsPoint3DSymbol &symbol )
{
  Qt3DExtras::QPhongMaterial *phong = new Qt3DExtras::QPhongMaterial;

  phong->setAmbient( symbol.material.ambient() );
  phong->setDiffuse( symbol.material.diffuse() );
  phong->setSpecular( symbol.material.specular() );
  phong->setShininess( symbol.material.shininess() );

  return phong;
}

void Model3DPointEntityFactory::addEntitiesForSelectedPoints( const Map3D &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, PointEntity *parent )
{
  QgsFeatureRequest req;
  req.setDestinationCrs( map.crs );
  req.setFilterFids( layer->selectedFeatureIds() );

  addMeshEntities( map, layer, req, symbol, parent, true );
}



void Model3DPointEntityFactory::addEntitiesForNotSelectedPoints( const Map3D &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, PointEntity *parent )
{
  // build the feature request to select features
  QgsFeatureRequest req;
  req.setDestinationCrs( map.crs );
  QgsFeatureIds notSelected = layer->allFeatureIds();
  notSelected.subtract( layer->selectedFeatureIds() );
  req.setFilterFids( notSelected );

  if ( symbol.shapeProperties["overwriteMaterial"].toBool() )
  {
    addMeshEntities( map, layer, req, symbol, parent, false );
  }
  else
  {
    addSceneEntities( map, layer, req, symbol, parent );
  }
}

void Model3DPointEntityFactory::addSceneEntities( const Map3D &map, QgsVectorLayer *layer, const QgsFeatureRequest &req, const QgsPoint3DSymbol &symbol, PointEntity *parent )
{
  QList<QVector3D> positions = Utils::positions( map, layer, req );
  Q_FOREACH ( const QVector3D &position, positions )
  {
    // build the entity
    Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

    QUrl url = QUrl::fromLocalFile( symbol.shapeProperties["model"].toString() );
    Qt3DRender::QSceneLoader *modelLoader = new Qt3DRender::QSceneLoader;
    modelLoader->setSource( url );

    entity->addComponent( modelLoader );
    entity->addComponent( transform( position, symbol ) );
    entity->setParent( parent );
  }
}

void Model3DPointEntityFactory::addMeshEntities( const Map3D &map, QgsVectorLayer *layer, const QgsFeatureRequest &req, const QgsPoint3DSymbol &symbol, PointEntity *parent, bool are_selected )
{
  // build the default material
  Qt3DExtras::QPhongMaterial *mat = phongMaterial( symbol );

  if ( are_selected )
  {
    mat->setDiffuse( map.selectionColor() );
    mat->setAmbient( map.selectionColor().darker() );
  }

  // get nodes
  QList<QVector3D> positions = Utils::positions( map, layer, req );
  Q_FOREACH ( const QVector3D &position, positions )
  {
    // build the entity
    Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

    QUrl url = QUrl::fromLocalFile( symbol.shapeProperties["model"].toString() );
    Qt3DRender::QMesh *mesh = new Qt3DRender::QMesh;
    mesh->setSource( url );

    entity->addComponent( mesh );
    entity->addComponent( mat );
    entity->addComponent( transform( position, symbol ) );
    entity->setParent( parent );
  }
}

Qt3DCore::QTransform *Model3DPointEntityFactory::transform( const QVector3D &position, const QgsPoint3DSymbol &symbol )
{
  Qt3DCore::QTransform *tr = new Qt3DCore::QTransform;
  tr->setMatrix( symbol.transform );
  tr->setTranslation( position + tr->translation() );
  return tr;
}
