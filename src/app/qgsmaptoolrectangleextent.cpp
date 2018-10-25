/***************************************************************************
    qgsmaptoolrectangleextent.cpp  -  map tool for adding rectangle
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

#include "qgsmaptoolrectangleextent.h"
#include "qgsgeometryrubberband.h"
#include "qgsgeometryutils.h"
#include "qgsmapcanvas.h"
#include "qgslinestring.h"
#include "qgspoint.h"
#include "qgsmapmouseevent.h"
#include <memory>

QgsMapToolRectangleExtent::QgsMapToolRectangleExtent( QgsMapToolCapture *parentTool,
    QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolAddRectangle( parentTool, canvas, mode )
{
}

void QgsMapToolRectangleExtent::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsPoint point = mapPoint( *e );

  if ( e->button() == Qt::LeftButton )
  {
    mPoints.append( point );

    if ( !mPoints.isEmpty() && !mTempRubberBand )
    {
      mTempRubberBand = createGeometryRubberBand( mLayerType, true );
      mTempRubberBand->show();
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

void QgsMapToolRectangleExtent::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsPoint point = mapPoint( *e );

  if ( mTempRubberBand )
  {
    switch ( mPoints.size() )
    {
      case 1:
      {
        if ( qgsDoubleNear( mCanvas->rotation(), 0.0 ) )
        {
          mRectangle = QgsBox3d( mPoints.at( 0 ), point );
          mTempRubberBand->setGeometry( QgsMapToolAddRectangle::rectangleToPolygon( ) );
        }
        else
        {
          double dist = mPoints.at( 0 ).distance( point );
          double angle = mPoints.at( 0 ).azimuth( point );

          mRectangle = QgsBox3d( mPoints.at( 0 ), mPoints.at( 0 ).project( dist, angle ) );
          mTempRubberBand->setGeometry( QgsMapToolAddRectangle::rectangleToPolygon() );
        }
      }
      break;
      default:
        break;
    }
  }
}
