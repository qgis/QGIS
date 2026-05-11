/***************************************************************************
  qgspostprocessingentity.h
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

#ifndef QGSPOSTPROCESSINGENTITY_H
#define QGSPOSTPROCESSINGENTITY_H

#include "qgs3d.h"
#include "qgsrenderpassquad.h"

#define SIP_NO_FILE

class QgsFrameGraph;
class QgsShadowRenderView;
class QgsDirectionalLightSettings;

namespace Qt3DRender
{
  class QCamera;
  class QParameter;
} //namespace Qt3DRender


/**
 * \ingroup qgis_3d
 * \brief An entity that is responsible for applying post processing effects.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.16
 */
class QgsPostprocessingEntity : public QgsRenderPassQuad
{
    Q_OBJECT

  public:
    //! Constructor
    QgsPostprocessingEntity( QgsFrameGraph *frameGraph, Qt3DRender::QLayer *layer, QNode *parent = nullptr );
    //! Sets whether shadow rendering is enabled
    void setShadowRenderingEnabled( bool enabled );

    /**
     * Sets whether the splits between cascading shadow map boundaries should be shown.
     *
     * \warning For testing purposes only!
     */
    void setShowCascadingShadowSplits( bool enabled );

    //! Sets the shadow bias value
    void setShadowBias( float shadowBias );
    //! Sets whether eye dome lighting is enabled
    void setEyeDomeLightingEnabled( bool enabled );
    //! Sets the eye dome lighting strength
    void setEyeDomeLightingStrength( double strength );
    //! Sets the eye dome lighting distance (contributes to the contrast of the image)
    void setEyeDomeLightingDistance( int distance );

    /**
     * Sets shadow rendering to use a directional light
     * \since QGIS 3.44
     */
    void updateShadowSettings( const QgsDirectionalLightSettings &light, float maximumShadowRenderingDistance );

    /**
     * Sets whether screen space ambient occlusion is enabled
     * \since QGIS 3.28
     */
    void setAmbientOcclusionEnabled( bool enabled );

  private:
    Qt3DRender::QCamera *mMainCamera = nullptr;

    Qt3DRender::QParameter *mColorTextureParameter = nullptr;
    Qt3DRender::QParameter *mDepthTextureParameter = nullptr;
    Qt3DRender::QParameter *mShadowMapParameter = nullptr;
    Qt3DRender::QParameter *mAmbientOcclusionTextureParameter = nullptr;
    Qt3DRender::QParameter *mFarPlaneParameter = nullptr;
    Qt3DRender::QParameter *mNearPlaneParameter = nullptr;
    Qt3DRender::QParameter *mMainCameraInvViewMatrixParameter = nullptr;
    Qt3DRender::QParameter *mMainCameraInvProjMatrixParameter = nullptr;

    Qt3DRender::QCamera *mLightCameras[Qgs3D::NUM_SHADOW_CASCADES] = { nullptr };
    Qt3DRender::QParameter *mCsmMatricesParameter = nullptr;
#if 0 // required for interval based cascades only
    Qt3DRender::QParameter *mCsmSplitsParameter = nullptr;
#endif

    Qt3DRender::QParameter *mRenderShadowsParameter = nullptr;
    Qt3DRender::QParameter *mShadowBiasParameter = nullptr;
    Qt3DRender::QParameter *mEyeDomeLightingEnabledParameter = nullptr;
    Qt3DRender::QParameter *mEyeDomeLightingStrengthParameter = nullptr;
    Qt3DRender::QParameter *mEyeDomeLightingDistanceParameter = nullptr;

    Qt3DRender::QParameter *mAmbientOcclusionEnabledParameter = nullptr;
};

#endif // QGSPOSTPROCESSINGENTITY_H
