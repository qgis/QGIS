/***************************************************************************
    qgmaptoolshapeellipsefoci.cpp  -  map tool for adding ellipse
    from foci and a point
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

#include "qgsmaptoolshapeellipsefoci.h"

#include <memory>

#include "qgsapplication.h"
#include "qgsgeometryrubberband.h"
#include "qgslinestring.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptoolcapture.h"
#include "qgspoint.h"

#include "moc_qgsmaptoolshapeellipsefoci.cpp"

const QString QgsMapToolShapeEllipseFociMetadata::TOOL_ID = u"ellipse-from-foci"_s;

QString QgsMapToolShapeEllipseFociMetadata::id() const
{
  return QgsMapToolShapeEllipseFociMetadata::TOOL_ID;
}

QString QgsMapToolShapeEllipseFociMetadata::name() const
{
  return QObject::tr( "Ellipse from Foci" );
}

QIcon QgsMapToolShapeEllipseFociMetadata::icon() const
{
  return QgsApplication::getThemeIcon( u"/mActionEllipseFoci.svg"_s );
}

QgsMapToolShapeAbstract::ShapeCategory QgsMapToolShapeEllipseFociMetadata::category() const
{
  return QgsMapToolShapeAbstract::ShapeCategory::Ellipse;
}

QgsMapToolShapeAbstract *QgsMapToolShapeEllipseFociMetadata::factory( QgsMapToolCapture *parentTool ) const
{
  return new QgsMapToolShapeEllipseFoci( parentTool );
}

QgsMapToolShapeEllipseFoci::QgsMapToolShapeEllipseFoci( QgsMapToolCapture *parentTool )
  : QgsMapToolShapeEllipseAbstract( QgsMapToolShapeEllipseFociMetadata::TOOL_ID, parentTool )
{
}

bool QgsMapToolShapeEllipseFoci::cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  const QgsPoint point = mParentTool->mapPoint( *e );

  if ( e->button() == Qt::LeftButton )
  {
    if ( mPoints.size() < 2 )
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

void QgsMapToolShapeEllipseFoci::cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  Q_UNUSED( mode )

  const QgsPoint point = mParentTool->mapPoint( *e );

  if ( mTempRubberBand )
  {
    switch ( mPoints.size() )
    {
      case 1:
      {
        auto line = std::make_unique<QgsLineString>();
        line->addVertex( mPoints.at( 0 ) );
        line->addVertex( point );
        mTempRubberBand->setGeometry( line.release() );
      }
      break;
      case 2:
      {
        mEllipse = QgsEllipse::fromFoci( mPoints.at( 0 ), mPoints.at( 1 ), point );
        mTempRubberBand->setGeometry( mEllipse.toPolygon() );
      }
      break;
      default:
        break;
    }
  }
}
