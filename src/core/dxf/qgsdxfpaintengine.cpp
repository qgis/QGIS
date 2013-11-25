/***************************************************************************
                         qgsdxpaintengine.cpp
                         --------------------
    begin                : November 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdxfpaintengine.h"
#include "qgsdxfexport.h"
#include "qgsdxfpaintdevice.h"
#include "qgslogger.h"

QgsDxfPaintEngine::QgsDxfPaintEngine( const QgsDxfPaintDevice* dxfDevice, QgsDxfExport* dxf ): QPaintEngine( QPaintEngine::AllFeatures /*QPaintEngine::PainterPaths | QPaintEngine::PaintOutsidePaintEvent*/ )
    , mPaintDevice( dxfDevice ), mDxf( dxf )
{

}

QgsDxfPaintEngine::~QgsDxfPaintEngine()
{

}

bool QgsDxfPaintEngine::begin( QPaintDevice* pdev )
{
  Q_UNUSED( pdev );
  return true;
}

bool QgsDxfPaintEngine::end()
{
  return true;
}

QPaintEngine::Type QgsDxfPaintEngine::type() const
{
  return QPaintEngine::User;
}

void QgsDxfPaintEngine::drawPixmap( const QRectF& r, const QPixmap& pm, const QRectF& sr )
{
  Q_UNUSED( r ); Q_UNUSED( pm ); Q_UNUSED( sr );
}

void QgsDxfPaintEngine::updateState( const QPaintEngineState& state )
{
  if ( state.state() | QPaintEngine::DirtyTransform )
  {
    mTransform = state.transform();
  }
  if ( state.state() | QPaintEngine::DirtyPen )
  {
    mPen = state.pen();
  }
}

void QgsDxfPaintEngine::drawPolygon( const QPointF* points, int pointCount, PolygonDrawMode mode )
{
  if ( !mDxf || !mPaintDevice )
  {
    return;
  }

  QgsPolyline polyline( pointCount );
  for ( int i = 0; i < pointCount; ++i )
  {
    polyline[i] = toDxfCoordinates( points[i] );
  }

  double width = mPen.widthF() * mPaintDevice->widthScaleFactor();
  mDxf->writePolyline( polyline, mLayer, "CONTINUOUS", currentPenColor(), width, mode != QPaintEngine::PolylineMode );
}

void QgsDxfPaintEngine::drawRects( const QRectF* rects, int rectCount )
{
  if ( !mDxf || !mPaintDevice )
  {
    return;
  }

  for ( int i = 0; i < rectCount; ++i )
  {
    double left = rects[i].left();
    double right = rects[i].right();
    double top = rects[i].top();
    double bottom = rects[i].bottom();
    QgsPoint pt1 = toDxfCoordinates( QPointF( left, bottom ) );
    QgsPoint pt2 = toDxfCoordinates( QPointF( right, bottom ) );
    QgsPoint pt3 = toDxfCoordinates( QPointF( left, top ) );
    QgsPoint pt4 = toDxfCoordinates( QPointF( right, top ) );
    mDxf->writeSolid( mLayer, currentPenColor(), pt1, pt2, pt3, pt4 );
  }
}

void QgsDxfPaintEngine::drawEllipse( const QRectF& rect )
{
  QgsDebugMsg( "***********************Dxf paint engine: drawing ellipse*********************" );
}

void QgsDxfPaintEngine::drawPath( const QPainterPath& path )
{
  QList<QPolygonF> polygonList = path.toFillPolygons();
  QList<QPolygonF>::const_iterator pIt = polygonList.constBegin();
  for ( ; pIt != polygonList.constEnd(); ++pIt )
  {
    drawPolygon( pIt->constData(), pIt->size(), pIt->isClosed() ? QPaintEngine::OddEvenMode : QPaintEngine::PolylineMode );
  }
}

void QgsDxfPaintEngine::drawLines( const QLineF* lines, int lineCount )
{
  QgsDebugMsg( "***********************Dxf paint engine: drawing path*********************" );
}

QgsPoint QgsDxfPaintEngine::toDxfCoordinates( const QPointF& pt ) const
{
  if ( !mPaintDevice || !mDxf )
  {
    return QgsPoint( pt.x(), pt.y() );
  }

  QPointF dxfPt = mPaintDevice->dxfCoordinates( mTransform.map( pt ) );
  return QgsPoint( dxfPt.x(), dxfPt.y() );
}

int QgsDxfPaintEngine::currentPenColor() const
{
  if ( !mDxf )
  {
    return 0;
  }

  return mDxf->closestColorMatch( mPen.color().rgb() );
}
