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

#include <Q3CanvasRectangle>
#include <QPixmap>

#define RTTI_Map 11111

class QgsMapRender;

class QgsMapCanvasMap : public Q3CanvasRectangle
{
  public:
    
    //! constructor
    QgsMapCanvasMap(Q3Canvas *canvas, QgsMapRender* render);
    
    //! resize canvas item and pixmap
    void resize(QSize size);
    
    //! unique identification for this canvas item
    int rtti () const { return RTTI_Map; }

    void enableAntiAliasing(bool flag) { mAntiAliasing = flag; }
    
    QPixmap& pixmap() { return mPixmap; }
    
    //! renders map using QgsMapRender to mPixmap
    void render();
    
    void setBgColor(const QColor& color) { mBgColor = color; }
    
    void setPanningOffset(const QPoint& point) { mOffset = point; }
    
  protected:
    
    //! called by map canvas to draw map
    void drawShape(QPainter & p);

  private:

    //! pixmap that holds rendered map
    QPixmap mPixmap;

    //! indicates whether antialiasing will be used for rendering
    bool mAntiAliasing;
    
    QgsMapRender* mRender;
    
    QColor mBgColor;
    
    QPoint mOffset;
};

#endif
