/***************************************************************************
    qgstablewidgetbase.h
     --------------------------------------
    Date                 : 09.2016
    Copyright            : (C) 2016 Patrick Valsecchi
    Email                : patrick.valsecchi@camptocamp.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTABLEWIDGETBASE_H
#define QGSTABLEWIDGETBASE_H

#include "ui_qgstablewidgetuibase.h"
#include <QAbstractTableModel>
#include <QVariant>
#include "qgis_gui.h"

/**
 * \ingroup gui
 * Base widget allowing to edit a collection, using a table.
 *
 * This widget includes buttons to add and remove rows.
 * Child classes must call init(QAbstractTableModel*) from their constructor.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsTableWidgetBase: public QWidget, protected Ui::QgsTableWidgetUiBase
{
    Q_OBJECT
  public:

    /**
     * Constructor.
     */
    explicit QgsTableWidgetBase( QWidget *parent );

  protected:

    /**
     * Initialize the table with the given model.
     * Must be called once in the child class' constructor.
     */
    void init( QAbstractTableModel *model );

  signals:

    /**
     * Emitted each time a key or a value is changed.
     */
    void valueChanged();

  private slots:

    /**
     * Called when the add button is clicked.
     */
    void addButton_clicked();

    /**
     * Called when the remove button is clicked.
     */
    void removeButton_clicked();

    /**
     * Called when the selection is changed to enable/disable the delete button.
     */
    void onSelectionChanged();

    friend class TestQgsKeyValueWidget;
    friend class TestQgsListWidget;

};


#endif // QGSTABLEWIDGETBASE_H
