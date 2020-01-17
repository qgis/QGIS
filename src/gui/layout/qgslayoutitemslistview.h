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

class GUI_EXPORT QgsLayoutItemsListViewModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:

    QgsLayoutItemsListViewModel( QgsLayoutModel *model, QObject *parent );

    QgsLayoutItem *itemFromIndex( const QModelIndex &index ) const;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;

  public slots:
    void setSelected( const QModelIndex &index );

  protected:

    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

  private:

    QgsLayoutModel *mModel = nullptr;
};

/**
 * A list view for showing items in a layout
 */
class GUI_EXPORT QgsLayoutItemsListView : public QTreeView
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemsListView.
     */
    QgsLayoutItemsListView( QWidget *parent, QgsLayoutDesignerInterface *designer );

    void setCurrentLayout( QgsLayout *layout );

  private slots:

    void showContextMenu( QPoint point );

  private:

    QgsLayout *mLayout = nullptr;
    QgsLayoutItemsListViewModel *mModel = nullptr;
    QgsLayoutDesignerInterface *mDesigner = nullptr;
};

#endif // QGSLAYOUTITEMSLISTVIEW_H
