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
#include "moc_qgsframegraph.cpp"
#include "qgsdirectionallightsettings.h"
#include "qgspostprocessingentity.h"
#include "qgs3dutils.h"
#include "qgsframegraphutils.h"
#include "qgsabstractrenderview.h"
#include "qgsshadowrenderview.h"


#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
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
#include <Qt3DRender/QAbstractTexture>
#include <Qt3DRender/QNoDraw>
#include "qgsshadowrenderview.h"
#include "qgsforwardrenderview.h"
#include "qgsdepthrenderview.h"
#include "qgsdepthentity.h"
#include "qgsdebugtexturerenderview.h"
#include "qgsdebugtextureentity.h"
#include "qgsambientocclusionrenderview.h"

const QString QgsFrameGraph::FORWARD_RENDERVIEW = "forward";
const QString QgsFrameGraph::SHADOW_RENDERVIEW = "shadow";
const QString QgsFrameGraph::AXIS3D_RENDERVIEW = "3daxis";
const QString QgsFrameGraph::DEPTH_RENDERVIEW = "depth";
const QString QgsFrameGraph::DEBUG_RENDERVIEW = "debug_texture";
const QString QgsFrameGraph::AMBIENT_OCCLUSION_RENDERVIEW = "ambient_occlusion";

void QgsFrameGraph::constructForwardRenderPass()
{
  registerRenderView( std::make_unique<QgsForwardRenderView>( FORWARD_RENDERVIEW, mMainCamera ), FORWARD_RENDERVIEW );
}

void QgsFrameGraph::constructShadowRenderPass()
{
  registerRenderView( std::make_unique<QgsShadowRenderView>( SHADOW_RENDERVIEW ), SHADOW_RENDERVIEW );
}

void QgsFrameGraph::constructDebugTexturePass( Qt3DRender::QFrameGraphNode *topNode )
{
  registerRenderView( std::make_unique<QgsDebugTextureRenderView>( DEBUG_RENDERVIEW ), DEBUG_RENDERVIEW, topNode );
}

Qt3DRender::QFrameGraphNode *QgsFrameGraph::constructSubPostPassForProcessing()
{
  Qt3DRender::QCameraSelector *cameraSelector = new Qt3DRender::QCameraSelector;
  cameraSelector->setObjectName( "Sub pass Postprocessing" );
  cameraSelector->setCamera( shadowRenderView().lightCamera() );

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
  constructDebugTexturePass( mRenderCaptureTargetSelector );
  constructSubPostPassForRenderCapture()->setParent( mRenderCaptureTargetSelector );

  return mRenderCaptureTargetSelector;
}

void QgsFrameGraph::constructAmbientOcclusionRenderPass()
{
  Qt3DRender::QTexture2D *forwardDepthTexture = forwardRenderView().depthTexture();

  QgsAmbientOcclusionRenderView *aorv = new QgsAmbientOcclusionRenderView( AMBIENT_OCCLUSION_RENDERVIEW, mMainCamera, mSize, forwardDepthTexture, mRootEntity );
  registerRenderView( std::unique_ptr<QgsAmbientOcclusionRenderView>( aorv ), AMBIENT_OCCLUSION_RENDERVIEW );
}

Qt3DRender::QFrameGraphNode *QgsFrameGraph::constructRubberBandsPass()
{
  mRubberBandsCameraSelector = new Qt3DRender::QCameraSelector;
  mRubberBandsCameraSelector->setObjectName( "RubberBands Pass CameraSelector" );
  mRubberBandsCameraSelector->setCamera( mMainCamera );

  mRubberBandsLayerFilter = new Qt3DRender::QLayerFilter( mRubberBandsCameraSelector );
  mRubberBandsLayerFilter->addLayer( mRubberBandsLayer );

  Qt3DRender::QBlendEquationArguments *blendState = new Qt3DRender::QBlendEquationArguments;
  blendState->setSourceRgb( Qt3DRender::QBlendEquationArguments::SourceAlpha );
  blendState->setDestinationRgb( Qt3DRender::QBlendEquationArguments::OneMinusSourceAlpha );

  Qt3DRender::QBlendEquation *blendEquation = new Qt3DRender::QBlendEquation;
  blendEquation->setBlendFunction( Qt3DRender::QBlendEquation::Add );

  mRubberBandsStateSet = new Qt3DRender::QRenderStateSet( mRubberBandsLayerFilter );
  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
  mRubberBandsStateSet->addRenderState( depthTest );
  mRubberBandsStateSet->addRenderState( blendState );
  mRubberBandsStateSet->addRenderState( blendEquation );

  // Here we attach our drawings to the render target also used by forward pass.
  // This is kind of okay, but as a result, post-processing effects get applied
  // to rubber bands too. Ideally we would want them on top of everything.
  mRubberBandsRenderTargetSelector = new Qt3DRender::QRenderTargetSelector( mRubberBandsStateSet );
  mRubberBandsRenderTargetSelector->setTarget( forwardRenderView().renderTargetSelector()->target() );

  return mRubberBandsCameraSelector;
}


