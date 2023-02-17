/***************************************************************************
    qgsmaptoolshaperectanglecenter.cpp  -  map tool for adding rectangle
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

#include "qgsmaptoolshaperectanglecenter.h"
#include "qgsgeometryrubberband.h"
#include "qgsgeometryutils.h"
#include "qgsmapcanvas.h"
#include "qgslinestring.h"
#include "qgspoint.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptoolcapture.h"
#include "qgsquadrilateral.h"
#include "qgsapplication.h"

#include <memory>

const QString QgsMapToolShapeRectangleCenterMetadata::TOOL_ID = QStringLiteral( "rectangle-from-center-and-a-point" );

QString QgsMapToolShapeRectangleCenterMetadata::id() const
{
  return QgsMapToolShapeRectangleCenterMetadata::TOOL_ID;
}

QString QgsMapToolShapeRectangleCenterMetadata::name() const
{
  return QObject::tr( "Rectangle from center and a point" );
}

QIcon QgsMapToolShapeRectangleCenterMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mActionRectangleCenter.svg" ) );
}

QgsMapToolShapeAbstract::ShapeCategory QgsMapToolShapeRectangleCenterMetadata::category() const
{
  return QgsMapToolShapeAbstract::ShapeCategory::Rectangle;
}

QgsMapToolShapeAbstract *QgsMapToolShapeRectangleCenterMetadata::factory( QgsMapToolCapture *parentTool ) const
{
  return new QgsMapToolShapeRectangleCenter( parentTool );
}

bool QgsMapToolShapeRectangleCenter::cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
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
    addRectangleToParentTool();
    return true;
  }

  return false;
}

void QgsMapToolShapeRectangleCenter::cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  Q_UNUSED( mode )

  const QgsPoint point = mParentTool->mapPoint( *e );

  if ( mTempRubberBand )
  {
    switch ( mPoints.size() )
    {
      case 1:
      {

        const double dist = mPoints.at( 0 ).distance( point );
        const double angle = mPoints.at( 0 ).azimuth( point );

        mRectangle = QgsQuadrilateral::rectangleFromExtent( mPoints.at( 0 ).project( -dist, angle ), mPoints.at( 0 ).project( dist, angle ) );
        mTempRubberBand->setGeometry( mRectangle.toPolygon() );

      }
      break;
      default:
        break;
    }
  }
}
