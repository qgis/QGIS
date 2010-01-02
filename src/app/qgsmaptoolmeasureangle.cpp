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
#include "qgsrubberband.h"
#include <QMouseEvent>
#include <QSettings>

#ifndef Q_OS_MACX
#include <cmath>
#else
#include <math.h>
#endif

QgsMapToolMeasureAngle::QgsMapToolMeasureAngle( QgsMapCanvas* canvas ): QgsMapTool( canvas ), mRubberBand( 0 ), mResultDisplay( 0 )
{
  mSnapper.setMapCanvas( canvas );
}

QgsMapToolMeasureAngle::~QgsMapToolMeasureAngle()
{
  stopMeasuring();
}

void QgsMapToolMeasureAngle::canvasMoveEvent( QMouseEvent * e )
{
  if ( !mRubberBand || mAnglePoints.size() < 1 || mAnglePoints.size() > 2 || !mRubberBand )
  {
    return;
  }

  QgsPoint point = snapPoint( e->pos() );
  mRubberBand->movePoint( point );
  if ( mAnglePoints.size() == 2 )
  {
    //do angle calculation
    QgsDistanceArea* distArea = mCanvas->mapRenderer()->distanceArea();
    if ( distArea )
    {
      double azimutOne = distArea->bearing( mAnglePoints.at( 1 ), mAnglePoints.at( 0 ) );
      double azimutTwo = distArea->bearing( mAnglePoints.at( 1 ), point );
      double resultAngle = azimutTwo - azimutOne;
      QgsDebugMsg( QString::number( fabs( resultAngle ) ) );
      QgsDebugMsg( QString::number( M_PI ) );
      if ( fabs( resultAngle ) > M_PI )
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

      //show angle in dialog
      if ( !mResultDisplay )
      {
        mResultDisplay = new QgsDisplayAngle( mCanvas->topLevelWidget() );
        QObject::connect( mResultDisplay, SIGNAL( rejected() ), this, SLOT( stopMeasuring() ) );
        mResultDisplay->move( e->pos() - QPoint( 100, 100 ) );
      }
      mResultDisplay->show();
      mResultDisplay->setValueInRadians( resultAngle );
    }
  }
}

void QgsMapToolMeasureAngle::canvasReleaseEvent( QMouseEvent * e )
{
  //add points until we have three
  if ( mAnglePoints.size() == 3 )
  {
    mAnglePoints.clear();
  }

  if ( mAnglePoints.size() < 1 )
  {
    createRubberBand();
  }

  if ( mAnglePoints.size() < 3 )
  {
    QgsPoint newPoint = snapPoint( e->pos() );
    mAnglePoints.push_back( newPoint );
    mRubberBand->addPoint( newPoint );
  }
}

void QgsMapToolMeasureAngle::stopMeasuring()
{
  delete mRubberBand;
  mRubberBand = 0;
  delete mResultDisplay;
  mResultDisplay = 0;
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
  mRubberBand = new QgsRubberBand( mCanvas, false );

  QSettings settings;
  int myRed = settings.value( "/qgis/default_measure_color_red", 180 ).toInt();
  int myGreen = settings.value( "/qgis/default_measure_color_green", 180 ).toInt();
  int myBlue = settings.value( "/qgis/default_measure_color_blue", 180 ).toInt();
  mRubberBand->setColor( QColor( myRed, myGreen, myBlue ) );
}

QgsPoint QgsMapToolMeasureAngle::snapPoint( const QPoint& p )
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



