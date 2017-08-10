#include "scene.h"

#include <Qt3DRender/QCamera>
#include <Qt3DRender/QPickingSettings>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/QSkyboxEntity>
#include <Qt3DLogic/QFrameAction>

#include <QTimer>

#include "aabb.h"
#include "qgsabstract3drenderer.h"
#include "cameracontroller.h"
#include "chunknode.h"
#include "qgsvectorlayer.h"
#include "map3d.h"
#include "terrain.h"
#include "terraingenerator.h"
//#include "testchunkloader.h"
#include "chunkedentity.h"
#include "utils.h"

#include <Qt3DRender/QMesh>
#include <Qt3DRender/QSceneLoader>
#include <Qt3DExtras/QPhongMaterial>


Scene::Scene( const Map3D &map, Qt3DExtras::QForwardRenderer *defaultFrameGraph, Qt3DRender::QRenderSettings *renderSettings, Qt3DRender::QCamera *camera, const QRect &viewportRect, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
  , mMap( map )
  , mTerrain( nullptr )
  , mForwardRenderer( defaultFrameGraph )
{

  connect( &map, &Map3D::backgroundColorChanged, this, &Scene::onBackgroundColorChanged );
  onBackgroundColorChanged();

  // TODO: strange - setting OnDemand render policy still keeps QGIS busy (Qt 5.9.0)
  // actually it is more busy than with the default "Always" policy although there are no changes in the scene.
  //renderSettings->setRenderPolicy( Qt3DRender::QRenderSettings::OnDemand );

#if QT_VERSION >= 0x050900
  // we want precise picking of terrain (also bounding volume picking does not seem to work - not sure why)
  renderSettings->pickingSettings()->setPickMethod( Qt3DRender::QPickingSettings::TrianglePicking );
#endif

  // Camera
  float aspectRatio = ( float )viewportRect.width() / viewportRect.height();
  camera->lens()->setPerspectiveProjection( 45.0f, aspectRatio, 10.f, 10000.0f );

  mFrameAction = new Qt3DLogic::QFrameAction();
  connect( mFrameAction, &Qt3DLogic::QFrameAction::triggered,
           this, &Scene::onFrameTriggered );
  addComponent( mFrameAction ); // takes ownership

  // Camera controlling
  mCameraController = new CameraController( this ); // attaches to the scene
  mCameraController->setViewport( viewportRect );
  mCameraController->setCamera( camera );
  mCameraController->resetView( 1000 );

  // create terrain entity

  createTerrain();
  connect( &map, &Map3D::terrainGeneratorChanged, this, &Scene::createTerrain );
  connect( &map, &Map3D::terrainVerticalScaleChanged, this, &Scene::createTerrain );
  connect( &map, &Map3D::mapTileResolutionChanged, this, &Scene::createTerrain );
  connect( &map, &Map3D::maxTerrainScreenErrorChanged, this, &Scene::createTerrain );
  connect( &map, &Map3D::maxTerrainGroundErrorChanged, this, &Scene::createTerrain );

  // create entities of renderers

  Q_FOREACH ( const QgsAbstract3DRenderer *renderer, map.renderers )
  {
    Qt3DCore::QEntity *newEntity = renderer->createEntity( map );
    newEntity->setParent( this );
  }

  // listen to changes of layers in order to add/remove 3D renderer entities
  connect( &map, &Map3D::layersChanged, this, &Scene::onLayersChanged );

  Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity;
  Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform;
  lightTransform->setTranslation( QVector3D( 0, 1000, 0 ) );
  // defaults: white, intensity 0.5
  // attenuation: constant 1.0, linear 0.0, quadratic 0.0
  Qt3DRender::QPointLight *light = new Qt3DRender::QPointLight;
  light->setConstantAttenuation( 0 );
  //light->setColor(Qt::white);
  //light->setIntensity(0.5);
  lightEntity->addComponent( light );
  lightEntity->addComponent( lightTransform );
  lightEntity->setParent( this );


#if 0
  ChunkedEntity *testChunkEntity = new ChunkedEntity( AABB( -500, 0, -500, 500, 100, 500 ), 2.f, 3.f, 7, new TestChunkLoaderFactory );
  testChunkEntity->setEnabled( false );
  testChunkEntity->setParent( this );
  chunkEntities << testChunkEntity;
#endif

  connect( mCameraController, &CameraController::cameraChanged, this, &Scene::onCameraChanged );
  connect( mCameraController, &CameraController::viewportChanged, this, &Scene::onCameraChanged );

#if 0
  // experiments with loading of existing 3D models.

  // scene loader only gets loaded only when added to a scene...
  // it loads everything: geometries, materials, transforms, lights, cameras (if any)
  Qt3DCore::QEntity *loaderEntity = new Qt3DCore::QEntity;
  Qt3DRender::QSceneLoader *loader = new Qt3DRender::QSceneLoader;
  loader->setSource( QUrl( "file:///home/martin/Downloads/LowPolyModels/tree.dae" ) );
  loaderEntity->addComponent( loader );
  loaderEntity->setParent( this );

  // mesh loads just geometry as one geometry...
  // so if there are different materials (e.g. colors) used in the model, this information is lost
  Qt3DCore::QEntity *meshEntity = new Qt3DCore::QEntity;
  Qt3DRender::QMesh *mesh = new Qt3DRender::QMesh;
  mesh->setSource( QUrl( "file:///home/martin/Downloads/LowPolyModels/tree.obj" ) );
  meshEntity->addComponent( mesh );
  Qt3DExtras::QPhongMaterial *material = new Qt3DExtras::QPhongMaterial;
  material->setAmbient( Qt::red );
  meshEntity->addComponent( material );
  Qt3DCore::QTransform *meshTransform = new Qt3DCore::QTransform;
  meshTransform->setScale( 1 );
  meshEntity->addComponent( meshTransform );
  meshEntity->setParent( this );
#endif

  if ( map.skybox )
  {
    Qt3DExtras::QSkyboxEntity *skybox = new Qt3DExtras::QSkyboxEntity;
    skybox->setBaseName( map.skyboxFileBase );
    skybox->setExtension( map.skyboxFileExtension );
    skybox->setParent( this );

    // docs say frustum culling must be disabled for skybox.
    // it _somehow_ works even when frustum culling is enabled with some camera positions,
    // but then when zoomed in more it would disappear - so let's keep frustum culling disabled
    defaultFrameGraph->setFrustumCullingEnabled( false );
  }

  // force initial update of chunked entities
  onCameraChanged();
}

void Scene::viewZoomFull()
{
  QgsRectangle extent = mMap.terrainGenerator()->extent();
  float side = qMax( extent.width(), extent.height() );
  mCameraController->resetView( side );  // assuming FOV being 45 degrees
}

SceneState _sceneState( CameraController *cameraController )
{
  Qt3DRender::QCamera *camera = cameraController->camera();
  SceneState state;
  state.cameraFov = camera->fieldOfView();
  state.cameraPos = camera->position();
  QRect rect = cameraController->viewport();
  state.screenSizePx = qMax( rect.width(), rect.height() ); // TODO: is this correct?
  state.viewProjectionMatrix = camera->projectionMatrix() * camera->viewMatrix();
  return state;
}

void Scene::onCameraChanged()
{
  Q_FOREACH ( ChunkedEntity *entity, chunkEntities )
  {
    if ( entity->isEnabled() )
      entity->update( _sceneState( mCameraController ) );
  }

  // Update near and far plane from the terrain.
  // this needs to be done with great care as we have kind of circular dependency here:
  // active nodes are culled based on the current frustum (which involves near + far plane)
  // and then based on active nodes we set near and far plane.
  //
  // All of this is just heuristics assuming that all other stuff is being rendered somewhere
  // around the area where the terrain is.
  //
  // Near/far plane is setup in order to make best use of the depth buffer to avoid:
  // 1. precision errors - if the range is too great
  // 2. unwanted clipping of scene - if the range is too small

  if ( mTerrain )
  {
    Qt3DRender::QCamera *camera = cameraController()->camera();
    QMatrix4x4 viewMatrix = camera->viewMatrix();
    float near = 1e9;
    float far = 0;

    QList<ChunkNode *> activeNodes = mTerrain->getActiveNodes();

    // it could be that there are no active nodes - they could be all culled or because root node
    // is not yet loaded - we still need at least something to understand bounds of our scene
    // so lets use the root node
    if ( activeNodes.isEmpty() )
      activeNodes << mTerrain->getRootNode();

    Q_FOREACH ( ChunkNode *node, activeNodes )
    {
      // project each corner of bbox to camera coordinates
      // and determine closest and farthest point.
      AABB bbox = node->bbox;
      for ( int i = 0; i < 8; ++i )
      {
        QVector4D p( ( ( i >> 0 ) & 1 ) ? bbox.xMin : bbox.xMax,
                     ( ( i >> 1 ) & 1 ) ? bbox.yMin : bbox.yMax,
                     ( ( i >> 2 ) & 1 ) ? bbox.zMin : bbox.zMax, 1 );
        QVector4D pc = viewMatrix * p;

        float dst = -pc.z();  // in camera coordinates, x grows right, y grows down, z grows to the back
        if ( dst < near )
          near = dst;
        if ( dst > far )
          far = dst;
      }
    }
    if ( near < 1 )
      near = 1;  // does not really make sense to use negative far plane (behind camera)

    if ( near == 1e9 && far == 0 )
    {
      // the update didn't work out... this should not happen
      // well at least temprarily use some conservative starting values
      qDebug() << "oops... this should not happen! couldn't determine near/far plane. defaulting to 1...1e9";
      near = 1;
      far = 1e9;
    }

    // set near/far plane - with some tolerance in front/behind expected near/far planes
    camera->setFarPlane( far * 2 );
    camera->setNearPlane( near / 2 );
  }
  else
    qDebug() << "no terrain - not setting near/far plane";

  //qDebug() << "camera near/far" << mCameraController->camera()->nearPlane() << mCameraController->camera()->farPlane();
}

void Scene::onFrameTriggered( float dt )
{
  mCameraController->frameTriggered( dt );

  Q_FOREACH ( ChunkedEntity *entity, chunkEntities )
  {
    if ( entity->isEnabled() && entity->needsUpdate )
    {
      qDebug() << "need for update";
      entity->update( _sceneState( mCameraController ) );
    }
  }
}

void Scene::createTerrain()
{
  if ( mTerrain )
  {
    chunkEntities.removeOne( mTerrain );

    mTerrain->deleteLater();
    mTerrain = nullptr;
  }

  if ( !mTerrainUpdateScheduled )
  {
    // defer re-creation of terrain: there may be multiple invocations of this slot, so create the new entity just once
    QTimer::singleShot( 0, this, &Scene::createTerrainDeferred );
    mTerrainUpdateScheduled = true;
  }
}

void Scene::createTerrainDeferred()
{
  double tile0width = mMap.terrainGenerator()->extent().width();
  int maxZoomLevel = Utils::maxZoomLevel( tile0width, mMap.mapTileResolution(), mMap.maxTerrainGroundError() );

  mTerrain = new Terrain( maxZoomLevel, mMap );
  //mTerrain->setEnabled(false);
  mTerrain->setParent( this );

  if ( mMap.showTerrainBoundingBoxes() )
    mTerrain->setShowBoundingBoxes( true );

  mCameraController->addTerrainPicker( mTerrain->terrainPicker() );

  chunkEntities << mTerrain;

  onCameraChanged();  // force update of the new terrain

  // make sure that renderers for layers are re-created as well
  Q_FOREACH ( QgsMapLayer *layer, mMap.layers() )
  {
    // remove old entity - if any
    removeLayerEntity( layer );

    // add new entity - if any 3D renderer
    addLayerEntity( layer );
  }

  mTerrainUpdateScheduled = false;
}

void Scene::onBackgroundColorChanged()
{
  mForwardRenderer->setClearColor( mMap.backgroundColor() );
}

void Scene::onLayerRenderer3DChanged()
{
  QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sender() );
  Q_ASSERT( layer );

  // remove old entity - if any
  removeLayerEntity( layer );

  // add new entity - if any 3D renderer
  addLayerEntity( layer );
}

