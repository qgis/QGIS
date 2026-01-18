/***************************************************************************
    qgsmaptoolshapeellipsecenter3points.cpp  -  map tool for adding ellipse
    from center and 3 points
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

#include "qgsmaptoolshapeellipsecenter3points.h"

#include <memory>

#include "qgsapplication.h"
#include "qgscircle.h"
#include "qgsgeometryrubberband.h"
#include "qgslinestring.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptoolcapture.h"
#include "qgspoint.h"

#include "moc_qgsmaptoolshapeellipsecenter3points.cpp"

const QString QgsMapToolShapeEllipseCenter3PointsMetadata::TOOL_ID = u"ellipse-center-3-points"_s;

QString QgsMapToolShapeEllipseCenter3PointsMetadata::id() const
{
  return QgsMapToolShapeEllipseCenter3PointsMetadata::TOOL_ID;
}

QString QgsMapToolShapeEllipseCenter3PointsMetadata::name() const
{
  return QObject::tr( "Ellipse from Center and 3 Points" );
}

QIcon QgsMapToolShapeEllipseCenter3PointsMetadata::icon() const
{
  return QgsApplication::getThemeIcon( u"/mActionEllipseCenter3Points.svg"_s );
}

QgsMapToolShapeAbstract::ShapeCategory QgsMapToolShapeEllipseCenter3PointsMetadata::category() const
{
  return QgsMapToolShapeAbstract::ShapeCategory::Ellipse;
}

QgsMapToolShapeAbstract *QgsMapToolShapeEllipseCenter3PointsMetadata::factory( QgsMapToolCapture *parentTool ) const
{
  return new QgsMapToolShapeEllipseCenter3Points( parentTool );
}

bool QgsMapToolShapeEllipseCenter3Points::cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
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

void QgsMapToolShapeEllipseCenter3Points::cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  Q_UNUSED( mode )

  const QgsPoint point = mParentTool->mapPoint( *e );

  if ( mTempRubberBand )
  {
    switch ( mPoints.size() )
    {
      case 1:
      {
        // Show line from center to first point
        auto line = std::make_unique<QgsLineString>();
        line->addVertex( mPoints.at( 0 ) );
        line->addVertex( point );
        mTempRubberBand->setGeometry( line.release() );
      }
      break;
      case 2:
      {
        // Show ellipse preview from center + 2 points
        mEllipse = QgsEllipse::fromCenter2Points( mPoints.at( 0 ), mPoints.at( 1 ), point );
        const QgsGeometry newGeometry( mEllipse.toPolygon( segments() ) );
        if ( !newGeometry.isEmpty() )
        {
          mTempRubberBand->setGeometry( newGeometry.constGet()->clone() );
          setTransientGeometry( newGeometry );
        }
      }
      break;
      case 3:
      {
        // Show ellipse from center + 3 points
        mEllipse = QgsEllipse::fromCenter3Points( mPoints.at( 0 ), mPoints.at( 1 ), mPoints.at( 2 ), point );
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
