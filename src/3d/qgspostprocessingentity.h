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

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QCamera>

class QgsShadowRenderingFrameGraph;

class QgsPostprocessingEntity : public Qt3DCore::QEntity
{
  public:
    QgsPostprocessingEntity( QgsShadowRenderingFrameGraph *frameGraph, const QString &vertexShaderPath, const QString &fragmentShaderPath, QNode *parent = nullptr );
    void setupShadowRenderingExtent( float minX, float maxX, float minZ, float maxZ );
    void setupDirectionalLight( QVector3D position, QVector3D direction );
    void setShadowRenderingEnabled( bool enabled );
  private:
    Qt3DRender::QMaterial *mMaterial = nullptr;
    Qt3DRender::QEffect *mEffect = nullptr;
    Qt3DRender::QParameter *mColorTextureParameter = nullptr;
    Qt3DRender::QParameter *mDepthTextureParameter = nullptr;
    Qt3DRender::QParameter *mShadowMapParameter = nullptr;
    Qt3DRender::QCamera *mMainCamera = nullptr;
    Qt3DRender::QParameter *mFarPlaneParameter = nullptr;
    Qt3DRender::QParameter *mNearPlaneParameter = nullptr;
    Qt3DRender::QParameter *mMainCameraViewMatrixParameter = nullptr;
    Qt3DRender::QParameter *mMainCameraProjMatrixParameter = nullptr;

    Qt3DRender::QCamera *mLightCamera = nullptr;
    Qt3DRender::QParameter *mLightFarPlaneParameter = nullptr;
    Qt3DRender::QParameter *mLightNearPlaneParameter = nullptr;

    Qt3DRender::QParameter *mLightPosition = nullptr;
    Qt3DRender::QParameter *mLightDirection = nullptr;

    Qt3DRender::QParameter *mShadowMinX = nullptr;
    Qt3DRender::QParameter *mShadowMaxX = nullptr;
    Qt3DRender::QParameter *mShadowMinZ = nullptr;
    Qt3DRender::QParameter *mShadowMaxZ = nullptr;

    Qt3DRender::QParameter *mRenderShadowsParameter = nullptr;
};

#endif // QGSPOSTPROCESSINGENTITY_H
