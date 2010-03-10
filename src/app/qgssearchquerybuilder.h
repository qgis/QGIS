/***************************************************************************
    qgssearchquerybuilder.h  - Query builder for search strings
    ----------------------
    begin                : March 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSSEARCHQUERYBUILDER_H
#define QGSSEARCHQUERYBUILDER_H

#include <map>
#include <vector>
#include <QStandardItemModel>
#include <QModelIndex>

#include "ui_qgsquerybuilderbase.h"
#include "qgisgui.h"

class QgsField;
class QgsVectorLayer;

/*!
 * \class QgsSearchQueryBuilder
 * \brief Query Builder for search strings
 *
 */
class QgsSearchQueryBuilder : public QDialog, private Ui::QgsQueryBuilderBase
{
    Q_OBJECT

  public:
    //! Constructor - takes pointer to vector layer as a parameter
    QgsSearchQueryBuilder( QgsVectorLayer* layer, QWidget *parent = 0,
                           Qt::WFlags fl = QgisGui::ModalDialogFlags );

    ~QgsSearchQueryBuilder();

    //! returns newly created search string
    QString searchString();

    //! change search string shown in text field
    void setSearchString( QString searchString );

  public slots:
    void on_btnEqual_clicked();
    void on_btnOk_clicked();
    void on_btnLessThan_clicked();
    void on_btnGreaterThan_clicked();
    void on_btnLike_clicked();
    void on_btnILike_clicked();

    void on_lstFields_doubleClicked( const QModelIndex &index );
    void on_lstValues_doubleClicked( const QModelIndex &index );
    void on_btnLessEqual_clicked();
    void on_btnGreaterEqual_clicked();
    void on_btnNotEqual_clicked();
    void on_btnAnd_clicked();
    void on_btnNot_clicked();
    void on_btnOr_clicked();
    void on_btnClear_clicked();

    /*! Test the constructed search string to see if it's correct.
     * The number of rows that would be returned is displayed in a message box.
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

  private:

    /*!
    * Populate the field list for the selected table
    */
    void populateFields();
    /*!
       * Setup models for listviews
     */
    void setupListViews();

    /*! Get the number of records that would be returned by the current SQL
     * @return Number of records or -1 if an error was encountered
     */
    long countRecords( QString sql );

    /*!
     * populates list box with values of selected field
     * @param limit if not zero, inserts only this count of values
     */
    void getFieldValues( int limit );

  private:

    //! Layer for which is the query builder opened
    QgsVectorLayer* mLayer;
    //! Map that holds field information, keyed by field name
    QMap<QString, int> mFieldMap;
    //! Model for fields ListView
    QStandardItemModel *mModelFields;
    //! Model for values ListView
    QStandardItemModel *mModelValues;
};
#endif //QGSSEARCHQUERYBUILDER_H
