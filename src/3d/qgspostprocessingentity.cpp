/***************************************************************************
  qgspostprocessingentity.cpp
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

#include "qgspostprocessingentity.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometry>

typedef Qt3DRender::QAttribute Qt3DQAttribute;
typedef Qt3DRender::QBuffer Qt3DQBuffer;
typedef Qt3DRender::QGeometry Qt3DQGeometry;
#else
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>

typedef Qt3DCore::QAttribute Qt3DQAttribute;
typedef Qt3DCore::QBuffer Qt3DQBuffer;
typedef Qt3DCore::QGeometry Qt3DQGeometry;
#endif

#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QDepthTest>
#include <QUrl>

#include "qgsshadowrenderingframegraph.h"

QgsPostprocessingEntity::QgsPostprocessingEntity( QgsShadowRenderingFrameGraph *frameGraph, QNode *parent )
  : QgsRenderPassQuad( parent )
  , mFrameGraph( frameGraph )
{
  mColorTextureParameter = new Qt3DRender::QParameter( QStringLiteral( "colorTexture" ), frameGraph->forwardRenderColorTexture() );
  mDepthTextureParameter = new Qt3DRender::QParameter( QStringLiteral( "depthTexture" ), frameGraph->forwardRenderDepthTexture() );
  mShadowMapParameter = new Qt3DRender::QParameter( QStringLiteral( "shadowTexture" ), frameGraph->shadowMapTexture() );
  mAmbientOcclusionTextureParameter = new Qt3DRender::QParameter( QStringLiteral( "ssaoTexture" ), frameGraph->blurredAmbientOcclusionFactorMap() );
  mMaterial->addParameter( mColorTextureParameter );
  mMaterial->addParameter( mDepthTextureParameter );
  mMaterial->addParameter( mShadowMapParameter );
  mMaterial->addParameter( mAmbientOcclusionTextureParameter );

  mMainCamera = frameGraph->mainCamera();
  mLightCamera = frameGraph->lightCamera();

  mFarPlaneParameter = new Qt3DRender::QParameter( QStringLiteral( "farPlane" ), mMainCamera->farPlane() );
  mMaterial->addParameter( mFarPlaneParameter );
  connect( mMainCamera, &Qt3DRender::QCamera::farPlaneChanged, mFarPlaneParameter, [&]( float farPlane )
  {
    mFarPlaneParameter->setValue( farPlane );
  } );
  mNearPlaneParameter = new Qt3DRender::QParameter( QStringLiteral( "nearPlane" ), mMainCamera->nearPlane() );
  mMaterial->addParameter( mNearPlaneParameter );
  connect( mMainCamera, &Qt3DRender::QCamera::nearPlaneChanged, mNearPlaneParameter, [&]( float nearPlane )
  {
    mNearPlaneParameter->setValue( nearPlane );
  } );

  mLightFarPlaneParameter = new Qt3DRender::QParameter( QStringLiteral( "lightFarPlane" ), mLightCamera->farPlane() );
  mMaterial->addParameter( mLightFarPlaneParameter );
  connect( mLightCamera, &Qt3DRender::QCamera::farPlaneChanged, mLightFarPlaneParameter, [&]( float farPlane )
  {
    mLightFarPlaneParameter->setValue( farPlane );
  } );
  mLightNearPlaneParameter = new Qt3DRender::QParameter( QStringLiteral( "lightNearPlane" ), mLightCamera->nearPlane() );
  mMaterial->addParameter( mLightNearPlaneParameter );
  connect( mLightCamera, &Qt3DRender::QCamera::nearPlaneChanged, mLightNearPlaneParameter, [&]( float nearPlane )
  {
    mLightNearPlaneParameter->setValue( nearPlane );
  } );

  mMainCameraInvViewMatrixParameter = new Qt3DRender::QParameter( QStringLiteral( "invertedCameraView" ), mMainCamera->viewMatrix().inverted() );
  mMaterial->addParameter( mMainCameraInvViewMatrixParameter );
  mMainCameraInvProjMatrixParameter = new Qt3DRender::QParameter( QStringLiteral( "invertedCameraProj" ), mMainCamera->projectionMatrix().inverted() );
  mMaterial->addParameter( mMainCameraInvProjMatrixParameter );
  connect( mMainCamera, &Qt3DRender::QCamera::projectionMatrixChanged, mMainCameraInvProjMatrixParameter, [&]( const QMatrix4x4 & projectionMatrix )
  {
    mMainCameraInvProjMatrixParameter->setValue( projectionMatrix.inverted() );
  } );
  connect( mMainCamera, &Qt3DRender::QCamera::viewMatrixChanged, mMainCameraInvViewMatrixParameter, [&]()
  {
    mMainCameraInvViewMatrixParameter->setValue( mMainCamera->viewMatrix().inverted() );
  } );

  mShadowMinX = new Qt3DRender::QParameter( QStringLiteral( "shadowMinX" ), QVariant::fromValue( 0.0f ) );
  mShadowMaxX = new Qt3DRender::QParameter( QStringLiteral( "shadowMaxX" ), QVariant::fromValue( 0.0f ) );
  mShadowMinZ = new Qt3DRender::QParameter( QStringLiteral( "shadowMinZ" ), QVariant::fromValue( 0.0f ) );
  mShadowMaxZ = new Qt3DRender::QParameter( QStringLiteral( "shadowMaxZ" ), QVariant::fromValue( 0.0f ) );
  mMaterial->addParameter( mShadowMinX );
  mMaterial->addParameter( mShadowMaxX );
  mMaterial->addParameter( mShadowMinZ );
  mMaterial->addParameter( mShadowMaxZ );

  mRenderShadowsParameter = new Qt3DRender::QParameter( QStringLiteral( "renderShadows" ), QVariant::fromValue( 0 ) );
  mMaterial->addParameter( mRenderShadowsParameter );

  mShadowBiasParameter = new Qt3DRender::QParameter( QStringLiteral( "shadowBias" ), QVariant::fromValue( 0.00001f ) );
  mMaterial->addParameter( mShadowBiasParameter );

  mEyeDomeLightingEnabledParameter = new Qt3DRender::QParameter( QStringLiteral( "edlEnabled" ), QVariant::fromValue( 0 ) );
  mEyeDomeLightingStrengthParameter = new Qt3DRender::QParameter( QStringLiteral( "edlStrength" ), QVariant::fromValue( 1000.0f ) );
  mEyeDomeLightingDistanceParameter = new Qt3DRender::QParameter( QStringLiteral( "edlDistance" ), QVariant::fromValue( 2.0f ) );
  mMaterial->addParameter( mEyeDomeLightingEnabledParameter );
  mMaterial->addParameter( mEyeDomeLightingStrengthParameter );
  mMaterial->addParameter( mEyeDomeLightingDistanceParameter );

  mAmbientOcclusionEnabledParameter = new Qt3DRender::QParameter( QStringLiteral( "ssaoEnabled" ), QVariant::fromValue( 0 ) );
  mMaterial->addParameter( mAmbientOcclusionEnabledParameter );

  mLightPosition = new Qt3DRender::QParameter( QStringLiteral( "lightPosition" ), QVariant::fromValue( QVector3D() ) );
  mLightDirection = new Qt3DRender::QParameter( QStringLiteral( "lightDirection" ), QVariant::fromValue( QVector3D() ) );
  mMaterial->addParameter( mLightPosition );
  mMaterial->addParameter( mLightDirection );

  const QString vertexShaderPath = QStringLiteral( "qrc:/shaders/postprocess.vert" );
  const QString fragmentShaderPath = QStringLiteral( "qrc:/shaders/postprocess.frag" );

  mShader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( vertexShaderPath ) ) );
  mShader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( fragmentShaderPath ) ) );
}

void QgsPostprocessingEntity::setupShadowRenderingExtent( float minX, float maxX, float minZ, float maxZ )
{
  mShadowMinX->setValue( minX );
  mShadowMaxX->setValue( maxX );
  mShadowMinZ->setValue( minZ );
  mShadowMaxZ->setValue( maxZ );
}

void QgsPostprocessingEntity::setupDirectionalLight( QVector3D position, QVector3D direction )
{
  mLightPosition->setValue( QVariant::fromValue( position ) );
  mLightDirection->setValue( QVariant::fromValue( direction.normalized() ) );
}

void QgsPostprocessingEntity::setShadowRenderingEnabled( bool enabled )
{
  mRenderShadowsParameter->setValue( QVariant::fromValue( enabled ? 1 : 0 ) );
}

void QgsPostprocessingEntity::setShadowBias( float shadowBias )
{
  mShadowBiasParameter->setValue( QVariant::fromValue( shadowBias ) );
}

void QgsPostprocessingEntity::setEyeDomeLightingEnabled( bool enabled )
{
  mEyeDomeLightingEnabledParameter->setValue( QVariant::fromValue( enabled ? 1 : 0 ) );
}

void QgsPostprocessingEntity::setEyeDomeLightingStrength( double strength )
{
  mEyeDomeLightingStrengthParameter->setValue( QVariant::fromValue( strength ) );
}

void QgsPostprocessingEntity::setEyeDomeLightingDistance( int distance )
{
  mEyeDomeLightingDistanceParameter->setValue( QVariant::fromValue( distance ) );
}

void QgsPostprocessingEntity::setAmbientOcclusionEnabled( bool enabled )
{
  mAmbientOcclusionEnabledParameter->setValue( enabled );
}
