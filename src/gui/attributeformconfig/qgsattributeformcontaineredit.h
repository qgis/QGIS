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

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include <QWidget>

#include "ui_qgsattributeformcontaineredit.h"

#include "qgis_gui.h"

class QTreeWidgetItem;

/**
 * Widget to edit a container (tab or group box) of a form configuration
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsAttributeFormContainerEdit : public QWidget, private Ui_QgsAttributeFormContainerEdit
{
    Q_OBJECT

  public:
    explicit QgsAttributeFormContainerEdit( QTreeWidgetItem *item, QgsVectorLayer *layer, QWidget *parent = nullptr );

    /**
     * Register an expression context generator class that will be used to retrieve
     * an expression context for the widget when required.
     * \since QGIS 3.14
     */
    void registerExpressionContextGenerator( QgsExpressionContextGenerator *generator );

    void updateItemData();

  private slots:

    void containerTypeChanged();

  private:
    QTreeWidgetItem *mTreeItem = nullptr;
};

#endif // QGSATTRIBUTEFORMCONTAINEREDIT_H
