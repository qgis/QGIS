/***************************************************************************
  qgsbloomrenderview.cpp
  --------------------------------------
  Date                 : May 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsbloomrenderview.h"

#include "qgsbloomdownsampleentity.h"
#include "qgsbloomupsampleentity.h"
#include "qgsrenderpassquad.h"

#include <QString>
#include <Qt3DRender/QBlendEquation>
#include <Qt3DRender/QBlendEquationArguments>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QFrameGraphNode>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QRenderTargetOutput>
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/qsubtreeenabler.h>

using namespace Qt::StringLiterals;

QgsBloomRenderView::QgsBloomRenderView( const QString &viewName, Qt3DRender::QTexture2D *sourceColorTexture, const QSize &size, Qt3DCore::QEntity *rootEntity )
  : QgsAbstractRenderView( viewName )
  , mBaseSize( size )
{
  mFilterRadiusParameter = new Qt3DRender::QParameter( u"filterRadius"_s, 0.005f, rootEntity );

  const float aspectRatio = static_cast<float>( size.width() ) / static_cast<float>( size.height() );
  mAspectRatioParameter = new Qt3DRender::QParameter( u"aspectRatio"_s, aspectRatio, rootEntity );

  // bloom is disabled by default
  QgsAbstractRenderView::setEnabled( false );
  buildRenderPasses( sourceColorTexture, rootEntity );
}

QgsBloomRenderView::~QgsBloomRenderView() = default;

Qt3DRender::QTexture2D *QgsBloomRenderView::bloomTexture() const
{
  return mTextures[0];
}

void QgsBloomRenderView::setFilterRadius( float radius )
{
  mFilterRadiusParameter->setValue( radius );
}

void QgsBloomRenderView::setAspectRatio( float ratio )
{
  mAspectRatioParameter->setValue( ratio );
}

void QgsBloomRenderView::buildRenderPasses( Qt3DRender::QTexture2D *sourceTexture, Qt3DCore::QEntity *rootEntity )
{
  Qt3DRender::QTexture2D *currentInputTexture = sourceTexture;

  // following CoD: Advanced Warfare approach, from ACM Siggraph 2014
  // see also https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom by Jorge Jimenez
  // comments from Jorge's post inline below

  mTextures.reserve( MIP_PASSES );

  Qt3DRender::QRenderStateSet *renderStateSet = new Qt3DRender::QRenderStateSet( mRendererEnabler );

  // downsample
  for ( int i = 0; i < MIP_PASSES; ++i )
  {
    auto passLayer = new Qt3DRender::QLayer( rootEntity );
    passLayer->setRecursive( true );
    passLayer->setObjectName( mViewName + u"::Layer(DownsamplePass%1)"_s.arg( i + 1 ) );

    auto renderTarget = new Qt3DRender::QRenderTarget();

    auto colorOutput = new Qt3DRender::QRenderTargetOutput( renderTarget );
    colorOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );

    auto mipTexture = new Qt3DRender::QTexture2D( colorOutput );
    // "we are downscaling an HDR color buffer, so we need a float texture format" (Jorge Jimenez)
    mipTexture->setFormat( Qt3DRender::QAbstractTexture::RG11B10F );
    // minimize VRAM during build -- if the effect is disabled, we don't want to waste resources
    mipTexture->setSize( 1, 1 );
    mipTexture->setGenerateMipMaps( false );
    // "we must enable linear filtering and edge-clamping for all mips" (Jorge Jimenez)
    mipTexture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
    mipTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
    mipTexture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::ClampToEdge );
    mipTexture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::ClampToEdge );
    mTextures.push_back( mipTexture );

    colorOutput->setTexture( mipTexture );
    renderTarget->addOutput( colorOutput );

    auto targetSelector = new Qt3DRender::QRenderTargetSelector( renderStateSet );
    targetSelector->setTarget( renderTarget );

    auto clearBuffers = new Qt3DRender::QClearBuffers( targetSelector );
    clearBuffers->setBuffers( Qt3DRender::QClearBuffers::ColorBuffer );
    clearBuffers->setClearColor( QColor::fromRgbF( 0.0, 0.0, 0.0, 1.0 ) );

    auto layerFilter = new Qt3DRender::QLayerFilter( clearBuffers );
    layerFilter->addLayer( passLayer );

    // owned by rootEntity
    ( void ) new QgsBloomDownsampleEntity( currentInputTexture, passLayer, rootEntity );

    currentInputTexture = mipTexture;
  }

  // upsample
  for ( int i = MIP_PASSES - 2; i >= 0; --i )
  {
    auto passLayer = new Qt3DRender::QLayer( rootEntity );
    passLayer->setRecursive( true );
    passLayer->setObjectName( mViewName + u"::Layer(UpsamplePass%1)"_s.arg( i + 1 ) );

    Qt3DRender::QRenderTarget *renderTarget = new Qt3DRender::QRenderTarget();

    Qt3DRender::QRenderTargetOutput *colorOutput = new Qt3DRender::QRenderTargetOutput( renderTarget );
    colorOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );

    Qt3DRender::QTexture2D *targetTexture = mTextures[i];
    colorOutput->setTexture( targetTexture );
    renderTarget->addOutput( colorOutput );

    auto targetSelector = new Qt3DRender::QRenderTargetSelector( renderStateSet );
    targetSelector->setTarget( renderTarget );

    // upsample is additive
    auto blendStateSet = new Qt3DRender::QRenderStateSet( targetSelector );

    auto blendEquation = new Qt3DRender::QBlendEquation( blendStateSet );
    blendEquation->setBlendFunction( Qt3DRender::QBlendEquation::Add );
    blendStateSet->addRenderState( blendEquation );

    auto blendEquationArgs = new Qt3DRender::QBlendEquationArguments( blendStateSet );
    blendEquationArgs->setSourceRgb( Qt3DRender::QBlendEquationArguments::Blending::One );
    blendEquationArgs->setDestinationRgb( Qt3DRender::QBlendEquationArguments::Blending::One );
    blendStateSet->addRenderState( blendEquationArgs );

    // owned by rootEntity
    auto quad = new QgsBloomUpsampleEntity( currentInputTexture, passLayer, rootEntity );
    quad->addParameters( { mFilterRadiusParameter, mAspectRatioParameter } );

    auto layerFilter = new Qt3DRender::QLayerFilter( blendStateSet );
    layerFilter->addLayer( passLayer );

    currentInputTexture = targetTexture;
  }
}

void QgsBloomRenderView::updateWindowResize( int width, int height )
{
  mBaseSize = QSize( width, height );

  const bool enabled = isEnabled();

  int currentWidth = width;
  int currentHeight = height;
  for ( int i = 0; i < MIP_PASSES; ++i )
  {
    // "If you go too small, make sure that you stop when you reach 1x1 in mip size" (Jorge Jimenez)
    currentWidth = std::max( 1, currentWidth / 2 );
    currentHeight = std::max( 1, currentHeight / 2 );
    if ( enabled )
    {
      mTextures[i]->setSize( currentWidth, currentHeight );
    }
    else
    {
      // minimize VRAM when disabled
      mTextures[i]->setSize( 1, 1 );
    }
  }

  const float aspectRatio = static_cast<float>( width ) / static_cast<float>( height );
  mAspectRatioParameter->setValue( aspectRatio );
}

void QgsBloomRenderView::setEnabled( bool enabled )
{
  QgsAbstractRenderView::setEnabled( enabled );
  updateWindowResize( mBaseSize.width(), mBaseSize.height() );
}
