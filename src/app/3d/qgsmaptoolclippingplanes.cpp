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
#include "qgs3dmapcanvas.h"
#include "qgs3dutils.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgspolygon.h"

#include <QVector4D>


QgsMapToolClippingPlanes::QgsMapToolClippingPlanes( QgsMapCanvas *canvas, const QgsVector3D &sceneOrigin )
  : QgsMapTool( canvas ), mSceneOrigin( sceneOrigin )
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
    clearRubberBand();
  }

  if ( e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace )
  {
    if ( mRubberBandPoints->numberOfVertices() == 0 )
    {
      return;
    }

    if ( mRubberBandPoints->numberOfVertices() == 1 )
    {
      clearRubberBand();
    }
    else
    {
      mRubberBandPoints->removeLastPoint();
      mRubberBandPolygon->reset( Qgis::GeometryType::Polygon );
    }
  }
  //TODO: somehow consume the event so we don't get the annoying info message about not having vector layer
}

void QgsMapToolClippingPlanes::deactivate()
{
  clearRubberBand();
  QgsMapTool::deactivate();
}

void QgsMapToolClippingPlanes::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( mClicked || mRubberBandPoints->numberOfVertices() == 0 )
    return;

  const QgsPointXY point = toMapCoordinates( e->pos() );
  if ( mRubberBandPoints->numberOfVertices() == 2 )
  {
    QgsVector vec( *mRubberBandPoints->getPoint( 0, 1 ) - *mRubberBandPoints->getPoint( 0, 0 ) );
    vec = vec.normalized().perpVector();
    mRectangleWidth = QgsGeometryUtils::distToInfiniteLine(
      QgsPoint( point ),
      QgsPoint( *mRubberBandPoints->getPoint( 0, 0 ) ),
      QgsPoint( *mRubberBandPoints->getPoint( 0, 1 ) )
    );
    const QVector<QgsPointXY> points( {
      *mRubberBandPoints->getPoint( 0, 0 ) + vec * mRectangleWidth,  //! top left corner
      *mRubberBandPoints->getPoint( 0, 1 ) + vec * mRectangleWidth,  //! top right corner
      *mRubberBandPoints->getPoint( 0, 1 ) + -vec * mRectangleWidth, //! bottom right corner
      *mRubberBandPoints->getPoint( 0, 0 ) + -vec * mRectangleWidth  //! bottom left corner
    } );
    mRubberBandPolygon->setToGeometry( QgsGeometry( new QgsPolygon( new QgsLineString( points ) ) ) );
  }
  else
  {
    mRubberBandLines->movePoint( point );
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
    if ( mRubberBandPoints->numberOfVertices() == 2 )
    {
      const QList<QVector4D> clippingPlanes = Qgs3DUtils::lineSegmentToClippingPlanes(
        *mRubberBandPoints->getPoint( 0, 0 ),
        *mRubberBandPoints->getPoint( 0, 1 ),
        mRectangleWidth,
        mSceneOrigin
      );
      emit clippingPlanesChanged( clippingPlanes );
    }
    else
    {
      if ( mRubberBandPoints->numberOfVertices() == 0 )
      {
        mRubberBandPoints->addPoint( point );
        mRubberBandLines->addPoint( point );
        mRubberBandLines->addPoint( point );
      }
      else
      {
        mRubberBandPoints->addPoint( point );
      }
    }
    mClicked = false;
  }
  else if ( e->button() == Qt::RightButton )
  {
    clearRubberBand();
  }
}

void QgsMapToolClippingPlanes::clearRubberBand() const
{
  mRubberBandLines->reset( Qgis::GeometryType::Line );
  mRubberBandPoints->reset( Qgis::GeometryType::Point );
  mRubberBandPolygon->reset( Qgis::GeometryType::Polygon );
}
