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

#include "qgs3dutils.h"
#include "qgsambientocclusionrenderview.h"
#include "qgsdirectionallightsettings.h"
#include "qgsforwardrenderview.h"
#include "qgsframegraph.h"
#include "qgsshadowrenderview.h"

#include <QUrl>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>

#include "moc_qgspostprocessingentity.cpp"

QgsPostprocessingEntity::QgsPostprocessingEntity( QgsFrameGraph *frameGraph, Qt3DRender::QLayer *layer, QNode *parent )
  : QgsRenderPassQuad( layer, parent )
{
  QgsShadowRenderView &shadowRenderView = frameGraph->shadowRenderView();
  QgsForwardRenderView &forwardRenderView = frameGraph->forwardRenderView();
  QgsAmbientOcclusionRenderView &aoRenderView = frameGraph->ambientOcclusionRenderView();

  mColorTextureParameter = new Qt3DRender::QParameter( u"colorTexture"_s, forwardRenderView.colorTexture() );
  mDepthTextureParameter = new Qt3DRender::QParameter( u"depthTexture"_s, forwardRenderView.depthTexture() );
  mShadowMapParameter = new Qt3DRender::QParameter( u"shadowTexture"_s, shadowRenderView.mapTexture() );
  mAmbientOcclusionTextureParameter = new Qt3DRender::QParameter( u"ssaoTexture"_s, aoRenderView.blurredFactorMapTexture() );
  mMaterial->addParameter( mColorTextureParameter );
  mMaterial->addParameter( mDepthTextureParameter );
  mMaterial->addParameter( mShadowMapParameter );
  mMaterial->addParameter( mAmbientOcclusionTextureParameter );

  mMainCamera = frameGraph->mainCamera();
  mLightCamera = shadowRenderView.lightCamera();

  mFarPlaneParameter = new Qt3DRender::QParameter( u"farPlane"_s, mMainCamera->farPlane() );
  mMaterial->addParameter( mFarPlaneParameter );
  connect( mMainCamera, &Qt3DRender::QCamera::farPlaneChanged, mFarPlaneParameter, [&]( float farPlane ) {
    mFarPlaneParameter->setValue( farPlane );
  } );
  mNearPlaneParameter = new Qt3DRender::QParameter( u"nearPlane"_s, mMainCamera->nearPlane() );
  mMaterial->addParameter( mNearPlaneParameter );
  connect( mMainCamera, &Qt3DRender::QCamera::nearPlaneChanged, mNearPlaneParameter, [&]( float nearPlane ) {
    mNearPlaneParameter->setValue( nearPlane );
  } );

  mLightFarPlaneParameter = new Qt3DRender::QParameter( u"lightFarPlane"_s, mLightCamera->farPlane() );
  mMaterial->addParameter( mLightFarPlaneParameter );
  connect( mLightCamera, &Qt3DRender::QCamera::farPlaneChanged, mLightFarPlaneParameter, [&]( float farPlane ) {
    mLightFarPlaneParameter->setValue( farPlane );
  } );
  mLightNearPlaneParameter = new Qt3DRender::QParameter( u"lightNearPlane"_s, mLightCamera->nearPlane() );
  mMaterial->addParameter( mLightNearPlaneParameter );
  connect( mLightCamera, &Qt3DRender::QCamera::nearPlaneChanged, mLightNearPlaneParameter, [&]( float nearPlane ) {
    mLightNearPlaneParameter->setValue( nearPlane );
  } );

  mMainCameraInvViewMatrixParameter = new Qt3DRender::QParameter( u"invertedCameraView"_s, mMainCamera->viewMatrix().inverted() );
  mMaterial->addParameter( mMainCameraInvViewMatrixParameter );
  mMainCameraInvProjMatrixParameter = new Qt3DRender::QParameter( u"invertedCameraProj"_s, mMainCamera->projectionMatrix().inverted() );
  mMaterial->addParameter( mMainCameraInvProjMatrixParameter );
  connect( mMainCamera, &Qt3DRender::QCamera::projectionMatrixChanged, mMainCameraInvProjMatrixParameter, [&]( const QMatrix4x4 &projectionMatrix ) {
    mMainCameraInvProjMatrixParameter->setValue( projectionMatrix.inverted() );
  } );
  connect( mMainCamera, &Qt3DRender::QCamera::viewMatrixChanged, mMainCameraInvViewMatrixParameter, [&]() {
    mMainCameraInvViewMatrixParameter->setValue( mMainCamera->viewMatrix().inverted() );
  } );

  mShadowMinX = new Qt3DRender::QParameter( u"shadowMinX"_s, QVariant::fromValue( 0.0f ) );
  mShadowMaxX = new Qt3DRender::QParameter( u"shadowMaxX"_s, QVariant::fromValue( 0.0f ) );
  mShadowMinY = new Qt3DRender::QParameter( u"shadowMinY"_s, QVariant::fromValue( 0.0f ) );
  mShadowMaxY = new Qt3DRender::QParameter( u"shadowMaxY"_s, QVariant::fromValue( 0.0f ) );
  mMaterial->addParameter( mShadowMinX );
  mMaterial->addParameter( mShadowMaxX );
  mMaterial->addParameter( mShadowMinY );
  mMaterial->addParameter( mShadowMaxY );

  mRenderShadowsParameter = new Qt3DRender::QParameter( u"renderShadows"_s, QVariant::fromValue( 0 ) );
  mMaterial->addParameter( mRenderShadowsParameter );

  mShadowBiasParameter = new Qt3DRender::QParameter( u"shadowBias"_s, QVariant::fromValue( 0.00001f ) );
  mMaterial->addParameter( mShadowBiasParameter );

  mEyeDomeLightingEnabledParameter = new Qt3DRender::QParameter( u"edlEnabled"_s, QVariant::fromValue( 0 ) );
  mEyeDomeLightingStrengthParameter = new Qt3DRender::QParameter( u"edlStrength"_s, QVariant::fromValue( 1000.0f ) );
  mEyeDomeLightingDistanceParameter = new Qt3DRender::QParameter( u"edlDistance"_s, QVariant::fromValue( 2.0f ) );
  mMaterial->addParameter( mEyeDomeLightingEnabledParameter );
  mMaterial->addParameter( mEyeDomeLightingStrengthParameter );
  mMaterial->addParameter( mEyeDomeLightingDistanceParameter );

  mAmbientOcclusionEnabledParameter = new Qt3DRender::QParameter( u"ssaoEnabled"_s, QVariant::fromValue( 0 ) );
  mMaterial->addParameter( mAmbientOcclusionEnabledParameter );

  mLightPosition = new Qt3DRender::QParameter( u"lightPosition"_s, QVariant::fromValue( QVector3D() ) );
  mLightDirection = new Qt3DRender::QParameter( u"lightDirection"_s, QVariant::fromValue( QVector3D() ) );
  mMaterial->addParameter( mLightPosition );
  mMaterial->addParameter( mLightDirection );

  const QString vertexShaderPath = u"qrc:/shaders/postprocess.vert"_s;
  const QString fragmentShaderPath = u"qrc:/shaders/postprocess.frag"_s;

  mShader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( vertexShaderPath ) ) );
  mShader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( fragmentShaderPath ) ) );
}

