/***************************************************************************
    qgsmaptoolcircle3tangents.h  -  map tool for adding circle
    from 3 tangents
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

#include "qgsmaptoolcircle3tangents.h"
#include "qgsgeometryrubberband.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgslinestring.h"
#include "qgssnappingutils.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgisapp.h"
#include "qgsmapmouseevent.h"


QgsMapToolCircle3Tangents::QgsMapToolCircle3Tangents( QgsMapToolCapture *parentTool,
    QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolAddCircle( parentTool, canvas, mode )
{
}

void QgsMapToolCircle3Tangents::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsPoint point = mapPoint( *e );
  EdgesOnlyFilter filter;
  QgsPointLocator::Match match = mCanvas->snappingUtils()->snapToMap( point, &filter );

  QgsPointXY p1, p2;

  if ( e->button() == Qt::LeftButton )
  {
    if ( match.isValid() && ( mPoints.size() <= 2 * 2 ) )
    {
      match.edgePoints( p1, p2 );
      mPoints.append( mapPoint( p1 ) );
      mPoints.append( mapPoint( p2 ) );
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    if ( match.isValid() && ( mPoints.size() == 4 ) )
    {
      match.edgePoints( p1, p2 );
      mPoints.append( mapPoint( p1 ) );
      mPoints.append( mapPoint( p2 ) );
      mCircle = QgsCircle().from3Tangents( mPoints.at( 0 ), mPoints.at( 1 ), mPoints.at( 2 ), mPoints.at( 3 ), mPoints.at( 4 ), mPoints.at( 5 ) );
      if ( mCircle.isEmpty() )
      {
        QgisApp::instance()->messageBar()->pushMessage( tr( "Error" ), tr( "At least two segments are parallels" ), Qgis::Critical, QgisApp::instance()->messageTimeout() );
        mPoints.clear();
        delete mTempRubberBand;
        mTempRubberBand = nullptr;
      }
    }
    deactivate();
    if ( mParentTool )
    {
      mParentTool->canvasReleaseEvent( e );
    }
  }
}

void QgsMapToolCircle3Tangents::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsPoint point = mapPoint( *e );
  EdgesOnlyFilter filter;
  QgsPointLocator::Match match = mCanvas->snappingUtils()->snapToMap( point, &filter );

  if ( !mTempRubberBand )
  {
    mTempRubberBand = createGeometryRubberBand( mLayerType, true );
    mTempRubberBand->show();
  }
  else
    mTempRubberBand->hide();

  if ( match.isValid() )
  {
    QgsPointXY p1, p2;
    match.edgePoints( p1, p2 );
    std::unique_ptr<QgsLineString> line( new QgsLineString() );

    line->addVertex( mapPoint( p1 ) );
    line->addVertex( mapPoint( p2 ) );

    mTempRubberBand->setGeometry( line.release() );
    mTempRubberBand->show();
  }

}
