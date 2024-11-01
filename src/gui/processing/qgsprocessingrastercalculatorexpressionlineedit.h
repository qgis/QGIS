/***************************************************************************
                         qgsprocessingrastercalculatorexpressionlineedit.h
                         ---------------------
    begin                : July 2023
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

#ifndef QGSPROCESSINGRASTERCALCULATOREXPRESSIONLINEEDIT_H
#define QGSPROCESSINGRASTERCALCULATOREXPRESSIONLINEEDIT_H

#define SIP_NO_FILE

#include <QWidget>
#include <QDialog>

#include "qgis.h"
#include "qgis_gui.h"
#include "ui_qgsprocessingrastercalculatorexpressiondialogbase.h"


class QgsFilterLineEdit;
class QToolButton;
class QgsMapLayer;

/// @cond PRIVATE

/**
 * Processing raster calculator expression line edit.
 * \ingroup gui
 * \class QgsProcessingRasterCalculatorExpressionLineEdit
 * \warning Not part of stable API and may change in future QGIS releases.
 * \since QGIS 3.34
 */
class GUI_EXPORT QgsProcessingRasterCalculatorExpressionLineEdit : public QWidget
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingRasterCalculatorExpressionLineEdit.
     * \param parent parent widget
     */
    explicit QgsProcessingRasterCalculatorExpressionLineEdit( QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsProcessingRasterCalculatorExpressionLineEdit() override;

    /**
     * Sets a layers associated with the widget.
     */
    void setLayers( const QVariantList &layers );

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
    QVariantList mLayers;
};

/**
 * A dialog for editing point cloud expressions.
 * \ingroup gui
 * \warning Not part of stable API and may change in future QGIS releases.
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsProcessingRasterCalculatorExpressionDialog : public QDialog, private Ui::QgsProcessingRasterCalculatorExpressionDialogBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingRasterCalculatorExpressionDialog.
     */
    QgsProcessingRasterCalculatorExpressionDialog( const QVariantList &layers, const QString &startExpression = QString(), QWidget *parent = nullptr );

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
    void mLayersList_itemDoubleClicked( QListWidgetItem *item );

    //calculator buttons
    void mBtnPlus_clicked();
    void mBtnMinus_clicked();
    void mBtnMultiply_clicked();
    void mBtnDivide_clicked();
    void mBtnPower_clicked();
    void mBtnSqrt_clicked();
    void mBtnOpenBracket_clicked();
    void mBtnCloseBracket_clicked();
    void mBtnGreater_clicked();
    void mBtnGreaterEqual_clicked();
    void mBtnLess_clicked();
    void mBtnLessEqual_clicked();
    void mBtnEqual_clicked();
    void mBtnNotEqual_clicked();
    void mBtnAnd_clicked();
    void mBtnOr_clicked();
    void mBtnIf_clicked();
    void mBtnMin_clicked();
    void mBtnMax_clicked();
    void mBtnAbs_clicked();
    void mBtnSin_clicked();
    void mBtnCos_clicked();
    void mBtnTan_clicked();
    void mBtnLog_clicked();
    void mBtnAsin_clicked();
    void mBtnAcos_clicked();
    void mBtnAtan_clicked();
    void mBtnLn_clicked();

  private:
    //! Populate the layer list
    void populateLayers();

    static QString quoteBandEntry( const QString &layerName );

    QVariantList mLayers;
    const QString mInitialText;
};

///@endcond
#endif // QGSPROCESSINGRASTERCALCULATOREXPRESSIONLINEEDIT_H
