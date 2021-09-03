/***************************************************************************
    qgsvertexmarker.cpp  - canvas item which shows a simple vertex marker
    ---------------------
    begin                : February 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QPainter>

#include "qgsvertexmarker.h"


QgsVertexMarker::QgsVertexMarker( QgsMapCanvas *mapCanvas )
  : QgsMapCanvasItem( mapCanvas )
{
  updatePath();
}

void QgsVertexMarker::setIconType( int type )
{
  mIconType = type;
  updatePath();
}

void QgsVertexMarker::setIconSize( int iconSize )
{
  mIconSize = iconSize;
  updatePath();
}

void QgsVertexMarker::setCenter( const QgsPointXY &point )
{
  mCenter = point;
  const QPointF pt = toCanvasCoordinates( mCenter );
  setPos( pt );
}

void QgsVertexMarker::setColor( const QColor &color )
{
  mColor = color;
  update();
}

void QgsVertexMarker::setFillColor( const QColor &color )
{
  mFillColor = color;
  update();
}

void QgsVertexMarker::setPenWidth( int width )
{
  mPenWidth = width;
}

void QgsVertexMarker::paint( QPainter *p )
{
  QPen pen( mColor );
  pen.setWidth( mPenWidth );
  p->setPen( pen );
  const QBrush brush( mFillColor );
  p->setBrush( brush );
  p->drawPath( mPath );
}

void QgsVertexMarker::updatePath()
{
  mPath = QPainterPath();

  const qreal s = ( mIconSize - 1 ) / 2.0;

  switch ( mIconType )
  {
    case ICON_NONE:
      break;

    case ICON_CROSS:
      mPath.moveTo( QPointF( -s, 0 ) );
      mPath.lineTo( QPointF( s, 0 ) );
      mPath.moveTo( QPointF( 0, -s ) );
      mPath.lineTo( QPointF( 0, s ) );
      break;

    case ICON_X:
      mPath.moveTo( QPointF( -s, -s ) );
      mPath.lineTo( QPointF( s, s ) );
      mPath.moveTo( QPointF( -s, s ) );
      mPath.lineTo( QPointF( s, -s ) );
      break;

    case ICON_BOX:
      mPath.addRect( QRectF( -s, -s, s * 2, s * 2 ) );
      break;

    case ICON_CIRCLE:
      mPath.addEllipse( QPointF( 0, 0 ), s, s );
      break;

    case ICON_DOUBLE_TRIANGLE:
      mPath.moveTo( QPointF( -s, -s ) );
      mPath.lineTo( QPointF( s, -s ) );
      mPath.lineTo( QPointF( -s, s ) );
      mPath.lineTo( QPointF( s, s ) );
      mPath.lineTo( QPointF( -s, -s ) );
      break;

    case ICON_TRIANGLE:
      mPath.moveTo( QPointF( -s, s ) );
      mPath.lineTo( QPointF( s, s ) );
      mPath.lineTo( QPointF( 0, -s ) );
      mPath.lineTo( QPointF( -s, s ) );
      break;

    case ICON_RHOMBUS:
      mPath.moveTo( QPointF( 0, -s ) );
      mPath.lineTo( QPointF( -s, 0 ) );
      mPath.lineTo( QPointF( 0, s ) );
      mPath.lineTo( QPointF( s, 0 ) );
      mPath.lineTo( QPointF( 0, -s ) );
      break;

    case ICON_INVERTED_TRIANGLE:
      mPath.moveTo( QPointF( -s, -s ) );
      mPath.lineTo( QPointF( s, -s ) );
      mPath.lineTo( QPointF( 0, s ) );
      mPath.lineTo( QPointF( -s, -s ) );
      break;
  }
}

QRectF QgsVertexMarker::boundingRect() const
{
  const qreal s = qreal( mIconSize + mPenWidth ) / 2.0;
  return QRectF( -s, -s, 2.0 * s, 2.0 * s );
}

void QgsVertexMarker::updatePosition()
{
  setCenter( mCenter );
}
