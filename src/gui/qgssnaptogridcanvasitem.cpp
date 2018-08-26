/***************************************************************************
    qgssnaptogridcanvasitem.cpp
    ----------------------
    begin                : August 2018
    copyright            : (C) Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssnaptogridcanvasitem.h"
#include "qgsmapcanvas.h"

QgsSnapToGridCanvasItem::QgsSnapToGridCanvasItem( QgsMapCanvas *mapCanvas )
  : QgsMapCanvasItem( mapCanvas )
  , mGridPen( QPen( QColor( 127, 127, 127, 150 ), 1 ) )
  , mCurrentPointPen( QPen( QColor( 200, 200, 200, 150 ), 3 ) )
{
  updateMapCanvasCrs();
  connect( mMapCanvas, &QgsMapCanvas::extentsChanged, this, &QgsSnapToGridCanvasItem::updateZoomFactor );
  connect( mMapCanvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsSnapToGridCanvasItem::updateMapCanvasCrs );
}

void QgsSnapToGridCanvasItem::paint( QPainter *painter )
{
  painter->save();
  QgsRectangle mapRect = mMapCanvas->extent();
  if ( rect() != mapRect )
    setRect( mapRect );

  painter->setRenderHints( QPainter::Antialiasing );
  painter->setCompositionMode( QPainter::CompositionMode_Difference );

  if ( mEnabled && mAvailableByZoomFactor )
  {
    const QgsRectangle layerExtent = mTransform.transformBoundingBox( mapRect, QgsCoordinateTransform::ReverseTransform );
    const QgsPointXY layerPt = mTransform.transform( mPoint, QgsCoordinateTransform::ReverseTransform );

    const double gridXMin = std::ceil( layerExtent.xMinimum() / mPrecision ) * mPrecision;
    const double gridXMax = std::ceil( layerExtent.xMaximum() / mPrecision ) * mPrecision;
    const double gridYMin = std::ceil( layerExtent.yMinimum() / mPrecision ) * mPrecision;
    const double gridYMax = std::ceil( layerExtent.yMaximum() / mPrecision ) * mPrecision;

    for ( int x = gridXMin ; x < gridXMax; x += mPrecision )
    {
      for ( int y = gridYMin ; y < gridYMax; y += mPrecision )
      {
        const QgsPointXY pt = mTransform.transform( x, y );
        const QPointF canvasPt = toCanvasCoordinates( pt );

        if ( qgsDoubleNear( layerPt.x(), x, mPrecision / 3 ) && qgsDoubleNear( layerPt.y(), y, mPrecision / 3 ) )
        {
          painter->setPen( mCurrentPointPen );
        }
        else
        {
          painter->setPen( mGridPen );
        }
        painter->drawLine( canvasPt.x() - 3, canvasPt.y(), canvasPt.x() + 3, canvasPt.y() );
        painter->drawLine( canvasPt.x(), canvasPt.y() - 3, canvasPt.x(), canvasPt.y() + 3 );
      }
    }
  }

  painter->restore();
}

QgsPointXY QgsSnapToGridCanvasItem::point() const
{
  return mPoint;
}

void QgsSnapToGridCanvasItem::setPoint( const QgsPointXY &point )
{
  mPoint = point;
  update();
}

double QgsSnapToGridCanvasItem::precision() const
{
  return mPrecision;
}

void QgsSnapToGridCanvasItem::setPrecision( double precision )
{
  mPrecision = precision;
  updateZoomFactor();
}

QgsCoordinateReferenceSystem QgsSnapToGridCanvasItem::crs() const
{
  return mTransform.sourceCrs();
}

void QgsSnapToGridCanvasItem::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mTransform.setSourceCrs( crs );
  updateZoomFactor();
}

bool QgsSnapToGridCanvasItem::enabled() const
{
  return mEnabled;
}

void QgsSnapToGridCanvasItem::setEnabled( bool enabled )
{
  mEnabled = enabled;
  update();
}

void QgsSnapToGridCanvasItem::updateMapCanvasCrs()
{
  mTransform.setContext( mMapCanvas->mapSettings().transformContext() );
  mTransform.setDestinationCrs( mMapCanvas->mapSettings().destinationCrs() );
  update();
}



void QgsSnapToGridCanvasItem::updateZoomFactor()
{
  if ( !isVisible() )
    return;

  try
  {
    const int threshold = 5;

    const QgsPointXY centerPoint = mMapCanvas->extent().center();
    const QPointF canvasCenter = toCanvasCoordinates( centerPoint );

    const QgsPointXY pt1 = mMapCanvas->mapSettings().mapToPixel().toMapCoordinates( canvasCenter.x() - threshold, canvasCenter.y() - threshold );
    const QgsPointXY pt2 = mMapCanvas->mapSettings().mapToPixel().toMapCoordinates( canvasCenter.x() + threshold, canvasCenter.y() + threshold );

    const QgsPointXY layerPt1 = mTransform.transform( pt1, QgsCoordinateTransform::ReverseTransform );
    const QgsPointXY layerPt2 = mTransform.transform( pt2, QgsCoordinateTransform::ReverseTransform );

    const double dist = layerPt1.distance( layerPt2 );

    if ( dist < mPrecision )
      mAvailableByZoomFactor = true;
    else
      mAvailableByZoomFactor = false;
  }
  catch ( QgsCsException &e )
  {
    // transform errors?
    // you've probably got worse problems than the grid with your digitizing operations in the current projection.
    mAvailableByZoomFactor = false;
  }
}
