/***************************************************************************
  qgsambientocclusionrenderentity.h
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

#ifndef QGSAMBIENTOCCLUSIONRENDERENTITY_H
#define QGSAMBIENTOCCLUSIONRENDERENTITY_H

#include "qgsrenderpassquad.h"

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief An entity that is responsible for producing an ambient occlusion factor map.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.28
 */
class QgsAmbientOcclusionRenderEntity : public QgsRenderPassQuad
{
  public:
    //! Constructor
    QgsAmbientOcclusionRenderEntity( Qt3DRender::QTexture2D *depthTexture, Qt3DRender::QCamera *camera, QNode *parent = nullptr );

    //! Sets the shading factor for the ambient occlusion effect
    void setShadingFactor( float factor );

    //! Sets the distance attenuation factor for the ambient occlusion effect
    void setDistanceAttenuationFactor( float factor );

    //! Sets the radius parameter for the ambient occlusion effect
    void setRadiusParameter( float radius );

  private:
    Qt3DRender::QParameter *mFarPlaneParameter = nullptr;
    Qt3DRender::QParameter *mNearPlaneParameter = nullptr;

    Qt3DRender::QParameter *mDepthTextureParameter = nullptr;

    Qt3DRender::QParameter *mAmbientOcclusionKernelParameter = nullptr;

    Qt3DRender::QParameter *mShadingFactorParameter = nullptr;
    Qt3DRender::QParameter *mDistanceAttenuationFactorParameter = nullptr;
    Qt3DRender::QParameter *mRadiusParameter = nullptr;
};

#endif // QGSAMBIENTOCCLUSIONRENDERENTITY_H
