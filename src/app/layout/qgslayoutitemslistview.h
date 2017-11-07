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

#include "qgis.h"
#include <QTreeView>

class QgsLayout;
class QgsLayoutDesignerDialog;
class QgsLayoutModel;

/**
 * A list view for showing items in a layout
 */
class QgsLayoutItemsListView : public QTreeView
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemsListView.
     */
    QgsLayoutItemsListView( QWidget *parent, QgsLayoutDesignerDialog *designer );

    void setCurrentLayout( QgsLayout *layout );

  private slots:

    void showContextMenu( QPoint point );

  private:

    QgsLayout *mLayout = nullptr;
    QgsLayoutModel *mModel = nullptr;
    QgsLayoutDesignerDialog *mDesigner = nullptr;
};

#endif // QGSLAYOUTITEMSLISTVIEW_H
