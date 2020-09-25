/***************************************************************************
  qgswindow3dengine.cpp
  --------------------------------------
  Date                 : July 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswindow3dengine.h"

#include <Qt3DRender/QRenderCapture>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DRender/QRenderSettings>

#include "qgspreviewquad.h"

QgsWindow3DEngine::QgsWindow3DEngine( QObject *parent )
  : QgsAbstract3DEngine( parent )
{
  mWindow3D = new Qt3DExtras::Qt3DWindow;

  mRoot = new Qt3DCore::QEntity;
  mWindow3D->setRootEntity( mRoot );

  mShadowRenderingFrameGraph = new QgsShadowRenderingFrameGraph( mWindow3D, mWindow3D->camera(), mRoot );

  mWindow3D->setActiveFrameGraph( mShadowRenderingFrameGraph->getFrameGraphRoot() );

  // force switching to no shadow rendering
  setShadowRenderingEnabled( false );
}

QWindow *QgsWindow3DEngine::window()
{
  return mWindow3D;
}

void QgsWindow3DEngine::requestCaptureImage()
{
  Qt3DRender::QRenderCaptureReply *captureReply;
  captureReply = mShadowRenderingFrameGraph->renderCapture()->requestCapture();
  connect( captureReply, &Qt3DRender::QRenderCaptureReply::completed, this, [ = ]
  {
    emit imageCaptured( captureReply->image() );
    captureReply->deleteLater();
  } );
}

void QgsWindow3DEngine::setShadowRenderingEnabled( bool enabled )
{
  mShadowRenderingEnabled = enabled;
  mShadowRenderingFrameGraph->setShadowRenderingEnabled( mShadowRenderingEnabled );
}

void QgsWindow3DEngine::setClearColor( const QColor &color )
{
  mShadowRenderingFrameGraph->setClearColor( color );
}

void QgsWindow3DEngine::setFrustumCullingEnabled( bool enabled )
{
  // Not sure if this works properly
  mShadowRenderingFrameGraph->setFrustumCullingEnabled( enabled );
}

void QgsWindow3DEngine::setRootEntity( Qt3DCore::QEntity *root )
{
  mSceneRoot = root;
  mSceneRoot->setParent( mRoot );
  mSceneRoot->addComponent( mShadowRenderingFrameGraph->forwardRenderLayer() );
  mSceneRoot->addComponent( mShadowRenderingFrameGraph->castShadowsLayer() );
}

Qt3DRender::QRenderSettings *QgsWindow3DEngine::renderSettings()
{
  return mWindow3D->renderSettings();
}

Qt3DRender::QCamera *QgsWindow3DEngine::camera()
{
  return mWindow3D->camera();
}

QSize QgsWindow3DEngine::size() const
{
  return mWindow3D->size();
}

QSurface *QgsWindow3DEngine::surface() const
{
  return mWindow3D;
}
