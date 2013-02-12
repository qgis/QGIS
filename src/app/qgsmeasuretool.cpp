/***************************************************************************
    qgsmeasuretool.cpp  -  map tool for measuring distances and areas
    ---------------------
    begin                : April 2007
    copyright            : (C) 2007 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdistancearea.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgsmaptopixel.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgstolerance.h"

#include "qgsmeasuredialog.h"
#include "qgsmeasuretool.h"
#include "qgscursors.h"

#include <QMessageBox>
#include <QMouseEvent>
#include <QSettings>

QgsMeasureTool::QgsMeasureTool( QgsMapCanvas* canvas, bool measureArea )
    : QgsMapTool( canvas )
{
  mMeasureArea = measureArea;

  mRubberBand = new QgsRubberBand( canvas, mMeasureArea ? QGis::Polygon : QGis::Line );

  QPixmap myCrossHairQPixmap = QPixmap(( const char ** ) cross_hair_cursor );
  mCursor = QCursor( myCrossHairQPixmap, 8, 8 );

  mDone = true;
  // Append point we will move
  mPoints.append( QgsPoint( 0, 0 ) );

  mDialog = new QgsMeasureDialog( this, Qt::WindowStaysOnTopHint );
  mSnapper.setMapCanvas( canvas );

  connect( canvas->mapRenderer(), SIGNAL( destinationSrsChanged() ),
           this, SLOT( updateSettings() ) );
}

QgsMeasureTool::~QgsMeasureTool()
{
  delete mDialog;
  delete mRubberBand;
}


const QList<QgsPoint>& QgsMeasureTool::points()
{
  return mPoints;
}


void QgsMeasureTool::activate()
{
  mDialog->restorePosition();
  QgsMapTool::activate();

  // ensure that we have correct settings
  updateSettings();

  // If we suspect that they have data that is projected, yet the
  // map CRS is set to a geographic one, warn them.
  if ( mCanvas->mapRenderer()->destinationCrs().geographicFlag() &&
       ( mCanvas->extent().height() > 360 ||
         mCanvas->extent().width() > 720 ) )
  {
    QMessageBox::warning( NULL, tr( "Incorrect measure results" ),
                          tr( "<p>This map is defined with a geographic coordinate system "
                              "(latitude/longitude) "
                              "but the map extents suggests that it is actually a projected "
                              "coordinate system (e.g., Mercator). "
                              "If so, the results from line or area measurements will be "
                              "incorrect.</p>"
                              "<p>To fix this, explicitly set an appropriate map coordinate "
                              "system using the <tt>Settings:Project Properties</tt> menu." ) );
    mWrongProjectProjection = true;
  }
}

void QgsMeasureTool::deactivate()
{
  mDialog->close();
  mRubberBand->reset();
  QgsMapTool::deactivate();
}


void QgsMeasureTool::restart()
{
  mPoints.clear();

  mRubberBand->reset( mMeasureArea ? QGis::Polygon : QGis::Line );

  // re-read settings
  updateSettings();

  mDone = true;
  mWrongProjectProjection = false;

}

void QgsMeasureTool::updateSettings()
{
  QSettings settings;

  int myRed = settings.value( "/qgis/default_measure_color_red", 180 ).toInt();
  int myGreen = settings.value( "/qgis/default_measure_color_green", 180 ).toInt();
  int myBlue = settings.value( "/qgis/default_measure_color_blue", 180 ).toInt();
  mRubberBand->setColor( QColor( myRed, myGreen, myBlue ) );
  mDialog->updateSettings();
}

//////////////////////////

void QgsMeasureTool::canvasPressEvent( QMouseEvent * e )
{
  if ( e->button() == Qt::LeftButton )
  {
    if ( mDone )
    {
      mDialog->restart();
      QgsPoint point = snapPoint( e->pos() );
      addPoint( point );
      mDone = false;
    }
  }
}

void QgsMeasureTool::canvasMoveEvent( QMouseEvent * e )
{
  if ( ! mDone )
  {
    QgsPoint point = snapPoint( e->pos() );

    mRubberBand->movePoint( point );
    if ( ! mPoints.isEmpty() )
    {
      // Update last point
      mPoints.removeLast();
      mPoints.append( point ) ;
      mDialog->mouseMove( point );
    }
  }
}


void QgsMeasureTool::canvasReleaseEvent( QMouseEvent * e )
{
  QgsPoint point = snapPoint( e->pos() );

  if ( e->button() == Qt::RightButton && ( e->buttons() & Qt::LeftButton ) == 0 ) // restart
  {
    if ( mDone )
    {
      mDialog->restart();
    }
    else
    {
      // The figure is finished
      mDone = true;
      mDialog->show();
    }
  }
  else if ( e->button() == Qt::LeftButton )
  {
    // Append point we will move
    addPoint( point );
    mDialog->show();
  }

}


void QgsMeasureTool::addPoint( QgsPoint &point )
{
  QgsDebugMsg( "point=" + point.toString() );

  int last = mPoints.size() - 1;
  // don't add points with the same coordinates
  if ( mPoints.size() > 1 && mPoints[ last ] == mPoints[ last - 1 ] )
    return;

  QgsPoint pnt( point );
  // Append point that we will be moving.
  mPoints.append( pnt );

  mRubberBand->addPoint( point );
  if ( ! mDone )
  {
    mDialog->addPoint( point );
  }
}

QgsPoint QgsMeasureTool::snapPoint( const QPoint& p )
{
  QList<QgsSnappingResult> snappingResults;
  if ( mSnapper.snapToBackgroundLayers( p, snappingResults ) != 0 || snappingResults.size() < 1 )
  {
    return mCanvas->getCoordinateTransform()->toMapCoordinates( p );
  }
  else
  {
    return snappingResults.constBegin()->snappedVertex;
  }
}
