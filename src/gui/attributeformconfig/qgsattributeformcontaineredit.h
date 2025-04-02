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

#include "qgsattributesformmodel.h"
#include "qgis_gui.h"
#include "ui_qgsattributeformcontaineredit.h"

#include <QWidget>


class QTreeWidgetItem;

/**
 * Widget to edit a container (tab or group box) of a form configuration
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsAttributeFormContainerEdit : public QWidget, private Ui_QgsAttributeFormContainerEdit
{
    Q_OBJECT

  public:
    explicit QgsAttributeFormContainerEdit( const QgsAttributeFormTreeData::DnDTreeItemData &itemData, QgsVectorLayer *layer, QWidget *parent = nullptr );


    void setTitle( const QString &containerName );

    /**
     * Sets up the container type comboBox
     * \param isTopLevelContainer Whether the container is allowed to be a tab or not
     *
     * \since QGIS 3.44
     */
    void setUpContainerTypeComboBox( bool isTopLevelContainer, const Qgis::AttributeEditorContainerType containerType );

    /**
     * Register an expression context generator class that will be used to retrieve
     * an expression context for the widget when required.
     * \since QGIS 3.14
     */
    void registerExpressionContextGenerator( QgsExpressionContextGenerator *generator );

    void updateItemData( QgsAttributeFormTreeData::DnDTreeItemData &itemData, QString &containerName );

  private slots:

    void containerTypeChanged();
};

#endif // QGSATTRIBUTEFORMCONTAINEREDIT_H
