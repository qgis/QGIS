/***************************************************************************
  qgsshadowrenderview.cpp
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

#include "qgsshadowrenderview.h"

#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QFrustumCulling>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QPolygonOffset>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QRenderSurfaceSelector>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QViewport>
#include <Qt3DRender/qsubtreeenabler.h>

QgsShadowRenderView::QgsShadowRenderView( const QString &viewName, Qt3DCore::QEntity *rootEntity )
  : QgsAbstractRenderView( viewName )
  , mRootEntity( rootEntity )
{
  mEntityCastingShadowsLayer = new Qt3DRender::QLayer;
  mEntityCastingShadowsLayer->setRecursive( true );
  mEntityCastingShadowsLayer->setObjectName( mViewName + "::Layer" );

  // shadow rendering pass
  buildRenderPass();
}

void QgsShadowRenderView::setEnabled( bool enable )
{
  QgsAbstractRenderView::setEnabled( enable );
  for ( int i = 0; i < Qgs3D::NUM_SHADOW_CASCADES; ++i )
  {
    if ( mLayerFilters[i] )
      mLayerFilters[i]->setEnabled( enable );
  }
}

Qt3DRender::QCamera *QgsShadowRenderView::lightCamera( int index )
{
  return mLightCameras[index];
}

void QgsShadowRenderView::buildRenderPass()
{
  mMapTextureArray = new Qt3DRender::QTexture2DArray;
  mMapTextureArray->setWidth( mDefaultMapResolution );
  mMapTextureArray->setHeight( mDefaultMapResolution );
  mMapTextureArray->setLayers( Qgs3D::NUM_SHADOW_CASCADES );
  mMapTextureArray->setFormat( Qt3DRender::QAbstractTexture::TextureFormat::DepthFormat );
  mMapTextureArray->setGenerateMipMaps( false );
  mMapTextureArray->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mMapTextureArray->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mMapTextureArray->wrapMode()->setX( Qt3DRender::QTextureWrapMode::ClampToEdge );
  mMapTextureArray->wrapMode()->setY( Qt3DRender::QTextureWrapMode::ClampToEdge );
  mMapTextureArray->setObjectName( mViewName + "::MapTextureArray" );

  for ( int i = 0; i < Qgs3D::NUM_SHADOW_CASCADES; ++i )
  {
    mLightCameras[i] = new Qt3DRender::QCamera( mRootEntity );
    mLightCameras[i]->setObjectName( mViewName + QString( "::LightCamera_%1" ).arg( i ) );

    Qt3DRender::QCameraSelector *lightCameraSelector = new Qt3DRender::QCameraSelector( mRendererEnabler );
    lightCameraSelector->setObjectName( mViewName + QString( "::CameraSelector_%1" ).arg( i ) );
    lightCameraSelector->setCamera( mLightCameras[i] );

    mLayerFilters[i] = new Qt3DRender::QLayerFilter( lightCameraSelector );
    mLayerFilters[i]->addLayer( mEntityCastingShadowsLayer );

    Qt3DRender::QRenderTargetSelector *renderTargetSelector = new Qt3DRender::QRenderTargetSelector( mLayerFilters[i] );

    Qt3DRender::QClearBuffers *clearBuffers = new Qt3DRender::QClearBuffers( renderTargetSelector );
    clearBuffers->setBuffers( Qt3DRender::QClearBuffers::BufferType::DepthBuffer );

    Qt3DRender::QRenderStateSet *renderStateSet = new Qt3DRender::QRenderStateSet( clearBuffers );

    Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
    depthTest->setDepthFunction( Qt3DRender::QDepthTest::Less );
    renderStateSet->addRenderState( depthTest );

    Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
    cullFace->setMode( Qt3DRender::QCullFace::CullingMode::Front );
    renderStateSet->addRenderState( cullFace );

    Qt3DRender::QPolygonOffset *polygonOffset = new Qt3DRender::QPolygonOffset;
    polygonOffset->setDepthSteps( 4.0 );
    polygonOffset->setScaleFactor( 1.1 );
    renderStateSet->addRenderState( polygonOffset );

    Qt3DRender::QRenderTargetOutput *output = new Qt3DRender::QRenderTargetOutput;
    output->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
    output->setTexture( mMapTextureArray );
    output->setLayer( i );

    Qt3DRender::QRenderTarget *renderTarget = new Qt3DRender::QRenderTarget;
    renderTarget->setObjectName( mViewName + QString( "::RenderTarget_%1" ).arg( i ) );
    renderTarget->addOutput( output );

    renderTargetSelector->setTarget( renderTarget );
  }
}

Qt3DRender::QLayer *QgsShadowRenderView::entityCastingShadowsLayer() const
{
  return mEntityCastingShadowsLayer;
}

void QgsShadowRenderView::setMapSize( int width, int height )
{
  mMapTextureArray->setSize( width, height );
}

Qt3DRender::QTexture2DArray *QgsShadowRenderView::mapTextureArray() const
{
  return mMapTextureArray;
}
