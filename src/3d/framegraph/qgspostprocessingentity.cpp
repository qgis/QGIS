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

#include <QString>
#include <QUrl>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>

#include "moc_qgspostprocessingentity.cpp"

using namespace Qt::StringLiterals;

QgsPostprocessingEntity::QgsPostprocessingEntity( QgsFrameGraph *frameGraph, Qt3DRender::QLayer *layer, QNode *parent )
  : QgsRenderPassQuad( layer, parent )
{
  QgsShadowRenderView &shadowRenderView = frameGraph->shadowRenderView();
  QgsForwardRenderView &forwardRenderView = frameGraph->forwardRenderView();
  QgsAmbientOcclusionRenderView &aoRenderView = frameGraph->ambientOcclusionRenderView();

  mColorTextureParameter = new Qt3DRender::QParameter( u"colorTexture"_s, forwardRenderView.colorTexture() );
  mDepthTextureParameter = new Qt3DRender::QParameter( u"depthTexture"_s, forwardRenderView.depthTexture() );
  mShadowMapParameter = new Qt3DRender::QParameter( u"shadowTexture"_s, shadowRenderView.mapTextureArray() );
  mAmbientOcclusionTextureParameter = new Qt3DRender::QParameter( u"ssaoTexture"_s, aoRenderView.blurredFactorMapTexture() );
  mMaterial->addParameter( mColorTextureParameter );
  mMaterial->addParameter( mDepthTextureParameter );
  mMaterial->addParameter( mShadowMapParameter );
  mMaterial->addParameter( mAmbientOcclusionTextureParameter );

  mMainCamera = frameGraph->mainCamera();

  for ( int i = 0; i < Qgs3D::NUM_SHADOW_CASCADES; ++i )
  {
    mLightCameras[i] = shadowRenderView.lightCamera( i );
  }

  // a [0] suffix for a QParameter name maps the parameter to a GLSL array.
  // We must take care that the parameter value is always a variant list of equal length!
  const QVariantList csmMatrices = QVariantList( Qgs3D::NUM_SHADOW_CASCADES, QVariant::fromValue( QMatrix4x4() ) );
  mCsmMatricesParameter = new Qt3DRender::QParameter( QString( "csmMatrices[0]" ), csmMatrices );
  mMaterial->addParameter( mCsmMatricesParameter );

  mFarPlaneParameter = new Qt3DRender::QParameter( u"farPlane"_s, mMainCamera->farPlane() );
  mMaterial->addParameter( mFarPlaneParameter );
  connect( mMainCamera, &Qt3DRender::QCamera::farPlaneChanged, mFarPlaneParameter, [&]( float farPlane ) { mFarPlaneParameter->setValue( farPlane ); } );
  mNearPlaneParameter = new Qt3DRender::QParameter( u"nearPlane"_s, mMainCamera->nearPlane() );
  mMaterial->addParameter( mNearPlaneParameter );
  connect( mMainCamera, &Qt3DRender::QCamera::nearPlaneChanged, mNearPlaneParameter, [&]( float nearPlane ) { mNearPlaneParameter->setValue( nearPlane ); } );

  mMainCameraInvViewMatrixParameter = new Qt3DRender::QParameter( u"invertedCameraView"_s, mMainCamera->viewMatrix().inverted() );
  mMaterial->addParameter( mMainCameraInvViewMatrixParameter );
  mMainCameraInvProjMatrixParameter = new Qt3DRender::QParameter( u"invertedCameraProj"_s, mMainCamera->projectionMatrix().inverted() );
  mMaterial->addParameter( mMainCameraInvProjMatrixParameter );
  connect( mMainCamera, &Qt3DRender::QCamera::projectionMatrixChanged, mMainCameraInvProjMatrixParameter, [&]( const QMatrix4x4 &projectionMatrix ) {
    mMainCameraInvProjMatrixParameter->setValue( projectionMatrix.inverted() );
  } );
  connect( mMainCamera, &Qt3DRender::QCamera::viewMatrixChanged, mMainCameraInvViewMatrixParameter, [&]() { mMainCameraInvViewMatrixParameter->setValue( mMainCamera->viewMatrix().inverted() ); } );

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

  const QString vertexShaderPath = u"qrc:/shaders/postprocess.vert"_s;
  const QString fragmentShaderPath = u"qrc:/shaders/postprocess.frag"_s;

  mShader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( vertexShaderPath ) ) );

  const QByteArray fragmentShaderCode = Qt3DRender::QShaderProgram::loadSource( QUrl( fragmentShaderPath ) );
  const QByteArray finalFragmentShaderCode = Qgs3DUtils::addDefinesToShaderCode( fragmentShaderCode, QStringList( { "ENABLE_EFFECTS" } ) );
  mShader->setFragmentShaderCode( finalFragmentShaderCode );
}

