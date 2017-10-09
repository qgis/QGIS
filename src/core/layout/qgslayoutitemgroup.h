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

    explicit QgsLayoutItemGroup( QgsLayout *layout );
    ~QgsLayoutItemGroup();

    int type() const override;
    QString stringType() const override;

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
    virtual void setVisibility( const bool visible ) override;

  protected:

    void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = nullptr ) override;

  private:

    void updateBoundingRect();

    QList< QPointer< QgsLayoutItem >> mItems;
    QRectF mBoundingRectangle;
};

#endif //QGSLAYOUTITEMGROUP_H



