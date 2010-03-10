/***************************************************************************
     qgsgcpcanvasitem.h
     --------------------------------------
    Date                 : 14-Feb-2010
    Copyright            : (C) 2010 by Jack R, Maxim Dubinin (GIS-Lab)
    Email                : sim@gis-lab.info
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsgcpcanvasitem.h"

QgsGCPCanvasItem::QgsGCPCanvasItem( QgsMapCanvas* mapCanvas, const QgsPoint& rasterCoords,
                                    const QgsPoint& worldCoords, bool isGCPSource )
    : QgsMapCanvasItem( mapCanvas )
    , mPointBrush( Qt::red )
    , mLabelBrush( Qt::yellow )
    , mRasterCoords( rasterCoords )
    , mWorldCoords( worldCoords )
    , mId( -1 )
    , mIsGCPSource( isGCPSource )
    , mEnabled( true )
{
  setFlags( QGraphicsItem::ItemIsMovable );

  updatePosition();
}

void QgsGCPCanvasItem::paint( QPainter* p )
{
  p->setRenderHint( QPainter::Antialiasing );
  p->setOpacity( mEnabled ? 1.0 : 0.3 );

  // draw the point
  p->setPen( Qt::black );
  p->setBrush( mPointBrush );
  p->drawEllipse( -2, -2, 5, 5 );

  QSettings s;
  bool showIDs = s.value( "/Plugin-GeoReferencer/Config/ShowId" ).toBool();
  if ( !showIDs && mIsGCPSource )
  {
    QString msg = QString( "X %1\nY %2" ).arg( QString::number( mWorldCoords.x(), 'f' ) ).
                  arg( QString::number( mWorldCoords.y(), 'f' ) );
    p->setFont( QFont( "helvetica", 9 ) );
    QRect textBounds = p->boundingRect( 6, 6, 10, 10, Qt::AlignLeft, msg );
    p->setBrush( mLabelBrush );
    p->drawRect( textBounds.x() - 2, textBounds.y() - 2, textBounds.width() + 4, textBounds.height() + 4 );
    p->drawText( textBounds, Qt::AlignLeft, msg );
    mTextBounds = QSizeF( textBounds.width() + 4, textBounds.height() + 4 );
  }
  else if ( showIDs )
  {
    p->setFont( QFont( "helvetica", 12 ) );
    QString msg = QString::number( mId );
    p->setBrush( mLabelBrush );
    p->drawRect( 5, 4, p->fontMetrics().width( msg ) + 2, 14 );
    p->drawText( 6, 16, msg );
    QFontMetrics fm = p->fontMetrics();
    mTextBounds = QSize( fm.width( msg ) + 4, fm.height() + 4 );
  }
  //  else
  //    mTextBounds = QSizeF(0, 0);
}

QRectF QgsGCPCanvasItem::boundingRect() const
{
  return QRectF( -2, -2, mTextBounds.width() + 6, mTextBounds.height() + 6 );
}

QPainterPath QgsGCPCanvasItem::shape() const
{
  QPainterPath p;
  p.addEllipse( -2, -2, 5, 5 );
  p.addRect( 6, 6, mTextBounds.width(), mTextBounds.height() );

  return p;
}

void QgsGCPCanvasItem::setEnabled( bool enabled )
{
  mEnabled = enabled;
  mPointBrush = enabled ? QBrush( Qt::red ) : QBrush( Qt::gray );
  mLabelBrush = enabled ? QBrush( Qt::yellow ) : QBrush( Qt::gray );
  update();
}

void QgsGCPCanvasItem::setRasterCoords( QgsPoint p )
{
  mRasterCoords = p;
}

void QgsGCPCanvasItem::setWorldCoords( QgsPoint p )
{
  mWorldCoords = p;
}

void QgsGCPCanvasItem::setId( int id )
{
  mId = id;
  update();
}

void QgsGCPCanvasItem::updatePosition()
{
  setPos( toCanvasCoordinates( mIsGCPSource ? mRasterCoords : mWorldCoords ) );
}
