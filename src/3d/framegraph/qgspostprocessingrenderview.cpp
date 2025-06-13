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
#include <QNode>
#include "qgspostprocessingentity.h"
#include "qgsforwardrenderview.h"
#include "qgsshadowrenderview.h"

QgsPostprocessingRenderView::QgsPostprocessingRenderView( const QString &viewName,                     //
                                                          QgsShadowRenderView &shadowRenderView,       //
                                                          QgsForwardRenderView &forwardRenderView,     //
                                                          QgsAmbientOcclusionRenderView &aoRenderView, //
                                                          QSize mSize,                                 //
                                                          Qt3DCore::QEntity *rootSceneEntity )
  : QgsAbstractRenderView( viewName )
{
  // postprocessing main rendering pass
  constructPostprocessingMainPass( mSize );

  // sub passes:
  QVector<Qt3DRender::QFrameGraphNode *> subpasses;
  // 1. postprocessing real render view
  subpasses << constructSubPostPassForProcessing( shadowRenderView, forwardRenderView, aoRenderView, rootSceneEntity );
  // 2. render capture render view
  subpasses << constructSubPostPassForRenderCapture();

  setSubPasses( subpasses );
}

QVector<Qt3DRender::QFrameGraphNode *> QgsPostprocessingRenderView::subPasses() const
{
  QVector<Qt3DRender::QFrameGraphNode *> out;
  Qt3DCore::QNodeVector children = mSubPassesNode->childNodes();
  for ( Qt3DCore::QNode *child : children )
    if ( dynamic_cast<Qt3DRender::QFrameGraphNode *>( child ) && child->parent() == mSubPassesNode )
      out << dynamic_cast<Qt3DRender::QFrameGraphNode *>( child );
  return out;
}

void QgsPostprocessingRenderView::setSubPasses( QVector<Qt3DRender::QFrameGraphNode *> topNodes )
{
  // detach all subpasses
  Qt3DCore::QNodeVector children = mSubPassesNode->childNodes();
  for ( Qt3DCore::QNode *child : children )
    if ( dynamic_cast<Qt3DRender::QFrameGraphNode *>( child ) && child->parent() == mSubPassesNode )
      child->setParent( ( Qt3DCore::QNode * ) nullptr );

  // attach new subpasses
  for ( Qt3DRender::QFrameGraphNode *child : topNodes )
    child->setParent( mSubPassesNode );
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

Qt3DRender::QFrameGraphNode *QgsPostprocessingRenderView::constructPostprocessingMainPass( QSize mSize )
{
  // Due to a bug in Qt5 (fixed in Qt6 - https://codereview.qt-project.org/c/qt/qt3d/+/462575) we need to move the render target selector at the top
  // of this branch. Doing so this allows Qt3d to have a FBO format matching the one need to do the capture
  mRenderCaptureTargetSelector = new Qt3DRender::QRenderTargetSelector( mRendererEnabler );
  mRenderCaptureTargetSelector->setObjectName( mViewName + "::RenderTargetSelector" );
  mRenderCaptureTargetSelector->setEnabled( false );

  // =============== v NEED ONLY DURING UNIT TEST v
  Qt3DRender::QRenderTarget *renderTarget = new Qt3DRender::QRenderTarget( mRenderCaptureTargetSelector );

  // The lifetime of the objects created here is managed
  // automatically, as they become children of this object.

  // Create a render target output for rendering color.
  Qt3DRender::QRenderTargetOutput *colorOutput = new Qt3DRender::QRenderTargetOutput( renderTarget );
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

  Qt3DRender::QRenderTargetOutput *depthOutput = new Qt3DRender::QRenderTargetOutput( renderTarget );

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

  mRenderCaptureTargetSelector->setTarget( renderTarget );
  // =============== ^ NEED ONLY DURING UNIT TEST ^

  mSubPassesNode = new Qt3DRender::QFrameGraphNode( mRenderCaptureTargetSelector );
  mSubPassesNode->setObjectName( mViewName + "::Sub passes top node" );

  return mRenderCaptureTargetSelector;
}


Qt3DRender::QFrameGraphNode *QgsPostprocessingRenderView::constructSubPostPassForProcessing( QgsShadowRenderView &shadowRenderView,       //
                                                                                             QgsForwardRenderView &forwardRenderView,     //
                                                                                             QgsAmbientOcclusionRenderView &aoRenderView, //
                                                                                             Qt3DCore::QEntity *rootSceneEntity )
{
  Qt3DRender::QCameraSelector *cameraSelector = new Qt3DRender::QCameraSelector;
  cameraSelector->setObjectName( mViewName + "::Sub pass::Postprocessing" );
  cameraSelector->setCamera( shadowRenderView.lightCamera() );

  Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter( cameraSelector );

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

  return cameraSelector;
}

Qt3DRender::QFrameGraphNode *QgsPostprocessingRenderView::constructSubPostPassForRenderCapture()
{
  Qt3DRender::QFrameGraphNode *top = new Qt3DRender::QNoDraw;
  top->setObjectName( mViewName + "::Sub pass::RenderCapture" );

  mRenderCapture = new Qt3DRender::QRenderCapture( top );

  return top;
}

Qt3DRender::QRenderCapture *QgsPostprocessingRenderView::renderCapture() const
{
  return mRenderCapture;
}

QgsPostprocessingEntity *QgsPostprocessingRenderView::entity() const
{
  return mPostprocessingEntity;
}
