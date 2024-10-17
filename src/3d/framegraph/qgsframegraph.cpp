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
#include "qgsframegraphutils.h"
#include "qgsabstractrenderview.h"
#include "qgsshadowrenderview.h"

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
#include <Qt3DRender/QPointSize>
#include <Qt3DRender/QSeamlessCubemap>
#include <Qt3DRender/QNoDepthMask>
#include <Qt3DRender/QBlendEquationArguments>
#include <Qt3DExtras/QTextureMaterial>
#include <Qt3DRender/QAbstractTexture>
#include <Qt3DRender/QNoDraw>

const QString QgsFrameGraph::SHADOW_RENDERVIEW = "shadow";
const QString QgsFrameGraph::AXIS3D_RENDERVIEW = "3daxis";

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
  mMainCameraSelector->setObjectName( "Forward render pass CameraSelector" );
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


void QgsFrameGraph::constructShadowRenderPass()
{
  Qt3DRender::QTexture2D *shadowMapTexture = new Qt3DRender::QTexture2D;
  shadowMapTexture->setWidth( QgsFrameGraph::mDefaultShadowMapResolution );
  shadowMapTexture->setHeight( QgsFrameGraph::mDefaultShadowMapResolution );
  shadowMapTexture->setFormat( Qt3DRender::QTexture2D::TextureFormat::DepthFormat );
  shadowMapTexture->setGenerateMipMaps( false );
  shadowMapTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  shadowMapTexture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  shadowMapTexture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::ClampToEdge );
  shadowMapTexture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::ClampToEdge );
  shadowMapTexture->setObjectName( "QgsShadowRenderView::DepthTarget" );

  Qt3DRender::QRenderTargetOutput *renderTargetOutput = new Qt3DRender::QRenderTargetOutput;
  renderTargetOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
  renderTargetOutput->setTexture( shadowMapTexture );

  QgsShadowRenderView *shadowRenderView = new QgsShadowRenderView( this, SHADOW_RENDERVIEW );
  shadowRenderView->setTargetOutputs( { renderTargetOutput } );
  registerRenderView( shadowRenderView, SHADOW_RENDERVIEW );
}

Qt3DRender::QFrameGraphNode *QgsFrameGraph::constructSubPostPassForTexturesPreview()
{
  Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter;
  layerFilter->setObjectName( "Sub pass TexturesPreview" );
  layerFilter->addLayer( mPreviewLayer );

  Qt3DRender::QRenderStateSet *renderStateSet = new Qt3DRender::QRenderStateSet( layerFilter );
  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
  renderStateSet->addRenderState( depthTest );
  Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
  cullFace->setMode( Qt3DRender::QCullFace::NoCulling );
  renderStateSet->addRenderState( cullFace );

  return layerFilter;
}

Qt3DRender::QFrameGraphNode *QgsFrameGraph::constructSubPostPassForProcessing()
{
  Qt3DRender::QCameraSelector *cameraSelector = new Qt3DRender::QCameraSelector;
  cameraSelector->setObjectName( "Sub pass Postprocessing" );
  cameraSelector->setCamera( dynamic_cast<QgsShadowRenderView *>( renderView( QgsFrameGraph::SHADOW_RENDERVIEW ) )->lightCamera() );

  Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter( cameraSelector );

  // could be the first of this branch
  new Qt3DRender::QClearBuffers( layerFilter );

  Qt3DRender::QLayer *postProcessingLayer = new Qt3DRender::QLayer();
  mPostprocessingEntity = new QgsPostprocessingEntity( this, postProcessingLayer, mRootEntity );
  layerFilter->addLayer( postProcessingLayer );
  mPostprocessingEntity->setObjectName( "PostProcessingPassEntity" );

  return cameraSelector;
}

Qt3DRender::QFrameGraphNode *QgsFrameGraph::constructSubPostPassForRenderCapture()
{
  Qt3DRender::QFrameGraphNode *top = new Qt3DRender::QNoDraw;
  top->setObjectName( "Sub pass RenderCapture" );

  mRenderCapture = new Qt3DRender::QRenderCapture( top );

  return top;
}

