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

#ifndef M_PI
#define M_PI 3.1415926535897931159979634685
#endif

const int RADIUS_SEGMENTS = 40;

QgsMapToolSelectRadius::QgsMapToolSelectRadius( QgsMapCanvas* canvas )
    : QgsMapTool( canvas )
    , mDragging( false )
{
  mRubberBand = nullptr;
  mCursor = Qt::ArrowCursor;
  mFillColor = QColor( 254, 178, 76, 63 );
  mBorderColour = QColor( 254, 58, 29, 100 );
}

QgsMapToolSelectRadius::~QgsMapToolSelectRadius()
{
  delete mRubberBand;
}

void QgsMapToolSelectRadius::canvasPressEvent( QgsMapMouseEvent* e )
{
  if ( e->button() != Qt::LeftButton )
    return;

  mRadiusCenter = toMapCoordinates( e->pos() );
}


void QgsMapToolSelectRadius::canvasMoveEvent( QgsMapMouseEvent* e )
{
  if ( e->buttons() != Qt::LeftButton )
    return;

  if ( !mDragging )
  {
    if ( !mRubberBand )
    {
      mRubberBand = new QgsRubberBand( mCanvas, QGis::Polygon );
      mRubberBand->setFillColor( mFillColor );
      mRubberBand->setBorderColor( mBorderColour );
    }
    mDragging = true;
  }
  QgsPoint radiusEdge = toMapCoordinates( e->pos() );
  setRadiusRubberBand( radiusEdge );
}


void QgsMapToolSelectRadius::canvasReleaseEvent( QgsMapMouseEvent* e )
{
  if ( e->button() != Qt::LeftButton )
    return;

  if ( !mDragging )
  {
    if ( !mRubberBand )
    {
      mRubberBand = new QgsRubberBand( mCanvas, QGis::Polygon );
      mRubberBand->setFillColor( mFillColor );
      mRubberBand->setBorderColor( mBorderColour );
    }
    mRadiusCenter = toMapCoordinates( e->pos() );
    QgsPoint radiusEdge = toMapCoordinates( QPoint( e->pos().x() + 1, e->pos().y() + 1 ) );
    setRadiusRubberBand( radiusEdge );
  }
  QgsGeometry* radiusGeometry = mRubberBand->asGeometry();
  QgsMapToolSelectUtils::selectMultipleFeatures( mCanvas, radiusGeometry, e );
  delete radiusGeometry;
  mRubberBand->reset( QGis::Polygon );
  delete mRubberBand;
  mRubberBand = nullptr;
  mDragging = false;
}


void QgsMapToolSelectRadius::setRadiusRubberBand( QgsPoint & radiusEdge )
{
  double r = sqrt( mRadiusCenter.sqrDist( radiusEdge ) );
  mRubberBand->reset( QGis::Polygon );
  for ( int i = 0; i <= RADIUS_SEGMENTS; ++i )
  {
    double theta = i * ( 2.0 * M_PI / RADIUS_SEGMENTS );
    QgsPoint radiusPoint( mRadiusCenter.x() + r * cos( theta ),
                          mRadiusCenter.y() + r * sin( theta ) );
    mRubberBand->addPoint( radiusPoint, false );
  }
  mRubberBand->closePoints( true );
}
