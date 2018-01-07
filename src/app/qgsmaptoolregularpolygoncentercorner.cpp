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
#include <QMouseEvent>

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
  QgsPoint mapPoint( e->mapPoint() );

  if ( e->button() == Qt::LeftButton )
  {
    mPoints.append( mapPoint );

    if ( !mPoints.isEmpty() )
    {
      if ( !mTempRubberBand )
      {
        mTempRubberBand = createGeometryRubberBand( ( mode() == CapturePolygon ) ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, true );
        mTempRubberBand->show();

        createNumberSidesSpinBox();
      }
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

void QgsMapToolRegularPolygonCenterCorner::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsPoint mapPoint( e->mapPoint() );
  if ( mTempRubberBand )
  {
    QgsRegularPolygon::ConstructionOption option = QgsRegularPolygon::InscribedCircle;
    mRegularPolygon = QgsRegularPolygon( mPoints.at( 0 ), mapPoint, mNumberSidesSpinBox->value(), option );
    mTempRubberBand->setGeometry( mRegularPolygon.toPolygon() );
  }
}
