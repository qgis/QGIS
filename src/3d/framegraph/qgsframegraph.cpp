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

#include "qgs3dutils.h"
#include "qgsabstractrenderview.h"
#include "qgsambientocclusionrenderview.h"
#include "qgsbloomrenderview.h"
#include "qgsdepthrenderview.h"
#include "qgsdirectionallightsettings.h"
#include "qgsforwardrenderview.h"
#include "qgsframegraphutils.h"
#include "qgshighlightsrenderview.h"
#include "qgsoverlaytextureentity.h"
#include "qgsoverlaytexturerenderview.h"
#include "qgspostprocessingentity.h"
#include "qgspostprocessingrenderview.h"
#include "qgsrubberbandrenderview.h"
#include "qgsshadowrenderview.h"
#include "qgssunlightsettings.h"

#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>
#include <Qt3DRender/QAbstractTexture>
#include <Qt3DRender/QBlendEquation>
#include <Qt3DRender/QBlendEquationArguments>
#include <Qt3DRender/QBlitFramebuffer>
#include <Qt3DRender/QColorMask>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QNoDepthMask>
#include <Qt3DRender/QNoDraw>
#include <Qt3DRender/QSortPolicy>
#include <Qt3DRender/QTechnique>

#ifdef HAVE_TRACY
#include "tracy/Tracy.hpp"
#endif

#include "moc_qgsframegraph.cpp"

const QString QgsFrameGraph::sForwardRenderView = "forward";
const QString QgsFrameGraph::sShadowRenderView = "shadow";
const QString QgsFrameGraph::sAxiS3DRenderView = "3daxis";
const QString QgsFrameGraph::sDepthRenderView = "depth";
const QString QgsFrameGraph::sOverlayRenderView = "overlay_texture";
const QString QgsFrameGraph::sAmbientOcclusionRenderView = "ambient_occlusion";
const QString QgsFrameGraph::sBloomRenderView = "bloom";
const QString QgsFrameGraph::sPostprocRenderView = "post_processing";
const QString QgsFrameGraph::sHighlightsRenderView = "highlights";
const QString QgsFrameGraph::sRubberRenderView = "rubber_band";

void QgsFrameGraph::constructForwardRenderPass()
{
  registerRenderView( std::make_unique<QgsForwardRenderView>( sForwardRenderView, mMainCamera ), sForwardRenderView );
}

void QgsFrameGraph::constructHighlightsPass()
{
  registerRenderView( std::make_unique<QgsHighlightsRenderView>( sHighlightsRenderView, forwardRenderView().renderTargetSelector()->target(), mMainCamera ), sHighlightsRenderView );
}

void QgsFrameGraph::constructShadowRenderPass()
{
  registerRenderView( std::make_unique<QgsShadowRenderView>( sShadowRenderView, mRootEntity ), sShadowRenderView );
}

void QgsFrameGraph::constructOverlayTexturePass( Qt3DRender::QFrameGraphNode *topNode )
{
  registerRenderView( std::make_unique<QgsOverlayTextureRenderView>( sOverlayRenderView ), sOverlayRenderView, topNode );
}

void QgsFrameGraph::constructPostprocessingPass( Qt3DRender::QFrameGraphNode *topNode )
{
  // create post processing render view and register it
  QgsPostprocessingRenderView *pprv = new QgsPostprocessingRenderView( sPostprocRenderView, this, mSize, mRootEntity );
  registerRenderView( std::unique_ptr<QgsPostprocessingRenderView>( pprv ), sPostprocRenderView, topNode );
}

void QgsFrameGraph::updateThumbnailTextureSize()
{
  // Tracy requires width/height to be multiples of 4, otherwise it will show
  // just a black rectangle.
  const int width = ( mSize.width() / 10 ) & ~3;
  const int height = ( mSize.height() / 10 ) & ~3;
  mThumbnailTexture->setSize( width, height );
}

