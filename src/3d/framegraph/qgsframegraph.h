/***************************************************************************
  qgsframegraph.h
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

#ifndef QGSFRAMEGRAPH_H
#define QGSFRAMEGRAPH_H

#include <map>
#include <QWindow>
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
#include <Qt3DRender/QRenderCapture>
#include <Qt3DRender/QDebugOverlay>

#include "qgspointlightsettings.h"

class QgsDirectionalLightSettings;
class QgsCameraController;
class QgsRectangle;
class QgsPostprocessingEntity;
class QgsAmbientOcclusionRenderEntity;
class QgsAmbientOcclusionBlurEntity;
class QgsAbstractRenderView;
class QgsForwardRenderView;
class QgsShadowRenderView;
class QgsDepthRenderView;
class QgsShadowSettings;
class QgsDebugTextureEntity;

#define SIP_NO_FILE

/**
 * \ingroup qgis_3d
 * \brief Container class that holds different objects related to frame graphs of 3D scenes.
 *
 * A frame graph captures configuration of rendering passes when 3D scene gets rendered.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.16
 */
class QgsFrameGraph : public Qt3DCore::QEntity
{
    Q_OBJECT

  public:
    //! Constructor
    QgsFrameGraph( QSurface *surface, QSize s, Qt3DRender::QCamera *mainCamera, Qt3DCore::QEntity *root );

    //! Returns the root of the frame graph object
    Qt3DRender::QFrameGraphNode *frameGraphRoot() { return mRenderSurfaceSelector; }

    /**
     * Returns blurred ambient occlusion factor values texture
     * \since QGIS 3.28
     */
    Qt3DRender::QTexture2D *blurredAmbientOcclusionFactorMap() { return mAmbientOcclusionBlurTexture; }

    //! Returns the main camera
    Qt3DRender::QCamera *mainCamera() { return mMainCamera; }

    //! Returns the postprocessing entity
    QgsPostprocessingEntity *postprocessingEntity() { return mPostprocessingEntity; }

    //! Returns entity for all rubber bands (to show them always on top)
    Qt3DCore::QEntity *rubberBandsRootEntity() { return mRubberBandsRootEntity; }

    //! Returns the render capture object used to take an image of the scene
    Qt3DRender::QRenderCapture *renderCapture() { return mRenderCapture; }

    //! Returns the render capture object used to take an image of the depth buffer of the scene
    Qt3DRender::QRenderCapture *depthRenderCapture();

    //! Sets whether frustum culling is enabled
    void setFrustumCullingEnabled( bool enabled );

    /**
     * Sets whether Screen Space Ambient Occlusion will be enabled
     * \since QGIS 3.28
     */
    void setAmbientOcclusionEnabled( bool enabled );

    /**
     * Returns whether Screen Space Ambient Occlusion is enabled
     * \since QGIS 3.28
     */
    bool ambientOcclusionEnabled() const { return mAmbientOcclusionEnabled; }

    /**
     * Sets the ambient occlusion intensity
     * \since QGIS 3.28
     */
    void setAmbientOcclusionIntensity( float intensity );

    /**
     * Returns the ambient occlusion intensity
     * \since QGIS 3.28
     */
    float ambientOcclusionIntensity() const { return mAmbientOcclusionIntensity; }

    /**
     * Sets the ambient occlusion radius
     * \since QGIS 3.28
     */
    void setAmbientOcclusionRadius( float radius );

    /**
     * Returns the ambient occlusion radius
     * \since QGIS 3.28
     */
    float ambientOcclusionRadius() const { return mAmbientOcclusionRadius; }

    /**
     * Sets the ambient occlusion threshold
     * \since QGIS 3.28
     */
    void setAmbientOcclusionThreshold( float threshold );

    /**
     * Returns the ambient occlusion threshold
     * \since QGIS 3.28
     */
    float ambientOcclusionThreshold() const { return mAmbientOcclusionThreshold; }

    //! Sets the clear color of the scene (background color)
    void setClearColor( const QColor &clearColor );

    //! Sets eye dome lighting shading related settings
    void setupEyeDomeLighting( bool enabled, double strength, int distance );
    //! Sets the size of the buffers used for rendering
    void setSize( QSize s );

