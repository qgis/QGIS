/***************************************************************************
  qgsframegraph.cpp
  --------------------------------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb underscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsframegraph.h"
#include "qgsdirectionallightsettings.h"
#include "qgspostprocessingentity.h"
#include "qgspreviewquad.h"
#include "qgs3dutils.h"
#include "qgsambientocclusionrenderentity.h"
#include "qgsambientocclusionblurentity.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometry>

typedef Qt3DRender::QAttribute Qt3DQAttribute;
typedef Qt3DRender::QBuffer Qt3DQBuffer;
typedef Qt3DRender::QGeometry Qt3DQGeometry;
#else
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>

typedef Qt3DCore::QAttribute Qt3DQAttribute;
typedef Qt3DCore::QBuffer Qt3DQBuffer;
typedef Qt3DCore::QGeometry Qt3DQGeometry;
#endif

#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QBlendEquation>
#include <Qt3DRender/QColorMask>
#include <Qt3DRender/QSortPolicy>
#include <Qt3DRender/QNoDepthMask>
#include <Qt3DRender/QBlendEquationArguments>

Qt3DRender::QFrameGraphNode *QgsFrameGraph::constructTexturesPreviewPass()
{
  mPreviewLayerFilter = new Qt3DRender::QLayerFilter;
  mPreviewLayerFilter->addLayer( mPreviewLayer );

  mPreviewRenderStateSet = new Qt3DRender::QRenderStateSet( mPreviewLayerFilter );
  mPreviewDepthTest = new Qt3DRender::QDepthTest;
  mPreviewDepthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
  mPreviewRenderStateSet->addRenderState( mPreviewDepthTest );
  mPreviewCullFace = new Qt3DRender::QCullFace;
  mPreviewCullFace->setMode( Qt3DRender::QCullFace::NoCulling );
  mPreviewRenderStateSet->addRenderState( mPreviewCullFace );

  return mPreviewLayerFilter;
}

Qt3DRender::QFrameGraphNode *QgsFrameGraph::constructForwardRenderPass()
{
  // This is where rendering of the 3D scene actually happens.
  // We define two forward passes: one for solid objects, followed by one for transparent objects.
  //
  //                                  |
  //                         +-----------------+
  //                         | QCameraSelector |  (using the main camera)
  //                         +-----------------+
  //                                  |
  //                         +-----------------+
  //                         |  QLayerFilter   |  (using mForwardRenderLayer)
  //                         +-----------------+
  //                                  |
  //                      +-----------------------+
  //                      | QRenderTargetSelector | (write mForwardColorTexture + mForwardDepthTexture)
  //                      +-----------------------+
  //                                  |
  //         +------------------------+---------------------+
  //         |                                              |
  //  +-----------------+    discard               +-----------------+    accept
  //  |  QLayerFilter   |  transparent             |  QLayerFilter   |  transparent
  //  +-----------------+    objects               +-----------------+    objects
  //         |                                              |
  //  +-----------------+  use depth test          +-----------------+   sort entities
  //  | QRenderStateSet |  cull back faces         |  QSortPolicy    |  back to front
  //  +-----------------+                          +-----------------+
  //         |                                              |
  //  +-----------------+              +--------------------+--------------------+
  //  | QFrustumCulling |              |                                         |
  //  +-----------------+     +-----------------+  use depth tests      +-----------------+  use depth tests
  //         |                | QRenderStateSet |  don't write depths   | QRenderStateSet |  write depths
  //         |                +-----------------+  write colors         +-----------------+  don't write colors
  //  +-----------------+                          use alpha blending                        don't use alpha blending
  //  |  QClearBuffers  |  color and depth         no culling                                no culling
  //  +-----------------+

  mMainCameraSelector = new Qt3DRender::QCameraSelector;
  mMainCameraSelector->setCamera( mMainCamera );

  mForwardRenderLayerFilter = new Qt3DRender::QLayerFilter( mMainCameraSelector );
  mForwardRenderLayerFilter->addLayer( mForwardRenderLayer );

  mForwardColorTexture = new Qt3DRender::QTexture2D;
  mForwardColorTexture->setWidth( mSize.width() );
  mForwardColorTexture->setHeight( mSize.height() );
  mForwardColorTexture->setFormat( Qt3DRender::QAbstractTexture::RGB8_UNorm );
  mForwardColorTexture->setGenerateMipMaps( false );
  mForwardColorTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  mForwardColorTexture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  mForwardColorTexture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::ClampToEdge );
  mForwardColorTexture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::ClampToEdge );

  mForwardDepthTexture = new Qt3DRender::QTexture2D;
  mForwardDepthTexture->setWidth( mSize.width() );
  mForwardDepthTexture->setHeight( mSize.height() );
  mForwardDepthTexture->setFormat( Qt3DRender::QTexture2D::TextureFormat::DepthFormat );
  mForwardDepthTexture->setGenerateMipMaps( false );
  mForwardDepthTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  mForwardDepthTexture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  mForwardDepthTexture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::ClampToEdge );
  mForwardDepthTexture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::ClampToEdge );

  Qt3DRender::QRenderTarget *forwardRenderTarget = new Qt3DRender::QRenderTarget;
  Qt3DRender::QRenderTargetOutput *forwardRenderTargetDepthOutput = new Qt3DRender::QRenderTargetOutput;
  forwardRenderTargetDepthOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
  forwardRenderTargetDepthOutput->setTexture( mForwardDepthTexture );
  forwardRenderTarget->addOutput( forwardRenderTargetDepthOutput );
  Qt3DRender::QRenderTargetOutput *forwardRenderTargetColorOutput = new Qt3DRender::QRenderTargetOutput;
  forwardRenderTargetColorOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );
  forwardRenderTargetColorOutput->setTexture( mForwardColorTexture );
  forwardRenderTarget->addOutput( forwardRenderTargetColorOutput );

  mForwardRenderTargetSelector = new Qt3DRender::QRenderTargetSelector( mForwardRenderLayerFilter );
  mForwardRenderTargetSelector->setTarget( forwardRenderTarget );

  // first branch: opaque layer filter
  Qt3DRender::QLayerFilter *opaqueObjectsFilter = new Qt3DRender::QLayerFilter( mForwardRenderTargetSelector );
  opaqueObjectsFilter->addLayer( mTransparentObjectsPassLayer );
  opaqueObjectsFilter->setFilterMode( Qt3DRender::QLayerFilter::DiscardAnyMatchingLayers );

  Qt3DRender::QRenderStateSet *forwardedRenderStateSet = new Qt3DRender::QRenderStateSet( opaqueObjectsFilter );

  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::Less );
  forwardedRenderStateSet->addRenderState( depthTest );

  Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
  cullFace->setMode( Qt3DRender::QCullFace::CullingMode::Back );
  forwardedRenderStateSet->addRenderState( cullFace );

  mFrustumCulling = new Qt3DRender::QFrustumCulling( forwardedRenderStateSet );

  mForwardClearBuffers = new Qt3DRender::QClearBuffers( mFrustumCulling );
  mForwardClearBuffers->setClearColor( QColor::fromRgbF( 0.0, 0.0, 1.0, 1.0 ) );
  mForwardClearBuffers->setBuffers( Qt3DRender::QClearBuffers::ColorDepthBuffer );
  mForwardClearBuffers->setClearDepthValue( 1.0f );

  // second branch: transparent layer filter - color
  Qt3DRender::QLayerFilter *transparentObjectsLayerFilter = new Qt3DRender::QLayerFilter( mForwardRenderTargetSelector );
  transparentObjectsLayerFilter->addLayer( mTransparentObjectsPassLayer );
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

  mDebugOverlay = new Qt3DRender::QDebugOverlay( mForwardClearBuffers );
  mDebugOverlay->setEnabled( false );

  // cppcheck wrongly believes transparentObjectsRenderStateSetColor and transparentObjectsRenderStateSetDepth will leak
  // cppcheck-suppress memleak
  return mMainCameraSelector;
}

Qt3DRender::QFrameGraphNode *QgsFrameGraph::constructShadowRenderPass()
{
  mLightCameraSelectorShadowPass = new Qt3DRender::QCameraSelector;
  mLightCameraSelectorShadowPass->setCamera( mLightCamera );

  mShadowSceneEntitiesFilter = new Qt3DRender::QLayerFilter( mLightCameraSelectorShadowPass );
  mShadowSceneEntitiesFilter->addLayer( mCastShadowsLayer );

  mShadowMapTexture = new Qt3DRender::QTexture2D;
  mShadowMapTexture->setWidth( mShadowMapResolution );
  mShadowMapTexture->setHeight( mShadowMapResolution );
  mShadowMapTexture->setFormat( Qt3DRender::QTexture2D::TextureFormat::DepthFormat );
  mShadowMapTexture->setGenerateMipMaps( false );
  mShadowMapTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  mShadowMapTexture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  mShadowMapTexture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::ClampToEdge );
  mShadowMapTexture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::ClampToEdge );

  Qt3DRender::QRenderTarget *shadowRenderTarget = new Qt3DRender::QRenderTarget;
  Qt3DRender::QRenderTargetOutput *shadowRenderTargetOutput = new Qt3DRender::QRenderTargetOutput;
  shadowRenderTargetOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
  shadowRenderTargetOutput->setTexture( mShadowMapTexture );
  shadowRenderTarget->addOutput( shadowRenderTargetOutput );

  mShadowRenderTargetSelector = new Qt3DRender::QRenderTargetSelector( mShadowSceneEntitiesFilter );
  mShadowRenderTargetSelector->setTarget( shadowRenderTarget );

  mShadowClearBuffers = new Qt3DRender::QClearBuffers( mShadowRenderTargetSelector );
  mShadowClearBuffers->setBuffers( Qt3DRender::QClearBuffers::BufferType::ColorDepthBuffer );
  mShadowClearBuffers->setClearColor( QColor::fromRgbF( 0.0f, 1.0f, 0.0f ) );

  mShadowRenderStateSet = new Qt3DRender::QRenderStateSet( mShadowClearBuffers );

  Qt3DRender::QDepthTest *shadowDepthTest = new Qt3DRender::QDepthTest;
  shadowDepthTest->setDepthFunction( Qt3DRender::QDepthTest::Less );
  mShadowRenderStateSet->addRenderState( shadowDepthTest );

  Qt3DRender::QCullFace *shadowCullFace = new Qt3DRender::QCullFace;
  shadowCullFace->setMode( Qt3DRender::QCullFace::CullingMode::Front );
  mShadowRenderStateSet->addRenderState( shadowCullFace );

  Qt3DRender::QPolygonOffset *polygonOffset = new Qt3DRender::QPolygonOffset;
  polygonOffset->setDepthSteps( 4.0 );
  polygonOffset->setScaleFactor( 1.1 );
  mShadowRenderStateSet->addRenderState( polygonOffset );

  return mLightCameraSelectorShadowPass;
}

Qt3DRender::QFrameGraphNode *QgsFrameGraph::constructPostprocessingPass()
{
  mPostProcessingCameraSelector = new Qt3DRender::QCameraSelector;
  mPostProcessingCameraSelector->setCamera( mLightCamera );

  mPostprocessPassLayerFilter = new Qt3DRender::QLayerFilter( mPostProcessingCameraSelector );

  mPostprocessClearBuffers = new Qt3DRender::QClearBuffers( mPostprocessPassLayerFilter );

  mRenderCaptureTargetSelector = new Qt3DRender::QRenderTargetSelector( mPostprocessClearBuffers );

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

  depthOutput->setTexture( mRenderCaptureDepthTexture );
  renderTarget->addOutput( depthOutput );

  mRenderCaptureTargetSelector->setTarget( renderTarget );

  mRenderCapture = new Qt3DRender::QRenderCapture( mRenderCaptureTargetSelector );

  mPostprocessingEntity = new QgsPostprocessingEntity( this, mRootEntity );
  mPostprocessPassLayerFilter->addLayer( mPostprocessingEntity->layer() );

  return mPostProcessingCameraSelector;
}

Qt3DRender::QFrameGraphNode *QgsFrameGraph::constructAmbientOcclusionRenderPass()
{
  mAmbientOcclusionRenderCameraSelector = new Qt3DRender::QCameraSelector;
  mAmbientOcclusionRenderCameraSelector->setCamera( mMainCamera );

  mAmbientOcclusionRenderStateSet = new Qt3DRender::QRenderStateSet( mAmbientOcclusionRenderCameraSelector );

  Qt3DRender::QDepthTest *depthRenderDepthTest = new Qt3DRender::QDepthTest;
  depthRenderDepthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );;
  Qt3DRender::QCullFace *depthRenderCullFace = new Qt3DRender::QCullFace;
  depthRenderCullFace->setMode( Qt3DRender::QCullFace::NoCulling );

  mAmbientOcclusionRenderStateSet->addRenderState( depthRenderDepthTest );
  mAmbientOcclusionRenderStateSet->addRenderState( depthRenderCullFace );

  mAmbientOcclusionRenderLayerFilter = new Qt3DRender::QLayerFilter( mAmbientOcclusionRenderStateSet );

  mAmbientOcclusionRenderCaptureTargetSelector = new Qt3DRender::QRenderTargetSelector( mAmbientOcclusionRenderLayerFilter );
  Qt3DRender::QRenderTarget *depthRenderTarget = new Qt3DRender::QRenderTarget( mAmbientOcclusionRenderCaptureTargetSelector );

  // The lifetime of the objects created here is managed
  // automatically, as they become children of this object.

  // Create a render target output for rendering color.
  Qt3DRender::QRenderTargetOutput *colorOutput = new Qt3DRender::QRenderTargetOutput( depthRenderTarget );
  colorOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );

  // Create a texture to render into.
  mAmbientOcclusionRenderTexture = new Qt3DRender::QTexture2D( colorOutput );
  mAmbientOcclusionRenderTexture->setSize( mSize.width(), mSize.height() );
  mAmbientOcclusionRenderTexture->setFormat( Qt3DRender::QAbstractTexture::R32F );
  mAmbientOcclusionRenderTexture->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mAmbientOcclusionRenderTexture->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );

  // Hook the texture up to our output, and the output up to this object.
  colorOutput->setTexture( mAmbientOcclusionRenderTexture );
  depthRenderTarget->addOutput( colorOutput );

  mAmbientOcclusionRenderCaptureTargetSelector->setTarget( depthRenderTarget );

  mAmbientOcclusionRenderEntity = new QgsAmbientOcclusionRenderEntity( mForwardDepthTexture, mMainCamera, mRootEntity );
  mAmbientOcclusionRenderLayerFilter->addLayer( mAmbientOcclusionRenderEntity->layer() );

  return mAmbientOcclusionRenderCameraSelector;
}

Qt3DRender::QFrameGraphNode *QgsFrameGraph::constructAmbientOcclusionBlurPass()
{
  mAmbientOcclusionBlurCameraSelector = new Qt3DRender::QCameraSelector;
  mAmbientOcclusionBlurCameraSelector->setCamera( mMainCamera );

  mAmbientOcclusionBlurStateSet = new Qt3DRender::QRenderStateSet( mAmbientOcclusionBlurCameraSelector );

  Qt3DRender::QDepthTest *depthRenderDepthTest = new Qt3DRender::QDepthTest;
  depthRenderDepthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );;
  Qt3DRender::QCullFace *depthRenderCullFace = new Qt3DRender::QCullFace;
  depthRenderCullFace->setMode( Qt3DRender::QCullFace::NoCulling );

  mAmbientOcclusionBlurStateSet->addRenderState( depthRenderDepthTest );
  mAmbientOcclusionBlurStateSet->addRenderState( depthRenderCullFace );

  mAmbientOcclusionBlurLayerFilter = new Qt3DRender::QLayerFilter( mAmbientOcclusionBlurStateSet );

  mAmbientOcclusionBlurRenderCaptureTargetSelector = new Qt3DRender::QRenderTargetSelector( mAmbientOcclusionBlurLayerFilter );
  Qt3DRender::QRenderTarget *depthRenderTarget = new Qt3DRender::QRenderTarget( mAmbientOcclusionBlurRenderCaptureTargetSelector );

  // The lifetime of the objects created here is managed
  // automatically, as they become children of this object.

  // Create a render target output for rendering color.
  Qt3DRender::QRenderTargetOutput *colorOutput = new Qt3DRender::QRenderTargetOutput( depthRenderTarget );
  colorOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );

  // Create a texture to render into.
  mAmbientOcclusionBlurTexture = new Qt3DRender::QTexture2D( colorOutput );
  mAmbientOcclusionBlurTexture->setSize( mSize.width(), mSize.height() );
  mAmbientOcclusionBlurTexture->setFormat( Qt3DRender::QAbstractTexture::R32F );
  mAmbientOcclusionBlurTexture->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mAmbientOcclusionBlurTexture->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );

  // Hook the texture up to our output, and the output up to this object.
  colorOutput->setTexture( mAmbientOcclusionBlurTexture );
  depthRenderTarget->addOutput( colorOutput );

  mAmbientOcclusionBlurRenderCaptureTargetSelector->setTarget( depthRenderTarget );

  mAmbientOcclusionBlurEntity = new QgsAmbientOcclusionBlurEntity( mAmbientOcclusionRenderTexture, mRootEntity );
  mAmbientOcclusionBlurLayerFilter->addLayer( mAmbientOcclusionBlurEntity->layer() );

  return mAmbientOcclusionBlurCameraSelector;
}


Qt3DRender::QFrameGraphNode *QgsFrameGraph::constructRubberBandsPass()
{
  mRubberBandsCameraSelector = new Qt3DRender::QCameraSelector;
  mRubberBandsCameraSelector->setCamera( mMainCamera );

  mRubberBandsLayerFilter = new Qt3DRender::QLayerFilter( mRubberBandsCameraSelector );
  mRubberBandsLayerFilter->addLayer( mRubberBandsLayer );

  mRubberBandsStateSet = new Qt3DRender::QRenderStateSet( mRubberBandsLayerFilter );
  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
  mRubberBandsStateSet->addRenderState( depthTest );

  // Here we attach our drawings to the render target also used by forward pass.
  // This is kind of okay, but as a result, post-processing effects get applied
  // to rubber bands too. Ideally we would want them on top of everything.
  mRubberBandsRenderTargetSelector = new Qt3DRender::QRenderTargetSelector( mRubberBandsStateSet );
  mRubberBandsRenderTargetSelector->setTarget( mForwardRenderTargetSelector->target() );

  return mRubberBandsCameraSelector;
}



Qt3DRender::QFrameGraphNode *QgsFrameGraph::constructDepthRenderPass()
{
  // depth buffer render to copy pass

  mDepthRenderCameraSelector = new Qt3DRender::QCameraSelector;
  mDepthRenderCameraSelector->setCamera( mMainCamera );

  mDepthRenderStateSet = new Qt3DRender::QRenderStateSet( mDepthRenderCameraSelector );

  Qt3DRender::QDepthTest *depthRenderDepthTest = new Qt3DRender::QDepthTest;
  depthRenderDepthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );;
  Qt3DRender::QCullFace *depthRenderCullFace = new Qt3DRender::QCullFace;
  depthRenderCullFace->setMode( Qt3DRender::QCullFace::NoCulling );

  mDepthRenderStateSet->addRenderState( depthRenderDepthTest );
  mDepthRenderStateSet->addRenderState( depthRenderCullFace );

  mDepthRenderLayerFilter = new Qt3DRender::QLayerFilter( mDepthRenderStateSet );
  mDepthRenderLayerFilter->addLayer( mDepthRenderPassLayer );

  mDepthRenderCaptureTargetSelector = new Qt3DRender::QRenderTargetSelector( mDepthRenderLayerFilter );
  Qt3DRender::QRenderTarget *depthRenderTarget = new Qt3DRender::QRenderTarget( mDepthRenderCaptureTargetSelector );

  // The lifetime of the objects created here is managed
  // automatically, as they become children of this object.

  // Create a render target output for rendering color.
  Qt3DRender::QRenderTargetOutput *colorOutput = new Qt3DRender::QRenderTargetOutput( depthRenderTarget );
  colorOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );

  // Create a texture to render into.
  mDepthRenderCaptureColorTexture = new Qt3DRender::QTexture2D( colorOutput );
  mDepthRenderCaptureColorTexture->setSize( mSize.width(), mSize.height() );
  mDepthRenderCaptureColorTexture->setFormat( Qt3DRender::QAbstractTexture::RGB8_UNorm );
  mDepthRenderCaptureColorTexture->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mDepthRenderCaptureColorTexture->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );

  // Hook the texture up to our output, and the output up to this object.
  colorOutput->setTexture( mDepthRenderCaptureColorTexture );
  depthRenderTarget->addOutput( colorOutput );

  Qt3DRender::QRenderTargetOutput *depthOutput = new Qt3DRender::QRenderTargetOutput( depthRenderTarget );

  depthOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
  mDepthRenderCaptureDepthTexture = new Qt3DRender::QTexture2D( depthOutput );
  mDepthRenderCaptureDepthTexture->setSize( mSize.width(), mSize.height() );
  mDepthRenderCaptureDepthTexture->setFormat( Qt3DRender::QAbstractTexture::DepthFormat );
  mDepthRenderCaptureDepthTexture->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mDepthRenderCaptureDepthTexture->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mDepthRenderCaptureDepthTexture->setComparisonFunction( Qt3DRender::QAbstractTexture::CompareLessEqual );
  mDepthRenderCaptureDepthTexture->setComparisonMode( Qt3DRender::QAbstractTexture::CompareRefToTexture );

  depthOutput->setTexture( mDepthRenderCaptureDepthTexture );
  depthRenderTarget->addOutput( depthOutput );

  mDepthRenderCaptureTargetSelector->setTarget( depthRenderTarget );

  // Note: We do not a clear buffers node since we are drawing a quad that will override the buffer's content anyway
  mDepthRenderCapture = new Qt3DRender::QRenderCapture( mDepthRenderCaptureTargetSelector );

  return mDepthRenderCameraSelector;
}

Qt3DCore::QEntity *QgsFrameGraph::constructDepthRenderQuad()
{
  Qt3DCore::QEntity *quad = new Qt3DCore::QEntity;
  quad->setObjectName( "depthRenderQuad" );

  Qt3DQGeometry *geom = new Qt3DQGeometry;
  Qt3DQAttribute *positionAttribute = new Qt3DQAttribute;
  const QVector<float> vert = { -1.0f, -1.0f, 1.0f, /**/ 1.0f, -1.0f, 1.0f, /**/ -1.0f,  1.0f, 1.0f, /**/ -1.0f,  1.0f, 1.0f, /**/ 1.0f, -1.0f, 1.0f, /**/ 1.0f,  1.0f, 1.0f };

  const QByteArray vertexArr( ( const char * ) vert.constData(), vert.size() * sizeof( float ) );
  Qt3DQBuffer *vertexBuffer = nullptr;
  vertexBuffer = new Qt3DQBuffer( this );
  vertexBuffer->setData( vertexArr );

  positionAttribute->setName( Qt3DQAttribute::defaultPositionAttributeName() );
  positionAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  positionAttribute->setVertexSize( 3 );
  positionAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  positionAttribute->setBuffer( vertexBuffer );
  positionAttribute->setByteOffset( 0 );
  positionAttribute->setByteStride( 3 * sizeof( float ) );
  positionAttribute->setCount( 6 );

  geom->addAttribute( positionAttribute );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::PrimitiveType::Triangles );
  renderer->setGeometry( geom );

  quad->addComponent( renderer );

  QMatrix4x4 modelMatrix;
  modelMatrix.setToIdentity();

  // construct material

  Qt3DRender::QMaterial *material = new Qt3DRender::QMaterial;
  Qt3DRender::QParameter *textureParameter = new Qt3DRender::QParameter( "depthTexture", mForwardDepthTexture );
  Qt3DRender::QParameter *textureTransformParameter = new Qt3DRender::QParameter( "modelMatrix", QVariant::fromValue( modelMatrix ) );
  material->addParameter( textureParameter );
  material->addParameter( textureTransformParameter );

  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect;

  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique;

  Qt3DRender::QGraphicsApiFilter *graphicsApiFilter = technique->graphicsApiFilter();
  graphicsApiFilter->setApi( Qt3DRender::QGraphicsApiFilter::Api::OpenGL );
  graphicsApiFilter->setProfile( Qt3DRender::QGraphicsApiFilter::OpenGLProfile::CoreProfile );
  graphicsApiFilter->setMajorVersion( 1 );
  graphicsApiFilter->setMinorVersion( 5 );

  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass;

  Qt3DRender::QShaderProgram *shader = new Qt3DRender::QShaderProgram;
  shader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( "qrc:/shaders/depth_render.vert" ) ) );
  shader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( "qrc:/shaders/depth_render.frag" ) ) );
  renderPass->setShaderProgram( shader );

  technique->addRenderPass( renderPass );

  effect->addTechnique( technique );
  material->setEffect( effect );

  quad->addComponent( material );

  return quad;
}

