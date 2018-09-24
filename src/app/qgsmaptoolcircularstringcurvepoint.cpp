/***************************************************************************
    qgsmaptoolcircularstringcurvepoint.cpp
    ---------------------
    begin                : August 2015
    copyright            : (C) 2015 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmaptoolcircularstringcurvepoint.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgsgeometryrubberband.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include <QMouseEvent>

QgsMapToolCircularStringCurvePoint::QgsMapToolCircularStringCurvePoint( QgsMapToolCapture *parentTool,
    QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolAddCircularString( parentTool, canvas, mode )
{

}

void QgsMapToolCircularStringCurvePoint::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsPoint point = mapPoint( *e );

  if ( e->button() == Qt::LeftButton )
  {
    mPoints.append( point );
    if ( !mCenterPointRubberBand && mShowCenterPointRubberBand )
    {
      createCenterPointRubberBand();
    }

    if ( !mPoints.isEmpty() )
    {
      if ( !mTempRubberBand )
      {
        mTempRubberBand = createGeometryRubberBand( layerType, true );
        mTempRubberBand->show();
      }

      QgsCircularString *c = new QgsCircularString();
      QgsPointSequence rubberBandPoints = mPoints.mid( mPoints.size() - 1 - ( mPoints.size() + 1 ) % 2 );
      rubberBandPoints.append( point );
      c->setPoints( rubberBandPoints );
      mTempRubberBand->setGeometry( c );
    }
    if ( mPoints.size() > 1 && mPoints.size() % 2 )
    {
      if ( !mRubberBand )
      {
        mRubberBand = createGeometryRubberBand( layerType );
        mRubberBand->show();
      }

      QgsCircularString *c = new QgsCircularString();
      QgsPointSequence rubberBandPoints = mPoints;
      rubberBandPoints.append( point );
      c->setPoints( rubberBandPoints );
      mRubberBand->setGeometry( c );
      removeCenterPointRubberBand();
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    deactivate();
    if ( mParentTool )
    {
      mParentTool->canvasReleaseEvent( e );
    }
  }
}

void QgsMapToolCircularStringCurvePoint::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsPoint mapPoint( e->mapPoint() );
  QgsVertexId idx( 0, 0, 1 + ( mPoints.size() + 1 ) % 2 );
  if ( mTempRubberBand )
  {
    mTempRubberBand->moveVertex( idx, mapPoint );
    updateCenterPointRubberBand( mapPoint );
  }
}
