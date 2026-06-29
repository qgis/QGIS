/***************************************************************************
  qgspostprocessingrenderview.cpp
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Benoit De Mezzo and (C) 2020 by Belgacem Nedjima
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

#include "qgsambientocclusionrenderview.h"
#include "qgsforwardrenderview.h"
#include "qgsoverlaytexturerenderview.h"
#include "qgspostprocessingentity.h"
#include "qgsshadowrenderview.h"

#include <QNode>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QNoDraw>
#include <Qt3DRender/QRenderCapture>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QRenderTargetOutput>
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/qsubtreeenabler.h>

QgsPostprocessingRenderView::QgsPostprocessingRenderView( const QString &viewName, QgsFrameGraph *frameGraph, QSize size, Qt3DCore::QEntity *rootSceneEntity )
  : QgsAbstractRenderView( viewName )
{
  // postprocessing main rendering pass
  Qt3DRender::QFrameGraphNode *subPassesNode = constructMainPass( size );

  // sub passes:
  Qt3DRender::QFrameGraphNode *node;
  // 1. postprocessing real render view
  node = constructSubPassForProcessing( frameGraph, rootSceneEntity );
  node->setParent( subPassesNode );

  // 2. texture overlay render view
  node = constructSubPassForOverlayTexture();
  node->setParent( subPassesNode );

  // 3. render capture render view
  node = constructSubPassForRenderCapture();
  node->setParent( subPassesNode );
}

void QgsPostprocessingRenderView::updateWindowResize( int width, int height )
{
  QgsAbstractRenderView::updateWindowResize( width, height );
  mRenderCaptureColorTexture->setSize( width, height );
  mRenderCaptureDepthTexture->setSize( width, height );
}

void QgsPostprocessingRenderView::setOffScreenRenderCaptureEnabled( bool enabled )
{
  if ( mRenderCaptureTargetSelector->isEnabled() == enabled )
    return;

  mRenderCaptureTargetSelector->setEnabled( enabled );
}


Qt3DRender::QRenderTarget *QgsPostprocessingRenderView::buildRenderCaptureTextures( QSize size )
{
  // =============== v NEEDED ONLY FOR OFFSCREEN ENGINE v
  Qt3DRender::QRenderTarget *renderTarget = new Qt3DRender::QRenderTarget;

  // Create a render target output for rendering color.
  Qt3DRender::QRenderTargetOutput *colorOutput = new Qt3DRender::QRenderTargetOutput;
  colorOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );

  // Create a texture to render into.
  mRenderCaptureColorTexture = new Qt3DRender::QTexture2D( colorOutput );
  mRenderCaptureColorTexture->setSize( size.width(), size.height() );
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
  mRenderCaptureDepthTexture->setSize( size.width(), size.height() );
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

Qt3DRender::QFrameGraphNode *QgsPostprocessingRenderView::constructMainPass( QSize size )
{
  // We need to move the render target selector at the top of this branch.
  // Doing so this allows Qt3d to have a FBO format matching the one need to do the capture
  mRenderCaptureTargetSelector = new Qt3DRender::QRenderTargetSelector( mRendererEnabler );
  mRenderCaptureTargetSelector->setObjectName( mViewName + "::RenderTargetSelector" );
  mRenderCaptureTargetSelector->setEnabled( false );

  // build texture part
  Qt3DRender::QRenderTarget *renderTarget = buildRenderCaptureTextures( size );
  mRenderCaptureTargetSelector->setTarget( renderTarget );

  return mRenderCaptureTargetSelector;
}


Qt3DRender::QFrameGraphNode *QgsPostprocessingRenderView::constructSubPassForProcessing( QgsFrameGraph *frameGraph, Qt3DCore::QEntity *rootSceneEntity )
{
  Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter;
  layerFilter->setObjectName( mViewName + "::Sub pass::Postprocessing" );

  Qt3DRender::QLayer *postProcessingLayer = new Qt3DRender::QLayer();
  postProcessingLayer->setRecursive( true );
  layerFilter->addLayer( postProcessingLayer );

  // end of this branch
  new Qt3DRender::QClearBuffers( layerFilter );

  mPostprocessingEntity = new QgsPostprocessingEntity( frameGraph, postProcessingLayer, rootSceneEntity );
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

Qt3DRender::QFrameGraphNode *QgsPostprocessingRenderView::constructSubPassForOverlayTexture()
{
  mOverlayTextureRenderView = std::make_unique<QgsOverlayTextureRenderView>( mViewName + "::Sub pass::OverlayTexture" );
  return mOverlayTextureRenderView->topGraphNode();
}


Qt3DRender::QRenderCapture *QgsPostprocessingRenderView::renderCapture() const
{
  return mRenderCapture;
}

QgsPostprocessingEntity *QgsPostprocessingRenderView::entity() const
{
  return mPostprocessingEntity;
}
