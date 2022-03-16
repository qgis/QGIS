/***************************************************************************
    qgsmaptoolshaperegularpolygon2points.cpp  -  map tool for adding regular
    polygon from 2 points
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

#include "qgsmaptoolshaperegularpolygon2points.h"
#include "qgsgeometryrubberband.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptoolcapture.h"
#include "qgsapplication.h"

const QString QgsMapToolShapeRegularPolygon2PointsMetadata::TOOL_ID = QStringLiteral( "regular-polygon-from-2-points" );

QString QgsMapToolShapeRegularPolygon2PointsMetadata::id() const
{
  return QgsMapToolShapeRegularPolygon2PointsMetadata::TOOL_ID;
}

QString QgsMapToolShapeRegularPolygon2PointsMetadata::name() const
{
  return QObject::tr( "Regular polygon from 2 points" );
}

QIcon QgsMapToolShapeRegularPolygon2PointsMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mActionRegularPolygon2Points.svg" ) );
}

QgsMapToolShapeAbstract::ShapeCategory QgsMapToolShapeRegularPolygon2PointsMetadata::category() const
{
  return QgsMapToolShapeAbstract::ShapeCategory::Circle;
}

QgsMapToolShapeAbstract *QgsMapToolShapeRegularPolygon2PointsMetadata::factory( QgsMapToolCapture *parentTool ) const
{
  return new QgsMapToolShapeRegularPolygon2Points( parentTool );
}


QgsMapToolShapeRegularPolygon2Points::~QgsMapToolShapeRegularPolygon2Points()
{
  if ( mNumberSidesSpinBox )
  {
    deleteNumberSidesSpinBox();
  }
}

bool QgsMapToolShapeRegularPolygon2Points::cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
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

      createNumberSidesSpinBox();
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    mPoints.append( point );
    addRegularPolygonToParentTool();
    return true;
  }

  return false;
}

void QgsMapToolShapeRegularPolygon2Points::cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  Q_UNUSED( mode )

  const QgsPoint point =  mParentTool->mapPoint( *e );

  if ( mTempRubberBand )
  {
    mRegularPolygon = QgsRegularPolygon( mPoints.at( 0 ), point, mNumberSidesSpinBox->value() );
    mTempRubberBand->setGeometry( mRegularPolygon.toPolygon() );
  }
}