QgsFrameGraph::QgsFrameGraph( QSurface *surface, QSize s, Qt3DRender::QCamera *mainCamera, Qt3DCore::QEntity *root )
  : Qt3DCore::QEntity( root )
  , mSize( s )
{

  // general overview of how the frame graph looks:
  //
  //  +------------------------+    using window or
  //  | QRenderSurfaceSelector |   offscreen surface
  //  +------------------------+
  //             |
  //  +-----------+
  //  | QViewport | (0,0,1,1)
  //  +-----------+
  //             |
  //     +--------------------------+-------------------+-----------------+
  //     |                          |                   |                 |
  // +--------------------+ +--------------+ +-----------------+ +-----------------+
  // | two forward passes | | shadows pass | |  depth buffer   | | post-processing |
  // |  (solid objects    | |              | | processing pass | |    passes       |
  // |  and transparent)  | +--------------+ +-----------------+ +-----------------+
  // +--------------------+
  //
  // Notes:
  // - depth buffer processing pass is used whenever we need depth map information
  //   (for camera navigation) and it converts depth texture to a color texture
  //   so that we can capture it with QRenderCapture - currently it is unable
  //   to capture depth buffer, only colors (see QTBUG-65155)
  // - there are multiple post-processing passes that take rendered output
  //   of the scene, optionally apply effects (add shadows, ambient occlusion,
  //   eye dome lighting) and finally output to the given surface
  // - there may be also two more passes when 3D axis is shown - see Qgs3DAxis

  mRootEntity = root;
  mMainCamera = mainCamera;
  mLightCamera = new Qt3DRender::QCamera;

  mPreviewLayer = new Qt3DRender::QLayer;
  mCastShadowsLayer = new Qt3DRender::QLayer;
  mForwardRenderLayer = new Qt3DRender::QLayer;
  mDepthRenderPassLayer = new Qt3DRender::QLayer;
  mTransparentObjectsPassLayer = new Qt3DRender::QLayer;
  mRubberBandsLayer = new Qt3DRender::QLayer;

  mPreviewLayer->setRecursive( true );
  mCastShadowsLayer->setRecursive( true );
  mForwardRenderLayer->setRecursive( true );
  mDepthRenderPassLayer->setRecursive( true );
  mTransparentObjectsPassLayer->setRecursive( true );
  mRubberBandsLayer->setRecursive( true );

  mRenderSurfaceSelector = new Qt3DRender::QRenderSurfaceSelector;

  QObject *surfaceObj = dynamic_cast< QObject *  >( surface );
  Q_ASSERT( surfaceObj );

  mRenderSurfaceSelector->setSurface( surfaceObj );
  mRenderSurfaceSelector->setExternalRenderTargetSize( mSize );

  mMainViewPort = new Qt3DRender::QViewport( mRenderSurfaceSelector );
  mMainViewPort->setNormalizedRect( QRectF( 0.0f, 0.0f, 1.0f, 1.0f ) );

  // Forward render
  Qt3DRender::QFrameGraphNode *forwardRenderPass = constructForwardRenderPass();
  forwardRenderPass->setParent( mMainViewPort );

  // rubber bands (they should be always on top)
  Qt3DRender::QFrameGraphNode *rubberBandsPass = constructRubberBandsPass();
  rubberBandsPass->setParent( mMainViewPort );

  // shadow rendering pass
  Qt3DRender::QFrameGraphNode *shadowRenderPass = constructShadowRenderPass();
  shadowRenderPass->setParent( mMainViewPort );

  // depth buffer processing
  Qt3DRender::QFrameGraphNode *depthBufferProcessingPass = constructDepthRenderPass();
  depthBufferProcessingPass->setParent( mMainViewPort );

  // Ambient occlusion factor render pass
  Qt3DRender::QFrameGraphNode *ambientOcclusionFactorRender = constructAmbientOcclusionRenderPass();
  ambientOcclusionFactorRender->setParent( mMainViewPort );

  Qt3DRender::QFrameGraphNode *ambientOcclusionBlurPass = constructAmbientOcclusionBlurPass();
  ambientOcclusionBlurPass->setParent( mMainViewPort );

  // post process
  Qt3DRender::QFrameGraphNode *postprocessingPass = constructPostprocessingPass();
  postprocessingPass->setParent( mMainViewPort );

  mRubberBandsRootEntity = new Qt3DCore::QEntity( mRootEntity );
  mRubberBandsRootEntity->addComponent( mRubberBandsLayer );

  // textures preview pass
  Qt3DRender::QFrameGraphNode *previewPass = constructTexturesPreviewPass();
  previewPass->setParent( mMainViewPort );

  Qt3DRender::QParameter *depthMapIsDepthParam = new Qt3DRender::QParameter( "isDepth", true );
  Qt3DRender::QParameter *shadowMapIsDepthParam = new Qt3DRender::QParameter( "isDepth", true );

  mDebugDepthMapPreviewQuad = this->addTexturePreviewOverlay( mForwardDepthTexture, QPointF( 0.9f, 0.9f ), QSizeF( 0.1, 0.1 ), QVector<Qt3DRender::QParameter *> { depthMapIsDepthParam } );
  mDebugShadowMapPreviewQuad = this->addTexturePreviewOverlay( mShadowMapTexture, QPointF( 0.9f, 0.9f ), QSizeF( 0.1, 0.1 ), QVector<Qt3DRender::QParameter *> { shadowMapIsDepthParam } );
  mDebugDepthMapPreviewQuad->setEnabled( false );
  mDebugShadowMapPreviewQuad->setEnabled( false );

  mDepthRenderQuad = constructDepthRenderQuad();
  mDepthRenderQuad->addComponent( mDepthRenderPassLayer );
  mDepthRenderQuad->setParent( mRootEntity );
}

