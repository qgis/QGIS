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
    : QgsMapTool( canvas ), mDragging( false )
{
  mRubberBand = 0;
  mCursor = Qt::ArrowCursor;
}

QgsMapToolSelectRadius::~QgsMapToolSelectRadius()
{
  delete mRubberBand;
}

void QgsMapToolSelectRadius::canvasPressEvent( QMouseEvent * e )
{
  if ( e->button() != Qt::LeftButton )
  {
    return;
  }
  mRadiusCenter = toMapCoordinates( e->pos() );
}


void QgsMapToolSelectRadius::canvasMoveEvent( QMouseEvent * e )
{
  if ( e->buttons() != Qt::LeftButton )
  {
    return;
  }
  if ( !mDragging )
  {
    if ( mRubberBand == NULL )
    {
      mRubberBand = new QgsRubberBand( mCanvas, true );
    }
    mDragging = true;
  }
  QgsPoint radiusEdge = toMapCoordinates( e->pos() );
  setRadiusRubberBand( radiusEdge );
}


void QgsMapToolSelectRadius::canvasReleaseEvent( QMouseEvent * e )
{
  if ( e->button() != Qt::LeftButton )
  {
    return;
  }
  if ( !mDragging )
  {
    if ( mRubberBand == NULL )
    {
      mRubberBand = new QgsRubberBand( mCanvas, true );
    }
    mRadiusCenter = toMapCoordinates( e->pos() );
    QgsPoint radiusEdge = toMapCoordinates( QPoint( e->pos().x() + 1, e->pos().y() + 1 ) );
    setRadiusRubberBand( radiusEdge );
  }
  QgsGeometry* radiusGeometry = mRubberBand->asGeometry();
  QgsMapToolSelectUtils::setSelectFeatures( mCanvas, radiusGeometry, e );
  delete radiusGeometry;
  mRubberBand->reset( true );
  delete mRubberBand;
  mRubberBand = 0;
  mDragging = false;
}


void QgsMapToolSelectRadius::setRadiusRubberBand( QgsPoint & radiusEdge )
{
  double r = sqrt( mRadiusCenter.sqrDist( radiusEdge ) );
  mRubberBand->reset( true );
  for ( int i = 0; i <= RADIUS_SEGMENTS; ++i )
  {
    double theta = i * ( 2.0 * M_PI / RADIUS_SEGMENTS );
    QgsPoint radiusPoint( mRadiusCenter.x() + r * cos( theta ),
                          mRadiusCenter.y() + r * sin( theta ) );
    mRubberBand->addPoint( radiusPoint );
  }
}
