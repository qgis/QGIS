/***************************************************************************
    qgsmaptoolextent.h  -  map tool that emits an extent
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#include "qgsmaptoolextent.h"
#include "qgsmapcanvas.h"
#include "qgswkbtypes.h"
#include "qgsmapmouseevent.h"


QgsMapToolExtent::QgsMapToolExtent( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
  mRubberBand.reset( new QgsRubberBand( canvas, QgsWkbTypes::PolygonGeometry ) );
}

void QgsMapToolExtent::activate()
{
  QgsMapTool::activate();
}

void QgsMapToolExtent::deactivate()
{
  clearRubberBand();
  QgsMapTool::deactivate();
}

void QgsMapToolExtent::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( !mDraw )
    return;


  QgsPointXY p = toMapCoordinates( e->pos() );
  if ( mRatio.width() > 0 && mRatio.height() > 0 )
  {
    const double width = std::fabs( p.x() - mStartPoint.x() );
    double height = width * ( mRatio.width() / mRatio.height() );
    if ( p.y() - mStartPoint.y() < 0 )
      height *= -1;
    p.setY( mStartPoint.y() + height );
  }

  mEndPoint = toMapCoordinates( e->pos() );
  calculateEndPoint( mEndPoint );
  drawExtent();
}

void QgsMapToolExtent::canvasPressEvent( QgsMapMouseEvent *e )
{
  mStartPoint = toMapCoordinates( e->pos() );
  mEndPoint = mStartPoint;
  drawExtent();

  mDraw = true;
}

void QgsMapToolExtent::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( !mDraw )
    return;

  mEndPoint = toMapCoordinates( e->pos() );
  calculateEndPoint( mEndPoint );
  drawExtent();

  emit extentChanged( extent() );

  mDraw = false;
}

QgsRectangle QgsMapToolExtent::extent() const
{
  if ( mStartPoint.x() != mEndPoint.x() && mStartPoint.y() != mEndPoint.y() )
  {
    return QgsRectangle( mStartPoint, mEndPoint );
  }
  else
  {
    return QgsRectangle();
  }
}

void QgsMapToolExtent::clearRubberBand()
{
  mRubberBand->reset( QgsWkbTypes::PolygonGeometry );
}

void QgsMapToolExtent::calculateEndPoint( QgsPointXY &point )
{
  if ( mRatio.width() > 0 && mRatio.height() > 0 )
  {
    const double width = std::fabs( point.x() - mStartPoint.x() );
    double height = width * mRatio.height() / mRatio.width();
    if ( point.y() - mStartPoint.y() < 0 )
      height *= -1;
    point.setY( mStartPoint.y() + height );
  }
}

void QgsMapToolExtent::drawExtent()
{
  if ( qgsDoubleNear( mStartPoint.x(), mEndPoint.x() ) && qgsDoubleNear( mStartPoint.y(), mEndPoint.y() ) )
    return;

  const QgsRectangle rect( mStartPoint, mEndPoint );

  mRubberBand->reset( QgsWkbTypes::PolygonGeometry );
  mRubberBand->addPoint( QgsPointXY( rect.xMinimum(), rect.yMinimum() ), false );
  mRubberBand->addPoint( QgsPointXY( rect.xMaximum(), rect.yMinimum() ), false );
  mRubberBand->addPoint( QgsPointXY( rect.xMaximum(), rect.yMaximum() ), false );
  mRubberBand->addPoint( QgsPointXY( rect.xMinimum(), rect.yMaximum() ), true );

  mRubberBand->show();
}
