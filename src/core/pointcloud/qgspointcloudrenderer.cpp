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

#include <QTime>

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


  QList<IndexedPointCloudNode> lvl1 = pc->children( IndexedPointCloudNode( 0, 0, 0, 0 ) );
  // qDebug() << lvl1;
  Q_ASSERT( lvl1.count() == 4 );

  QgsVector3D scale;
  QgsVector3D offset;
  QgsPointCloudDataBounds db;

  //dumpHierarchy( pc );
  IndexedPointCloudNode n0( 0, 0, 0, 0 );
  IndexedPointCloudNode n1( 1, 0, 0, 0 );
  IndexedPointCloudNode n2( 2, 2, 2, 0 );
  IndexedPointCloudNode n4( 4, 6, 7, 1 );
  IndexedPointCloudNode n5( 5, 14, 14, 3 );
  QVector<qint32> nodeData = pc->nodePositionDataAsInt32( n1, scale, offset, db );

  imgW = 256;
  imgH = imgW;
  img = QImage( imgW, imgH, QImage::Format_RGB32 );
  img.fill( Qt::black );
  mBounds = QgsPointCloudDataBounds( db );

  QTime t;
  t.start();

  // TODO: traverse with spatial filter

  QList<IndexedPointCloudNode> nodes = _traverseTree( pc, n1, 2 );
  // qDebug() << nodes;

  // drawing
  for ( auto n : nodes )
  {
    //drawImg.drawData(  );
    drawData( pc->nodePositionDataAsInt32( n, scale, offset, db ) );
  }

  qDebug() << "grand total incl loading " << t.elapsed() << "ms";

  qDebug() << "totals:" << nodesDrawn << "nodes | " << pointsDrawn << " points | " << drawingTime << "ms";

  //drawData( drawImg, nodeData );

  // img.save( "/tmp/chmura.png" );

  painter->restore();
  painter->drawImage( 0, 0, img, imgW, imgH );

  return true;
}


QgsPointCloudRenderer::~QgsPointCloudRenderer() = default;


void QgsPointCloudRenderer::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{

}

void QgsPointCloudRenderer::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{

}

void QgsPointCloudRenderer::drawData( const QVector<qint32> &data )
{
  QTime tD;
  tD.start();
  QRgb *rgb = ( QRgb * ) img.bits();
  const qint32 *ptr = data.constData();
  int count = data.count() / 3;
  for ( int i = 0; i < count; ++i )
  {
    qint32 ix = ptr[i * 3 + 0];
    qint32 iy = ptr[i * 3 + 1];
    qint32 iz = ptr[i * 3 + 2];
    int imgx = round( double( ix - mBounds.xMin() ) / ( mBounds.xMax() - mBounds.xMin() ) * ( imgW - 1 ) );
    int imgy = round( double( iy - mBounds.yMin() ) / ( mBounds.yMax() - mBounds.yMin() ) * ( imgH - 1 ) );
    int imgz = round( double( iz - mBounds.zMin() ) / ( mBounds.zMax() - mBounds.zMin() ) * 155 );
    int c = 100 + imgz;
    imgy = imgH - imgy - 1;  // Y in QImage is 0 at the top an increases towards bottom - need to invert it
    if ( imgx >= 0 && imgx < imgW && imgy >= 0 && imgy < imgH )
      rgb[imgx + imgy * imgW] = qRgb( c, c, c );
    //qDebug() << imgx << imgy << imgz;
    //drawImg.img.setPixelColor( imgx, imgy, QColor( c, c, c ) ); // Qt::yellow );
  }

  // stats
  ++nodesDrawn;
  pointsDrawn += count;
  int timeDraw = tD.elapsed();
  drawingTime += timeDraw;
  qDebug() << "time draw" << timeDraw << "ms";
}
