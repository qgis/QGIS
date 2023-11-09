/***************************************************************************
    qgsmapcanvasmap.cpp  -  draws the map in map canvas
    ----------------------
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

#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvasmap.h"
#include "qgsmaprendererjob.h"
#include "qgsmapsettings.h"
#include "qgsmaplayer.h"

#include <QPainter>

/// @cond PRIVATE

QgsMapCanvasMap::QgsMapCanvasMap( QgsMapCanvas *canvas )
  : QgsMapCanvasItem( canvas )
{
  setZValue( -10 );
}

void QgsMapCanvasMap::setContent( const QImage &image, const QgsRectangle &rect )
{
  mPreviewImages.clear();

  mImage = image;

  // For true retro fans: this is approximately how the graphics looked like in 1990
  if ( mMapCanvas->property( "retro" ).toBool() )
    mImage = mImage.scaled( mImage.width() / 3, mImage.height() / 3 )
             .convertToFormat( QImage::Format_Indexed8, Qt::OrderedDither | Qt::OrderedAlphaDither );

  setRect( rect );
}

void QgsMapCanvasMap::addPreviewImage( const QImage &image, const QgsRectangle &rect )
{
  mPreviewImages.append( qMakePair( image, rect ) );
  update();
}

QRectF QgsMapCanvasMap::boundingRect() const
{
  const double width = mItemSize.width();
  const double height = mItemSize.height();

  return QRectF( -width, -height, 3 * width, 3 * height );
}

void QgsMapCanvasMap::paint( QPainter *painter )
{
  // setRect() makes the size +2 :-(
  const int w = std::round( mItemSize.width() ) - 2;
  const int h = std::round( mItemSize.height() ) - 2;

  bool scale = false;
  if ( mImage.size() != QSize( w, h ) * mImage.devicePixelRatioF() )
  {
    QgsDebugMsgLevel( QStringLiteral( "map paint DIFFERENT SIZE: img %1,%2  item %3,%4" )
                      .arg( mImage.width() / mImage.devicePixelRatioF() )
                      .arg( mImage.height() / mImage.devicePixelRatioF() )
                      .arg( w ).arg( h ), 2 );
    // This happens on zoom events when ::paint is called before
    // the renderer has completed
    scale = true;
  }

  /*Offset between 0/0 and mRect.xMinimum/mRect.yMinimum.
  We need to consider the offset, because mRect is not updated yet and there might be an offset*/
  const QgsPointXY pt = toMapCoordinates( QPoint( 0, 0 ) );
  const double offsetX = pt.x() - mRect.xMinimum();
  const double offsetY = pt.y() - mRect.yMaximum();

  //draw preview images first
  QList< QPair< QImage, QgsRectangle > >::const_iterator imIt = mPreviewImages.constBegin();
  for ( ; imIt != mPreviewImages.constEnd(); ++imIt )
  {
    const QPointF ul = toCanvasCoordinates( QgsPoint( imIt->second.xMinimum() + offsetX, imIt->second.yMaximum() + offsetY ) );
    const QPointF lr = toCanvasCoordinates( QgsPoint( imIt->second.xMaximum() + offsetX, imIt->second.yMinimum() + offsetY ) );
    painter->drawImage( QRectF( ul.x(), ul.y(), lr.x() - ul.x(), lr.y() - ul.y() ), imIt->first, QRect( 0, 0, imIt->first.width(), imIt->first.height() ) );
  }

  if ( scale )
    painter->drawImage( QRect( 0, 0, w, h ), mImage );
  else
    painter->drawImage( 0, 0, mImage );

  // For debugging:
#if 0
  QRectF br = boundingRect();
  QPointF c = br.center();
  double rad = std::max( br.width(), br.height() ) / 10;
  painter->drawRoundedRect( br, rad, rad );
  painter->drawLine( QLineF( 0, 0, br.width(), br.height() ) );
  painter->drawLine( QLineF( br.width(), 0, 0, br.height() ) );

  double nw = br.width() * 0.5;
  double nh = br.height() * 0.5;
  br = QRectF( c - QPointF( nw / 2, nh / 2 ), QSize( nw, nh ) );
  painter->drawRoundedRect( br, rad, rad );

  nw = br.width() * 0.5;
  nh = br.height() * 0.5;
  br = QRectF( c - QPointF( nw / 2, nh / 2 ), QSize( nw, nh ) );
  painter->drawRoundedRect( br, rad, rad );
#endif
}

/// @endcond
