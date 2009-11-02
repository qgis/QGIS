/***************************************************************************
    qgspgquerybuilder.h - Subclassed PostgreSQL query builder
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
#ifndef QGSPGQUERYBUILDER_H
#define QGSPGQUERYBUILDER_H
#include <map>
#include <vector>
extern "C"
{
#include <libpq-fe.h>
}
#include <QStandardItemModel>
#include <QStandardItem>
#include <QModelIndex>
#include "ui_qgspgquerybuilderbase.h"
#include "qgisgui.h"
#include "qgsfield.h"
#include "qgsdatasourceuri.h"
/*!
 * \class QgsPgQueryBuilder
 * \brief Query Builder for PostgreSQL layers.
 *
 * The query builder allows interactive creation of a SQL for limiting the
 * features displayed in a database layer.  The fields in the table are
 * displayed and sample values (or all values) can be viewed to aid in
 * constructing the query. A test function returns the number of features that
 * will be returned.
 *
 */
class QgsPgQueryBuilder : public QDialog, private Ui::QgsPgQueryBuilderBase
{
    Q_OBJECT
  public:
    //! Default constructor - not very useful
    QgsPgQueryBuilder( QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );

    /*! Constructor which also takes the table name and PG connection pointer.
    * This constructor is used when adding layers to the map from a PG database since
    * the query builder can use the same connection as the layer selection dialog.
    * @param tableName Name of the table being queried
    * @param con PostgreSQL connection from the Add PostGIS Layer dialog
    * @param parent Parent widget
    * @param name Name of the widget
    */
    QgsPgQueryBuilder( QString tableName, PGconn *con, QWidget *parent = 0,
                       Qt::WFlags fl = QgisGui::ModalDialogFlags );

    /*! Constructor that uses a data source URI to create its own connection to the
    * PG database. This constructor should be used when a layer's own PG connection
    * cannot. Using the same connection as that of the layer typically causes problems
    * and crashes. This constructor is used when the query builder is called from the
    * vector layer properties dialog
    * @param uri Reference to a fully populates QgsDataSourceURI structure
     * @param parent Parent widget
     * @param name Name of the widget
     */
    QgsPgQueryBuilder( QgsDataSourceURI *uri, QWidget *parent = 0,
                       Qt::WFlags fl = QgisGui::ModalDialogFlags );

    ~QgsPgQueryBuilder();

    /*!
     * Set the connection used the by query builder
     * @param con Active PostgreSQL connection
     */
    void setConnection( PGconn *con );

  public slots:
    void on_btnEqual_clicked();
    void on_btnOk_clicked();
    void on_btnLessThan_clicked();
    void on_btnGreaterThan_clicked();
    void on_btnPct_clicked();
    void on_btnIn_clicked();
    void on_btnNotIn_clicked();
    void on_btnLike_clicked();
    void on_btnILike_clicked();
    QString sql();
    void setSql( QString sqlStatement );
    void on_lstFields_clicked( const QModelIndex &index );
    void on_lstFields_doubleClicked( const QModelIndex &index );
    void on_lstValues_doubleClicked( const QModelIndex &index );
    void on_btnLessEqual_clicked();
    void on_btnGreaterEqual_clicked();
    void on_btnNotEqual_clicked();
    void on_btnAnd_clicked();
    void on_btnNot_clicked();
    void on_btnOr_clicked();
    void on_btnClear_clicked();
    /*! Test the constructed sql statement to see if the database likes it.
     * The number of rows that would be returned is displayed in a message box.
     * The test uses a "select count(*) from ..." query to test the SQL
     * statement.
     * @param showResults If true, the results are displayed in a QMessageBox
     */
    void on_btnTest_clicked();
    /*!
     * Get all distinct values for the field. Values are inserted
     * into the value list box
     */
    void on_btnGetAllValues_clicked();
    /*!
     * Get sample distinct values for the selected field. The sample size is
     * limited to an arbitrary value (currently set to 25). The values
     * are inserted into the values list box.
     */
    void on_btnSampleValues_clicked();
    void setDatasourceDescription( QString uri );
  private:
    /*!
     * Populate the field list for the selected table
     */
    void populateFields();
    /*!
     * Setup models for listviews
     */
    void setupGuiViews();
    void setupLstFieldsModel();
    void fillValues( QString theSQL );

    /*! Get the number of records that would be returned by the current SQL
     * @return Number of records or -1 if an error was encountered
     */
    long countRecords( QString sql );

    // private members
    //! Datasource URI
    QgsDataSourceURI *mUri;
    //! PostgreSQL connection object
    PGconn *mPgConnection;
    //! Table name
    QString mTableName;
    //! Vector of QgsField objects
    std::vector<QgsField> mFields;
    //! Map that holds field information, keyed by field name
    std::map<QString, QgsField> mFieldMap;
    //! Latest PG error message
    QString mPgErrorMessage;
    //! Flag to indicate if the class owns the connection to the pg database
    bool mOwnConnection;
    //! Model for fields ListView
    QStandardItemModel *mModelFields;
    //! Model for values ListView
    QStandardItemModel *mModelValues;
    //! Previous field row to delete model
    int mPreviousFieldRow;
};
#endif //QGSPGQUERYBUILDER_H