void QgsFrameGraph::constructDepthRenderPass()
{
  // entity used to draw the depth texture and convert it to rgb image
  Qt3DRender::QTexture2D *forwardDepthTexture = forwardRenderView().depthTexture();
  QgsDepthRenderView *rv = new QgsDepthRenderView( DEPTH_RENDERVIEW, mSize, forwardDepthTexture, mRootEntity );
  registerRenderView( std::unique_ptr<QgsDepthRenderView>( rv ), DEPTH_RENDERVIEW );
}

Qt3DRender::QRenderCapture *QgsFrameGraph::depthRenderCapture()
{
  return depthRenderView().renderCapture();
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

  mRubberBandsLayer = new Qt3DRender::QLayer;
  mRubberBandsLayer->setObjectName( "mRubberBandsLayer" );
  mRubberBandsLayer->setRecursive( true );

  mRenderSurfaceSelector = new Qt3DRender::QRenderSurfaceSelector;

  QObject *surfaceObj = dynamic_cast<QObject *>( surface );
  Q_ASSERT( surfaceObj );

  mRenderSurfaceSelector->setSurface( surfaceObj );
  mRenderSurfaceSelector->setExternalRenderTargetSize( mSize );

  mMainViewPort = new Qt3DRender::QViewport( mRenderSurfaceSelector );
  mMainViewPort->setNormalizedRect( QRectF( 0.0f, 0.0f, 1.0f, 1.0f ) );

  // Forward render
  constructForwardRenderPass();

  // rubber bands (they should be always on top)
  Qt3DRender::QFrameGraphNode *rubberBandsPass = constructRubberBandsPass();
  rubberBandsPass->setObjectName( "rubberBandsPass" );
  rubberBandsPass->setParent( mMainViewPort );

  // shadow rendering pass
  constructShadowRenderPass();

  // depth buffer processing
  constructDepthRenderPass();

  // Ambient occlusion factor render pass
  constructAmbientOcclusionRenderPass();

  // post process
  Qt3DRender::QFrameGraphNode *postprocessingPass = constructPostprocessingPass();
  postprocessingPass->setParent( mMainViewPort );
  postprocessingPass->setObjectName( "PostProcessingPass" );

  mRubberBandsRootEntity = new Qt3DCore::QEntity( mRootEntity );
  mRubberBandsRootEntity->setObjectName( "mRubberBandsRootEntity" );
  mRubberBandsRootEntity->addComponent( mRubberBandsLayer );
}

void QgsFrameGraph::unregisterRenderView( const QString &name )
{
  if ( mRenderViewMap.find( name ) != mRenderViewMap.end() )
  {
    mRenderViewMap[name]->topGraphNode()->setParent( ( QNode * ) nullptr );
    mRenderViewMap.erase( name );
  }
}

bool QgsFrameGraph::registerRenderView( std::unique_ptr<QgsAbstractRenderView> renderView, const QString &name, Qt3DRender::QFrameGraphNode *topNode )
{
  bool out;
  if ( mRenderViewMap.find( name ) == mRenderViewMap.end() )
  {
    mRenderViewMap[name] = std::move( renderView );
    mRenderViewMap[name]->topGraphNode()->setParent( topNode ? topNode : mMainViewPort );
    mRenderViewMap[name]->updateWindowResize( mSize.width(), mSize.height() );
    out = true;
  }
  else
    out = false;

  return out;
}

void QgsFrameGraph::setRenderViewEnabled( const QString &name, bool enable )
{
  if ( mRenderViewMap[name] )
  {
    mRenderViewMap[name]->setEnabled( enable );
  }
}

QgsAbstractRenderView *QgsFrameGraph::renderView( const QString &name )
{
  if ( mRenderViewMap.find( name ) != mRenderViewMap.end() )
  {
    return mRenderViewMap[name].get();
  }
  return nullptr;
}

