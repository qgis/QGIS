/***************************************************************************
    qgsmaptoolmeasurebearing.cpp
    ------------------------
    begin                : June 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolmeasurebearing.h"
#include "qgsdisplayangle.h"
#include "qgsdistancearea.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgssnappingutils.h"
#include "qgssettings.h"
#include "qgssnapindicator.h"
#include "qgsmapmouseevent.h"

#include <cmath>

QgsMapToolMeasureBearing::QgsMapToolMeasureBearing( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
  , mSnapIndicator( new QgsSnapIndicator( canvas ) )
{
  mToolName = tr( "Measure bearing" );

  connect( canvas, &QgsMapCanvas::destinationCrsChanged,
           this, &QgsMapToolMeasureBearing::updateSettings );
}

QgsMapToolMeasureBearing::~QgsMapToolMeasureBearing()
{
  stopMeasuring();
}

void QgsMapToolMeasureBearing::canvasMoveEvent( QgsMapMouseEvent *e )
{
  const QgsPointXY point = e->snapPoint();
  mSnapIndicator->setMatch( e->mapPointMatch() );

  if ( !mRubberBand || mAnglePoints.empty() || mAnglePoints.size() >= 2 )
  {
    return;
  }

  mRubberBand->movePoint( point );
  if ( mAnglePoints.size() == 1 )
  {
    try
    {
      const double bearing = mDa.bearing( mAnglePoints.at( 0 ), point );
      mResultDisplay->setBearingInRadians( bearing );

      if ( !mResultDisplay->isVisible() )
      {
        mResultDisplay->move( e->pos() - QPoint( 100, 100 ) );
        mResultDisplay->show();
      }
    }
    catch ( QgsCsException & )
    {

    }
  }
}

void QgsMapToolMeasureBearing::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  // if we clicked the right button we cancel the operation, unless it's the "final" click
  if ( e->button() == Qt::RightButton && mAnglePoints.size() != 1 )
  {
    stopMeasuring();
    return;
  }

  //add points until we have two
  if ( mAnglePoints.size() == 2 )
  {
    mAnglePoints.clear();
  }

  if ( mAnglePoints.empty() )
  {
    if ( !mResultDisplay )
    {
      mResultDisplay = new QgsDisplayAngle( this );
      mResultDisplay->setWindowFlags( mResultDisplay->windowFlags() | Qt::Tool );
      mResultDisplay->setWindowTitle( tr( "Bearing" ) );
      connect( mResultDisplay, &QDialog::rejected, this, &QgsMapToolMeasureBearing::stopMeasuring );
    }
    configureDistanceArea();
    createRubberBand();
  }

  if ( mAnglePoints.size() < 2 )
  {
    const QgsPointXY newPoint = e->snapPoint();
    mAnglePoints.push_back( newPoint );
    mRubberBand->addPoint( newPoint );
  }
}

void QgsMapToolMeasureBearing::keyPressEvent( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Escape )
  {
    stopMeasuring();
  }
  else if ( ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete ) )
  {
    if ( !mAnglePoints.empty() && mRubberBand )
    {
      if ( mAnglePoints.size() == 1 )
      {
        //removing first point, so restart everything
        stopMeasuring();
      }
      else
      {
        //remove second last point from line band, and last point from points band
        mRubberBand->removePoint( -2, true );
        mAnglePoints.removeLast();
      }
    }

    // Override default shortcut management in MapCanvas
    e->ignore();
  }
}

void QgsMapToolMeasureBearing::stopMeasuring()
{
  delete mRubberBand;
  mRubberBand = nullptr;
  delete mResultDisplay;
  mResultDisplay = nullptr;
  mAnglePoints.clear();
}

void QgsMapToolMeasureBearing::activate()
{
  QgsMapTool::activate();
}

void QgsMapToolMeasureBearing::deactivate()
{
  mSnapIndicator->setMatch( QgsPointLocator::Match() );

  stopMeasuring();
  QgsMapTool::deactivate();
}

void QgsMapToolMeasureBearing::createRubberBand()
{
  delete mRubberBand;
  mRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::LineGeometry );

  const QgsSettings settings;
  const int myRed = settings.value( QStringLiteral( "qgis/default_measure_color_red" ), 180 ).toInt();
  const int myGreen = settings.value( QStringLiteral( "qgis/default_measure_color_green" ), 180 ).toInt();
  const int myBlue = settings.value( QStringLiteral( "qgis/default_measure_color_blue" ), 180 ).toInt();
  mRubberBand->setColor( QColor( myRed, myGreen, myBlue, 100 ) );
  mRubberBand->setWidth( 3 );
}

void QgsMapToolMeasureBearing::updateSettings()
{
  if ( mAnglePoints.size() != 2 )
    return;

  if ( !mResultDisplay )
    return;

  configureDistanceArea();

  try
  {
    const double bearing = mDa.bearing( mAnglePoints.at( 0 ), mAnglePoints.at( 1 ) );
    mResultDisplay->setBearingInRadians( bearing );
  }
  catch ( QgsCsException & )
  {}
}

void QgsMapToolMeasureBearing::configureDistanceArea()
{
  const QString ellipsoidId = QgsProject::instance()->ellipsoid();
  mDa.setSourceCrs( mCanvas->mapSettings().destinationCrs(), QgsProject::instance()->transformContext() );
  mDa.setEllipsoid( ellipsoidId );
}
