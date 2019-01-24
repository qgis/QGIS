/***************************************************************************
                             qgstreewidgetitem.h
                             -------------------
    begin                : 06 Nov, 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSTREEWIDGETITEM_H
#define QGSTREEWIDGETITEM_H

#include <QTreeWidgetItem>
#include "qgis_sip.h"
#include <QObject>
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \class QgsTreeWidgetItem
 * QTreeWidgetItem subclass with custom handling for item sorting.
 *
 * QgsTreeWidgetItem allows for items to be sorted using a specified user role, and
 * also correctly handles sorting numeric or mixed text and numeric values.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsTreeWidgetItem : public QTreeWidgetItem
{
  public:

    /**
     * Constructor for QgsTreeWidgetItem
     * \param view parent QTreeWidget view
     * \param type item type
     */
    explicit QgsTreeWidgetItem( QTreeWidget *view SIP_TRANSFERTHIS, int type = Type );

    /**
     * Constructor for QgsTreeWidgetItem
     * \param type item type
     */
    explicit QgsTreeWidgetItem( int type = Type );

    /**
     * Constructor for QgsTreeWidgetItem
     * \param strings list of strings containing text for each column in the item
     * \param type item type
     */
    QgsTreeWidgetItem( const QStringList &strings, int type = Type );

    /**
     * Constructor for QgsTreeWidgetItem
     * \param view parent QTreeWidget view
     * \param strings list of strings containing text for each column in the item
     * \param type item type
     */
    QgsTreeWidgetItem( QTreeWidget *view SIP_TRANSFERTHIS, const QStringList &strings, int type = Type );

    /**
     * Constructor for QgsTreeWidgetItem
     * \param view parent QTreeWidget view
     * \param after QTreeWidgetItem to place insert item after in the view
     * \param type item type
     */
    QgsTreeWidgetItem( QTreeWidget *view SIP_TRANSFERTHIS, QTreeWidgetItem *after, int type = Type );

    /**
     * Constructor for QgsTreeWidgetItem
     * \param parent QTreeWidgetItem item
     * \param type item type
     */
    explicit QgsTreeWidgetItem( QTreeWidgetItem *parent SIP_TRANSFERTHIS, int type = Type );

    /**
     * Constructor for QgsTreeWidgetItem
     * \param parent QTreeWidgetItem item
     * \param strings list of strings containing text for each column in the item
     * \param type item type
     */
    QgsTreeWidgetItem( QTreeWidgetItem *parent SIP_TRANSFERTHIS, const QStringList &strings, int type = Type );

    /**
     * Constructor for QgsTreeWidgetItem
     * \param parent QTreeWidgetItem item
     * \param after QTreeWidgetItem to place insert item after in the view
     * \param type item type
     */
    QgsTreeWidgetItem( QTreeWidgetItem *parent SIP_TRANSFERTHIS, QTreeWidgetItem *after, int type = Type );

    /**
     * Sets the custom sort data for a specified column. If set, this value will be used when
     * sorting the item instead of the item's display text. If not set, the item's display
     * text will be used when sorting.
     * \param column column index
     * \param value sort value
     * \see sortData()
     */
    void setSortData( int column, const QVariant &value );

    /**
     * Returns the custom sort data for a specified column. If set, this value will be used when
     * sorting the item instead of the item's display text. If not set, the item's display
     * text will be used when sorting.
     * \see setSortData()
     */
    QVariant sortData( int column ) const;

    /**
     * Sets a the item to display always on top of other items in the widget, regardless of the
     * sort column and sort or display value for the item.
     * \param priority priority for sorting always on top items. Items with a lower priority will
     * be placed above items with a higher priority.
     * \see alwaysOnTopPriority()
     */
    void setAlwaysOnTopPriority( int priority );

    /**
     * Returns the item's priority when it is set to show always on top. Items with a lower priority will
     * be placed above items with a higher priority.
     * \returns priority, or -1 if item is not set to show always on top
     * \see setAlwaysOnTopPriority()
     */
    int alwaysOnTopPriority() const;

    /**
     * Returns true if this item should appear before another item when sorting
     * a list of items.
     */
    bool operator<( const QTreeWidgetItem &other ) const override;

    /**
     * Returns true if this item should appear after another item when sorting
     * a list of items.
     */
    bool operator>=( const QTreeWidgetItem &other ) const { return !( *this < other ); }

  private:

    enum ItemDataRole
    {
      CustomSortRole = Qt::UserRole + 1001,
      AlwaysOnTopPriorityRole = Qt::UserRole + 1002,
    };

};

/**
 * \ingroup gui
 * \class QgsTreeWidgetItemObject
 * Custom QgsTreeWidgetItem with extra signals when item is edited.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsTreeWidgetItemObject: public QObject, public QgsTreeWidgetItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsTreeWidgetItemObject
     * \param type item type
     */
    explicit QgsTreeWidgetItemObject( int type = Type );

    //! Constructs a tree widget item of the specified type and appends it to the items in the given parent.
    explicit QgsTreeWidgetItemObject( QTreeWidget *parent SIP_TRANSFERTHIS, int type = Type );

    //! Sets the value for the item's column and role to the given value.
    void setData( int column, int role, const QVariant &value ) override;

  signals:
    //! This signal is emitted when the contents of the column in the specified item has been edited by the user.
    void itemEdited( QTreeWidgetItem *item, int column );
};

#endif // QGSTREEWIDGETITEM_H
