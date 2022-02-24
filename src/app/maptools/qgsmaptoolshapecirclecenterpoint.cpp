/***************************************************************************
    qgmaptoolshapecirclecenterpoint.cpp  -  map tool for adding circle
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

#include "qgsmaptoolshapecirclecenterpoint.h"
#include "qgsgeometryrubberband.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptoolcapture.h"
#include "qgsapplication.h"

const QString QgsMapToolShapeCircleCenterPointMetadata::TOOL_ID = QStringLiteral( "circle-by-a-center-point-and-another-point" );

QString QgsMapToolShapeCircleCenterPointMetadata::id() const
{
  return QgsMapToolShapeCircleCenterPointMetadata::TOOL_ID;
}

QString QgsMapToolShapeCircleCenterPointMetadata::name() const
{
  return QObject::tr( "Circle by a center point and another point" );
}

QIcon QgsMapToolShapeCircleCenterPointMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mActionCircleCenterPoint.svg" ) );
}

QgsMapToolShapeAbstract::ShapeCategory QgsMapToolShapeCircleCenterPointMetadata::category() const
{
  return QgsMapToolShapeAbstract::ShapeCategory::Circle;
}

QgsMapToolShapeAbstract *QgsMapToolShapeCircleCenterPointMetadata::factory( QgsMapToolCapture *parentTool ) const
{
  return new QgsMapToolShapeCircleCenterPoint( parentTool );
}


bool QgsMapToolShapeCircleCenterPoint::cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  const QgsPoint point = mParentTool->mapPoint( *e );

  if ( e->button() == Qt::LeftButton )
  {
    if ( mPoints.empty() )
      mPoints.append( point );

    if ( !mTempRubberBand )
    {
      QgsWkbTypes::GeometryType type = mode == QgsMapToolCapture::CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry;
      mTempRubberBand = mParentTool->createGeometryRubberBand( type, true );
      mTempRubberBand->show();
    }

  }
  else if ( e->button() == Qt::RightButton )
  {
    mPoints.append( point );
    addCircleToParentTool();
    return true;
  }
  return false;
}

void QgsMapToolShapeCircleCenterPoint::cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  Q_UNUSED( mode )

  const QgsPoint point = mParentTool->mapPoint( *e );

  if ( mTempRubberBand )
  {
    mCircle = QgsCircle::fromCenterPoint( mPoints.at( 0 ), point );
    mTempRubberBand->setGeometry( mCircle.toCircularString( true ) );
  }
}
