/***************************************************************************
   qgsmaptoolshaperectangle3points.cpp  -  map tool for adding rectangle
   from 3 points
   ---------------------
   begin                : September 2017
   copyright            : (C) 2017 by Loïc Bartoletti
   email                : lbartoletti at tuxfamily dot org
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsmaptoolshaperectangle3points.h"
#include "qgsgeometryrubberband.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgsmapmouseevent.h"
#include <memory>
#include "qgsmaptoolcapture.h"
#include "qgsapplication.h"

const QString QgsMapToolShapeRectangle3PointsMetadata::TOOL_ID_DISTANCE = QStringLiteral( "rectangle-from-3-points-distance" );
const QString QgsMapToolShapeRectangle3PointsMetadata::TOOL_ID_PROJECTED = QStringLiteral( "rectangle-from-3-points-projected" );

QString QgsMapToolShapeRectangle3PointsMetadata::id() const
{
  switch ( mCreateMode )
  {
    case CreateMode::Distance:
      return QgsMapToolShapeRectangle3PointsMetadata::TOOL_ID_DISTANCE;
    case CreateMode::Projected:
      return QgsMapToolShapeRectangle3PointsMetadata::TOOL_ID_PROJECTED;
  }
  BUILTIN_UNREACHABLE
}

QString QgsMapToolShapeRectangle3PointsMetadata::name() const
{
  switch ( mCreateMode )
  {
    case CreateMode::Distance:
      return QObject::tr( "Rectangle from 3 points (distance)" );
    case CreateMode::Projected:
      return QObject::tr( "Rectangle from 3 points (projected)" );
  }
  BUILTIN_UNREACHABLE
}

QIcon QgsMapToolShapeRectangle3PointsMetadata::icon() const
{
  switch ( mCreateMode )
  {
    case CreateMode::Distance:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionRectangle3PointsDistance.svg" ) );
    case CreateMode::Projected:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionRectangle3PointsProjected.svg" ) );
  }

  return QIcon();
}

QgsMapToolShapeAbstract::ShapeCategory QgsMapToolShapeRectangle3PointsMetadata::category() const
{
  return QgsMapToolShapeAbstract::ShapeCategory::Rectangle;
}

QgsMapToolShapeAbstract *QgsMapToolShapeRectangle3PointsMetadata::factory( QgsMapToolCapture *parentTool ) const
{
  return new QgsMapToolShapeRectangle3Points( id(), mCreateMode, parentTool );
}

QgsMapToolShapeRectangle3Points::QgsMapToolShapeRectangle3Points( const QString &id, QgsMapToolShapeRectangle3PointsMetadata::CreateMode createMode, QgsMapToolCapture *parentTool )
  : QgsMapToolShapeRectangleAbstract( id, parentTool ),
    mCreateMode( createMode )
{
}


bool QgsMapToolShapeRectangle3Points::cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  QgsPoint point = mParentTool->mapPoint( *e );

  if ( e->button() == Qt::LeftButton )
  {
    bool is3D = false;
    QgsVectorLayer *currentLayer = qobject_cast<QgsVectorLayer *>( mParentTool->canvas()->currentLayer() );
    if ( currentLayer )
      is3D = QgsWkbTypes::hasZ( currentLayer->wkbType() );

    if ( is3D && !point.is3D() )
      point.addZValue( mParentTool->defaultZValue() );

    if ( mPoints.size() < 2 )
    {
      mPoints.append( point );
    }

    QgsWkbTypes::GeometryType type = mode == QgsMapToolCapture::CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry;

    if ( !mPoints.isEmpty() && !mTempRubberBand )
    {
      mTempRubberBand = mParentTool->createGeometryRubberBand( type, true );
      mTempRubberBand->show();
    }
    if ( mPoints.size() == 3 )
    {
      delete mTempRubberBand;
      mTempRubberBand = mParentTool->createGeometryRubberBand( type, true ); // recreate rubberband for polygon
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    addRectangleToParentTool();
    return true;
  }

  return false;
}

void QgsMapToolShapeRectangle3Points::cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode )
{
  Q_UNUSED( mode )

  QgsPoint point = mParentTool->mapPoint( *e );

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
        bool is3D = false;
        QgsVectorLayer *currentLayer = qobject_cast<QgsVectorLayer *>( mParentTool->canvas()->currentLayer() );
        if ( currentLayer )
          is3D = QgsWkbTypes::hasZ( currentLayer->wkbType() );

        if ( is3D && !point.is3D() )
          point.addZValue( mParentTool->defaultZValue() );

        switch ( mCreateMode )
        {
          case QgsMapToolShapeRectangle3PointsMetadata::CreateMode::Distance:
            mRectangle = QgsQuadrilateral::rectangleFrom3Points( mPoints.at( 0 ), mPoints.at( 1 ), point, QgsQuadrilateral::Distance );
            break;
          case QgsMapToolShapeRectangle3PointsMetadata::CreateMode::Projected:
            mRectangle = QgsQuadrilateral::rectangleFrom3Points( mPoints.at( 0 ), mPoints.at( 1 ), point, QgsQuadrilateral::Projected );
            break;
        }
        mTempRubberBand->setGeometry( mRectangle.toPolygon( ) );
      }
      break;
      default:
        break;
    }
  }
}
