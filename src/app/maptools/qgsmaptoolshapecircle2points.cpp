/***************************************************************************
    qgsmaptoolshapecircle2points.cpp  -  map tool for adding circle
    from 2 points
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

#include "qgsmaptoolshapecircle2points.h"
#include "moc_qgsmaptoolshapecircle2points.cpp"
#include "qgsgeometryrubberband.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptoolcapture.h"
#include "qgsapplication.h"

QString QgsMapToolShapeCircle2PointsMetadata::id() const
{
  return QgsMapToolShapeCircle2PointsMetadata::TOOL_ID;
}

QString QgsMapToolShapeCircle2PointsMetadata::name() const
{
  return QObject::tr( "Circle from 2 points" );
}

QIcon QgsMapToolShapeCircle2PointsMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mActionCircle2Points.svg" ) );
}

QgsMapToolShapeAbstract::ShapeCategory QgsMapToolShapeCircle2PointsMetadata::category() const
{
  return QgsMapToolShapeAbstract::ShapeCategory::Circle;
}

QgsMapToolShapeAbstract *QgsMapToolShapeCircle2PointsMetadata::factory( QgsMapToolCapture *parentTool ) const
{
  return new QgsMapToolShapeCircle2Points( parentTool );
}


bool QgsMapToolShapeCircle2Points::cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  if ( e->button() == Qt::LeftButton )
  {
    if ( mPoints.isEmpty() )
      mPoints.append( mParentTool->mapPoint( *e ) );

    if ( !mTempRubberBand )
    {
      Qgis::GeometryType type = mode == QgsMapToolCapture::CapturePolygon ? Qgis::GeometryType::Polygon : Qgis::GeometryType::Line;
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

void QgsMapToolShapeCircle2Points::cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  Q_UNUSED( mode )
  if ( !mTempRubberBand )
    return;

  mCircle = QgsCircle::from2Points( mPoints.at( 0 ), mParentTool->mapPoint( *e ) );
  mTempRubberBand->setGeometry( mCircle.toCircularString( true ) );
}
