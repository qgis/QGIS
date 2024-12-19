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
 * \brief Base widget allowing to edit a collection, using a table.
 *
 * This widget includes buttons to add and remove rows.
 * Child classes must call init(QAbstractTableModel*) from their constructor.
 *
 */
class GUI_EXPORT QgsTableWidgetBase: public QWidget, protected Ui::QgsTableWidgetUiBase
{
    Q_OBJECT
  public:

    /**
     * Constructor.
     */
    explicit QgsTableWidgetBase( QWidget *parent );

    /**
     * Returns TRUE if the widget is shown in a read-only state.
     *
     * \see setReadOnly()
     * \since QGIS 3.38
     */
    bool isReadOnly() const { return mReadOnly; }

  public slots:

    /**
    * Sets whether the widget should be shown in a read-only state.
    *
    * \see isReadOnly()
    * \since QGIS 3.38
    */
    virtual void setReadOnly( bool readOnly );

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

  private:

    bool mReadOnly = false;

    friend class TestQgsKeyValueWidget;
    friend class TestQgsListWidget;

};


#endif // QGSTABLEWIDGETBASE_H
