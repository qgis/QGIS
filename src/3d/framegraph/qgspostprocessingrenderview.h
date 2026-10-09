/***************************************************************************
  qgspostprocessingrenderview.h
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Benoit De Mezzo and (C) 2020 by Belgacem Nedjima
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOSTPROCESSINGRENDERVIEW_H
#define QGSPOSTPROCESSINGRENDERVIEW_H

#include "qgsabstractrenderview.h"

#define SIP_NO_FILE

namespace Qt3DCore
{
  class QEntity;
} //namespace Qt3DCore

namespace Qt3DRender
{
  class QTexture2D;
  class QRenderTargetSelector;
  class QClearBuffers;
  class QRenderStateSet;
  class QRenderCapture;
  class QRenderTarget;
  class QLayer;
} //namespace Qt3DRender

class QgsFrameGraph;
class QgsPostprocessingEntity;
class QgsOverlayTextureRenderView;

/**
 * \ingroup qgis_3d
 * \brief Container class that holds different objects related to postprocessing rendering
 *
 * \note Not available in Python bindings
 *
 * The postprocessing pass apply post-processing effects (EDL, SSAO).
 *
 * \since QGIS 4.2
 */
class QgsPostprocessingRenderView : public QgsAbstractRenderView
{
  public:
    //! Default constructor
    QgsPostprocessingRenderView( const QString &viewName, QgsFrameGraph *frameGraph, QSize size, Qt3DCore::QEntity *rootSceneEntity );

    virtual void updateWindowResize( int width, int height ) override;

    //! Returns the render capture object used to take an image of the postprocessing buffer of the scene
    Qt3DRender::QRenderCapture *renderCapture() const;

    //! Returns the QT3D entity used to do the rendering
    QgsPostprocessingEntity *entity() const;

    /**
     * Sets whether it will be possible to render to an image
     */
    void setOffScreenRenderCaptureEnabled( bool enabled );

    //! Returns overlay texture render view
    QgsOverlayTextureRenderView *overlayTextureRenderView() const { return mOverlayTextureRenderView.get(); }

  private:
    Qt3DRender::QRenderTarget *buildRenderCaptureTextures( QSize size );
    Qt3DRender::QFrameGraphNode *constructMainPass( QSize size );
    Qt3DRender::QFrameGraphNode *constructSubPassForProcessing( QgsFrameGraph *frameGraph, Qt3DCore::QEntity *rootSceneEntity );
    Qt3DRender::QFrameGraphNode *constructSubPassForRenderCapture();
    Qt3DRender::QFrameGraphNode *constructSubPassForOverlayTexture();

    std::unique_ptr<QgsOverlayTextureRenderView> mOverlayTextureRenderView;

    Qt3DRender::QRenderTargetSelector *mRenderCaptureTargetSelector = nullptr;
    Qt3DRender::QRenderCapture *mRenderCapture = nullptr;

    QgsPostprocessingEntity *mPostprocessingEntity = nullptr;

    // Post processing pass texture related objects:
    Qt3DRender::QTexture2D *mRenderCaptureColorTexture = nullptr;
    Qt3DRender::QTexture2D *mRenderCaptureDepthTexture = nullptr;
};

#endif // QGSPOSTPROCESSINGRENDERVIEW_H
