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
#include "qgis_sip.h"
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
 * The query builder allows interactive creation of an expression for limiting the
 * points displayed in a point cloud layer.  The point attributes are displayed
 * and sample values can be viewed to aid in constructing the expression.
 * A test function checks that the typed expression is valid.
 *
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsPointCloudQueryBuilder : public QgsSubsetStringEditorInterface, private Ui::QgsPointCloudQueryBuilderBase
{
    Q_OBJECT
  public:

    /**
     * This constructor is used when the query builder is called from the
     * layer properties dialog
     * \param layer existing point cloud layer
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
     */
    QgsCodeEditor *codeEditorWidget() const;
    SIP_END
    SIP_IF_FEATURE( !HAVE_QSCI_SIP )

    /**
     * Returns the code editor widget for the SQL.
     */
    QWidget *codeEditorWidget() const;
    SIP_END
#else

    /**
     * Returns the code editor widget for the SQL.
     */
    QgsCodeEditor *codeEditorWidget() const { return mTxtSql; }
#endif

  public slots:
    void accept() override;
    void reject() override;
    void clear();

    /**
     * Test if the typed expression is valid and can be used as a \a QgsPointCloudExpression
     */
    virtual void test();

    /**
     * Save expression to the XML file
     */
    void saveQuery();

    /**
     * Load expression from the XML file
     */
    void loadQuery();

    void setDatasourceDescription( const QString &uri );

  private slots:
    void btnEqual_clicked();
    void btnLessThan_clicked();
    void btnGreaterThan_clicked();
    void btnIn_clicked();
    void btnNotIn_clicked();
    void lstFields_clicked( const QModelIndex &index );
    void lstFields_doubleClicked( const QModelIndex &index );
    void btnLessEqual_clicked();
    void btnGreaterEqual_clicked();
    void btnNotEqual_clicked();
    void btnAnd_clicked();
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

    //! the point cloud layer
    QgsPointCloudLayer *mLayer = nullptr;

    //! original subset string
    QString mOrigSubsetString;

    //! whether to ignore subsetStringChanged() signal from the layer
    bool mIgnoreLayerSubsetStringChangedSignal = false;

    friend class TestQgsPointCloudQueryBuilder;
};
#endif //QGSPOINTCLOUDQUERYBUILDER_H
