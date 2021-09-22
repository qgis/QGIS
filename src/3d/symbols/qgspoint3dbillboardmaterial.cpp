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
#include <QUrl>

#include "qgslogger.h"
#include "qgspoint3dbillboardmaterial.h"
#include "qgsterraintextureimage_p.h"
#include "qgsmarkersymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgssettings.h"
#include "qgs3dmapsettings.h"
#include "qgsmarkersymbol.h"

QgsPoint3DBillboardMaterial::QgsPoint3DBillboardMaterial()
  : mSize( new Qt3DRender::QParameter( "BB_SIZE", QSizeF( 100, 100 ), this ) )
  , mViewportSize( new Qt3DRender::QParameter( "WIN_SCALE", QSizeF( 800, 600 ), this ) )
{
  addParameter( mSize );
  addParameter( mViewportSize );

  // Initialize with empty parameter.
  mTexture2D = new Qt3DRender::QParameter( "tex0", QVariant(), this );
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

void QgsPoint3DBillboardMaterial::setTexture2DFromImage( QImage image, double size )
{
  // Create texture image
  const QgsRectangle randomExtent = QgsRectangle( rand(), rand(), rand(), rand() );
  QgsTerrainTextureImage *billboardTextureImage = new QgsTerrainTextureImage( image, randomExtent, QStringLiteral( "billboard material." ) );

  setTexture2DFromTextureImage( billboardTextureImage );
  setSize( QSizeF( size + size, size + size ) );
}

void QgsPoint3DBillboardMaterial::useDefaultSymbol( const Qgs3DMapSettings &map, bool selected )
{
  // Default texture
  const std::unique_ptr< QgsMarkerSymbol> defaultSymbol( static_cast<QgsMarkerSymbol *>( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) ) );
  setTexture2DFromSymbol( defaultSymbol.get(), map, selected );
}

void QgsPoint3DBillboardMaterial::setTexture2DFromSymbol( QgsMarkerSymbol *markerSymbol, const Qgs3DMapSettings &map, bool selected )
{
  QgsRenderContext context;
  context.setSelectionColor( map.selectionColor() );
  context.setScaleFactor( map.outputDpi() / 25.4 );
  const double pixelSize = context.convertToPainterUnits( markerSymbol->size( context ),  markerSymbol->sizeUnit() );

  // This number is an max estimation ratio between stroke width and symbol size.
  const double strokeRatio = 0.5;
  // Minimum extra width, just in case the size is small, but the stroke is quite big.
  // 10 mm is quite big based on Raymond's experiece.
  // 10 mm has around 37 pixel in 96 dpi, round up become 40.
  const double minimumExtraSize = 40;
  const double extraPixel = minimumExtraSize > pixelSize * strokeRatio ? minimumExtraSize : pixelSize * strokeRatio;
  const int pixelWithExtra = std::ceil( pixelSize + extraPixel );
  const QPixmap symbolPixmap = QgsSymbolLayerUtils::symbolPreviewPixmap( markerSymbol, QSize( pixelWithExtra, pixelWithExtra ), 0, &context, selected );
  const QImage symbolImage = symbolPixmap.toImage();
  const QImage flippedSymbolImage = symbolImage.mirrored();
  setTexture2DFromImage( flippedSymbolImage, pixelWithExtra );
}

void QgsPoint3DBillboardMaterial::setTexture2DFromTextureImage( Qt3DRender::QAbstractTextureImage *textureImage )
{
  // Texture2D
  Qt3DRender::QTexture2D *texture2D = new Qt3DRender::QTexture2D( this );
  texture2D->setGenerateMipMaps( false );
  texture2D->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  texture2D->setMinificationFilter( Qt3DRender::QTexture2D::Linear );

  // The textureImage gets parented to texture2D here
  texture2D->addTextureImage( textureImage );

  mTexture2D->setValue( QVariant::fromValue( texture2D ) );
}
