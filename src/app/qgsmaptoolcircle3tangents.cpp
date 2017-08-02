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
#include "qgslinestring.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgisapp.h"
#include <QMouseEvent>

QgsMapToolCircle3Tangents::QgsMapToolCircle3Tangents( QgsMapToolCapture *parentTool,
    QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolAddCircle( parentTool, canvas, mode )
{
}

QgsMapToolCircle3Tangents::~QgsMapToolCircle3Tangents()
{
}

void QgsMapToolCircle3Tangents::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{

  QList<QgsPointXY> segment = e->snapSegment( QgsMapMouseEvent::SnapProjectConfig );

  if ( e->button() == Qt::LeftButton )
  {
    if ( !segment.empty() && ( mPoints.size() <= 2 * 2 ) )
    {
      mPoints.append( QgsPoint( segment.at( 0 ) ) );
      mPoints.append( QgsPoint( segment.at( 1 ) ) );
    }
    if ( !mPoints.isEmpty() )
    {
      if ( !mTempRubberBand )
      {
        mTempRubberBand = createGeometryRubberBand( ( mode() == CapturePolygon ) ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, true );
        mTempRubberBand->show();
      }
      std::unique_ptr<QgsLineString> line( new QgsLineString() );

      line->addVertex( QgsPoint( segment.at( 0 ) ) );
      line->addVertex( QgsPoint( segment.at( 1 ) ) );

      mTempRubberBand->setGeometry( line.release() );
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    if ( !segment.empty() && ( mPoints.size() == 4 ) )
    {
      mPoints.append( QgsPoint( segment.at( 0 ) ) );
      mPoints.append( QgsPoint( segment.at( 1 ) ) );
      mCircle = QgsCircle().from3Tangents( mPoints.at( 0 ), mPoints.at( 1 ), mPoints.at( 2 ), mPoints.at( 3 ), mPoints.at( 4 ), mPoints.at( 5 ) );
      if ( mCircle.isEmpty() )
      {
        QgisApp::instance()->messageBar()->pushMessage( tr( "Error" ), tr( "At least two segments are parallels" ), QgsMessageBar::WARNING, QgisApp::instance()->messageTimeout() );
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