QgsPreviewQuad *QgsFrameGraph::addTexturePreviewOverlay( Qt3DRender::QTexture2D *texture, const QPointF &centerTexCoords, const QSizeF &sizeTexCoords, QVector<Qt3DRender::QParameter *> additionalShaderParameters )
{
  QgsPreviewQuad *previewQuad = new QgsPreviewQuad( texture, centerTexCoords, sizeTexCoords, additionalShaderParameters );
  previewQuad->addComponent( mPreviewLayer );
  previewQuad->setParent( mRootEntity );
  mPreviewQuads.push_back( previewQuad );
  return previewQuad;
}

// computes the portion of the Y=y plane the camera is looking at
void calculateViewExtent( Qt3DRender::QCamera *camera, float shadowRenderingDistance, float y, float &minX, float &maxX, float &minY, float &maxY, float &minZ, float &maxZ )
{
  const QVector3D cameraPos = camera->position();
  const QMatrix4x4 projectionMatrix = camera->projectionMatrix();
  const QMatrix4x4 viewMatrix = camera->viewMatrix();
  float depth = 1.0f;
  QVector4D viewCenter =  viewMatrix * QVector4D( camera->viewCenter(), 1.0f );
  viewCenter /= viewCenter.w();
  viewCenter = projectionMatrix * viewCenter;
  viewCenter /= viewCenter.w();
  depth = viewCenter.z();
  QVector<QVector3D> viewFrustumPoints =
  {
    QVector3D( 0.0f,  0.0f, depth ),
    QVector3D( 0.0f,  1.0f, depth ),
    QVector3D( 1.0f,  0.0f, depth ),
    QVector3D( 1.0f,  1.0f, depth ),
    QVector3D( 0.0f,  0.0f, 0 ),
    QVector3D( 0.0f,  1.0f, 0 ),
    QVector3D( 1.0f,  0.0f, 0 ),
    QVector3D( 1.0f,  1.0f, 0 )
  };
  maxX = std::numeric_limits<float>::lowest();
  maxY = std::numeric_limits<float>::lowest();
  maxZ = std::numeric_limits<float>::lowest();
  minX = std::numeric_limits<float>::max();
  minY = std::numeric_limits<float>::max();
  minZ = std::numeric_limits<float>::max();
  for ( int i = 0; i < viewFrustumPoints.size(); ++i )
  {
    // convert from view port space to world space
    viewFrustumPoints[i] = viewFrustumPoints[i].unproject( viewMatrix, projectionMatrix, QRect( 0, 0, 1, 1 ) );
    minX = std::min( minX, viewFrustumPoints[i].x() );
    maxX = std::max( maxX, viewFrustumPoints[i].x() );
    minY = std::min( minY, viewFrustumPoints[i].y() );
    maxY = std::max( maxY, viewFrustumPoints[i].y() );
    minZ = std::min( minZ, viewFrustumPoints[i].z() );
    maxZ = std::max( maxZ, viewFrustumPoints[i].z() );
    // find the intersection between the line going from cameraPos to the frustum quad point
    // and the horizontal plane Y=y
    // if the intersection is on the back side of the viewing panel we get a point that is
    // shadowRenderingDistance units in front of the camera
    const QVector3D pt = cameraPos;
    const QVector3D vect = ( viewFrustumPoints[i] - pt ).normalized();
    float t = ( y - pt.y() ) / vect.y();
    if ( t < 0 )
      t = shadowRenderingDistance;
    else
      t = std::min( t, shadowRenderingDistance );
    viewFrustumPoints[i] = pt + t * vect;
    minX = std::min( minX, viewFrustumPoints[i].x() );
    maxX = std::max( maxX, viewFrustumPoints[i].x() );
    minY = std::min( minY, viewFrustumPoints[i].y() );
    maxY = std::max( maxY, viewFrustumPoints[i].y() );
    minZ = std::min( minZ, viewFrustumPoints[i].z() );
    maxZ = std::max( maxZ, viewFrustumPoints[i].z() );
  }
}

