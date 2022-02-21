/***************************************************************************
    qgsmaptoolshaperegularpolygoncentercorner.cpp  -  map tool for adding regular
    polygon from center and a corner
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

#include "qgsmaptoolshaperegularpolygoncentercorner.h"
#include "qgsgeometryrubberband.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptoolcapture.h"
#include "qgsapplication.h"

const QString QgsMapToolShapeRegularPolygonCenterCornerMetadata::TOOL_ID = QStringLiteral( "regular-polygon-from-center-and-a-corner" );

QString QgsMapToolShapeRegularPolygonCenterCornerMetadata::id() const
{
  return QgsMapToolShapeRegularPolygonCenterCornerMetadata::TOOL_ID;
}

QString QgsMapToolShapeRegularPolygonCenterCornerMetadata::name() const
{
  return QObject::tr( "Regular polygon from center and a corner" );
}

QIcon QgsMapToolShapeRegularPolygonCenterCornerMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mActionRegularPolygonCenterCorner.svg" ) );
}

QgsMapToolShapeAbstract::ShapeCategory QgsMapToolShapeRegularPolygonCenterCornerMetadata::category() const
{
  return QgsMapToolShapeAbstract::ShapeCategory::RegularPolygon;
}

QgsMapToolShapeAbstract *QgsMapToolShapeRegularPolygonCenterCornerMetadata::factory( QgsMapToolCapture *parentTool ) const
{
  return new QgsMapToolShapeRegularPolygonCenterCorner( parentTool );
}


QgsMapToolShapeRegularPolygonCenterCorner::~QgsMapToolShapeRegularPolygonCenterCorner()
{
  deleteNumberSidesSpinBox();
}

bool QgsMapToolShapeRegularPolygonCenterCorner::cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
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

void QgsMapToolShapeRegularPolygonCenterCorner::cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  Q_UNUSED( mode )

  const QgsPoint point = mParentTool->mapPoint( *e );

  if ( mTempRubberBand )
  {
    const QgsRegularPolygon::ConstructionOption option = QgsRegularPolygon::InscribedCircle;
    mRegularPolygon = QgsRegularPolygon( mPoints.at( 0 ), point, mNumberSidesSpinBox->value(), option );
    mTempRubberBand->setGeometry( mRegularPolygon.toPolygon() );
  }
}
