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
  mRubberBandPoints = new QgsRubberBand( canvas, QGis::Point );

  QPixmap myCrossHairQPixmap = QPixmap(( const char ** ) cross_hair_cursor );
  mCursor = QCursor( myCrossHairQPixmap, 8, 8 );

  mDone = true;
  // Append point we will move
  mPoints.append( QgsPoint( 0, 0 ) );

  mDialog = new QgsMeasureDialog( this, Qt::WindowStaysOnTopHint );
  mDialog->restorePosition();
  mSnapper.setMapCanvas( canvas );

  connect( canvas, SIGNAL( destinationCrsChanged() ),
           this, SLOT( updateSettings() ) );
}

QgsMeasureTool::~QgsMeasureTool()
{
  delete mDialog;
  delete mRubberBand;
  delete mRubberBandPoints;
}


const QList<QgsPoint>& QgsMeasureTool::points()
{
  return mPoints;
}


void QgsMeasureTool::activate()
{
  mDialog->show();
  QgsMapTool::activate();

  // ensure that we have correct settings
  updateSettings();

  // If we suspect that they have data that is projected, yet the
  // map CRS is set to a geographic one, warn them.
  if ( mCanvas->mapSettings().destinationCrs().geographicFlag() &&
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
  mDialog->hide();
  QgsMapTool::deactivate();
}

void QgsMeasureTool::restart()
{
  mPoints.clear();

  mRubberBand->reset( mMeasureArea ? QGis::Polygon : QGis::Line );
  mRubberBandPoints->reset( QGis::Point );

  // re-read settings
  updateSettings();

  mDone = true;
  mWrongProjectProjection = false;

}

void QgsMeasureTool::updateSettings()
{
  QSettings settings;

  int myRed = settings.value( "/qgis/default_measure_color_red", 222 ).toInt();
  int myGreen = settings.value( "/qgis/default_measure_color_green", 155 ).toInt();
  int myBlue = settings.value( "/qgis/default_measure_color_blue", 67 ).toInt();
  mRubberBand->setColor( QColor( myRed, myGreen, myBlue, 100 ) );
  mRubberBand->setWidth( 3 );
  mRubberBandPoints->setIcon( QgsRubberBand::ICON_CIRCLE );
  mRubberBandPoints->setIconSize( 10 );
  mRubberBandPoints->setColor( QColor( myRed, myGreen, myBlue, 150 ) );
  mDialog->updateSettings();
}

//////////////////////////

void QgsMeasureTool::canvasPressEvent( QMouseEvent * e )
{
  Q_UNUSED( e );
}

void QgsMeasureTool::canvasMoveEvent( QMouseEvent * e )
{
  if ( ! mDone )
  {
    QgsPoint point = snapPoint( e->pos() );

    mRubberBand->movePoint( point );
    mDialog->mouseMove( point );
  }
}


void QgsMeasureTool::canvasReleaseEvent( QMouseEvent * e )
{
  QgsPoint point = snapPoint( e->pos() );

  if ( mDone ) // if we have stopped measuring any mouse click restart measuring
  {
    mDialog->restart();
  }

  if ( e->button() == Qt::RightButton ) // if we clicked the right button we stop measuring
  {
    mDone = true;
  }
  else if ( e->button() == Qt::LeftButton )
  {
    mDone = false;
  }

  // we always add the clicked point to the measuring feature
  addPoint( point );
  mDialog->show();

}

void QgsMeasureTool::undo()
{
  if ( mRubberBand )
  {
    if ( mPoints.size() < 1 )
    {
      return;
    }

    if ( mPoints.size() == 1 )
    {
      //removing first point, so restart everything
      restart();
      mDialog->restart();
    }
    else
    {
      //remove second last point from line band, and last point from points band
      mRubberBand->removePoint( -2, true );
      mRubberBandPoints->removePoint( -1, true );
      mPoints.removeLast();

      mDialog->removeLastPoint();
    }

  }
}

void QgsMeasureTool::keyPressEvent( QKeyEvent* e )
{
  if (( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete ) )
  {
    if ( !mDone )
    {
      undo();
    }

    // Override default shortcut management in MapCanvas
    e->ignore();
  }
}


void QgsMeasureTool::addPoint( QgsPoint &point )
{
  QgsDebugMsg( "point=" + point.toString() );

  // don't add points with the same coordinates
  if ( !mPoints.isEmpty() && mPoints.last() == point )
  {
    return;
  }

  QgsPoint pnt( point );
  // Append point that we will be moving.
  mPoints.append( pnt );

  mRubberBand->addPoint( point );
  mRubberBandPoints->addPoint( point );
  if ( ! mDone )    // Prevent the insertion of a new item in segments measure table
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
