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

#include "ui_qgstablewidgetbase.h"
#include <QAbstractTableModel>
#include <QVariant>

/** \ingroup gui
 * Base widget allowing to edit a collection, using a table.
 *
 * This widget includes buttons to add and remove rows.
 * Child classes must call init(QAbstractTableModel*) from their constructor.
 *
 * @note added in QGIS 3.0
 */
class GUI_EXPORT QgsTableWidgetBase: public QWidget, public Ui::QgsTableWidgetBase
{
    Q_OBJECT
  public:
    /**
     * Constructor.
     */
    explicit QgsTableWidgetBase( QWidget* parent );

  protected:
    /**
     * Initialise the table with the given model.
     * Must be called once in the child class' constructor.
     */
    void init( QAbstractTableModel* model );

  signals:
    /**
     * Emitted each time a key or a value is changed.
     */
    void valueChanged();

  private slots:
    /**
     * Called when the add button is clicked.
     */
    void on_addButton_clicked();

    /**
     * Called when the remove button is clicked.
     */
    void on_removeButton_clicked();

    /**
     * Called when the selection is changed to enable/disable the delete button.
     */
    void onSelectionChanged();
};


#endif // QGSTABLEWIDGETBASE_H