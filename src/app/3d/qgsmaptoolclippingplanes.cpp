/***************************************************************************
    qgsmaptoolclippingplanes.cpp
    ---------------------
    begin                : March 2025
    copyright            : (C) 2025 by Matej Bagar
    email                : matej dot bagar at lutraconsulting dot co dot uk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsmaptoolclippingplanes.h"

#include "qgisapp.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dmapcanvaswidget.h"
#include "qgs3dmapscene.h"
#include "qgs3dutils.h"
#include "qgscoordinatetransform.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgspolygon.h"
#include "qgsrubberband.h"

#include <QVector4D>

#include "moc_qgsmaptoolclippingplanes.cpp"

QgsMapToolClippingPlanes::QgsMapToolClippingPlanes( QgsMapCanvas *canvas, Qgs3DMapCanvasWidget *mapCanvas )
  : QgsMapTool( canvas ), m3DCanvasWidget( mapCanvas )
{
  mRubberBandPolygon.reset( new QgsRubberBand( canvas, Qgis::GeometryType::Polygon ) );
  mRubberBandLines.reset( new QgsRubberBand( canvas, Qgis::GeometryType::Line ) );
  mRubberBandPoints.reset( new QgsRubberBand( canvas, Qgis::GeometryType::Point ) );
  mRubberBandPoints->setColor( QColorConstants::Red );
  mRubberBandPoints->setIconSize( 10 );
  mRubberBandLines->setColor( QColorConstants::Red );
  mRubberBandLines->setWidth( 3 );
  QColor polygonColor = QColorConstants::Red.lighter();
  polygonColor.setAlphaF( 0.5 );
  mRubberBandPolygon->setColor( polygonColor );
}

void QgsMapToolClippingPlanes::activate()
{
  QgsMapTool::activate();
  mRubberBandPoints->show();
  mRubberBandLines->show();
  mRubberBandPolygon->show();
  mCt = std::make_unique<QgsCoordinateTransform>(
    mCanvas->mapSettings().destinationCrs(),
    m3DCanvasWidget->mapCanvas3D()->mapSettings()->crs(),
    mCanvas->mapSettings().transformContext()
  );
}

void QgsMapToolClippingPlanes::keyReleaseEvent( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Escape )
  {
    clear();
  }
}

void QgsMapToolClippingPlanes::deactivate()
{
  clearRubberBand();
  QgsMapTool::deactivate();
}

void QgsMapToolClippingPlanes::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( mRubberBandPoints->numberOfVertices() == 0 )
    return;

  const QgsPointXY point = toMapCoordinates( e->pos() );
  if ( mRubberBandPoints->numberOfVertices() == 2 )
  {
    // Let's do the Cartesian math in 3d map coordinates which are guaranteed to be projected
    try
    {
      const QgsPointXY startPointMap3d = mCt->transform( *mRubberBandPoints->getPoint( 0, 0 ) );
      const QgsPointXY endPointMap3d = mCt->transform( *mRubberBandPoints->getPoint( 0, 1 ) );
      const QgsPointXY widthPointMap3d = mCt->transform( point );
      mRectangleWidth = endPointMap3d.distance( widthPointMap3d );
      QgsVector vec( endPointMap3d - startPointMap3d );
      vec = vec.normalized().perpVector();

      const QVector<QgsPointXY> points3DMap( {
        startPointMap3d + vec * mRectangleWidth, //! top left corner
        endPointMap3d + vec * mRectangleWidth,   //! top right corner
        endPointMap3d - vec * mRectangleWidth,   //! bottom right corner
        startPointMap3d - vec * mRectangleWidth  //! bottom left corner
      } );

      // build a polygon and transform it back to 2d map canvas coordinates
      QgsGeometry geom( new QgsPolygon( new QgsLineString( points3DMap ) ) );
      geom.transform( *mCt, Qgis::TransformDirection::Reverse );
      mRubberBandPolygon->setToGeometry( geom );
    }
    catch ( const QgsCsException & )
    {
      QgsDebugError( u"Could not reproject cross section coordinates to 3d map crs."_s );
    }
  }
  else
  {
    mRubberBandLines->movePoint( point );
  }
}

void QgsMapToolClippingPlanes::canvasPressEvent( QgsMapMouseEvent * )
{
  if ( mRubberBandPoints->numberOfVertices() == 0 )
  {
    clearHighLightedArea();
  }
}

void QgsMapToolClippingPlanes::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( e->button() == Qt::LeftButton )
  {
    const QgsPointXY point = toMapCoordinates( e->pos() );
    if ( mRubberBandPoints->numberOfVertices() == 2 )
    {
      //check if cross-section is in canvas extents
      QgsGeometry crossSectionPolygon = mRubberBandPolygon->asGeometry();

      try
      {
        crossSectionPolygon.transform( *mCt );
      }
      catch ( const QgsCsException & )
      {
        crossSectionPolygon.set( nullptr );
        QgsDebugError( u"Could not reproject cross-section extent to 3d map canvas crs."_s );
      }

      if ( !crossSectionPolygon.intersects( m3DCanvasWidget->mapCanvas3D()->scene()->sceneExtent() ) )
      {
        clear();
        if ( crossSectionPolygon.isNull() )
          emit messageEmitted( tr( "Could not reproject the cross-section extent to 3D map coordinates." ), Qgis::MessageLevel::Warning );
        else
          emit messageEmitted( tr( "The cross section is outside of the scene extent, please select a new one!" ), Qgis::MessageLevel::Info );
      }
      else
      {
        QgsPointXY pt0 = *mRubberBandPoints->getPoint( 0, 0 );
        QgsPointXY pt1 = *mRubberBandPoints->getPoint( 0, 1 );
        try
        {
          pt0 = mCt->transform( pt0 );
          pt1 = mCt->transform( pt1 );
        }
        catch ( const QgsCsException & )
        {
          emit messageEmitted( tr( "Could not reproject the cross-section extent to 3D map coordinates." ), Qgis::MessageLevel::Warning );
        }

        m3DCanvasWidget->mapCanvas3D()->enableCrossSection(
          pt0,
          pt1,
          mRectangleWidth,
          true
        );

        const QgsSettings settings;
        QColor highlightColor = QColor( settings.value( u"Map/highlight/color"_s, Qgis::DEFAULT_HIGHLIGHT_COLOR.name() ).toString() );
        highlightColor.setAlphaF( 0.5 );
        mRubberBandPolygon->setColor( highlightColor );

        emit finishedSuccessfully();
      }
    }
    else
    {
      if ( mRubberBandPoints->numberOfVertices() == 0 )
      {
        mRubberBandLines->addPoint( point );
        mRubberBandLines->addPoint( point );
      }
      mRubberBandPoints->addPoint( point );
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    clear();
  }
}

void QgsMapToolClippingPlanes::clear() const
{
  clearRubberBand();
  clearHighLightedArea();
}

void QgsMapToolClippingPlanes::clearRubberBand() const
{
  mRubberBandLines->reset( Qgis::GeometryType::Line );
  mRubberBandPoints->reset( Qgis::GeometryType::Point );
}

void QgsMapToolClippingPlanes::clearHighLightedArea() const
{
  mRubberBandPolygon->reset( Qgis::GeometryType::Polygon );
}

QgsGeometry QgsMapToolClippingPlanes::clippedPolygon() const
{
  return mRubberBandPolygon->asGeometry();
}
