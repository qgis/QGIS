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

#include <QUrl>
#include <QVector3D>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QTechnique>

#include "moc_qgsrenderpassquad.cpp"

QgsRenderPassQuad::QgsRenderPassQuad( Qt3DRender::QLayer *layer, QNode *parent )
  : Qt3DCore::QEntity() // Qt3D bug: if we set the parent in the super constructor
                        // the entity is not attached properly. We attach it at the constructor end.
{
  Qt3DCore::QGeometry *geom = new Qt3DCore::QGeometry( this );
  Qt3DCore::QAttribute *positionAttribute = new Qt3DCore::QAttribute( this );
  const QVector<float> vert = { -1.0f, -1.0f, 0.0f, //
                                1.0f, -1.0f, 0.0f,  //
                                -1.0f, 1.0f, 0.0f,  //
                                -1.0f, 1.0f, 0.0f,  //
                                1.0f, -1.0f, 0.0f,  //
                                1.0f, 1.0f, 0.0f };

  const QByteArray vertexArr( ( const char * ) vert.constData(), vert.size() * sizeof( float ) );
  Qt3DCore::QBuffer *vertexBuffer = nullptr;
  vertexBuffer = new Qt3DCore::QBuffer( this );
  vertexBuffer->setData( vertexArr );

  positionAttribute->setName( Qt3DCore::QAttribute::defaultPositionAttributeName() );
  positionAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  positionAttribute->setVertexSize( 3 );
  positionAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
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

  addComponent( layer );

  setParent( parent ); // Needed!
}
