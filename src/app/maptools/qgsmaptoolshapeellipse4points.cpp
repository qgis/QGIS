/***************************************************************************
    qgsmaptoolshapeellipse4points.cpp  -  map tool for adding ellipse
    from 4 points
    ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Loïc Bartoletti
    email                : lituus at free dot fr
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolshapeellipse4points.h"

#include <memory>

#include "qgsapplication.h"
#include "qgscircle.h"
#include "qgsgeometryrubberband.h"
#include "qgslinestring.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptoolcapture.h"
#include "qgspoint.h"

#include "moc_qgsmaptoolshapeellipse4points.cpp"

const QString QgsMapToolShapeEllipse4PointsMetadata::TOOL_ID = u"ellipse-from-4-points"_s;

QString QgsMapToolShapeEllipse4PointsMetadata::id() const
{
  return QgsMapToolShapeEllipse4PointsMetadata::TOOL_ID;
}

QString QgsMapToolShapeEllipse4PointsMetadata::name() const
{
  return QObject::tr( "Ellipse from 4 Points" );
}

QIcon QgsMapToolShapeEllipse4PointsMetadata::icon() const
{
  return QgsApplication::getThemeIcon( u"/mActionEllipse4Points.svg"_s );
}

QgsMapToolShapeAbstract::ShapeCategory QgsMapToolShapeEllipse4PointsMetadata::category() const
{
  return QgsMapToolShapeAbstract::ShapeCategory::Ellipse;
}

QgsMapToolShapeAbstract *QgsMapToolShapeEllipse4PointsMetadata::factory( QgsMapToolCapture *parentTool ) const
{
  return new QgsMapToolShapeEllipse4Points( parentTool );
}

bool QgsMapToolShapeEllipse4Points::cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  const QgsPoint point = mParentTool->mapPoint( *e );

  if ( e->button() == Qt::LeftButton )
  {
    if ( mPoints.size() < 3 )
      mPoints.append( point );

    if ( !mTempRubberBand )
    {
      Qgis::GeometryType type = mode == QgsMapToolCapture::CapturePolygon ? Qgis::GeometryType::Polygon : Qgis::GeometryType::Line;
      mTempRubberBand = mParentTool->createGeometryRubberBand( type, true );
      mTempRubberBand->show();
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    if ( mEllipse.isEmpty() )
      return false;

    addEllipseToParentTool();
    return true;
  }

  return false;
}

void QgsMapToolShapeEllipse4Points::cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  Q_UNUSED( mode )

  const QgsPoint point = mParentTool->mapPoint( *e );

  if ( mTempRubberBand )
  {
    switch ( mPoints.size() )
    {
      case 1:
      {
        // Show line from first to current point
        auto line = std::make_unique<QgsLineString>();
        line->addVertex( mPoints.at( 0 ) );
        line->addVertex( point );
        mTempRubberBand->setGeometry( line.release() );
      }
      break;
      case 2:
      {
        // Show circle through 3 points as preview
        const QgsCircle circle = QgsCircle::from3Points( mPoints.at( 0 ), mPoints.at( 1 ), point );
        const QgsGeometry newGeometry( circle.toPolygon( segments() ) );
        if ( !newGeometry.isEmpty() )
        {
          mTempRubberBand->setGeometry( newGeometry.constGet()->clone() );
          setTransientGeometry( newGeometry );
        }
      }
      break;
      case 3:
      {
        // Show actual ellipse from 4 points
        mEllipse = QgsEllipse::from4Points( mPoints.at( 0 ), mPoints.at( 1 ), mPoints.at( 2 ), point );
        const QgsGeometry newGeometry( mEllipse.toPolygon( segments() ) );
        if ( !newGeometry.isEmpty() )
        {
          mTempRubberBand->setGeometry( newGeometry.constGet()->clone() );
          setTransientGeometry( newGeometry );
        }
      }
      break;
      default:
        break;
    }
  }
}
