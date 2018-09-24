/***************************************************************************
    qgsmaptoolrectangle3points.cpp  -  map tool for adding rectangle
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
  : QgsMapToolAddRectangle( parentTool, canvas, mode )
{
}

void QgsMapToolRectangle3Points::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsPoint point = mapPoint( *e );

  if ( e->button() == Qt::LeftButton )
  {
    if ( mPoints.size() < 2 )
      mPoints.append( point );

    if ( !mPoints.isEmpty() && !mTempRubberBand )
    {
      mTempRubberBand = createGeometryRubberBand( layerType, true );
      mTempRubberBand->show();
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    deactivate( true );
    if ( mParentTool )
    {
      mParentTool->canvasReleaseEvent( e );
    }
  }
}

void QgsMapToolRectangle3Points::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsPoint point = mapPoint( *e );

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
        setAzimuth( mPoints.at( 0 ).azimuth( point ) );
        setDistance1( mPoints.at( 0 ).distance( point ) );
      }
      break;
      case 2:
      {
        setDistance2( mPoints.at( 1 ).distance( point ) );
        int side = QgsGeometryUtils::leftOfLine( point.x(), point.y(),
                   mPoints.at( 0 ).x(), mPoints.at( 0 ).y(),
                   mPoints.at( 1 ).x(), mPoints.at( 1 ).y() );

        setSide( side < 0 ? -1 : 1 );

        const double xMin = mPoints.at( 0 ).x();
        const double xMax = mPoints.at( 0 ).x() + distance2( );

        const double yMin = mPoints.at( 0 ).y();
        const double yMax = mPoints.at( 0 ).y() + distance1();

        const double z = mPoints.at( 0 ).z();

        mRectangle = QgsBox3d( xMin, yMin, z, xMax, yMax, z );


        mTempRubberBand->setGeometry( QgsMapToolAddRectangle::rectangleToPolygon( true ) );
      }
      break;
      default:
        break;
    }
  }
}
