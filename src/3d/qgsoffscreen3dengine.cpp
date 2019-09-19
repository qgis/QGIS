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
  mRenderAspect = new Qt3DRender::QRenderAspect( Qt3DRender::QRenderAspect::Threaded ); // Only threaded mode seems to work right now.
  mLogicAspect = new Qt3DLogic::QLogicAspect();

  mAspectEngine->registerAspect( mRenderAspect );
  mAspectEngine->registerAspect( mLogicAspect );

  // Create the root entity of the engine.
  // This is not the same as the 3D scene root: the QRenderSettings
  // component must be held by the root of the QEntity tree,
  // so it is added to this one. The 3D scene is added as a subtree later,
  // in setRootEntity().
  mRoot = new Qt3DCore::QEntity();
  mRenderSettings = new Qt3DRender::QRenderSettings( mRoot );
  mRoot->addComponent( mRenderSettings );

  mCamera->setParent( mRoot );

  // Create the offscreen frame graph, which will manage all of the resources required
  // for rendering without a QWindow.
  createFrameGraph();

  // Set this frame graph to be in use.
  // the render settings also sets itself as the parent of mSurfaceSelector
  mRenderSettings->setActiveFrameGraph( mSurfaceSelector );

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

  mTexture->setSize( mSize.width(), mSize.height() );
  mDepthTexture->setSize( mSize.width(), mSize.height() );
  mSurfaceSelector->setExternalRenderTargetSize( mSize );

  mCamera->setAspectRatio( float( mSize.width() ) / float( mSize.height() ) );
}

void QgsOffscreen3DEngine::setClearColor( const QColor &color )
{
  mClearBuffers->setClearColor( color );
}

void QgsOffscreen3DEngine::setFrustumCullingEnabled( bool enabled )
{
  // TODO
  Q_UNUSED( enabled )
}

void QgsOffscreen3DEngine::createRenderTarget()
{
  mTextureTarget = new Qt3DRender::QRenderTarget;

  // The lifetime of the objects created here is managed
  // automatically, as they become children of this object.

  // Create a render target output for rendering color.
  mTextureOutput = new Qt3DRender::QRenderTargetOutput( mTextureTarget );
  mTextureOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );

  // Create a texture to render into.
  mTexture = new Qt3DRender::QTexture2D( mTextureOutput );
  mTexture->setSize( mSize.width(), mSize.height() );
  mTexture->setFormat( Qt3DRender::QAbstractTexture::RGB8_UNorm );
  mTexture->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mTexture->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );

  // Hook the texture up to our output, and the output up to this object.
  mTextureOutput->setTexture( mTexture );
  mTextureTarget->addOutput( mTextureOutput );

  mDepthTextureOutput = new Qt3DRender::QRenderTargetOutput( mTextureTarget );
  mDepthTextureOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
  mDepthTexture = new Qt3DRender::QTexture2D( mDepthTextureOutput );
  mDepthTexture->setSize( mSize.width(), mSize.height() );
  mDepthTexture->setFormat( Qt3DRender::QAbstractTexture::DepthFormat );
  mDepthTexture->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mDepthTexture->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mDepthTexture->setComparisonFunction( Qt3DRender::QAbstractTexture::CompareLessEqual );
  mDepthTexture->setComparisonMode( Qt3DRender::QAbstractTexture::CompareRefToTexture );
  // Hook up the depth texture
  mDepthTextureOutput->setTexture( mDepthTexture );
  mTextureTarget->addOutput( mDepthTextureOutput );
}

void QgsOffscreen3DEngine::createFrameGraph()
{
  // Firstly, create the offscreen surface. This will take the place
  // of a QWindow, allowing us to render our scene without one.
  mOffscreenSurface = new QOffscreenSurface();
  mOffscreenSurface->setFormat( QSurfaceFormat::defaultFormat() );
  mOffscreenSurface->create();

  // Hook it up to the frame graph.
  mSurfaceSelector = new Qt3DRender::QRenderSurfaceSelector( mRenderSettings );
  mSurfaceSelector->setSurface( mOffscreenSurface );
  mSurfaceSelector->setExternalRenderTargetSize( mSize );

  // Create a texture to render into. This acts as the buffer that
  // holds the rendered image.
  mRenderTargetSelector = new Qt3DRender::QRenderTargetSelector( mSurfaceSelector );
  createRenderTarget();
  // the target selector also sets itself as the parent of mTextureTarget
  mRenderTargetSelector->setTarget( mTextureTarget );

  // Create a node used for clearing the required buffers.
  mClearBuffers = new Qt3DRender::QClearBuffers( mRenderTargetSelector );
  mClearBuffers->setClearColor( QColor( 100, 100, 100, 255 ) );
  mClearBuffers->setBuffers( Qt3DRender::QClearBuffers::ColorDepthBuffer );

  // Create a viewport node. The viewport here just covers the entire render area.
  mViewport = new Qt3DRender::QViewport( mRenderTargetSelector );
  mViewport->setNormalizedRect( QRectF( 0.0, 0.0, 1.0, 1.0 ) );

  // Create a camera selector node, and tell it to use the camera we've ben given.
  mCameraSelector = new Qt3DRender::QCameraSelector( mViewport );
  mCameraSelector->setCamera( mCamera );

  // Add a render capture node to the frame graph.
  // This is set as the next child of the render target selector node,
  // so that the capture will be taken from the specified render target
  // once all other rendering operations have taken place.
  mRenderCapture = new Qt3DRender::QRenderCapture( mRenderTargetSelector );
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
  mSceneRoot->setParent( mAspectEngine->rootEntity().data() );
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

void QgsOffscreen3DEngine::requestCaptureImage()
{
  if ( mReply )
  {
    qDebug() << "already having a pending capture, skipping";
    return;
  }
  mReply = mRenderCapture->requestCapture();
  connect( mReply, &Qt3DRender::QRenderCaptureReply::completed, this, [ = ]
  {
    QImage image = mReply->image();
    mReply->deleteLater();
    mReply = nullptr;
    emit imageCaptured( image );
  } );
}
