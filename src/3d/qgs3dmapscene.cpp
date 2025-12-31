/***************************************************************************
  qgs3dmapscene.cpp
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

#include "qgs3dmapscene.h"

#include "qgs3daxis.h"
#include "qgs3dmapexportsettings.h"
#include "qgs3dmapsettings.h"
#include "qgs3dsceneexporter.h"
#include "qgs3dutils.h"
#include "qgsaabb.h"
#include "qgsabstract3dengine.h"
#include "qgsabstract3drenderer.h"
#include "qgsabstractterrainsettings.h"
#include "qgsambientocclusionrenderview.h"
#include "qgsannotationlayer.h"
#include "qgsannotationlayer3drenderer.h"
#include "qgsapplication.h"
#include "qgscameracontroller.h"
#include "qgschunkedentity.h"
#include "qgschunknode.h"
#include "qgsdirectionallightsettings.h"
#include "qgseventtracing.h"
#include "qgsforwardrenderview.h"
#include "qgsframegraph.h"
#include "qgsgeotransform.h"
#include "qgsglobechunkedentity.h"
#include "qgslinematerial_p.h"
#include "qgslogger.h"
#include "qgsmaplayerelevationproperties.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgsmaterial.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayer3drenderer.h"
#include "qgsmessageoutput.h"
#include "qgspoint3dbillboardmaterial.h"
#include "qgspoint3dsymbol.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudlayer3drenderer.h"
#include "qgspostprocessingentity.h"
#include "qgsrulebased3drenderer.h"
#include "qgsskyboxentity.h"
#include "qgsskyboxsettings.h"
#include "qgssourcecache.h"
#include "qgsterrainentity.h"
#include "qgsterraingenerator.h"
#include "qgstiledscenelayer.h"
#include "qgstiledscenelayer3drenderer.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayer3drenderer.h"
#include "qgswindow3dengine.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSurface>
#include <QTimer>
#include <QUrl>
#include <Qt3DExtras/QDiffuseSpecularMaterial>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/QPhongAlphaMaterial>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DLogic/QFrameAction>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DRender/QRenderState>
#include <Qt3DRender/QSceneLoader>
#include <Qt3DRender/QTechnique>
#include <QtMath>

#include "moc_qgs3dmapscene.cpp"

std::function<QMap<QString, Qgs3DMapScene *>()> Qgs3DMapScene::sOpenScenesFunction = [] { return QMap<QString, Qgs3DMapScene *>(); };

Qgs3DMapScene::Qgs3DMapScene( Qgs3DMapSettings &map, QgsAbstract3DEngine *engine )
  : mMap( map )
  , mEngine( engine )
{
  connect( &map, &Qgs3DMapSettings::backgroundColorChanged, this, &Qgs3DMapScene::onBackgroundColorChanged );
  onBackgroundColorChanged();

  // The default render policy in Qt3D is "Always" - i.e. the 3D map scene gets refreshed up to 60 fps
  // even if there's no change. Switching to "on demand" should only re-render when something has changed
  // and we save quite a lot of resources
  mEngine->renderSettings()->setRenderPolicy( Qt3DRender::QRenderSettings::OnDemand );

  QRect viewportRect( QPoint( 0, 0 ), mEngine->size() );

  // Get the maximum of clip planes available
  mMaxClipPlanes = Qgs3DUtils::openGlMaxClipPlanes( mEngine->surface() );

  // Camera
  float aspectRatio = ( float ) viewportRect.width() / viewportRect.height();
  mEngine->camera()->lens()->setPerspectiveProjection( mMap.fieldOfView(), aspectRatio, 10.f, 10000.0f );

  mFrameAction = new Qt3DLogic::QFrameAction();
  connect( mFrameAction, &Qt3DLogic::QFrameAction::triggered, this, &Qgs3DMapScene::onFrameTriggered );
  addComponent( mFrameAction ); // takes ownership

  // Camera controlling
  mCameraController = new QgsCameraController( this ); // attaches to the scene

  if ( mMap.sceneMode() == Qgis::SceneMode::Globe )
    mCameraController->resetGlobe( 10'000'000 );
  else
    mCameraController->resetView( 1000 );

  addCameraViewCenterEntity( mEngine->camera() );
  addCameraRotationCenterEntity( mCameraController );
  updateLights();

  // create terrain entity

  createTerrainDeferred();
  connect( &map, &Qgs3DMapSettings::extentChanged, this, &Qgs3DMapScene::createTerrain );
  connect( &map, &Qgs3DMapSettings::terrainGeneratorChanged, this, &Qgs3DMapScene::createTerrain );

  connect( &map, &Qgs3DMapSettings::terrainSettingsChanged, this, &Qgs3DMapScene::createTerrain );

  connect( &map, &Qgs3DMapSettings::terrainShadingChanged, this, &Qgs3DMapScene::createTerrain );
  connect( &map, &Qgs3DMapSettings::lightSourcesChanged, this, &Qgs3DMapScene::updateLights );
  connect( &map, &Qgs3DMapSettings::showLightSourceOriginsChanged, this, &Qgs3DMapScene::updateLights );
  connect( &map, &Qgs3DMapSettings::fieldOfViewChanged, this, &Qgs3DMapScene::updateCameraLens );
  connect( &map, &Qgs3DMapSettings::projectionTypeChanged, this, &Qgs3DMapScene::updateCameraLens );
  connect( &map, &Qgs3DMapSettings::skyboxSettingsChanged, this, &Qgs3DMapScene::onSkyboxSettingsChanged );
  connect( &map, &Qgs3DMapSettings::shadowSettingsChanged, this, &Qgs3DMapScene::onShadowSettingsChanged );
  connect( &map, &Qgs3DMapSettings::ambientOcclusionSettingsChanged, this, &Qgs3DMapScene::onAmbientOcclusionSettingsChanged );
  connect( &map, &Qgs3DMapSettings::eyeDomeLightingEnabledChanged, this, &Qgs3DMapScene::onEyeDomeShadingSettingsChanged );
  connect( &map, &Qgs3DMapSettings::eyeDomeLightingStrengthChanged, this, &Qgs3DMapScene::onEyeDomeShadingSettingsChanged );
  connect( &map, &Qgs3DMapSettings::eyeDomeLightingDistanceChanged, this, &Qgs3DMapScene::onEyeDomeShadingSettingsChanged );
  connect( &map, &Qgs3DMapSettings::debugShadowMapSettingsChanged, this, &Qgs3DMapScene::onDebugShadowMapSettingsChanged );
  connect( &map, &Qgs3DMapSettings::debugDepthMapSettingsChanged, this, &Qgs3DMapScene::onDebugDepthMapSettingsChanged );
  connect( &map, &Qgs3DMapSettings::fpsCounterEnabledChanged, this, &Qgs3DMapScene::fpsCounterEnabledChanged );
  connect( &map, &Qgs3DMapSettings::cameraMovementSpeedChanged, this, &Qgs3DMapScene::onCameraMovementSpeedChanged );
  connect( &map, &Qgs3DMapSettings::cameraNavigationModeChanged, this, &Qgs3DMapScene::onCameraNavigationModeChanged );
  connect( &map, &Qgs3DMapSettings::debugOverlayEnabledChanged, this, &Qgs3DMapScene::onDebugOverlayEnabledChanged );
  connect( &map, &Qgs3DMapSettings::stopUpdatesChanged, this, &Qgs3DMapScene::onStopUpdatesChanged );

  connect( &map, &Qgs3DMapSettings::axisSettingsChanged, this, &Qgs3DMapScene::on3DAxisSettingsChanged );

  connect( &map, &Qgs3DMapSettings::originChanged, this, &Qgs3DMapScene::onOriginChanged );

  connect( QgsApplication::sourceCache(), &QgsSourceCache::remoteSourceFetched, this, [this]( const QString &url ) {
    const QList<QgsMapLayer *> modelVectorLayers = mModelVectorLayers;
    for ( QgsMapLayer *layer : modelVectorLayers )
    {
      QgsAbstract3DRenderer *renderer = layer->renderer3D();
      if ( renderer )
      {
        if ( renderer->type() == "vector"_L1 )
        {
          const QgsPoint3DSymbol *pointSymbol = static_cast<const QgsPoint3DSymbol *>( static_cast<QgsVectorLayer3DRenderer *>( renderer )->symbol() );
          if ( pointSymbol && pointSymbol->shapeProperty( u"model"_s ).toString() == url )
          {
            removeLayerEntity( layer );
            addLayerEntity( layer );
          }
        }
        else if ( renderer->type() == "rulebased"_L1 )
        {
          const QgsRuleBased3DRenderer::RuleList rules = static_cast<QgsRuleBased3DRenderer *>( renderer )->rootRule()->descendants();
          for ( const QgsRuleBased3DRenderer::Rule *rule : rules )
          {
            const QgsPoint3DSymbol *pointSymbol = dynamic_cast<const QgsPoint3DSymbol *>( rule->symbol() );
            if ( pointSymbol && pointSymbol->shapeProperty( u"model"_s ).toString() == url )
            {
              removeLayerEntity( layer );
              addLayerEntity( layer );
              break;
            }
          }
        }
      }
    }
  } );

  // listen to changes of layers in order to add/remove 3D renderer entities
  connect( &map, &Qgs3DMapSettings::layersChanged, this, &Qgs3DMapScene::onLayersChanged );

  connect( mCameraController, &QgsCameraController::cameraChanged, this, &Qgs3DMapScene::onCameraChanged );
  connect( mEngine, &QgsAbstract3DEngine::sizeChanged, this, &Qgs3DMapScene::onCameraChanged );

  onSkyboxSettingsChanged();

  // force initial update of chunked entities
  onCameraChanged();
  // force initial update of eye dome shading
  onEyeDomeShadingSettingsChanged();
  // force initial update of debugging setting of preview quads
  onDebugShadowMapSettingsChanged();
  onDebugDepthMapSettingsChanged();
  // force initial update of ambient occlusion settings
  onAmbientOcclusionSettingsChanged();

  onCameraMovementSpeedChanged();

  on3DAxisSettingsChanged();
}

void Qgs3DMapScene::viewZoomFull()
{
  if ( mMap.sceneMode() == Qgis::SceneMode::Globe )
  {
    mCameraController->resetGlobe( 10'000'000 );
    return;
  }

  const QgsDoubleRange zRange = elevationRange();
  const QgsRectangle extent = sceneExtent();
  const double side = std::max( extent.width(), extent.height() );
  double d = side / 2 / std::tan( cameraController()->camera()->fieldOfView() / 2 * M_PI / 180 );
  d += zRange.isInfinite() ? 0. : zRange.upper();
  mCameraController->resetView( static_cast<float>( d ) );
}

void Qgs3DMapScene::setViewFrom2DExtent( const QgsRectangle &extent )
{
  QgsPointXY center = extent.center();
  const QgsVector3D origin = mMap.origin();

  const QgsVector3D p1 = mMap.mapToWorldCoordinates( QgsVector3D( extent.xMinimum(), extent.yMinimum(), 0 ) );
  const QgsVector3D p2 = mMap.mapToWorldCoordinates( QgsVector3D( extent.xMaximum(), extent.yMaximum(), 0 ) );

  const double xSide = std::abs( p1.x() - p2.x() );
  const double ySide = std::abs( p1.y() - p2.y() );
  const double side = std::max( xSide, ySide );

  const double fov = qDegreesToRadians( cameraController()->camera()->fieldOfView() );
  double distance = side / 2.0f / std::tan( fov / 2.0f );

  // adjust by elevation
  const QgsDoubleRange zRange = elevationRange();
  if ( !zRange.isInfinite() )
    distance += zRange.upper();

  // subtract map origin so coordinates are relative to it
  mCameraController->setViewFromTop(
    static_cast<float>( center.x() - origin.x() ),
    static_cast<float>( center.y() - origin.y() ),
    static_cast<float>( distance )
  );
}

QVector<QgsPointXY> Qgs3DMapScene::viewFrustum2DExtent() const
{
  Qt3DRender::QCamera *camera = mCameraController->camera();
  QVector<QgsPointXY> extent;
  QVector<int> pointsOrder = { 0, 1, 3, 2 };
  for ( int i : pointsOrder )
  {
    const QPoint p( ( ( i >> 0 ) & 1 ) ? 0 : mEngine->size().width(), ( ( i >> 1 ) & 1 ) ? 0 : mEngine->size().height() );
    QgsRay3D ray = Qgs3DUtils::rayFromScreenPoint( p, mEngine->size(), camera );
    QVector3D dir = ray.direction();
    if ( dir.z() == 0.0 )
      dir.setZ( 0.000001 );
    double t = -ray.origin().z() / dir.z();
    if ( t < 0 )
    {
      // If the projected point is on the back of the camera we choose the farthest point in the front
      t = camera->farPlane();
    }
    else
    {
      // If the projected point is on the front of the camera we choose the closest between it and farthest point in the front
      t = std::min<float>( t, camera->farPlane() );
    }
    QVector3D planePoint = ray.origin() + t * dir;
    QgsVector3D pMap = mMap.worldToMapCoordinates( planePoint );
    extent.push_back( QgsPointXY( pMap.x(), pMap.y() ) );
  }
  return extent;
}

int Qgs3DMapScene::totalPendingJobsCount() const
{
  int count = 0;
  for ( Qgs3DMapSceneEntity *entity : std::as_const( mSceneEntities ) )
    count += entity->pendingJobsCount();
  return count;
}

double Qgs3DMapScene::worldSpaceError( double epsilon, double distance ) const
{
  Qt3DRender::QCamera *camera = mCameraController->camera();
  const double fov = camera->fieldOfView();
  const QSize size = mEngine->size();
  const int screenSizePx = std::max( size.width(), size.height() ); // TODO: is this correct?

  // see Qgs3DUtils::screenSpaceError() for the inverse calculation (world space error to screen space error)
  // with explanation of the math.
  const double frustumWidthAtDistance = 2 * distance * tan( fov / 2 );
  const double err = frustumWidthAtDistance * epsilon / screenSizePx;
  return err;
}

void Qgs3DMapScene::onCameraChanged()
{
  if ( mMap.projectionType() == Qt3DRender::QCameraLens::OrthographicProjection )
  {
    QRect viewportRect( QPoint( 0, 0 ), mEngine->size() );
    const float viewWidthFromCenter = mCameraController->distance();
    const float viewHeightFromCenter = viewportRect.height() * viewWidthFromCenter / viewportRect.width();
    mEngine->camera()->lens()->setOrthographicProjection( -viewWidthFromCenter, viewWidthFromCenter, -viewHeightFromCenter, viewHeightFromCenter, mEngine->camera()->nearPlane(), mEngine->camera()->farPlane() );
  }

  updateScene( true );
  updateCameraNearFarPlanes();

  onShadowSettingsChanged();

  QVector<QgsPointXY> extent2D = viewFrustum2DExtent();
  emit viewed2DExtentFrom3DChanged( extent2D );

  // The magic to make things work better in large scenes (e.g. more than 50km across)
  // is here: we will simply move the origin of the scene, and update transforms
  // of the camera and all other entities. That should ensure we will not need to deal
  // with large coordinates in 32-bit floats (and if we do have large coordinates,
  // because the scene is far from the camera, we don't care, because those errors
  // end up being tiny when viewed from far away).
  constexpr float ORIGIN_SHIFT_THRESHOLD = 10'000;
  if ( mSceneOriginShiftEnabled && mEngine->camera()->position().length() > ORIGIN_SHIFT_THRESHOLD )
  {
    const QgsVector3D newOrigin = mMap.origin() + QgsVector3D( mEngine->camera()->position() );
    QgsDebugMsgLevel( u"Rebasing scene origin from %1 to %2"_s.arg( mMap.origin().toString( 1 ), newOrigin.toString( 1 ) ), 2 );
    mMap.setOrigin( newOrigin );
  }
}

bool Qgs3DMapScene::updateScene( bool forceUpdate )
{
  if ( !mSceneUpdatesEnabled )
  {
    QgsDebugMsgLevel( "Scene update skipped", 2 );
    return false;
  }

  QgsEventTracing::ScopedEvent traceEvent( u"3D"_s, forceUpdate ? u"Force update scene"_s : u"Update scene"_s );

  Qgs3DMapSceneEntity::SceneContext sceneContext;
  Qt3DRender::QCamera *camera = mEngine->camera();
  sceneContext.cameraFov = camera->fieldOfView();
  sceneContext.cameraPos = camera->position();
  const QSize size = mEngine->size();
  sceneContext.screenSizePx = std::max( size.width(), size.height() ); // TODO: is this correct?

  // Make our own projection matrix so that frustum culling done by the
  // entities isn't dependent on the current near/far planes, which would then
  // require multiple steps to stabilize.
  // The matrix is constructed just like in QMatrix4x4::perspective() /
  // ortho(), but for all elements involving the near and far plane, the limit
  // of the expression with the far plane going to infinity is taken.
  QMatrix4x4 projMatrix;
  switch ( mMap.projectionType() )
  {
    case Qt3DRender::QCameraLens::PerspectiveProjection:
    {
      float fovRadians = ( camera->fieldOfView() / 2.0f ) * static_cast<float>( M_PI ) / 180.0f;
      float fovCotan = std::cos( fovRadians ) / std::sin( fovRadians );
      projMatrix = {
        fovCotan / camera->aspectRatio(), 0, 0, 0,
        0, fovCotan, 0, 0,
        0, 0, -1, -2,
        0, 0, -1, 0
      };
      break;
    }
    case Qt3DRender::QCameraLens::OrthographicProjection:
    {
      Qt3DRender::QCameraLens *lens = camera->lens();
      projMatrix = {
        2.0f / ( lens->right() - lens->left() ),
        0,
        0,
        0,
        0,
        2.0f / ( lens->top() - lens->bottom() ),
        0,
        0,
        0,
        0,
        1,
        0,
        -( lens->left() + lens->right() ) / ( lens->right() - lens->left() ),
        -( lens->top() + lens->bottom() ) / ( lens->top() - lens->bottom() ),
        -1.0f,
        1.0f,
      };
      break;
    }
    default:
      QgsDebugError( "Unhandled 3D projection type" );
      projMatrix = camera->projectionMatrix(); // Give up and use the current matrix
  }
  sceneContext.viewProjectionMatrix = projMatrix * camera->viewMatrix();


  bool anyUpdated = false;
  for ( Qgs3DMapSceneEntity *entity : std::as_const( mSceneEntities ) )
  {
    if ( forceUpdate || ( entity->isEnabled() && entity->needsUpdate() ) )
    {
      anyUpdated = true;
      entity->handleSceneUpdate( sceneContext );
      if ( entity->hasReachedGpuMemoryLimit() )
        emit gpuMemoryLimitReached();
    }
  }

  updateSceneState();

  return anyUpdated;
}

bool Qgs3DMapScene::updateCameraNearFarPlanes()
{
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

  Qt3DRender::QCamera *camera = cameraController()->camera();
  QMatrix4x4 viewMatrix = camera->viewMatrix();
  float fnear = 1e9;
  float ffar = 0;

  // Iterate all scene entities to make sure that they will not get
  // clipped by the near or far plane
  for ( Qgs3DMapSceneEntity *se : std::as_const( mSceneEntities ) )
  {
    const QgsRange<float> depthRange = se->getNearFarPlaneRange( viewMatrix );

    fnear = std::min( fnear, depthRange.lower() );
    ffar = std::max( ffar, depthRange.upper() );
  }

  if ( fnear < 1 )
    fnear = 1; // does not really make sense to use negative far plane (behind camera)

  // the update didn't work out... this can happen if the scene does not contain
  // any Qgs3DMapSceneEntity. Use the scene extent to compute near and far planes
  // as a fallback.
  if ( fnear == 1e9 && ffar == 0 )
  {
    QgsDoubleRange sceneZRange = elevationRange();
    sceneZRange = sceneZRange.isInfinite() ? QgsDoubleRange( 0.0, 0.0 ) : sceneZRange;
    const QgsAABB sceneBbox = Qgs3DUtils::mapToWorldExtent( mMap.extent(), sceneZRange.lower(), sceneZRange.upper(), mMap.origin() );
    Qgs3DUtils::computeBoundingBoxNearFarPlanes( sceneBbox, viewMatrix, fnear, ffar );
  }

  // when zooming in a lot, fnear can become smaller than ffar. This should not happen
  if ( fnear > ffar )
    std::swap( fnear, ffar );

  // set near/far plane - with some tolerance in front/behind expected near/far planes
  float newFar = ffar * 2;
  float newNear = fnear / 2;
  if ( !qgsFloatNear( newFar, camera->farPlane() ) || !qgsFloatNear( newNear, camera->nearPlane() ) )
  {
    camera->setFarPlane( newFar );
    camera->setNearPlane( newNear );
    return true;
  }

  return false;
}

void Qgs3DMapScene::onFrameTriggered( float dt )
{
  QgsEventTracing::addEvent( QgsEventTracing::EventType::Instant, u"3D"_s, u"Frame begins"_s );

  mCameraController->frameTriggered( dt );

  if ( updateScene() )
    // If the scene was changed, node bboxes might have changed, so we need to
    // update near/far planes.
    updateCameraNearFarPlanes();

  // lock changing the FPS counter to 5 fps
  static int frameCount = 0;
  static float accumulatedTime = 0.0f;

  if ( !mMap.isFpsCounterEnabled() )
  {
    frameCount = 0;
    accumulatedTime = 0;
    return;
  }

  frameCount++;
  accumulatedTime += dt;
  if ( accumulatedTime >= 0.2f )
  {
    float fps = ( float ) frameCount / accumulatedTime;
    frameCount = 0;
    accumulatedTime = 0.0f;
    emit fpsCountChanged( fps );
  }
}

void Qgs3DMapScene::createTerrain()
{
  if ( mTerrain )
  {
    mSceneEntities.removeOne( mTerrain );

    delete mTerrain;
    mTerrain = nullptr;
  }

  if ( mGlobe )
  {
    mSceneEntities.removeOne( mGlobe );

    delete mGlobe;
    mGlobe = nullptr;
  }

  if ( !mTerrainUpdateScheduled )
  {
    // defer re-creation of terrain: there may be multiple invocations of this slot, so create the new entity just once
    QTimer::singleShot( 0, this, &Qgs3DMapScene::createTerrainDeferred );
    mTerrainUpdateScheduled = true;
    setSceneState( Updating );
  }
  else
  {
    emit terrainEntityChanged();
  }
}

void Qgs3DMapScene::createTerrainDeferred()
{
  QgsChunkedEntity *terrainOrGlobe = nullptr;

  if ( mMap.sceneMode() == Qgis::SceneMode::Globe && mMap.terrainRenderingEnabled() )
  {
    mGlobe = new QgsGlobeEntity( &mMap );
    terrainOrGlobe = mGlobe;
  }
  else if ( mMap.sceneMode() == Qgis::SceneMode::Local && mMap.terrainRenderingEnabled() && mMap.terrainGenerator() )
  {
    double tile0width = mMap.terrainGenerator()->rootChunkExtent().width();
    int maxZoomLevel = Qgs3DUtils::maxZoomLevel( tile0width, mMap.terrainSettings()->mapTileResolution(), mMap.terrainSettings()->maximumGroundError() );
    const QgsBox3D rootBox3D = mMap.terrainGenerator()->rootChunkBox3D( mMap );
    float rootError = mMap.terrainGenerator()->rootChunkError( mMap );
    const QgsBox3D clippingBox3D( mMap.extent(), rootBox3D.zMinimum(), rootBox3D.zMaximum() );
    mMap.terrainGenerator()->setupQuadtree( rootBox3D, rootError, maxZoomLevel, clippingBox3D );

    mTerrain = new QgsTerrainEntity( &mMap );
    terrainOrGlobe = mTerrain;
  }

  if ( terrainOrGlobe )
  {
    terrainOrGlobe->setParent( this );
    terrainOrGlobe->setShowBoundingBoxes( mMap.showTerrainBoundingBoxes() );

    mSceneEntities << terrainOrGlobe;

    connect( terrainOrGlobe, &QgsChunkedEntity::pendingJobsCountChanged, this, &Qgs3DMapScene::totalPendingJobsCountChanged );
    connect( terrainOrGlobe, &Qgs3DMapSceneEntity::newEntityCreated, this, [this]( Qt3DCore::QEntity *entity ) {
      // let's make sure that any entity we're about to show has the right scene origin set
      const QList<QgsGeoTransform *> transforms = entity->findChildren<QgsGeoTransform *>();
      for ( QgsGeoTransform *transform : transforms )
      {
        transform->setOrigin( mMap.origin() );
      }

      // enable clipping on the terrain if necessary
      handleClippingOnEntity( entity );
    } );
  }

  // make sure that renderers for layers are re-created as well
  const QList<QgsMapLayer *> layers = mMap.layers();
  for ( QgsMapLayer *layer : layers )
  {
    // remove old entity - if any
    removeLayerEntity( layer );

    // add new entity - if any 3D renderer
    addLayerEntity( layer );
  }

  emit terrainEntityChanged();
  onCameraChanged(); // force update of the new terrain
  mTerrainUpdateScheduled = false;
}

void Qgs3DMapScene::onBackgroundColorChanged()
{
  mEngine->setClearColor( mMap.backgroundColor() );
}

void Qgs3DMapScene::updateLights()
{
  for ( Qt3DCore::QEntity *entity : std::as_const( mLightEntities ) )
    entity->deleteLater();
  mLightEntities.clear();

  const QList<QgsLightSource *> newLights = mMap.lightSources();
  for ( const QgsLightSource *source : newLights )
  {
    mLightEntities.append( source->createEntity( mMap, this ) );
  }

  onShadowSettingsChanged();
}

void Qgs3DMapScene::updateCameraLens()
{
  mEngine->camera()->lens()->setFieldOfView( mMap.fieldOfView() );
  mEngine->camera()->lens()->setProjectionType( mMap.projectionType() );
  onCameraChanged();
}

void Qgs3DMapScene::onLayerRenderer3DChanged()
{
  QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sender() );
  Q_ASSERT( layer );

  // remove old entity - if any
  removeLayerEntity( layer );

  // add new entity - if any 3D renderer
  addLayerEntity( layer );
}

void Qgs3DMapScene::onLayersChanged()
{
  QSet<QgsMapLayer *> layersBefore = qgis::listToSet( mLayerEntities.keys() );
  QList<QgsMapLayer *> layersAdded;
  const QList<QgsMapLayer *> layers = mMap.layers();
  for ( QgsMapLayer *layer : layers )
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
  for ( QgsMapLayer *layer : std::as_const( layersBefore ) )
  {
    removeLayerEntity( layer );
  }

  for ( QgsMapLayer *layer : std::as_const( layersAdded ) )
  {
    addLayerEntity( layer );
  }
}

void Qgs3DMapScene::updateTemporal()
{
  const QList<QgsMapLayer *> layers = mLayerEntities.keys();
  for ( QgsMapLayer *layer : layers )
  {
    if ( QgsMapLayerTemporalProperties *temporalProperties = layer->temporalProperties() )
    {
      if ( temporalProperties->isActive() )
      {
        removeLayerEntity( layer );
        addLayerEntity( layer );
      }
    }
  }
}

void Qgs3DMapScene::addSceneEntity( Qgs3DMapSceneEntity *sceneNewEntity )
{
  Q_ASSERT( sceneNewEntity );

  mSceneEntities.append( sceneNewEntity );

  sceneNewEntity->setParent( this );

  finalizeNewEntity( sceneNewEntity );

  connect( sceneNewEntity, &Qgs3DMapSceneEntity::newEntityCreated, this, [this]( Qt3DCore::QEntity *entity ) {
    finalizeNewEntity( entity );
    // this ensures to update the near/far planes with the exact bounding box of the new entity.
    updateCameraNearFarPlanes();
  } );

  connect( sceneNewEntity, &Qgs3DMapSceneEntity::pendingJobsCountChanged, this, &Qgs3DMapScene::totalPendingJobsCountChanged );

  onCameraChanged(); // needed for chunked entities
}

void Qgs3DMapScene::removeSceneEntity( Qgs3DMapSceneEntity *sceneEntity )
{
  Q_ASSERT( sceneEntity );

  mSceneEntities.removeOne( sceneEntity );

  sceneEntity->deleteLater();
}


void Qgs3DMapScene::addLayerEntity( QgsMapLayer *layer )
{
  QgsAbstract3DRenderer *renderer = layer->renderer3D();
  if ( renderer )
  {
    // Fix vector layer's renderer to make sure the renderer is pointing to its layer.
    // It has happened before that renderer pointed to a different layer (probably after copying a style).
    // This is a bit of a hack and it should be handled in QgsMapLayer::setRenderer3D() but in qgis_core
    // the vector layer 3D renderer classes are not available.
    if ( layer->type() == Qgis::LayerType::Vector && ( renderer->type() == "vector"_L1 || renderer->type() == "rulebased"_L1 ) )
    {
      static_cast<QgsAbstractVectorLayer3DRenderer *>( renderer )->setLayer( static_cast<QgsVectorLayer *>( layer ) );
      if ( renderer->type() == "vector"_L1 )
      {
        QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
        if ( vlayer->geometryType() == Qgis::GeometryType::Point )
        {
          const QgsPoint3DSymbol *pointSymbol = static_cast<const QgsPoint3DSymbol *>( static_cast<QgsVectorLayer3DRenderer *>( renderer )->symbol() );
          if ( pointSymbol->shape() == Qgis::Point3DShape::Model )
          {
            mModelVectorLayers.append( layer );
          }
        }
      }
      else if ( renderer->type() == "rulebased"_L1 )
      {
        const QgsRuleBased3DRenderer::RuleList rules = static_cast<QgsRuleBased3DRenderer *>( renderer )->rootRule()->descendants();
        for ( auto rule : rules )
        {
          const QgsPoint3DSymbol *pointSymbol = dynamic_cast<const QgsPoint3DSymbol *>( rule->symbol() );
          if ( pointSymbol && pointSymbol->shape() == Qgis::Point3DShape::Model )
          {
            mModelVectorLayers.append( layer );
            break;
          }
        }
      }
    }
    else if ( layer->type() == Qgis::LayerType::Mesh && renderer->type() == "mesh"_L1 )
    {
      QgsMeshLayer3DRenderer *meshRenderer = static_cast<QgsMeshLayer3DRenderer *>( renderer );
      meshRenderer->setLayer( static_cast<QgsMeshLayer *>( layer ) );

      // Before entity creation, set the maximum texture size
      // Not very clean, but for now, only place found in the workflow to do that simple
      QgsMesh3DSymbol *sym = meshRenderer->symbol()->clone();
      sym->setMaximumTextureSize( maximumTextureSize() );
      meshRenderer->setSymbol( sym );
    }
    else if ( layer->type() == Qgis::LayerType::PointCloud && renderer->type() == "pointcloud"_L1 )
    {
      QgsPointCloudLayer3DRenderer *pointCloudRenderer = static_cast<QgsPointCloudLayer3DRenderer *>( renderer );
      pointCloudRenderer->setLayer( static_cast<QgsPointCloudLayer *>( layer ) );
    }
    else if ( layer->type() == Qgis::LayerType::TiledScene && renderer->type() == "tiledscene"_L1 )
    {
      QgsTiledSceneLayer3DRenderer *tiledSceneRenderer = static_cast<QgsTiledSceneLayer3DRenderer *>( renderer );
      tiledSceneRenderer->setLayer( static_cast<QgsTiledSceneLayer *>( layer ) );
    }
    else if ( layer->type() == Qgis::LayerType::Annotation && renderer->type() == "annotation"_L1 )
    {
      auto annotationLayerRenderer = qgis::down_cast<QgsAnnotationLayer3DRenderer *>( renderer );
      annotationLayerRenderer->setLayer( qobject_cast<QgsAnnotationLayer *>( layer ) );
    }

    Qt3DCore::QEntity *newEntity = renderer->createEntity( &mMap );
    if ( newEntity )
    {
      mLayerEntities.insert( layer, newEntity );

      if ( Qgs3DMapSceneEntity *sceneNewEntity = qobject_cast<Qgs3DMapSceneEntity *>( newEntity ) )
      {
        // also sets this scene as the entity's parent and finalizes it
        addSceneEntity( sceneNewEntity );
      }
      else
      {
        newEntity->setParent( this );
        finalizeNewEntity( newEntity );
      }
    }
  }

  connect( layer, &QgsMapLayer::request3DUpdate, this, &Qgs3DMapScene::onLayerRenderer3DChanged );

  if ( layer->type() == Qgis::LayerType::Vector )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
    connect( vlayer, &QgsVectorLayer::selectionChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );
    connect( vlayer, &QgsVectorLayer::layerModified, this, &Qgs3DMapScene::onLayerRenderer3DChanged );
    connect( vlayer, &QgsVectorLayer::subsetStringChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );
  }

  if ( layer->type() == Qgis::LayerType::Mesh )
  {
    connect( layer, &QgsMapLayer::rendererChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );
  }

  if ( layer->type() == Qgis::LayerType::PointCloud )
  {
    QgsPointCloudLayer *pclayer = qobject_cast<QgsPointCloudLayer *>( layer );
    connect( pclayer, &QgsPointCloudLayer::renderer3DChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );
    connect( pclayer, &QgsPointCloudLayer::subsetStringChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );
  }
}

void Qgs3DMapScene::removeLayerEntity( QgsMapLayer *layer )
{
  Qt3DCore::QEntity *entity = mLayerEntities.take( layer );

  if ( Qgs3DMapSceneEntity *sceneEntity = qobject_cast<Qgs3DMapSceneEntity *>( entity ) )
  {
    // also schedules the entity for deletion
    removeSceneEntity( sceneEntity );
  }
  else
  {
    if ( entity )
      entity->deleteLater();
  }

  disconnect( layer, &QgsMapLayer::request3DUpdate, this, &Qgs3DMapScene::onLayerRenderer3DChanged );

  if ( layer->type() == Qgis::LayerType::Vector )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
    disconnect( vlayer, &QgsVectorLayer::selectionChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );
    disconnect( vlayer, &QgsVectorLayer::layerModified, this, &Qgs3DMapScene::onLayerRenderer3DChanged );
    disconnect( vlayer, &QgsVectorLayer::subsetStringChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );
    mModelVectorLayers.removeAll( layer );
  }

  if ( layer->type() == Qgis::LayerType::Mesh )
  {
    disconnect( layer, &QgsMapLayer::rendererChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );
  }

  if ( layer->type() == Qgis::LayerType::PointCloud )
  {
    QgsPointCloudLayer *pclayer = qobject_cast<QgsPointCloudLayer *>( layer );
    disconnect( pclayer, &QgsPointCloudLayer::renderer3DChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );
    disconnect( pclayer, &QgsPointCloudLayer::subsetStringChanged, this, &Qgs3DMapScene::onLayerRenderer3DChanged );
    disconnect( pclayer, &QgsPointCloudLayer::layerModified, this, &Qgs3DMapScene::onLayerRenderer3DChanged );
  }
}

void Qgs3DMapScene::finalizeNewEntity( Qt3DCore::QEntity *newEntity )
{
  // let's make sure that any entity we're about to show has the right scene origin set
  const QList<QgsGeoTransform *> transforms = newEntity->findChildren<QgsGeoTransform *>();
  for ( QgsGeoTransform *transform : transforms )
  {
    transform->setOrigin( mMap.origin() );
  }

  // set clip planes on the new entity if necessary
  handleClippingOnEntity( newEntity );

  // this is probably not the best place for material-specific configuration,
  // maybe this could be more generalized when other materials need some specific treatment
  const QList<QgsLineMaterial *> childLineMaterials = newEntity->findChildren<QgsLineMaterial *>();
  for ( QgsLineMaterial *lm : childLineMaterials )
  {
    connect( mEngine, &QgsAbstract3DEngine::sizeChanged, lm, [lm, this] {
      lm->setViewportSize( mEngine->size() );
    } );

    lm->setViewportSize( mEngine->size() );
  }
  // configure billboard's viewport when the viewport is changed.
  const QList<QgsPoint3DBillboardMaterial *> childBillboardMaterials = newEntity->findChildren<QgsPoint3DBillboardMaterial *>();
  for ( QgsPoint3DBillboardMaterial *bm : childBillboardMaterials )
  {
    connect( mEngine, &QgsAbstract3DEngine::sizeChanged, bm, [bm, this] {
      bm->setViewportSize( mEngine->size() );
    } );

    bm->setViewportSize( mEngine->size() );
  }

  // Finalize adding the 3D transparent objects by adding the layer components to the entities
  QgsFrameGraph *frameGraph = mEngine->frameGraph();
  Qt3DRender::QLayer *transparentLayer = frameGraph->forwardRenderView().transparentObjectLayer();
  const QList<Qt3DRender::QMaterial *> childMaterials = newEntity->findChildren<Qt3DRender::QMaterial *>();
  for ( Qt3DRender::QMaterial *material : childMaterials )
  {
    // This handles the phong material without data defined properties.
    if ( Qt3DExtras::QDiffuseSpecularMaterial *ph = qobject_cast<Qt3DExtras::QDiffuseSpecularMaterial *>( material ) )
    {
      if ( ph->diffuse().value<QColor>().alphaF() != 1.0f )
      {
        Qt3DCore::QEntity *entity = qobject_cast<Qt3DCore::QEntity *>( ph->parent() );
        if ( entity && !entity->components().contains( transparentLayer ) )
        {
          entity->addComponent( transparentLayer );
        }
      }
    }
#if 0
    /*
     * Adds transparency layer to QgsPoint3DBillboardMaterial entities,
     * so that they get rendered in the transparent pipeline instead
     * of the opaque pipeline. Permits semi-opaque pixel rendering.
     *
     * Pros: nicely smoothed billboard symbol rendering, without harsh
     * aliased edges. Billboard symbols can use semi-transparent colors.
     *
     * Cons: Introduces ordering issues for billboards, where billboards
     * which should be shown behind others will appear in front from
     * some angles (i.e. the same issue as we get for 3d polygon objects
     * with transparency)
     *
     * Consider enabling if/when we have some workaround for the stacking issue,
     * eg CPU based sorting on camera movement...
     */
    else if ( QgsPoint3DBillboardMaterial *billboardMaterial = qobject_cast<QgsPoint3DBillboardMaterial *>( material ) )
    {
      Qt3DCore::QEntity *entity = qobject_cast<Qt3DCore::QEntity *>( billboardMaterial->parent() );
      if ( entity && !entity->components().contains( transparentLayer ) )
      {
        entity->addComponent( transparentLayer );
      }
    }
