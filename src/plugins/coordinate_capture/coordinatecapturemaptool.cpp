/***************************************************************************
    coordinatecapturemaptool.cpp  -  map tool for getting click coords
    ---------------------
    begin                : August 2008
    copyright            : (C) 2008 by Tim Sutton
    email                : tim at linfiniti.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QSettings>
#include <QCursor>
#include <QPixmap>

#include "coordinatecapturemaptool.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsrubberband.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsmapmouseevent.h"


CoordinateCaptureMapTool::CoordinateCaptureMapTool( QgsMapCanvas *thepCanvas )
  : QgsMapTool( thepCanvas )
{
  setCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::CrossHair ) );
  mpMapCanvas = thepCanvas;
  mpRubberBand = new QgsRubberBand( mpMapCanvas, QgsWkbTypes::PolygonGeometry );
  mpRubberBand->setColor( Qt::red );
  mpRubberBand->setWidth( 1 );
}

void CoordinateCaptureMapTool::canvasMoveEvent( QgsMapMouseEvent *thepEvent )
{
  QgsPointXY myOriginalPoint =
    mCanvas->getCoordinateTransform()->toMapCoordinates( thepEvent->x(), thepEvent->y() );
  emit mouseMoved( myOriginalPoint );
}

void CoordinateCaptureMapTool::canvasPressEvent( QgsMapMouseEvent *thepEvent )
{
  Q_UNUSED( thepEvent );
}

void CoordinateCaptureMapTool::canvasReleaseEvent( QgsMapMouseEvent *thepEvent )
{
  QgsPointXY myOriginalPoint =
    mCanvas->getCoordinateTransform()->toMapCoordinates( thepEvent->x(), thepEvent->y() );
  emit mouseClicked( myOriginalPoint );

  //make a little box for display

  QgsPointXY myPoint1 =
    mCanvas->getCoordinateTransform()->toMapCoordinates( thepEvent->x() - 1, thepEvent->y() - 1 );
  QgsPointXY myPoint2 =
    mCanvas->getCoordinateTransform()->toMapCoordinates( thepEvent->x() + 1, thepEvent->y() - 1 );
  QgsPointXY myPoint3 =
    mCanvas->getCoordinateTransform()->toMapCoordinates( thepEvent->x() + 1, thepEvent->y() + 1 );
  QgsPointXY myPoint4 =
    mCanvas->getCoordinateTransform()->toMapCoordinates( thepEvent->x() - 1, thepEvent->y() + 1 );

  mpRubberBand->reset( QgsWkbTypes::PolygonGeometry );
  // convert screen coordinates to map coordinates
  mpRubberBand->addPoint( myPoint1, false ); //true - update canvas
  mpRubberBand->addPoint( myPoint2, false ); //true - update canvas
  mpRubberBand->addPoint( myPoint3, false ); //true - update canvas
  mpRubberBand->addPoint( myPoint4, true ); //true - update canvas
  mpRubberBand->show();
}


void CoordinateCaptureMapTool::deactivate()
{
  mpRubberBand->reset( QgsWkbTypes::LineGeometry );
  QgsMapTool::deactivate();
}
