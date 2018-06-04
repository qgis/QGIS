/***************************************************************************
                              qgslayoutitemgroup.h
                             ---------------------
    begin                : October 2017
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

#ifndef QGSLAYOUTITEMGROUP_H
#define QGSLAYOUTITEMGROUP_H

#include "qgis_core.h"
#include "qgslayoutitem.h"

/**
 * \ingroup core
 * A container for grouping several QgsLayoutItems.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemGroup: public QgsLayoutItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemGroup, belonging to the specified \a layout.
     */
    explicit QgsLayoutItemGroup( QgsLayout *layout );

    void cleanup() override;

    int type() const override;
    QString displayName() const override;

    /**
     * Returns a new group item for the specified \a layout.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsLayoutItemGroup *create( QgsLayout *layout ) SIP_FACTORY;

    /**
     * Adds an \a item to the group. Ownership of the item
     * is transferred to the group.
    */
    void addItem( QgsLayoutItem *item SIP_TRANSFER );

    /**
     * Removes all items from the group (but does not delete them).
     * Items remain in the scene but are no longer grouped together
     */
    void removeItems();

    /**
     * Returns a list of items contained by the group.
     */
    QList<QgsLayoutItem *> items() const;

    //overridden to also hide grouped items
    void setVisibility( bool visible ) override;

    //overridden to move child items
    void attemptMove( const QgsLayoutPoint &point, bool useReferencePoint = true, bool includesFrame = false, int page = -1 ) override;
    void attemptResize( const QgsLayoutSize &size, bool includesFrame = false ) override;

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget ) override;

    void finalizeRestoreFromXml() override;

  protected:
    void draw( QgsLayoutItemRenderContext &context ) override;
    bool writePropertiesToElement( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &itemElement, const QDomDocument &document, const QgsReadWriteContext &context ) override;

  private:

    void resetBoundingRect();
    void updateBoundingRect( QgsLayoutItem *item );
    void setSceneRect( const QRectF &rectangle );

    QList< QString > mItemUuids;
    QList< QPointer< QgsLayoutItem >> mItems;
    QRectF mBoundingRectangle;
};

#endif //QGSLAYOUTITEMGROUP_H