#endif
    else
    {
      // This handles the phong material with data defined properties, the textured case and point (instanced) symbols.
      Qt3DRender::QEffect *effect = material->effect();
      if ( effect )
      {
        const QVector<Qt3DRender::QParameter *> parameters = effect->parameters();
        for ( const Qt3DRender::QParameter *parameter : parameters )
        {
          if ( parameter->name() == "opacity" && parameter->value() != 1.0f )
          {
            Qt3DCore::QEntity *entity = qobject_cast<Qt3DCore::QEntity *>( material->parent() );
            if ( entity && !entity->components().contains( transparentLayer ) )
            {
              entity->addComponent( transparentLayer );
            }
            break;
          }
        }
      }
    }
  }
}

int Qgs3DMapScene::maximumTextureSize() const
{
  QSurface *surface = mEngine->surface();
  QOpenGLContext context;
  context.create();
  bool success = context.makeCurrent( surface );

  if ( success )
  {
    QOpenGLFunctions openglFunctions = QOpenGLFunctions( &context );

    GLint size;
    openglFunctions.initializeOpenGLFunctions();
    openglFunctions.glGetIntegerv( GL_MAX_TEXTURE_SIZE, &size );
    return int( size );
  }
  else
  {
    return 4096; //we can't have a context to defined the max texture size, we use this reasonable value
  }
}

