/***************************************************************************
  qgsdepthrenderview.cpp
  --------------------------------------
  Date                 : August 2022
  Copyright            : (C) 2022 by Benoit De Mezzo and (C) 2020 by Belgacem Nedjima
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdepthrenderview.h"
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/qsubtreeenabler.h>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QRenderCapture>
#include "qgsdepthentity.h"


QgsDepthRenderView::QgsDepthRenderView( const QString &viewName, QSize size, Qt3DRender::QTexture2D *forwardDepthTexture, Qt3DCore::QEntity *rootSceneEntity )
  : QgsAbstractRenderView( viewName )
  , mMainCamera( mainCamera )
{
  mLayer = new Qt3DRender::QLayer;
  mLayer->setRecursive( true );
  mLayer->setObjectName( mViewName + "::Layer" );

  // depth rendering pass
  buildRenderPass( size, forwardDepthTexture, rootSceneEntity );
}

void QgsDepthRenderView::updateWindowResize( int width, int height )
{
  mColorTexture->setSize( width, height );
  mDepthTexture->setSize( width, height );
}

Qt3DRender::QRenderTarget *QgsDepthRenderView::buildTextures( QSize size )
{
  // depth buffer render to copy pass
  // Create a render target output for rendering color.
  Qt3DRender::QRenderTargetOutput *colorOutput = new Qt3DRender::QRenderTargetOutput;
  colorOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );

  // Create a texture to render into.
  mColorTexture = new Qt3DRender::QTexture2D( colorOutput );
  mColorTexture->setSize( size.width(), size.height() );
  mColorTexture->setFormat( Qt3DRender::QAbstractTexture::RGB8_UNorm );
  mColorTexture->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mColorTexture->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mColorTexture->setObjectName( mViewName + "::mColorTexture" );

  // Hook the texture up to our output, and the output up to this object.
  colorOutput->setTexture( mColorTexture );

  Qt3DRender::QRenderTargetOutput *depthOutput = new Qt3DRender::QRenderTargetOutput;

  depthOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
  mDepthTexture = new Qt3DRender::QTexture2D( depthOutput );
  mDepthTexture->setSize( size.width(), size.height() );
  mDepthTexture->setFormat( Qt3DRender::QAbstractTexture::DepthFormat );
  mDepthTexture->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mDepthTexture->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mDepthTexture->setComparisonFunction( Qt3DRender::QAbstractTexture::CompareLessEqual );
  mDepthTexture->setComparisonMode( Qt3DRender::QAbstractTexture::CompareRefToTexture );
  mDepthTexture->setObjectName( mViewName + "::mDepthTexture" );

  depthOutput->setTexture( mDepthTexture );

  Qt3DRender::QRenderTarget *renderTarget = new Qt3DRender::QRenderTarget;
  renderTarget->setObjectName( mViewName + "::RenderTarget" );
  renderTarget->addOutput( colorOutput );
  renderTarget->addOutput( depthOutput );

  return renderTarget;
}

void QgsDepthRenderView::buildRenderPass( QSize size, Qt3DRender::QTexture2D *forwardDepthTexture, Qt3DCore::QEntity *rootSceneEntity )
{
  // depth buffer render to copy pass

  Qt3DRender::QCameraSelector *cameraSelector = new Qt3DRender::QCameraSelector( mRendererEnabler );
  cameraSelector->setObjectName( mViewName + "::CameraSelector" );
  cameraSelector->setCamera( mMainCamera );

  Qt3DRender::QRenderStateSet *stateSet = new Qt3DRender::QRenderStateSet( cameraSelector );

  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
  Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
  cullFace->setMode( Qt3DRender::QCullFace::NoCulling );

  stateSet->addRenderState( depthTest );
  stateSet->addRenderState( cullFace );

  Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter( stateSet );
  layerFilter->addLayer( mLayer );

  Qt3DRender::QRenderTargetSelector *renderTargetSelector = new Qt3DRender::QRenderTargetSelector( layerFilter );

  // build texture part
  Qt3DRender::QRenderTarget *renderTarget = buildTextures( size );

  renderTargetSelector->setTarget( renderTarget );

  // Note: We do not a clear buffers node since we are drawing a quad that will override the buffer's content anyway
  mDepthRenderCapture = new Qt3DRender::QRenderCapture( renderTargetSelector );

  // entity used to draw the depth texture and convert it to rgb image
  new QgsDepthEntity( forwardDepthTexture, mLayer, rootSceneEntity );
}