void QgsFrameGraph::constructThumbnailCapturePass()
{
  // Diagram of the nodes we're creating:
  // mMainViewPort
  // ├── QBlitFramebuffer (copy from forwardRenderView to mThumbnailTexture)
  // └── QRenderTargetSelector (set to mThumbnailTexture)
  //     └── QRenderCapture (mThumbnailCapture)

  mThumbnailTexture = new Qt3DRender::QTexture2D( this );
  mThumbnailTexture->setFormat( Qt3DRender::QAbstractTexture::RGBA8_UNorm );
  updateThumbnailTextureSize();

  Qt3DRender::QRenderTargetOutput *renderTargetOutput = new Qt3DRender::QRenderTargetOutput( this );
  renderTargetOutput->setTexture( mThumbnailTexture );

  Qt3DRender::QRenderTarget *renderTarget = new Qt3DRender::QRenderTarget( this );
  renderTarget->addOutput( renderTargetOutput );

  // Copy from forward render pass to thumbnail texture
  Qt3DRender::QBlitFramebuffer *blit = new Qt3DRender::QBlitFramebuffer( mMainViewPort );
  blit->setObjectName( "Framebuffer copy for thumbnail" );
  blit->setSource( forwardRenderView().renderTargetSelector()->target() );
  blit->setDestination( renderTarget );
  blit->setSourceRect( QRectF( 0, 0, 1, 1 ) );
  blit->setDestinationRect( QRectF( 0, 0, 1, 1 ) );
  blit->setInterpolationMethod( Qt3DRender::QBlitFramebuffer::Linear );

  Qt3DRender::QRenderTargetSelector *targetSelector = new Qt3DRender::QRenderTargetSelector( mMainViewPort );
  targetSelector->setObjectName( "Thumbnail capture target selector" );
  targetSelector->setTarget( renderTarget );

  mThumbnailCapture = new Qt3DRender::QRenderCapture( targetSelector );
  mThumbnailCapture->setObjectName( "Thumbnail capture" );

  // Request initial capture
  // Ugly hack: Requesting just once results in a capture every two frames.
  // "Buffering" an extra capture like this works around that.
  Qt3DRender::QRenderCaptureReply *reply1 = mThumbnailCapture->requestCapture();
  Qt3DRender::QRenderCaptureReply *reply2 = mThumbnailCapture->requestCapture();
  for ( Qt3DRender::QRenderCaptureReply *reply : { reply1, reply2 } )
    connect( reply, &Qt3DRender::QRenderCaptureReply::completed, this, [this, reply]() { onThumbnailCaptureCompleted( reply ); } );
}

void QgsFrameGraph::onThumbnailCaptureCompleted( Qt3DRender::QRenderCaptureReply *reply )
{
  reply->deleteLater();

#ifdef HAVE_TRACY
  // Convert to RGBA8888 format (required by Tracy)
  QImage rgbaImage = reply->image().convertToFormat( QImage::Format_RGBA8888 );

  // Send to Tracy
  FrameImage(
    rgbaImage.constBits(),
    static_cast<uint16_t>( rgbaImage.width() ),
    static_cast<uint16_t>( rgbaImage.height() ),
    0,   // offset: 0 = current frame
    true // flip Y coord: true for OpenGL
  );
#endif

  // Request next frame capture
  Qt3DRender::QRenderCaptureReply *nextReply = mThumbnailCapture->requestCapture();
  connect( nextReply, &Qt3DRender::QRenderCaptureReply::completed, this, [this, nextReply]() { onThumbnailCaptureCompleted( nextReply ); } );
}

void QgsFrameGraph::constructAmbientOcclusionRenderPass()
{
  Qt3DRender::QTexture2D *forwardDepthTexture = forwardRenderView().depthTexture();

  QgsAmbientOcclusionRenderView *aorv = new QgsAmbientOcclusionRenderView( sAmbientOcclusionRenderView, mMainCamera, mSize, forwardDepthTexture, mRootEntity );
  registerRenderView( std::unique_ptr<QgsAmbientOcclusionRenderView>( aorv ), sAmbientOcclusionRenderView );
}

void QgsFrameGraph::constructBloomRenderPass()
{
  Qt3DRender::QTexture2D *forwardColorTexture = forwardRenderView().colorTexture();

  QgsBloomRenderView *rv = new QgsBloomRenderView( sBloomRenderView, forwardColorTexture, mSize, mRootEntity );
  registerRenderView( std::unique_ptr<QgsBloomRenderView>( rv ), sBloomRenderView );
}

void QgsFrameGraph::constructRubberBandsPass( Qt3DRender::QFrameGraphNode *topNode )
{
  // rubber band render view writes to the same output textures than the forward render view:
  QgsRubberBandRenderView *rv = new QgsRubberBandRenderView( sRubberRenderView, mMainCamera, mRootEntity, forwardRenderView().renderTargetSelector()->target() );
  registerRenderView( std::unique_ptr<QgsRubberBandRenderView>( rv ), sRubberRenderView, topNode );
}

void QgsFrameGraph::constructDepthRenderPass()
{
  // entity used to draw the depth texture and convert it to rgb image
  Qt3DRender::QTexture2D *forwardDepthTexture = forwardRenderView().depthTexture();
  QgsDepthRenderView *rv = new QgsDepthRenderView( sDepthRenderView, mSize, forwardDepthTexture, mRootEntity );
  registerRenderView( std::unique_ptr<QgsDepthRenderView>( rv ), sDepthRenderView );
}

