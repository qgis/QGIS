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

#include <QMouseEvent>
#include <cmath>

QgsMapToolMeasureAngle::QgsMapToolMeasureAngle( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )

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
  if ( !mRubberBand || mAnglePoints.empty() || mAnglePoints.size() > 2 )
  {
    return;
  }

  QgsPointXY point = snapPoint( e->pos() );
  mRubberBand->movePoint( point );
  if ( mAnglePoints.size() == 2 )
  {
    if ( !mResultDisplay->isVisible() )
    {
      mResultDisplay->move( e->pos() - QPoint( 100, 100 ) );
      mResultDisplay->show();
    }

    //angle calculation
    double azimuthOne = mDa.bearing( mAnglePoints.at( 1 ), mAnglePoints.at( 0 ) );
    double azimuthTwo = mDa.bearing( mAnglePoints.at( 1 ), point );
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

    mResultDisplay->setValueInRadians( resultAngle );
  }
}

void QgsMapToolMeasureAngle::canvasReleaseEvent( QgsMapMouseEvent *e )
{
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
    QgsPointXY newPoint = snapPoint( e->pos() );
    mAnglePoints.push_back( newPoint );
    mRubberBand->addPoint( newPoint );
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
  stopMeasuring();
  QgsMapTool::deactivate();
}

void QgsMapToolMeasureAngle::createRubberBand()
{
  delete mRubberBand;
  mRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::LineGeometry );

  QgsSettings settings;
  int myRed = settings.value( QStringLiteral( "qgis/default_measure_color_red" ), 180 ).toInt();
  int myGreen = settings.value( QStringLiteral( "qgis/default_measure_color_green" ), 180 ).toInt();
  int myBlue = settings.value( QStringLiteral( "qgis/default_measure_color_blue" ), 180 ).toInt();
  mRubberBand->setColor( QColor( myRed, myGreen, myBlue, 100 ) );
  mRubberBand->setWidth( 3 );
}

QgsPointXY QgsMapToolMeasureAngle::snapPoint( QPoint p )
{
  QgsPointLocator::Match m = mCanvas->snappingUtils()->snapToMap( p );
  return m.isValid() ? m.point() : mCanvas->getCoordinateTransform()->toMapCoordinates( p );
}

void QgsMapToolMeasureAngle::updateSettings()
{
  if ( mAnglePoints.size() != 3 )
    return;

  if ( !mResultDisplay )
    return;

  configureDistanceArea();

  //angle calculation
  double azimuthOne = mDa.bearing( mAnglePoints.at( 1 ), mAnglePoints.at( 0 ) );
  double azimuthTwo = mDa.bearing( mAnglePoints.at( 1 ), mAnglePoints.at( 2 ) );
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

  mResultDisplay->setValueInRadians( resultAngle );
}

void QgsMapToolMeasureAngle::configureDistanceArea()
{
  QString ellipsoidId = QgsProject::instance()->ellipsoid();
  mDa.setSourceCrs( mCanvas->mapSettings().destinationCrs() );
  mDa.setEllipsoid( ellipsoidId );
}
