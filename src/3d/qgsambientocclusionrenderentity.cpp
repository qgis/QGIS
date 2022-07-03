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

  QVariantList ssaoKernelValues;

  std::uniform_real_distribution<float> randomFloats( 0.0, 1.0 ); // random floats between [0.0, 1.0]
  std::default_random_engine generator;
  for ( unsigned int i = 0; i < 64; ++i )
  {
    QVector3D sample(
      randomFloats( generator ) * 2.0 - 1.0,
      randomFloats( generator ) * 2.0 - 1.0,
      randomFloats( generator ) * 2.0 - 1.0
    );
    sample.normalize();
    sample *= randomFloats( generator );
    ssaoKernelValues.push_back( sample );
  }

  mAmbientOcclusionKernelParameter = new Qt3DRender::QParameter( QStringLiteral( "ssaoKernel[0]" ), ssaoKernelValues );
  mMaterial->addParameter( mAmbientOcclusionKernelParameter );

  mShadingFactorParameter = new Qt3DRender::QParameter( QStringLiteral( "shadingFactor" ), 50.0f );
  mMaterial->addParameter( mShadingFactorParameter );

  mDistanceAttenuationFactorParameter = new Qt3DRender::QParameter( QStringLiteral( "distanceAttenuationFactor" ), 500.0f );
  mMaterial->addParameter( mDistanceAttenuationFactorParameter );

  mRadiusParameter = new Qt3DRender::QParameter( QStringLiteral( "radiusParameter" ), 0.05f );
  mMaterial->addParameter( mRadiusParameter );

  const QString vertexShaderPath = QStringLiteral( "qrc:/shaders/ssao_factor_render.vert" );
  const QString fragmentShaderPath = QStringLiteral( "qrc:/shaders/ssao_factor_render.frag" );

  mShader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( vertexShaderPath ) ) );
  mShader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( fragmentShaderPath ) ) );
}

void QgsAmbientOcclusionRenderEntity::setShadingFactor( float factor )
{
  mShadingFactorParameter->setValue( factor );
}

void QgsAmbientOcclusionRenderEntity::setDistanceAttenuationFactor( float factor )
{
  mDistanceAttenuationFactorParameter->setValue( factor );
}

void QgsAmbientOcclusionRenderEntity::setRadiusParameter( float radius )
{
  mRadiusParameter->setValue( radius );
}
