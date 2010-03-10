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

#ifndef QGSGCPCANVASITEM_H
#define QGSGCPCANVASITEM_H

#include "qgsmapcanvas.h"
#include "qgsmapcanvasitem.h"

class QgsGCPCanvasItem : public QgsMapCanvasItem
{
  public:
    QgsGCPCanvasItem( QgsMapCanvas* mapCanvas, const QgsPoint& rasterCoords,
                      const QgsPoint& worldCoords, bool isGCPSource/* = true*/ );

    //! draws point information
    void paint( QPainter* p );

    //! handler for manual updating of position and size
    QRectF boundingRect() const;

    QPainterPath shape() const;

    void setEnabled( bool enabled );

    void setRasterCoords( QgsPoint p );
    void setWorldCoords( QgsPoint p );

    int id() { return mId; }
    void setId( int id );

    void updatePosition();

  private:
    QSizeF mTextBounds;
    QBrush mPointBrush;
    QBrush mLabelBrush;
    QgsPoint mRasterCoords;
    QgsPoint mWorldCoords;

    int mId;
    bool mIsGCPSource;
    bool mEnabled;
};

#endif // QGSGCPCANVASITEM_H
