/***************************************************************************
                         qgsprocessingpointcloudexpressionlineedit.h
                         ---------------------
    begin                : April 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGPOINTCLOUDEXPRESSIONLINEEDIT_H
#define QGSPROCESSINGPOINTCLOUDEXPRESSIONLINEEDIT_H

#define SIP_NO_FILE

#include <QWidget>
#include <QDialog>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QModelIndex>

#include "qgis.h"
#include "qgis_gui.h"
#include "ui_qgsprocessingpointcloudexpressiondialogbase.h"


class QgsFilterLineEdit;
class QToolButton;
class QgsPointCloudLayer;

/// @cond PRIVATE

/**
 * Processing point cloud expression line edit.
 * \ingroup gui
 * \class QgsProcessingPointCloudExpressionLineEdit
 * \warning Not part of stable API and may change in future QGIS releases.
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsProcessingPointCloudExpressionLineEdit : public QWidget
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingPointCloudExpressionLineEdit.
     * \param parent parent widget
     */
    explicit QgsProcessingPointCloudExpressionLineEdit( QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsProcessingPointCloudExpressionLineEdit() override;

    /**
     * Sets a layer associated with the widget. Required in order to get the attributes
     * from the layer.
     */
    void setLayer( QgsPointCloudLayer *layer );

    /**
     * Returns the layer currently associated with the widget.
     * \see setLayer()
     */
    QgsPointCloudLayer *layer() const;

    /**
     * Returns the current expression shown in the widget.
     * \see setExpression()
     */
    QString expression() const;

  signals:

    /**
     * Emitted when the expression is changed.
     * \param expression new expression
     */
    void expressionChanged( const QString &expression );

  public slots:

    /**
     * Sets the current expression to show in the widget.
     * \param expression expression string
     * \see expression()
     */
    void setExpression( const QString &expression );

  private slots:

    void expressionEdited( const QString &expression );
    void expressionEdited();

    //! Opens the expression editor dialog to edit the current expression
    void editExpression();

  private:
    QgsFilterLineEdit *mLineEdit = nullptr;
    QToolButton *mButton = nullptr;
    QgsPointCloudLayer *mLayer = nullptr;
};

/**
 * A dialog for editing point cloud expressions.
 * \ingroup gui
 * \warning Not part of stable API and may change in future QGIS releases.
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsProcessingPointCloudExpressionDialog : public QDialog, private Ui::QgsProcessingPointCloudExpressionDialogBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingPointCloudExpressionDialog.
     */
    QgsProcessingPointCloudExpressionDialog( QgsPointCloudLayer *layer, const QString &startExpression = QString(), QWidget *parent = nullptr );

    /**
     * Sets the current expression to show in the widget.
     * \param text expression string
     * \see expression()
     */
    void setExpression( const QString &text );

    /**
     * Returns the current expression shown in the widget.
     * \see setExpression()
     */
    QString expression();

  private slots:

    /**
     * Test if the typed expression is valid and can be used as a \a QgsPointCloudExpression
     */
    void test();

    /**
     * Clears the typed expression
     */
    void clear();

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

    //! Model for attributes ListView
    QStandardItemModel *mModelAttributes = nullptr;

    //! Model for values ListView
    QStandardItemModel *mModelValues = nullptr;

    QgsPointCloudLayer *mLayer = nullptr;
    const QString mInitialText;
};

///@endcond
#endif // QGSPROCESSINGPOINTCLOUDEXPRESSIONLINEEDIT_H
