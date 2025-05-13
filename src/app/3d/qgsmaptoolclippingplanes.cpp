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
#include "moc_qgsmaptoolclippingplanes.cpp"
#include "qgs3dmapcanvas.h"
#include "qgs3dutils.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgspolygon.h"
#include "qgs3dmapcanvaswidget.h"
#include "qgs3dmapscene.h"
#include "qgsmessagebar.h"
#include "qgisapp.h"

#include <QVector4D>


QgsMapToolClippingPlanes::QgsMapToolClippingPlanes( QgsMapCanvas *canvas, Qgs3DMapCanvasWidget *mapCanvas )
  : QgsMapTool( canvas ), m3DCanvas( mapCanvas )
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
  if ( mClicked || mRubberBandPoints->numberOfVertices() == 0 )
    return;

  const QgsPointXY point = toMapCoordinates( e->pos() );
  if ( mRubberBandPoints->numberOfVertices() == 2 )
  {
    QgsVector vec( *mRubberBandPoints->getPoint( 0, 1 ) - *mRubberBandPoints->getPoint( 0, 0 ) );
    vec = vec.normalized().perpVector();
    mRectangleWidth = QgsGeometryUtils::distToInfiniteLine(
      QgsPoint( point ),
      QgsPoint( *mRubberBandPoints->getPoint( 0, 0 ) ),
      QgsPoint( *mRubberBandPoints->getPoint( 0, 1 ) )
    );
    const QVector<QgsPointXY> points( {
      *mRubberBandPoints->getPoint( 0, 0 ) + vec * mRectangleWidth, //! top left corner
      *mRubberBandPoints->getPoint( 0, 1 ) + vec * mRectangleWidth, //! top right corner
      *mRubberBandPoints->getPoint( 0, 1 ) - vec * mRectangleWidth, //! bottom right corner
      *mRubberBandPoints->getPoint( 0, 0 ) - vec * mRectangleWidth  //! bottom left corner
    } );
    mRubberBandPolygon->setToGeometry( QgsGeometry( new QgsPolygon( new QgsLineString( points ) ) ) );
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
  mClicked = true;
}

void QgsMapToolClippingPlanes::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( e->button() == Qt::LeftButton )
  {
    const QgsPointXY point = toMapCoordinates( e->pos() );
    if ( mRubberBandPoints->numberOfVertices() == 2 )
    {
      //check if cross-section is in canvas extents
      const QgsGeometry crossSectionPolygon = mRubberBandPolygon->asGeometry();
      if ( !crossSectionPolygon.intersects( m3DCanvas->mapCanvas3D()->scene()->sceneExtent() ) )
      {
        clear();
        QgsMessageBar *msgBar = QgisApp::instance()->messageBar();
        msgBar->pushInfo( QString(), tr( "The cross section is outside of the scene extent, please select a new one!" ) );
      }
      else
      {
        const QgsVector3D startPoint = QgsVector3D( mRubberBandPoints->getPoint( 0, 0 )->x(), mRubberBandPoints->getPoint( 0, 0 )->y(), 0 );
        const QgsVector3D endPoint = QgsVector3D( mRubberBandPoints->getPoint( 0, 1 )->x(), mRubberBandPoints->getPoint( 0, 1 )->y(), 0 );
        const QList<QVector4D> clippingPlanes = Qgs3DUtils::lineSegmentToClippingPlanes(
          QgsVector3D( startPoint.x(), startPoint.y(), 0 ),
          QgsVector3D( endPoint.x(), endPoint.y(), 0 ),
          mRectangleWidth,
          m3DCanvas->mapCanvas3D()->mapSettings()->origin()
        );

        // calculate the middle of the front side defined by clipping planes
        QgsVector linePerpVec( ( endPoint - startPoint ).x(), ( endPoint - startPoint ).y() );
        linePerpVec = -linePerpVec.normalized().perpVector();
        const QgsVector3D linePerpVec3D( linePerpVec.x(), linePerpVec.y(), 0 );
        const QgsVector3D frontStartPoint( startPoint + linePerpVec3D * mRectangleWidth );
        const QgsVector3D frontEndPoint( endPoint + linePerpVec3D * mRectangleWidth );

        const QgsCameraPose camPose = Qgs3DUtils::lineSegmentToCameraPose(
          frontStartPoint,
          frontEndPoint,
          m3DCanvas->mapCanvas3D()->scene()->elevationRange( true ),
          m3DCanvas->mapCanvas3D()->scene()->cameraController()->camera()->fieldOfView(),
          m3DCanvas->mapCanvas3D()->mapSettings()->origin()
        );

        m3DCanvas->enableClippingPlanes( clippingPlanes, camPose );

        const QgsSettings settings;
        QColor highlightColor = QColor( settings.value( QStringLiteral( "Map/highlight/color" ), Qgis::DEFAULT_HIGHLIGHT_COLOR.name() ).toString() );
        highlightColor.setAlphaF( 0.5 );
        mRubberBandPolygon->setColor( highlightColor );
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
    mClicked = false;
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
