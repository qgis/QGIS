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

#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QDepthTest>
#include <QUrl>

#include "qgsshadowrenderingframegraph.h"

QgsPostprocessingEntity::QgsPostprocessingEntity( QgsShadowRenderingFrameGraph *frameGraph, QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  Qt3DRender::QGeometry *geom = new Qt3DRender::QGeometry( this );
  Qt3DRender::QAttribute *positionAttribute = new Qt3DRender::QAttribute( this );
  const QVector<float> vert = { -1.0f, -1.0f, 0.0f, /**/ 1.0f, -1.0f, 0.0f, /**/ -1.0f,  1.0f, 0.0f, /**/ -1.0f,  1.0f, 0.0f, /**/ 1.0f, -1.0f, 0.0f, /**/ 1.0f,  1.0f, 0.0f };

  const QByteArray vertexArr( ( const char * ) vert.constData(), vert.size() * sizeof( float ) );
  Qt3DRender::QBuffer *vertexBuffer = nullptr;
  vertexBuffer = new Qt3DRender::QBuffer( this );
  vertexBuffer->setData( vertexArr );

  positionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );
  positionAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  positionAttribute->setVertexSize( 3 );
  positionAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  positionAttribute->setBuffer( vertexBuffer );
  positionAttribute->setByteOffset( 0 );
  positionAttribute->setByteStride( 3 * sizeof( float ) );
  positionAttribute->setCount( 6 );

  geom->addAttribute( positionAttribute );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer( this );
  renderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::PrimitiveType::Triangles );
  renderer->setGeometry( geom );

  addComponent( renderer );

  mMaterial = new Qt3DRender::QMaterial( this );
  mColorTextureParameter = new Qt3DRender::QParameter( QStringLiteral( "colorTexture" ), frameGraph->forwardRenderColorTexture() );
  mDepthTextureParameter = new Qt3DRender::QParameter( QStringLiteral( "depthTexture" ), frameGraph->forwardRenderDepthTexture() );
  mShadowMapParameter = new Qt3DRender::QParameter( QStringLiteral( "shadowTexture" ), frameGraph->shadowMapTexture() );
  mMaterial->addParameter( mColorTextureParameter );
  mMaterial->addParameter( mDepthTextureParameter );
  mMaterial->addParameter( mShadowMapParameter );

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

  mLightPosition = new Qt3DRender::QParameter( QStringLiteral( "lightPosition" ), QVariant::fromValue( QVector3D() ) );
  mLightDirection = new Qt3DRender::QParameter( QStringLiteral( "lightDirection" ), QVariant::fromValue( QVector3D() ) );
  mMaterial->addParameter( mLightPosition );
  mMaterial->addParameter( mLightDirection );

  mEffect = new Qt3DRender::QEffect( this );
  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique( this );
  Qt3DRender::QGraphicsApiFilter *graphicsApiFilter = technique->graphicsApiFilter();
  graphicsApiFilter->setApi( Qt3DRender::QGraphicsApiFilter::Api::OpenGL );
  graphicsApiFilter->setProfile( Qt3DRender::QGraphicsApiFilter::OpenGLProfile::CoreProfile );
  graphicsApiFilter->setMajorVersion( 1 );
  graphicsApiFilter->setMinorVersion( 5 );
  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass( this );
  Qt3DRender::QShaderProgram *shader = new Qt3DRender::QShaderProgram( this );

  const QString vertexShaderPath = QStringLiteral( "qrc:/shaders/postprocess.vert" );
  const QString fragmentShaderPath = QStringLiteral( "qrc:/shaders/postprocess.frag" );

  shader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( vertexShaderPath ) ) );
  shader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( fragmentShaderPath ) ) );
  renderPass->setShaderProgram( shader );

  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest( this );
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );

  renderPass->addRenderState( depthTest );

  technique->addRenderPass( renderPass );

  mEffect->addTechnique( technique );
  mMaterial->setEffect( mEffect );

  addComponent( mMaterial );
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
