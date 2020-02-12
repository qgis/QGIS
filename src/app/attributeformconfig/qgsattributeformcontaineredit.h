/***************************************************************************
    qgsattributeformcontaineredit.h
    ---------------------
    begin                : February 2020
    copyright            : (C) 2020 Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTEFORMCONTAINEREDIT_H
#define QGSATTRIBUTEFORMCONTAINEREDIT_H

#include <QWidget>

#include "ui_qgsattributeformcontaineredit.h"

#include "qgis_app.h"

class QTreeWidgetItem;

/**
 * Widget to edit a container (tab or group box) of a form configuration
 */
class APP_EXPORT QgsAttributeFormContainerEdit: public QWidget, private Ui_QgsAttributeFormContainerEdit
{
    Q_OBJECT

  public:
    explicit QgsAttributeFormContainerEdit( QTreeWidgetItem *item, QWidget *parent = nullptr );


    void updateItemData();


  private:
    QTreeWidgetItem *mTreeItem;
};

#endif // QGSATTRIBUTEFORMCONTAINEREDIT_H
