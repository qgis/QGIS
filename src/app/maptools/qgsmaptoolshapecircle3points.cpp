/***************************************************************************
    qgsmaptoolshapecircle3points.h  -  map tool for adding circle
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

#include "qgsmaptoolshapecircle3points.h"
#include "qgsgeometryrubberband.h"
#include "qgslinestring.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptoolcapture.h"
#include "qgsapplication.h"

const QString QgsMapToolShapeCircle3PointsMetadata::TOOL_ID = QStringLiteral( "circle-from-3-points" );

QString QgsMapToolShapeCircle3PointsMetadata::id() const
{
  return QgsMapToolShapeCircle3PointsMetadata::TOOL_ID;
}

QString QgsMapToolShapeCircle3PointsMetadata::name() const
{
  return QObject::tr( "Circle from 3 points" );
}

QIcon QgsMapToolShapeCircle3PointsMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mActionCircle3Points.svg" ) );
}

QgsMapToolShapeAbstract::ShapeCategory QgsMapToolShapeCircle3PointsMetadata::category() const
{
  return QgsMapToolShapeAbstract::ShapeCategory::Circle;
}

QgsMapToolShapeAbstract *QgsMapToolShapeCircle3PointsMetadata::factory( QgsMapToolCapture *parentTool ) const
{
  return new QgsMapToolShapeCircle3Points( parentTool );
}

bool QgsMapToolShapeCircle3Points::cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  if ( e->button() == Qt::LeftButton )
  {
    if ( mPoints.size() < 2 )
      mPoints.append( mParentTool->mapPoint( *e ) );
    if ( !mPoints.isEmpty() && !mTempRubberBand )
    {
      QgsWkbTypes::GeometryType type = mode == QgsMapToolCapture::CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry;
      mTempRubberBand = mParentTool->createGeometryRubberBand( type, true );
      mTempRubberBand->show();
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    mPoints.append( mParentTool->mapPoint( *e ) );
    addCircleToParentTool();
    return true;
  }

  return false;
}

void QgsMapToolShapeCircle3Points::cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  Q_UNUSED( mode )

  if ( !mTempRubberBand )
    return;

  switch ( mPoints.size() )
  {
    case 1:
    {
      std::unique_ptr<QgsLineString> line( new QgsLineString() );
      line->addVertex( mPoints.at( 0 ) );
      line->addVertex( mParentTool->mapPoint( *e ) );
      mTempRubberBand->setGeometry( line.release() );
    }
    break;
    case 2:
    {
      mCircle = QgsCircle::from3Points( mPoints.at( 0 ), mPoints.at( 1 ), mParentTool->mapPoint( *e ) );
      mTempRubberBand->setGeometry( mCircle.toCircularString( true ) );
    }
    break;
    default:
      break;
  }
}
