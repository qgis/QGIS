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
  const QVector3D lightDirection = light.direction().toVector3D().normalized();
  QVector3D up( 0.0f, 1.0f, 0.0f );
  if ( std::abs( QVector3D::dotProduct( lightDirection, up ) ) > 0.99f )
    up = QVector3D( 0.0f, 0.0f, 1.0f );

  const float nearPlane = mMainCamera->nearPlane();
  // cap the far plane to the shadow rendering distance so we don't waste shadow resolution
  const float farPlane = std::min( mMainCamera->farPlane(), maximumShadowRenderingDistance );

  std::vector<float> cascadeSplits( Qgs3D::NUM_SHADOW_CASCADES + 1 );

  // "Practical Split Scheme" for cascading shadow maps.
  // using a quite large lambda to account for typical near/far plane distances seen in
  // QGIS 3d maps (0.5 - ~2500)
  constexpr float PRACTICAL_SPLIT_SCHEME_LAMBDA = 0.96f;
  for ( int i = 0; i <= Qgs3D::NUM_SHADOW_CASCADES; ++i )
  {
    const float p = static_cast<float>( i ) / static_cast<float>( Qgs3D::NUM_SHADOW_CASCADES );
    const float logSplit = nearPlane * std::pow( farPlane / nearPlane, p );
    const float uniSplit = nearPlane + ( farPlane - nearPlane ) * p;
    cascadeSplits[i] = PRACTICAL_SPLIT_SCHEME_LAMBDA * logSplit + ( 1.0f - PRACTICAL_SPLIT_SCHEME_LAMBDA ) * uniSplit;
  }

  const QMatrix4x4 invertedCameraView = mMainCamera->viewMatrix().inverted();
  const float fovC = static_cast< float >( std::tan( mMainCamera->fieldOfView() * M_PI / 360.0 ) );
  const float aspect = mMainCamera->aspectRatio();

  QVariantList csmSplits( Qgs3D::NUM_SHADOW_CASCADES, QVariant() );
  QVariantList csmMatrices( Qgs3D::NUM_SHADOW_CASCADES, QVariant() );
  for ( int i = 0; i < Qgs3D::NUM_SHADOW_CASCADES; ++i )
  {
    const float zNear = cascadeSplits[i];
    const float zFar = cascadeSplits[i + 1];
    csmSplits[i] = zFar;

    // calculate the 8 corners of the camera frustum slice in camera view space
    const float halfYNear = zNear * fovC;
    const float halfXNear = halfYNear * aspect;
    const float halfYFar = zFar * fovC;
    const float halfXFar = halfYFar * aspect;
    QVector3D corners[8] = {
      QVector3D( -halfXNear, -halfYNear, -zNear ),
      QVector3D( halfXNear, -halfYNear, -zNear ),
      QVector3D( halfXNear, halfYNear, -zNear ),
      QVector3D( -halfXNear, halfYNear, -zNear ),
      QVector3D( -halfXFar, -halfYFar, -zFar ),
      QVector3D( halfXFar, -halfYFar, -zFar ),
      QVector3D( halfXFar, halfYFar, -zFar ),
      QVector3D( -halfXFar, halfYFar, -zFar )
    };

    // transform corners to world space and find the center
    QVector3D center( 0, 0, 0 );
    for ( int j = 0; j < 8; ++j )
    {
      corners[j] = invertedCameraView.map( corners[j] );
      center += corners[j];
    }
    center /= 8.0f;

    // create the light view matrix
    QMatrix4x4 lightView;
    constexpr float lightPosDistanceFactor = 1000.0f;
    const QVector3D lightPos = center - lightDirection * lightPosDistanceFactor; // Pull back the camera position
    lightView.lookAt( lightPos, center, up );

    // apply to the specific light camera
    mLightCameras[i]->setPosition( lightPos );
    mLightCameras[i]->setViewCenter( center );
    mLightCameras[i]->setUpVector( up );

    // transform corners to light space to find the bounding box
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();
    for ( int j = 0; j < 8; ++j )
    {
      const QVector3D lightSpaceCorner = lightView.map( corners[j] );
      minX = std::min( minX, lightSpaceCorner.x() );
      maxX = std::max( maxX, lightSpaceCorner.x() );
      minY = std::min( minY, lightSpaceCorner.y() );
      maxY = std::max( maxY, lightSpaceCorner.y() );
      const float zDistance = -lightSpaceCorner.z();
      minZ = std::min( minZ, zDistance );
      maxZ = std::max( maxZ, zDistance );
    }

    // Pull the near plane way back to catch shadows from behind the camera
    constexpr float nearPlanRetreat = 5000.0f;
    minZ -= nearPlanRetreat;

    // apply the corresponding Orthographic projection to the camera
    mLightCameras[i]->setProjectionType( Qt3DRender::QCameraLens::ProjectionType::OrthographicProjection );
    mLightCameras[i]->lens()->setOrthographicProjection( minX, maxX, minY, maxY, minZ, maxZ );

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
