/***************************************************************************
    qgspointcloudquerybuilder.h - query builder
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
#ifndef QGSPOINTCLOUDQUERYBUILDER_H
#define QGSPOINTCLOUDQUERYBUILDER_H
#include <map>
#include "qgis_sip.h"
#include <vector>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QModelIndex>
#include "ui_qgspointcloudquerybuilderbase.h"
#include "qgsguiutils.h"
#include "qgis_gui.h"
#include "qgssubsetstringeditorinterface.h"

class QgsPointCloudLayer;
class QgsCodeEditor;

/**
 * \ingroup gui
 * \class QgsPointCloudQueryBuilder
 * \brief Query Builder for layers.
 *
 * The query builder allows interactive creation of a SQL for limiting the
 * features displayed in a vector layer.  The fields in the table are
 * displayed and sample values (or all values) can be viewed to aid in
 * constructing the query. A test function returns the number of features that
 * will be returned.
 *
 */
class GUI_EXPORT QgsPointCloudQueryBuilder : public QgsSubsetStringEditorInterface, private Ui::QgsPointCloudQueryBuilderBase
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
    QgsPointCloudQueryBuilder( QgsPointCloudLayer *layer, QWidget *parent SIP_TRANSFERTHIS = nullptr,
                               Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    void showEvent( QShowEvent *event ) override;

    //! Returns the sql statement entered in the dialog.
    QString sql() const;

    //! Set the sql statement to display in the dialog.
    void setSql( const QString &sqlStatement );

    QString subsetString() const override { return sql(); }
    void setSubsetString( const QString &subsetString ) override { setSql( subsetString ); }

#ifdef SIP_RUN
    SIP_IF_FEATURE( HAVE_QSCI_SIP )

    /**
     * Returns the code editor widget for the SQL.
     * \since QGIS 3.18
     */
    QgsCodeEditor *codeEditorWidget() const;
    SIP_END
    SIP_IF_FEATURE( !HAVE_QSCI_SIP )

    /**
     * Returns the code editor widget for the SQL.
     * \since QGIS 3.18
     */
    QWidget *codeEditorWidget() const;
    SIP_END
#else

    /**
     * Returns the code editor widget for the SQL.
     * \since QGIS 3.18
     */
    QgsCodeEditor *codeEditorWidget() const { return mTxtSql; }
#endif

  public slots:
    void accept() override;
    void reject() override;
    void clear();

    /**
     * The default implementation tests that the constructed sql statement to
     * see if the vector layer data provider likes it.
     * The number of rows that would be returned is displayed in a message box.
     * The test uses a "select count(*) from ..." query to test the SQL
     * statement.
     */
    virtual void test();

    /**
     * Save query to the XML file
     * \since QGIS 3.16
     */
    void saveQuery();

    /**
     * Load query from the XML file
     * \since QGIS 3.16
     */
    void loadQuery();

    void setDatasourceDescription( const QString &uri );

  private slots:
    void btnEqual_clicked();
    void btnLessThan_clicked();
    void btnGreaterThan_clicked();
    void btnPct_clicked();
    void btnIn_clicked();
    void btnNotIn_clicked();
    void lstFields_clicked( const QModelIndex &index );
    void lstFields_doubleClicked( const QModelIndex &index );
    void btnLessEqual_clicked();
    void btnGreaterEqual_clicked();
    void btnNotEqual_clicked();
    void btnAnd_clicked();
    void btnNot_clicked();
    void btnOr_clicked();
    void layerSubsetStringChanged();


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

    // private members
    //! Model for fields ListView
    QStandardItemModel *mModelFields = nullptr;
    //! Previous field row to delete model
    int mPreviousFieldRow;

    //! vector layer
    QgsPointCloudLayer *mLayer = nullptr;

    //! original subset string
    QString mOrigSubsetString;

    //! whether to ignore subsetStringChanged() signal from the layer
    bool mIgnoreLayerSubsetStringChangedSignal = false;

    friend class TestQgsPointCloudQueryBuilder;
};
#endif //QGSPOINTCLOUDQUERYBUILDER_H
