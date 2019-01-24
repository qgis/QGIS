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

#ifndef QGSMAPCANVASITEM_H
#define QGSMAPCANVASITEM_H

#include <QGraphicsItem>
#include "qgis_sip.h"
#include "qgsrectangle.h"
#include "qgis_gui.h"

class QgsMapCanvas;
class QgsRenderContext;
class QPainter;

/**
 * \ingroup gui
 * An abstract class for items that can be placed on the
 * map canvas.
 */
class GUI_EXPORT QgsMapCanvasItem : public QGraphicsItem
{
  protected:

    //! protected constructor: cannot be constructed directly
    QgsMapCanvasItem( QgsMapCanvas *mapCanvas SIP_TRANSFERTHIS );

    ~QgsMapCanvasItem() override;

    //! function to be implemented by derived classes
    virtual void paint( QPainter *painter ) = 0;

    void paint( QPainter *painter,
                const QStyleOptionGraphicsItem *option,
                QWidget *widget = nullptr ) override;

    //! schedules map canvas for repaint
    void updateCanvas();

    /**
     * Sets render context parameters
    \param p painter for rendering
    \param context out: configured context
    \returns true in case of success */
    bool setRenderContextVariables( QPainter *p, QgsRenderContext &context ) const;

  public:

    //! called on changed extent or resize event to update position of the item
    virtual void updatePosition();

    QRectF boundingRect() const override;

    //! returns canvas item rectangle in map units
    QgsRectangle rect() const;

    //! sets canvas item rectangle in map units
    void setRect( const QgsRectangle &r, bool resetRotation = true );

    //! transformation from screen coordinates to map coordinates
    QgsPointXY toMapCoordinates( QPoint point ) const;

    //! transformation from map coordinates to screen coordinates
    QPointF toCanvasCoordinates( const QgsPointXY &point ) const;

  protected:

    //! pointer to map canvas
    QgsMapCanvas *mMapCanvas = nullptr;

    /**
     * cached canvas item rectangle in map coordinates
     * encodes position (xmin,ymax) and size (width/height)
     * used to re-position and re-size the item on zoom/pan
     * while waiting for the renderer to complete.
     *
     * NOTE: does not include rotation information, so cannot
     * be used to correctly present pre-rendered map
     * on rotation change
     */
    QgsRectangle mRect;

    double mRectRotation;

    //! cached size of the item (to return in boundingRect())
    QSizeF mItemSize;

};


#endif