Qt3DRender::QFrameGraphNode *QgsFrameGraph::constructPostprocessingPass()
{
  mRenderCaptureTargetSelector = new Qt3DRender::QRenderTargetSelector;
  mRenderCaptureTargetSelector->setObjectName( "Postprocessing render pass" );
  mRenderCaptureTargetSelector->setEnabled( mRenderCaptureEnabled );

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
  mRenderCaptureColorTexture->setObjectName( "PostProcessingPass::ColorTarget" );

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
  mRenderCaptureDepthTexture->setObjectName( "PostProcessingPass::DepthTarget" );

  depthOutput->setTexture( mRenderCaptureDepthTexture );
  renderTarget->addOutput( depthOutput );

  mRenderCaptureTargetSelector->setTarget( renderTarget );

  // sub passes:
  constructSubPostPassForProcessing()->setParent( mRenderCaptureTargetSelector );
  constructSubPostPassForTexturesPreview()->setParent( mRenderCaptureTargetSelector );
  constructSubPostPassForRenderCapture()->setParent( mRenderCaptureTargetSelector );

  return mRenderCaptureTargetSelector;
}

Qt3DRender::QFrameGraphNode *QgsFrameGraph::constructAmbientOcclusionRenderPass()
{
  mAmbientOcclusionRenderCameraSelector = new Qt3DRender::QCameraSelector;
  mAmbientOcclusionRenderCameraSelector->setObjectName( "AmbientOcclusion render pass CameraSelector" );
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
  Qt3DRender::QRenderTarget *colorRenderTarget = new Qt3DRender::QRenderTarget( mAmbientOcclusionRenderCaptureTargetSelector );

  // The lifetime of the objects created here is managed
  // automatically, as they become children of this object.

  // Create a render target output for rendering color.
  Qt3DRender::QRenderTargetOutput *colorOutput = new Qt3DRender::QRenderTargetOutput( colorRenderTarget );
  colorOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );

  // Create a texture to render into.
  mAmbientOcclusionRenderTexture = new Qt3DRender::QTexture2D( colorOutput );
  mAmbientOcclusionRenderTexture->setSize( mSize.width(), mSize.height() );
  mAmbientOcclusionRenderTexture->setFormat( Qt3DRender::QAbstractTexture::R32F );
  mAmbientOcclusionRenderTexture->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mAmbientOcclusionRenderTexture->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );

  // Hook the texture up to our output, and the output up to this object.
  colorOutput->setTexture( mAmbientOcclusionRenderTexture );
  colorRenderTarget->addOutput( colorOutput );

  mAmbientOcclusionRenderCaptureTargetSelector->setTarget( colorRenderTarget );

  Qt3DRender::QLayer *ambientOcclusionRenderLayer = new Qt3DRender::QLayer();
  mAmbientOcclusionRenderEntity = new QgsAmbientOcclusionRenderEntity( mForwardDepthTexture, ambientOcclusionRenderLayer, mMainCamera, mRootEntity );
  mAmbientOcclusionRenderLayerFilter->addLayer( ambientOcclusionRenderLayer );

  return mAmbientOcclusionRenderCameraSelector;
}

Qt3DRender::QFrameGraphNode *QgsFrameGraph::constructAmbientOcclusionBlurPass()
{
  mAmbientOcclusionBlurCameraSelector = new Qt3DRender::QCameraSelector;
  mAmbientOcclusionBlurCameraSelector->setObjectName( "AmbientOcclusion blur pass CameraSelector" );
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

  Qt3DRender::QLayer *ambientOcclusionBlurLayer = new Qt3DRender::QLayer();
  mAmbientOcclusionBlurEntity = new QgsAmbientOcclusionBlurEntity( mAmbientOcclusionRenderTexture, ambientOcclusionBlurLayer, mRootEntity );
  mAmbientOcclusionBlurLayerFilter->addLayer( ambientOcclusionBlurLayer );

  return mAmbientOcclusionBlurCameraSelector;
}


