/***************************************************************************
   qgsmaptoolrectangle3points.cpp  -  map tool for adding rectangle
   from 3 points
   ---------------------
   begin                : September 2017
   copyright            : (C) 2017 by Loïc Bartoletti
   email                : lbartoletti at tuxfamily dot org
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsmaptoolrectangle3points.h"
#include "qgsgeometryrubberband.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgsmapmouseevent.h"
#include <memory>
#include "qgssnapindicator.h"

QgsMapToolRectangle3Points::QgsMapToolRectangle3Points( QgsMapToolCapture *parentTool,
    QgsMapCanvas *canvas, CreateMode createMode, CaptureMode mode )
  : QgsMapToolAddRectangle( parentTool, canvas, mode ),
    mCreateMode( createMode )
{
}

void QgsMapToolRectangle3Points::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsPoint point = mapPoint( *e );

  if ( e->button() == Qt::LeftButton )
  {
    if ( !point.is3D() )
      point.addZValue( defaultZValue() );
    if ( mPoints.size() < 2 )
    {
      mPoints.append( point );
    }

    if ( !mPoints.isEmpty() && !mTempRubberBand )
    {
      mTempRubberBand = createGeometryRubberBand( mLayerType, true );
      mTempRubberBand->show();
    }
    if ( mPoints.size() == 3 )
    {
      delete mTempRubberBand;
      mTempRubberBand = createGeometryRubberBand( mLayerType, true ); // recreate rubberband for polygon
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    deactivate( );
    if ( mParentTool )
    {
      mParentTool->canvasReleaseEvent( e );
    }
  }
}

void QgsMapToolRectangle3Points::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsPoint point = mapPoint( *e );

  mSnapIndicator->setMatch( e->mapPointMatch() );

  if ( mTempRubberBand )
  {
    switch ( mPoints.size() )
    {
      case 1:
      {
        std::unique_ptr<QgsLineString> line( new QgsLineString() );
        line->addVertex( mPoints.at( 0 ) );
        line->addVertex( point );
        mTempRubberBand->setGeometry( line.release() );
        break;
      }
      case 2:
      {
        if ( !point.is3D() )
          point.addZValue( defaultZValue() );
        switch ( mCreateMode )
        {
          case DistanceMode:
            mRectangle = QgsQuadrilateral::rectangleFrom3Points( mPoints.at( 0 ), mPoints.at( 1 ), point, QgsQuadrilateral::Distance );
            break;
          case ProjectedMode:
            mRectangle = QgsQuadrilateral::rectangleFrom3Points( mPoints.at( 0 ), mPoints.at( 1 ), point, QgsQuadrilateral::Projected );
            break;
        }

        mTempRubberBand->setGeometry( mRectangle.toPolygon() );
        break;
      }
    }
  }
}
