/***************************************************************************
                             qgslayoutitemslistview.h
                             ------------------------
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

#ifndef QGSLAYOUTITEMSLISTVIEW_H
#define QGSLAYOUTITEMSLISTVIEW_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QTreeView>
#include <QSortFilterProxyModel>

class QgsLayout;
class QgsLayoutDesignerInterface;
class QgsLayoutModel;
class QgsLayoutItem;

/**
 * \ingroup gui
 * \brief Model for the layout items list view.
 * \see QgsLayoutItemsListView
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutItemsListViewModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    //! constructor
    QgsLayoutItemsListViewModel( QgsLayoutModel *model, QObject *parent );

    //! Returns the layout item listed at the specified index
    QgsLayoutItem *itemFromIndex( const QModelIndex &index ) const;
    //! Returns the model index matching the specified layout item
    QModelIndex indexForItem( QgsLayoutItem *item, const int column = 0 ) const;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;

  public slots:
    //! Sets the selected index
    void setSelected( const QModelIndex &index );

  protected:
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

  private:
    QgsLayoutModel *mModel = nullptr;
};

/**
 * \ingroup gui
 * \brief A list view for showing items in a layout
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutItemsListView : public QTreeView
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsLayoutItemsListView.
     */
    QgsLayoutItemsListView( QWidget *parent, QgsLayoutDesignerInterface *designer );

    //! Sets the current layout
    void setCurrentLayout( QgsLayout *layout );

  protected:
    void keyPressEvent( QKeyEvent *event ) override;

  private slots:

    void showContextMenu( QPoint point );

    //! Update LayoutView selection from the item list
    void updateSelection();
    //! Update item list selected from the layout view
    void onItemFocused( QgsLayoutItem *focusedItem );

  private:
    QgsLayout *mLayout = nullptr;
    QgsLayoutItemsListViewModel *mModel = nullptr;
    QgsLayoutDesignerInterface *mDesigner = nullptr;

    bool mUpdatingSelection = false;
    bool mUpdatingFromView = false;
};

#endif // QGSLAYOUTITEMSLISTVIEW_H
