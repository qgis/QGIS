/***************************************************************************
    qgspointcloudquerybuilder.h  - Query builder for point cloud layers
    ---------------------------
    begin                : March 2022
    copyright            : (C) 2022 by Stefanos Natsis
    email                : uclaros at gmail dot com
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
#include <QStandardItemModel>
#include <QStandardItem>
#include <QModelIndex>
#include "ui_qgspointcloudquerybuilderbase.h"
#include "qgis_sip.h"
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
 * points displayed in a point cloud layer. The point attributes are displayed and
 * value ranges can be viewed to aid in constructing the expression.
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

    QString subsetString() const override { return mTxtSql->text(); }
    void setSubsetString( const QString &subsetString ) override { mTxtSql->setText( subsetString ); }


  public slots:
    void accept() override;
    void reject() override;

  private slots:

    /**
     * Test if the typed expression is valid and can be used as a \a QgsPointCloudExpression
     */
    virtual void test();

    /**
     * Clears the typed expression
     */
    void clear();

    /**
     * Save expression to the XML file
     */
    void saveQuery();

    /**
     * Load expression from the XML file
     */
    void loadQuery();

    void lstAttributes_currentChanged( const QModelIndex &current, const QModelIndex &previous );
    void lstAttributes_doubleClicked( const QModelIndex &index );
    void lstValues_doubleClicked( const QModelIndex &index );
    void btnEqual_clicked();
    void btnLessThan_clicked();
    void btnGreaterThan_clicked();
    void btnIn_clicked();
    void btnNotIn_clicked();
    void btnLessEqual_clicked();
    void btnGreaterEqual_clicked();
    void btnNotEqual_clicked();
    void btnAnd_clicked();
    void btnOr_clicked();

  private:

    //! Populate the attribute list for the selected layer
    void populateAttributes();

    //! Setup models for listviews
    void setupGuiViews();

    // private members
    //! Model for attributes ListView
    QStandardItemModel *mModelAttributes = nullptr;

    //! Model for values ListView
    QStandardItemModel *mModelValues = nullptr;

    //! the point cloud layer
    QgsPointCloudLayer *mLayer = nullptr;

    //! original subset string
    QString mOrigSubsetString;
};
#endif //QGSPOINTCLOUDQUERYBUILDER_H
