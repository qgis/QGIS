/***************************************************************************
    qgsquerybuilder.h - query builder
     --------------------------------------
    Date                 : 2004-11-19
    Copyright            : (C) 2004 by Gary E.Sherman
    Email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSQUERYBUILDER_H
#define QGSQUERYBUILDER_H
#include <map>
#include "qgis_sip.h"
#include <vector>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QModelIndex>
#include "ui_qgsquerybuilderbase.h"
#include "qgsguiutils.h"
#include "qgshelp.h"
#include "qgis_gui.h"

class QgsVectorLayer;

/**
 * \ingroup gui
 * \class QgsQueryBuilder
 * \brief Query Builder for layers.
 *
 * The query builder allows interactive creation of a SQL for limiting the
 * features displayed in a vector layer.  The fields in the table are
 * displayed and sample values (or all values) can be viewed to aid in
 * constructing the query. A test function returns the number of features that
 * will be returned.
 *
 */
class GUI_EXPORT QgsQueryBuilder : public QDialog, private Ui::QgsQueryBuilderBase
{
    Q_OBJECT
  public:

    /**
     * This constructor is used when the query builder is called from the
     * vector layer properties dialog
     * \param layer existing vector layer
     * \param parent Parent widget
     * \param fl dialog flags
     */
    QgsQueryBuilder( QgsVectorLayer *layer, QWidget *parent SIP_TRANSFERTHIS = nullptr,
                     Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    ~QgsQueryBuilder() override;

    void showEvent( QShowEvent *event ) override;

    QString sql();
    void setSql( const QString &sqlStatement );

  public slots:
    void accept() override;
    void reject() override;
    void clear();

    /**
     * Test the constructed sql statement to see if the vector layer data provider likes it.
     * The number of rows that would be returned is displayed in a message box.
     * The test uses a "select count(*) from ..." query to test the SQL
     * statement.
     */
    void test();

    void setDatasourceDescription( const QString &uri );

  private slots:
    void btnEqual_clicked();
    void btnLessThan_clicked();
    void btnGreaterThan_clicked();
    void btnPct_clicked();
    void btnIn_clicked();
    void btnNotIn_clicked();
    void btnLike_clicked();
    void btnILike_clicked();
    void lstFields_clicked( const QModelIndex &index );
    void lstFields_doubleClicked( const QModelIndex &index );
    void lstValues_doubleClicked( const QModelIndex &index );
    void btnLessEqual_clicked();
    void btnGreaterEqual_clicked();
    void btnNotEqual_clicked();
    void btnAnd_clicked();
    void btnNot_clicked();
    void btnOr_clicked();
    void onTextChanged( const QString &text );

    /**
     * Gets all distinct values for the field. Values are inserted
     * into the value list box
     */
    void btnGetAllValues_clicked();

    /**
     * Gets sample distinct values for the selected field. The sample size is
     * limited to an arbitrary value (currently set to 25). The values
     * are inserted into the values list box.
     */
    void btnSampleValues_clicked();

  private:

    /**
     * Populate the field list for the selected table
     */
    void populateFields();

    void showHelp();

    /**
     * Setup models for listviews
     */
    void setupGuiViews();
    void setupLstFieldsModel();
    void fillValues( int idx, int limit );

    // private members
    //! Model for fields ListView
    QStandardItemModel *mModelFields = nullptr;
    //! Model for values ListView
    QStandardItemModel *mModelValues = nullptr;
    //! Filter proxy Model for values ListView
    QSortFilterProxyModel *mProxyValues = nullptr;
    //! Previous field row to delete model
    int mPreviousFieldRow;

    //! vector layer
    QgsVectorLayer *mLayer = nullptr;

    //! original subset string
    QString mOrigSubsetString;
};
#endif //QGSQUERYBUILDER_H
