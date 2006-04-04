/***************************************************************************
    qgsmapcanvasitem.h  - base class for map canvas items
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

#ifndef QGSMAPCANVASITEM_H
#define QGSMAPCANVASITEM_H

#include <Q3CanvasItem>
#include "qgsrect.h"

class QgsMapCanvas;
class QPainter;

class QgsMapCanvasItem : public QObject, public Q3CanvasRectangle
{
  Q_OBJECT;
  
  protected:
    
    //! protected constructor: cannot be constructed directly
    QgsMapCanvasItem(QgsMapCanvas* mapCanvas);
    
    virtual ~QgsMapCanvasItem();
    
    //! function to be implemented by derived classes
    virtual void drawShape(QPainter & p)=0;
    
    //! schedules map canvas for repaint
    void updateCanvas();
    
  
  public:
    
    //! sets current offset, to be called from QgsMapCanvas
    void setPanningOffset(const QPoint& point);
    
    //! returns canvas item rectangle
    QgsRect rect();
    
    //! sets canvas item rectangle
    void setRect(const QgsRect& r);
    
    //! transformation from screen coordinates to map coordinates
    QgsPoint toMapCoords(const QPoint& point);
    
    //! transformation from map coordinates to screen coordinates
    QPoint toCanvasCoords(const QgsPoint& point);

    /** called on changed extents or changed item rectangle
     * Override this in your subclass if you wish to have custom
     * behaviour for when the canvas area of interest is changed */
    virtual void updatePosition();

  protected:
    
    //! pointer to map canvas
    QgsMapCanvas* mMapCanvas;
    
    //! canvas item rectangle (in map coordinates)
    QgsRect mRect;

    //! offset from normal position due current panning operation,
    //! used when converting map coordinates to move map canvas items
    QPoint mPanningOffset;
};


#endif
