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
#include "moc_qgsshadowrenderview.cpp"
#include "qgsdirectionallightsettings.h"
#include "qgsshadowsettings.h"

#include <Qt3DRender/QCamera>
#include <Qt3DRender/QRenderSurfaceSelector>
#include <Qt3DRender/QViewport>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QFrustumCulling>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QPolygonOffset>
#include <Qt3DRender/qsubtreeenabler.h>

QgsShadowRenderView::QgsShadowRenderView( QObject *parent, const QString &viewName )
  : QgsAbstractRenderView( parent, viewName )
{
  mLightCamera = new Qt3DRender::QCamera;
  mLightCamera->setObjectName( objectName() + "::LightCamera" );
  mEntityCastingShadowsLayer = new Qt3DRender::QLayer;
  mEntityCastingShadowsLayer->setRecursive( true );
  mEntityCastingShadowsLayer->setObjectName( objectName() + "::Layer" );

  // shadow rendering pass
  buildRenderPass();
}

void QgsShadowRenderView::setEnabled( bool enable )
{
  QgsAbstractRenderView::setEnabled( enable );
  mLayerFilter->setEnabled( enable );
}

Qt3DRender::QRenderTarget *QgsShadowRenderView::buildTextures()
{
  mMapTexture = new Qt3DRender::QTexture2D;
  mMapTexture->setWidth( mDefaultMapResolution );
  mMapTexture->setHeight( mDefaultMapResolution );
  mMapTexture->setFormat( Qt3DRender::QTexture2D::TextureFormat::DepthFormat );
  mMapTexture->setGenerateMipMaps( false );
  mMapTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  mMapTexture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  mMapTexture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::ClampToEdge );
  mMapTexture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::ClampToEdge );
  mMapTexture->setObjectName( "QgsShadowRenderView::DepthTarget" );

  Qt3DRender::QRenderTargetOutput *renderTargetOutput = new Qt3DRender::QRenderTargetOutput;
  renderTargetOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
  renderTargetOutput->setTexture( mMapTexture );

  Qt3DRender::QRenderTarget *renderTarget = new Qt3DRender::QRenderTarget;
  renderTarget->setObjectName( objectName() + "::Target" );
  renderTarget->addOutput( renderTargetOutput );

  return renderTarget;
}

void QgsShadowRenderView::buildRenderPass()
{
  // build render pass
  mLightCameraSelector = new Qt3DRender::QCameraSelector( mRendererEnabler );
  mLightCameraSelector->setObjectName( objectName() + "::CameraSelector" );
  mLightCameraSelector->setCamera( mLightCamera );

  mLayerFilter = new Qt3DRender::QLayerFilter( mLightCameraSelector );
  mLayerFilter->addLayer( mEntityCastingShadowsLayer );

  mRenderTargetSelector = new Qt3DRender::QRenderTargetSelector( mLayerFilter );

  mClearBuffers = new Qt3DRender::QClearBuffers( mRenderTargetSelector );
  mClearBuffers->setBuffers( Qt3DRender::QClearBuffers::BufferType::ColorDepthBuffer );
  mClearBuffers->setClearColor( QColor::fromRgbF( 0.0f, 1.0f, 0.0f ) );

  mRenderStateSet = new Qt3DRender::QRenderStateSet( mClearBuffers );

  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::Less );
  mRenderStateSet->addRenderState( depthTest );

  Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
  cullFace->setMode( Qt3DRender::QCullFace::CullingMode::Front );
  mRenderStateSet->addRenderState( cullFace );

  Qt3DRender::QPolygonOffset *polygonOffset = new Qt3DRender::QPolygonOffset;
  polygonOffset->setDepthSteps( 4.0 );
  polygonOffset->setScaleFactor( 1.1 );
  mRenderStateSet->addRenderState( polygonOffset );

  // build texture part
  Qt3DRender::QRenderTarget *renderTarget = buildTextures();

  mRenderTargetSelector->setTarget( renderTarget );
}

Qt3DRender::QLayer *QgsShadowRenderView::entityCastingShadowsLayer() const
{
  return mEntityCastingShadowsLayer;
}

void QgsShadowRenderView::setMapSize( int width, int height )
{
  mMapTexture->setSize( width, height );
}

Qt3DRender::QTexture2D *QgsShadowRenderView::mapTexture() const
{
  return mMapTexture;
}