void Qgs3DMapScene::addCameraViewCenterEntity( Qt3DRender::QCamera *camera )
{
  mEntityCameraViewCenter = new Qt3DCore::QEntity;

  Qt3DCore::QTransform *trCameraViewCenter = new Qt3DCore::QTransform;
  mEntityCameraViewCenter->addComponent( trCameraViewCenter );
  connect( camera, &Qt3DRender::QCamera::viewCenterChanged, this, [trCameraViewCenter, camera] {
    trCameraViewCenter->setTranslation( camera->viewCenter() );
  } );

  Qt3DExtras::QPhongMaterial *materialCameraViewCenter = new Qt3DExtras::QPhongMaterial;
  materialCameraViewCenter->setAmbient( Qt::red );
  mEntityCameraViewCenter->addComponent( materialCameraViewCenter );

  Qt3DExtras::QSphereMesh *rendererCameraViewCenter = new Qt3DExtras::QSphereMesh;
  rendererCameraViewCenter->setRadius( 10 );
  mEntityCameraViewCenter->addComponent( rendererCameraViewCenter );

  mEntityCameraViewCenter->setEnabled( mMap.showCameraViewCenter() );
  mEntityCameraViewCenter->setParent( this );

  connect( &mMap, &Qgs3DMapSettings::showCameraViewCenterChanged, this, [this] {
    mEntityCameraViewCenter->setEnabled( mMap.showCameraViewCenter() );
  } );
}

