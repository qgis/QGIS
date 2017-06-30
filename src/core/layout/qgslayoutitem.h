/***************************************************************************
                              qgslayoutitem.h
                             -------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTITEM_H
#define QGSLAYOUTITEM_H

#include "qgis_core.h"
#include "qgslayoutobject.h"
#include "qgslayoutitemregistry.h"
#include <QGraphicsRectItem>

class QgsLayout;
class QPainter;

/**
 * \ingroup core
 * \class QgsLayoutItem
 * \brief Base class for graphical items within a QgsLayout.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItem : public QgsLayoutObject, public QGraphicsRectItem
{

  public:

    QgsLayoutItem( QgsLayout *layout );

    //! Return correct graphics item type.
    virtual int type() const = 0;

    /**
     * Handles preparing a paint surface for the layout item and painting the item's
     * content. Derived classes must not override this method, but instead implement
     * the pure virtual method QgsLayoutItem::draw.
     */
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget ) override;

  protected:

    /** Draws a debugging rectangle of the item's current bounds within the specified
     * painter.
     * @param painter destination QPainter
     */
    virtual void drawDebugRect( QPainter *painter );

  private:

    /**
     * Draws the item's contents on a specified \a painter.
     */
    virtual void draw( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget ) = 0;

    //! Prepares a painter by setting rendering flags
    void preparePainter( QPainter *painter );

    friend class TestQgsLayoutItem;
};

#endif //QGSLAYOUTITEM_H



