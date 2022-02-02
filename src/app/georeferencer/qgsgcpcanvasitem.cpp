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

#include "qgsgcpcanvasitem.h"
#include "qgsgeorefdatapoint.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgssettings.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsrendercontext.h"

QgsGCPCanvasItem::QgsGCPCanvasItem( QgsMapCanvas *mapCanvas, QgsGeorefDataPoint *dataPoint, bool isGCPSource )
  : QgsMapCanvasItem( mapCanvas )
  , mDataPoint( dataPoint )
  , mPointBrush( Qt::red )
  , mLabelBrush( Qt::yellow )
  , mIsGCPSource( isGCPSource )
{
  setFlags( QGraphicsItem::ItemIsMovable );
  mResidualPen.setColor( QColor( 255, 0, 0 ) );
  mResidualPen.setWidthF( 2.0 );

  updatePosition();
}

void QgsGCPCanvasItem::paint( QPainter *p )
{
  QgsRenderContext context;
  if ( !setRenderContextVariables( p, context ) )
  {
    return;
  }

  p->setRenderHint( QPainter::Antialiasing );

  bool enabled = true;
  QgsPointXY worldCoords;
  int id = -1;
  const QgsCoordinateReferenceSystem mapCrs = mMapCanvas->mapSettings().destinationCrs();

  if ( mDataPoint )
  {
    enabled = mDataPoint->isEnabled();
    worldCoords = mDataPoint->destinationMapCoords();
    id = mDataPoint->id();
  }
  p->setOpacity( enabled ? 1.0 : 0.3 );

  // draw the point
  p->setPen( Qt::black );
  p->setBrush( mPointBrush );
  p->drawEllipse( -2, -2, 5, 5 );

  const QgsSettings s;
  const bool showIDs = s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/ShowId" ) ).toBool();
  const bool showCoords = s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/ShowCoords" ) ).toBool();

  QString msg;
  if ( showIDs && showCoords )
  {
    msg = QStringLiteral( "%1\nX %2\nY %3" ).arg( QString::number( id ), QString::number( worldCoords.x(), 'f' ), QString::number( worldCoords.y(), 'f' ) );
  }
  else if ( showIDs )
  {
    msg = QString::number( id );
  }
  else if ( showCoords )
  {
    msg = QStringLiteral( "X %1\nY %2" ).arg( QString::number( worldCoords.x(), 'f' ), QString::number( worldCoords.y(), 'f' ) );
  }

  if ( !msg.isEmpty() )
  {
    p->setBrush( mLabelBrush );
    QFont textFont( QStringLiteral( "helvetica" ) );
    textFont.setPixelSize( fontSizePainterUnits( 12, context ) );
    p->setFont( textFont );
    const QRectF textBounds = p->boundingRect( 3 * context.scaleFactor(), 3 * context.scaleFactor(), 5 * context.scaleFactor(), 5 * context.scaleFactor(), Qt::AlignLeft, msg );
    mTextBoxRect = QRectF( textBounds.x() - context.scaleFactor() * 1, textBounds.y() - context.scaleFactor() * 1,
                           textBounds.width() + 2 * context.scaleFactor(), textBounds.height() + 2 * context.scaleFactor() );
    p->drawRect( mTextBoxRect );
    p->drawText( textBounds, Qt::AlignLeft, msg );
  }

  if ( data( 1 ) != "composer" ) //draw residuals only on screen
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

  //only considering screen resolution is OK for the bounding box function
  const double rf = residualToScreenFactor();

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

  const QRectF residualArrowRect( QPointF( residualLeft, residualTop ), QPointF( residualRight, residualBottom ) );
  const QRectF markerRect( -2, -2, mTextBounds.width() + 6, mTextBounds.height() + 6 );
  QRectF boundingRect = residualArrowRect.united( markerRect );
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

  if ( mIsGCPSource )
  {
    setPos( toCanvasCoordinates( mDataPoint->sourceCoords() ) );
  }
  else
  {
    if ( mDataPoint->destinationInCanvasPixels().isEmpty() )
    {
      const QgsCoordinateReferenceSystem canvasCrs = mMapCanvas->mapSettings().destinationCrs();
      const QgsCoordinateTransform pointToCanvasTransform( mDataPoint->destinationCrs(), canvasCrs, QgsProject::instance() );
      const QgsPointXY canvasMapCoords = pointToCanvasTransform.transform( mDataPoint->destinationMapCoords() );
      const QPointF canvasCoordinatesInPixels = toCanvasCoordinates( canvasMapCoords );
      mDataPoint->setDestinationInCanvasPixels( canvasCoordinatesInPixels );
    }
    setPos( mDataPoint->destinationInCanvasPixels().toQPointF() );
  }
}

void QgsGCPCanvasItem::drawResidualArrow( QPainter *p, const QgsRenderContext &context )
{
  Q_UNUSED( context )
  if ( !mDataPoint || !mIsGCPSource || !mMapCanvas )
  {
    return;
  }

  QPointF residual = mDataPoint->residual();

  const double rf = residualToScreenFactor();
  p->setPen( mResidualPen );
  p->drawLine( QPointF( 0, 0 ), QPointF( residual.rx() * rf, residual.ry() * rf ) );

}

double QgsGCPCanvasItem::residualToScreenFactor() const
{
  if ( !mMapCanvas )
  {
    return 1;
  }

  const double mapUnitsPerScreenPixel = mMapCanvas->mapUnitsPerPixel();
  double mapUnitsPerRasterPixel = 1.0;

  const QList<QgsMapLayer *> canvasLayers = mMapCanvas->mapSettings().layers();
  if ( !canvasLayers.isEmpty() )
  {
    QgsMapLayer *mapLayer = canvasLayers.at( 0 );
    if ( mapLayer )
    {
      QgsRasterLayer *rasterLayer = qobject_cast<QgsRasterLayer *>( mapLayer );
      if ( rasterLayer )
      {
        mapUnitsPerRasterPixel = rasterLayer->rasterUnitsPerPixelX();
      }
    }
  }

  return 1.0 / ( mapUnitsPerScreenPixel * mapUnitsPerRasterPixel );
}

void QgsGCPCanvasItem::checkBoundingRectChange()
{
  prepareGeometryChange();
}

void QgsGCPCanvasItem::setPointColor( const QColor &color )
{
  mPointBrush.setColor( color );
}

double QgsGCPCanvasItem::fontSizePainterUnits( double points, const QgsRenderContext &c )
{
  return points * 0.3527 * c.scaleFactor();
}

