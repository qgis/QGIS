/***************************************************************************
  qgspoint3dbillboardmaterial.h
  --------------------------------------
  Date                 : Jul 2019
  Copyright            : (C) 2019 by Ismail Sunni
  Email                : imajimatika at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QEffect>
#include <QPainter>

#include "qgspoint3dbillboardmaterial.h"

QgsPoint3DBillboardMaterial::QgsPoint3DBillboardMaterial()
  : mSize( new Qt3DRender::QParameter( "BB_SIZE", QSizeF( 100, 100 ), this ) )
  , mWindowSize( new Qt3DRender::QParameter( "WIN_SCALE", QSizeF( 800, 600 ), this ) )
{
  addParameter( mSize );
  addParameter( mWindowSize );

  Qt3DRender::QTextureImage *image = new Qt3DRender::QTextureImage;
  image->setSource( QUrl( QStringLiteral( "qrc:/shaders/success-kid.png" ) ) );

  // Texture2D
  Qt3DRender::QTexture2D *texture2D = new Qt3DRender::QTexture2D;
  texture2D->setGenerateMipMaps( false );
  texture2D->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  texture2D->setMinificationFilter( Qt3DRender::QTexture2D::Linear );

  texture2D->addTextureImage( image );

  mTexture2D = new Qt3DRender::QParameter( "tex0", texture2D, this );

  addParameter( mTexture2D );

  // Shader program
  Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram( this );
  shaderProgram->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/billboards.vert" ) ) ) );
  shaderProgram->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/billboards.frag" ) ) ) );
  shaderProgram->setGeometryShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/billboards.geom" ) ) ) );

  // Render Pass
  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass( this );
  renderPass->setShaderProgram( shaderProgram );

  // without this filter the default forward renderer would not render this
  Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey;
  filterKey->setName( QStringLiteral( "renderingStyle" ) );
  filterKey->setValue( "forward" );

  // Technique
  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique;
  technique->addRenderPass( renderPass );
  technique->addFilterKey( filterKey );
  technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
  technique->graphicsApiFilter()->setMajorVersion( 3 );
  technique->graphicsApiFilter()->setMinorVersion( 1 );

  // Effect
  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect( this );
  effect->addTechnique( technique );

  setEffect( effect );
}

void QgsPoint3DBillboardMaterial::setSize( const QSizeF size )
{
  mSize->setValue( size );
}

QSizeF QgsPoint3DBillboardMaterial::size() const
{
  return mSize->value().value<QSizeF>();
}

void QgsPoint3DBillboardMaterial::setWindowSize( const QSizeF size )
{
  mWindowSize->setValue( size );
}

QSizeF QgsPoint3DBillboardMaterial::windowSize() const
{
  return mWindowSize->value().value<QSizeF>();
}

void QgsPoint3DBillboardMaterial::setTexture2D( Qt3DRender::QTexture2D *texture2D )
{
  mTexture2D->setValue( QVariant::fromValue( texture2D ) );
}

Qt3DRender::QTexture2D *QgsPoint3DBillboardMaterial::texture2D()
{
  QVariant variant = mTexture2D->value();
  return qvariant_cast<Qt3DRender::QTexture2D *>( variant );
}

void QgsPoint3DBillboardMaterial::setTexture2DFromImagePath( QString imagePath )
{
  // Texture Image
  Qt3DRender::QTextureImage *image = new Qt3DRender::QTextureImage;
  image->setSource( QUrl( imagePath ) );

  // Texture2D
  Qt3DRender::QTexture2D *texture2D = new Qt3DRender::QTexture2D;
  texture2D->setGenerateMipMaps( false );
  texture2D->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  texture2D->setMinificationFilter( Qt3DRender::QTexture2D::Linear );

  texture2D->addTextureImage( image );

  setTexture2D( texture2D );
}

void QgsPoint3DBillboardMaterial::setTexture2DFromImage( QImage image )
{
  // Store it as QgsBillboardTextureImage
  QgsBillboardTextureImage *billboardTextureImage = new QgsBillboardTextureImage();
  billboardTextureImage->setImage( &image );

  // Texture2D
  Qt3DRender::QTexture2D *texture2D = new Qt3DRender::QTexture2D;
  texture2D->setGenerateMipMaps( false );
  texture2D->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  texture2D->setMinificationFilter( Qt3DRender::QTexture2D::Linear );

  texture2D->addTextureImage( billboardTextureImage );

  setTexture2D( texture2D );
}


void QgsBillboardTextureImage::paint( QPainter *painter )
{
  QRect rectangle = QRect( 0, 0, mImage->width(), mImage->height() );
  painter->drawImage( rectangle, *mImage, rectangle );
}

void QgsBillboardTextureImage::setImage( QImage *image )
{
  mImage = image;
  setSize( image->size() );
  QPainter *painter = new QPainter();
  paint( painter );
}

QImage *QgsBillboardTextureImage::image()
{
  return  mImage;
}
