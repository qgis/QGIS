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
/* $Id$ */

#include "coordinatecapturemaptool.h"
#include "qgscursors.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsrubberband.h"
#include "qgscoordinatereferencesystem.h"

#include <QSettings>
#include <QMessageBox>
#include <QMouseEvent>
#include <QCursor>
#include <QPixmap>

CoordinateCaptureMapTool::CoordinateCaptureMapTool( QgsMapCanvas* thepCanvas )
    : QgsMapTool( thepCanvas )
{
  // set cursor
  QPixmap myCursor = QPixmap(( const char ** ) capture_point_cursor );
  mCursor = QCursor( myCursor, 8, 8 ); //8,8 is the point in the cursor where clicks register
  mpMapCanvas = thepCanvas;
  mpRubberBand = new QgsRubberBand( mpMapCanvas, true ); //true - its a polygon
  mpRubberBand->setColor( Qt::red );
  mpRubberBand->setWidth( 1 );
}

CoordinateCaptureMapTool::~CoordinateCaptureMapTool()
{
  if ( mpRubberBand != 0 )
    delete mpRubberBand;
}

void CoordinateCaptureMapTool::canvasMoveEvent( QMouseEvent * thepEvent )
{
  QgsPoint myOriginalPoint =
    mCanvas->getCoordinateTransform()->toMapCoordinates( thepEvent->x(), thepEvent->y() );
  emit mouseMoved( myOriginalPoint );
}

void CoordinateCaptureMapTool::canvasPressEvent( QMouseEvent * thepEvent )
{
}

void CoordinateCaptureMapTool::canvasReleaseEvent( QMouseEvent * thepEvent )
{
  if ( !mpMapCanvas || mpMapCanvas->isDrawing() )
  {
    return;
  }

  QgsPoint myOriginalPoint =
    mCanvas->getCoordinateTransform()->toMapCoordinates( thepEvent->x(), thepEvent->y() );
  emit mouseClicked( myOriginalPoint );

  //make a little box for display

  QgsPoint myPoint1 =
    mCanvas->getCoordinateTransform()->toMapCoordinates( thepEvent->x() - 1, thepEvent->y() - 1 );
  QgsPoint myPoint2 =
    mCanvas->getCoordinateTransform()->toMapCoordinates( thepEvent->x() + 1, thepEvent->y() - 1 );
  QgsPoint myPoint3 =
    mCanvas->getCoordinateTransform()->toMapCoordinates( thepEvent->x() + 1, thepEvent->y() + 1 );
  QgsPoint myPoint4 =
    mCanvas->getCoordinateTransform()->toMapCoordinates( thepEvent->x() - 1, thepEvent->y() + 1 );

  mpRubberBand->reset( true );
  // convert screen coordinates to map coordinates
  mpRubberBand->addPoint( myPoint1, false ); //true - update canvas
  mpRubberBand->addPoint( myPoint2, false ); //true - update canvas
  mpRubberBand->addPoint( myPoint3, false ); //true - update canvas
  mpRubberBand->addPoint( myPoint4, true ); //true - update canvas
  mpRubberBand->show();
}


void CoordinateCaptureMapTool::deactivate()
{
  mpRubberBand->reset( false );
  QgsMapTool::deactivate();
}
