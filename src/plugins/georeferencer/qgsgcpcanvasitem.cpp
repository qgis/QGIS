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
  QgsRenderContext context;
  if ( !setRenderContextVariables( p, context ) )
  {
    return;
  }

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
  bool showCoords = s.value( "/Plugin-GeoReferencer/Config/ShowCoords" ).toBool();

  QString msg;
  if ( showIDs && showCoords )
  {
    msg = QString( "%1\nX %2\nY %3" ).arg( QString::number( id ) ).arg( QString::number( worldCoords.x(), 'f' ) ).arg( QString::number( worldCoords.y(), 'f' ) );
  }
  else if ( showIDs )
  {
    msg = msg = QString::number( id );
  }
  else if ( showCoords )
  {
    msg = QString( "X %1\nY %2" ).arg( QString::number( worldCoords.x(), 'f' ) ).arg( QString::number( worldCoords.y(), 'f' ) );
  }

  if ( !msg.isEmpty() )
  {
    p->setBrush( mLabelBrush );
    QFont textFont( "helvetica" );
    textFont.setPixelSize( fontSizePainterUnits( 12, context ) );
    p->setFont( textFont );
    QRectF textBounds = p->boundingRect( 3 * context.scaleFactor(), 3 * context.scaleFactor(), 5 * context.scaleFactor(), 5 * context.scaleFactor(), Qt::AlignLeft, msg );
    mTextBoxRect = QRectF( textBounds.x() - context.scaleFactor() * 1, textBounds.y() - context.scaleFactor() * 1, \
                           textBounds.width() + 2 * context.scaleFactor(), textBounds.height() + 2 * context.scaleFactor() );
    p->drawRect( mTextBoxRect );
    p->drawText( textBounds, Qt::AlignLeft, msg );
  }

  if ( data( 0 ) != "composer" ) //draw residuals only on screen
  {
    drawResidualArrow( p, context );
  }
}

QRectF QgsGCPCanvasItem::boundingRect() const
{
  double residualLeft, residualRight, residualTop, residualBottom;

  QPointF residual;
  if ( mDataPoint )
  {
    residual = mDataPoint->residual();
  }

  //only considering screen resolution is ok for the bounding box function
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
  QRectF boundingRect =  residualArrowRect.united( markerRect );
  if ( !mTextBoxRect.isNull() )
  {
    boundingRect = boundingRect.united( mTextBoxRect );
  }
  return boundingRect;
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

void QgsGCPCanvasItem::drawResidualArrow( QPainter* p, const QgsRenderContext& context )
{
  if ( !mDataPoint || !mIsGCPSource || !mMapCanvas )
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

double QgsGCPCanvasItem::fontSizePainterUnits( double points, const QgsRenderContext& c )
{
  return points * 0.3527 * c.scaleFactor();
}

