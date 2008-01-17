/***************************************************************************
    qgsmapcanvasmap.h  -  draws the map in map canvas
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
/* $Id$ */

#ifndef QGSMAPCANVASMAP_H
#define QGSMAPCANVASMAP_H

#include <QGraphicsRectItem>
#include <QPixmap>


class QgsMapRender;
class QgsMapCanvas;

class GUI_EXPORT QgsMapCanvasMap : public QGraphicsRectItem
{
  public:
    
    //! constructor
    QgsMapCanvasMap(QgsMapCanvas* canvas);
    
    //! resize canvas item and pixmap
    void resize(QSize size);
    
    void enableAntiAliasing(bool flag) { mAntiAliasing = flag; }
    
    void useQImageToRender(bool flag) { mUseQImageToRender = flag; }

    //! renders map using QgsMapRender to mPixmap
    void render();
    
    void setBgColor(const QColor& color) { mBgColor = color; }
    
    void setPanningOffset(const QPoint& point);
    
    //deprecated. Please use paintDevice() function
    //which is also save in case QImage is used
    QPixmap& pixmap() { return mPixmap; }

    QPaintDevice& paintDevice();
    
    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*);

    QRectF boundingRect() const;
  
  
  private:

    //! indicates whether antialiasing will be used for rendering
    bool mAntiAliasing;
    
    //! Whether to use a QPixmap or a QImage for the rendering
    bool mUseQImageToRender;
    
    QPixmap mPixmap;
    QImage mImage;

    //QgsMapRender* mRender;
    QgsMapCanvas* mCanvas;
    
    QColor mBgColor;
    
    QPoint mOffset;
};

#endif
