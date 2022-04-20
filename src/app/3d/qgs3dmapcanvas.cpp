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

#include "qgs3dmapcanvas.h"

#include <QBoxLayout>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DRender/QRenderCapture>
#include <Qt3DLogic/QFrameAction>
#include <QMouseEvent>


#include "qgscameracontroller.h"
#include "qgs3dmapsettings.h"
#include "qgs3dmapscene.h"
#include "qgs3dmaptool.h"
#include "qgswindow3dengine.h"
#include "qgs3dnavigationwidget.h"
#include "qgsproject.h"
#include "qgsprojectviewsettings.h"
#include "qgssettings.h"
#include "qgstemporalcontroller.h"
#include "qgsflatterraingenerator.h"
#include "qgsonlineterraingenerator.h"
#include "qgsray3d.h"
#include "qgs3dutils.h"
#include "qgsoffscreen3dengine.h"
#include "qgscoordinatetransform.h"

Qgs3DMapCanvas::Qgs3DMapCanvas( QWidget *parent )
  : QWidget( parent )
{
  const QgsSettings setting;
  mEngine = new QgsWindow3DEngine( this );

  connect( mEngine, &QgsAbstract3DEngine::imageCaptured, [ = ]( const QImage & image )
  {
    image.save( mCaptureFileName, mCaptureFileFormat.toLocal8Bit().data() );
    mEngine->setRenderCaptureEnabled( false );
    emit savedAsImage( mCaptureFileName );
  } );

  mSplitter = new QSplitter( this );

  mContainer = QWidget::createWindowContainer( mEngine->window() );
  mNavigationWidget = new Qgs3DNavigationWidget( this );

  mSplitter->addWidget( mContainer );
  mSplitter->addWidget( mNavigationWidget );

  QHBoxLayout *hLayout = new QHBoxLayout( this );
  hLayout->setContentsMargins( 0, 0, 0, 0 );
  hLayout->addWidget( mSplitter );
  this->setOnScreenNavigationVisibility(
    setting.value( QStringLiteral( "/3D/navigationWidget/visibility" ), true, QgsSettings::Gui ).toBool()
  );

  mEngine->window()->setCursor( Qt::OpenHandCursor );
  mEngine->window()->installEventFilter( this );

  connect( mSplitter, &QSplitter::splitterMoved, [&]( int, int )
  {
    QRect viewportRect( QPoint( 0, 0 ), mContainer->size() );
    mScene->cameraController()->setViewport( viewportRect );
    mEngine->setSize( viewportRect.size() );
  } );

  connect( mNavigationWidget, &Qgs3DNavigationWidget::sizeChanged, [&]( const QSize & newSize )
  {
    QSize widgetSize = size();
    QRect viewportRect( QPoint( 0, 0 ), QSize( widgetSize.width() - newSize.width(), widgetSize.height() ) );
    if ( mScene && mScene->cameraController() )
      mScene->cameraController()->setViewport( viewportRect );
    mEngine->setSize( viewportRect.size() );
  } );

  QRect viewportRect( QPoint( 0, 0 ), mContainer->size() );
  mEngine->setSize( viewportRect.size() );
}

Qgs3DMapCanvas::~Qgs3DMapCanvas()
{
  if ( mMapTool )
    mMapTool->deactivate();
  // make sure the scene is deleted while map settings object is still alive
  mScene->deleteLater();
  mScene = nullptr;
  mMap->deleteLater();
  mMap = nullptr;
}

void Qgs3DMapCanvas::resizeEvent( QResizeEvent *ev )
{
  QWidget::resizeEvent( ev );

  if ( !mScene )
    return;

  QRect viewportRect( QPoint( 0, 0 ), mContainer->size() );
  mScene->cameraController()->setViewport( viewportRect );

  mEngine->setSize( viewportRect.size() );
}