void Qgs3DMapScene::setSceneState( Qgs3DMapScene::SceneState state )
{
  if ( mSceneState == state )
    return;
  mSceneState = state;
  emit sceneStateChanged();
}

void Qgs3DMapScene::updateSceneState()
{
  if ( mTerrainUpdateScheduled )
  {
    setSceneState( Updating );
    return;
  }

  for ( Qgs3DMapSceneEntity *entity : std::as_const( mSceneEntities ) )
  {
    if ( entity->isEnabled() && entity->pendingJobsCount() > 0 )
    {
      setSceneState( Updating );
      return;
    }
  }

  setSceneState( Ready );
}

void Qgs3DMapScene::onSkyboxSettingsChanged()
{
  QgsSkyboxSettings skyboxSettings = mMap.skyboxSettings();
  if ( mSkybox )
  {
    mSkybox->deleteLater();
    mSkybox = nullptr;
  }

  mEngine->setFrustumCullingEnabled( !mMap.isSkyboxEnabled() );

  if ( mMap.isSkyboxEnabled() )
  {
    QMap<QString, QString> faces;
    switch ( skyboxSettings.skyboxType() )
    {
      case QgsSkyboxEntity::DistinctTexturesSkybox:
        faces = skyboxSettings.cubeMapFacesPaths();
        mSkybox = new QgsCubeFacesSkyboxEntity(
          faces[u"posX"_s], faces[u"posY"_s], faces[u"posZ"_s],
          faces[u"negX"_s], faces[u"negY"_s], faces[u"negZ"_s],
          this
        );
        break;
      case QgsSkyboxEntity::PanoramicSkybox:
        mSkybox = new QgsPanoramicSkyboxEntity( skyboxSettings.panoramicTexturePath(), this );
        break;
    }
  }
}

