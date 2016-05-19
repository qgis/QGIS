/***************************************************************************
qgsmaptoolselectfreehand.cpp  -  map tool for selecting features by freehand
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

#include "qgsmaptoolselectfreehand.h"
#include "qgsmaptoolselectutils.h"
#include "qgsgeometry.h"
#include "qgsrubberband.h"
#include "qgsmapcanvas.h"
#include "qgis.h"

#include <QMouseEvent>


QgsMapToolSelectFreehand::QgsMapToolSelectFreehand( QgsMapCanvas* canvas )
    : QgsMapTool( canvas )
    , mDragging( false )
{
  mRubberBand = nullptr;
  mCursor = Qt::ArrowCursor;
  mFillColor = QColor( 254, 178, 76, 63 );
  mBorderColour = QColor( 254, 58, 29, 100 );
}

QgsMapToolSelectFreehand::~QgsMapToolSelectFreehand()
{
  delete mRubberBand;
}

void QgsMapToolSelectFreehand::canvasPressEvent( QgsMapMouseEvent* e )
{
  if ( e->button() != Qt::LeftButton )
    return;

  if ( !mRubberBand )
  {
    mRubberBand = new QgsRubberBand( mCanvas, QGis::Polygon );
    mRubberBand->setFillColor( mFillColor );
    mRubberBand->setBorderColor( mBorderColour );
  }
  mRubberBand->addPoint( toMapCoordinates( e->pos() ) );
  mDragging = true;
}


void QgsMapToolSelectFreehand::canvasMoveEvent( QgsMapMouseEvent* e )
{
  if ( !mDragging || !mRubberBand )
    return;

  mRubberBand->addPoint( toMapCoordinates( e->pos() ) );
}


void QgsMapToolSelectFreehand::canvasReleaseEvent( QgsMapMouseEvent* e )
{
  if ( !mRubberBand )
    return;

  bool singleSelect = false;
  if ( mRubberBand->numberOfVertices() > 0 && mRubberBand->numberOfVertices() <= 2 )
  {
    // single click, not drag - create a rectangle around clicked point
    QgsVectorLayer* vlayer = QgsMapToolSelectUtils::getCurrentVectorLayer( mCanvas );
    if ( vlayer )
    {
      QRect selectRect;
      QgsMapToolSelectUtils::expandSelectRectangle( selectRect, vlayer, e->pos() );
      QgsMapToolSelectUtils::setRubberBand( mCanvas, selectRect, mRubberBand );
      singleSelect = true;
    }
  }

  if ( mRubberBand->numberOfVertices() > 2 )
  {
    QgsGeometry* shapeGeom = mRubberBand->asGeometry();
    if ( singleSelect )
      QgsMapToolSelectUtils::selectSingleFeature( mCanvas, shapeGeom, e );
    else
      QgsMapToolSelectUtils::selectMultipleFeatures( mCanvas, shapeGeom, e );
    delete shapeGeom;
  }

  mRubberBand->reset( QGis::Polygon );
  delete mRubberBand;
  mRubberBand = nullptr;
  mDragging = false;
}
