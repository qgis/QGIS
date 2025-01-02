/***************************************************************************
  qgs3dmapcanvas.cpp
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

#include <Qt3DCore/QAspectEngine>
#include <Qt3DCore/QEntity>
#if QT_VERSION >= QT_VERSION_CHECK( 6, 0, 0 )
#include <Qt3DCore/QCoreAspect>
#endif
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DRender/QRenderAspect>
#include <Qt3DInput/QInputAspect>
#include <Qt3DInput/QInputSettings>
#include <Qt3DLogic/QLogicAspect>
#include <Qt3DRender/QCamera>
#include <Qt3DLogic/QFrameAction>

#include "qgs3dmapcanvas.h"
#include "qgs3dmapscene.h"
#include "qgswindow3dengine.h"
#include "qgs3dmapsettings.h"
#include "qgs3dmaptool.h"
#include "qgstemporalcontroller.h"
#include "qgsframegraph.h"
#include "qgspointcloudlayer3drenderer.h"
#include "qgsrubberband3d.h"

#include "moc_qgs3dmapcanvas.cpp"


Qgs3DMapCanvas::Qgs3DMapCanvas()
  : m_aspectEngine( new Qt3DCore::QAspectEngine )
  , m_renderAspect( new Qt3DRender::QRenderAspect )
  , m_inputAspect( new Qt3DInput::QInputAspect )
  , m_logicAspect( new Qt3DLogic::QLogicAspect )
  , m_renderSettings( new Qt3DRender::QRenderSettings )
  , m_defaultCamera( new Qt3DRender::QCamera )
  , m_inputSettings( new Qt3DInput::QInputSettings )
  , m_root( new Qt3DCore::QEntity )
  , m_userRoot( nullptr )
  , m_initialized( false )
{
  setSurfaceType( QSurface::OpenGLSurface );

  // register aspects
#if QT_VERSION >= QT_VERSION_CHECK( 6, 0, 0 )
  m_aspectEngine->registerAspect( new Qt3DCore::QCoreAspect );
#endif
  m_aspectEngine->registerAspect( m_renderAspect );
  m_aspectEngine->registerAspect( m_inputAspect );
  m_aspectEngine->registerAspect( m_logicAspect );

  m_defaultCamera->setParent( m_root );
  m_inputSettings->setEventSource( this );

  const QgsSettings setting;
  mEngine = new QgsWindow3DEngine( this );

  connect( mEngine, &QgsAbstract3DEngine::imageCaptured, this, [=]( const QImage &image ) {
    image.save( mCaptureFileName, mCaptureFileFormat.toLocal8Bit().data() );
    mEngine->setRenderCaptureEnabled( false );
    emit savedAsImage( mCaptureFileName );
  } );

  setCursor( Qt::OpenHandCursor );
  installEventFilter( this );
}

Qgs3DMapCanvas::~Qgs3DMapCanvas()
{
  if ( mMapTool )
    mMapTool->deactivate();
  // make sure the scene is deleted while map settings object is still alive
  mScene->deleteLater();
  mScene = nullptr;
  mMapSettings->deleteLater();
  mMapSettings = nullptr;
  qDeleteAll( mHighlights );
  mHighlights.clear();

  delete m_aspectEngine;
}

void Qgs3DMapCanvas::setRootEntity( Qt3DCore::QEntity *root )
{
  if ( m_userRoot != root )
  {
    if ( m_userRoot )
      m_userRoot->setParent( static_cast<Qt3DCore::QNode *>( nullptr ) );
    if ( root )
      root->setParent( m_root );
    m_userRoot = root;
  }
}

void Qgs3DMapCanvas::setActiveFrameGraph( Qt3DRender::QFrameGraphNode *activeFrameGraph )
{
  m_renderSettings->setActiveFrameGraph( activeFrameGraph );
}

Qt3DRender::QFrameGraphNode *Qgs3DMapCanvas::activeFrameGraph() const
{
  return m_renderSettings->activeFrameGraph();
}

Qt3DRender::QCamera *Qgs3DMapCanvas::camera() const
{
  return m_defaultCamera;
}

Qt3DRender::QRenderSettings *Qgs3DMapCanvas::renderSettings() const
{
  return m_renderSettings;
}

void Qgs3DMapCanvas::showEvent( QShowEvent *e )
{
  if ( !m_initialized )
  {
    m_root->addComponent( m_renderSettings );
    m_root->addComponent( m_inputSettings );
    m_aspectEngine->setRootEntity( Qt3DCore::QEntityPtr( m_root ) );

    m_initialized = true;
  }
  QWindow::showEvent( e );
}

void Qgs3DMapCanvas::resizeEvent( QResizeEvent * )
{
  m_defaultCamera->setAspectRatio( float( width() ) / std::max( 1.f, static_cast<float>( height() ) ) );

  mEngine->setSize( size() );
}

void Qgs3DMapCanvas::setMapSettings( Qgs3DMapSettings *mapSettings )
{
  // TODO: eventually we want to get rid of this
  Q_ASSERT( !mMapSettings );
  Q_ASSERT( !mScene );

  Qgs3DMapScene *newScene = new Qgs3DMapScene( *mapSettings, mEngine );

  mEngine->setSize( size() );
  mEngine->setRootEntity( newScene );

  if ( mScene )
  {
    mScene->deleteLater();
  }
  mScene = newScene;
  connect( mScene, &Qgs3DMapScene::fpsCountChanged, this, &Qgs3DMapCanvas::fpsCountChanged );
  connect( mScene, &Qgs3DMapScene::fpsCounterEnabledChanged, this, &Qgs3DMapCanvas::fpsCounterEnabledChanged );
  connect( mScene, &Qgs3DMapScene::viewed2DExtentFrom3DChanged, this, &Qgs3DMapCanvas::viewed2DExtentFrom3DChanged );

  delete mMapSettings;
  mMapSettings = mapSettings;

  resetView();

  connect( cameraController(), &QgsCameraController::setCursorPosition, this, [=]( QPoint point ) {
    QCursor::setPos( mapToGlobal( point ) );
  } );
  connect( cameraController(), &QgsCameraController::cameraMovementSpeedChanged, mMapSettings, &Qgs3DMapSettings::setCameraMovementSpeed );
  connect( cameraController(), &QgsCameraController::cameraMovementSpeedChanged, this, &Qgs3DMapCanvas::cameraNavigationSpeedChanged );
  connect( cameraController(), &QgsCameraController::navigationModeChanged, this, &Qgs3DMapCanvas::onNavigationModeChanged );
  connect( cameraController(), &QgsCameraController::requestDepthBufferCapture, this, &Qgs3DMapCanvas::captureDepthBuffer );

  connect( mEngine, &QgsAbstract3DEngine::depthBufferCaptured, cameraController(), &QgsCameraController::depthBufferCaptured );

  emit mapSettingsChanged();
}

QgsCameraController *Qgs3DMapCanvas::cameraController()
{
  return mScene ? mScene->cameraController() : nullptr;
}

void Qgs3DMapCanvas::resetView()
{
  if ( !mScene )
    return;

  mScene->viewZoomFull();
}

void Qgs3DMapCanvas::setViewFromTop( const QgsPointXY &center, float distance, float rotation )
{
  if ( !mScene )
    return;

  const float worldX = center.x() - mMapSettings->origin().x();
  const float worldY = center.y() - mMapSettings->origin().y();
  mScene->cameraController()->setViewFromTop( worldX, worldY, distance, rotation );
}

void Qgs3DMapCanvas::saveAsImage( const QString &fileName, const QString &fileFormat )
{
  if ( !mScene || fileName.isEmpty() )
    return;

  mCaptureFileName = fileName;
  mCaptureFileFormat = fileFormat;
  mEngine->setRenderCaptureEnabled( true );
  // Setup a frame action that is used to wait until next frame
  Qt3DLogic::QFrameAction *screenCaptureFrameAction = new Qt3DLogic::QFrameAction;
  mScene->addComponent( screenCaptureFrameAction );
  // Wait to have the render capture enabled in the next frame
  connect( screenCaptureFrameAction, &Qt3DLogic::QFrameAction::triggered, this, [=]( float ) {
    mEngine->requestCaptureImage();
    mScene->removeComponent( screenCaptureFrameAction );
    screenCaptureFrameAction->deleteLater();
  } );
}

void Qgs3DMapCanvas::captureDepthBuffer()
{
  if ( !mScene )
    return;

  // Setup a frame action that is used to wait until next frame
  Qt3DLogic::QFrameAction *screenCaptureFrameAction = new Qt3DLogic::QFrameAction;
  mScene->addComponent( screenCaptureFrameAction );
  // Wait to have the render capture enabled in the next frame
  connect( screenCaptureFrameAction, &Qt3DLogic::QFrameAction::triggered, this, [=]( float ) {
    mEngine->requestDepthBufferCapture();
    mScene->removeComponent( screenCaptureFrameAction );
    screenCaptureFrameAction->deleteLater();
  } );
}

void Qgs3DMapCanvas::setMapTool( Qgs3DMapTool *tool )
{
  if ( !mScene )
    return;

  if ( tool == mMapTool )
    return;

  // For Camera Control tool
  if ( mMapTool && !tool )
  {
    mScene->cameraController()->setEnabled( true );
    setCursor( Qt::OpenHandCursor );
  }
  else if ( !mMapTool && tool )
  {
    mScene->cameraController()->setEnabled( tool->allowsCameraControls() );
  }

  if ( mMapTool )
    mMapTool->deactivate();

  mMapTool = tool;

  if ( mMapTool )
  {
    mMapTool->activate();
    setCursor( mMapTool->cursor() );
  }
}

bool Qgs3DMapCanvas::eventFilter( QObject *watched, QEvent *event )
{
  if ( watched != this )
    return false;

  if ( event->type() == QEvent::ShortcutOverride )
  {
    // if the camera controller will handle a key event, don't allow it to propagate
    // outside of the 3d window or it may be grabbed by a parent window level shortcut
    // and accordingly never be received by the camera controller
    if ( cameraController() && cameraController()->willHandleKeyEvent( static_cast<QKeyEvent *>( event ) ) )
    {
      event->accept();
      return true;
    }
    return false;
  }

  if ( !mMapTool )
    return false;

  switch ( event->type() )
  {
    case QEvent::MouseButtonPress:
      mMapTool->mousePressEvent( static_cast<QMouseEvent *>( event ) );
      break;
    case QEvent::MouseButtonRelease:
      mMapTool->mouseReleaseEvent( static_cast<QMouseEvent *>( event ) );
      break;
    case QEvent::MouseMove:
      mMapTool->mouseMoveEvent( static_cast<QMouseEvent *>( event ) );
      break;
    case QEvent::KeyPress:
      mMapTool->keyPressEvent( static_cast<QKeyEvent *>( event ) );
      break;
    default:
      break;
  }
  return false;
}

void Qgs3DMapCanvas::setTemporalController( QgsTemporalController *temporalController )
{
  if ( mTemporalController )
    disconnect( mTemporalController, &QgsTemporalController::updateTemporalRange, this, &Qgs3DMapCanvas::updateTemporalRange );

  mTemporalController = temporalController;
  connect( mTemporalController, &QgsTemporalController::updateTemporalRange, this, &Qgs3DMapCanvas::updateTemporalRange );
}

void Qgs3DMapCanvas::updateTemporalRange( const QgsDateTimeRange &temporalrange )
{
  if ( !mScene )
    return;

  mMapSettings->setTemporalRange( temporalrange );
  mScene->updateTemporal();
}

void Qgs3DMapCanvas::onNavigationModeChanged( Qgis::NavigationMode mode )
{
  mMapSettings->setCameraNavigationMode( mode );
}

void Qgs3DMapCanvas::setViewFrom2DExtent( const QgsRectangle &extent )
{
  if ( !mScene )
    return;

  mScene->setViewFrom2DExtent( extent );
}

QVector<QgsPointXY> Qgs3DMapCanvas::viewFrustum2DExtent()
{
  return mScene ? mScene->viewFrustum2DExtent() : QVector<QgsPointXY>();
}

void Qgs3DMapCanvas::highlightFeature( const QgsFeature &feature, QgsMapLayer *layer )
{
  // we only support point clouds for now
  if ( layer->type() != Qgis::LayerType::PointCloud )
    return;

  const QgsGeometry geom = feature.geometry();
  const QgsPoint pt( geom.vertexAt( 0 ) );

  if ( !mHighlights.contains( layer ) )
  {
    QgsRubberBand3D *band = new QgsRubberBand3D( *mMapSettings, mEngine, mEngine->frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Point );

    const QgsSettings settings;
    const QColor color = QColor( settings.value( QStringLiteral( "Map/highlight/color" ), Qgis::DEFAULT_HIGHLIGHT_COLOR.name() ).toString() );
    band->setColor( color );
    band->setMarkerType( QgsRubberBand3D::MarkerType::Square );
    if ( QgsPointCloudLayer3DRenderer *pcRenderer = dynamic_cast<QgsPointCloudLayer3DRenderer *>( layer->renderer3D() ) )
    {
      band->setWidth( pcRenderer->symbol()->pointSize() + 1 );
    }
    mHighlights.insert( layer, band );

    connect( layer, &QgsMapLayer::renderer3DChanged, this, &Qgs3DMapCanvas::updateHighlightSizes );
  }
  mHighlights[layer]->addPoint( pt );
}

void Qgs3DMapCanvas::updateHighlightSizes()
{
  if ( QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sender() ) )
  {
    if ( QgsPointCloudLayer3DRenderer *rnd = dynamic_cast<QgsPointCloudLayer3DRenderer *>( layer->renderer3D() ) )
    {
      if ( mHighlights.contains( layer ) )
      {
        mHighlights[layer]->setWidth( rnd->symbol()->pointSize() + 1 );
      }
    }
  }
}

void Qgs3DMapCanvas::clearHighlights()
{
  for ( auto it = mHighlights.keyBegin(); it != mHighlights.keyEnd(); it++ )
  {
    disconnect( it.base().key(), &QgsMapLayer::renderer3DChanged, this, &Qgs3DMapCanvas::updateHighlightSizes );
  }

  qDeleteAll( mHighlights );
  mHighlights.clear();
}