void QgsFrameGraph::setupDirectionalLight( const QgsDirectionalLightSettings &light, float maximumShadowRenderingDistance )
{
  float minX, maxX, minY, maxY, minZ, maxZ;
  QVector3D lookingAt = mMainCamera->viewCenter();
  const float d = 2 * ( mMainCamera->position() - mMainCamera->viewCenter() ).length();

  const QVector3D vertical = QVector3D( 0.0f, d, 0.0f );
  const QVector3D lightDirection = QVector3D( light.direction().x(), light.direction().y(), light.direction().z() ).normalized();
  calculateViewExtent( mMainCamera, maximumShadowRenderingDistance, lookingAt.y(), minX, maxX, minY, maxY, minZ, maxZ );

  lookingAt = QVector3D( 0.5 * ( minX + maxX ), mMainCamera->viewCenter().y(), 0.5 * ( minZ + maxZ ) );
  const QVector3D lightPosition = lookingAt + vertical;
  mLightCamera->setPosition( lightPosition );
  mLightCamera->setViewCenter( lookingAt );
  mLightCamera->setUpVector( QVector3D( 0.0f, 1.0f, 0.0f ) );
  mLightCamera->rotateAboutViewCenter( QQuaternion::rotationTo( vertical.normalized(), -lightDirection.normalized() ) );

  mLightCamera->setProjectionType( Qt3DRender::QCameraLens::ProjectionType::OrthographicProjection );
  mLightCamera->lens()->setOrthographicProjection(
    - 0.7 * ( maxX - minX ), 0.7 * ( maxX - minX ),
    - 0.7 * ( maxZ - minZ ), 0.7 * ( maxZ - minZ ),
    1.0f, 2 * ( lookingAt - lightPosition ).length() );

  mPostprocessingEntity->setupShadowRenderingExtent( minX, maxX, minZ, maxZ );
  mPostprocessingEntity->setupDirectionalLight( lightPosition, lightDirection );
}

