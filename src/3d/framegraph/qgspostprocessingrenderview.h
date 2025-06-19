/***************************************************************************
  qgspostprocessingrenderview.h
  --------------------------------------
  Date                 : May 2025
  Copyright            : (C) 2025 by Benoit De Mezzo and (C) 2020 by Belgacem Nedjima
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

namespace Qt3DRender
{
  class QTexture2D;
  class QRenderTargetSelector;
  class QClearBuffers;
  class QRenderStateSet;
  class QRenderCapture;
  class QRenderTarget;
} //namespace Qt3DRender

class QgsPostprocessingEntity;

#define SIP_NO_FILE

/**
 * \ingroup qgis_3d
 * \brief Container class that holds different objects related to postprocessing rendering
 *
 * \note Not available in Python bindings
 *
 * The postprocessing pass apply post-processing effects (shadows, EDL, SSAO).
 *
 * \since QGIS 3.44
 */
class QgsPostprocessingRenderView : public QgsAbstractRenderView
{
  public:
    //! Default constructor
    QgsPostprocessingRenderView( const QString &viewName,                     //
                                 QgsShadowRenderView &shadowRenderView,       //
                                 QgsForwardRenderView &forwardRenderView,     //
                                 QgsAmbientOcclusionRenderView &aoRenderView, //
                                 QSize mSize,                                 //
                                 Qt3DCore::QEntity *rootSceneEntity );

    virtual void updateWindowResize( int width, int height ) override;

    //! Returns the render capture object used to take an image of the postprocessing buffer of the scene
    Qt3DRender::QRenderCapture *renderCapture() const;

    //! Returns the QT3D entity used to do the rendering
    QgsPostprocessingEntity *entity() const;

    /**
     * Sets whether it will be possible to render to an image
     */
    void setOffScreenRenderCaptureEnabled( bool enabled );

    //! Returns the top node of all subpasses
    QVector<Qt3DRender::QFrameGraphNode *> subPasses() const;

    /**
     * Updates the subpasses with the new \a topNodes
     */
    void setSubPasses( QVector<Qt3DRender::QFrameGraphNode *> topNodes );

  private:
    Qt3DRender::QRenderTarget *buildRenderCaptureTextures( QSize mSize );
    Qt3DRender::QFrameGraphNode *constructPostprocessingMainPass( QSize mSize );
    Qt3DRender::QFrameGraphNode *constructSubPostPassForProcessing( QgsShadowRenderView &shadowRenderView,       //
                                                                    QgsForwardRenderView &forwardRenderView,     //
                                                                    QgsAmbientOcclusionRenderView &aoRenderView, //
                                                                    Qt3DCore::QEntity *rootSceneEntity );
    Qt3DRender::QFrameGraphNode *constructSubPostPassForRenderCapture();

    Qt3DRender::QRenderTargetSelector *mRenderCaptureTargetSelector = nullptr;
    Qt3DRender::QRenderCapture *mRenderCapture = nullptr;

    QgsPostprocessingEntity *mPostprocessingEntity = nullptr;

    // Post processing pass texture related objects:
    Qt3DRender::QTexture2D *mRenderCaptureColorTexture = nullptr;
    Qt3DRender::QTexture2D *mRenderCaptureDepthTexture = nullptr;

    Qt3DRender::QFrameGraphNode *mSubPassesNode = nullptr;
};

#endif // QGSPOSTPROCESSINGRENDERVIEW_H
