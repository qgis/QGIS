/***************************************************************************
    qgsmaptoolclippingplanes.h  -
    ---------------------
    begin                : March 2025
    copyright            : (C) 2025 by Matej Bagar
    email                : matej dot bagar at lutraconsulting dot co dot uk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsmaptoolclippingplanes.h"
#include "moc_qgsmaptoolclippingplanes.cpp"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"

#include <QVector4D>


QgsMapToolClippingPlanes::QgsMapToolClippingPlanes( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
  mRubberBandLines.reset( new QgsRubberBand( canvas, Qgis::GeometryType::Line ) );
  mRubberBandPoints.reset( new QgsRubberBand( canvas, Qgis::GeometryType::Point ) );
  mRubberBandPoints->setColor( QColorConstants::Red );
  mRubberBandPoints->setIconSize( 10 );
  mRubberBandLines->setColor( QColorConstants::Red );
  mRubberBandLines->setWidth( 3 );
}

void QgsMapToolClippingPlanes::activate()
{
  QgsMapTool::activate();
  mRubberBandPoints->show();
  mRubberBandLines->show();
}

void QgsMapToolClippingPlanes::keyReleaseEvent( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Escape )
  {
    mPoints.clear();
    clearRubberBand();
  }

  if ( e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace )
  {
    if ( mPoints.size() == 2 )
    {
      mPoints.clear();
      clearRubberBand();
    }
    else if ( mPoints.empty() )
    {
      return;
    }
    else
    {
      mRubberBandPoints->removeLastPoint();
      mPoints.removeLast();
    }

    if ( mPoints.size() % 2 == 1 )
    {
      mRubberBandLines->removeLastPoint();
    }
  }
  //TODO: somehow consume the event so we don't get the annoying info message about not having vector layer
}

void QgsMapToolClippingPlanes::deactivate()
{
  mPoints.clear();
  clearRubberBand();
  QgsMapTool::deactivate();
}

void QgsMapToolClippingPlanes::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( mClicked || mPoints.isEmpty() )
    return;

  const QgsPointXY point = toMapCoordinates( e->pos() );
  mPoints.replace( mPoints.size() - 1, point );
  if ( mPoints.size() % 2 == 0 )
  {
    mRubberBandLines->movePoint( point );
  }
}

void QgsMapToolClippingPlanes::canvasPressEvent( QgsMapMouseEvent *e )
{
  mClicked = true;
}

void QgsMapToolClippingPlanes::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( mPoints.size() > 2 )
  {
    return;
  }

  if ( e->button() == Qt::LeftButton )
  {
    const QgsPointXY point = toMapCoordinates( e->pos() );
    if ( mPoints.isEmpty() )
    {
      mPoints << point;
      mRubberBandPoints->addPoint( point );
      mRubberBandLines->addPoint( point );
    }
    else
    {
      mRubberBandPoints->addPoint( point );
    }

    mPoints << point;
    if ( mPoints.size() <= 2 )
    {
      mRubberBandLines->addPoint( point );
    }
    else
    {
      // fixes fast mouse movements on finish
      mRubberBandLines->movePoint( point );
      calculateClippingPlanes();
    }

    mClicked = false;
  }
}

void QgsMapToolClippingPlanes::calculateClippingPlanes()
{
  QgsVector vec( mPoints.at( 1 ) - mPoints.at( 0 ) );
  vec = vec.normalized().perpVector();
  emit clippingPlanesChanged( QVector<QPair<QgsVector3D, QgsVector3D>>( { QPair<QgsVector3D, QgsVector3D>( QgsVector3D( mPoints.at( 0 ).x(), mPoints.at( 0 ).y(), 0 ), QgsVector3D( vec.x(), vec.y(), 0 ) ) } ) );
}

void QgsMapToolClippingPlanes::clearRubberBand()
{
  mRubberBandLines->reset( Qgis::GeometryType::Line );
  mRubberBandPoints->reset( Qgis::GeometryType::Point );
}
