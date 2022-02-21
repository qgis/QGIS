/***************************************************************************
    qgsmaptoolshaperegularpolygoncenterpoint.cpp  -  map tool for adding regular
    polygon from center and a point
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

#include "qgsmaptoolshaperegularpolygoncenterpoint.h"
#include "qgsgeometryrubberband.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptoolcapture.h"
#include "qgsapplication.h"

const QString QgsMapToolShapeRegularPolygonCenterPointMetadata::TOOL_ID = QStringLiteral( "regular-polygon-from-center-point" );

QString QgsMapToolShapeRegularPolygonCenterPointMetadata::id() const
{
  return QgsMapToolShapeRegularPolygonCenterPointMetadata::TOOL_ID;
}

QString QgsMapToolShapeRegularPolygonCenterPointMetadata::name() const
{
  return QObject::tr( "Regular polygon from center and a point" );
}

QIcon QgsMapToolShapeRegularPolygonCenterPointMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mActionRegularPolygonCenterPoint.svg" ) );
}

QgsMapToolShapeAbstract::ShapeCategory QgsMapToolShapeRegularPolygonCenterPointMetadata::category() const
{
  return QgsMapToolShapeAbstract::ShapeCategory::RegularPolygon;
}

QgsMapToolShapeAbstract *QgsMapToolShapeRegularPolygonCenterPointMetadata::factory( QgsMapToolCapture *parentTool ) const
{
  return new QgsMapToolShapeRegularPolygonCenterPoint( parentTool );
}

QgsMapToolShapeRegularPolygonCenterPoint::~QgsMapToolShapeRegularPolygonCenterPoint()
{
  deleteNumberSidesSpinBox();
}

bool QgsMapToolShapeRegularPolygonCenterPoint::cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{

  const QgsPoint point = mParentTool->mapPoint( *e );

  if ( e->button() == Qt::LeftButton )
  {
    if ( mPoints.size() < 1 )
      mPoints.append( point );

    if ( !mPoints.isEmpty() )
    {
      if ( !mTempRubberBand )
      {
        QgsWkbTypes::GeometryType type = mode == QgsMapToolCapture::CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry;
        mTempRubberBand = mParentTool->createGeometryRubberBand( type, true );
        mTempRubberBand->show();

        createNumberSidesSpinBox();
      }
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

void QgsMapToolShapeRegularPolygonCenterPoint::cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  Q_UNUSED( mode )

  const QgsPoint point = mParentTool->mapPoint( *e );

  if ( mTempRubberBand )
  {
    const QgsRegularPolygon::ConstructionOption option = QgsRegularPolygon::CircumscribedCircle;
    mRegularPolygon = QgsRegularPolygon( mPoints.at( 0 ), point, mNumberSidesSpinBox->value(), option );
    mTempRubberBand->setGeometry( mRegularPolygon.toPolygon() );
  }
}