void QgsFrameGraph::setClearColor( const QColor &clearColor )
{
  mForwardClearBuffers->setClearColor( clearColor );
}

void QgsFrameGraph::setShadowRenderingEnabled( bool enabled )
{
  mShadowRenderingEnabled = enabled;
  mPostprocessingEntity->setShadowRenderingEnabled( mShadowRenderingEnabled );
  if ( mShadowRenderingEnabled )
    mShadowSceneEntitiesFilter->setEnabled( true );
  else
    mShadowSceneEntitiesFilter->setEnabled( false );
}

void QgsFrameGraph::setShadowBias( float shadowBias )
{
  mShadowBias = shadowBias;
  mPostprocessingEntity->setShadowBias( mShadowBias );
}

void QgsFrameGraph::setShadowMapResolution( int resolution )
{
  mShadowMapResolution = resolution;
  mShadowMapTexture->setWidth( mShadowMapResolution );
  mShadowMapTexture->setHeight( mShadowMapResolution );
}

void QgsFrameGraph::setAmbientOcclusionEnabled( bool enabled )
{
  mAmbientOcclusionEnabled = enabled;
  mAmbientOcclusionRenderEntity->setEnabled( enabled );
  mPostprocessingEntity->setAmbientOcclusionEnabled( enabled );
}

