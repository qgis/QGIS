/***************************************************************************
    qgmaptoolellipsecenterpoint.cpp  -  map tool for adding ellipse
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

#include "qgsmaptoolshapeellipsecenterpoint.h"
#include "qgsgeometryrubberband.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptoolcapture.h"
#include "qgsapplication.h"

const QString QgsMapToolShapeEllipseCenterPointMetadata::TOOL_ID = QStringLiteral( "ellipse-center-point" );

QString QgsMapToolShapeEllipseCenterPointMetadata::id() const
{
  return QgsMapToolShapeEllipseCenterPointMetadata::TOOL_ID;
}

QString QgsMapToolShapeEllipseCenterPointMetadata::name() const
{
  return QObject::tr( "Ellipse from center and a point" );
}

QIcon QgsMapToolShapeEllipseCenterPointMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mActionEllipseCenterPoint.svg" ) );
}

QgsMapToolShapeAbstract::ShapeCategory QgsMapToolShapeEllipseCenterPointMetadata::category() const
{
  return QgsMapToolShapeAbstract::ShapeCategory::Ellipse;
}

QgsMapToolShapeAbstract *QgsMapToolShapeEllipseCenterPointMetadata::factory( QgsMapToolCapture *parentTool ) const
{
  return new QgsMapToolShapeEllipseCenterPoint( parentTool );
}


bool QgsMapToolShapeEllipseCenterPoint::cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
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
    addEllipseToParentTool();
    return true;
  }

  return false;
}

void QgsMapToolShapeEllipseCenterPoint::cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  Q_UNUSED( mode )

  const QgsPoint point = mParentTool->mapPoint( *e );

  if ( mTempRubberBand )
  {
    mEllipse = QgsEllipse::fromCenterPoint( mPoints.at( 0 ), point );
    mTempRubberBand->setGeometry( mEllipse.toPolygon( segments() ) );
  }
}
