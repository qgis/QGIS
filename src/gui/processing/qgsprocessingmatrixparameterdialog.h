/***************************************************************************
                             qgsprocessingmatrixparameterdialog.h
                             ----------------------------------
    Date                 : February 2019
    Copyright            : (C) 2019 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGMATRIXPARAMETERDIALOG_H
#define QGSPROCESSINGMATRIXPARAMETERDIALOG_H

#include "qgis.h"
#include "qgis_gui.h"
#include "ui_qgsprocessingmatrixparameterdialogbase.h"
#include "qgsprocessingparameters.h"

#define SIP_NO_FILE

class QStandardItemModel;
class QToolButton;

///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief Panel widget for configuration of a matrix (fixed table) parameter.
 * \note Not stable API
 * \since QGIS 3.6
 */
class GUI_EXPORT QgsProcessingMatrixParameterPanelWidget : public QgsPanelWidget, private Ui::QgsProcessingMatrixParameterDialogBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingMatrixParameterDialog.
     */
    QgsProcessingMatrixParameterPanelWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr, const QgsProcessingParameterMatrix *param = nullptr,
        const QVariantList &initialTable = QVariantList() );

    /**
     * Returns the table's contents as a 1 dimensional array.
     */
    QVariantList table() const;

  private slots:

    void addRow();
    void deleteRow();
    void deleteAllRows();

  private:

    QPushButton *mButtonAdd = nullptr;
    QPushButton *mButtonRemove = nullptr;
    QPushButton *mButtonRemoveAll = nullptr;
    const QgsProcessingParameterMatrix *mParam = nullptr;
    QStandardItemModel *mModel = nullptr;
    bool mWasCanceled = false;

    void populateTable( const QVariantList &contents );

    friend class TestProcessingGui;
};


/**
 * \ingroup gui
 * \brief Widget for configuration of a matrix (fixed table) parameter.
 * \note Not stable API
 * \since QGIS 3.6
 */
class GUI_EXPORT QgsProcessingMatrixParameterPanel : public QWidget
{
    Q_OBJECT

  public:

    QgsProcessingMatrixParameterPanel( QWidget *parent = nullptr, const QgsProcessingParameterMatrix *param = nullptr );

    QVariantList value() const { return mTable; }

    void setValue( const QVariantList &value );

  signals:

    void changed();

  private slots:

    void showDialog();

  private:

    void updateSummaryText();

    const QgsProcessingParameterMatrix *mParam = nullptr;
    QLineEdit *mLineEdit = nullptr;
    QToolButton *mToolButton = nullptr;

    QVariantList mTable;

    friend class TestProcessingGui;
};

///@endcond

#endif // QGSPROCESSINGMATRIXPARAMETERDIALOG_H
