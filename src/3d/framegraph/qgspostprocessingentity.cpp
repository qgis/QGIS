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
#include "qgsbloomrenderview.h"
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
  QgsBloomRenderView &bloomRenderView = frameGraph->bloomRenderView();

  mColorTextureParameter = new Qt3DRender::QParameter( u"colorTexture"_s, forwardRenderView.colorTexture() );
  mDepthTextureParameter = new Qt3DRender::QParameter( u"depthTexture"_s, forwardRenderView.depthTexture() );
  mAmbientOcclusionTextureParameter = new Qt3DRender::QParameter( u"ssaoTexture"_s, aoRenderView.blurredFactorMapTexture() );

  mMaterial->addParameter( mColorTextureParameter );
  mMaterial->addParameter( mDepthTextureParameter );
  mMaterial->addParameter( mAmbientOcclusionTextureParameter );

  QList<Qt3DRender::QParameter *> globalShadowParams;
  mShadowMapParameter = new Qt3DRender::QParameter( u"shadowTexture"_s, shadowRenderView.mapTextureArray() );
  globalShadowParams << mShadowMapParameter;

  mMainCamera = frameGraph->mainCamera();

  for ( int i = 0; i < Qgs3D::NUM_SHADOW_CASCADES; ++i )
  {
    mLightCameras[i] = shadowRenderView.lightCamera( i );
  }

  // a [0] suffix for a QParameter name maps the parameter to a GLSL array.
  // We must take care that the parameter value is always a variant list of equal length!
  const QVariantList csmMatrices = QVariantList( Qgs3D::NUM_SHADOW_CASCADES, QVariant::fromValue( QMatrix4x4() ) );
  mCsmMatricesParameter = new Qt3DRender::QParameter( QString( "csmMatrices[0]" ), csmMatrices );
  globalShadowParams << mCsmMatricesParameter;
  mCsmBoundsMatricesParameter = new Qt3DRender::QParameter( QString( "csmBoundsMatrices[0]" ), csmMatrices );
  globalShadowParams << mCsmBoundsMatricesParameter;
  mMaxShadowDistanceParameter = new Qt3DRender::QParameter( u"maxShadowDistance"_s, QVariant::fromValue( 0.0f ) );
  globalShadowParams << mMaxShadowDistanceParameter;

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
  globalShadowParams << mRenderShadowsParameter;
  mShadowLightIndexParameter = new Qt3DRender::QParameter( u"shadowLightIndex"_s, QVariant::fromValue( 0 ) );
  globalShadowParams << mShadowLightIndexParameter;
  mShadowBiasParameter = new Qt3DRender::QParameter( u"shadowBias"_s, QVariant::fromValue( 0.00001f ) );
  globalShadowParams << mShadowBiasParameter;

  frameGraph->addGlobalParameters( globalShadowParams );

  mEyeDomeLightingEnabledParameter = new Qt3DRender::QParameter( u"edlEnabled"_s, QVariant::fromValue( 0 ) );
  mEyeDomeLightingStrengthParameter = new Qt3DRender::QParameter( u"edlStrength"_s, QVariant::fromValue( 1000.0f ) );
  mEyeDomeLightingDistanceParameter = new Qt3DRender::QParameter( u"edlDistance"_s, QVariant::fromValue( 2.0f ) );
  mMaterial->addParameter( mEyeDomeLightingEnabledParameter );
  mMaterial->addParameter( mEyeDomeLightingStrengthParameter );
  mMaterial->addParameter( mEyeDomeLightingDistanceParameter );

  mAmbientOcclusionEnabledParameter = new Qt3DRender::QParameter( u"ssaoEnabled"_s, QVariant::fromValue( 0 ) );
  mMaterial->addParameter( mAmbientOcclusionEnabledParameter );

  mBloomTextureParameter = new Qt3DRender::QParameter( u"bloomTexture"_s, bloomRenderView.bloomTexture() );
  mMaterial->addParameter( mBloomTextureParameter );

  mBloomEnabledParameter = new Qt3DRender::QParameter( u"bloomEnabled"_s, QVariant::fromValue( 1 ) );
  mMaterial->addParameter( mBloomEnabledParameter );

  mBloomFactorParameter = new Qt3DRender::QParameter( u"bloomFactor"_s, 0.05 );
  mMaterial->addParameter( mBloomFactorParameter );

  mExposureParameter = new Qt3DRender::QParameter( u"exposureAdjustment"_s, 0.0f );
  mMaterial->addParameter( mExposureParameter );
  mToneMappingParameter = new Qt3DRender::QParameter( u"toneMapping"_s, 1 );
  mMaterial->addParameter( mToneMappingParameter );

  const QString vertexShaderPath = u"qrc:/shaders/postprocess.vert"_s;
  const QString fragmentShaderPath = u"qrc:/shaders/postprocess.frag"_s;

  mShader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( vertexShaderPath ) ) );

  const QByteArray fragmentShaderCode = Qt3DRender::QShaderProgram::loadSource( QUrl( fragmentShaderPath ) );
  const QByteArray finalFragmentShaderCode = Qgs3DUtils::addDefinesToShaderCode( fragmentShaderCode, QStringList( { "ENABLE_EFFECTS" } ) );
  mShader->setFragmentShaderCode( finalFragmentShaderCode );
}

