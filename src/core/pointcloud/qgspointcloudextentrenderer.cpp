/***************************************************************************
                         qgspointcloudextentrenderer.h
                         --------------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudextentrenderer.h"
#include "qgspointcloudblock.h"
#include "qgssymbollayerutils.h"
#include "qgssymbol.h"
#include "qgswkbtypes.h"
#include "qgspolygon.h"
#include "qgscurve.h"
#include "qgslinesymbollayer.h"
#include "qgslayertreemodellegendnode.h"
#include "qgsfillsymbol.h"

QgsPointCloudExtentRenderer::QgsPointCloudExtentRenderer( QgsFillSymbol *symbol )
  : mFillSymbol( symbol ? symbol : defaultFillSymbol() )
{

}

QgsPointCloudExtentRenderer::~QgsPointCloudExtentRenderer() = default;

QString QgsPointCloudExtentRenderer::type() const
{
  return QStringLiteral( "extent" );
}

QgsPointCloudRenderer *QgsPointCloudExtentRenderer::clone() const
{
  std::unique_ptr< QgsPointCloudExtentRenderer > res = std::make_unique< QgsPointCloudExtentRenderer >( mFillSymbol ? mFillSymbol->clone() : nullptr );
  copyCommonProperties( res.get() );
  return res.release();
}

void QgsPointCloudExtentRenderer::renderBlock( const QgsPointCloudBlock *, QgsPointCloudRenderContext & )
{

}

QgsPointCloudRenderer *QgsPointCloudExtentRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  std::unique_ptr< QgsPointCloudExtentRenderer > r = std::make_unique< QgsPointCloudExtentRenderer >();

  const QDomElement symbolElem = element.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !symbolElem.isNull() )
  {
    r->mFillSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( symbolElem, context ) );
  }

  r->restoreCommonProperties( element, context );
  return r.release();
}

void QgsPointCloudExtentRenderer::renderExtent( const QgsGeometry &extent, QgsPointCloudRenderContext &context )
{
  auto transformRing = [&context]( QPolygonF & pts )
  {
    //transform the QPolygonF to screen coordinates
    if ( context.renderContext().coordinateTransform().isValid() )
    {
      try
      {
        context.renderContext().coordinateTransform().transformPolygon( pts );
      }
      catch ( QgsCsException & )
      {
        // we don't abort the rendering here, instead we remove any invalid points and just plot those which ARE valid
      }
    }

    // remove non-finite points, e.g. infinite or NaN points caused by reprojecting errors
    pts.erase( std::remove_if( pts.begin(), pts.end(),
                               []( const QPointF point )
    {
      return !std::isfinite( point.x() ) || !std::isfinite( point.y() );
    } ), pts.end() );

    QPointF *ptr = pts.data();
    for ( int i = 0; i < pts.size(); ++i, ++ptr )
    {
      context.renderContext().mapToPixel().transformInPlace( ptr->rx(), ptr->ry() );
    }
  };

  for ( auto it = extent.const_parts_begin(); it != extent.const_parts_end(); ++it )
  {
    if ( const QgsPolygon *polygon = qgsgeometry_cast< const QgsPolygon * >( *it ) )
    {
      QPolygonF exterior = polygon->exteriorRing()->asQPolygonF();
      transformRing( exterior );
      QVector<QPolygonF> rings;
      rings.reserve( polygon->numInteriorRings() );
      for ( int i = 0; i < polygon->numInteriorRings(); ++i )
      {
        QPolygonF ring = polygon->interiorRing( i )->asQPolygonF();
        transformRing( ring );
        rings.append( ring );
      }

      mFillSymbol->renderPolygon( exterior, rings.empty() ? nullptr : &rings, nullptr, context.renderContext() );
    }
  }
}

QgsFillSymbol *QgsPointCloudExtentRenderer::defaultFillSymbol()
{
  std::unique_ptr< QgsSimpleLineSymbolLayer > layer = std::make_unique< QgsSimpleLineSymbolLayer >();
  layer->setColor( QColor( 228, 26, 28 ) );
  layer->setWidth( 0.960000 );
  layer->setPenStyle( Qt::DotLine );
  layer->setWidthUnit( QgsUnitTypes::RenderMillimeters );
  return new QgsFillSymbol( QgsSymbolLayerList() << layer.release() );
}

QgsFillSymbol *QgsPointCloudExtentRenderer::fillSymbol() const
{
  return mFillSymbol.get();
}

void QgsPointCloudExtentRenderer::setFillSymbol( QgsFillSymbol *symbol )
{
  mFillSymbol.reset( symbol );
}

QDomElement QgsPointCloudExtentRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement rendererElem = doc.createElement( QStringLiteral( "renderer" ) );

  rendererElem.setAttribute( QStringLiteral( "type" ), type() );

  const QDomElement symbolElem = QgsSymbolLayerUtils::saveSymbol( QString(), mFillSymbol.get(), doc, context );
  rendererElem.appendChild( symbolElem );

  saveCommonProperties( rendererElem, context );
  return rendererElem;
}

void QgsPointCloudExtentRenderer::startRender( QgsPointCloudRenderContext &context )
{
  QgsPointCloudRenderer::startRender( context );
  mFillSymbol->startRender( context.renderContext() );
}

void QgsPointCloudExtentRenderer::stopRender( QgsPointCloudRenderContext &context )
{
  QgsPointCloudRenderer::stopRender( context );
  mFillSymbol->stopRender( context.renderContext() );
}

QList<QgsLayerTreeModelLegendNode *> QgsPointCloudExtentRenderer::createLegendNodes( QgsLayerTreeLayer *nodeLayer )
{
  QList<QgsLayerTreeModelLegendNode *> nodes;

  const QgsLegendSymbolItem extentItem( mFillSymbol.get(), QStringLiteral( "extent" ), QStringLiteral( "extent" ) );
  QgsSymbolLegendNode *node = new QgsSymbolLegendNode( nodeLayer, extentItem );
  node->setEmbeddedInParent( true );
  nodes << node;

  return nodes;
}
