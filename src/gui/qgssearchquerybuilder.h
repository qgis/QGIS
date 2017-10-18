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

#ifndef QGSSEARCHQUERYBUILDER_H
#define QGSSEARCHQUERYBUILDER_H

#include <map>
#include <vector>
#include <QStandardItemModel>
#include <QModelIndex>

#include "ui_qgsquerybuilderbase.h"
#include "qgsguiutils.h"
#include "qgis_gui.h"

class QgsField;
class QgsVectorLayer;

/**
 * \ingroup gui
 * \class QgsSearchQueryBuilder
 * \brief Query Builder for search strings
 */
class GUI_EXPORT QgsSearchQueryBuilder : public QDialog, private Ui::QgsQueryBuilderBase
{
    Q_OBJECT

  public:
    //! Constructor - takes pointer to vector layer as a parameter
    QgsSearchQueryBuilder( QgsVectorLayer *layer,
                           QWidget *parent SIP_TRANSFERTHIS = nullptr,
                           Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    //! returns newly created search string
    QString searchString();

    //! change search string shown in text field
    void setSearchString( const QString &searchString );

  public slots:

    void saveQuery();
    void loadQuery();

  private slots:
    void btnEqual_clicked();
    void on_btnOk_clicked();
    void btnLessThan_clicked();
    void btnGreaterThan_clicked();
    void btnLike_clicked();
    void btnILike_clicked();
    void btnPct_clicked();
    void btnIn_clicked();
    void btnNotIn_clicked();

    void lstFields_doubleClicked( const QModelIndex &index );
    void lstValues_doubleClicked( const QModelIndex &index );
    void btnLessEqual_clicked();
    void btnGreaterEqual_clicked();
    void btnNotEqual_clicked();
    void btnAnd_clicked();
    void btnNot_clicked();
    void btnOr_clicked();
    void btnClear_clicked();

    /**
     * Test the constructed search string to see if it's correct.
     * The number of rows that would be returned is displayed in a message box.
     */
    void btnTest_clicked();

    /**
     * Get all distinct values for the field. Values are inserted
     * into the value list box
     */
    void btnGetAllValues_clicked();

    /**
     * Get sample distinct values for the selected field. The sample size is
     * limited to an arbitrary value (currently set to 25). The values
     * are inserted into the values list box.
     */
    void btnSampleValues_clicked();

  private:

    /**
     * Populate the field list for the selected table
     */
    void populateFields();

    /**
     * Setup models for listviews
     */
    void setupListViews();

    /**
     * Get the number of records that would be returned by the current SQL
     * \returns Number of records or -1 if an error was encountered
     */
    long countRecords( const QString &sql );

    /**
     * populates list box with values of selected field
     * \param limit if not zero, inserts only this count of values
     */
    void getFieldValues( int limit );

    /**
     * Open the help in a browser
     */
    void showHelp();

  private:

    //! Layer for which is the query builder opened
    QgsVectorLayer *mLayer = nullptr;
    //! Map that holds field information, keyed by field name
    QMap<QString, int> mFieldMap;
    //! Model for fields ListView
    QStandardItemModel *mModelFields = nullptr;
    //! Model for values ListView
    QStandardItemModel *mModelValues = nullptr;
};
#endif //QGSSEARCHQUERYBUILDER_H
