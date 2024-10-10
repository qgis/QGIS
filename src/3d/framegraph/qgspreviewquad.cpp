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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
typedef Qt3DRender::QAttribute Qt3DQAttribute;
typedef Qt3DRender::QBuffer Qt3DQBuffer;
typedef Qt3DRender::QGeometry Qt3DQGeometry;
#else
#include <Qt3DCore/QGeometry>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
typedef Qt3DCore::QAttribute Qt3DQAttribute;
typedef Qt3DCore::QBuffer Qt3DQBuffer;
typedef Qt3DCore::QGeometry Qt3DQGeometry;
#endif
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QShaderProgram>
#include <QMatrix4x4>
#include <QUrl>
#include <QVector2D>

QgsPreviewQuad::QgsPreviewQuad( Qt3DRender::QAbstractTexture *texture,
                                const QPointF &centerTexCoords, const QSizeF &sizeTexCoords,
                                QVector<Qt3DRender::QParameter *> additionalShaderParameters,
                                Qt3DCore::QEntity *parent )
  : Qt3DCore::QEntity( parent )
{
  setObjectName( "Preview Quad" );
  Qt3DQGeometry *geom = new Qt3DQGeometry;
  Qt3DQAttribute *positionAttribute = new Qt3DQAttribute;
  const QVector<float> vert = { -1.0f, -1.0f, 1.0f, /**/ 1.0f, -1.0f, 1.0f, /**/ -1.0f,  1.0f, 1.0f, /**/ -1.0f,  1.0f, 1.0f, /**/ 1.0f, -1.0f, 1.0f, /**/ 1.0f,  1.0f, 1.0f };

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

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::PrimitiveType::Triangles );
  renderer->setGeometry( geom );

  addComponent( renderer );

  mMaterial = new QgsPreviewQuadMaterial( texture, additionalShaderParameters );

  addComponent( mMaterial );

  setViewPort( centerTexCoords, sizeTexCoords );
}

void QgsPreviewQuad::setViewPort( const QPointF &centerTexCoords, const QSizeF &sizeTexCoords )
{
  mMaterial->setViewPort( QVector2D( centerTexCoords.x(), centerTexCoords.y() ), QVector2D( sizeTexCoords.width(), sizeTexCoords.height() ) );
}

QgsPreviewQuadMaterial::QgsPreviewQuadMaterial( Qt3DRender::QAbstractTexture *texture, QVector<Qt3DRender::QParameter *> additionalShaderParameters, QNode *parent )
  : Qt3DRender::QMaterial( parent )
{
  mTextureParameter = new Qt3DRender::QParameter( "previewTexture", texture );
  mCenterTextureCoords = new Qt3DRender::QParameter( "centerTexCoords", QVector2D( 0, 0 ) );
  mSizeTextureCoords = new Qt3DRender::QParameter( "sizeTexCoords", QVector2D( 1, 1 ) );
  addParameter( mTextureParameter );
  addParameter( mCenterTextureCoords );
  addParameter( mSizeTextureCoords );
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

void QgsPreviewQuadMaterial::setViewPort( QVector2D centerTexCoords, QVector2D sizeTexCoords )
{
  mCenterTextureCoords->setValue( centerTexCoords );
  mSizeTextureCoords->setValue( sizeTexCoords );
}