Qt3DRender::QFrameGraphNode *QgsFrameGraph::constructRubberBandsPass()
{
  mRubberBandsCameraSelector = new Qt3DRender::QCameraSelector;
  mRubberBandsCameraSelector->setObjectName( "RubberBands Pass CameraSelector" );
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
  mDepthRenderCameraSelector->setObjectName( "Depth render view CameraSelector" );
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

  mPreviewLayer = new Qt3DRender::QLayer;
  mForwardRenderLayer = new Qt3DRender::QLayer;
  mDepthRenderPassLayer = new Qt3DRender::QLayer;
  mTransparentObjectsPassLayer = new Qt3DRender::QLayer;
  mRubberBandsLayer = new Qt3DRender::QLayer;

  mRubberBandsLayer->setObjectName( "mRubberBandsLayer" );

  mPreviewLayer->setRecursive( true );
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
  rubberBandsPass->setObjectName( "rubberBandsPass" );
  rubberBandsPass->setParent( mMainViewPort );

  // shadow rendering pass
  constructShadowRenderPass();

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
  postprocessingPass->setObjectName( "PostProcessingPass" );

  mRubberBandsRootEntity = new Qt3DCore::QEntity( mRootEntity );
  mRubberBandsRootEntity->setObjectName( "mRubberBandsRootEntity" );
  mRubberBandsRootEntity->addComponent( mRubberBandsLayer );

  Qt3DRender::QParameter *depthMapIsDepthParam = new Qt3DRender::QParameter( "isDepth", true );
  Qt3DRender::QParameter *shadowMapIsDepthParam = new Qt3DRender::QParameter( "isDepth", true );

  mDebugDepthMapPreviewQuad = this->addTexturePreviewOverlay( mForwardDepthTexture, QPointF( 0.9f, 0.9f ), QSizeF( 0.1, 0.1 ), QVector<Qt3DRender::QParameter *> { depthMapIsDepthParam } );

  QgsAbstractRenderView *shadowRenderView = renderView( QgsFrameGraph::SHADOW_RENDERVIEW );
  Qt3DRender::QTexture2D *shadowDepthTexture = shadowRenderView->outputTexture( Qt3DRender::QRenderTargetOutput::Depth );
  mDebugShadowMapPreviewQuad = this->addTexturePreviewOverlay( shadowDepthTexture, QPointF( 0.9f, 0.9f ), QSizeF( 0.1, 0.1 ), QVector<Qt3DRender::QParameter *> { shadowMapIsDepthParam } );

  mDebugDepthMapPreviewQuad->setEnabled( false );
  mDebugShadowMapPreviewQuad->setEnabled( false );

  mDepthRenderQuad = constructDepthRenderQuad();
  mDepthRenderQuad->addComponent( mDepthRenderPassLayer );
  mDepthRenderQuad->setParent( mRootEntity );
}

void QgsFrameGraph::unregisterRenderView( const QString &name )
{
  QgsAbstractRenderView *renderView = mRenderViewMap [name];
  if ( renderView )
  {
    renderView->topGraphNode()->setParent( ( QNode * )nullptr );
    mRenderViewMap.remove( name );
  }
}

bool QgsFrameGraph::registerRenderView( QgsAbstractRenderView *renderView, const QString &name )
{
  bool out;
  if ( mRenderViewMap [name] == nullptr )
  {
    mRenderViewMap [name] = renderView;
    renderView->topGraphNode()->setParent( mMainViewPort );
    out = true;
  }
  else
    out = false;

  return out;
}

void QgsFrameGraph::setEnableRenderView( const QString &name, bool enable )
{
  if ( mRenderViewMap [name] )
  {
    mRenderViewMap [name]->setEnabled( enable );
  }
}

QgsAbstractRenderView *QgsFrameGraph::renderView( const QString &name )
{
  return mRenderViewMap [name];
}

bool QgsFrameGraph::isRenderViewEnabled( const QString &name )
{
  return mRenderViewMap [name] != nullptr && mRenderViewMap [name]->isEnabled();
}

QgsPreviewQuad *QgsFrameGraph::addTexturePreviewOverlay( Qt3DRender::QTexture2D *texture, const QPointF &centerTexCoords, const QSizeF &sizeTexCoords, QVector<Qt3DRender::QParameter *> additionalShaderParameters )
{
  QgsPreviewQuad *previewQuad = new QgsPreviewQuad( texture, centerTexCoords, sizeTexCoords, additionalShaderParameters );
  previewQuad->addComponent( mPreviewLayer );
  previewQuad->setParent( mRootEntity );
  mPreviewQuads.push_back( previewQuad );
  return previewQuad;
}


QString QgsFrameGraph::dumpFrameGraph() const
{
  QObject *top = mRenderSurfaceSelector;
  while ( top->parent() && dynamic_cast<Qt3DRender::QFrameGraphNode *>( top->parent() ) )
    top = top->parent();

  QgsFramegraphUtils::FgDumpContext context;
  context.lowestId = mMainCamera->id().id();
  QStringList strList = QgsFramegraphUtils::dumpFrameGraph( dynamic_cast<Qt3DRender::QFrameGraphNode *>( top ), context );

  return strList.join( "\n" ) + QString( "\n" );
}

QString QgsFrameGraph::dumpSceneGraph() const
{
  QStringList strList = QgsFramegraphUtils::dumpSceneGraph( mRootEntity, QgsFramegraphUtils::FgDumpContext() );
  return strList.join( "\n" ) + QString( "\n" );
}

void QgsFrameGraph::setClearColor( const QColor &clearColor )
{
  mForwardClearBuffers->setClearColor( clearColor );
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
