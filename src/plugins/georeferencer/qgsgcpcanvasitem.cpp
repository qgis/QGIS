/***************************************************************************
     qgsgcpcanvasitem.h
     --------------------------------------
    Date                 : 14-Feb-2010
    Copyright            : (C) 2010 by Jack R, Maxim Dubinin (GIS-Lab)
    Email                : sim@gis-lab.info
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsgcpcanvasitem.h"
#include "qgsgeorefdatapoint.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsrasterlayer.h"

QgsGCPCanvasItem::QgsGCPCanvasItem( QgsMapCanvas* mapCanvas, const QgsGeorefDataPoint* dataPoint, bool isGCPSource )
    : QgsMapCanvasItem( mapCanvas ), mDataPoint( dataPoint )
    , mPointBrush( Qt::red )
    , mLabelBrush( Qt::yellow )
    , mIsGCPSource( isGCPSource )
{
  setFlags( QGraphicsItem::ItemIsMovable );
  mResidualPen.setColor( QColor( 255, 0, 0 ) );
  mResidualPen.setWidthF( 2.0 );

  updatePosition();
}

void QgsGCPCanvasItem::paint( QPainter* p )
{
  p->setRenderHint( QPainter::Antialiasing );

  bool enabled = true;
  QgsPoint worldCoords;
  int id = -1;

  if ( mDataPoint )
  {
    enabled = mDataPoint->isEnabled();
    worldCoords = mDataPoint->mapCoords();
    id = mDataPoint->id();
  }

  p->setOpacity( enabled ? 1.0 : 0.3 );

  // draw the point
  p->setPen( Qt::black );
  p->setBrush( mPointBrush );
  p->drawEllipse( -2, -2, 5, 5 );

  QSettings s;
  bool showIDs = s.value( "/Plugin-GeoReferencer/Config/ShowId" ).toBool();
  if ( !showIDs && mIsGCPSource )
  {
    QString msg = QString( "X %1\nY %2" ).arg( QString::number( worldCoords.x(), 'f' ) ).
                  arg( QString::number( worldCoords.y(), 'f' ) );
    p->setFont( QFont( "helvetica", 9 ) );
    QRect textBounds = p->boundingRect( 6, 6, 10, 10, Qt::AlignLeft, msg );
    p->setBrush( mLabelBrush );
    p->drawRect( textBounds.x() - 2, textBounds.y() - 2, textBounds.width() + 4, textBounds.height() + 4 );
    p->drawText( textBounds, Qt::AlignLeft, msg );
    mTextBounds = QSizeF( textBounds.width() + 4, textBounds.height() + 4 );
  }
  else if ( showIDs )
  {
    p->setFont( QFont( "helvetica", 12 ) );
    QString msg = QString::number( id );
    p->setBrush( mLabelBrush );
    p->drawRect( 5, 4, p->fontMetrics().width( msg ) + 2, 14 );
    p->drawText( 6, 16, msg );
    QFontMetrics fm = p->fontMetrics();
    mTextBounds = QSize( fm.width( msg ) + 4, fm.height() + 4 );
  }

  drawResidualArrow( p );
}

QRectF QgsGCPCanvasItem::boundingRect() const
{
  double residualLeft, residualRight, residualTop, residualBottom;

  QPointF residual;
  if ( mDataPoint )
  {
    residual = mDataPoint->residual();
  }
  double rf = residualToScreenFactor();

  if ( residual.x() > 0 )
  {
    residualRight = residual.x() * rf + mResidualPen.widthF();
    residualLeft = -mResidualPen.widthF();
  }
  else
  {
    residualLeft = residual.x() * rf - mResidualPen.widthF();
    residualRight = mResidualPen.widthF();
  }
  if ( residual.y() > 0 )
  {
    residualBottom = residual.y() * rf + mResidualPen.widthF();
    residualTop = -mResidualPen.widthF();
  }
  else
  {
    residualBottom = mResidualPen.widthF();
    residualTop = residual.y() * rf - mResidualPen.widthF();
  }

  QRectF residualArrowRect( QPointF( residualLeft, residualTop ), QPointF( residualRight, residualBottom ) );
  QRectF markerRect( -2, -2, mTextBounds.width() + 6, mTextBounds.height() + 6 );
  return residualArrowRect.united( markerRect );
}

QPainterPath QgsGCPCanvasItem::shape() const
{
  QPainterPath p;
  p.addEllipse( -2, -2, 5, 5 );
  p.addRect( 6, 6, mTextBounds.width(), mTextBounds.height() );

  return p;
}

void QgsGCPCanvasItem::updatePosition()
{
  if ( !mDataPoint )
  {
    return;
  }

  setPos( toCanvasCoordinates( mIsGCPSource ? mDataPoint->pixelCoords() : mDataPoint->mapCoords() ) );
}

void QgsGCPCanvasItem::drawResidualArrow( QPainter* p )
{
  if ( !mDataPoint || !mIsGCPSource )
  {
    return;
  }

  QPointF residual = mDataPoint->residual();

  double rf = residualToScreenFactor();
  p->setPen( mResidualPen );
  p->drawLine( QPointF( 0, 0 ), QPointF( residual.rx() * rf, residual.ry() * rf ) );

}

double QgsGCPCanvasItem::residualToScreenFactor() const
{
  if ( !mMapCanvas )
  {
    return 1;
  }

  double mapUnitsPerScreenPixel = mMapCanvas->mapUnitsPerPixel();
  double mapUnitsPerRasterPixel = 1.0;

  if ( mMapCanvas->mapRenderer() )
  {
    QStringList canvasLayers = mMapCanvas->mapRenderer()->layerSet();
    if ( canvasLayers.size() > 0 )
    {
      QString layerId = canvasLayers.at( 0 );
      QgsMapLayer* mapLayer = QgsMapLayerRegistry::instance()->mapLayer( layerId );
      if ( mapLayer )
      {
        QgsRasterLayer* rasterLayer = dynamic_cast<QgsRasterLayer*>( mapLayer );
        if ( rasterLayer )
        {
          mapUnitsPerRasterPixel = rasterLayer->rasterUnitsPerPixel();
        }
      }
    }
  }

  return 1.0 / ( mapUnitsPerScreenPixel * mapUnitsPerRasterPixel );
}

void QgsGCPCanvasItem::checkBoundingRectChange()
{
  prepareGeometryChange();
}
