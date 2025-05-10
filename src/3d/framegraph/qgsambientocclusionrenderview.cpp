/***************************************************************************
  qgsambientocclusionrenderview.cpp
  --------------------------------------
  Date                 : June 2024
  Copyright            : (C) 2024 by Benoit De Mezzo and (C) 2020 by Belgacem Nedjima
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
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/qsubtreeenabler.h>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QCullFace>


QgsAmbientOcclusionRenderView::QgsAmbientOcclusionRenderView( const QString &viewName, Qt3DRender::QCamera *mainCamera, QSize mSize, Qt3DRender::QTexture2D *forwardDepthTexture, Qt3DCore::QEntity *rootSceneEntity )
  : QgsAbstractRenderView( viewName )
  , mMainCamera( mainCamera )
{
  mAOPassLayer = new Qt3DRender::QLayer;
  mAOPassLayer->setRecursive( true );
  mAOPassLayer->setObjectName( mViewName + "::Layer(AO)" );

  mBlurPassLayer = new Qt3DRender::QLayer;
  mBlurPassLayer->setRecursive( true );
  mBlurPassLayer->setObjectName( mViewName + "::Layer(Blur)" );

  // ambient occlusion rendering pass
  buildRenderPass( mSize, forwardDepthTexture, rootSceneEntity );
}

void QgsAmbientOcclusionRenderView::updateWindowResize( int width, int height )
{
  mAOPassTexture->setSize( width, height );
  mBlurPassTexture->setSize( width, height );
}

void QgsAmbientOcclusionRenderView::setEnabled( bool enable )
{
  QgsAbstractRenderView::setEnabled( enable );
  if ( mAmbientOcclusionRenderEntity != nullptr )
    mAmbientOcclusionRenderEntity->setEnabled( enable );
  if ( mAmbientOcclusionBlurEntity != nullptr )
    mAmbientOcclusionBlurEntity->setEnabled( enable );
}

Qt3DRender::QRenderTarget *QgsAmbientOcclusionRenderView::buildTextures( QSize mSize )
{
  // Create a texture to render into.
  Qt3DRender::QRenderTargetOutput *colorTargetOutput = new Qt3DRender::QRenderTargetOutput;
  colorTargetOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );

  mBlurPassTexture = new Qt3DRender::QTexture2D( colorTargetOutput );
  mBlurPassTexture->setSize( mSize.width(), mSize.height() );
  mBlurPassTexture->setFormat( Qt3DRender::QAbstractTexture::R32F );
  mBlurPassTexture->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mBlurPassTexture->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mBlurPassTexture->setObjectName( mViewName + "::ColorTarget" );
  colorTargetOutput->setTexture( mBlurPassTexture );

  Qt3DRender::QRenderTarget *renderTarget = new Qt3DRender::QRenderTarget;
  renderTarget->addOutput( colorTargetOutput );

  return renderTarget;
}

void QgsAmbientOcclusionRenderView::buildRenderPass( QSize mSize, Qt3DRender::QTexture2D *forwardDepthTexture, Qt3DCore::QEntity *rootSceneEntity )
{
  // AO pass
  {
    Qt3DRender::QCameraSelector *cameraSelector = new Qt3DRender::QCameraSelector( mRendererEnabler );
    cameraSelector->setObjectName( mViewName + "::CameraSelector(AO)" );
    cameraSelector->setCamera( mMainCamera );

    Qt3DRender::QRenderStateSet *renderStateSet = new Qt3DRender::QRenderStateSet( cameraSelector );

    Qt3DRender::QDepthTest *depthRenderDepthTest = new Qt3DRender::QDepthTest;
    depthRenderDepthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
    Qt3DRender::QCullFace *depthRenderCullFace = new Qt3DRender::QCullFace;
    depthRenderCullFace->setMode( Qt3DRender::QCullFace::NoCulling );

    renderStateSet->addRenderState( depthRenderDepthTest );
    renderStateSet->addRenderState( depthRenderCullFace );

    Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter( renderStateSet );
    layerFilter->addLayer( mAOPassLayer );

    Qt3DRender::QRenderTargetSelector *renderTargetSelector = new Qt3DRender::QRenderTargetSelector( layerFilter );
    renderTargetSelector->setObjectName( mViewName + "::RenderTargetSelector(AO)" );

    // create intermediate texture
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
    renderTarget->setObjectName( mViewName + "::Target(AO)" );
    renderTarget->addOutput( colorTargetOutput );

    renderTargetSelector->setTarget( renderTarget );

    mAmbientOcclusionRenderEntity = new QgsAmbientOcclusionRenderEntity( forwardDepthTexture, mAOPassLayer, mMainCamera, rootSceneEntity );
  }

  // blur pass
  {
    Qt3DRender::QCameraSelector *cameraSelector = new Qt3DRender::QCameraSelector( mRendererEnabler );
    cameraSelector->setObjectName( mViewName + "::CameraSelector(Blur)" );
    cameraSelector->setCamera( mMainCamera );

    Qt3DRender::QRenderStateSet *renderStateSet = new Qt3DRender::QRenderStateSet( cameraSelector );

    Qt3DRender::QDepthTest *depthRenderDepthTest = new Qt3DRender::QDepthTest;
    depthRenderDepthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
    Qt3DRender::QCullFace *depthRenderCullFace = new Qt3DRender::QCullFace;
    depthRenderCullFace->setMode( Qt3DRender::QCullFace::NoCulling );

    renderStateSet->addRenderState( depthRenderDepthTest );
    renderStateSet->addRenderState( depthRenderCullFace );

    Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter( renderStateSet );
    layerFilter->addLayer( mBlurPassLayer );

    Qt3DRender::QRenderTarget *renderTarget = buildTextures( mSize );

    mRenderTargetSelector = new Qt3DRender::QRenderTargetSelector( layerFilter );
    mRenderTargetSelector->setObjectName( mViewName + "::RenderTargetSelector(Blur)" );
    mRenderTargetSelector->setTarget( renderTarget );

    mAmbientOcclusionBlurEntity = new QgsAmbientOcclusionBlurEntity( mAOPassTexture, mBlurPassLayer, rootSceneEntity );
  }
}

void QgsAmbientOcclusionRenderView::setIntensity( float intensity )
{
  if ( mAmbientOcclusionRenderEntity != nullptr )
  {
    mAmbientOcclusionRenderEntity->setIntensity( intensity );
  }
}

void QgsAmbientOcclusionRenderView::setRadius( float radius )
{
  if ( mAmbientOcclusionRenderEntity != nullptr )
  {
    mAmbientOcclusionRenderEntity->setRadius( radius );
  }
}

void QgsAmbientOcclusionRenderView::setThreshold( float threshold )
{
  if ( mAmbientOcclusionRenderEntity != nullptr )
  {
    mAmbientOcclusionRenderEntity->setThreshold( threshold );
  }
}

Qt3DRender::QTexture2D *QgsAmbientOcclusionRenderView::blurredFactorMapTexture() const
{
  return mBlurPassTexture;
}
