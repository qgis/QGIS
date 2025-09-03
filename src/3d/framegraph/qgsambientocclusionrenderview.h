/***************************************************************************
  qgsambientocclusionrenderview.h
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

#ifndef QGSAMBIENTOCCLUSIONRENDERVIEW_H
#define QGSAMBIENTOCCLUSIONRENDERVIEW_H

#include "qgsabstractrenderview.h"

#define SIP_NO_FILE

namespace Qt3DRender
{
  class QRenderSettings;
  class QLayer;
  class QSubtreeEnabler;
  class QTexture2D;
  class QCamera;
  class QCameraSelector;
  class QLayerFilter;
  class QRenderTargetSelector;
  class QRenderTarget;
} //namespace Qt3DRender

namespace Qt3DCore
{
  class QEntity;
} //namespace Qt3DCore

class QgsAmbientOcclusionRenderEntity;
class QgsAmbientOcclusionBlurEntity;

/**
 * \ingroup qgis_3d
 * \brief Container class that holds different objects related to ambient occlusion rendering
 *
 * \note Not available in Python bindings
 *
 * This renderview create 2 passes with their own entity: 
 *  - the first pass computes the ambient occlusion (creates a QgsAmbientOcclusionRenderEntity)
 *  - the second generates a blur (creates a QgsAmbientOcclusionBlurEntity)
 * 
 * \since QGIS 3.44
 */
class QgsAmbientOcclusionRenderView : public QgsAbstractRenderView
{
  public:
    //! Default constructor
    QgsAmbientOcclusionRenderView( const QString &viewName, Qt3DRender::QCamera *mainCamera, QSize mSize, Qt3DRender::QTexture2D *forwardDepthTexture, Qt3DCore::QEntity *rootSceneEntity );

    //! Delegates to QgsAmbientOcclusionRenderEntity::setIntensity
    void setIntensity( float intensity );

    //! Delegates to QgsAmbientOcclusionRenderEntity::setRadius
    void setRadius( float radius );

    //! Delegates to QgsAmbientOcclusionRenderEntity::setThreshold
    void setThreshold( float threshold );

    //! Returns blur pass texture
    Qt3DRender::QTexture2D *blurredFactorMapTexture() const;

    virtual void updateWindowResize( int width, int height ) override;
    virtual void setEnabled( bool enable ) override;

  private:
    Qt3DRender::QLayer *mAOPassLayer = nullptr;
    Qt3DRender::QTexture2D *mAOPassTexture = nullptr;
    Qt3DRender::QTexture2D *mBlurPassTexture = nullptr;
    Qt3DRender::QLayer *mBlurPassLayer = nullptr;

    QgsAmbientOcclusionRenderEntity *mAmbientOcclusionRenderEntity = nullptr;
    QgsAmbientOcclusionBlurEntity *mAmbientOcclusionBlurEntity = nullptr;

    void buildRenderPasses( QSize mSize, Qt3DRender::QTexture2D *forwardDepthTexture, Qt3DCore::QEntity *rootSceneEntity, Qt3DRender::QCamera *mainCamera );

    /**
     * Build AO texture and add it to a new rendertarget
     */
    Qt3DRender::QRenderTarget *buildAOTexture( QSize mSize );

    /**
     * Build blur texture and add it to a new rendertarget
     */
    Qt3DRender::QRenderTarget *buildBlurTexture( QSize mSize );
};

#endif // QGSAMBIENTOCCLUSIONRENDERVIEW_H
