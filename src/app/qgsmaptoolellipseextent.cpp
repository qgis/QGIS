/***************************************************************************
    qgmaptoolellipseextent.cpp  -  map tool for adding ellipse
    from extent
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

#include "qgsmaptoolellipseextent.h"
#include "qgsgeometryrubberband.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include <QMouseEvent>

QgsMapToolEllipseExtent::QgsMapToolEllipseExtent( QgsMapToolCapture *parentTool,
    QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolAddEllipse( parentTool, canvas, mode )
{
}

void QgsMapToolEllipseExtent::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsPoint mapPoint( e->mapPoint() );

  if ( e->button() == Qt::LeftButton )
  {
    mPoints.append( mapPoint );

    if ( !mPoints.isEmpty() && !mTempRubberBand )
    {
      mTempRubberBand = createGeometryRubberBand( ( mode() == CapturePolygon ) ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, true );
      mTempRubberBand->show();
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

void QgsMapToolEllipseExtent::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsPoint mapPoint( e->mapPoint() );

  if ( mTempRubberBand )
  {
    switch ( mPoints.size() )
    {
      case 1:
      {
        if ( qgsDoubleNear( mCanvas->rotation(), 0.0 ) )
        {
          mEllipse = QgsEllipse().fromExtent( mPoints.at( 0 ), mapPoint );
          mTempRubberBand->setGeometry( mEllipse.toPolygon( segments() ) );
        }
        else
        {
          double dist = mPoints.at( 0 ).distance( mapPoint );
          double angle = mPoints.at( 0 ).azimuth( mapPoint );

          mEllipse = QgsEllipse().fromExtent( mPoints.at( 0 ), mPoints.at( 0 ).project( dist, angle ) );
          mTempRubberBand->setGeometry( mEllipse.toPolygon( segments() ) );
        }
      }
      break;
      default:
        break;
    }
  }
}