void Scene::onLayersChanged()
{
  QSet<QgsMapLayer *> layersBefore = QSet<QgsMapLayer *>::fromList( mLayerEntities.keys() );
  QList<QgsMapLayer *> layersAdded;
  Q_FOREACH ( QgsMapLayer *layer, mMap.layers() )
  {
    if ( !layersBefore.contains( layer ) )
    {
      layersAdded << layer;
    }
    else
    {
      layersBefore.remove( layer );
    }
  }

  // what is left in layersBefore are layers that have been removed
  Q_FOREACH ( QgsMapLayer *layer, layersBefore )
  {
    removeLayerEntity( layer );
  }

  Q_FOREACH ( QgsMapLayer *layer, layersAdded )
  {
    addLayerEntity( layer );
  }
}

void Scene::addLayerEntity( QgsMapLayer *layer )
{
  QgsAbstract3DRenderer *renderer = layer->renderer3D();
  if ( renderer )
  {
    Qt3DCore::QEntity *newEntity = renderer->createEntity( mMap );
    newEntity->setParent( this );
    mLayerEntities.insert( layer, newEntity );
  }

  connect( layer, &QgsMapLayer::renderer3DChanged, this, &Scene::onLayerRenderer3DChanged );

  if ( layer->type() == QgsMapLayer::VectorLayer )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
    connect( vlayer, &QgsVectorLayer::selectionChanged, this, &Scene::onLayerRenderer3DChanged );
  }
}

void Scene::removeLayerEntity( QgsMapLayer *layer )
{
  Qt3DCore::QEntity *entity = mLayerEntities.take( layer );
  if ( entity )
    entity->deleteLater();

  disconnect( layer, &QgsMapLayer::renderer3DChanged, this, &Scene::onLayerRenderer3DChanged );

  if ( layer->type() == QgsMapLayer::VectorLayer )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
    disconnect( vlayer, &QgsVectorLayer::selectionChanged, this, &Scene::onLayerRenderer3DChanged );
  }
}