void QgsPostprocessingEntity::setupShadowRenderingExtent( float minX, float maxX, float minY, float maxY )
{
  mShadowMinX->setValue( minX );
  mShadowMaxX->setValue( maxX );
  mShadowMinY->setValue( minY );
  mShadowMaxY->setValue( maxY );
}

void QgsPostprocessingEntity::setupDirectionalLight( QVector3D position, QVector3D direction )
{
  mLightPosition->setValue( QVariant::fromValue( position ) );
  mLightDirection->setValue( QVariant::fromValue( direction.normalized() ) );
}

void QgsPostprocessingEntity::updateShadowSettings( const QgsDirectionalLightSettings &light, float maximumShadowRenderingDistance )
{
  float minX, maxX, minY, maxY, minZ, maxZ;
  QVector3D lookingAt = mMainCamera->viewCenter();
  const float d = 2 * ( mMainCamera->position() - mMainCamera->viewCenter() ).length();

  const QVector3D lightDirection = light.direction().toVector3D().normalized();
  Qgs3DUtils::calculateViewExtent( mMainCamera, maximumShadowRenderingDistance, lookingAt.z(), minX, maxX, minY, maxY, minZ, maxZ );

  lookingAt = QVector3D( 0.5f * ( minX + maxX ), 0.5f * ( minY + maxY ), mMainCamera->viewCenter().z() );
  const QVector3D lightPosition = lookingAt + QVector3D( 0.0f, 0.0f, d );
  mLightCamera->setPosition( lightPosition );
  mLightCamera->setViewCenter( lookingAt );
  mLightCamera->setUpVector( QVector3D( 0.0f, 1.0f, 0.0f ) );
  mLightCamera->rotateAboutViewCenter( QQuaternion::rotationTo( QVector3D( 0.0f, 0.0f, -1.0f ), lightDirection ) );

  mLightCamera->setProjectionType( Qt3DRender::QCameraLens::ProjectionType::OrthographicProjection );
  mLightCamera->lens()->setOrthographicProjection(
    -0.7f * ( maxX - minX ), 0.7f * ( maxX - minX ),
    -0.7f * ( maxY - minY ), 0.7f * ( maxY - minY ),
    1.0f, 2 * ( lookingAt - lightPosition ).length()
  );

  setupShadowRenderingExtent( minX, maxX, minY, maxY );
  setupDirectionalLight( lightPosition, lightDirection );
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
