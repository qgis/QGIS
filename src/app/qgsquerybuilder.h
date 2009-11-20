/***************************************************************************
    qgsquerybuilder.h - Subclassed PostgreSQL query builder
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
#include <QStandardItemModel>
#include <QStandardItem>
#include <QModelIndex>
#include "ui_qgsquerybuilderbase.h"
#include "qgisgui.h"
#include "qgsfield.h"
#include "qgscontexthelp.h"

class QgsVectorLayer;

/*!
 * \class QgsQueryBuilder
 * \brief Query Builder for PostgreSQL layers.
 *
 * The query builder allows interactive creation of a SQL for limiting the
 * features displayed in a database layer.  The fields in the table are
 * displayed and sample values (or all values) can be viewed to aid in
 * constructing the query. A test function returns the number of features that
 * will be returned.
 *
 */
class QgsQueryBuilder : public QDialog, private Ui::QgsQueryBuilderBase
{
    Q_OBJECT
  public:
    //! Default constructor - not very useful
    QgsQueryBuilder( QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );

    /*! Constructor which also takes the table name and PG connection pointer.
    * This constructor is used when adding layers to the map from a PG database since
    * the query builder can use the same connection as the layer selection dialog.
    * @param tableName Name of the table being queried
    * @param con PostgreSQL connection from the Add PostGIS Layer dialog
    * @param parent Parent widget
    * @param name Name of the widget
    */
    QgsQueryBuilder( QString tableName, QWidget *parent = 0,
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
    QgsQueryBuilder( QgsVectorLayer *layer, QWidget *parent = 0,
                     Qt::WFlags fl = QgisGui::ModalDialogFlags );

    ~QgsQueryBuilder();

  public slots:
    void accept();
    void reject();
    void helpClicked();
    void on_btnClear_clicked();
    void on_btnEqual_clicked();
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

    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }


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
    void fillValues( int idx, QString subsetString, int limit );

    // private members
    //! Model for fields ListView
    QStandardItemModel *mModelFields;
    //! Model for values ListView
    QStandardItemModel *mModelValues;
    //! Previous field row to delete model
    int mPreviousFieldRow;

    //! vector layer
    QgsVectorLayer *mLayer;

    //! original subset string
    QString mOrigSubsetString;
};
#endif //QGSPGQUERYBUILDER_H