void Qgs3DMapScene::onShadowSettingsChanged()
{
  mEngine->frameGraph()->updateShadowSettings( mMap.shadowSettings(), mMap.lightSources() );
}

void Qgs3DMapScene::onAmbientOcclusionSettingsChanged()
{
  mEngine->frameGraph()->updateAmbientOcclusionSettings( mMap.ambientOcclusionSettings() );
}

void Qgs3DMapScene::onDebugShadowMapSettingsChanged()
{
  mEngine->frameGraph()->updateDebugShadowMapSettings( mMap );
}

void Qgs3DMapScene::onDebugDepthMapSettingsChanged()
{
  mEngine->frameGraph()->updateDebugDepthMapSettings( mMap );
}

void Qgs3DMapScene::onDebugOverlayEnabledChanged()
{
  mEngine->frameGraph()->setDebugOverlayEnabled( mMap.isDebugOverlayEnabled() );
  mEngine->renderSettings()->setRenderPolicy( mMap.isDebugOverlayEnabled() ? Qt3DRender::QRenderSettings::Always : Qt3DRender::QRenderSettings::OnDemand );
}

void Qgs3DMapScene::onEyeDomeShadingSettingsChanged()
{
  mEngine->frameGraph()->updateEyeDomeSettings( mMap );
}