Qt3DRender::QRenderCapture *QgsFrameGraph::depthRenderCapture()
{
  return depthRenderView().renderCapture();
}

void QgsFrameGraph::addGlobalParameters( const QList<Qt3DRender::QParameter *> &parameters )
{
  for ( Qt3DRender::QParameter *param : parameters )
  {
    mGlobalParamsStorage->addParameter( param );
  }
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
  //     +---------------------+---------------+------------------------+-------------------+
  //     |                     |               |                        |                   |
  //     |                     |               | (optional)             |                   |
  // +--------------+ +------------------+ +-----------------+  +-----------------+ +-----------------+
  // | shadows pass | | forward passes   | |    MSAA blit    |  |  depth buffer   | | post-processing |
  // |              | | (solid objects,  | | (color + depth) |  | processing pass | |    passes       |
  // +--------------+ | transparent,     | +-----------------+  +-----------------+ +-----------------+
  //                  | highlights,      |
  //                  | rubber bands)    |
  //                  +------------------+
  //
  // Notes:
  // - (optional) MSAA blits multisampled (4 samples) color and depth textures
  //   so that other passes can sample them
  // - shadows pass MUST come before other forward passes, as we use the shadow
  //   information in the material shaders
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

  mRenderSurfaceSelector = new Qt3DRender::QRenderSurfaceSelector;

  QObject *surfaceObj = dynamic_cast<QObject *>( surface );
  Q_ASSERT( surfaceObj );

  mRenderSurfaceSelector->setSurface( surfaceObj );
  mRenderSurfaceSelector->setExternalRenderTargetSize( mSize );

  mMainViewPort = new Qt3DRender::QViewport( mRenderSurfaceSelector );
  mMainViewPort->setNormalizedRect( QRectF( 0.0f, 0.0f, 1.0f, 1.0f ) );

  mGlobalParamsStorage = new Qt3DRender::QRenderPassFilter( mMainViewPort );
  mGlobalParamsStorage->setObjectName( "GlobalParametersStore" );

  // shadow rendering pass -- must be constructed BEFORE the forward render pass,
  // to ensure it always has the correct depth available.
  constructShadowRenderPass();

  // Forward render
  constructForwardRenderPass();

  // Highlighted items pass
  constructHighlightsPass();

  // rubber bands (they should be always on top)
  constructRubberBandsPass( mGlobalParamsStorage );

  mMsaaBlitNode = new Qt3DRender::QBlitFramebuffer( mGlobalParamsStorage );
  mMsaaBlitNode->setObjectName( "MsaaBlitFramebuffer" );
  mMsaaBlitNode->setEnabled( false );

  mMsaaDepthBlitNode = new Qt3DRender::QBlitFramebuffer( mGlobalParamsStorage );
  mMsaaDepthBlitNode->setObjectName( "MsaaDepthBlitFramebuffer" );
  mMsaaDepthBlitNode->setEnabled( false );

  // depth buffer processing
  constructDepthRenderPass();

  // Ambient occlusion factor render pass
  constructAmbientOcclusionRenderPass();

  constructBloomRenderPass();

  // post process
  constructPostprocessingPass( mGlobalParamsStorage );

#ifdef HAVE_TRACY
  constructThumbnailCapturePass();
#endif
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
    mRenderViewMap[name]->topGraphNode()->setParent( topNode ? topNode : mGlobalParamsStorage );
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

  postprocessingRenderView().entity()->setAmbientOcclusionEnabled( settings.isEnabled() );
}

void QgsFrameGraph::updateEyeDomeSettings( const Qgs3DMapSettings &settings )
{
  postprocessingRenderView().entity()->setEyeDomeLightingEnabled( settings.eyeDomeLightingEnabled() );
  postprocessingRenderView().entity()->updateEyeDomeSettings( settings );
}

void QgsFrameGraph::updateBloomSettings( const QgsBloomSettings &settings )
{
  QgsBloomRenderView &renderView = bloomRenderView();

  renderView.setEnabled( settings.isEnabled() );
  renderView.setFilterRadius( static_cast< float >( settings.radius() ) );

  postprocessingRenderView().entity()->setBloomEnabled( settings.isEnabled() );
  postprocessingRenderView().entity()->updateBloomSettings( settings );
}

void QgsFrameGraph::updateColorGradingSettings( const QgsColorGradingSettings &settings )
{
  postprocessingRenderView().entity()->updateColorGradingSettings( settings );
}

