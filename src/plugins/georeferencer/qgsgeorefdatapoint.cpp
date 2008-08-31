/***************************************************************************
     qgsgeorefdatapoint.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:02:45 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QPainter>
#include "qgsgeorefdatapoint.h"
#include "qgsmapcanvas.h"

QgsGeorefDataPoint::QgsGeorefDataPoint( QgsMapCanvas* mapCanvas, int id,
                                        const QgsPoint& pixelCoords, const QgsPoint& mapCoords )
    : QgsMapCanvasItem( mapCanvas ), mId( id ),
    mPixelCoords( pixelCoords ), mMapCoords( mapCoords )
{
  updatePosition();
}


void QgsGeorefDataPoint::paint( QPainter* p )
{
  QString msg = QString( "X %1\nY %2" ).arg( QString::number( mMapCoords.x(), 'f' ) ).arg( QString::number( mMapCoords.y(), 'f' ) );
  QFont font;
  p->setFont( QFont( "helvetica", 9 ) );
  p->setPen( Qt::black );
  p->setBrush( Qt::red );
  p->drawRect( -2, -2, 5, 5 );
  QRect textBounds = p->boundingRect( 4, 4, 10, 10, Qt::AlignLeft, msg );
  p->setBrush( Qt::yellow );
  p->drawRect( 2, 2, textBounds.width() + 4, textBounds.height() + 4 );
  p->drawText( textBounds, Qt::AlignLeft, msg );

  mTextBounds = QSizeF( textBounds.width(), textBounds.height() );
}

QRectF QgsGeorefDataPoint::boundingRect() const
{
  return QRectF( -2, -2, mTextBounds.width() + 6, mTextBounds.height() + 6 );
}

void QgsGeorefDataPoint::updatePosition()
{
  setPos( toCanvasCoordinates( mPixelCoords ) );
}