void Qgs3DMapScene::onCameraMovementSpeedChanged()
{
  mCameraController->setCameraMovementSpeed( mMap.cameraMovementSpeed() );
}

void Qgs3DMapScene::onCameraNavigationModeChanged()
{
  mCameraController->setCameraNavigationMode( mMap.cameraNavigationMode() );
}

bool Qgs3DMapScene::exportScene( const Qgs3DMapExportSettings &exportSettings )
{
  QVector<QString> notParsedLayers;
  Qgs3DSceneExporter exporter;

  exporter.setTerrainResolution( exportSettings.terrrainResolution() );
  exporter.setSmoothEdges( exportSettings.smoothEdges() );
  exporter.setExportNormals( exportSettings.exportNormals() );
  exporter.setExportTextures( exportSettings.exportTextures() );
  exporter.setTerrainTextureResolution( exportSettings.terrainTextureResolution() );
  exporter.setScale( exportSettings.scale() );

  for ( auto it = mLayerEntities.constBegin(); it != mLayerEntities.constEnd(); ++it )
  {
    QgsMapLayer *layer = it.key();
    Qt3DCore::QEntity *rootEntity = it.value();
    Qgis::LayerType layerType = layer->type();
    switch ( layerType )
    {
      case Qgis::LayerType::Vector:
        if ( !exporter.parseVectorLayerEntity( rootEntity, qobject_cast<QgsVectorLayer *>( layer ) ) )
          notParsedLayers.push_back( layer->name() );
        break;
      case Qgis::LayerType::Raster:
      case Qgis::LayerType::Plugin:
      case Qgis::LayerType::Mesh:
      case Qgis::LayerType::VectorTile:
      case Qgis::LayerType::Annotation:
      case Qgis::LayerType::PointCloud:
      case Qgis::LayerType::Group:
      case Qgis::LayerType::TiledScene:
        notParsedLayers.push_back( layer->name() );
        break;
    }
  }

  if ( mTerrain )
    exporter.parseTerrain( mTerrain, "Terrain" );

  const bool sceneSaved = exporter.save( exportSettings.sceneName(), exportSettings.sceneFolderPath() );
  if ( !sceneSaved )
  {
    return false;
  }

  if ( !notParsedLayers.empty() )
  {
    QString message = tr( "The following layers were not exported:" ) + "\n";
    for ( const QString &layerName : notParsedLayers )
      message += layerName + "\n";
    QgsMessageOutput::showMessage( tr( "3D exporter warning" ), message, QgsMessageOutput::MessageText );
  }

  return true;
}

