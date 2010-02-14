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

class QgsGCPCanvasItem : public QgsMapCanvasItem {
public:
  QgsGCPCanvasItem( QgsMapCanvas* mapCanvas, const QgsPoint& rasterCoords, const QgsPoint& worldCoords, bool isGCPSource = true );
  
  //! draws point information
  virtual void paint( QPainter* p );

  //! handler for manual updating of position and size
  virtual QRectF boundingRect() const;

  virtual void updatePosition();
private:
  bool mIsGCPSource;
  QSizeF mTextBounds;
  QgsPoint mRasterCoords, mWorldCoords;
};

QgsGeorefDataPoint::QgsGeorefDataPoint( QgsMapCanvas* srcCanvas, QgsMapCanvas *dstCanvas, int id,
                                        const QgsPoint& pixelCoords, const QgsPoint& mapCoords )
    : mId( id ), mPixelCoords( pixelCoords ), mMapCoords( mapCoords )
{
  mGCPSourceItem = new QgsGCPCanvasItem(srcCanvas, pixelCoords, mapCoords, true);
  mGCPDestinationItem = new QgsGCPCanvasItem(dstCanvas, pixelCoords, mapCoords, false);
  mGCPSourceItem->show();
  mGCPDestinationItem->show();
}

QgsGeorefDataPoint::~QgsGeorefDataPoint()
{
  delete mGCPSourceItem;
  delete mGCPDestinationItem;
}


QgsGCPCanvasItem::QgsGCPCanvasItem(QgsMapCanvas* mapCanvas, const QgsPoint& rasterCoords, const QgsPoint& worldCoords, 
                                   bool isGCPSource ) : QgsMapCanvasItem( mapCanvas )
{
  mRasterCoords = rasterCoords;
  mWorldCoords = worldCoords; 
  mIsGCPSource = isGCPSource;
  updatePosition();
}



void QgsGCPCanvasItem::paint( QPainter* p )
{
  // draw the point
  p->setPen( Qt::black );
  p->setBrush( Qt::red );
  p->drawRect( -2, -2, 5, 5 );
  
  if (mIsGCPSource) 
  {
    QString msg = QString( "X %1\nY %2" ).arg( QString::number( mWorldCoords.x(), 'f' ) ).arg( QString::number( mWorldCoords.y(), 'f' ) );
    QFont font;
    p->setFont( QFont( "helvetica", 9 ) );
    QRect textBounds = p->boundingRect( 4, 4, 10, 10, Qt::AlignLeft, msg );
    p->setBrush( Qt::yellow );
    p->drawRect( 2, 2, textBounds.width() + 4, textBounds.height() + 4 );
    p->drawText( textBounds, Qt::AlignLeft, msg );
    mTextBounds = QSizeF(textBounds.width() + 4, textBounds.height() + 4);
  }
  else
    mTextBounds = QSizeF(0, 0);
}

QRectF QgsGCPCanvasItem::boundingRect() const
{
  return QRectF( -2, -2, mTextBounds.width() + 2, mTextBounds.height() + 2 );
}

void QgsGCPCanvasItem::updatePosition()
{
  setPos( toCanvasCoordinates( mIsGCPSource ? mRasterCoords : mWorldCoords) );
}
