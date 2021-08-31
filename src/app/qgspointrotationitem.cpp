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
#include <QLocale>
#include <cmath>
#include "qgsguiutils.h"

QgsPointRotationItem::QgsPointRotationItem( QgsMapCanvas *canvas )
  : QgsMapCanvasItem( canvas )
{
  //setup font
  mFont.setPointSize( 12 );
  mFont.setBold( true );

  QImage im( 24, 24, QImage::Format_ARGB32 );
  im.fill( Qt::transparent );
  setSymbol( im );
}

void QgsPointRotationItem::paint( QPainter *painter )
{
  if ( !painter )
  {
    return;
  }
  painter->setRenderHint( QPainter::Antialiasing, true );
  painter->save();

  //do a bit of trigonometry to find out how to transform a rotated item such that the center point is at the point feature
  double x = 0;
  double y = 0;
  double h, dAngel;
  if ( mPixmap.width() > 0 && mPixmap.height() > 0 )
  {
    h = std::sqrt( ( double ) mPixmap.width() * mPixmap.width() + mPixmap.height() * mPixmap.height() ) / 2; //the half of the item diagonal
    dAngel = std::acos( mPixmap.width() / ( h * 2 ) ) * 180 / M_PI; //the diagonal angel of the original rect
    x = h * std::cos( ( painterRotation( mRotation ) - dAngel ) * M_PI / 180 );
    y = h * std::sin( ( painterRotation( mRotation ) - dAngel ) * M_PI / 180 );
  }

  painter->rotate( painterRotation( mRotation ) );
  painter->translate( x - mPixmap.width() / 2.0, -y - mPixmap.height() / 2.0 );
  painter->drawPixmap( 0, 0, mPixmap );

  //draw arrow, using a red line over a thicker white line so that the arrow is visible against a range of backgrounds
  QPen pen;
  pen.setWidth( QgsGuiUtils::scaleIconSize( 4 ) );
  pen.setColor( QColor( Qt::white ) );
  painter->setPen( pen );
  painter->drawPath( mArrowPath );
  pen.setWidth( QgsGuiUtils::scaleIconSize( 1 ) );
  pen.setColor( QColor( Qt::red ) );
  painter->setPen( pen );
  painter->drawPath( mArrowPath );
  painter->restore();

  //draw numeric value beside the symbol
  painter->save();

  QPen bufferPen;
  bufferPen.setColor( Qt::white );
  bufferPen.setWidthF( QgsGuiUtils::scaleIconSize( 4 ) );
  const QFontMetricsF fm( mFont );
  QPainterPath label;
  const double rotationText = mRotation * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AngleDegrees,
                              mRotationUnit );
  label.addText( mPixmap.width(),
                 mPixmap.height() / 2.0 + fm.height() / 2.0,
                 mFont,
                 QgsUnitTypes::formatAngle( rotationText, -1, mRotationUnit ) );
  painter->setPen( bufferPen );
  painter->setBrush( Qt::NoBrush );
  painter->drawPath( label );
  painter->setPen( Qt::NoPen );
  painter->setBrush( QBrush( Qt::black ) );
  painter->drawPath( label );

  painter->restore();
}

void QgsPointRotationItem::setPointLocation( const QgsPointXY &p )
{
  const QPointF transformedPoint = toCanvasCoordinates( p );
  setPos( transformedPoint.x() - mPixmap.width() / 2.0, transformedPoint.y() - mPixmap.height() / 2.0 );
}

void QgsPointRotationItem::setRotationUnit( const QgsUnitTypes::AngleUnit &rotationUnit )
{
  mRotationUnit = rotationUnit;
}

void QgsPointRotationItem::setSymbol( const QImage &symbolImage )
{
  mPixmap = QPixmap::fromImage( symbolImage );
  const QFontMetricsF fm( mFont );

  //set item size: 6283 millirad arcseconds = 360°
  mItemSize.setWidth( mPixmap.width() + fm.horizontalAdvance( QStringLiteral( "6283 millirad" ) ) );
  const double pixmapHeight = mPixmap.height();
  const double fontHeight = fm.height();
  if ( pixmapHeight >= fontHeight )
  {
    mItemSize.setHeight( mPixmap.height() );
  }
  else
  {
    mItemSize.setHeight( fm.height() );
  }

  const double halfItemWidth = mPixmap.width() / 2.0;
  mArrowPath = QPainterPath();
  mArrowPath.moveTo( halfItemWidth, pixmapHeight );
  mArrowPath.lineTo( halfItemWidth, 0 );
  mArrowPath.moveTo( mPixmap.width() * 0.25, pixmapHeight * 0.25 );
  mArrowPath.lineTo( halfItemWidth, 0 );
  mArrowPath.lineTo( mPixmap.width() * 0.75, pixmapHeight * 0.25 );
}

int QgsPointRotationItem::painterRotation( int rotation ) const
{
  if ( mOrientation == Clockwise )
  {
    return rotation;
  }

  return 360 - ( rotation % 360 );
}

