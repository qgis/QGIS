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
#include "qgsmaptopixel.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgssnappingutils.h"
#include "qgstolerance.h"
#include "qgsexception.h"
#include "qgsmeasuredialog.h"
#include "qgsmeasuretool.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"
#include "qgsproject.h"
#include "qgssnapindicator.h"
#include "qgsmapmouseevent.h"

#include <QMessageBox>


QgsMeasureTool::QgsMeasureTool( QgsMapCanvas *canvas, bool measureArea )
  : QgsMapTool( canvas )
  , mMeasureArea( measureArea )
  , mSnapIndicator( new QgsSnapIndicator( canvas ) )
{
  mRubberBand = new QgsRubberBand( canvas, mMeasureArea ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
  mRubberBandPoints = new QgsRubberBand( canvas, QgsWkbTypes::PointGeometry );

  // Append point we will move
  mPoints.append( QgsPointXY( 0, 0 ) );
  mDestinationCrs = canvas->mapSettings().destinationCrs();

  mDialog = new QgsMeasureDialog( this );
  mDialog->setWindowFlags( mDialog->windowFlags() | Qt::Tool );
  mDialog->restorePosition();

  connect( canvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsMeasureTool::updateSettings );
}

QgsMeasureTool::~QgsMeasureTool()
{
  // important - dialog is not parented to this tool (it's parented to the main window)
  // but we want to clean it up now
  delete mDialog;
}

QVector<QgsPointXY> QgsMeasureTool::points() const
{
  return mPoints;
}


void QgsMeasureTool::activate()
{
  mDialog->show();
  mRubberBand->show();
  mRubberBandPoints->show();
  QgsMapTool::activate();

  // ensure that we have correct settings
  updateSettings();

  // If we suspect that they have data that is projected, yet the
  // map CRS is set to a geographic one, warn them.
  if ( mCanvas->mapSettings().destinationCrs().isValid() &&
       mCanvas->mapSettings().destinationCrs().isGeographic() &&
       ( mCanvas->extent().height() > 360 ||
         mCanvas->extent().width() > 720 ) )
  {
    QMessageBox::warning( nullptr, tr( "Incorrect Measure Results" ),
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
  mSnapIndicator->setMatch( QgsPointLocator::Match() );

  mDialog->hide();
  mRubberBand->hide();
  mRubberBandPoints->hide();
  QgsMapTool::deactivate();
}

void QgsMeasureTool::restart()
{
  mPoints.clear();

  mRubberBand->reset( mMeasureArea ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
  mRubberBandPoints->reset( QgsWkbTypes::PointGeometry );

  mDone = true;
  mWrongProjectProjection = false;
}

void QgsMeasureTool::updateSettings()
{
  const QgsSettings settings;

  const int myRed = settings.value( QStringLiteral( "qgis/default_measure_color_red" ), 222 ).toInt();
  const int myGreen = settings.value( QStringLiteral( "qgis/default_measure_color_green" ), 155 ).toInt();
  const int myBlue = settings.value( QStringLiteral( "qgis/default_measure_color_blue" ), 67 ).toInt();
  mRubberBand->setColor( QColor( myRed, myGreen, myBlue, 100 ) );
  mRubberBand->setWidth( 3 );
  mRubberBandPoints->setIcon( QgsRubberBand::ICON_CIRCLE );
  mRubberBandPoints->setIconSize( 10 );
  mRubberBandPoints->setColor( QColor( myRed, myGreen, myBlue, 150 ) );

  // Reproject the points to the new destination CoordinateReferenceSystem
  if ( mRubberBand->size() > 0 && mDestinationCrs != mCanvas->mapSettings().destinationCrs() && mCanvas->mapSettings().destinationCrs().isValid() )
  {
    const QVector<QgsPointXY> points = mPoints;
    const bool lastDone = mDone;

    mDialog->restart();
    mDone = lastDone;
    const QgsCoordinateTransform ct( mDestinationCrs, mCanvas->mapSettings().destinationCrs(), QgsProject::instance() );

    const auto constPoints = points;
    for ( const QgsPointXY &previousPoint : constPoints )
    {
      try
      {
        const QgsPointXY point = ct.transform( previousPoint );

        mPoints.append( point );
        mRubberBand->addPoint( point, false );
        mRubberBandPoints->addPoint( point, false );
      }
      catch ( QgsCsException &cse )
      {
        QgsMessageLog::logMessage( tr( "Transform error caught at the MeasureTool: %1" ).arg( cse.what() ) );
      }
    }

    mRubberBand->updatePosition();
    mRubberBand->update();
    mRubberBandPoints->updatePosition();
    mRubberBandPoints->update();
  }
  mDestinationCrs = mCanvas->mapSettings().destinationCrs();

  mDialog->updateSettings();

  if ( !mDone && mRubberBand->size() > 0 )
  {
    mRubberBand->addPoint( mPoints.last() );
    mDialog->addPoint();
  }
  if ( mRubberBand->size() > 0 )
  {
    mRubberBand->setVisible( true );
    mRubberBandPoints->setVisible( true );
  }
}

//////////////////////////

void QgsMeasureTool::canvasPressEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
}

void QgsMeasureTool::canvasMoveEvent( QgsMapMouseEvent *e )
{
  const QgsPointXY point = e->snapPoint();
  mSnapIndicator->setMatch( e->mapPointMatch() );

  if ( ! mDone )
  {
    mRubberBand->movePoint( point );
    mDialog->mouseMove( point );
  }
}


void QgsMeasureTool::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  const QgsPointXY point = e->snapPoint();

  if ( mDone ) // if we have stopped measuring any mouse click restart measuring
  {
    mDialog->restart();
  }

  if ( e->button() == Qt::RightButton ) // if we clicked the right button we stop measuring
  {
    mDone = true;
    mRubberBand->removeLastPoint();
    mDialog->removeLastPoint();
  }
  else if ( e->button() == Qt::LeftButton )
  {
    mDone = false;
    addPoint( point );
  }

  mDialog->show();

}

void QgsMeasureTool::undo()
{
  if ( mRubberBand )
  {
    if ( mPoints.empty() )
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

void QgsMeasureTool::keyPressEvent( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Escape )
  {
    mDialog->restart();
  }
  else if ( ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete ) )
  {
    if ( !mDone )
    {
      undo();
    }

    // Override default shortcut management in MapCanvas
    e->ignore();
  }
}


void QgsMeasureTool::addPoint( const QgsPointXY &point )
{
  QgsDebugMsg( "point=" + point.toString() );

  // don't add points with the same coordinates
  if ( !mPoints.isEmpty() && mPoints.last() == point )
  {
    return;
  }

  const QgsPointXY pnt( point );
  // Append point that we will be moving.
  mPoints.append( pnt );

  mRubberBand->addPoint( point );
  mRubberBandPoints->addPoint( point );
  if ( ! mDone )    // Prevent the insertion of a new item in segments measure table
  {
    mDialog->addPoint();
  }
}