void QgsPostprocessingEntity::updateBloomSettings( const QgsBloomSettings &settings )
{
  setBloomFactor( static_cast< float >( settings.intensity() ) );
}

void QgsPostprocessingEntity::updateShadowSettings( const QgsShadowSettings &shadowSettings, const QgsVector3D &lightDir, int size, int globalLightIndex )
{
  // We are using "Cascading Shadow Maps" technique.
  // Reading/watching which was useful during development:
  // https://learnopengl.com/Guest-Articles/2021/CSM
  // https://developer.download.nvidia.com/SDK/10.5/opengl/src/cascaded_shadow_maps/doc/cascaded_shadow_maps.pdf
  // https://www.youtube.com/watch?v=Jhopq2lkzMQ
  // https://www.youtube.com/watch?v=qbDrqARX07o
  // https://web.archive.org/web/20170710150304/https://cesiumjs.org/presentations/ShadowsAndCesiumImplementation.pdf

  mShadowMapResolution = size;
  setShadowLightIndex( globalLightIndex );
  setShadowBias( static_cast<float>( shadowSettings.shadowBias() ) );
  float maximumShadowRenderingDistance = static_cast<float>( shadowSettings.maximumShadowRenderingDistance() );

  const QVector3D lightDirection = lightDir.toVector3D().normalized();
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

  const float shadowMapResolution = static_cast< float >( mShadowMapResolution );

  // we are building two matrix lists, one containing the exact bounds of each
  // cascade, and the other which is an exact match for the actual camera used
  // for each cascade's texture. These differ, as we pull back the camera's
  // near plane for reasons described below...
  QVariantList csmMatrices( Qgs3D::NUM_SHADOW_CASCADES, QVariant() );
  QVariantList csmBoundsMatrices( Qgs3D::NUM_SHADOW_CASCADES, QVariant() );

  // here we are calculating the cascades using bounding spheres, in order to stabilise the
  // matrices and avoid shadow shimmer when the camera is moved or rotated
  // see eg https://media.gdcvault.com/gdc09/slides/100_Handout%203.pdf from slide 21
  for ( int i = 0; i < Qgs3D::NUM_SHADOW_CASCADES; ++i )
  {
    const float zNear = cascadeSplits[i];
    const float zFar = cascadeSplits[i + 1];

    // calculate the 8 corners of the camera frustum slice in world space
    QVector3D worldFrustumCorners[8];
    QVector3D worldFrustrumCenter;
    Qgs3DUtils::calculateFrustumSliceCorners( zNear, zFar, cameraFov, cameraAspect, invertedCameraView, worldFrustumCorners, worldFrustrumCenter );

    // calculate the bounding sphere radius around the frustum center
    float rawRadius = 0.0f;
    for ( int j = 0; j < 8; ++j )
    {
      rawRadius = std::max( rawRadius, ( worldFrustumCorners[j] - worldFrustrumCenter ).length() );
    }

    // round up slightly to stabilize against floating point inaccuracies
    // use dynamic step size based on the raw radius so we round larger radius to coarser increments
    const float stepSize = std::max( std::pow( 2.0f, std::floor( std::log2( rawRadius ) ) - 4.0f ), 0.01f );
    const float radius = std::ceil( rawRadius / stepSize ) * stepSize;

    // project the actual world frustum center into this rotation-aligned light space
    QMatrix4x4 lightRotation;
    lightRotation.lookAt( QVector3D( 0, 0, 0 ), lightDirection, up );
    QVector3D centerLightSpace = lightRotation * worldFrustrumCenter;

    // snap to texels
    // calculate how many world units are represented by a single texel
    const float worldUnitsPerTexel = ( 2.0f * radius ) / shadowMapResolution;
    // snap the light center to the nearest texel
    centerLightSpace.setX( std::floor( centerLightSpace.x() / worldUnitsPerTexel ) * worldUnitsPerTexel );
    centerLightSpace.setY( std::floor( centerLightSpace.y() / worldUnitsPerTexel ) * worldUnitsPerTexel );
    const QVector3D snappedWorldCenter = lightRotation.inverted() * centerLightSpace;

    // create the light view matrix
    QMatrix4x4 lightView;
    const QVector3D lightPos = snappedWorldCenter - ( lightDirection * radius );
    lightView.lookAt( lightPos, snappedWorldCenter, up );

    // apply to the specific light camera
    mLightCameras[i]->setPosition( lightPos );
    mLightCameras[i]->setViewCenter( snappedWorldCenter );
    mLightCameras[i]->setUpVector( up );

    float lightCameraLeft = -radius;
    float lightCameraRight = radius;
    float lightCameraBottom = -radius;
    float lightCameraTop = radius;
    // the Z-bounds must cover the entire bounding sphere
    float lightCameraNearPlane = -radius;
    float lightCameraFarPlane = radius * 2.0f;

    QMatrix4x4 orthoBoundsMatrix;
    orthoBoundsMatrix.ortho( lightCameraLeft, lightCameraRight, lightCameraBottom, lightCameraTop, lightCameraNearPlane, lightCameraFarPlane );
    csmBoundsMatrices[i] = QVariant::fromValue( orthoBoundsMatrix * lightView );

    // Pull the near plane way back to catch shadows from behind the camera
    // If we don't do this, then we'll lose the tops of shadows which should be cast by objects
    // which sit behind this cascade slice's frustrum
    constexpr float NEAR_PLANE_RETREAT = 5000.0f;
    lightCameraNearPlane -= NEAR_PLANE_RETREAT;

    // apply the corresponding Orthographic projection to the camera
    mLightCameras[i]->lens()->setOrthographicProjection( lightCameraLeft, lightCameraRight, lightCameraBottom, lightCameraTop, lightCameraNearPlane, lightCameraFarPlane );

    // calculate combined light space matrix for the shader
    QMatrix4x4 orthoMatrix;
    orthoMatrix.ortho( lightCameraLeft, lightCameraRight, lightCameraBottom, lightCameraTop, lightCameraNearPlane, lightCameraFarPlane );
    csmMatrices[i] = QVariant::fromValue( orthoMatrix * lightView );
  }

  mCsmMatricesParameter->setValue( csmMatrices );
  mCsmBoundsMatricesParameter->setValue( csmBoundsMatrices );
  mMaxShadowDistanceParameter->setValue( mainCameraFarPlane );

  setShowCascadingShadowSplits( shadowSettings.showCascadeSplits() );
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

void QgsPostprocessingEntity::setShadowLightIndex( int index )
{
  mShadowLightIndexParameter->setValue( QVariant::fromValue( index ) );
}

void QgsPostprocessingEntity::setShadowBias( float shadowBias )
{
  mShadowBiasParameter->setValue( QVariant::fromValue( shadowBias ) );
}

void QgsPostprocessingEntity::updateEyeDomeSettings( const Qgs3DMapSettings &settings )
{
  setEyeDomeLightingStrength( settings.eyeDomeLightingStrength() );
  setEyeDomeLightingDistance( settings.eyeDomeLightingDistance() );
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

void QgsPostprocessingEntity::setBloomEnabled( bool enabled )
{
  mBloomEnabledParameter->setValue( QVariant::fromValue( enabled ? 1 : 0 ) );
}

void QgsPostprocessingEntity::setBloomFactor( float factor )
{
  mBloomFactorParameter->setValue( factor );
}

void QgsPostprocessingEntity::updateColorGradingSettings( const QgsColorGradingSettings &settings )
{
  mExposureParameter->setValue( static_cast< float >( settings.exposureAdjustment() ) );
  mToneMappingParameter->setValue( static_cast< int >( settings.toneMapping() ) );
}
