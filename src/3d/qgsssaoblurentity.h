/***************************************************************************
  qgsssaoblurentity.h
  --------------------------------------
  Date                 : June 2022
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

#ifndef QGSSSAOBLURENTITY_H
#define QGSSSAOBLURENTITY_H

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QCamera>

class QgsShadowRenderingFrameGraph;

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief An entity that is responsible for blurring the SSAO factor texture.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.28
 */
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

#endif // QGSSSAOBLURENTITY_H
