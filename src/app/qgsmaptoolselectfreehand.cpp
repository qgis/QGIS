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


QgsMapToolSelectFreehand::QgsMapToolSelectFreehand( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
  mRubberBand = nullptr;
  mCursor = Qt::ArrowCursor;
  mFillColor = QColor( 254, 178, 76, 63 );
  mStrokeColor = QColor( 254, 58, 29, 100 );
}

QgsMapToolSelectFreehand::~QgsMapToolSelectFreehand()
{
  delete mRubberBand;
}


void QgsMapToolSelectFreehand::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( !mActive || !mRubberBand )
    return;

  mRubberBand->addPoint( toMapCoordinates( e->pos() ) );
}


void QgsMapToolSelectFreehand::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( !mActive )
  {
    if ( e->button() != Qt::LeftButton )
      return;

    if ( !mRubberBand )
    {
      mRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::PolygonGeometry );
      mRubberBand->setFillColor( mFillColor );
      mRubberBand->setStrokeColor( mStrokeColor );
    }
    else
    {
      mRubberBand->reset( QgsWkbTypes::PolygonGeometry );
    }
    mRubberBand->addPoint( toMapCoordinates( e->pos() ) );
    mActive = true;
  }
  else
  {
    if ( e->button() == Qt::LeftButton )
    {
      if ( mRubberBand && mRubberBand->numberOfVertices() > 2 )
      {
        QgsGeometry shapeGeom = mRubberBand->asGeometry();
        QgsMapToolSelectUtils::selectMultipleFeatures( mCanvas, shapeGeom, e->modifiers() );
      }
    }

    delete mRubberBand;
    mRubberBand = nullptr;
    mActive = false;
  }
}

void QgsMapToolSelectFreehand::keyReleaseEvent( QKeyEvent *e )
{
  if ( mActive && e->key() == Qt::Key_Escape )
  {
    delete mRubberBand;
    mRubberBand = nullptr;
    mActive = false;
    return;
  }
  QgsMapTool::keyReleaseEvent( e );
}
