/***************************************************************************
    qgsmaptoolshapecircle3tangents.h  -  map tool for adding circle
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

#include "qgsmaptoolshapecircle3tangents.h"
#include "qgsgeometryrubberband.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgslinestring.h"
#include "qgssnappingutils.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgisapp.h"
#include "qgsmapmouseevent.h"
#include "qgsmessagebar.h"
#include "qgsmaptoolcapture.h"
#include "qgsapplication.h"

const QString QgsMapToolShapeCircle3TangentsMetadata::TOOL_ID = QStringLiteral( "circle-from-3-tangents" );

QString QgsMapToolShapeCircle3TangentsMetadata::id() const
{
  return QgsMapToolShapeCircle3TangentsMetadata::TOOL_ID;
}

QString QgsMapToolShapeCircle3TangentsMetadata::name() const
{
  return QObject::tr( "Circle from 3 tangents" );
}

QIcon QgsMapToolShapeCircle3TangentsMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mActionCircle3Tangents.svg" ) );
}

QgsMapToolShapeAbstract::ShapeCategory QgsMapToolShapeCircle3TangentsMetadata::category() const
{
  return QgsMapToolShapeAbstract::ShapeCategory::Circle;
}

QgsMapToolShapeAbstract *QgsMapToolShapeCircle3TangentsMetadata::factory( QgsMapToolCapture *parentTool ) const
{
  return new QgsMapToolShapeCircle3Tangents( parentTool );
}


static QgsPoint getFirstPointOnParallels( const QgsPoint p1_line1, const QgsPoint p2_line1, const QgsPoint pos_line1, const QgsPoint p1_line2, const QgsPoint p2_line2, const QgsPoint pos_line2, const QgsPoint p1_line3, const QgsPoint p2_line3 )
{
  QgsPoint intersection;
  bool isInter;

  if ( ( !QgsGeometryUtils::segmentIntersection( p1_line1, p2_line1, p1_line2, p2_line2, intersection, isInter, true ) ) || ( !QgsGeometryUtils::segmentIntersection( p1_line1, p2_line1, p1_line3, p2_line3, intersection, isInter, true ) ) )
    return pos_line1;
  if ( !QgsGeometryUtils::segmentIntersection( p1_line2, p2_line2, p1_line3, p2_line3, intersection, isInter, true ) )
    return pos_line2;

  return QgsPoint();
}

bool QgsMapToolShapeCircle3Tangents::cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  Q_UNUSED( mode )

  const QgsPoint point = mParentTool->mapPoint( *e );

  EdgesOnlyFilter filter;
  const QgsPointLocator::Match match = mParentTool->canvas()->snappingUtils()->snapToMap( point, &filter );

  QgsPointXY p1, p2;

  if ( e->button() == Qt::LeftButton )
  {
    if ( match.isValid() && ( mPoints.size() <= 2 * 2 ) )
    {
      mPosPoints.append( mParentTool->mapPoint( match.point() ) );
      match.edgePoints( p1, p2 );
      mPoints.append( mParentTool->mapPoint( p1 ) );
      mPoints.append( mParentTool->mapPoint( p2 ) );
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    if ( match.isValid() && ( mPoints.size() == 4 ) )
    {
      match.edgePoints( p1, p2 );
      mPoints.append( mParentTool->mapPoint( p1 ) );
      mPoints.append( mParentTool->mapPoint( p2 ) );
      const QgsPoint pos = getFirstPointOnParallels( mPoints.at( 0 ), mPoints.at( 1 ), mPosPoints.at( 0 ), mPoints.at( 2 ), mPoints.at( 3 ), mPosPoints.at( 1 ), mPoints.at( 4 ), mPoints.at( 5 ) );
      mCircle = QgsCircle::from3Tangents( mPoints.at( 0 ), mPoints.at( 1 ), mPoints.at( 2 ), mPoints.at( 3 ), mPoints.at( 4 ), mPoints.at( 5 ), 1E-8, pos );
      if ( mCircle.isEmpty() )
      {
        QgisApp::instance()->messageBar()->pushMessage( tr( "Error" ), tr( "The three segments are parallel" ), Qgis::MessageLevel::Critical );
        mPoints.clear();
        mPosPoints.clear();
        delete mTempRubberBand;
        mTempRubberBand = nullptr;
      }
    }

    addCircleToParentTool();
    return true;
  }

  return false;
}

void QgsMapToolShapeCircle3Tangents::cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  const QgsPoint point = mParentTool->mapPoint( *e );

  EdgesOnlyFilter filter;
  const QgsPointLocator::Match match = mParentTool->canvas()->snappingUtils()->snapToMap( point, &filter );

  if ( !mTempRubberBand )
  {
    QgsWkbTypes::GeometryType type = mode == QgsMapToolCapture::CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry;
    mTempRubberBand = mParentTool->createGeometryRubberBand( type, true );
    mTempRubberBand->show();
  }
  else
    mTempRubberBand->hide();

  if ( match.isValid() )
  {
    QgsPointXY p1, p2;
    match.edgePoints( p1, p2 );
    if ( mPoints.size() == 4 )
    {
      const QgsPoint pos = getFirstPointOnParallels( mPoints.at( 0 ), mPoints.at( 1 ), mPosPoints.at( 0 ), mPoints.at( 2 ), mPoints.at( 3 ), mPosPoints.at( 1 ), QgsPoint( p1 ), QgsPoint( p2 ) );
      mCircle = QgsCircle::from3Tangents( mPoints.at( 0 ), mPoints.at( 1 ), mPoints.at( 2 ), mPoints.at( 3 ), QgsPoint( p1 ), QgsPoint( p2 ), 1E-8, pos );
      mTempRubberBand->setGeometry( mCircle.toLineString() );
      mTempRubberBand->show();
    }
    else
    {
      std::unique_ptr<QgsLineString> line( new QgsLineString() );

      line->addVertex( mParentTool->mapPoint( p1 ) );
      line->addVertex( mParentTool->mapPoint( p2 ) );

      mTempRubberBand->setGeometry( line.release() );
      mTempRubberBand->show();
    }
  }

}

void QgsMapToolShapeCircle3Tangents::clean( )
{
  mPosPoints.clear();
  QgsMapToolShapeCircleAbstract::clean();
}