void Qgs3DMapCanvas::setMap( Qgs3DMapSettings *map )
{
  // TODO: eventually we want to get rid of this
  Q_ASSERT( !mMap );
  Q_ASSERT( !mScene );

  QRect viewportRect( QPoint( 0, 0 ), mContainer->size() );
  Qgs3DMapScene *newScene = new Qgs3DMapScene( *map, mEngine );

  mEngine->setSize( viewportRect.size() );
  mEngine->setRootEntity( newScene );

  if ( mScene )
  {
    mScene->deleteLater();
  }
  mScene = newScene;
  connect( mScene, &Qgs3DMapScene::fpsCountChanged, this, &Qgs3DMapCanvas::fpsCountChanged );
  connect( mScene, &Qgs3DMapScene::fpsCounterEnabledChanged, this, &Qgs3DMapCanvas::fpsCounterEnabledChanged );
  connect( mScene, &Qgs3DMapScene::viewed2DExtentFrom3DChanged, this, &Qgs3DMapCanvas::viewed2DExtentFrom3DChanged );

  delete mMap;
  mMap = map;

  mScene->cameraController()->setViewport( viewportRect );

  resetView();

  // Connect the camera to the navigation widget.
  connect( cameraController(), &QgsCameraController::cameraChanged, mNavigationWidget, &Qgs3DNavigationWidget::updateFromCamera );
  connect( cameraController(), &QgsCameraController::setCursorPosition, this, [ = ]( QPoint point )
  {
    QCursor::setPos( mapToGlobal( point ) );
  } );
  connect( cameraController(), &QgsCameraController::cameraMovementSpeedChanged, mMap, &Qgs3DMapSettings::setCameraMovementSpeed );
  connect( cameraController(), &QgsCameraController::cameraMovementSpeedChanged, this, &Qgs3DMapCanvas::cameraNavigationSpeedChanged );
  connect( cameraController(), &QgsCameraController::navigationModeHotKeyPressed, this, &Qgs3DMapCanvas::onNavigationModeHotKeyPressed );
  connect( cameraController(), &QgsCameraController::requestDepthBufferCapture, this, &Qgs3DMapCanvas::captureDepthBuffer );

  connect( mEngine, &QgsAbstract3DEngine::depthBufferCaptured, cameraController(), &QgsCameraController::depthBufferCaptured );

  emit mapSettingsChanged();
}

QgsCameraController *Qgs3DMapCanvas::cameraController()
{
  return mScene ? mScene->cameraController() : nullptr;
}

void Qgs3DMapCanvas::resetView( bool resetExtent )
{
  if ( resetExtent )
  {
    if ( map()->terrainRenderingEnabled()
         && map()->terrainGenerator()
         && ( map()->terrainGenerator()->type() == QgsTerrainGenerator::Flat ||
              map()->terrainGenerator()->type() == QgsTerrainGenerator::Online ) )
    {
      const QgsReferencedRectangle extent = QgsProject::instance()->viewSettings()->fullExtent();
      QgsCoordinateTransform ct( extent.crs(), map()->crs(), QgsProject::instance()->transformContext() );
      ct.setBallparkTransformsAreAppropriate( true );
      QgsRectangle rect;
      try
      {
        rect = ct.transformBoundingBox( extent );
      }
      catch ( QgsCsException & )
      {
        rect = extent;
      }
      map()->terrainGenerator()->setExtent( rect );

      const QgsRectangle te = mScene->sceneExtent();
      const QgsPointXY center = te.center();
      map()->setOrigin( QgsVector3D( center.x(), center.y(), 0 ) );
    }
    if ( !map()->terrainRenderingEnabled() || !map()->terrainGenerator() )
    {
      const QgsReferencedRectangle extent = QgsProject::instance()->viewSettings()->fullExtent();
      QgsCoordinateTransform ct( extent.crs(), map()->crs(), QgsProject::instance()->transformContext() );
      ct.setBallparkTransformsAreAppropriate( true );
      QgsRectangle rect;
      try
      {
        rect = ct.transformBoundingBox( extent );
      }
      catch ( QgsCsException & )
      {
        rect = extent;
      }

      const QgsPointXY center = rect.center();
      map()->setOrigin( QgsVector3D( center.x(), center.y(), 0 ) );
    }
  }

  mScene->viewZoomFull();
}

