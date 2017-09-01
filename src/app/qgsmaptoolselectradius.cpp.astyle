/***************************************************************************
qgsmaptoolselectradius.cpp  -  map tool for selecting features by radius
---------------------
begin                : May 2010
copyright            : (C) 2010 by Jeremy Palmer
email                : jpalmer at linz dot govt dot nz
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsmaptoolselectradius.h"
#include "qgsmaptoolselectutils.h"
#include "qgsgeometry.h"
#include "qgsrubberband.h"
#include "qgsmapcanvas.h"
#include "qgis.h"
#include "qgslogger.h"

#include <cmath>
#include <QMouseEvent>

const int RADIUS_SEGMENTS = 40;

QgsMapToolSelectRadius::QgsMapToolSelectRadius( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
  , mDragging( false )
{
  mRubberBand = nullptr;
  mCursor = Qt::ArrowCursor;
  mFillColor = QColor( 254, 178, 76, 63 );
  mStrokeColor = QColor( 254, 58, 29, 100 );
}

QgsMapToolSelectRadius::~QgsMapToolSelectRadius()
{
  delete mRubberBand;
}

void QgsMapToolSelectRadius::canvasPressEvent( QgsMapMouseEvent *e )
{
  if ( e->button() != Qt::LeftButton )
    return;

  mRadiusCenter = toMapCoordinates( e->pos() );
}


void QgsMapToolSelectRadius::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( e->buttons() != Qt::LeftButton )
    return;

  if ( !mDragging )
  {
    if ( !mRubberBand )
    {
      mRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::PolygonGeometry );
      mRubberBand->setFillColor( mFillColor );
      mRubberBand->setStrokeColor( mStrokeColor );
    }
    mDragging = true;
  }
  QgsPointXY radiusEdge = toMapCoordinates( e->pos() );
  setRadiusRubberBand( radiusEdge );
}


void QgsMapToolSelectRadius::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( e->button() != Qt::LeftButton )
    return;

  if ( !mDragging )
  {
    if ( !mRubberBand )
    {
      mRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::PolygonGeometry );
      mRubberBand->setFillColor( mFillColor );
      mRubberBand->setStrokeColor( mStrokeColor );
    }
    mRadiusCenter = toMapCoordinates( e->pos() );
    QgsPointXY radiusEdge = toMapCoordinates( QPoint( e->pos().x() + 1, e->pos().y() + 1 ) );
    setRadiusRubberBand( radiusEdge );
  }
  QgsGeometry radiusGeometry = mRubberBand->asGeometry();
  QgsMapToolSelectUtils::selectMultipleFeatures( mCanvas, radiusGeometry, e );
  mRubberBand->reset( QgsWkbTypes::PolygonGeometry );
  delete mRubberBand;
  mRubberBand = nullptr;
  mDragging = false;
}


void QgsMapToolSelectRadius::setRadiusRubberBand( QgsPointXY &radiusEdge )
{
  double r = std::sqrt( mRadiusCenter.sqrDist( radiusEdge ) );
  mRubberBand->reset( QgsWkbTypes::PolygonGeometry );
  for ( int i = 0; i <= RADIUS_SEGMENTS; ++i )
  {
    double theta = i * ( 2.0 * M_PI / RADIUS_SEGMENTS );
    QgsPointXY radiusPoint( mRadiusCenter.x() + r * std::cos( theta ),
                            mRadiusCenter.y() + r * std::sin( theta ) );
    mRubberBand->addPoint( radiusPoint, false );
  }
  mRubberBand->closePoints( true );
}
