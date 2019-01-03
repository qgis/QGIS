/***************************************************************************
   qgsexpressionlineedit.h
    ----------------------
   Date                 : 18.08.2016
   Copyright            : (C) 2016 Nyall Dawson
   Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSEXPRESSIONLINEEDIT_H
#define QGSEXPRESSIONLINEEDIT_H

#include <QWidget>
#include "qgis.h"
#include "qgsexpressioncontext.h"
#include "qgsdistancearea.h"
#include "qgis_gui.h"
#include <memory>

class QgsFilterLineEdit;
class QToolButton;
class QgsDistanceArea;
class QgsExpressionContextGenerator;
class QgsCodeEditorExpression;

/**
 * \ingroup gui
 * \class QgsExpressionLineEdit
 * \brief The QgsExpressionLineEdit widget includes a line edit for entering expressions
 * together with a button to open the expression creation dialog.
 *
 * This widget is designed for use in contexts where no layer fields are available for
 * use in an expression. In contexts where the expression is directly associated with
 * a layer and fields can be used, then QgsFieldExpressionWidget is a more appropriate
 * choice as it gives users direct access to select fields from a drop-down list.
 *
 * QgsExpressionLineEdit also supports a multiline editor mode which is useful where
 * more space is available for the widget, but where QgsExpressionBuilderWidget
 * is too complex or large for use.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsExpressionLineEdit : public QWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsExpressionLineEdit.
     * \param parent parent widget
     */
    explicit QgsExpressionLineEdit( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the title used in the expression builder dialog
     * \param title dialog title
     * \see expressionDialogTitle()
     */
    void setExpressionDialogTitle( const QString &title );

    /**
     * Returns the title used for the expression dialog.
     * \see setExpressionDialogTitle()
     */
    QString expressionDialogTitle() const { return mExpressionDialogTitle; }

    /**
     * Sets whether the widget should show a multiline text editor.
     * \param multiLine set to true to show multiline editor, or false
     * to show single line editor (the default).
     */
    void setMultiLine( bool multiLine );

    /**
     * Returns the expected format string, which is shown in the expression builder dialog for the widget.
     * This is purely a text format and no expression validation
     * is done against it.
     * \see setExpectedOutputFormat()
     * \since QGIS 3.4
     */
    QString expectedOutputFormat() const;

    /**
     * Set the \a expected format string, which is shown in the expression builder dialog for the widget.
     * This is purely a text format and no expression validation is done against it.
     * \see expectedOutputFormat()
     * \since QGIS 3.4
     */
    void setExpectedOutputFormat( const QString &expected );

    /**
     * Set the geometry calculator used in the expression dialog.
     * \param distanceArea calculator
     */
    void setGeomCalculator( const QgsDistanceArea &distanceArea );

    /**
     * Sets a layer associated with the widget. Required in order to get the fields and values
     * from the layer.
     * This will also automatically register the layer as expression context generator if
     * no generator has been set before or the previous layer has been used as generator.
     *
     * \see registerExpressionContextGenerator
     */
    void setLayer( QgsVectorLayer *layer );

    /**
     * Returns the current expression shown in the widget.
     * \see setExpression()
     */
    QString expression() const;

    /**
      * Determines if the current expression is valid.
      * \param expressionError will be set to any generated error message if specified
      * \returns true if the current expression is valid.
      */
    bool isValidExpression( QString *expressionError SIP_OUT = nullptr ) const;

    /**
     * Register an expression context generator class that will be used to retrieve
     * an expression context for the widget.
     * \param generator A QgsExpressionContextGenerator class that will be used to
     *                  create an expression context when required.
     */
    void registerExpressionContextGenerator( const QgsExpressionContextGenerator *generator );

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

  protected:

    void changeEvent( QEvent *event ) override;

  private slots:

    //! When the expression is edited by the user in the line edit, it will be checked for validity
    void expressionEdited( const QString &expression );
    void expressionEdited();

    //! Opens the expression editor dialog to edit the current expression or add a new expression
    void editExpression();

    /**
     * \brief updateLineEditStyle will re-style (color/font) the line edit depending on content and status
     * \param expression if expression is given it will be evaluated for the given string, otherwise it takes
     * current expression from the model
     */
    void updateLineEditStyle( const QString &expression = QString() );

  private:
    QgsFilterLineEdit *mLineEdit = nullptr;
    QgsCodeEditorExpression *mCodeEditor = nullptr;
    QToolButton *mButton = nullptr;
    QString mExpressionDialogTitle;
    std::unique_ptr<QgsDistanceArea> mDa;
    QgsExpressionContext mExpressionContext;
    const QgsExpressionContextGenerator *mExpressionContextGenerator = nullptr;
    QgsVectorLayer *mLayer = nullptr;
    QString mExpectedOutputFormat;

    bool isExpressionValid( const QString &expressionStr );

    friend class TestQgsFieldExpressionWidget;
};

#endif // QGSEXPRESSIONLINEEDIT_H
