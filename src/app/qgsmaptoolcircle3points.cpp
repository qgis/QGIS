/***************************************************************************
    qgmaptoolcircle3points.h  -  map tool for adding circle
    from 3 points
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

#include "qgsmaptoolcircle3points.h"
#include "qgsgeometryrubberband.h"
#include "qgslinestring.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgsmapmouseevent.h"
#include "qgssnapindicator.h"

QgsMapToolCircle3Points::QgsMapToolCircle3Points( QgsMapToolCapture *parentTool,
    QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolAddCircle( parentTool, canvas, mode )
{
  mToolName = tr( "Add circle from 3 points" );
}

void QgsMapToolCircle3Points::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsPoint point = mapPoint( *e );

  if ( !currentVectorLayer() )
  {
    notifyNotVectorLayer();
    clean();
    stopCapturing();
    e->ignore();
    return;
  }

  if ( e->button() == Qt::LeftButton )
  {
    if ( mPoints.size() < 2 )
      mPoints.append( point );
    if ( !mPoints.isEmpty() && !mTempRubberBand )
    {
      mTempRubberBand = createGeometryRubberBand( mLayerType, true );
      mTempRubberBand->show();
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    release( e );
  }
}

void QgsMapToolCircle3Points::cadCanvasMoveEvent( QgsMapMouseEvent *e )
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
      }
      break;
      case 2:
      {
        mCircle = QgsCircle().from3Points( mPoints.at( 0 ), mPoints.at( 1 ), point );
        mTempRubberBand->setGeometry( mCircle.toCircularString( true ) );
      }
      break;
      default:
        break;
    }
  }
}
