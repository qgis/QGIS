/***************************************************************************
  qgsssaorenderentity.h
  --------------------------------------
  Date                 : Juin 2022
  Copyright            : (C) 2022 by Belgacem Nedjima
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSSAORENDERENTITY_H
#define QGSSSAORENDERENTITY_H

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QCamera>

class QgsShadowRenderingFrameGraph;

#define SIP_NO_FILE

class QgsSsaoRenderEntity : public Qt3DCore::QEntity
{
  public:
    //! Constructor
    QgsSsaoRenderEntity( QgsShadowRenderingFrameGraph *frameGraph, QNode *parent = nullptr );

    void setShadingFactor( float factor );
    void setDistanceAttenuationFactor( float factor );
    void setRadiusParameter( float radius );

  private:
    Qt3DRender::QMaterial *mMaterial = nullptr;
    Qt3DRender::QEffect *mEffect = nullptr;
    Qt3DRender::QParameter *mColorTextureParameter = nullptr;
    Qt3DRender::QParameter *mDepthTextureParameter = nullptr;
    Qt3DRender::QParameter *mShadowMapParameter = nullptr;
    Qt3DRender::QCamera *mMainCamera = nullptr;
    Qt3DRender::QParameter *mFarPlaneParameter = nullptr;
    Qt3DRender::QParameter *mNearPlaneParameter = nullptr;
    Qt3DRender::QParameter *mMainCameraInvViewMatrixParameter = nullptr;
    Qt3DRender::QParameter *mMainCameraInvProjMatrixParameter = nullptr;

    Qt3DRender::QParameter *mSsaoKernelParameter = nullptr;
    Qt3DRender::QTexture2D *mSsaoNoiseTexture = nullptr;
    Qt3DRender::QParameter *mSsaoNoiseTextureParameter = nullptr;

    Qt3DRender::QParameter *mShadingFactorParameter = nullptr;
    Qt3DRender::QParameter *mDistanceAttenuationFactorParameter = nullptr;
    Qt3DRender::QParameter *mRadiusParameter = nullptr;
};

class QgsSsaoBlurEntity : public Qt3DCore::QEntity
{
  public:
    //! Constructor
    QgsSsaoBlurEntity( QgsShadowRenderingFrameGraph *frameGraph, QNode *parent = nullptr );
  private:
    Qt3DRender::QMaterial *mMaterial = nullptr;
    Qt3DRender::QEffect *mEffect = nullptr;
    Qt3DRender::QParameter *mColorTextureParameter = nullptr;
    Qt3DRender::QParameter *mDepthTextureParameter = nullptr;
    Qt3DRender::QParameter *mShadowMapParameter = nullptr;
    Qt3DRender::QCamera *mMainCamera = nullptr;
    Qt3DRender::QParameter *mFarPlaneParameter = nullptr;
    Qt3DRender::QParameter *mNearPlaneParameter = nullptr;
    Qt3DRender::QParameter *mMainCameraInvViewMatrixParameter = nullptr;
    Qt3DRender::QParameter *mMainCameraInvProjMatrixParameter = nullptr;

    Qt3DRender::QParameter *mSsaoFactorTextureParameter = nullptr;
};

#endif // QGSSSAORENDERENTITY_H
