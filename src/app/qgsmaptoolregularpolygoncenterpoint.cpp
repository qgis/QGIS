/***************************************************************************
    qgsmaptoolregularpolygoncenterpoint.cpp  -  map tool for adding regular
    polygon from center and a point
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017 by Loïc Bartoletti
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolregularpolygoncenterpoint.h"
#include "qgsgeometryrubberband.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include <QMouseEvent>

QgsMapToolRegularPolygonCenterPoint::QgsMapToolRegularPolygonCenterPoint( QgsMapToolCapture *parentTool,
    QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolAddRegularPolygon( parentTool, canvas, mode )
{
}

QgsMapToolRegularPolygonCenterPoint::~QgsMapToolRegularPolygonCenterPoint()
{
  deleteNumberSidesSpinBox();
}

void QgsMapToolRegularPolygonCenterPoint::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsPoint point = mapPoint( *e );

  if ( e->button() == Qt::LeftButton )
  {
    mPoints.append( point );

    if ( !mPoints.isEmpty() )
    {
      if ( !mTempRubberBand )
      {
        mTempRubberBand = createGeometryRubberBand( mLayerType, true );
        mTempRubberBand->show();

        createNumberSidesSpinBox();
      }
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    mPoints.append( point );

    deactivate();
    if ( mParentTool )
    {
      mParentTool->canvasReleaseEvent( e );
    }
  }
}

void QgsMapToolRegularPolygonCenterPoint::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsPoint point = mapPoint( *e );
  if ( mTempRubberBand )
  {
    QgsRegularPolygon::ConstructionOption option = QgsRegularPolygon::CircumscribedCircle;
    mRegularPolygon = QgsRegularPolygon( mPoints.at( 0 ), point, mNumberSidesSpinBox->value(), option );
    mTempRubberBand->setGeometry( mRegularPolygon.toPolygon() );
  }
}
