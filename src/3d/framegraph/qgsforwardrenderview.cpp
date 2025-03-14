/***************************************************************************
  qgsforwardrenderview.cpp
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

#include "qgsforwardrenderview.h"
#include <Qt3DRender/QCamera>
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
#include <Qt3DRender/QBlendEquation>
#include <Qt3DRender/QColorMask>
#include <Qt3DRender/QSortPolicy>
#include <Qt3DRender/QNoDepthMask>
#include <Qt3DRender/QBlendEquationArguments>
#include <Qt3DRender/QClipPlane>

#if QT_VERSION >= QT_VERSION_CHECK( 5, 15, 0 )
#include <Qt3DRender/QDebugOverlay>
#endif

QgsForwardRenderView::QgsForwardRenderView( const QString &viewName, Qt3DRender::QCamera *mainCamera )
  : QgsAbstractRenderView( viewName )
  , mMainCamera( mainCamera )
{
  mRenderLayer = new Qt3DRender::QLayer;
  mRenderLayer->setRecursive( true );
  mRenderLayer->setObjectName( mViewName + "::Layer" );

  mTransparentObjectsLayer = new Qt3DRender::QLayer;
  mTransparentObjectsLayer->setRecursive( true );
  mTransparentObjectsLayer->setObjectName( mViewName + "::TransparentLayer" );

  // forward rendering pass
  buildRenderPasses();
}

Qt3DRender::QRenderTarget *QgsForwardRenderView::buildTextures()
{
  mColorTexture = new Qt3DRender::QTexture2D;
  mColorTexture->setFormat( Qt3DRender::QAbstractTexture::RGB8_UNorm );
  mColorTexture->setGenerateMipMaps( false );
  mColorTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  mColorTexture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  mColorTexture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::ClampToEdge );
  mColorTexture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::ClampToEdge );

  mDepthTexture = new Qt3DRender::QTexture2D;
  mDepthTexture->setFormat( Qt3DRender::QTexture2D::TextureFormat::DepthFormat );
  mDepthTexture->setGenerateMipMaps( false );
  mDepthTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  mDepthTexture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  mDepthTexture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::ClampToEdge );
  mDepthTexture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::ClampToEdge );

  Qt3DRender::QRenderTarget *renderTarget = new Qt3DRender::QRenderTarget;
  Qt3DRender::QRenderTargetOutput *renderTargetDepthOutput = new Qt3DRender::QRenderTargetOutput;
  renderTargetDepthOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
  renderTargetDepthOutput->setTexture( mDepthTexture );
  renderTarget->addOutput( renderTargetDepthOutput );

  Qt3DRender::QRenderTargetOutput *renderTargetColorOutput = new Qt3DRender::QRenderTargetOutput;
  renderTargetColorOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );
  renderTargetColorOutput->setTexture( mColorTexture );
  renderTarget->addOutput( renderTargetColorOutput );

  return renderTarget;
}

/*
 * We define three forward passes: one for solid objects, followed by two for transparent objects (one to write colors but no depths, one to write depths) :
 *
 *                                  |
 *                         +-----------------+
 *                         | QCameraSelector |  (using the main camera)
 *                         +-----------------+
 *                                  |
 *                         +-----------------+
 *                         |  QLayerFilter   |  (using mForwardRenderLayer)
 *                         +-----------------+
 *                                  |
 *                         +-----------------+
 *                         | QRenderStateSet |  define clip planes
 *                         +-----------------+
 *                                  |
 *                      +-----------------------+
 *                      | QRenderTargetSelector | (write mForwardColorTexture + mForwardDepthTexture)
 *                      +-----------------------+
 *                                  |
 *         +------------------------+---------------------+
 *         |                                              |
 *  +-----------------+    discard               +-----------------+    accept
 *  |  QLayerFilter   |  transparent             |  QLayerFilter   |  transparent
 *  +-----------------+    objects               +-----------------+    objects
 *         |                                              |
 *  +-----------------+  use depth test          +-----------------+   sort entities
 *  | QRenderStateSet |  cull back faces         |  QSortPolicy    |  back to front
 *  +-----------------+                          +-----------------+
 *         |                                              |
 *  +-----------------+              +--------------------+--------------------+
 *  | QFrustumCulling |              |                                         |
 *  +-----------------+     +-----------------+  use depth tests      +-----------------+  use depth tests
 *         |                | QRenderStateSet |  don't write depths   | QRenderStateSet |  write depths
 *         |                +-----------------+  write colors         +-----------------+  don't write colors
 *  +-----------------+                          use alpha blending                        don't use alpha blending
 *  |  QClearBuffers  |  color and depth         no culling                                no culling
 *  +-----------------+
 *         |
 *  +-----------------+
 *  |  QDebugOverlay  |
 *  +-----------------+
 *
 */