void QgsFrameGraph::setAmbientOcclusionIntensity( float intensity )
{
  mAmbientOcclusionIntensity = intensity;
  mAmbientOcclusionRenderEntity->setIntensity( intensity );
}

void QgsFrameGraph::setAmbientOcclusionRadius( float radius )
{
  mAmbientOcclusionRadius = radius;
  mAmbientOcclusionRenderEntity->setRadius( radius );
}

void QgsFrameGraph::setAmbientOcclusionThreshold( float threshold )
{
  mAmbientOcclusionThreshold = threshold;
  mAmbientOcclusionRenderEntity->setThreshold( threshold );
}

void QgsFrameGraph::setFrustumCullingEnabled( bool enabled )
{
  if ( enabled == mFrustumCullingEnabled )
    return;
  mFrustumCullingEnabled = enabled;
  if ( mFrustumCullingEnabled )
    mFrustumCulling->setParent( mForwardClearBuffers );
  else
    mFrustumCulling->setParent( ( Qt3DCore::QNode * )nullptr );
}

void QgsFrameGraph::setupEyeDomeLighting( bool enabled, double strength, int distance )
{
  mEyeDomeLightingEnabled = enabled;
  mEyeDomeLightingStrength = strength;
  mEyeDomeLightingDistance = distance;
  mPostprocessingEntity->setEyeDomeLightingEnabled( enabled );
  mPostprocessingEntity->setEyeDomeLightingStrength( strength );
  mPostprocessingEntity->setEyeDomeLightingDistance( distance );
}