void Qgs3DMapCanvas::setViewFromTop( const QgsPointXY &center, float distance, float rotation )
{
  const float worldX = center.x() - mMap->origin().x();
  const float worldY = center.y() - mMap->origin().y();
  mScene->cameraController()->setViewFromTop( worldX, -worldY, distance, rotation );
}

void Qgs3DMapCanvas::saveAsImage( const QString &fileName, const QString &fileFormat )
{
  if ( !fileName.isEmpty() )
  {
    mCaptureFileName = fileName;
    mCaptureFileFormat = fileFormat;
    mEngine->setRenderCaptureEnabled( true );
    // Setup a frame action that is used to wait until next frame
    Qt3DLogic::QFrameAction *screenCaptureFrameAction = new Qt3DLogic::QFrameAction;
    mScene->addComponent( screenCaptureFrameAction );
    // Wait to have the render capture enabled in the next frame
    connect( screenCaptureFrameAction, &Qt3DLogic::QFrameAction::triggered, [ = ]( float )
    {
      mEngine->requestCaptureImage();
      mScene->removeComponent( screenCaptureFrameAction );
      screenCaptureFrameAction->deleteLater();
    } );
  }
}

void Qgs3DMapCanvas::captureDepthBuffer()
{
  // Setup a frame action that is used to wait until next frame
  Qt3DLogic::QFrameAction *screenCaptureFrameAction = new Qt3DLogic::QFrameAction;
  mScene->addComponent( screenCaptureFrameAction );
  // Wait to have the render capture enabled in the next frame
  connect( screenCaptureFrameAction, &Qt3DLogic::QFrameAction::triggered, [ = ]( float )
  {
    mEngine->requestDepthBufferCapture();
    mScene->removeComponent( screenCaptureFrameAction );
    screenCaptureFrameAction->deleteLater();
  } );
}

void Qgs3DMapCanvas::setMapTool( Qgs3DMapTool *tool )
{
  if ( tool == mMapTool )
    return;

  // For Camera Control tool
  if ( mMapTool && !tool )
  {
    mScene->cameraController()->setEnabled( true );
    mEngine->window()->setCursor( Qt::OpenHandCursor );
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
    mEngine->window()->setCursor( mMapTool->cursor() );
  }

}

bool Qgs3DMapCanvas::eventFilter( QObject *watched, QEvent *event )
{
  if ( watched != mEngine->window() )
    return false;

  if ( event->type() == QEvent::ShortcutOverride )
  {
    // if the camera controller will handle a key event, don't allow it to propagate
    // outside of the 3d window or it may be grabbed by a parent window level shortcut
    // and accordingly never be received by the camera controller
    if ( cameraController() && cameraController()->willHandleKeyEvent( static_cast< QKeyEvent * >( event ) ) )
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
    default:
      break;
  }
  return false;
}

void Qgs3DMapCanvas::setOnScreenNavigationVisibility( bool visibility )
{
  mNavigationWidget->setVisible( visibility );
  QgsSettings setting;
  setting.setValue( QStringLiteral( "/3D/navigationWidget/visibility" ), visibility, QgsSettings::Gui );
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
  mMap->setTemporalRange( temporalrange );
  mScene->updateTemporal();
}

QSize Qgs3DMapCanvas::windowSize() const
{
  return mEngine->size();
}

void Qgs3DMapCanvas::onNavigationModeHotKeyPressed( QgsCameraController::NavigationMode mode )
{
  mMap->setCameraNavigationMode( mode );
  mScene->cameraController()->setCameraNavigationMode( mode );
}

void Qgs3DMapCanvas::setViewFrom2DExtent( const QgsRectangle &extent )
{
  mScene->setViewFrom2DExtent( extent );
}

QVector<QgsPointXY> Qgs3DMapCanvas::viewFrustum2DExtent()
{
  return mScene->viewFrustum2DExtent();
}