QVector<const QgsChunkNode *> Qgs3DMapScene::getLayerActiveChunkNodes( QgsMapLayer *layer )
{
  QVector<const QgsChunkNode *> chunks;
  if ( !mLayerEntities.contains( layer ) )
    return chunks;
  if ( QgsChunkedEntity *c = qobject_cast<QgsChunkedEntity *>( mLayerEntities[layer] ) )
  {
    const QList<QgsChunkNode *> activeNodes = c->activeNodes();
    for ( QgsChunkNode *n : activeNodes )
      chunks.push_back( n );
  }
  return chunks;
}

QgsRectangle Qgs3DMapScene::sceneExtent() const
{
  return mMap.extent();
}

QgsDoubleRange Qgs3DMapScene::elevationRange( const bool ignoreTerrain ) const
{
  double zMin = std::numeric_limits<double>::max();
  double zMax = std::numeric_limits<double>::lowest();
  if ( mMap.terrainRenderingEnabled() && mTerrain && !ignoreTerrain )
  {
    const QgsBox3D box3D = mTerrain->rootNode()->box3D();
    zMin = std::min( zMin, box3D.zMinimum() );
    zMax = std::max( zMax, box3D.zMaximum() );
  }

  for ( auto it = mLayerEntities.constBegin(); it != mLayerEntities.constEnd(); it++ )
  {
    QgsMapLayer *layer = it.key();
    switch ( layer->type() )
    {
      case Qgis::LayerType::PointCloud:
      {
        QgsPointCloudLayer *pcl = qobject_cast<QgsPointCloudLayer *>( layer );
        QgsDoubleRange zRange = pcl->elevationProperties()->calculateZRange( pcl );
        zMin = std::min( zMin, zRange.lower() );
        zMax = std::max( zMax, zRange.upper() );
        break;
      }
      case Qgis::LayerType::Mesh:
      {
        QgsMeshLayer *meshLayer = qobject_cast<QgsMeshLayer *>( layer );
        QgsAbstract3DRenderer *renderer3D = meshLayer->renderer3D();
        if ( renderer3D )
        {
          QgsMeshLayer3DRenderer *meshLayerRenderer = static_cast<QgsMeshLayer3DRenderer *>( renderer3D );
          const int verticalGroupDatasetIndex = meshLayerRenderer->symbol()->verticalDatasetGroupIndex();
          const QgsMeshDatasetGroupMetadata verticalGroupMetadata = meshLayer->datasetGroupMetadata( verticalGroupDatasetIndex );
          const double verticalScale = meshLayerRenderer->symbol()->verticalScale();
          zMin = std::min( zMin, verticalGroupMetadata.minimum() * verticalScale );
          zMax = std::max( zMax, verticalGroupMetadata.maximum() * verticalScale );
        }
        break;
      }
      case Qgis::LayerType::TiledScene:
      {
        QgsTiledSceneLayer *sceneLayer = qobject_cast<QgsTiledSceneLayer *>( layer );
        const QgsDoubleRange zRange = sceneLayer->elevationProperties()->calculateZRange( sceneLayer );
        if ( !zRange.isInfinite() && !zRange.isEmpty() )
        {
          zMin = std::min( zMin, zRange.lower() );
          zMax = std::max( zMax, zRange.upper() );
        }
        break;
      }
      case Qgis::LayerType::Annotation:
      case Qgis::LayerType::Group:
      case Qgis::LayerType::Plugin:
      case Qgis::LayerType::Raster:
      case Qgis::LayerType::Vector:
      case Qgis::LayerType::VectorTile:
        break;
    }
  }
  const QgsDoubleRange zRange( std::min( zMin, std::numeric_limits<double>::max() ), std::max( zMax, std::numeric_limits<double>::lowest() ) );
  return zRange.isEmpty() ? QgsDoubleRange() : zRange;
}

