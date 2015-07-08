/***************************************************************************
                         qgsgeometryrubberband.cpp
                         -------------------------
    begin                : December 2014
    copyright            : (C) 2014 by Marco Hugentobler
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

#include "qgsgeometryrubberband.h"
#include "qgsabstractgeometryv2.h"
#include "qgsmapcanvas.h"
#include "qgspointv2.h"
#include <QPainter>

QgsGeometryRubberBand::QgsGeometryRubberBand( QgsMapCanvas* mapCanvas, QGis::GeometryType geomType ): QgsMapCanvasItem( mapCanvas ),
    mGeometry( 0 ), mIconSize( 5 ), mIconType( ICON_CIRCLE ), mGeometryType( geomType )
{
  mPen = QPen( QColor( 255, 0, 0 ) );
  mBrush = QBrush( QColor( 255, 0, 0 ) );
}

QgsGeometryRubberBand::~QgsGeometryRubberBand()
{
  delete mGeometry;
}

void QgsGeometryRubberBand::paint( QPainter* painter )
{
  if ( !mGeometry || !painter )
  {
    return;
  }

  painter->save();
  painter->translate( -pos() );

  if ( mGeometryType == QGis::Polygon )
  {
    painter->setBrush( mBrush );
  }
  else
  {
    painter->setBrush( Qt::NoBrush );
  }
  painter->setPen( mPen );


  QgsAbstractGeometryV2* paintGeom = mGeometry->clone();

  paintGeom->transform( mMapCanvas->getCoordinateTransform()->transform() );
  paintGeom->draw( *painter );

  //draw vertices
  QgsVertexId vertexId;
  QgsPointV2 vertex;
  while ( paintGeom->nextVertex( vertexId, vertex ) )
  {
    drawVertex( painter, vertex.x(), vertex.y() );
  }

  delete paintGeom;
  painter->restore();
}

void QgsGeometryRubberBand::drawVertex( QPainter* p, double x, double y )
{
  qreal s = ( mIconSize - 1 ) / 2;

  switch ( mIconType )
  {
    case ICON_NONE:
      break;

    case ICON_CROSS:
      p->drawLine( QLineF( x - s, y, x + s, y ) );
      p->drawLine( QLineF( x, y - s, x, y + s ) );
      break;

    case ICON_X:
      p->drawLine( QLineF( x - s, y - s, x + s, y + s ) );
      p->drawLine( QLineF( x - s, y + s, x + s, y - s ) );
      break;

    case ICON_BOX:
      p->drawLine( QLineF( x - s, y - s, x + s, y - s ) );
      p->drawLine( QLineF( x + s, y - s, x + s, y + s ) );
      p->drawLine( QLineF( x + s, y + s, x - s, y + s ) );
      p->drawLine( QLineF( x - s, y + s, x - s, y - s ) );
      break;

    case ICON_FULL_BOX:
      p->drawRect( x - s, y - s, mIconSize, mIconSize );
      break;

    case ICON_CIRCLE:
      p->drawEllipse( x - s, y - s, mIconSize, mIconSize );
      break;
  }
}

void QgsGeometryRubberBand::setGeometry( QgsAbstractGeometryV2* geom )
{
  delete mGeometry;
  mGeometry = geom;

  if ( mGeometry )
  {
    setRect( rubberBandRectangle() );
  }
}

void QgsGeometryRubberBand::moveVertex( const QgsVertexId& id, const QgsPointV2& newPos )
{
  if ( mGeometry )
  {
    mGeometry->moveVertex( id, newPos );
    setRect( rubberBandRectangle() );
  }
}

void QgsGeometryRubberBand::setFillColor( const QColor& c )
{
  mBrush.setColor( c );
}

void QgsGeometryRubberBand::setOutlineColor( const QColor& c )
{
  mPen.setColor( c );
}

void QgsGeometryRubberBand::setOutlineWidth( int width )
{
  mPen.setWidth( width );
}

void QgsGeometryRubberBand::setLineStyle( Qt::PenStyle penStyle )
{
  mPen.setStyle( penStyle );
}

void QgsGeometryRubberBand::setBrushStyle( Qt::BrushStyle brushStyle )
{
  mBrush.setStyle( brushStyle );
}

QgsRectangle QgsGeometryRubberBand::rubberBandRectangle() const
{
  qreal scale = mMapCanvas->mapUnitsPerPixel();
  qreal s = ( mIconSize - 1 ) / 2.0 * scale;
  qreal p = mPen.width() * scale;
  return mGeometry->boundingBox().buffer( s + p );
}
