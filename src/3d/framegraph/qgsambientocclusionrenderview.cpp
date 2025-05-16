/***************************************************************************
  qgsambientocclusionrenderview.cpp
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

#include "qgsambientocclusionrenderview.h"
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/qsubtreeenabler.h>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QCullFace>

#include "qgsambientocclusionrenderentity.h"
#include "qgsambientocclusionblurentity.h"


QgsAmbientOcclusionRenderView::QgsAmbientOcclusionRenderView( const QString &viewName, Qt3DRender::QCamera *mainCamera, QSize mSize, Qt3DRender::QTexture2D *forwardDepthTexture, Qt3DCore::QEntity *rootSceneEntity )
  : QgsAbstractRenderView( viewName )
{
  mAOPassLayer = new Qt3DRender::QLayer;
  mAOPassLayer->setRecursive( true );
  mAOPassLayer->setObjectName( mViewName + "::Layer(AO)" );

  mBlurPassLayer = new Qt3DRender::QLayer;
  mBlurPassLayer->setRecursive( true );
  mBlurPassLayer->setObjectName( mViewName + "::Layer(Blur)" );

  // ambient occlusion rendering pass
  buildRenderPasses( mSize, forwardDepthTexture, rootSceneEntity, mainCamera );
}

void QgsAmbientOcclusionRenderView::updateWindowResize( int width, int height )
{
  mAOPassTexture->setSize( width, height );
  mBlurPassTexture->setSize( width, height );
}

void QgsAmbientOcclusionRenderView::setEnabled( bool enable )
{
  QgsAbstractRenderView::setEnabled( enable );
  mAmbientOcclusionRenderEntity->setEnabled( enable );
  mAmbientOcclusionBlurEntity->setEnabled( enable );
}

Qt3DRender::QRenderTarget *QgsAmbientOcclusionRenderView::buildAOTexture( QSize mSize )
{
  // Create a texture to render into.
  Qt3DRender::QRenderTargetOutput *colorTargetOutput = new Qt3DRender::QRenderTargetOutput;
  colorTargetOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );

  mAOPassTexture = new Qt3DRender::QTexture2D( colorTargetOutput );
  mAOPassTexture->setSize( mSize.width(), mSize.height() );
  mAOPassTexture->setFormat( Qt3DRender::QAbstractTexture::R32F );
  mAOPassTexture->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mAOPassTexture->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mAOPassTexture->setObjectName( mViewName + "::ColorTarget(AO)" );
  colorTargetOutput->setTexture( mAOPassTexture );

  Qt3DRender::QRenderTarget *renderTarget = new Qt3DRender::QRenderTarget;
  renderTarget->addOutput( colorTargetOutput );

  return renderTarget;
}

Qt3DRender::QRenderTarget *QgsAmbientOcclusionRenderView::buildBlurTexture( QSize mSize )
{
  // Create a texture to render into.
  Qt3DRender::QRenderTargetOutput *colorTargetOutput = new Qt3DRender::QRenderTargetOutput;
  colorTargetOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );

  mBlurPassTexture = new Qt3DRender::QTexture2D( colorTargetOutput );
  mBlurPassTexture->setSize( mSize.width(), mSize.height() );
  mBlurPassTexture->setFormat( Qt3DRender::QAbstractTexture::R32F );
  mBlurPassTexture->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mBlurPassTexture->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mBlurPassTexture->setObjectName( mViewName + "::ColorTarget(blur)" );
  colorTargetOutput->setTexture( mBlurPassTexture );

  Qt3DRender::QRenderTarget *renderTarget = new Qt3DRender::QRenderTarget;
  renderTarget->addOutput( colorTargetOutput );

  return renderTarget;
}

void QgsAmbientOcclusionRenderView::buildRenderPasses( QSize mSize, Qt3DRender::QTexture2D *forwardDepthTexture, Qt3DCore::QEntity *rootSceneEntity, Qt3DRender::QCamera *mainCamera )
{
  // AO pass
  {
    Qt3DRender::QRenderStateSet *renderStateSet = new Qt3DRender::QRenderStateSet( mRendererEnabler );

    Qt3DRender::QDepthTest *depthRenderDepthTest = new Qt3DRender::QDepthTest;
    depthRenderDepthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
    Qt3DRender::QCullFace *depthRenderCullFace = new Qt3DRender::QCullFace;
    depthRenderCullFace->setMode( Qt3DRender::QCullFace::NoCulling );

    renderStateSet->addRenderState( depthRenderDepthTest );
    renderStateSet->addRenderState( depthRenderCullFace );

    Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter( renderStateSet );
    layerFilter->addLayer( mAOPassLayer );

    Qt3DRender::QRenderTarget *renderTarget = buildAOTexture( mSize );

    Qt3DRender::QRenderTargetSelector *renderTargetSelector = new Qt3DRender::QRenderTargetSelector( layerFilter );
    renderTargetSelector->setObjectName( mViewName + "::RenderTargetSelector(AO)" );
    renderTargetSelector->setTarget( renderTarget );

    mAmbientOcclusionRenderEntity = new QgsAmbientOcclusionRenderEntity( forwardDepthTexture, mAOPassLayer, mainCamera, rootSceneEntity );
  }

  // blur pass
  {
    Qt3DRender::QRenderStateSet *renderStateSet = new Qt3DRender::QRenderStateSet( mRendererEnabler );

    Qt3DRender::QDepthTest *depthRenderDepthTest = new Qt3DRender::QDepthTest;
    depthRenderDepthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
    Qt3DRender::QCullFace *depthRenderCullFace = new Qt3DRender::QCullFace;
    depthRenderCullFace->setMode( Qt3DRender::QCullFace::NoCulling );

    renderStateSet->addRenderState( depthRenderDepthTest );
    renderStateSet->addRenderState( depthRenderCullFace );

    Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter( renderStateSet );
    layerFilter->addLayer( mBlurPassLayer );

    Qt3DRender::QRenderTarget *renderTarget = buildBlurTexture( mSize );

    Qt3DRender::QRenderTargetSelector *renderTargetSelector = new Qt3DRender::QRenderTargetSelector( layerFilter );
    renderTargetSelector->setObjectName( mViewName + "::RenderTargetSelector(Blur)" );
    renderTargetSelector->setTarget( renderTarget );

    mAmbientOcclusionBlurEntity = new QgsAmbientOcclusionBlurEntity( mAOPassTexture, mBlurPassLayer, rootSceneEntity );
  }
}

void QgsAmbientOcclusionRenderView::setIntensity( float intensity )
{
  mAmbientOcclusionRenderEntity->setIntensity( intensity );
}

void QgsAmbientOcclusionRenderView::setRadius( float radius )
{
  mAmbientOcclusionRenderEntity->setRadius( radius );
}

void QgsAmbientOcclusionRenderView::setThreshold( float threshold )
{
  mAmbientOcclusionRenderEntity->setThreshold( threshold );
}

Qt3DRender::QTexture2D *QgsAmbientOcclusionRenderView::blurredFactorMapTexture() const
{
  return mBlurPassTexture;
}