QMap<QString, Qgs3DMapScene *> Qgs3DMapScene::openScenes()
{
  return sOpenScenesFunction();
}

void Qgs3DMapScene::addCameraRotationCenterEntity( QgsCameraController *controller )
{
  mEntityRotationCenter = new Qt3DCore::QEntity;

  Qt3DCore::QTransform *trRotationCenter = new Qt3DCore::QTransform;
  mEntityRotationCenter->addComponent( trRotationCenter );
  Qt3DExtras::QPhongMaterial *materialRotationCenter = new Qt3DExtras::QPhongMaterial;
  materialRotationCenter->setAmbient( Qt::blue );
  mEntityRotationCenter->addComponent( materialRotationCenter );
  Qt3DExtras::QSphereMesh *rendererRotationCenter = new Qt3DExtras::QSphereMesh;
  rendererRotationCenter->setRadius( 10 );
  mEntityRotationCenter->addComponent( rendererRotationCenter );
  mEntityRotationCenter->setEnabled( false );
  mEntityRotationCenter->setParent( this );

  connect( controller, &QgsCameraController::cameraRotationCenterChanged, this, [trRotationCenter]( QVector3D center ) {
    trRotationCenter->setTranslation( center );
  } );

  connect( &mMap, &Qgs3DMapSettings::showCameraRotationCenterChanged, this, [this] {
    mEntityRotationCenter->setEnabled( mMap.showCameraRotationCenter() );
  } );
}

void Qgs3DMapScene::on3DAxisSettingsChanged()
{
  if ( m3DAxis )
  {
    m3DAxis->onAxisSettingsChanged();
  }
  else
  {
    if ( QgsWindow3DEngine *engine = dynamic_cast<QgsWindow3DEngine *>( mEngine ) )
    {
      m3DAxis = new Qgs3DAxis( static_cast<Qgs3DMapCanvas *>( engine->window() ), engine->root(), this, mCameraController, &mMap );
    }
  }
}

void Qgs3DMapScene::onOriginChanged()
{
  const QList<QgsGeoTransform *> geoTransforms = findChildren<QgsGeoTransform *>();
  for ( QgsGeoTransform *transform : geoTransforms )
  {
    transform->setOrigin( mMap.origin() );
  }

  const QList<QgsGeoTransform *> rubberBandGeoTransforms = mEngine->frameGraph()->rubberBandsRootEntity()->findChildren<QgsGeoTransform *>();
  for ( QgsGeoTransform *transform : rubberBandGeoTransforms )
  {
    transform->setOrigin( mMap.origin() );
  }

  const QgsVector3D oldOrigin = mCameraController->origin();
  mCameraController->setOrigin( mMap.origin() );

  if ( !mClipPlanesEquations.isEmpty() )
  {
    // how the math works - for a plane defined as (a,b,c,d), only "d" changes when
    // moving the origin - the plane normal vector (a,b,c) stays the same.
    // - line equation for old shift: a * (x - x0) + b * (y - y0) + c * (z - z0) + d0 = 0
    // - line equation for new shift: a * (x - x1) + b * (y - y1) + c * (z - z1) + d1 = 0
    // - we solve for d1:
    //     d1 = a * (x1 - x0) + b * (y1 - y0) + c * (z1 - z0) + d0

    QList<QVector4D> newPlanes;
    QgsVector3D originShift = mMap.origin() - oldOrigin;
    for ( QVector4D plane : std::as_const( mClipPlanesEquations ) )
    {
      plane.setW( originShift.x() * plane.x() + originShift.y() * plane.y() + originShift.z() * plane.z() + plane.w() );
      newPlanes.append( plane );
    }
    enableClipping( newPlanes );
  }
}

void Qgs3DMapScene::handleClippingOnEntity( QEntity *entity ) const
{
  if ( mClipPlanesEquations.isEmpty() ) // no clip plane equations, disable clipping
  {
    for ( QgsMaterial *material : entity->componentsOfType<QgsMaterial>() )
    {
      material->disableClipping();
    }
  }
  else // enable clipping
  {
    for ( QgsMaterial *material : entity->componentsOfType<QgsMaterial>() )
    {
      material->enableClipping( mClipPlanesEquations );
    }
  }

  // recursive call
  // enable or disable clipping on the children accordingly
  for ( QObject *child : entity->children() )
  {
    Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>( child );
    if ( childEntity )
    {
      handleClippingOnEntity( childEntity );
    }
  }
}

void Qgs3DMapScene::handleClippingOnAllEntities() const
{
  // Need to loop mLayerEntities instead of mSceneEntities to handle entities
  // which do no inherit from Qgs3DMapSceneEntity. For example, mesh entities.
  for ( auto it = mLayerEntities.constBegin(); it != mLayerEntities.constEnd(); ++it )
  {
    handleClippingOnEntity( it.value() );
  }
  if ( mTerrain )
  {
    handleClippingOnEntity( mTerrain );
  }
  if ( mGlobe )
  {
    handleClippingOnEntity( mGlobe );
  }
}

void Qgs3DMapScene::enableClipping( const QList<QVector4D> &clipPlaneEquations )
{
  if ( clipPlaneEquations.size() > mMaxClipPlanes )
  {
    QgsDebugMsgLevel( u"Qgs3DMapScene::enableClipping: it is not possible to use more than %1 clipping planes."_s.arg( mMaxClipPlanes ), 2 );
  }
  mClipPlanesEquations = clipPlaneEquations.mid( 0, mMaxClipPlanes );

  // enable the clip planes on the framegraph
  mEngine->frameGraph()->addClipPlanes( clipPlaneEquations.size() );

  // Enable the clip planes for the material of each entity.
  handleClippingOnAllEntities();
}

void Qgs3DMapScene::disableClipping()
{
  mClipPlanesEquations.clear();

  // disable the clip planes on the framegraph
  mEngine->frameGraph()->removeClipPlanes();

  // Disable the clip planes for the material of each entity.
  handleClippingOnAllEntities();
}

void Qgs3DMapScene::onStopUpdatesChanged()
{
  mSceneUpdatesEnabled = !mMap.stopUpdates();
}
