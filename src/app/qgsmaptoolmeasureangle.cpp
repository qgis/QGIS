/***************************************************************************
    qgsmaptoolmeasureangle.cpp
    --------------------------
    begin                : December 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolmeasureangle.h"
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

QgsMapToolMeasureAngle::QgsMapToolMeasureAngle( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
  , mSnapIndicator( new QgsSnapIndicator( canvas ) )
{
  mToolName = tr( "Measure angle" );

  connect( canvas, &QgsMapCanvas::destinationCrsChanged,
           this, &QgsMapToolMeasureAngle::updateSettings );
}

QgsMapToolMeasureAngle::~QgsMapToolMeasureAngle()
{
  stopMeasuring();
}

void QgsMapToolMeasureAngle::canvasMoveEvent( QgsMapMouseEvent *e )
{
  const QgsPointXY point = e->snapPoint();
  mSnapIndicator->setMatch( e->mapPointMatch() );

  if ( !mRubberBand || mAnglePoints.empty() || mAnglePoints.size() > 2 )
  {
    return;
  }

  mRubberBand->movePoint( point );
  if ( mAnglePoints.size() == 2 )
  {
    double azimuthOne = 0;
    double azimuthTwo = 0;
    try
    {
      azimuthOne = mDa.bearing( mAnglePoints.at( 1 ), mAnglePoints.at( 0 ) );
      azimuthTwo = mDa.bearing( mAnglePoints.at( 1 ), point );
    }
    catch ( QgsCsException & )
    {
      return;
    }

    if ( !mResultDisplay->isVisible() )
    {
      mResultDisplay->move( e->pos() - QPoint( 100, 100 ) );
      mResultDisplay->show();
    }

    //angle calculation
    double resultAngle = azimuthTwo - azimuthOne;
    QgsDebugMsg( QString::number( std::fabs( resultAngle ) ) );
    QgsDebugMsg( QString::number( M_PI ) );
    if ( std::fabs( resultAngle ) > M_PI )
    {
      if ( resultAngle < 0 )
      {
        resultAngle = M_PI + ( resultAngle + M_PI );
      }
      else
      {
        resultAngle = -M_PI + ( resultAngle - M_PI );
      }
    }

    mResultDisplay->setAngleInRadians( resultAngle );
  }
}

void QgsMapToolMeasureAngle::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  // if we clicked the right button we cancel the operation, unless it's the "final" click
  if ( e->button() == Qt::RightButton && mAnglePoints.size() != 2 )
  {
    stopMeasuring();
    return;
  }

  //add points until we have three
  if ( mAnglePoints.size() == 3 )
  {
    mAnglePoints.clear();
  }

  if ( mAnglePoints.empty() )
  {
    if ( !mResultDisplay )
    {
      mResultDisplay = new QgsDisplayAngle( this );
      mResultDisplay->setWindowFlags( mResultDisplay->windowFlags() | Qt::Tool );
      connect( mResultDisplay, &QDialog::rejected, this, &QgsMapToolMeasureAngle::stopMeasuring );
    }
    configureDistanceArea();
    createRubberBand();
  }

  if ( mAnglePoints.size() < 3 )
  {
    const QgsPointXY newPoint = e->snapPoint();
    mAnglePoints.push_back( newPoint );
    mRubberBand->addPoint( newPoint );
  }
}

void QgsMapToolMeasureAngle::keyPressEvent( QKeyEvent *e )
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

void QgsMapToolMeasureAngle::stopMeasuring()
{
  delete mRubberBand;
  mRubberBand = nullptr;
  delete mResultDisplay;
  mResultDisplay = nullptr;
  mAnglePoints.clear();
}

void QgsMapToolMeasureAngle::activate()
{
  QgsMapTool::activate();
}

void QgsMapToolMeasureAngle::deactivate()
{
  mSnapIndicator->setMatch( QgsPointLocator::Match() );

  stopMeasuring();
  QgsMapTool::deactivate();
}

void QgsMapToolMeasureAngle::createRubberBand()
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

void QgsMapToolMeasureAngle::updateSettings()
{
  if ( mAnglePoints.size() != 3 )
    return;

  if ( !mResultDisplay )
    return;

  configureDistanceArea();

  //angle calculation
  double azimuthOne = 0;
  double azimuthTwo = 0;
  try
  {
    azimuthOne = mDa.bearing( mAnglePoints.at( 1 ), mAnglePoints.at( 0 ) );
    azimuthTwo = mDa.bearing( mAnglePoints.at( 1 ), mAnglePoints.at( 2 ) );
  }
  catch ( QgsCsException & )
  {
    return;
  }
  double resultAngle = azimuthTwo - azimuthOne;
  QgsDebugMsg( QString::number( std::fabs( resultAngle ) ) );
  QgsDebugMsg( QString::number( M_PI ) );
  if ( std::fabs( resultAngle ) > M_PI )
  {
    if ( resultAngle < 0 )
    {
      resultAngle = M_PI + ( resultAngle + M_PI );
    }
    else
    {
      resultAngle = -M_PI + ( resultAngle - M_PI );
    }
  }

  mResultDisplay->setAngleInRadians( resultAngle );
}

void QgsMapToolMeasureAngle::configureDistanceArea()
{
  const QString ellipsoidId = QgsProject::instance()->ellipsoid();
  mDa.setSourceCrs( mCanvas->mapSettings().destinationCrs(), QgsProject::instance()->transformContext() );
  mDa.setEllipsoid( ellipsoidId );
}
