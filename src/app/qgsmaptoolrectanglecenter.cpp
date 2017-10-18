/***************************************************************************
    qgsmaptoolrectanglecenter.cpp  -  map tool for adding rectangle
    from center and a point
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

#include "qgsmaptoolrectanglecenter.h"
#include "qgsgeometryrubberband.h"
#include "qgsgeometryutils.h"
#include "qgsmapcanvas.h"
#include "qgslinestring.h"
#include "qgspoint.h"
#include <QMouseEvent>
#include <memory>

QgsMapToolRectangleCenter::QgsMapToolRectangleCenter( QgsMapToolCapture *parentTool,
    QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolAddRectangle( parentTool, canvas, mode )
{
}

void QgsMapToolRectangleCenter::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
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

void QgsMapToolRectangleCenter::cadCanvasMoveEvent( QgsMapMouseEvent *e )
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
          double xOffset = fabs( mapPoint.x() - mPoints.at( 0 ).x() );
          double yOffset = fabs( mapPoint.y() - mPoints.at( 0 ).y() );

          mRectangle = QgsRectangle( QgsPoint( mPoints.at( 0 ).x() - xOffset, mPoints.at( 0 ).y() - yOffset ), QgsPoint( mPoints.at( 0 ).x() + xOffset, mPoints.at( 0 ).y() + yOffset ) );

          mTempRubberBand->setGeometry( QgsMapToolAddRectangle::rectangleToPolygon() );
        }
        else
        {
          double dist = mPoints.at( 0 ).distance( mapPoint );
          double angle = mPoints.at( 0 ).azimuth( mapPoint );

          mRectangle = QgsRectangle( mPoints.at( 0 ).project( -dist, angle ), mPoints.at( 0 ).project( dist, angle ) );
          mTempRubberBand->setGeometry( QgsMapToolAddRectangle::rectangleToPolygon() );
        }
      }
      break;
      default:
        break;
    }
  }
}
