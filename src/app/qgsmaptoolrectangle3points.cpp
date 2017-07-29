/***************************************************************************
    qgmaptoolrectangle3points.cpp  -  map tool for adding rectangle
    from 3 points
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
#include "qgsmaptoolrectangle3points.h"
#include "qgsgeometryrubberband.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include <QMouseEvent>
#include <memory>

QgsMapToolRectangle3Points::QgsMapToolRectangle3Points( QgsMapToolCapture *parentTool,
    QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolAddRegularShape( parentTool, canvas, mode )
{
}

QgsMapToolRectangle3Points::~QgsMapToolRectangle3Points()
{
}

void QgsMapToolRectangle3Points::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsPoint mapPoint( e->mapPoint() );

  if ( e->button() == Qt::LeftButton )
  {
    mPoints.append( mapPoint );

    if ( !mPoints.isEmpty() )
    {
      if ( !mTempRubberBand )
      {
        mTempRubberBand = createGeometryRubberBand( ( mode() == CapturePolygon ) ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, true );
        mTempRubberBand->show();
      }
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    deactivate();
    if ( mParentTool )
    {
      mParentTool->canvasReleaseEvent( e );
    }
  }
}

void QgsMapToolRectangle3Points::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsPoint mapPoint( e->mapPoint() );

  if ( mTempRubberBand )
  {
    switch ( mPoints.size() )
    {
      case 1:
      {
        std::unique_ptr<QgsLineString> line( new QgsLineString() );
        line->addVertex( mPoints.at( 0 ) );
        line->addVertex( mapPoint );
        mTempRubberBand->setGeometry( line.release() );
      }
      break;
      case 2:
      {
        double distance = mPoints.at( 1 ).distance( mapPoint );
        double azimuth = mPoints.at( 0 ).azimuth( mPoints.at( 1 ) );
        int left_right = QgsGeometryUtils::leftOfLine( mapPoint.x(), mapPoint.y(),
                         mPoints.at( 0 ).x(), mPoints.at( 0 ).y(),
                         mPoints.at( 1 ).x(), mPoints.at( 1 ).y() ) < 0 ? -1 : 1;

        QgsPoint pt_1, pt_2, pt_3, pt_4;

        std::unique_ptr<QgsLineString> line( new QgsLineString() );
        pt_1 = mPoints.at( 0 );
        pt_2 = mPoints.at( 1 );

        pt_3 = mPoints.at( 1 ).project( distance * left_right, azimuth + 90.0 );
        pt_4 = mPoints.at( 0 ).project( distance * left_right, azimuth + 90.0 );

        line->addVertex( pt_1 );
        line->addVertex( pt_2 );
        line->addVertex( pt_3 );
        line->addVertex( pt_4 );
        line->addVertex( pt_1 );

        mRegularShape = line.release();
        mTempRubberBand->setGeometry( mRegularShape );
      }
      break;
      default:
        break;
    }
  }
}
