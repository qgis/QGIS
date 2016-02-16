/***************************************************************************
    qgspointrotationitem.cpp
    ------------------------
    begin                : September 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointrotationitem.h"
#include <QPainter>
#include <cmath>

QgsPointRotationItem::QgsPointRotationItem( QgsMapCanvas* canvas )
    : QgsMapCanvasItem( canvas )
    , mOrientation( Clockwise )
    , mRotation( 0.0 )
{
  //setup font
  mFont.setPointSize( 12 );
  mFont.setBold( true );
}

QgsPointRotationItem::QgsPointRotationItem()
    : QgsMapCanvasItem( nullptr )
    , mOrientation( Clockwise )
    , mRotation( 0.0 )
{

}

QgsPointRotationItem::~QgsPointRotationItem()
{

}

void QgsPointRotationItem::paint( QPainter * painter )
{
  if ( !painter )
  {
    return;
  }
  painter->save();

  //do a bit of trigonometry to find out how to transform a rotated item such that the center point is at the point feature
  double x = 0;
  double y = 0;
  double h, dAngel;
  if ( mPixmap.width() > 0 && mPixmap.height() > 0 )
  {
    h = sqrt(( double ) mPixmap.width() * mPixmap.width() + mPixmap.height() * mPixmap.height() ) / 2; //the half of the item diagonal
    dAngel = acos( mPixmap.width() / ( h * 2 ) ) * 180 / M_PI; //the diagonal angel of the original rect
    x = h * cos(( painterRotation( mRotation ) - dAngel ) * M_PI / 180 );
    y = h * sin(( painterRotation( mRotation ) - dAngel ) * M_PI / 180 );
  }

  painter->rotate( painterRotation( mRotation ) );
  painter->translate( x - mPixmap.width() / 2.0, -y - mPixmap.height() / 2.0 );
  painter->drawPixmap( 0, 0, mPixmap );

  //draw numeric value beside the symbol
  painter->restore();
  QFontMetricsF fm( mFont );
  painter->fillRect( mPixmap.width(), 0, mItemSize.width() - mPixmap.width(), mItemSize.height(), QColor( Qt::white ) );
  painter->setFont( mFont );
  painter->drawText( mPixmap.width(), mPixmap.height() / 2.0 + fm.height() / 2.0, QString::number( mRotation ) );
}

void QgsPointRotationItem::setPointLocation( const QgsPoint& p )
{
  QPointF transformedPoint = toCanvasCoordinates( p );
  setPos( transformedPoint.x() - mPixmap.width() / 2.0, transformedPoint.y() - mPixmap.height() / 2.0 );
}

void QgsPointRotationItem::setSymbol( const QImage& symbolImage )
{
  mPixmap = QPixmap::fromImage( symbolImage );
  QFontMetricsF fm( mFont );

  //draw arrow
  QPainter p( &mPixmap );
  QPen pen;
  pen.setWidth( 1 );
  pen.setColor( QColor( Qt::red ) );
  p.setPen( pen );
  int halfItemWidth = mPixmap.width() / 2;
  int quarterItemHeight = mPixmap.height() / 4;
  p.drawLine( halfItemWidth, mPixmap.height(), halfItemWidth, 0 );
  p.drawLine( halfItemWidth, 0, mPixmap.width() / 4, quarterItemHeight );
  p.drawLine( halfItemWidth, 0, mPixmap.width() * 0.75, quarterItemHeight );

  //set item size
  mItemSize.setWidth( mPixmap.width() + fm.width( "360" ) );
  double pixmapHeight = mPixmap.height();
  double fontHeight = fm.height();
  if ( pixmapHeight >= fontHeight )
  {
    mItemSize.setHeight( mPixmap.height() );
  }
  else
  {
    mItemSize.setHeight( fm.height() );
  }
}

int QgsPointRotationItem::painterRotation( int rotation ) const
{
  if ( mOrientation == Clockwise )
  {
    return rotation;
  }

  return 360 - ( rotation % 360 );
}