void QgsFrameGraph::updateShadowSettings( const Qgs3DMapSettings &mapSettings )
{
  const QgsShadowSettings shadowSettings = mapSettings.shadowSettings();
  const QList<QgsLightSource *> lightSources = mapSettings.lightSources();
  if ( shadowSettings.renderShadows() )
  {
    const QString lightSourceId = shadowSettings.lightSource();
    QgsLightSource *light = nullptr;
    int globalLightIndex = 0;

    QgsLightSource *backupLightSource = nullptr;
    int backupLightSourceIndex = 0;
    for ( int i = 0; !light && i < lightSources.size(); i++ )
    {
      if ( !backupLightSource )
      {
        if ( auto directionalLight = dynamic_cast< QgsDirectionalLightSettings * >( lightSources[i] ) )
        {
          backupLightSource = directionalLight;
          backupLightSourceIndex = i;
        }
        else if ( auto sunLight = dynamic_cast< QgsSunLightSettings * >( lightSources[i] ) )
        {
          backupLightSource = sunLight;
          backupLightSourceIndex = i;
        }
      }

      if ( lightSources[i]->id() == lightSourceId )
      {
        light = lightSources[i];
        globalLightIndex = i;
      }
    }

    if ( !light )
    {
      // if we didn't find an light matching the exact ID requested, then
      // fallback to just the first compatible light found in the scene
      light = backupLightSource;
      globalLightIndex = backupLightSourceIndex;
    }

    if ( light )
    {
      QgsVector3D lightDirection;
      if ( auto directionalLight = dynamic_cast< QgsDirectionalLightSettings * >( light ) )
      {
        lightDirection = directionalLight->direction();
      }
      else if ( auto sunLight = dynamic_cast< QgsSunLightSettings * >( light ) )
      {
        lightDirection = sunLight->direction( mapSettings );
      }

      const int size = shadowSettings.qualityToMapResolution( shadowSettings.shadowQuality() );
      shadowRenderView().setMapSize( size, size );
      shadowRenderView().setEnabled( true );

      postprocessingRenderView().entity()->setShadowRenderingEnabled( true );
      postprocessingRenderView().entity()->updateShadowSettings( shadowSettings, lightDirection, size, globalLightIndex );
    }
  }
  else
  {
    shadowRenderView().setEnabled( false );
    postprocessingRenderView().entity()->setShadowRenderingEnabled( false );
  }
}

void QgsFrameGraph::updateDebugDepthMapSettings( const Qgs3DMapSettings &settings )
{
  QgsOverlayTextureRenderView &overlayRenderView = overlayTextureRenderView();

  if ( !mDepthTextureDebugging && settings.debugDepthMapEnabled() )
  {
    Qt3DRender::QTexture2D *forwardDepthTexture = forwardRenderView().depthTexture();
    mDepthTextureDebugging = new QgsOverlayTextureEntity( forwardDepthTexture, overlayRenderView.overlayLayer(), this );
  }

  overlayRenderView.setEnabled( settings.debugDepthMapEnabled() || settings.is2DMapOverlayEnabled() );

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

  mRenderSurfaceSelector->setExternalRenderTargetSize( mSize );

  mMsaaBlitNode->setSourceRect( QRect( 0, 0, mSize.width(), mSize.height() ) );
  mMsaaBlitNode->setDestinationRect( QRect( 0, 0, mSize.width(), mSize.height() ) );
  mMsaaDepthBlitNode->setSourceRect( QRect( 0, 0, mSize.width(), mSize.height() ) );
  mMsaaDepthBlitNode->setDestinationRect( QRect( 0, 0, mSize.width(), mSize.height() ) );

  if ( mThumbnailTexture )
    updateThumbnailTextureSize();
}

Qt3DRender::QRenderCapture *QgsFrameGraph::renderCapture()
{
  return postprocessingRenderView().renderCapture();
}

void QgsFrameGraph::setRenderCaptureEnabled( bool enabled )
{
  postprocessingRenderView().setOffScreenRenderCaptureEnabled( enabled );
}

void QgsFrameGraph::setDebugOverlayEnabled( bool enabled )
{
  forwardRenderView().setDebugOverlayEnabled( enabled );
}

