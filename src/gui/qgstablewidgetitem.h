/***************************************************************************
  qgstablewidgetitem.h - QgsTableWidgetItem

 ---------------------
 begin                : 27.3.2016
 copyright            : (C) 2016 by Matthias Kuhn, OPENGIS.ch
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSTABLEWIDGETITEM_H
#define QGSTABLEWIDGETITEM_H

#include <QTableWidget>
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \brief This can be used like a regular QTableWidgetItem with the difference that a
 * specific role can be set to sort.
 */
class GUI_EXPORT QgsTableWidgetItem : public QTableWidgetItem
{
  public:

    QgsTableWidgetItem() = default;

    /**
     * Creates a new table widget item with the specified text.
     */
    QgsTableWidgetItem( const QString &text );


    /**
     * Set the role by which the items should be sorted.
     * By default this will be set to Qt::DisplayRole
     */
    void setSortRole( int role );

    /**
     * Gets the role by which the items should be sorted.
     * By default this will be Qt::DisplayRole
     */
    int sortRole() const;

    bool operator <( const QTableWidgetItem &other ) const override;

  private:
    int mSortRole = Qt::DisplayRole;
};

#endif // QGSTABLEWIDGETITEM_H