bool QgsFrameGraph::isRenderViewEnabled( const QString &name )
{
  return mRenderViewMap[name] != nullptr && mRenderViewMap[name]->isEnabled();
}

void QgsFrameGraph::updateAmbientOcclusionSettings( const QgsAmbientOcclusionSettings &settings )
{
  QgsAmbientOcclusionRenderView &aoRenderView = ambientOcclusionRenderView();

  aoRenderView.setRadius( settings.radius() );
  aoRenderView.setIntensity( settings.intensity() );
  aoRenderView.setThreshold( settings.threshold() );
  aoRenderView.setEnabled( settings.isEnabled() );

  mPostprocessingEntity->setAmbientOcclusionEnabled( settings.isEnabled() );
}

void QgsFrameGraph::updateEyeDomeSettings( const Qgs3DMapSettings &settings )
{
  mPostprocessingEntity->setEyeDomeLightingEnabled( settings.eyeDomeLightingEnabled() );
  mPostprocessingEntity->setEyeDomeLightingStrength( settings.eyeDomeLightingStrength() );
  mPostprocessingEntity->setEyeDomeLightingDistance( settings.eyeDomeLightingDistance() );
}

void QgsFrameGraph::updateShadowSettings( const QgsShadowSettings &shadowSettings, const QList<QgsLightSource *> &lightSources )
{
  if ( shadowSettings.renderShadows() )
  {
    int selectedLight = shadowSettings.selectedDirectionalLight();
    QgsDirectionalLightSettings *light = nullptr;
    for ( int i = 0, dirLight = 0; !light && i < lightSources.size(); i++ )
    {
      if ( lightSources[i]->type() == Qgis::LightSourceType::Directional )
      {
        if ( dirLight == selectedLight )
          light = qgis::down_cast< QgsDirectionalLightSettings * >( lightSources[i] );
        dirLight++;
      }
    }

    if ( light )
    {
      shadowRenderView().setMapSize( shadowSettings.shadowMapResolution(), shadowSettings.shadowMapResolution() );
      shadowRenderView().setEnabled( true );
      mPostprocessingEntity->setShadowRenderingEnabled( true );
      mPostprocessingEntity->setShadowBias( static_cast<float>( shadowSettings.shadowBias() ) );
      mPostprocessingEntity->updateShadowSettings( *light, static_cast<float>( shadowSettings.maximumShadowRenderingDistance() ) );
    }
  }
  else
  {
    shadowRenderView().setEnabled( false );
    mPostprocessingEntity->setShadowRenderingEnabled( false );
  }
}

void QgsFrameGraph::updateDebugShadowMapSettings( const Qgs3DMapSettings &settings )
{
  QgsDebugTextureRenderView *debugRenderView = dynamic_cast<QgsDebugTextureRenderView *>( mRenderViewMap[DEBUG_RENDERVIEW].get() );
  if ( !mShadowTextureDebugging && settings.debugShadowMapEnabled() )
  {
    Qt3DRender::QTexture2D *shadowDepthTexture = shadowRenderView().mapTexture();
    mShadowTextureDebugging = new QgsDebugTextureEntity( shadowDepthTexture, debugRenderView->debugLayer(), this );
  }

  debugRenderView->setEnabled( settings.debugShadowMapEnabled() || settings.debugDepthMapEnabled() );

  if ( mShadowTextureDebugging )
  {
    mShadowTextureDebugging->setEnabled( settings.debugShadowMapEnabled() );
    if ( settings.debugShadowMapEnabled() )
      mShadowTextureDebugging->setPosition( settings.debugShadowMapCorner(), settings.debugShadowMapSize() );
    else
    {
      delete mShadowTextureDebugging;
      mShadowTextureDebugging = nullptr;
    }
  }
}