void QgsFrameGraph::setMsaaEnabled( bool enabled )
{
  mMsaaEnabled = enabled;

  if ( !enabled && mMsaaBlitConfigured )
  {
    mMsaaBlitNode->setSource( nullptr );
    mMsaaBlitNode->setDestination( nullptr );
    mMsaaDepthBlitNode->setSource( nullptr );
    mMsaaDepthBlitNode->setDestination( nullptr );
    mMsaaBlitConfigured = false;
  }

  forwardRenderView().setMsaaEnabled( enabled );

  if ( enabled && !mMsaaBlitConfigured )
  {
    mMsaaBlitConfigured = true;
    mMsaaBlitNode->setSource( forwardRenderView().msaaRenderTarget() );
    mMsaaBlitNode->setDestination( forwardRenderView().regularRenderTarget() );
    mMsaaBlitNode->setSourceRect( QRect( 0, 0, mSize.width(), mSize.height() ) );
    mMsaaBlitNode->setDestinationRect( QRect( 0, 0, mSize.width(), mSize.height() ) );
    mMsaaBlitNode->setSourceAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );
    mMsaaBlitNode->setDestinationAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );
    mMsaaBlitNode->setInterpolationMethod( Qt3DRender::QBlitFramebuffer::Nearest );

    mMsaaDepthBlitNode->setSource( forwardRenderView().msaaRenderTarget() );
    mMsaaDepthBlitNode->setDestination( forwardRenderView().regularRenderTarget() );
    mMsaaDepthBlitNode->setSourceRect( QRect( 0, 0, mSize.width(), mSize.height() ) );
    mMsaaDepthBlitNode->setDestinationRect( QRect( 0, 0, mSize.width(), mSize.height() ) );
    mMsaaDepthBlitNode->setSourceAttachmentPoint( Qt3DRender::QRenderTargetOutput::DepthStencil );
    mMsaaDepthBlitNode->setDestinationAttachmentPoint( Qt3DRender::QRenderTargetOutput::DepthStencil );
    mMsaaDepthBlitNode->setInterpolationMethod( Qt3DRender::QBlitFramebuffer::Nearest );
  }

  Qt3DRender::QRenderTarget *target = enabled ? forwardRenderView().msaaRenderTarget() : forwardRenderView().regularRenderTarget();
  highlightsRenderView().setRenderTarget( target );
  rubberBandRenderView().renderTargetSelector()->setTarget( target );
  mMsaaBlitNode->setEnabled( enabled );
  mMsaaDepthBlitNode->setEnabled( enabled );
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
  QgsAbstractRenderView *rv = mRenderViewMap[QgsFrameGraph::sForwardRenderView].get();
  return *( dynamic_cast<QgsForwardRenderView *>( rv ) );
}

QgsShadowRenderView &QgsFrameGraph::shadowRenderView()
{
  QgsAbstractRenderView *rv = mRenderViewMap[QgsFrameGraph::sShadowRenderView].get();
  return *( dynamic_cast<QgsShadowRenderView *>( rv ) );
}

QgsDepthRenderView &QgsFrameGraph::depthRenderView()
{
  QgsAbstractRenderView *rv = mRenderViewMap[QgsFrameGraph::sDepthRenderView].get();
  return *( dynamic_cast<QgsDepthRenderView *>( rv ) );
}

QgsAmbientOcclusionRenderView &QgsFrameGraph::ambientOcclusionRenderView()
{
  QgsAbstractRenderView *rv = mRenderViewMap[QgsFrameGraph::sAmbientOcclusionRenderView].get();
  return *( dynamic_cast<QgsAmbientOcclusionRenderView *>( rv ) );
}

QgsBloomRenderView &QgsFrameGraph::bloomRenderView()
{
  QgsAbstractRenderView *rv = mRenderViewMap[QgsFrameGraph::sBloomRenderView].get();
  return *( dynamic_cast<QgsBloomRenderView *>( rv ) );
}

QgsPostprocessingRenderView &QgsFrameGraph::postprocessingRenderView()
{
  QgsAbstractRenderView *rv = mRenderViewMap[QgsFrameGraph::sPostprocRenderView].get();
  return *( dynamic_cast<QgsPostprocessingRenderView *>( rv ) );
}

QgsHighlightsRenderView &QgsFrameGraph::highlightsRenderView()
{
  QgsAbstractRenderView *rv = mRenderViewMap[QgsFrameGraph::sHighlightsRenderView].get();
  return *( dynamic_cast<QgsHighlightsRenderView *>( rv ) );
}

QgsOverlayTextureRenderView &QgsFrameGraph::overlayTextureRenderView()
{
  return *( postprocessingRenderView().overlayTextureRenderView() );
}

QgsRubberBandRenderView &QgsFrameGraph::rubberBandRenderView()
{
  QgsAbstractRenderView *rv = mRenderViewMap[QgsFrameGraph::sRubberRenderView].get();
  return *( dynamic_cast<QgsRubberBandRenderView *>( rv ) );
}
