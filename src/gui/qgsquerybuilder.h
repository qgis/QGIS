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
#include <vector>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QModelIndex>
#include "ui_qgsquerybuilderbase.h"
#include "qgisgui.h"
#include "qgsfield.h"
#include "qgscontexthelp.h"

class QgsVectorLayer;

/** \ingroup gui
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
    /** This constructor is used when the query builder is called from the
     * vector layer properties dialog
     * @param layer existing vector layer
     * @param parent Parent widget
     * @param fl dialog flags
     */
    QgsQueryBuilder( QgsVectorLayer *layer, QWidget *parent = nullptr,
                     const Qt::WindowFlags& fl = QgisGui::ModalDialogFlags );

    ~QgsQueryBuilder();

    void showEvent( QShowEvent *event ) override;

  public slots:
    void accept() override;
    void reject() override;
    void clear();
    void on_btnEqual_clicked();
    void on_btnLessThan_clicked();
    void on_btnGreaterThan_clicked();
    void on_btnPct_clicked();
    void on_btnIn_clicked();
    void on_btnNotIn_clicked();
    void on_btnLike_clicked();
    void on_btnILike_clicked();
    QString sql();
    void setSql( const QString& sqlStatement );
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

    /** Test the constructed sql statement to see if the vector layer data provider likes it.
     * The number of rows that would be returned is displayed in a message box.
     * The test uses a "select count(*) from ..." query to test the SQL
     * statement.
     */
    void test();
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

    void setDatasourceDescription( const QString& uri );

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
    void fillValues( int idx, int limit );

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
#endif //QGSQUERYBUILDER_H