void QgsFrameGraph::updateDebugDepthMapSettings( const Qgs3DMapSettings &settings )
{
  QgsDebugTextureRenderView *debugRenderView = dynamic_cast<QgsDebugTextureRenderView *>( mRenderViewMap[DEBUG_RENDERVIEW].get() );
  if ( !mDepthTextureDebugging && settings.debugDepthMapEnabled() )
  {
    Qt3DRender::QTexture2D *forwardDepthTexture = forwardRenderView().depthTexture();
    mDepthTextureDebugging = new QgsDebugTextureEntity( forwardDepthTexture, debugRenderView->debugLayer(), this );
  }

  debugRenderView->setEnabled( settings.debugShadowMapEnabled() || settings.debugDepthMapEnabled() );

  if ( mDepthTextureDebugging )
  {
    mDepthTextureDebugging->setEnabled( settings.debugDepthMapEnabled() );
    if ( settings.debugDepthMapEnabled() )
      mDepthTextureDebugging->setPosition( settings.debugDepthMapCorner(), settings.debugDepthMapSize() );
    else
    {
      delete mDepthTextureDebugging;
      mDepthTextureDebugging = nullptr;
    }
  }
}

QString QgsFrameGraph::dumpFrameGraph() const
{
  QObject *top = mRenderSurfaceSelector;
  while ( top->parent() && dynamic_cast<Qt3DRender::QFrameGraphNode *>( top->parent() ) )
    top = top->parent();

  QgsFrameGraphUtils::FgDumpContext context;
  context.lowestId = mMainCamera->id().id();
  QStringList strList = QgsFrameGraphUtils::dumpFrameGraph( dynamic_cast<Qt3DRender::QFrameGraphNode *>( top ), context );

  return strList.join( "\n" ) + QString( "\n" );
}

QString QgsFrameGraph::dumpSceneGraph() const
{
  QStringList strList = QgsFrameGraphUtils::dumpSceneGraph( mRootEntity, QgsFrameGraphUtils::FgDumpContext() );
  return strList.join( "\n" ) + QString( "\n" );
}

void QgsFrameGraph::setClearColor( const QColor &clearColor )
{
  forwardRenderView().setClearColor( clearColor );
}

void QgsFrameGraph::setFrustumCullingEnabled( bool enabled )
{
  forwardRenderView().setFrustumCullingEnabled( enabled );
}

void QgsFrameGraph::setSize( QSize s )
{
  mSize = s;
  for ( auto it = mRenderViewMap.begin(); it != mRenderViewMap.end(); ++it )
  {
    QgsAbstractRenderView *rv = it->second.get();
    rv->updateWindowResize( mSize.width(), mSize.height() );
  }

  mRenderCaptureColorTexture->setSize( mSize.width(), mSize.height() );
  mRenderCaptureDepthTexture->setSize( mSize.width(), mSize.height() );
  mRenderSurfaceSelector->setExternalRenderTargetSize( mSize );
}

Qt3DRender::QRenderCapture *QgsFrameGraph::renderCapture()
{
  return mRenderCapture;
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
  forwardRenderView().setDebugOverlayEnabled( enabled );
}

void QgsFrameGraph::removeClipPlanes()
{
  forwardRenderView().removeClipPlanes();
}

void QgsFrameGraph::addClipPlanes( int nrClipPlanes )
{
  forwardRenderView().addClipPlanes( nrClipPlanes );
}

QgsForwardRenderView &QgsFrameGraph::forwardRenderView()
{
  QgsAbstractRenderView *rv = mRenderViewMap[QgsFrameGraph::FORWARD_RENDERVIEW].get();
  return *( dynamic_cast<QgsForwardRenderView *>( rv ) );
}

QgsShadowRenderView &QgsFrameGraph::shadowRenderView()
{
  QgsAbstractRenderView *rv = mRenderViewMap[QgsFrameGraph::SHADOW_RENDERVIEW].get();
  return *( dynamic_cast<QgsShadowRenderView *>( rv ) );
}

QgsDepthRenderView &QgsFrameGraph::depthRenderView()
{
  QgsAbstractRenderView *rv = mRenderViewMap[QgsFrameGraph::DEPTH_RENDERVIEW].get();
  return *( dynamic_cast<QgsDepthRenderView *>( rv ) );
}

QgsAmbientOcclusionRenderView &QgsFrameGraph::ambientOcclusionRenderView()
{
  QgsAbstractRenderView *rv = mRenderViewMap[QgsFrameGraph::AMBIENT_OCCLUSION_RENDERVIEW].get();
  return *( dynamic_cast<QgsAmbientOcclusionRenderView *>( rv ) );
}

QgsDebugTextureRenderView &QgsFrameGraph::debugTextureRenderView()
{
  QgsAbstractRenderView *renderView = mRenderViewMap[QgsFrameGraph::DEBUG_RENDERVIEW].get();
  return *( dynamic_cast<QgsDebugTextureRenderView *>( renderView ) );
}
