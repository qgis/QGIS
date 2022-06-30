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

QgsAmbientOcclusionRenderEntity::QgsAmbientOcclusionRenderEntity( QgsShadowRenderingFrameGraph *frameGraph, QNode *parent )
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

  mEffect = new Qt3DRender::QEffect( this );
  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique( this );
  Qt3DRender::QGraphicsApiFilter *graphicsApiFilter = technique->graphicsApiFilter();
  graphicsApiFilter->setApi( Qt3DRender::QGraphicsApiFilter::Api::OpenGL );
  graphicsApiFilter->setProfile( Qt3DRender::QGraphicsApiFilter::OpenGLProfile::CoreProfile );
  graphicsApiFilter->setMajorVersion( 1 );
  graphicsApiFilter->setMinorVersion( 5 );
  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass( this );
  Qt3DRender::QShaderProgram *shader = new Qt3DRender::QShaderProgram( this );

  const QString vertexShaderPath = QStringLiteral( "qrc:/shaders/ssao_factor_render.vert" );
  const QString fragmentShaderPath = QStringLiteral( "qrc:/shaders/ssao_factor_render.frag" );

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
