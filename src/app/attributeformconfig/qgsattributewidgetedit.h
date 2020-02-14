/***************************************************************************
    qgsattributewidgetedit.h
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

#ifndef QGSATTRIBUTEWIDGETEDIT_H
#define QGSATTRIBUTEWIDGETEDIT_H

#include <QWidget>

#include "ui_qgsattributewidgeteditgroupbox.h"

#include "qgis_app.h"

class QTreeWidgetItem;

/**
 * Widget to edit a container (tab or group box) of a form configuration
 */
class APP_EXPORT QgsAttributeWidgetEdit: public QgsCollapsibleGroupBox, private Ui_QgsAttributeWidgetEditGroupBox
{
    Q_OBJECT

  public:
    explicit QgsAttributeWidgetEdit( QTreeWidgetItem *item, QWidget *parent = nullptr );


    void updateItemData();


  private:
    QTreeWidgetItem *mTreeItem;
};

#endif // QGSATTRIBUTEWIDGETEDIT_H