void QgsPostprocessingEntity::updateShadowSettings( const QgsDirectionalLightSettings &light, float maximumShadowRenderingDistance )
{
  // We are using "Cascading Shadow Maps" technique.
  // Reading/watching which was useful during development:
  // https://learnopengl.com/Guest-Articles/2021/CSM
  // https://developer.download.nvidia.com/SDK/10.5/opengl/src/cascaded_shadow_maps/doc/cascaded_shadow_maps.pdf
  // https://www.youtube.com/watch?v=Jhopq2lkzMQ
  // https://www.youtube.com/watch?v=qbDrqARX07o
  // https://web.archive.org/web/20170710150304/https://cesiumjs.org/presentations/ShadowsAndCesiumImplementation.pdf

  const QVector3D lightDirection = light.direction().toVector3D().normalized();
  const QVector3D up = Qgs3DUtils::calculateDirectionalLightUpVector( lightDirection );

  const float mainCameraNearPlane = mMainCamera->nearPlane();
  // cap the far plane to the shadow rendering distance so we don't waste shadow resolution
  const float mainCameraFarPlane = std::min( mMainCamera->farPlane(), maximumShadowRenderingDistance );

  // "Practical Split Scheme" for cascading shadow maps.
  // using a quite large lambda to account for typical near/far plane distances seen in
  // QGIS 3d maps (0.5 - ~2500)
  // We match Cesium's lambda -- see https://web.archive.org/web/20170710150304/https://cesiumjs.org/presentations/ShadowsAndCesiumImplementation.pdf (slide 38)
  constexpr float PRACTICAL_SPLIT_SCHEME_LAMBDA = 0.9f;
  const std::vector<float> cascadeSplits = Qgs3DUtils::calculateCascadeSplits( Qgs3D::NUM_SHADOW_CASCADES, mainCameraNearPlane, mainCameraFarPlane, PRACTICAL_SPLIT_SCHEME_LAMBDA );

  const QMatrix4x4 invertedCameraView = mMainCamera->viewMatrix().inverted();
  const float cameraFov = mMainCamera->fieldOfView();
  const float cameraAspect = mMainCamera->aspectRatio();

  QVariantList csmMatrices( Qgs3D::NUM_SHADOW_CASCADES, QVariant() );
  for ( int i = 0; i < Qgs3D::NUM_SHADOW_CASCADES; ++i )
  {
    const float zNear = cascadeSplits[i];
    const float zFar = cascadeSplits[i + 1];

    // calculate the 8 corners of the camera frustum slice in world space
    QVector3D worldFrustumCorners[8];
    QVector3D worldFrustrumCenter;
    Qgs3DUtils::calculateFrustumSliceCorners( zNear, zFar, cameraFov, cameraAspect, invertedCameraView, worldFrustumCorners, worldFrustrumCenter );

    // create the light view matrix
    QMatrix4x4 lightView;
    const QVector3D lightPos = worldFrustrumCenter - lightDirection;
    lightView.lookAt( lightPos, worldFrustrumCenter, up );

    // apply to the specific light camera
    mLightCameras[i]->setPosition( lightPos );
    mLightCameras[i]->setViewCenter( worldFrustrumCenter );
    mLightCameras[i]->setUpVector( up );

    float lightCameraLeft = 0;
    float lightCameraRight = 0;
    float lightCameraBottom = 0;
    float lightCameraTop = 0;
    float lightCameraNearPlane = 0;
    float lightCameraFarPlane = 0;
    Qgs3DUtils::calculateViewSpaceOrthographicBounds( worldFrustumCorners, lightView, lightCameraLeft, lightCameraRight, lightCameraBottom, lightCameraTop, lightCameraNearPlane, lightCameraFarPlane );

    // Pull the near plane way back to catch shadows from behind the camera
    // If we don't do this, then we'll lose the tops of shadows which should be cast by objects
    // which sit behind this cascade slice's frustrum
    constexpr float NEAR_PLANE_RETREAT = 5000.0f;
    lightCameraNearPlane -= NEAR_PLANE_RETREAT;

    // apply the corresponding Orthographic projection to the camera
    mLightCameras[i]->setProjectionType( Qt3DRender::QCameraLens::ProjectionType::OrthographicProjection );
    mLightCameras[i]->lens()->setOrthographicProjection( lightCameraLeft, lightCameraRight, lightCameraBottom, lightCameraTop, lightCameraNearPlane, lightCameraFarPlane );

    // calculate combined light space matrix for the shader
    csmMatrices[i] = QVariant::fromValue( mLightCameras[i]->projectionMatrix() * lightView );
  }

  mCsmMatricesParameter->setValue( csmMatrices );
}

void QgsPostprocessingEntity::setShadowRenderingEnabled( bool enabled )
{
  mRenderShadowsParameter->setValue( QVariant::fromValue( enabled ? 1 : 0 ) );
}

void QgsPostprocessingEntity::setShowCascadingShadowSplits( bool enabled )
{
  // yes, this is very hacky! It only works once, when first enabling display
  // of the cascading shadow splits. That's ok, it's only here so that we can visualize them
  // in tests...
  if ( enabled )
  {
    const QString fragmentShaderPath = u"qrc:/shaders/postprocess.frag"_s;
    const QByteArray fragmentShaderCode = Qt3DRender::QShaderProgram::loadSource( QUrl( fragmentShaderPath ) );
    QStringList defines { "ENABLE_EFFECTS", "TINT_CASCADES" };
    const QByteArray finalFragmentShaderCode = Qgs3DUtils::addDefinesToShaderCode( fragmentShaderCode, defines );
    mShader->setFragmentShaderCode( finalFragmentShaderCode );
  }
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
