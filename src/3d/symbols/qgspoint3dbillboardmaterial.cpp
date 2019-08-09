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
#include <Qt3DRender/QBlendEquationArguments>
#include <Qt3DRender/QBlendEquation>
#include <Qt3DRender/QNoDepthMask>
#include <QPainter>

#include "qgslogger.h"
#include "qgspoint3dbillboardmaterial.h"
#include "qgsterraintextureimage_p.h"
#include "qgsmarkersymbollayer.h"
#include "qgssymbollayerutils.h"

QgsPoint3DBillboardMaterial::QgsPoint3DBillboardMaterial()
  : mSize( new Qt3DRender::QParameter( "BB_SIZE", QSizeF( 100, 100 ), this ) )
  , mViewportSize( new Qt3DRender::QParameter( "WIN_SCALE", QSizeF( 800, 600 ), this ) )
{
  addParameter( mSize );
  addParameter( mViewportSize );

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

  // Blending for handling transparency
  Qt3DRender::QBlendEquationArguments *blendState = new Qt3DRender::QBlendEquationArguments;
  blendState->setSourceRgb( Qt3DRender::QBlendEquationArguments::SourceAlpha );
  blendState->setDestinationRgb( Qt3DRender::QBlendEquationArguments::OneMinusSourceAlpha );

  Qt3DRender::QBlendEquation *blendEquation = new Qt3DRender::QBlendEquation;
  blendEquation->setBlendFunction( Qt3DRender::QBlendEquation::Add );

  // Shader program
  Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram( this );
  shaderProgram->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/billboards.vert" ) ) ) );
  shaderProgram->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/billboards.frag" ) ) ) );
  shaderProgram->setGeometryShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/billboards.geom" ) ) ) );

  // Render Pass
  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass( this );
  renderPass->setShaderProgram( shaderProgram );
  renderPass->addRenderState( blendState );
  renderPass->addRenderState( blendEquation );

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

void QgsPoint3DBillboardMaterial::setViewportSize( const QSizeF size )
{
  mViewportSize->setValue( size );
}

QSizeF QgsPoint3DBillboardMaterial::windowSize() const
{
  return mViewportSize->value().value<QSizeF>();
}

void QgsPoint3DBillboardMaterial::setTexture2D( Qt3DRender::QTexture2D *texture2D )
{
//  removeParameter(mTexture2D);

//  mTexture2D = new Qt3DRender::QParameter( "tex0", texture2D, this );

//  addParameter( mTexture2D );
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
  // Create texture image
  QgsRectangle dummyExtent = QgsRectangle( 0 );
  QgsTerrainTextureImage *billboardTextureImage = new QgsTerrainTextureImage( image, dummyExtent, mName );

  // Create texture 2D from the texture image
  Qt3DRender::QTexture2D *texture = new Qt3DRender::QTexture2D;
  texture->setGenerateMipMaps( false );
  texture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  texture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
//  texture->setSize(image.size().width(), image.size().height());
  texture->addTextureImage( billboardTextureImage );

  // Set texture 2D
  setTexture2D( texture );
}

void QgsPoint3DBillboardMaterial::useDefaultSymbol()
{
  // Default texture
  QgsMarkerSymbol *defaultSymbol = static_cast<QgsMarkerSymbol *>( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  defaultSymbol->setSizeUnit( QgsUnitTypes::RenderUnit::RenderPixels );
  defaultSymbol->setSize( 20 );

  setTexture2DFromSymbol( defaultSymbol );
  mName = QStringLiteral( "default symbol" );
}

void QgsPoint3DBillboardMaterial::setTexture2DFromSymbol( QgsMarkerSymbol *markerSymbol )
{
  markerSymbol->setSizeUnit( QgsUnitTypes::RenderUnit::RenderPixels );
  markerSymbol->setSize( 20 );
  QPixmap symbolPixmap = QgsSymbolLayerUtils::symbolPreviewPixmap( markerSymbol, QSize( int( markerSymbol->size() ) + 5, int( markerSymbol->size() ) + 5 ), 0 );
  QImage symbolImage = symbolPixmap.toImage();
//  symbolImage.save( "/home/ismailsunni/dev/cpp/test.png" );
  setTexture2DFromImage( symbolImage );
  mName = markerSymbol->color().name();
}

void QgsPoint3DBillboardMaterial::debug()
{
  QVector<Qt3DRender::QAbstractTextureImage *> textureImages = texture2D()->textureImages();
  QgsDebugMsg( "Name or color: " + name() );
//    QVectorIterator<Qt3DRender::QAbstractTextureImage *> i(textureImages);
//    int p = 0;
//    while (i.hasNext())
//    {
//        p++;
//        QgsTerrainTextureImage* tti = dynamic_cast<QgsTerrainTextureImage *>(i.next());
////        QgsDebugMsg("Terrain Texture Image " + QString::number(p) + " : " + tti->imageDebugText());
//        QgsDebugMsg(i.next()->metaObject()->className() );
//        i.next();
//    }
//    QgsDebugMsg("Number of texture: " + QString::number(p));
}