    /**
     * Sets whether it will be possible to render to an image
     * \since QGIS 3.18
     */
    void setRenderCaptureEnabled( bool enabled );

    /**
     * Returns whether it will be possible to render to an image
     * \since QGIS 3.18
     */
    bool renderCaptureEnabled() const { return mRenderCaptureEnabled; }

    /**
     * Sets whether debug overlay is enabled
     * \since QGIS 3.26
     */
    void setDebugOverlayEnabled( bool enabled );

    //! Dumps frame graph as string
    QString dumpFrameGraph() const;

    //! Dumps scene graph as string
    QString dumpSceneGraph() const;

    /**
     * Setups \a nrClipPlanes clip planes in the forward pass to enable OpenGL clipping.
     * If \a nrClipPlanes is equal to 0, the clipping is disabled.
     *
     * \see removeClipPlanes()
     * \since QGIS 3.40
    */
    void addClipPlanes( int nrClipPlanes );

    /**
     * Disables OpenGL clipping
     *
     * \see addClipPlanes()
     * \since QGIS 3.40
    */
    void removeClipPlanes();

    /**
     * Registers a new the render view \a renderView with name \a name.
     *
     * \a topNode is where the new renderview will be attached to. If nullptr then it will be attached to the main viewport.
     * Will take ownership of the renderView.
     * \since QGIS 3.44
     */
    bool registerRenderView( std::unique_ptr<QgsAbstractRenderView> renderView, const QString &name, Qt3DRender::QFrameGraphNode *topNode = nullptr );

    /**
     * Unregisters the render view named \a name, if any
     * \since QGIS 3.44
     */
    void unregisterRenderView( const QString &name );

    /**
     * Enables or disables the render view named \a name according to \a enable
     * \since QGIS 3.44
     */
    void setRenderViewEnabled( const QString &name, bool enable );

    /**
     * Returns true if the render view named \a name is found and enabled
     * \since QGIS 3.44
     */
    bool isRenderViewEnabled( const QString &name );

    /**
     * Returns the render view named \a name, if any
     * \since QGIS 3.44
     */
    QgsAbstractRenderView *renderView( const QString &name );

    /**
     * Returns shadow renderview
     * \since QGIS 3.44
     */
    QgsShadowRenderView &shadowRenderView();

    /**
     * Returns forward renderview
     * \since QGIS 3.44
     */
    QgsForwardRenderView &forwardRenderView();

    /**
     * Returns depth renderview
     * \since QGIS 3.44
     */
    QgsDepthRenderView &depthRenderView();

    /**
     * Updates shadow bias, light and texture size according to \a shadowSettings and \a lightSources
     * \since QGIS 3.44
     */
    void updateShadowSettings( const QgsShadowSettings &shadowSettings, const QList<QgsLightSource *> &lightSources );

    /**
     * Updates settings for shadows debug map
     * \since QGIS 3.44
     */
    void updateDebugShadowMapSettings( const Qgs3DMapSettings &settings );

    /**
     * Updates settings for depth debug map
     * \since QGIS 3.44
     */
    void updateDebugDepthMapSettings( const Qgs3DMapSettings &settings );

    static const QString FORWARD_RENDERVIEW;
    static const QString SHADOW_RENDERVIEW;
    static const QString AXIS3D_RENDERVIEW;
    static const QString DEPTH_RENDERVIEW;
    static const QString DEBUG_RENDERVIEW;

  private:
    Qt3DRender::QRenderSurfaceSelector *mRenderSurfaceSelector = nullptr;
    Qt3DRender::QViewport *mMainViewPort = nullptr;

    Qt3DRender::QCamera *mMainCamera = nullptr;

    // Post processing pass branch nodes:
    Qt3DRender::QRenderTargetSelector *mRenderCaptureTargetSelector = nullptr;
    Qt3DRender::QRenderCapture *mRenderCapture = nullptr;
    // Post processing pass texture related objects:
    Qt3DRender::QTexture2D *mRenderCaptureColorTexture = nullptr;
    Qt3DRender::QTexture2D *mRenderCaptureDepthTexture = nullptr;