void QgsForwardRenderView::buildRenderPasses()
{
  mMainCameraSelector = new Qt3DRender::QCameraSelector( mRendererEnabler );
  mMainCameraSelector->setObjectName( mViewName + "::CameraSelector" );
  mMainCameraSelector->setCamera( mMainCamera );

  mLayerFilter = new Qt3DRender::QLayerFilter( mMainCameraSelector );
  mLayerFilter->addLayer( mRenderLayer );

  mClipRenderStateSet = new Qt3DRender::QRenderStateSet( mLayerFilter );
  mClipRenderStateSet->setObjectName( mViewName + "::Clip Plane RenderStateSet" );

  Qt3DRender::QRenderTarget *renderTarget = buildTextures();

  mRenderTargetSelector = new Qt3DRender::QRenderTargetSelector( mClipRenderStateSet );
  mRenderTargetSelector->setTarget( renderTarget );

  // first branch: opaque layer filter
  Qt3DRender::QLayerFilter *opaqueObjectsFilter = new Qt3DRender::QLayerFilter( mRenderTargetSelector );
  opaqueObjectsFilter->addLayer( mTransparentObjectsLayer );
  opaqueObjectsFilter->setFilterMode( Qt3DRender::QLayerFilter::DiscardAnyMatchingLayers );

  Qt3DRender::QRenderStateSet *renderStateSet = new Qt3DRender::QRenderStateSet( opaqueObjectsFilter );

  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::Less );
  renderStateSet->addRenderState( depthTest );

  Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
  cullFace->setMode( Qt3DRender::QCullFace::CullingMode::Back );
  renderStateSet->addRenderState( cullFace );

  mFrustumCulling = new Qt3DRender::QFrustumCulling( renderStateSet );

  mClearBuffers = new Qt3DRender::QClearBuffers( mFrustumCulling );
  mClearBuffers->setClearColor( QColor::fromRgbF( 0.0, 0.0, 1.0, 1.0 ) );
  mClearBuffers->setBuffers( Qt3DRender::QClearBuffers::ColorDepthBuffer );
  mClearBuffers->setClearDepthValue( 1.0f );

  // second branch: transparent layer filter - color
  Qt3DRender::QLayerFilter *transparentObjectsLayerFilter = new Qt3DRender::QLayerFilter( mRenderTargetSelector );
  transparentObjectsLayerFilter->addLayer( mTransparentObjectsLayer );
  transparentObjectsLayerFilter->setFilterMode( Qt3DRender::QLayerFilter::AcceptAnyMatchingLayers );

  Qt3DRender::QSortPolicy *sortPolicy = new Qt3DRender::QSortPolicy( transparentObjectsLayerFilter );
  QVector<Qt3DRender::QSortPolicy::SortType> sortTypes;
  sortTypes.push_back( Qt3DRender::QSortPolicy::BackToFront );
  sortPolicy->setSortTypes( sortTypes );

  Qt3DRender::QRenderStateSet *transparentObjectsRenderStateSetColor = new Qt3DRender::QRenderStateSet( sortPolicy );
  {
    Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
    depthTest->setDepthFunction( Qt3DRender::QDepthTest::Less );
    transparentObjectsRenderStateSetColor->addRenderState( depthTest );

    Qt3DRender::QNoDepthMask *noDepthMask = new Qt3DRender::QNoDepthMask;
    transparentObjectsRenderStateSetColor->addRenderState( noDepthMask );

    Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
    cullFace->setMode( Qt3DRender::QCullFace::CullingMode::NoCulling );
    transparentObjectsRenderStateSetColor->addRenderState( cullFace );

    Qt3DRender::QBlendEquation *blendEquation = new Qt3DRender::QBlendEquation;
    blendEquation->setBlendFunction( Qt3DRender::QBlendEquation::Add );
    transparentObjectsRenderStateSetColor->addRenderState( blendEquation );

    Qt3DRender::QBlendEquationArguments *blendEquationArgs = new Qt3DRender::QBlendEquationArguments;
    blendEquationArgs->setSourceRgb( Qt3DRender::QBlendEquationArguments::Blending::SourceAlpha );
    blendEquationArgs->setDestinationRgb( Qt3DRender::QBlendEquationArguments::Blending::OneMinusSourceAlpha );
    transparentObjectsRenderStateSetColor->addRenderState( blendEquationArgs );
  }

  // third branch: transparent layer filter - depth
  Qt3DRender::QRenderStateSet *transparentObjectsRenderStateSetDepth = new Qt3DRender::QRenderStateSet( sortPolicy );
  {
    Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
    depthTest->setDepthFunction( Qt3DRender::QDepthTest::Less );
    transparentObjectsRenderStateSetDepth->addRenderState( depthTest );

    Qt3DRender::QColorMask *noColorMask = new Qt3DRender::QColorMask;
    noColorMask->setAlphaMasked( false );
    noColorMask->setRedMasked( false );
    noColorMask->setGreenMasked( false );
    noColorMask->setBlueMasked( false );
    transparentObjectsRenderStateSetDepth->addRenderState( noColorMask );

    Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
    cullFace->setMode( Qt3DRender::QCullFace::CullingMode::NoCulling );
    transparentObjectsRenderStateSetDepth->addRenderState( cullFace );
  }

  mDebugOverlay = new Qt3DRender::QDebugOverlay( mClearBuffers );
  mDebugOverlay->setEnabled( false );
}

