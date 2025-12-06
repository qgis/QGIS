/***************************************************************************
  qgspostprocessingrenderview.cpp
  --------------------------------------
  Date                 : May 2025
  Copyright            : (C) 2025 by Benoit De Mezzo and (C) 2020 by Belgacem Nedjima
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspostprocessingrenderview.h"
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/qsubtreeenabler.h>
#include <Qt3DRender/QRenderCapture>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QRenderTargetOutput>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QNoDraw>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QDepthTest>

#include <QNode>
#include "qgspostprocessingentity.h"
#include "qgsforwardrenderview.h"
#include "qgsshadowrenderview.h"
#include "qgsambientocclusionrenderview.h"

QgsPostprocessingRenderView::QgsPostprocessingRenderView( const QString &viewName,                     //
                                                          QgsShadowRenderView &shadowRenderView,       //
                                                          QgsForwardRenderView &forwardRenderView,     //
                                                          QgsAmbientOcclusionRenderView &aoRenderView, //
                                                          QSize mSize,                                 //
                                                          Qt3DCore::QEntity *rootSceneEntity )
  : QgsAbstractRenderView( viewName )
{
  mDebugTextureLayer = new Qt3DRender::QLayer;
  mDebugTextureLayer->setRecursive( true );
  mDebugTextureLayer->setObjectName( mViewName + "::DebugTextureLayer" );

  // postprocessing main rendering pass
  constructMainPass( shadowRenderView, forwardRenderView, aoRenderView, mSize, rootSceneEntity );
}

QgsPostprocessingRenderView::~QgsPostprocessingRenderView()
{
  delete mDebugTextureRoot.data();
  mDebugTextureRoot.clear();
}

void QgsPostprocessingRenderView::updateWindowResize( int width, int height )
{
  QgsAbstractRenderView::updateWindowResize( width, height );
  mRenderCaptureColorTexture->setSize( width, height );
  mRenderCaptureDepthTexture->setSize( width, height );
}

void QgsPostprocessingRenderView::setOffScreenRenderCaptureEnabled( bool enabled )
{
  if ( !isEnabled() || mRenderCaptureTargetSelector->isEnabled() == enabled )
    return;

  mRenderCaptureTargetSelector->setEnabled( enabled );
  mRenderCapture->parentNode()->setEnabled( enabled ); // for NoDraw node
}


Qt3DRender::QRenderTarget *QgsPostprocessingRenderView::buildRenderCaptureTextures( QSize mSize )
{
  // =============== v NEEDED ONLY FOR OFFSCREEN ENGINE v
  Qt3DRender::QRenderTarget *renderTarget = new Qt3DRender::QRenderTarget;

  // Create a render target output for rendering color.
  Qt3DRender::QRenderTargetOutput *colorOutput = new Qt3DRender::QRenderTargetOutput;
  colorOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );

  // Create a texture to render into.
  mRenderCaptureColorTexture = new Qt3DRender::QTexture2D( colorOutput );
  mRenderCaptureColorTexture->setSize( mSize.width(), mSize.height() );
  mRenderCaptureColorTexture->setFormat( Qt3DRender::QAbstractTexture::RGB8_UNorm );
  mRenderCaptureColorTexture->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mRenderCaptureColorTexture->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mRenderCaptureColorTexture->setObjectName( mViewName + "::ColorTarget" );

  // Hook the texture up to our output, and the output up to this object.
  colorOutput->setTexture( mRenderCaptureColorTexture );
  renderTarget->addOutput( colorOutput );

  Qt3DRender::QRenderTargetOutput *depthOutput = new Qt3DRender::QRenderTargetOutput;

  depthOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
  mRenderCaptureDepthTexture = new Qt3DRender::QTexture2D( depthOutput );
  mRenderCaptureDepthTexture->setSize( mSize.width(), mSize.height() );
  mRenderCaptureDepthTexture->setFormat( Qt3DRender::QAbstractTexture::DepthFormat );
  mRenderCaptureDepthTexture->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mRenderCaptureDepthTexture->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mRenderCaptureDepthTexture->setComparisonFunction( Qt3DRender::QAbstractTexture::CompareLessEqual );
  mRenderCaptureDepthTexture->setComparisonMode( Qt3DRender::QAbstractTexture::CompareRefToTexture );
  mRenderCaptureDepthTexture->setObjectName( mViewName + "::DepthTarget" );

  depthOutput->setTexture( mRenderCaptureDepthTexture );
  renderTarget->addOutput( depthOutput );

  return renderTarget;
  // =============== ^ NEEDED ONLY FOR OFFSCREEN ENGINE ^
}

Qt3DRender::QFrameGraphNode *QgsPostprocessingRenderView::constructMainPass( QgsShadowRenderView &shadowRenderView,       //
                                                                             QgsForwardRenderView &forwardRenderView,     //
                                                                             QgsAmbientOcclusionRenderView &aoRenderView, //
                                                                             QSize mSize,                                 //
                                                                             Qt3DCore::QEntity *rootSceneEntity )
{
  // Due to a bug in Qt5 (fixed in Qt6 - https://codereview.qt-project.org/c/qt/qt3d/+/462575) we need to move the render target selector at the top
  // of this branch. Doing so this allows Qt3d to have a FBO format matching the one need to do the capture
  mRenderCaptureTargetSelector = new Qt3DRender::QRenderTargetSelector( mRendererEnabler );
  mRenderCaptureTargetSelector->setObjectName( mViewName + "::RenderTargetSelector" );
  mRenderCaptureTargetSelector->setEnabled( false );

  // build texture part
  Qt3DRender::QRenderTarget *renderTarget = buildRenderCaptureTextures( mSize );
  mRenderCaptureTargetSelector->setTarget( renderTarget );

  Qt3DRender::QFrameGraphNode *postprocessingNode = constructSubPassForPostProcessing( shadowRenderView, forwardRenderView, aoRenderView, rootSceneEntity );
  postprocessingNode->setParent( mRenderCaptureTargetSelector );

  Qt3DRender::QFrameGraphNode *debugTextureNode = constructSubPassForDebugTexture();
  debugTextureNode->setParent( mRenderCaptureTargetSelector );

  Qt3DRender::QFrameGraphNode *renderCaptureNode = constructSubPassForRenderCapture();
  renderCaptureNode->setParent( mRenderCaptureTargetSelector );

  return mRenderCaptureTargetSelector;
}


Qt3DRender::QFrameGraphNode *QgsPostprocessingRenderView::constructSubPassForPostProcessing( QgsShadowRenderView &shadowRenderView,       //
                                                                                             QgsForwardRenderView &forwardRenderView,     //
                                                                                             QgsAmbientOcclusionRenderView &aoRenderView, //
                                                                                             Qt3DCore::QEntity *rootSceneEntity )
{
  Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter;
  layerFilter->setObjectName( mViewName + "::Sub pass::Postprocessing" );

  Qt3DRender::QLayer *postProcessingLayer = new Qt3DRender::QLayer();
  postProcessingLayer->setRecursive( true );
  layerFilter->addLayer( postProcessingLayer );

  // end of this branch
  new Qt3DRender::QClearBuffers( layerFilter );

  mPostprocessingEntity = new QgsPostprocessingEntity( shadowRenderView,    //
                                                       forwardRenderView,   //
                                                       aoRenderView,        //
                                                       postProcessingLayer, //
                                                       rootSceneEntity );
  mPostprocessingEntity->setObjectName( "PostProcessingPassEntity" );

  return layerFilter;
}

Qt3DRender::QFrameGraphNode *QgsPostprocessingRenderView::constructSubPassForRenderCapture()
{
  Qt3DRender::QFrameGraphNode *top = new Qt3DRender::QNoDraw;
  top->setObjectName( mViewName + "::Sub pass::RenderCapture" );

  mRenderCapture = new Qt3DRender::QRenderCapture( top );

  return top;
}


Qt3DRender::QFrameGraphNode *QgsPostprocessingRenderView::constructSubPassForDebugTexture()
{
  mDebugTextureRoot = new Qt3DRender::QNoDraw;
  mDebugTextureRoot->setEnabled( false );
  mDebugTextureRoot->setObjectName( mViewName + "::DebugTexture::NoDraw" );
  mDebugTextureRendererEnabler = new Qt3DRender::QSubtreeEnabler( mDebugTextureRoot );
  mDebugTextureRendererEnabler->setEnablement( Qt3DRender::QSubtreeEnabler::Persistent );
  mDebugTextureRendererEnabler->setObjectName( mViewName + "::DebugTexture::SubtreeEnabler" );

  Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter( mDebugTextureRendererEnabler );
  layerFilter->addLayer( mDebugTextureLayer );

  Qt3DRender::QRenderStateSet *renderStateSet = new Qt3DRender::QRenderStateSet( layerFilter );
  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
  renderStateSet->addRenderState( depthTest );
  Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
  cullFace->setMode( Qt3DRender::QCullFace::NoCulling );
  renderStateSet->addRenderState( cullFace );

  return mDebugTextureRoot;
}

void QgsPostprocessingRenderView::setDebugTextureEnabled( bool enable )
{
  mDebugTextureRoot->setEnabled( !enable );
  mDebugTextureRendererEnabler->setEnabled( enable );
}

Qt3DRender::QLayer *QgsPostprocessingRenderView::debugTextureLayer() const
{
  return mDebugTextureLayer;
}

Qt3DRender::QRenderCapture *QgsPostprocessingRenderView::renderCapture() const
{
  return mRenderCapture;
}

QgsPostprocessingEntity *QgsPostprocessingRenderView::entity() const
{
  return mPostprocessingEntity;
}
