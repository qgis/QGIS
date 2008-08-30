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

#include <QGraphicsItem>
#include "qgsrect.h"

class QgsMapCanvas;
class QPainter;

/** \ingroup gui
 * An abstract class for items that can be placed on the 
 * map canvas.
 */
class GUI_EXPORT QgsMapCanvasItem : public QGraphicsItem
{
  protected:

    //! protected constructor: cannot be constructed directly
    QgsMapCanvasItem( QgsMapCanvas* mapCanvas );

    virtual ~QgsMapCanvasItem();

    //! function to be implemented by derived classes
    virtual void paint( QPainter * painter ) = 0;

    //! paint function called by map canvas
    virtual void paint( QPainter * painter,
                        const QStyleOptionGraphicsItem * option,
                        QWidget * widget = 0 );

    //! schedules map canvas for repaint
    void updateCanvas();


  public:

    //! called on changed extent or resize event to update position of the item
    virtual void updatePosition();

    //! default implementation for canvas items
    virtual QRectF boundingRect() const;

    //! sets current offset, to be called from QgsMapCanvas
    void setPanningOffset( const QPoint& point );

    //! returns canvas item rectangle
    QgsRect rect() const;

    //! sets canvas item rectangle
    void setRect( const QgsRect& r );

    //! transformation from screen coordinates to map coordinates
    QgsPoint toMapCoords( const QPoint& point );

    //! transformation from map coordinates to screen coordinates
    QPointF toCanvasCoords( const QgsPoint& point );

  protected:

    //! pointer to map canvas
    QgsMapCanvas* mMapCanvas;

    //! canvas item rectangle (in map coordinates)
    QgsRect mRect;

    //! offset from normal position due current panning operation,
    //! used when converting map coordinates to move map canvas items
    QPoint mPanningOffset;

    //! cached size of the item (to return in boundingRect())
    QSizeF mItemSize;
};


#endif
