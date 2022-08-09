/***************************************************************************
  qgsrenderpassquad.cpp
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

#include "qgsrenderpassquad.h"

#include <random>

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
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QEffect>
#include <QUrl>
#include <QVector3D>

QgsRenderPassQuad::QgsRenderPassQuad( QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  Qt3DQGeometry *geom = new Qt3DQGeometry( this );
  Qt3DQAttribute *positionAttribute = new Qt3DQAttribute( this );
  const QVector<float> vert = { -1.0f, -1.0f, 0.0f, /**/ 1.0f, -1.0f, 0.0f, /**/ -1.0f,  1.0f, 0.0f, /**/ -1.0f,  1.0f, 0.0f, /**/ 1.0f, -1.0f, 0.0f, /**/ 1.0f,  1.0f, 0.0f };

  const QByteArray vertexArr( ( const char * ) vert.constData(), vert.size() * sizeof( float ) );
  Qt3DQBuffer *vertexBuffer = nullptr;
  vertexBuffer = new Qt3DQBuffer( this );
  vertexBuffer->setData( vertexArr );

  positionAttribute->setName( Qt3DQAttribute::defaultPositionAttributeName() );
  positionAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  positionAttribute->setVertexSize( 3 );
  positionAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
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

  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect( this );
  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique( this );
  Qt3DRender::QGraphicsApiFilter *graphicsApiFilter = technique->graphicsApiFilter();
  graphicsApiFilter->setApi( Qt3DRender::QGraphicsApiFilter::Api::OpenGL );
  graphicsApiFilter->setProfile( Qt3DRender::QGraphicsApiFilter::OpenGLProfile::CoreProfile );
  graphicsApiFilter->setMajorVersion( 3 );
  graphicsApiFilter->setMinorVersion( 1 );
  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass( this );
  mShader = new Qt3DRender::QShaderProgram( this );

  renderPass->setShaderProgram( mShader );

  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest( this );
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );

  renderPass->addRenderState( depthTest );

  technique->addRenderPass( renderPass );

  effect->addTechnique( technique );
  mMaterial->setEffect( effect );

  addComponent( mMaterial );

  mLayer = new Qt3DRender::QLayer( this );
  mLayer->setRecursive( true );
  addComponent( mLayer );
}

