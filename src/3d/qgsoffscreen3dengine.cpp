/***************************************************************************
  qgsoffscreen3dengine.cpp
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

#include "qgsoffscreen3dengine.h"

#include <QOffscreenSurface>
#include <QSurfaceFormat>
#include <QOpenGLFunctions>

#include <Qt3DCore/QAspectEngine>
#include <Qt3DLogic/QLogicAspect>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QRenderAspect>
#include <Qt3DRender/QRenderCapture>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QRenderTargetOutput>
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/QRenderSurfaceSelector>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QViewport>
#include <QtGui/QOpenGLContext>

QgsOffscreen3DEngine::QgsOffscreen3DEngine()
{
  // Set up the default OpenGL surface format.
  QSurfaceFormat format;

  // by default we get just some older version of OpenGL from the system,
  // but for 3D lines we use "primitive restart" functionality supported in OpenGL >= 3.1
  // Qt3DWindow uses this - requesting OpenGL 4.3 - so let's request the same version.
#ifdef QT_OPENGL_ES_2
  format.setRenderableType( QSurfaceFormat::OpenGLES );
#else
  if ( QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL )
  {
    format.setVersion( 4, 3 );
    format.setProfile( QSurfaceFormat::CoreProfile );
  }
#endif

  format.setMajorVersion( 3 );
  format.setDepthBufferSize( 32 ); // TODO: or 24?  (used by QWindow3D)
  format.setSamples( 8 );
  QSurfaceFormat::setDefaultFormat( format );

  // Set up a camera to point at the shapes.
  mCamera = new Qt3DRender::QCamera;
  mCamera->lens()->setPerspectiveProjection( 45.0f, float( mSize.width() ) / float( mSize.height() ), 0.1f, 1000.0f );
  mCamera->setPosition( QVector3D( 0, 0, 20.0f ) );
  mCamera->setUpVector( QVector3D( 0, 1, 0 ) );
  mCamera->setViewCenter( QVector3D( 0, 0, 0 ) );

  // Set up the engine and the aspects that we want to use.
  mAspectEngine = new Qt3DCore::QAspectEngine();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  mRenderAspect = new Qt3DRender::QRenderAspect( Qt3DRender::QRenderAspect::Threaded ); // Only threaded mode seems to work right now.
#else
  mRenderAspect = new Qt3DRender::QRenderAspect();
#endif

  mLogicAspect = new Qt3DLogic::QLogicAspect();

  mAspectEngine->registerAspect( mRenderAspect );
  mAspectEngine->registerAspect( mLogicAspect );

  // Create the root entity of the engine.
  // This is not the same as the 3D scene root: the QRenderSettings
  // component must be held by the root of the QEntity tree,
  // so it is added to this one. The 3D scene is added as a subtree later,
  // in setRootEntity().
  mRoot = new Qt3DCore::QEntity;
  mRenderSettings = new Qt3DRender::QRenderSettings( mRoot );
  mRoot->addComponent( mRenderSettings );

  mCamera->setParent( mRoot );

  // Create the offscreen frame graph, which will manage all of the resources required
  // for rendering without a QWindow.
  mOffscreenSurface = new QOffscreenSurface();
  mOffscreenSurface->setFormat( QSurfaceFormat::defaultFormat() );
  mOffscreenSurface->create();

  mFrameGraph = new QgsShadowRenderingFrameGraph( mOffscreenSurface, mSize, mCamera, mRoot );
  mFrameGraph->setRenderCaptureEnabled( true );
  mFrameGraph->setShadowRenderingEnabled( false );
  // Set this frame graph to be in use.
  // the render settings also sets itself as the parent of mSurfaceSelector
  mRenderSettings->setActiveFrameGraph( mFrameGraph->frameGraphRoot() );

  // Set the root entity of the engine. This causes the engine to begin running.
  mAspectEngine->setRootEntity( Qt3DCore::QEntityPtr( mRoot ) );

}

QgsOffscreen3DEngine::~QgsOffscreen3DEngine()
{
  delete mAspectEngine;
  delete mOffscreenSurface;
}

void QgsOffscreen3DEngine::setSize( QSize s )
{
  mSize = s;

  mFrameGraph->setSize( mSize );
  mCamera->setAspectRatio( float( mSize.width() ) / float( mSize.height() ) );
}

void QgsOffscreen3DEngine::setClearColor( const QColor &color )
{
  mFrameGraph->setClearColor( color );
}

void QgsOffscreen3DEngine::setFrustumCullingEnabled( bool enabled )
{
  mFrameGraph->setFrustumCullingEnabled( enabled );
}

void QgsOffscreen3DEngine::setRootEntity( Qt3DCore::QEntity *root )
{
  // Make sure any existing scene root is unparented.
  if ( mSceneRoot )
  {
    mSceneRoot->setParent( static_cast<Qt3DCore::QNode *>( nullptr ) );
  }

  // Parent the incoming scene root to our current root entity.
  mSceneRoot = root;
  mSceneRoot->setParent( mRoot );
  root->addComponent( mFrameGraph->forwardRenderLayer() );
  root->addComponent( mFrameGraph->castShadowsLayer() );
}

Qt3DRender::QRenderSettings *QgsOffscreen3DEngine::renderSettings()
{
  return mRenderSettings;
}

Qt3DRender::QCamera *QgsOffscreen3DEngine::camera()
{
  return mCamera;
}

QSize QgsOffscreen3DEngine::size() const
{
  return mSize;
}

QSurface *QgsOffscreen3DEngine::surface() const
{
  return mOffscreenSurface;
}
