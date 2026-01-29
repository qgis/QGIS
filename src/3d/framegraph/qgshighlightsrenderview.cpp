/***************************************************************************
    qgshighlightsrenderview.cpp
    ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshighlightsrenderview.h"

#include <Qt3DRender/QBlendEquation>
#include <Qt3DRender/QBlendEquationArguments>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QNoDepthMask>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/QStencilMask>
#include <Qt3DRender/QStencilOperation>
#include <Qt3DRender/QStencilOperationArguments>
#include <Qt3DRender/QStencilTest>
#include <Qt3DRender/QStencilTestArguments>
#include <Qt3DRender/QViewport>
#include <Qt3DRender/qsubtreeenabler.h>

QgsHighlightsRenderView::QgsHighlightsRenderView( const QString &viewName, Qt3DRender::QRenderTarget *target, Qt3DRender::QCamera *camera )
  : QgsAbstractRenderView( viewName )
  , mRenderTarget( target )
  , mMainCamera( camera )
{
  mHighlightsLayer = new Qt3DRender::QLayer;
  mHighlightsLayer->setRecursive( true );
  mHighlightsLayer->setObjectName( mViewName + "::Layer" );

  buildRenderPasses();
}

void QgsHighlightsRenderView::buildRenderPasses()
{
  Qt3DRender::QRenderTargetSelector *highlightsRenderTargetSelector = new Qt3DRender::QRenderTargetSelector( mRendererEnabler );
  highlightsRenderTargetSelector->setTarget( mRenderTarget );

  // Step 1: draw semi-transparent highlight
  //
  // Clear stencil and render all Highlights Layer's entities as semi-transparent, with no depth test,
  // while writing to the stencil buffer. Depth buffer is not modified.
  {
    Qt3DRender::QRenderStateSet *stateSet = new Qt3DRender::QRenderStateSet( highlightsRenderTargetSelector );

    // we need to allow writing to the stencil buffer before clearing it
    Qt3DRender::QStencilMask *forceWriteMask = new Qt3DRender::QStencilMask( stateSet );
    forceWriteMask->setFrontOutputMask( 0xFF );
    forceWriteMask->setBackOutputMask( 0xFF );
    stateSet->addRenderState( forceWriteMask );

    Qt3DRender::QClearBuffers *clearStencilBuffer = new Qt3DRender::QClearBuffers( stateSet );
    clearStencilBuffer->setBuffers( Qt3DRender::QClearBuffers::StencilBuffer );
    clearStencilBuffer->setClearStencilValue( 0 );

    Qt3DRender::QCameraSelector *cameraSelector = new Qt3DRender::QCameraSelector( clearStencilBuffer );
    cameraSelector->setCamera( mMainCamera );

    Qt3DRender::QLayerFilter *LayerFilter = new Qt3DRender::QLayerFilter( cameraSelector );
    LayerFilter->addLayer( mHighlightsLayer );

    Qt3DRender::QBlendEquationArguments *blendState = new Qt3DRender::QBlendEquationArguments;
    blendState->setSourceRgb( Qt3DRender::QBlendEquationArguments::SourceAlpha );
    blendState->setDestinationRgb( Qt3DRender::QBlendEquationArguments::OneMinusSourceAlpha );
    stateSet->addRenderState( blendState );

    Qt3DRender::QBlendEquation *blendEquation = new Qt3DRender::QBlendEquation;
    blendEquation->setBlendFunction( Qt3DRender::QBlendEquation::Add );
    stateSet->addRenderState( blendEquation );

    Qt3DRender::QNoDepthMask *noDepthMask = new Qt3DRender::QNoDepthMask;
    stateSet->addRenderState( noDepthMask );

    Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
    depthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
    stateSet->addRenderState( depthTest );

    Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
    cullFace->setMode( Qt3DRender::QCullFace::NoCulling );
    stateSet->addRenderState( cullFace );

    Qt3DRender::QStencilOperation *stencilOp = new Qt3DRender::QStencilOperation;
    stencilOp->front()->setStencilTestFailureOperation( Qt3DRender::QStencilOperationArguments::Keep );
    stencilOp->front()->setDepthTestFailureOperation( Qt3DRender::QStencilOperationArguments::Keep );
    stencilOp->front()->setAllTestsPassOperation( Qt3DRender::QStencilOperationArguments::Replace );
    stencilOp->back()->setStencilTestFailureOperation( Qt3DRender::QStencilOperationArguments::Keep );
    stencilOp->back()->setDepthTestFailureOperation( Qt3DRender::QStencilOperationArguments::Keep );
    stencilOp->back()->setAllTestsPassOperation( Qt3DRender::QStencilOperationArguments::Replace );
    stateSet->addRenderState( stencilOp );

    Qt3DRender::QStencilTest *stencilTest = new Qt3DRender::QStencilTest;
    stencilTest->front()->setStencilFunction( Qt3DRender::QStencilTestArguments::Always );
    stencilTest->front()->setComparisonMask( 0xFF );
    stencilTest->front()->setReferenceValue( 1 );
    stencilTest->back()->setStencilFunction( Qt3DRender::QStencilTestArguments::Always );
    stencilTest->back()->setComparisonMask( 0xFF );
    stencilTest->back()->setReferenceValue( 1 );
    stateSet->addRenderState( stencilTest );
  }

  // Step 2: draw silhouette around highlighted entities (solid color, no transparency)
  //
  // Render the highlights entities offseted by x pixels on each of 4 directions using stencil test
  // to only paint around the existing semi-transparent highlight entities.
  {
    Qt3DRender::QRenderStateSet *stateSet = new Qt3DRender::QRenderStateSet( highlightsRenderTargetSelector );

    // we need to allow writing to the stencil buffer before clearing it
    Qt3DRender::QStencilMask *stencilReadMask = new Qt3DRender::QStencilMask( stateSet );
    stencilReadMask->setFrontOutputMask( 0x00 );
    stencilReadMask->setBackOutputMask( 0x00 );
    stateSet->addRenderState( stencilReadMask );

    Qt3DRender::QNoDepthMask *noDepthMask = new Qt3DRender::QNoDepthMask;
    stateSet->addRenderState( noDepthMask );

    // always pass depth test so we "render on top"
    Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
    depthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
    stateSet->addRenderState( depthTest );

    Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
    cullFace->setMode( Qt3DRender::QCullFace::NoCulling );
    stateSet->addRenderState( cullFace );

    Qt3DRender::QStencilTest *stencilTest = new Qt3DRender::QStencilTest;
    stencilTest->front()->setStencilFunction( Qt3DRender::QStencilTestArguments::Equal );
    stencilTest->front()->setComparisonMask( 0xFF );
    stencilTest->front()->setReferenceValue( 0 );
    stencilTest->back()->setStencilFunction( Qt3DRender::QStencilTestArguments::Equal );
    stencilTest->back()->setComparisonMask( 0xFF );
    stencilTest->back()->setReferenceValue( 0 );
    stateSet->addRenderState( stencilTest );

    Qt3DRender::QCameraSelector *cameraSelector = new Qt3DRender::QCameraSelector( stateSet );
    cameraSelector->setObjectName( "Highlights Pass CameraSelector2" );
    cameraSelector->setCamera( mMainCamera );

    Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter( cameraSelector );
    layerFilter->addLayer( mHighlightsLayer );

    mViewportUp = new Qt3DRender::QViewport( layerFilter );
    mViewportDown = new Qt3DRender::QViewport( layerFilter );
    mViewportLeft = new Qt3DRender::QViewport( layerFilter );
    mViewportRight = new Qt3DRender::QViewport( layerFilter );
  }

  const QVector<Qt3DRender::QRenderTargetOutput *> outputs = mRenderTarget->outputs();
  for ( Qt3DRender::QRenderTargetOutput *o : outputs )
  {
    Qt3DRender::QAbstractTexture *texture = o->texture();
    if ( texture )
    {
      updateViewportSizes( texture->width(), texture->height() );
      break;
    }
  }
}

void QgsHighlightsRenderView::updateViewportSizes( int width, int height )
{
  const float offsetX = silhouetteWidth() / static_cast<float>( width );
  const float offsetY = silhouetteWidth() / static_cast<float>( height );
  mViewportUp->setNormalizedRect( QRectF( offsetX, 0.0f, 1.0f + offsetX, 1.0f ) );
  mViewportDown->setNormalizedRect( QRectF( 0.0f, offsetY, 1.0f, 1.0f + offsetY ) );
  mViewportLeft->setNormalizedRect( QRectF( -offsetX, 0.0f, 1.0f - offsetX, 1.0f ) );
  mViewportRight->setNormalizedRect( QRectF( 0.0f, -offsetY, 1.0f, 1.0f - offsetY ) );
}

void QgsHighlightsRenderView::updateWindowResize( int width, int height )
{
  updateViewportSizes( width, height );
}
