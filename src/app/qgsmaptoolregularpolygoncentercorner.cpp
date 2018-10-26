/***************************************************************************
    qgsmaptoolregularpolygoncentercorner.cpp  -  map tool for adding regular
    polygon from center and a corner
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

#include "qgsmaptoolregularpolygoncentercorner.h"
#include "qgsgeometryrubberband.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgsmapmouseevent.h"
#include "qgssnapindicator.h"


QgsMapToolRegularPolygonCenterCorner::QgsMapToolRegularPolygonCenterCorner( QgsMapToolCapture *parentTool,
    QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolAddRegularPolygon( parentTool, canvas, mode )
{
}

QgsMapToolRegularPolygonCenterCorner::~QgsMapToolRegularPolygonCenterCorner()
{
  deleteNumberSidesSpinBox();
}

void QgsMapToolRegularPolygonCenterCorner::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
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

void QgsMapToolRegularPolygonCenterCorner::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsPoint point = mapPoint( *e );

  mSnapIndicator->setMatch( e->mapPointMatch() );

  if ( mTempRubberBand )
  {
    QgsRegularPolygon::ConstructionOption option = QgsRegularPolygon::InscribedCircle;
    mRegularPolygon = QgsRegularPolygon( mPoints.at( 0 ), point, mNumberSidesSpinBox->value(), option );
    mTempRubberBand->setGeometry( mRegularPolygon.toPolygon() );
  }
}