void QgsForwardRenderView::updateWindowResize( int width, int height )
{
  mColorTexture->setSize( width, height );
  mDepthTexture->setSize( width, height );
}


void QgsForwardRenderView::setClearColor( const QColor &clearColor )
{
  mClearBuffers->setClearColor( clearColor );
}


void QgsForwardRenderView::setFrustumCullingEnabled( bool enabled )
{
  if ( enabled == mFrustumCullingEnabled )
    return;
  mFrustumCullingEnabled = enabled;
  mFrustumCulling->setEnabled( enabled );
}


void QgsForwardRenderView::setDebugOverlayEnabled( bool enabled )
{
  mDebugOverlay->setEnabled( enabled );
}

Qt3DRender::QTexture2D *QgsForwardRenderView::depthTexture() const
{
  return mDepthTexture;
}

Qt3DRender::QTexture2D *QgsForwardRenderView::colorTexture() const
{
  return mColorTexture;
}

void QgsForwardRenderView::removeClipPlanes()
{
  for ( Qt3DRender::QRenderState *state : mClipRenderStateSet->renderStates() )
  {
    if ( qobject_cast<Qt3DRender::QClipPlane *>( state ) )
    {
      mClipRenderStateSet->removeRenderState( state );
    }
  }
}

void QgsForwardRenderView::addClipPlanes( int nrClipPlanes )
{
  // remove existing QClipPlane
  removeClipPlanes();

  // create new QClipPlane
  for ( int i = 0; i < nrClipPlanes; ++i )
  {
    Qt3DRender::QClipPlane *clipPlane = new Qt3DRender::QClipPlane;
    clipPlane->setPlaneIndex( i );
    mClipRenderStateSet->addRenderState( clipPlane );
  }
}