void QgsFrameGraph::setupShadowMapDebugging( bool enabled, Qt::Corner corner, double size )
{
  mDebugShadowMapPreviewQuad->setEnabled( enabled );
  if ( enabled )
  {
    switch ( corner )
    {
      case Qt::Corner::TopRightCorner:
        mDebugShadowMapPreviewQuad->setViewPort( QPointF( 1.0f - size / 2, 0.0f + size / 2 ), 0.5 * QSizeF( size, size ) );
        break;
      case Qt::Corner::TopLeftCorner:
        mDebugShadowMapPreviewQuad->setViewPort( QPointF( 0.0f + size / 2, 0.0f + size / 2 ), 0.5 * QSizeF( size, size ) );
        break;
      case Qt::Corner::BottomRightCorner:
        mDebugShadowMapPreviewQuad->setViewPort( QPointF( 1.0f - size / 2, 1.0f - size / 2 ), 0.5 * QSizeF( size, size ) );
        break;
      case Qt::Corner::BottomLeftCorner:
        mDebugShadowMapPreviewQuad->setViewPort( QPointF( 0.0f + size / 2, 1.0f - size / 2 ), 0.5 * QSizeF( size, size ) );
        break;
    }
  }
}

void QgsFrameGraph::setupDepthMapDebugging( bool enabled, Qt::Corner corner, double size )
{
  mDebugDepthMapPreviewQuad->setEnabled( enabled );

  if ( enabled )
  {
    switch ( corner )
    {
      case Qt::Corner::TopRightCorner:
        mDebugDepthMapPreviewQuad->setViewPort( QPointF( 1.0f - size / 2, 0.0f + size / 2 ), 0.5 * QSizeF( size, size ) );
        break;
      case Qt::Corner::TopLeftCorner:
        mDebugDepthMapPreviewQuad->setViewPort( QPointF( 0.0f + size / 2, 0.0f + size / 2 ), 0.5 * QSizeF( size, size ) );
        break;
      case Qt::Corner::BottomRightCorner:
        mDebugDepthMapPreviewQuad->setViewPort( QPointF( 1.0f - size / 2, 1.0f - size / 2 ), 0.5 * QSizeF( size, size ) );
        break;
      case Qt::Corner::BottomLeftCorner:
        mDebugDepthMapPreviewQuad->setViewPort( QPointF( 0.0f + size / 2, 1.0f - size / 2 ), 0.5 * QSizeF( size, size ) );
        break;
    }
  }
}

