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

#include "qgscameracontroller.h"
#include "qgs3dmapsettings.h"
#include "qgs3dmapscene.h"
#include "qgs3dmaptool.h"
#include "qgswindow3dengine.h"


Qgs3DMapCanvas::Qgs3DMapCanvas( QWidget *parent )
  : QWidget( parent )
{
  mEngine = new QgsWindow3DEngine;

  connect( mEngine, &QgsAbstract3DEngine::imageCaptured, this, [ = ]( const QImage & image )
  {
    image.save( mCaptureFileName, mCaptureFileFormat.toLocal8Bit().data() );
    emit savedAsImage( mCaptureFileName );
  } );

  mContainer = QWidget::createWindowContainer( mEngine->window() );

  QHBoxLayout *hLayout = new QHBoxLayout( this );
  hLayout->setMargin( 0 );
  hLayout->addWidget( mContainer, 1 );

  mEngine->window()->setCursor( Qt::OpenHandCursor );
}

Qgs3DMapCanvas::~Qgs3DMapCanvas()
{
  delete mMap;
}

void Qgs3DMapCanvas::resizeEvent( QResizeEvent *ev )
{
  QWidget::resizeEvent( ev );

  if ( !mScene )
    return;

  QRect viewportRect( QPoint( 0, 0 ), size() );
  mScene->cameraController()->setViewport( viewportRect );
}

void Qgs3DMapCanvas::setMap( Qgs3DMapSettings *map )
{
  // TODO: eventually we want to get rid of this
  Q_ASSERT( !mMap );
  Q_ASSERT( !mScene );

  //QRect viewportRect( QPoint( 0, 0 ), size() );
  Qgs3DMapScene *newScene = new Qgs3DMapScene( *map, mEngine );

  mEngine->setRootEntity( newScene );

  if ( mScene )
    mScene->deleteLater();
  mScene = newScene;

  delete mMap;
  mMap = map;

  resetView();
}

QgsCameraController *Qgs3DMapCanvas::cameraController()
{
  return mScene ? mScene->cameraController() : nullptr;
}

void Qgs3DMapCanvas::resetView()
{
  mScene->viewZoomFull();
}

void Qgs3DMapCanvas::setViewFromTop( const QgsPointXY &center, float distance, float rotation )
{
  float worldX = center.x() - mMap->origin().x();
  float worldY = center.y() - mMap->origin().y();
  mScene->cameraController()->setViewFromTop( worldX, -worldY, distance, rotation );
}

void Qgs3DMapCanvas::saveAsImage( const QString fileName, const QString fileFormat )
{
  if ( !fileName.isEmpty() )
  {
    mCaptureFileName = fileName;
    mCaptureFileFormat = fileFormat;
    mEngine->requestCaptureImage();
  }
}

void Qgs3DMapCanvas::setMapTool( Qgs3DMapTool *tool )
{
  if ( tool == mMapTool )
    return;

  if ( mMapTool && !tool )
  {
    mEngine->window()->removeEventFilter( this );
    mScene->cameraController()->setEnabled( true );
    mEngine->window()->setCursor( Qt::OpenHandCursor );
  }
  else if ( !mMapTool && tool )
  {
    mEngine->window()->installEventFilter( this );
    mScene->cameraController()->setEnabled( false );
    mEngine->window()->setCursor( Qt::CrossCursor );
  }

  if ( mMapTool )
    mMapTool->deactivate();

  mMapTool = tool;

  if ( mMapTool )
    mMapTool->activate();
}

bool Qgs3DMapCanvas::eventFilter( QObject *watched, QEvent *event )
{
  if ( !mMapTool )
    return false;

  Q_UNUSED( watched );
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
