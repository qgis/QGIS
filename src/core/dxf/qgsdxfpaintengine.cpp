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
  QgsDebugMsg( "***********************Dxf paint engine: drawing polygon*********************" );
}

void QgsDxfPaintEngine::drawRects( const QRectF * rects, int rectCount )
{
  QgsDebugMsg( "***********************Dxf paint engine: drawing rects*********************" );
}

void QgsDxfPaintEngine::drawEllipse( const QRectF& rect )
{
  QgsDebugMsg( "***********************Dxf paint engine: drawing ellipse*********************" );
}

void QgsDxfPaintEngine::drawPath( const QPainterPath& path )
{
  QgsDebugMsg( "***********************Dxf paint engine: drawing path*********************" );
}

void QgsDxfPaintEngine::drawLines( const QLineF* lines, int lineCount )
{
  QgsDebugMsg( "***********************Dxf paint engine: drawing path*********************" );
}
