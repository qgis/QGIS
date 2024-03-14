/***************************************************************************
  qgsrastercontourrenderer.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrastercontourrenderer.h"

#include "qgsreadwritecontext.h"
#include "qgssymbollayerutils.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslinesymbol.h"

#include <gdal_alg.h>

QgsRasterContourRenderer::QgsRasterContourRenderer( QgsRasterInterface *input )
  : QgsRasterRenderer( input, QStringLiteral( "contour" ) )
{
  mContourSymbol.reset( static_cast<QgsLineSymbol *>( QgsLineSymbol::defaultSymbol( Qgis::GeometryType::Line ) ) );
}

QgsRasterContourRenderer::~QgsRasterContourRenderer() = default;

QgsRasterContourRenderer *QgsRasterContourRenderer::clone() const
{
  QgsRasterContourRenderer *renderer = new QgsRasterContourRenderer( nullptr );
  renderer->copyCommonProperties( this );
  renderer->mContourSymbol.reset( mContourSymbol ? mContourSymbol->clone() : nullptr );
  renderer->mContourIndexSymbol.reset( mContourIndexSymbol ? mContourIndexSymbol->clone() : nullptr );
  renderer->mContourInterval = mContourInterval;
  renderer->mContourIndexInterval = mContourIndexInterval;
  renderer->mInputBand = mInputBand;
  renderer->mDownscale = mDownscale;
  return renderer;
}

QgsRasterRenderer *QgsRasterContourRenderer::create( const QDomElement &elem, QgsRasterInterface *input )
{
  if ( elem.isNull() )
  {
    return nullptr;
  }

  QgsRasterContourRenderer *r = new QgsRasterContourRenderer( input );
  r->readXml( elem );

  const int inputBand = elem.attribute( QStringLiteral( "band" ), QStringLiteral( "-1" ) ).toInt();
  const double contourInterval = elem.attribute( QStringLiteral( "contour-interval" ), QStringLiteral( "100" ) ).toDouble();
  const double contourIndexInterval = elem.attribute( QStringLiteral( "contour-index-interval" ), QStringLiteral( "0" ) ).toDouble();
  const double downscale = elem.attribute( QStringLiteral( "downscale" ), QStringLiteral( "4" ) ).toDouble();

  r->setInputBand( inputBand );
  r->setContourInterval( contourInterval );
  r->setContourIndexInterval( contourIndexInterval );
  r->setDownscale( downscale );

  QDomElement symbolsElem = elem.firstChildElement( QStringLiteral( "symbols" ) );
  if ( !symbolsElem.isNull() )
  {
    QgsSymbolMap symbolMap = QgsSymbolLayerUtils::loadSymbols( symbolsElem, QgsReadWriteContext() );
    if ( symbolMap.contains( QStringLiteral( "contour" ) ) )
    {
      QgsSymbol *symbol = symbolMap.take( QStringLiteral( "contour" ) );
      if ( symbol->type() == Qgis::SymbolType::Line )
        r->setContourSymbol( static_cast<QgsLineSymbol *>( symbol ) );
    }
    if ( symbolMap.contains( QStringLiteral( "index-contour" ) ) )
    {
      QgsSymbol *symbol = symbolMap.take( QStringLiteral( "index-contour" ) );
      if ( symbol->type() == Qgis::SymbolType::Line )
        r->setContourIndexSymbol( static_cast<QgsLineSymbol *>( symbol ) );
    }
  }
  return r;
}

void QgsRasterContourRenderer::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( QStringLiteral( "rasterrenderer" ) );
  _writeXml( doc, rasterRendererElem );

  rasterRendererElem.setAttribute( QStringLiteral( "band" ), mInputBand );
  rasterRendererElem.setAttribute( QStringLiteral( "contour-interval" ), mContourInterval );
  rasterRendererElem.setAttribute( QStringLiteral( "contour-index-interval" ), mContourIndexInterval );
  rasterRendererElem.setAttribute( QStringLiteral( "downscale" ), mDownscale );

  QgsSymbolMap symbols;
  symbols[QStringLiteral( "contour" )] = mContourSymbol.get();
  if ( mContourIndexSymbol )
    symbols[QStringLiteral( "index-contour" )] = mContourIndexSymbol.get();
  const QDomElement symbolsElem = QgsSymbolLayerUtils::saveSymbols( symbols, QStringLiteral( "symbols" ), doc, QgsReadWriteContext() );
  rasterRendererElem.appendChild( symbolsElem );

  parentElem.appendChild( rasterRendererElem );
}

struct ContourWriterData
{
  QPainter *painter;
  double scaleX, scaleY;
  QgsLineSymbol *symbol;
  QgsLineSymbol *indexSymbol;
  double indexInterval;
  QgsRenderContext *context;
};

CPLErr _rasterContourWriter( double dfLevel, int nPoints, double *padfX, double *padfY, void *ptr )
{
  Q_UNUSED( dfLevel )
  ContourWriterData *crData = static_cast<ContourWriterData *>( ptr );
  QPolygonF polygon( nPoints );
  QPointF *d = polygon.data();
  for ( int i = 0; i < nPoints; ++i )
  {
    d[i] = QPointF( padfX[i] * crData->scaleX, padfY[i] * crData->scaleY );
  }

  if ( crData->indexSymbol && !qgsDoubleNear( crData->indexInterval, 0 ) && qgsDoubleNear( fmod( dfLevel, crData->indexInterval ), 0 ) )
    crData->indexSymbol->renderPolyline( polygon, nullptr, *crData->context );
  else
    crData->symbol->renderPolyline( polygon, nullptr, *crData->context );
  return CE_None;
}

QgsRasterBlock *QgsRasterContourRenderer::block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( bandNo )

  std::unique_ptr< QgsRasterBlock > outputBlock( new QgsRasterBlock() );
  if ( !mInput || !mContourSymbol )
  {
    return outputBlock.release();
  }

  const int inputWidth = static_cast<int>( round( width / mDownscale ) );
  const int inputHeight = static_cast<int>( round( height / mDownscale ) );

  std::unique_ptr< QgsRasterBlock > inputBlock( mInput->block( mInputBand, extent, inputWidth, inputHeight, feedback ) );
  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugError( QStringLiteral( "No raster data!" ) );
    return outputBlock.release();
  }

  if ( !inputBlock->convert( Qgis::DataType::Float64 ) ) // contouring algorithm requires double
    return outputBlock.release();
  double *scanline = reinterpret_cast<double *>( inputBlock->bits() );

  QImage img( width, height, QImage::Format_ARGB32_Premultiplied );
  img.fill( Qt::transparent );

  QPainter p( &img );
  p.setRenderHint( QPainter::Antialiasing );

  QgsRenderContext context = QgsRenderContext::fromQPainter( &p );

  ContourWriterData crData;
  crData.painter = &p;
  crData.scaleX = width / double( inputWidth );
  crData.scaleY = height / double( inputHeight );
  crData.symbol = mContourSymbol.get();
  crData.indexSymbol = mContourIndexSymbol.get();
  crData.indexInterval = mContourIndexInterval;
  crData.context = &context;

  crData.symbol->startRender( context );
  if ( crData.indexSymbol )
    crData.indexSymbol->startRender( context );

  const double contourBase = 0.;
  GDALContourGeneratorH cg = GDAL_CG_Create( inputBlock->width(), inputBlock->height(),
                             inputBlock->hasNoDataValue(), inputBlock->noDataValue(),
                             mContourInterval, contourBase,
                             _rasterContourWriter, static_cast<void *>( &crData ) );
  for ( int i = 0; i < inputHeight; ++i )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    GDAL_CG_FeedLine( cg, scanline );
    scanline += inputWidth;
  }
  GDAL_CG_Destroy( cg );

  crData.symbol->stopRender( context );
  if ( crData.indexSymbol )
    crData.indexSymbol->stopRender( context );

  p.end();

  outputBlock->setImage( &img );
  return outputBlock.release();
}

QList<int> QgsRasterContourRenderer::usesBands() const
{
  QList<int> bandList;
  if ( mInputBand != -1 )
  {
    bandList << mInputBand;
  }
  return bandList;
}

QList<QgsLayerTreeModelLegendNode *> QgsRasterContourRenderer::createLegendNodes( QgsLayerTreeLayer *nodeLayer )
{
  QList<QgsLayerTreeModelLegendNode *> nodes;

  const QgsLegendSymbolItem contourItem( mContourSymbol.get(), QString::number( mContourInterval ), QStringLiteral( "contour" ) );
  nodes << new QgsSymbolLegendNode( nodeLayer, contourItem );

  if ( mContourIndexInterval > 0 )
  {
    const QgsLegendSymbolItem indexItem( mContourIndexSymbol.get(), QString::number( mContourIndexInterval ), QStringLiteral( "index" ) );
    nodes << new QgsSymbolLegendNode( nodeLayer, indexItem );
  }

  return nodes;
}

int QgsRasterContourRenderer::inputBand() const
{
  return mInputBand;
}

bool QgsRasterContourRenderer::setInputBand( int band )
{
  if ( !mInput )
  {
    mInputBand = band;
    return true;
  }
  else if ( band > 0 && band <= mInput->bandCount() )
  {
    mInputBand = band;
    return true;
  }
  return false;
}

void QgsRasterContourRenderer::setContourSymbol( QgsLineSymbol *symbol )
{
  mContourSymbol.reset( symbol );
}

void QgsRasterContourRenderer::setContourIndexSymbol( QgsLineSymbol *symbol )
{
  mContourIndexSymbol.reset( symbol );
}
