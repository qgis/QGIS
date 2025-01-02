/***************************************************************************
                             qgsprocessingmatrixmodelerwidget.h
                             ----------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGMATRIXMODELERWIDGET_H
#define QGSPROCESSINGMATRIXMODELERWIDGET_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgis_gui.h"
#include "ui_qgsprocessingmatrixmodelerwidgetbase.h"
#include <QStandardItem>
#include <QStandardItemModel>

///@cond PRIVATE

/**
 * Processing matrix widget for configuring matrix parameter in modeler.
 * \ingroup gui
 * \note Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsProcessingMatrixModelerWidget : public QWidget, private Ui::QgsProcessingMatrixModelerWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingMatrixModelerWidget.
     */
    QgsProcessingMatrixModelerWidget( QWidget *parent = nullptr );

    /**
    * Returns list of matrix headers.
    */
    QStringList headers() const;

    /**
    * Returns matrix defined by user.
    *
    * \see setValue()
    */
    QVariant value() const;

    /**
    * Sets value of the widget.
    *
    * \see value()
    */
    void setValue( const QStringList &headers, const QVariant &defaultValue );

    /**
     * Returns TRUE if the parameter has fixed number of rows.
     * \see setFixedRows()
     */
    bool fixedRows() const;

    /**
     * Sets whether the parameter has fixed number of rows.
     * \see fixedRows()
     */
    void setFixedRows( bool fixedRows );

  private slots:

    void addColumn();
    void removeColumns();
    void addRow();
    void removeRows();
    void clearTable();
    void changeHeader( int index );

  private:
    QStandardItemModel *mModel = nullptr;

    friend class TestProcessingGui;
};

///@endcond

#endif // QGSPROCESSINGMATRIXMODELERWIDGET_H
