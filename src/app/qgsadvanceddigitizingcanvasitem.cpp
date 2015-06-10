/***************************************************************************
    qgsadvanceddigitizingcanvasitem.cpp  -  map canvas item for CAD tools
    ----------------------
    begin                : October 2014
    copyright            : (C) Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QPainter>

#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsadvanceddigitizingcanvasitem.h"
#include "qgsmapcanvas.h"


QgsAdvancedDigitizingCanvasItem::QgsAdvancedDigitizingCanvasItem( QgsMapCanvas* canvas, QgsAdvancedDigitizingDockWidget* cadDockWidget )
    : QgsMapCanvasItem( canvas )
    , mLockedPen( QPen( QColor( 100, 100, 255, 255 ), .7, Qt::DashLine ) )
    , mConstruction1Pen( QPen( QColor( 100, 255, 100, 150 ), .7, Qt::DashLine ) )
    , mConstruction2Pen( QPen( QColor( 100, 255, 100, 255 ), .7, Qt::DashLine ) )
    , mSnapPen( QPen( QColor( 255, 175, 100, 150 ), 7 ) )
    , mSnapLinePen( QPen( QColor( 200, 100, 50, 150 ), .7, Qt::DashLine ) )
    , mCursorPen( QPen( QColor( 100, 255, 100, 255 ), .7 ) )
    , mAdvancedDigitizingDockWidget( cadDockWidget )
{
}

void QgsAdvancedDigitizingCanvasItem::paint( QPainter* painter )
{
  if ( !mAdvancedDigitizingDockWidget->cadEnabled() )
    return;

  QgsRectangle mapRect = mMapCanvas->extent();
  setRect( mapRect );

  int nPoints = mAdvancedDigitizingDockWidget->pointsCount();
  if ( !nPoints )
    return;

  bool previousPointExist, penulPointExist;
  const QgsPoint curPoint = mAdvancedDigitizingDockWidget->currentPoint( );
  const QgsPoint prevPoint = mAdvancedDigitizingDockWidget->previousPoint( &previousPointExist );
  const QgsPoint penulPoint = mAdvancedDigitizingDockWidget->penultimatePoint( &penulPointExist );
  const bool snappedToVertex = mAdvancedDigitizingDockWidget->snappedToVertex();
  const QList<QgsPoint> snappedSegment = mAdvancedDigitizingDockWidget->snappedSegment();
  const bool hasSnappedSegment = snappedSegment.count() == 2;

  const bool curPointExist = mapRect.contains( curPoint );

  const double mupp = mMapCanvas->getCoordinateTransform()->mapUnitsPerPixel();
  if ( mupp == 0 )
    return;

  QPointF curPointPix, prevPointPix, penulPointPix, snapSegmentPix1, snapSegmentPix2;

  if ( curPointExist )
  {
    curPointPix = toCanvasCoordinates( curPoint );
  }
  if ( previousPointExist )
  {
    prevPointPix = toCanvasCoordinates( prevPoint );
  }
  if ( penulPointExist )
  {
    penulPointPix = toCanvasCoordinates( penulPoint );
  }
  if ( hasSnappedSegment )
  {
    snapSegmentPix1 = toCanvasCoordinates( snappedSegment[0] );
    snapSegmentPix2 = toCanvasCoordinates( snappedSegment[1] );
  }

  painter->setRenderHints( QPainter::Antialiasing );

  // Draw point snap
  if ( curPointExist && snappedToVertex )
  {
    painter->setPen( mSnapPen );
    painter->drawEllipse( curPointPix, 10, 10 );
  }

  // Draw segment snap
  if ( hasSnappedSegment && !snappedToVertex )
  {
    painter->setPen( mSnapPen );
    painter->drawLine( snapSegmentPix1.x(),
                       snapSegmentPix1.y(),
                       snapSegmentPix2.x(),
                       snapSegmentPix2.y() );

    if ( curPointExist )
    {
      painter->setPen( mSnapLinePen );
      painter->drawLine( snapSegmentPix1.x(),
                         snapSegmentPix1.y(),
                         curPointPix.x(),
                         curPointPix.y() );
    }
  }

  // Draw segment par/per input
  if ( mAdvancedDigitizingDockWidget->additionalConstraint() !=   QgsAdvancedDigitizingDockWidget::NoConstraint && hasSnappedSegment )
  {
    painter->setPen( mConstruction2Pen );
    painter->drawLine( snapSegmentPix1.x(),
                       snapSegmentPix1.y(),
                       snapSegmentPix2.x(),
                       snapSegmentPix2.y() );
  }

  // Draw angle
  if ( nPoints > 1 )
  {
    double a0, a;
    if ( mAdvancedDigitizingDockWidget->constraintAngle()->relative() && nPoints > 2 )
    {
      a0 = qAtan2( -( prevPoint.y() - penulPoint.y() ), prevPoint.x() - penulPoint.x() );
    }
    else
    {
      a0 = 0;
    }
    if ( mAdvancedDigitizingDockWidget->constraintAngle()->isLocked() )
    {
      a = a0 - mAdvancedDigitizingDockWidget->constraintAngle()->value() * M_PI / 180;
    }
    else
    {
      a = qAtan2( -( curPoint.y() - prevPoint.y() ), curPoint.x() - prevPoint.x() );
    }
    painter->setPen( mConstruction2Pen );
    painter->drawArc( prevPointPix.x() - 20,
                      prevPointPix.y() - 20,
                      40, 40,
                      ( int )16 * -a0 * 180 / M_PI,
                      ( int )16 * ( a0 - a ) * 180 / M_PI );
    painter->drawLine( prevPointPix.x(),
                       prevPointPix.y(),
                       prevPointPix.x() + 60*qCos( a0 ),
                       prevPointPix.y() + 60*qSin( a0 ) );

    if ( mAdvancedDigitizingDockWidget->constraintAngle()->isLocked() )
    {
      painter->setPen( mLockedPen );
      double d = std::max( boundingRect().width(), boundingRect().height() );
      painter->drawLine( prevPointPix.x() - d*qCos( a ),
                         prevPointPix.y() - d*qSin( a ),
                         prevPointPix.x() + d*qCos( a ),
                         prevPointPix.y() + d*qSin( a ) );
    }
  }

  // Draw distance
  if ( nPoints > 1 && mAdvancedDigitizingDockWidget->constraintDistance()->isLocked() )
  {
    painter->setPen( mLockedPen );
    double r = mAdvancedDigitizingDockWidget->constraintDistance()->value() / mupp;
    painter->drawEllipse( prevPointPix, r, r );
  }

  // Draw x
  if ( mAdvancedDigitizingDockWidget->constraintX()->isLocked() )
  {
    double x;
    bool draw = true;
    painter->setPen( mLockedPen );
    if ( mAdvancedDigitizingDockWidget->constraintX()->relative() )
    {
      if ( nPoints > 1 )
      {
        x = mAdvancedDigitizingDockWidget->constraintX()->value() / mupp + prevPointPix.x();
      }
      else
      {
        draw = false;
      }
    }
    else
    {
      x = toCanvasCoordinates( QgsPoint( mAdvancedDigitizingDockWidget->constraintX()->value(), 0 ) ).x();
    }
    if ( draw )
    {
      painter->drawLine( x,
                         0,
                         x,
                         boundingRect().height() );
    }
  }

  // Draw y
  if ( mAdvancedDigitizingDockWidget->constraintY()->isLocked() )
  {
    double y;
    bool draw = true;
    painter->setPen( mLockedPen );
    if ( mAdvancedDigitizingDockWidget->constraintY()->relative() )
    {
      if ( nPoints > 1 )
      {
        // y is reversed!
        y = -mAdvancedDigitizingDockWidget->constraintY()->value() / mupp + prevPointPix.y();
      }
      else
      {
        draw = false;
      }
    }
    else
    {
      y = toCanvasCoordinates( QgsPoint( 0, mAdvancedDigitizingDockWidget->constraintY()->value() ) ).y();
    }
    if ( draw )
    {
      painter->drawLine( 0,
                         y,
                         boundingRect().width(),
                         y );
    }
  }

  // Draw constr
  if ( mAdvancedDigitizingDockWidget->additionalConstraint() == QgsAdvancedDigitizingDockWidget::NoConstraint )
  {
    if ( curPointExist && previousPointExist )
    {
      painter->setPen( mConstruction2Pen );
      painter->drawLine( prevPointPix.x(),
                         prevPointPix.y(),
                         curPointPix.x(),
                         curPointPix.y() );
    }

    if ( previousPointExist && penulPointExist )
    {
      painter->setPen( mConstruction1Pen );
      painter->drawLine( penulPointPix.x(),
                         penulPointPix.y(),
                         prevPointPix.x(),
                         prevPointPix.y() );
    }
  }

  if ( curPointExist )
  {
    painter->setPen( mCursorPen );
    painter->drawLine( curPointPix.x() - 5,
                       curPointPix.y() - 5,
                       curPointPix.x() + 5,
                       curPointPix.y() + 5 );
    painter->drawLine( curPointPix.x() - 5,
                       curPointPix.y() + 5,
                       curPointPix.x() + 5,
                       curPointPix.y() - 5 );
  }

}
