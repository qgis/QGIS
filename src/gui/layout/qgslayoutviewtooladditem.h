/***************************************************************************
                             qgslayoutviewtooladditem.h
                             --------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTVIEWTOOLADDITEM_H
#define QGSLAYOUTVIEWTOOLADDITEM_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgslayoutviewtool.h"

/**
 * \ingroup gui
 * Layout view tool for adding items to a layout.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutViewToolAddItem : public QgsLayoutViewTool
{

    Q_OBJECT

  public:

    QgsLayoutViewToolAddItem( QgsLayoutView *view );

    /**
     * Returns the item type for items created by the tool.
     * \see setItemType()
     */
    int itemType() const;

    /**
     * Sets the item \a type for items created by the tool.
     * \see itemType()
     */
    void setItemType( int type );

  private:

    int mItemType = 0;
};

#endif // QGSLAYOUTVIEWTOOLADDITEM_H
