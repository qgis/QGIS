/***************************************************************************
  qgsambientocclusionrenderentity.cpp
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

#include "qgsambientocclusionrenderentity.h"

#include <random>

#include <Qt3DRender/QParameter>

QgsAmbientOcclusionRenderEntity::QgsAmbientOcclusionRenderEntity( Qt3DRender::QTexture2D *depthTexture, Qt3DRender::QCamera *camera, QNode *parent )
  : QgsRenderPassQuad( parent )
{
  mDepthTextureParameter = new Qt3DRender::QParameter( QStringLiteral( "depthTexture" ), depthTexture );
  mMaterial->addParameter( mDepthTextureParameter );

  mFarPlaneParameter = new Qt3DRender::QParameter( QStringLiteral( "farPlane" ), camera->farPlane() );
  mMaterial->addParameter( mFarPlaneParameter );
  connect( camera, &Qt3DRender::QCamera::farPlaneChanged, mFarPlaneParameter, [&]( float farPlane )
  {
    mFarPlaneParameter->setValue( farPlane );
  } );
  mNearPlaneParameter = new Qt3DRender::QParameter( QStringLiteral( "nearPlane" ), camera->nearPlane() );
  mMaterial->addParameter( mNearPlaneParameter );
  connect( camera, &Qt3DRender::QCamera::nearPlaneChanged, mNearPlaneParameter, [&]( float nearPlane )
  {
    mNearPlaneParameter->setValue( nearPlane );
  } );
  mProjMatrixParameter = new Qt3DRender::QParameter( QStringLiteral( "origProjMatrix" ), camera->projectionMatrix() );
  mMaterial->addParameter( mProjMatrixParameter );
  connect( camera, &Qt3DRender::QCamera::projectionMatrixChanged, mProjMatrixParameter, [&]( const QMatrix4x4 & projectionMatrix )
  {
    mProjMatrixParameter->setValue( projectionMatrix );
  } );
  mAspectRatioParameter = new Qt3DRender::QParameter( QStringLiteral( "uAspectRatio" ), camera->aspectRatio() );
  mMaterial->addParameter( mAspectRatioParameter );
  connect( camera, &Qt3DRender::QCamera::aspectRatioChanged, mAspectRatioParameter, [&]( float ratio )
  {
    mAspectRatioParameter->setValue( ratio );
  } );
  mTanHalfFovParameter = new Qt3DRender::QParameter( QStringLiteral( "uTanHalfFov" ), tan( camera->fieldOfView() / 2 * M_PI / 180 ) );
  mMaterial->addParameter( mTanHalfFovParameter );
  connect( camera, &Qt3DRender::QCamera::fieldOfViewChanged, mTanHalfFovParameter, [&]( float fov )
  {
    mTanHalfFovParameter->setValue( tan( fov / 2 * M_PI / 180 ) );
  } );

  QVariantList ssaoKernelValues;

  std::uniform_real_distribution<float> randomFloats( 0.0, 1.0 ); // random floats between [0.0, 1.0]
  std::default_random_engine generator;
  unsigned int kernelSize = 64;
  for ( unsigned int i = 0; i < kernelSize; ++i )
  {
    QVector3D sample(
      randomFloats( generator ) * 2.0 - 1.0,
      randomFloats( generator ) * 2.0 - 1.0,
      randomFloats( generator ) * 2.0 - 1.0
    );
    sample.normalize();
    float scale = i / kernelSize;
    scale = 0.1 + 0.9 * scale * scale;
    sample *= scale;
    ssaoKernelValues.push_back( sample );
  }

  // 4x4 array of random rotation vectors
  QVariantList ssaoNoise;
  for ( unsigned int i = 0; i < 16; ++i )
  {
    QVector3D sample(
      randomFloats( generator ),
      randomFloats( generator ),
      0.0
    );
    ssaoNoise.push_back( sample );
  }
  mAmbientOcclusionKernelParameter = new Qt3DRender::QParameter( QStringLiteral( "ssaoKernel[0]" ), ssaoKernelValues );
  mMaterial->addParameter( mAmbientOcclusionKernelParameter );

  Qt3DRender::QParameter *noiseParameter = new Qt3DRender::QParameter( QStringLiteral( "ssaoNoise[0]" ), ssaoNoise );
  mMaterial->addParameter( noiseParameter );

  mIntensityParameter = new Qt3DRender::QParameter( QStringLiteral( "intensity" ), 0.5f );
  mMaterial->addParameter( mIntensityParameter );

  mRadiusParameter = new Qt3DRender::QParameter( QStringLiteral( "radius" ), 25.0f );
  mMaterial->addParameter( mRadiusParameter );

  mThresholdParameter = new Qt3DRender::QParameter( QStringLiteral( "threshold" ), 0.5f );
  mMaterial->addParameter( mThresholdParameter );

  const QString vertexShaderPath = QStringLiteral( "qrc:/shaders/ssao_factor_render.vert" );
  const QString fragmentShaderPath = QStringLiteral( "qrc:/shaders/ssao_factor_render.frag" );

  mShader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( vertexShaderPath ) ) );
  mShader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( fragmentShaderPath ) ) );
}

void QgsAmbientOcclusionRenderEntity::setIntensity( float intensity )
{
  mIntensityParameter->setValue( intensity );
}

void QgsAmbientOcclusionRenderEntity::setRadius( float radius )
{
  mRadiusParameter->setValue( radius );
}

void QgsAmbientOcclusionRenderEntity::setThreshold( float threshold )
{
  mThresholdParameter->setValue( threshold );
}
