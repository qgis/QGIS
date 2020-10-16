/***************************************************************************
                         qgspointcloudrenderer.cpp
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QElapsedTimer>

#include "qgspointcloudrenderer.h"
#include "qgspointcloudlayer.h"
#include "qgsrendercontext.h"
#include "qgspointcloudindex.h"

QgsPointCloudRenderer::QgsPointCloudRenderer( QgsPointCloudLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context )
  , mLayer( layer )
{

}

static QList<IndexedPointCloudNode> _traverseTree( QgsPointCloudIndex *pc, IndexedPointCloudNode n, int maxDepth )
{
  QList<IndexedPointCloudNode> nodes;
  nodes.append( n );

  for ( auto nn : pc->children( n ) )
  {
    if ( maxDepth > 0 )
      nodes += _traverseTree( pc, nn, maxDepth - 1 );
  }

  return nodes;
}

bool QgsPointCloudRenderer::render()
{
  // TODO cache!?
  QgsPointCloudIndex *pc = mLayer->pointCloudIndex();

  QgsRenderContext &context = *renderContext();

  // Set up the render configuration options
  QPainter *painter = context.painter();

  painter->save();
  context.setPainterFlagsUsingContext( painter );

  QPen pen = painter->pen();
  pen.setCapStyle( Qt::FlatCap );
  pen.setJoinStyle( Qt::MiterJoin );

  double penWidth = context.convertToPainterUnits( 1, QgsUnitTypes::RenderUnit::RenderMillimeters );
  pen.setWidthF( penWidth );
  pen.setColor( Qt::black );
  painter->setPen( pen );

  QgsPointCloudDataBounds db;

  QElapsedTimer t;
  t.start();

  // TODO: traverse with spatial filter
  QList<IndexedPointCloudNode> nodes = _traverseTree( pc, pc->root(), 4 );

  // drawing
  for ( auto n : nodes )
  {
    drawData( painter, pc->nodePositionDataAsInt32( n ) );
  }

  qDebug() << "totals:" << nodesDrawn << "nodes | " << pointsDrawn << " points | " << t.elapsed() << "ms";

  painter->restore();

  return true;
}


QgsPointCloudRenderer::~QgsPointCloudRenderer() = default;


void QgsPointCloudRenderer::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{

}

void QgsPointCloudRenderer::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{

}

void QgsPointCloudRenderer::drawData( QPainter *painter, const QVector<qint32> &data )
{
  const QgsMapToPixel mapToPixel = renderContext()->mapToPixel();
  const QgsVector3D scale = mLayer->pointCloudIndex()->scale();
  const QgsVector3D offset = mLayer->pointCloudIndex()->offset();

  const qint32 *ptr = data.constData();
  int count = data.count() / 3;
  for ( int i = 0; i < count; ++i )
  {
    qint32 ix = ptr[i * 3 + 0];
    qint32 iy = ptr[i * 3 + 1];
    // qint32 iz = ptr[i * 3 + 2];

    double x = offset.x() + scale.x() * ix;
    double y = offset.y() + scale.y() * iy;
    // double z = offset.z() + scale.z() * iz;
    mapToPixel.transformInPlace( x, y );

    painter->drawPoint( QPointF( x, y ) );
  }

  // stats
  ++nodesDrawn;
  pointsDrawn += count;
}
