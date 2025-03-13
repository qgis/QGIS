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
#include "qgspolygon.h"


QgsMapToolClippingPlanes::QgsMapToolClippingPlanes( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
  mRubberBandPolygon.reset( new QgsRubberBand( canvas, Qgis::GeometryType::Polygon ) );
  mRubberBandLines.reset( new QgsRubberBand( canvas, Qgis::GeometryType::Line ) );
  mRubberBandPoints.reset( new QgsRubberBand( canvas, Qgis::GeometryType::Point ) );
  mRubberBandPoints->setColor( QColorConstants::Red );
  mRubberBandPoints->setIconSize( 10 );
  mRubberBandLines->setColor( QColorConstants::Red );
  mRubberBandLines->setWidth( 3 );
  QColor polygonColor = QColorConstants::Red.lighter();
  polygonColor.setAlphaF( 0.5 );
  mRubberBandPolygon->setColor( polygonColor );
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
      mRubberBandPolygon->reset( Qgis::GeometryType::Polygon );
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
  if ( mRubberBandPoints->numberOfVertices() == 2 )
  {
    QgsVector vec( *mRubberBandPoints->getPoint( 0, 1 ) - *mRubberBandPoints->getPoint( 0, 0 ) );
    vec = vec.normalized().perpVector();
    const double distance = vec * ( point - *mRubberBandPoints->getPoint( 0 ) );
    const QVector<QgsPointXY> points( {
      *mRubberBandPoints->getPoint( 0, 0 ) + vec * distance,  //! top left corner
      *mRubberBandPoints->getPoint( 0, 1 ) + vec * distance,  //! top right corner
      *mRubberBandPoints->getPoint( 0, 1 ) + -vec * distance, //! bottom right corner
      *mRubberBandPoints->getPoint( 0, 0 ) + -vec * distance  //! bottom left corner
    } );
    QgsPolygon *rect = new QgsPolygon( new QgsLineString( points ) );
    mRubberBandPolygon->setToGeometry( QgsGeometry( rect ) );
    mPoints = points;
  }
  else
  {
    mPoints.replace( mPoints.size() - 1, point );
    if ( mPoints.size() % 2 == 0 )
    {
      mRubberBandLines->movePoint( point );
    }
  }
}

void QgsMapToolClippingPlanes::canvasPressEvent( QgsMapMouseEvent * )
{
  mClicked = true;
}

void QgsMapToolClippingPlanes::canvasReleaseEvent( QgsMapMouseEvent *e )
{
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
    else if ( mPoints.size() > 3 )
    {
      calculateClippingPlanes();
    }

    mClicked = false;
  }
  else if ( e->button() == Qt::RightButton )
  {
    mPoints.clear();
    clearRubberBand();
  }
}

void QgsMapToolClippingPlanes::calculateClippingPlanes()
{
  QgsVector vec( mPoints.at( 1 ) - mPoints.at( 0 ) );
  vec = vec.normalized();
  QVector<QPair<QgsVector3D, QgsVector3D>> planes;
  planes.append( QPair<QgsVector3D, QgsVector3D>( QgsVector3D( mPoints.at( 0 ).x(), mPoints.at( 0 ).y(), 0 ), QgsVector3D( vec.x(), vec.y(), 0 ) ) );                             //! left clip plane
  planes.append( QPair<QgsVector3D, QgsVector3D>( QgsVector3D( mPoints.at( 0 ).x(), mPoints.at( 0 ).y(), 0 ), QgsVector3D( -vec.perpVector().x(), -vec.perpVector().y(), 0 ) ) ); //! top clip plane
  planes.append( QPair<QgsVector3D, QgsVector3D>( QgsVector3D( mPoints.at( 1 ).x(), mPoints.at( 1 ).y(), 0 ), QgsVector3D( -vec.x(), -vec.y(), 0 ) ) );                           //! right clip plane
  planes.append( QPair<QgsVector3D, QgsVector3D>( QgsVector3D( mPoints.at( 2 ).x(), mPoints.at( 2 ).y(), 0 ), QgsVector3D( vec.perpVector().x(), vec.perpVector().y(), 0 ) ) );   //! bottom clip plane
  emit clippingPlanesChanged( planes );
}

void QgsMapToolClippingPlanes::clearRubberBand() const
{
  mRubberBandLines->reset( Qgis::GeometryType::Line );
  mRubberBandPoints->reset( Qgis::GeometryType::Point );
  mRubberBandPolygon->reset( Qgis::GeometryType::Polygon );
}
