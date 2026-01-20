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
#include "qgspoint3dbillboardmaterial.h"

#include "qgs3drendercontext.h"
#include "qgs3dutils.h"
#include "qgsimagetexture.h"
#include "qgsmarkersymbol.h"
#include "qgssymbollayerutils.h"

#include <QUrl>
#include <Qt3DRender/QBlendEquation>
#include <Qt3DRender/QBlendEquationArguments>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QNoDepthMask>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QTechnique>

#include "moc_qgspoint3dbillboardmaterial.cpp"

QgsPoint3DBillboardMaterial::QgsPoint3DBillboardMaterial( Mode mode )
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

  const QUrl urlVert( u"qrc:/shaders/billboards.vert"_s );
  const QUrl urlGeom( u"qrc:/shaders/billboards.geom"_s );

  switch ( mode )
  {
    case Mode::SingleTexture:
    {
      shaderProgram->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( urlVert ) );
      shaderProgram->setGeometryShaderCode( Qt3DRender::QShaderProgram::loadSource( urlGeom ) );
      break;
    }
    case Mode::AtlasTexture:
    {
      const QByteArray vertexShaderCode = Qt3DRender::QShaderProgram::loadSource( urlVert );
      const QByteArray finalVertexShaderCode = Qgs3DUtils::addDefinesToShaderCode( vertexShaderCode, QStringList( { "TEXTURE_ATLAS" } ) );
      shaderProgram->setVertexShaderCode( finalVertexShaderCode );

      const QByteArray geomShaderCode = Qt3DRender::QShaderProgram::loadSource( urlGeom );
      const QByteArray finalGeomShaderCode = Qgs3DUtils::addDefinesToShaderCode( geomShaderCode, QStringList( { "TEXTURE_ATLAS" } ) );
      shaderProgram->setGeometryShaderCode( finalGeomShaderCode );
      break;
    }
    case Mode::AtlasTextureWithPixelOffsets:
    {
      const QByteArray vertexShaderCode = Qt3DRender::QShaderProgram::loadSource( urlVert );
      const QByteArray finalVertexShaderCode = Qgs3DUtils::addDefinesToShaderCode( vertexShaderCode, QStringList( { "TEXTURE_ATLAS", "TEXTURE_ATLAS_PIXEL_OFFSETS" } ) );
      shaderProgram->setVertexShaderCode( finalVertexShaderCode );

      const QByteArray geomShaderCode = Qt3DRender::QShaderProgram::loadSource( urlGeom );
      const QByteArray finalGeomShaderCode = Qgs3DUtils::addDefinesToShaderCode( geomShaderCode, QStringList( { "TEXTURE_ATLAS", "TEXTURE_ATLAS_PIXEL_OFFSETS" } ) );
      shaderProgram->setGeometryShaderCode( finalGeomShaderCode );
      break;
    }
  }
  shaderProgram->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/billboards.frag"_s ) ) );

  // Render Pass
  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass( this );
  renderPass->setShaderProgram( shaderProgram );
  renderPass->addRenderState( blendState );
  renderPass->addRenderState( blendEquation );

  // without this filter the default forward renderer would not render this
  Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey;
  filterKey->setName( u"renderingStyle"_s );
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

QgsPoint3DBillboardMaterial::~QgsPoint3DBillboardMaterial() = default;

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

void QgsPoint3DBillboardMaterial::setTexture2DFromImage( const QImage &image )
{
  // Create texture image
  QgsImageTexture *textureImage = new QgsImageTexture( image );
  setTexture2DFromTextureImage( textureImage );

  setSize( QSizeF( image.size().width(), image.size().height() ) );
}

void QgsPoint3DBillboardMaterial::useDefaultSymbol( const Qgs3DRenderContext &context, bool selected )
{
  // Default texture
  const std::unique_ptr<QgsMarkerSymbol> defaultSymbol( static_cast<QgsMarkerSymbol *>( QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ) ) );
  setTexture2DFromSymbol( defaultSymbol.get(), context, selected );
}

QImage QgsPoint3DBillboardMaterial::renderSymbolToImage( const QgsMarkerSymbol *markerSymbol, const Qgs3DRenderContext &context, bool selected )
{
  QgsRenderContext context2D;
  context2D.setSelectionColor( context.selectionColor() );
  context2D.setScaleFactor( context.outputDpi() / 25.4 );
  context2D.setFlag( Qgis::RenderContextFlag::Antialiasing );
  context2D.setFlag( Qgis::RenderContextFlag::HighQualityImageTransforms );

  std::unique_ptr< QgsMarkerSymbol > clonedSymbol( markerSymbol->clone() );
  clonedSymbol->startRender( context2D );

  constexpr int BUFFER_SIZE_PIXELS = 2;

  const QRectF bounds = markerSymbol->bounds( QPointF( 0, 0 ), context2D );

  QImage image( static_cast< int >( std::ceil( bounds.size().width() ) ) + 2 * BUFFER_SIZE_PIXELS, static_cast< int >( std::ceil( bounds.size().height() ) ) + 2 * BUFFER_SIZE_PIXELS, QImage::Format_ARGB32_Premultiplied );
  image.fill( Qt::transparent );

  QPainter painter( &image );
  context2D.setPainter( &painter );

  clonedSymbol->renderPoint( QPointF( -bounds.left() + BUFFER_SIZE_PIXELS, -bounds.top() + BUFFER_SIZE_PIXELS ), nullptr, context2D, -1, selected );

  painter.end();

  clonedSymbol->stopRender( context2D );
  return image;
}

void QgsPoint3DBillboardMaterial::setTexture2DFromSymbol( const QgsMarkerSymbol *markerSymbol, const Qgs3DRenderContext &context, bool selected )
{
  const QImage symbolImage = renderSymbolToImage( markerSymbol, context, selected );
  setTexture2DFromImage( symbolImage );
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
