/***************************************************************************
  qgsdebugtextureentity.cpp
  --------------------------------------
  Date                 : June 2024
  Copyright            : (C) 2024 by Benoit De Mezzo
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdebugtextureentity.h"
#include "moc_qgsdebugtextureentity.cpp"
#include <Qt3DRender/QTexture>

#include <Qt3DRender/QParameter>
#include <QUrl>
#include <QVector2D>

QgsDebugTextureEntity::QgsDebugTextureEntity( Qt3DRender::QTexture2D *texture, Qt3DRender::QLayer *layer, QNode *parent )
  : QgsRenderPassQuad( layer, parent )
{
  setObjectName( "DebugTextureQuad" );

  mTextureParameter = new Qt3DRender::QParameter( "previewTexture", texture );
  mCenterTextureCoords = new Qt3DRender::QParameter( "centerTexCoords", QVector2D( 0, 0 ) );
  mSizeTextureCoords = new Qt3DRender::QParameter( "sizeTexCoords", QVector2D( 1, 1 ) );
  mMaterial->addParameter( mTextureParameter );
  mMaterial->addParameter( mCenterTextureCoords );
  mMaterial->addParameter( mSizeTextureCoords );
  mMaterial->addParameter( new Qt3DRender::QParameter( "isDepth", true ) );

  mShader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( "qrc:/shaders/preview.vert" ) ) );
  mShader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( "qrc:/shaders/preview.frag" ) ) );

  setViewport( QPointF( 0.9f, 0.9f ), QSizeF( 0.1, 0.1 ) );

  setEnabled( false );
}

void QgsDebugTextureEntity::setPosition( Qt::Corner corner, double size )
{
  switch ( corner )
  {
    case Qt::Corner::TopRightCorner:
      setViewport( QPointF( 1.0f - size / 2, 0.0f + size / 2 ), 0.5 * QSizeF( size, size ) );
      break;
    case Qt::Corner::TopLeftCorner:
      setViewport( QPointF( 0.0f + size / 2, 0.0f + size / 2 ), 0.5 * QSizeF( size, size ) );
      break;
    case Qt::Corner::BottomRightCorner:
      setViewport( QPointF( 1.0f - size / 2, 1.0f - size / 2 ), 0.5 * QSizeF( size, size ) );
      break;
    case Qt::Corner::BottomLeftCorner:
      setViewport( QPointF( 0.0f + size / 2, 1.0f - size / 2 ), 0.5 * QSizeF( size, size ) );
      break;
  }
}


void QgsDebugTextureEntity::setViewport( const QPointF &centerTexCoords, const QSizeF &sizeTexCoords )
{
  mCenterTextureCoords->setValue( QVector2D( static_cast<float>( centerTexCoords.x() ), static_cast<float>( centerTexCoords.y() ) ) );
  mSizeTextureCoords->setValue( QVector2D( static_cast<float>( sizeTexCoords.width() ), static_cast<float>( sizeTexCoords.height() ) ) );
}