    // Ambient occlusion factor generation pass
    Qt3DRender::QCameraSelector *mAmbientOcclusionRenderCameraSelector = nullptr;
    Qt3DRender::QRenderStateSet *mAmbientOcclusionRenderStateSet = nullptr;
    Qt3DRender::QLayerFilter *mAmbientOcclusionRenderLayerFilter = nullptr;
    Qt3DRender::QRenderTargetSelector *mAmbientOcclusionRenderCaptureTargetSelector = nullptr;
    // Ambient occlusion factor generation pass texture related objects:
    Qt3DRender::QTexture2D *mAmbientOcclusionRenderTexture = nullptr;

    // Ambient occlusion factor blur pass
    Qt3DRender::QCameraSelector *mAmbientOcclusionBlurCameraSelector = nullptr;
    Qt3DRender::QRenderStateSet *mAmbientOcclusionBlurStateSet = nullptr;
    Qt3DRender::QLayerFilter *mAmbientOcclusionBlurLayerFilter = nullptr;
    Qt3DRender::QRenderTargetSelector *mAmbientOcclusionBlurRenderCaptureTargetSelector = nullptr;
    // Ambient occlusion factor blur pass texture related objects:
    Qt3DRender::QTexture2D *mAmbientOcclusionBlurTexture = nullptr;

    // Rubber bands pass
    Qt3DRender::QCameraSelector *mRubberBandsCameraSelector = nullptr;
    Qt3DRender::QLayerFilter *mRubberBandsLayerFilter = nullptr;
    Qt3DRender::QRenderStateSet *mRubberBandsStateSet = nullptr;
    Qt3DRender::QRenderTargetSelector *mRubberBandsRenderTargetSelector = nullptr;

    // Ambient occlusion related settings
    bool mAmbientOcclusionEnabled = false;
    float mAmbientOcclusionIntensity = 0.5f;
    float mAmbientOcclusionRadius = 25.f;
    float mAmbientOcclusionThreshold = 0.5f;

    QSize mSize = QSize( 1024, 768 );

    bool mEyeDomeLightingEnabled = false;
    double mEyeDomeLightingStrength = 1000.0;
    int mEyeDomeLightingDistance = 1;

    QVector3D mLightDirection = QVector3D( 0.0, -1.0f, 0.0f );

    Qt3DCore::QEntity *mRootEntity = nullptr;

    Qt3DRender::QLayer *mRubberBandsLayer = nullptr;

    QgsPostprocessingEntity *mPostprocessingEntity = nullptr;
    QgsAmbientOcclusionRenderEntity *mAmbientOcclusionRenderEntity = nullptr;
    QgsAmbientOcclusionBlurEntity *mAmbientOcclusionBlurEntity = nullptr;

    Qt3DCore::QEntity *mRubberBandsRootEntity = nullptr;

    //! shadow texture debugging
    QgsDebugTextureEntity *mShadowTextureDebugging = nullptr;
    //! depth texture debugging
    QgsDebugTextureEntity *mDepthTextureDebugging = nullptr;

    void constructShadowRenderPass();
    void constructForwardRenderPass();
    void constructDebugTexturePass( Qt3DRender::QFrameGraphNode *topNode = nullptr );
    Qt3DRender::QFrameGraphNode *constructPostprocessingPass();
    void constructDepthRenderPass();
    Qt3DRender::QFrameGraphNode *constructAmbientOcclusionRenderPass();
    Qt3DRender::QFrameGraphNode *constructAmbientOcclusionBlurPass();
    Qt3DRender::QFrameGraphNode *constructRubberBandsPass();

    Qt3DRender::QFrameGraphNode *constructSubPostPassForProcessing();
    Qt3DRender::QFrameGraphNode *constructSubPostPassForRenderCapture();
    Qt3DRender::QFrameGraphNode *constructSubPostPassForTexturesPreview();

    bool mRenderCaptureEnabled = false;

    // holds renderviews according to their name
    std::map<QString, std::unique_ptr<QgsAbstractRenderView>> mRenderViewMap;

    Q_DISABLE_COPY( QgsFrameGraph )
};

#endif // QGSFRAMEGRAPH_H
