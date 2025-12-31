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

#include <gdal_alg.h>

#include "qgslayertreemodellegendnode.h"
#include "qgslinesymbol.h"
#include "qgsreadwritecontext.h"
#include "qgssymbollayerutils.h"

QgsRasterContourRenderer::QgsRasterContourRenderer( QgsRasterInterface *input )
  : QgsRasterRenderer( input, u"contour"_s )
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

Qgis::RasterRendererFlags QgsRasterContourRenderer::flags() const
{
  return Qgis::RasterRendererFlag::UseNoDataForOutOfRangePixels;
}

QgsRasterRenderer *QgsRasterContourRenderer::create( const QDomElement &elem, QgsRasterInterface *input )
{
  if ( elem.isNull() )
  {
    return nullptr;
  }

  QgsRasterContourRenderer *r = new QgsRasterContourRenderer( input );
  r->readXml( elem );

  const int inputBand = elem.attribute( u"band"_s, u"-1"_s ).toInt();
  const double contourInterval = elem.attribute( u"contour-interval"_s, u"100"_s ).toDouble();
  const double contourIndexInterval = elem.attribute( u"contour-index-interval"_s, u"0"_s ).toDouble();
  const double downscale = elem.attribute( u"downscale"_s, u"4"_s ).toDouble();

  r->setInputBand( inputBand );
  r->setContourInterval( contourInterval );
  r->setContourIndexInterval( contourIndexInterval );
  r->setDownscale( downscale );

  QDomElement symbolsElem = elem.firstChildElement( u"symbols"_s );
  if ( !symbolsElem.isNull() )
  {
    QgsSymbolMap symbolMap = QgsSymbolLayerUtils::loadSymbols( symbolsElem, QgsReadWriteContext() );
    if ( symbolMap.contains( u"contour"_s ) )
    {
      QgsSymbol *symbol = symbolMap.take( u"contour"_s );
      if ( symbol->type() == Qgis::SymbolType::Line )
        r->setContourSymbol( static_cast<QgsLineSymbol *>( symbol ) );
    }
    if ( symbolMap.contains( u"index-contour"_s ) )
    {
      QgsSymbol *symbol = symbolMap.take( u"index-contour"_s );
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

  QDomElement rasterRendererElem = doc.createElement( u"rasterrenderer"_s );
  _writeXml( doc, rasterRendererElem );

  rasterRendererElem.setAttribute( u"band"_s, mInputBand );
  rasterRendererElem.setAttribute( u"contour-interval"_s, mContourInterval );
  rasterRendererElem.setAttribute( u"contour-index-interval"_s, mContourIndexInterval );
  rasterRendererElem.setAttribute( u"downscale"_s, mDownscale );

  QgsSymbolMap symbols;
  symbols[u"contour"_s] = mContourSymbol.get();
  if ( mContourIndexSymbol )
    symbols[u"index-contour"_s] = mContourIndexSymbol.get();
  const QDomElement symbolsElem = QgsSymbolLayerUtils::saveSymbols( symbols, u"symbols"_s, doc, QgsReadWriteContext() );
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

  auto outputBlock = std::make_unique<QgsRasterBlock>();
  if ( !mInput || !mContourSymbol )
  {
    return outputBlock.release();
  }

  const int inputWidth = static_cast<int>( round( width / mDownscale ) );
  const int inputHeight = static_cast<int>( round( height / mDownscale ) );

  std::unique_ptr< QgsRasterBlock > inputBlock( mInput->block( mInputBand, extent, inputWidth, inputHeight, feedback ) );
  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugError( u"No raster data!"_s );
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

  const QgsLegendSymbolItem contourItem( mContourSymbol.get(), QString::number( mContourInterval ), u"contour"_s );
  nodes << new QgsSymbolLegendNode( nodeLayer, contourItem );

  if ( mContourIndexInterval > 0 )
  {
    const QgsLegendSymbolItem indexItem( mContourIndexSymbol.get(), QString::number( mContourIndexInterval ), u"index"_s );
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
