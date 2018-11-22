/***************************************************************************
   qgslayoutitemcombobox.h
    --------------------------------------
   Date                 : October 2017
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

#ifndef QGSLAYOUTITEMCOMBOBOX_H
#define QGSLAYOUTITEMCOMBOBOX_H

#include <QComboBox>
#include "qgis.h"
#include "qgslayoutitem.h"
#include "qgslayoutitemregistry.h"
#include "qgis_gui.h"
#include "qgslayoutmodel.h"

/**
 * \class QgsLayoutItemComboBox
 * \ingroup gui
 * \brief The QgsLayoutItemComboBox class is a combo box which displays items of
 * a matching type from a layout.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutItemComboBox : public QComboBox
{
    Q_OBJECT

  public:

    /**
     * QgsLayoutItemComboBox creates a combo box to display a list of items in a
     * \a layout. The items can optionally be filtered by type.
     * If \a layout is not set, no items will be shown
     * until setCurrentLayout() is called
     */
    explicit QgsLayoutItemComboBox( QWidget *parent SIP_TRANSFERTHIS = nullptr, QgsLayout *layout = nullptr );

    /**
     * Sets the \a layout containing the items to list in the combo box.
     */
    void setCurrentLayout( QgsLayout *layout );

    /**
     * Sets a filter for the item type to show in the combo box.
     * \param itemType type of items to show. Set to QgsLayoutItemRegistry::LayoutItem to
     * show all items.
     * \see itemType()
     */
    void setItemType( QgsLayoutItemRegistry::ItemType itemType );

    /**
     * Returns the filter for the item types to show in the combo box.
     * \see setItemType()
     */
    QgsLayoutItemRegistry::ItemType itemType() const;

    /**
     * Sets a list of specific items to exclude from the combo box.
     * \see exceptedItemList()
     */
    void setExceptedItemList( const QList< QgsLayoutItem * > &exceptList );

    /**
     * Returns the list of specific items excluded from the combo box.
     * \see setExceptedItemList()
     */
    QList< QgsLayoutItem * > exceptedItemList() const;

    /**
     * Returns the item currently shown at the specified \a index within the combo box.
     * \see currentItem()
     */
    QgsLayoutItem *item( int index ) const;

    /**
     * Returns the item currently selected in the combo box.
     */
    QgsLayoutItem *currentItem() const;

  public slots:

    /**
     * Sets the currently selected \a item in the combo box.
     */
    void setItem( const QgsLayoutItem *item );

  signals:

    //! Emitted whenever the currently selected item changes
    void itemChanged( QgsLayoutItem *item );

  private slots:
    void indexChanged( int i );
    void rowsChanged();

  private:
    std::unique_ptr< QgsLayoutProxyModel > mProxyModel;

};

#endif // QGSLAYOUTITEMCOMBOBOX_H
