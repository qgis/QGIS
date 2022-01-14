/***************************************************************************
    qgsmaptoolshapeellipsecenter2points.cpp  -  map tool for adding ellipse
    from center and 2 points
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

#include "qgsmaptoolshapeellipsecenter2points.h"
#include "qgsgeometryrubberband.h"
#include "qgslinestring.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptoolcapture.h"
#include <memory>
#include "qgsapplication.h"

const QString QgsMapToolShapeEllipseCenter2PointsMetadata::TOOL_ID = QStringLiteral( "ellipse-center-2-points" );

QString QgsMapToolShapeEllipseCenter2PointsMetadata::id() const
{
  return QgsMapToolShapeEllipseCenter2PointsMetadata::TOOL_ID;
}

QString QgsMapToolShapeEllipseCenter2PointsMetadata::name() const
{
  return QObject::tr( "Ellipse from center and 2 points" );
}

QIcon QgsMapToolShapeEllipseCenter2PointsMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mActionEllipseCenter2Points.svg" ) );
}

QgsMapToolShapeAbstract::ShapeCategory QgsMapToolShapeEllipseCenter2PointsMetadata::category() const
{
  return QgsMapToolShapeAbstract::ShapeCategory::Ellipse;
}

QgsMapToolShapeAbstract *QgsMapToolShapeEllipseCenter2PointsMetadata::factory( QgsMapToolCapture *parentTool ) const
{
  return new QgsMapToolShapeEllipseCenter2Points( parentTool );
}

bool QgsMapToolShapeEllipseCenter2Points::cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  const QgsPoint point = mParentTool->mapPoint( *e );
  if ( e->button() == Qt::LeftButton )
  {

    if ( mPoints.size() < 2 )
      mPoints.append( point );

    if ( !mPoints.isEmpty() && !mTempRubberBand )
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

void QgsMapToolShapeEllipseCenter2Points::cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  Q_UNUSED( mode )

  const QgsPoint point = mParentTool->mapPoint( *e );

  if ( mTempRubberBand )
  {
    switch ( mPoints.size() )
    {
      case 1:
      {
        std::unique_ptr<QgsLineString> line( new QgsLineString() );
        line->addVertex( mPoints.at( 0 ) );
        line->addVertex( point );
        mTempRubberBand->setGeometry( line.release() );
      }
      break;
      case 2:
      {
        mEllipse = QgsEllipse::fromCenter2Points( mPoints.at( 0 ), mPoints.at( 1 ), point );
        mTempRubberBand->setGeometry( mEllipse.toPolygon( segments() ) );
      }
      break;
      default:
        break;
    }
  }
}