void QgsFrameGraph::setSize( QSize s )
{
  mSize = s;
  mForwardColorTexture->setSize( mSize.width(), mSize.height() );
  mForwardDepthTexture->setSize( mSize.width(), mSize.height() );
  mRenderCaptureColorTexture->setSize( mSize.width(), mSize.height() );
  mRenderCaptureDepthTexture->setSize( mSize.width(), mSize.height() );
  mDepthRenderCaptureDepthTexture->setSize( mSize.width(), mSize.height() );
  mDepthRenderCaptureColorTexture->setSize( mSize.width(), mSize.height() );
  mRenderSurfaceSelector->setExternalRenderTargetSize( mSize );

  mAmbientOcclusionRenderTexture->setSize( mSize.width(), mSize.height() );
  mAmbientOcclusionBlurTexture->setSize( mSize.width(), mSize.height() );
}

void QgsFrameGraph::setRenderCaptureEnabled( bool enabled )
{
  if ( enabled == mRenderCaptureEnabled )
    return;
  mRenderCaptureEnabled = enabled;
  mRenderCaptureTargetSelector->setEnabled( mRenderCaptureEnabled );
}

void QgsFrameGraph::setDebugOverlayEnabled( bool enabled )
{
  mDebugOverlay->setEnabled( enabled );
}
