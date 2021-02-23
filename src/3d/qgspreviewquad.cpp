/***************************************************************************
  qgspreviewquad.cpp
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

#include "qgspreviewquad.h"

#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QShaderProgram>
#include <QMatrix4x4>
#include <QUrl>

QgsPreviewQuad::QgsPreviewQuad( Qt3DRender::QAbstractTexture *texture,
                                const QPointF &centerNDC, const QSizeF &size,
                                QVector<Qt3DRender::QParameter *> additionalShaderParameters,
                                Qt3DCore::QEntity *parent )
  : Qt3DCore::QEntity( parent )
{
  setObjectName( "Preview Quad" );
  Qt3DRender::QGeometry *geom = new Qt3DRender::QGeometry;
  Qt3DRender::QAttribute *positionAttribute = new Qt3DRender::QAttribute;
  QVector<float> vert = { -1.0f, -1.0f, 1.0f, /**/ 1.0f, -1.0f, 1.0f, /**/ -1.0f,  1.0f, 1.0f, /**/ -1.0f,  1.0f, 1.0f, /**/ 1.0f, -1.0f, 1.0f, /**/ 1.0f,  1.0f, 1.0f };

  QByteArray vertexArr( ( const char * ) vert.constData(), vert.size() * sizeof( float ) );
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

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::PrimitiveType::Triangles );
  renderer->setGeometry( geom );

  addComponent( renderer );

  QMatrix4x4 modelMatrix;
  modelMatrix.setToIdentity();
  modelMatrix.translate( centerNDC.x(), centerNDC.y() );
  modelMatrix.scale( size.width(), size.height() );
  mMaterial = new QgsPreviewQuadMaterial( texture, modelMatrix, additionalShaderParameters );

  addComponent( mMaterial );
}

void QgsPreviewQuad::setViewPort( const QPointF &centerNDC, const QSizeF &size )
{
  QMatrix4x4 modelMatrix;
  modelMatrix.setToIdentity();
  modelMatrix.translate( centerNDC.x(), centerNDC.y() );
  modelMatrix.scale( size.width(), size.height() );
  mMaterial->setModelMatrix( modelMatrix );
}

QgsPreviewQuadMaterial::QgsPreviewQuadMaterial( Qt3DRender::QAbstractTexture *texture, const QMatrix4x4 &modelMatrix, QVector<Qt3DRender::QParameter *> additionalShaderParameters, QNode *parent )
  : Qt3DRender::QMaterial( parent )
{
  mTextureParameter = new Qt3DRender::QParameter( "previewTexture", texture );
  mTextureTransformParameter = new Qt3DRender::QParameter( "modelMatrix", QVariant::fromValue( modelMatrix ) );
  addParameter( mTextureParameter );
  addParameter( mTextureTransformParameter );
  for ( Qt3DRender::QParameter *parameter : additionalShaderParameters ) addParameter( parameter );

  mEffect = new Qt3DRender::QEffect;

  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique;

  Qt3DRender::QGraphicsApiFilter *graphicsApiFilter = technique->graphicsApiFilter();
  graphicsApiFilter->setApi( Qt3DRender::QGraphicsApiFilter::Api::OpenGL );
  graphicsApiFilter->setProfile( Qt3DRender::QGraphicsApiFilter::OpenGLProfile::CoreProfile );
  graphicsApiFilter->setMajorVersion( 1 );
  graphicsApiFilter->setMinorVersion( 5 );

  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass;

  Qt3DRender::QShaderProgram *shader = new Qt3DRender::QShaderProgram;
  shader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( "qrc:/shaders/preview.vert" ) ) );
  shader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( "qrc:/shaders/preview.frag" ) ) );
  renderPass->setShaderProgram( shader );

  technique->addRenderPass( renderPass );

  mEffect->addTechnique( technique );
  setEffect( mEffect );
}

void QgsPreviewQuadMaterial::setModelMatrix( const QMatrix4x4 &modelMatrix )
{
  mTextureTransformParameter->setValue( modelMatrix );
}
