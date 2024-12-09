/***************************************************************************
   qgsfieldexpressionwidget.h
    --------------------------------------
   Date                 : 01.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSFIELDEXPRESSIONWIDGET_H
#define QGSFIELDEXPRESSIONWIDGET_H

#include <QColor>
#include <QComboBox>
#include <QToolButton>
#include <QWidget>
#include <memory>

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsdistancearea.h"
#include "qgsexpressioncontext.h"
#include "qgsfieldproxymodel.h"


class QgsMapLayer;
class QgsVectorLayer;
class QgsExpressionContextGenerator;


/**
 * \ingroup gui
 * \brief The QgsFieldExpressionWidget class creates a widget to choose fields and edit expressions
 * It contains a combo box to display the fields and expression and a button to open the expression dialog.
 * The combo box is editable, allowing expressions to be edited inline.
 * The validity of the expression is checked live on key press, invalid expressions are displayed in red.
 * The expression will be added to the model (and the fieldChanged signals emitted)
 * only when editing in the line edit is finished (focus lost, enter key pressed).
 */
class GUI_EXPORT QgsFieldExpressionWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString expressionDialogTitle READ expressionDialogTitle WRITE setExpressionDialogTitle )
    Q_PROPERTY( QgsFieldProxyModel::Filters filters READ filters WRITE setFilters )
    Q_PROPERTY( bool allowEmptyFieldName READ allowEmptyFieldName WRITE setAllowEmptyFieldName )
    Q_PROPERTY( bool allowEvalErrors READ allowEvalErrors WRITE setAllowEvalErrors NOTIFY allowEvalErrorsChanged )
    Q_PROPERTY( bool buttonVisible READ buttonVisible WRITE setButtonVisible NOTIFY buttonVisibleChanged )

  public:
    /**
     * \brief QgsFieldExpressionWidget creates a widget with a combo box to display the fields and expression and a button to open the expression dialog
     */
    explicit QgsFieldExpressionWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! define the title used in the expression dialog
    void setExpressionDialogTitle( const QString &title );

    /**
     * Appends a scope to the current expression context.
     *
     * \param scope The scope to add.
     *
     * \since QGIS 3.2
     */
    void appendScope( QgsExpressionContextScope *scope SIP_TRANSFER );

    //! Returns the title used for the expression dialog
    const QString expressionDialogTitle() { return mExpressionDialogTitle; }

    //! setFilters allows filtering according to the type of field
    void setFilters( QgsFieldProxyModel::Filters filters );

    /**
     * Sets whether an optional empty field ("not set") option is shown in the combo box.
     * \see allowEmptyFieldName()
     * \since QGIS 3.4.6
     */
    void setAllowEmptyFieldName( bool allowEmpty );

    /**
     * Returns TRUE if the combo box allows the empty field ("not set") choice.
     * \see setAllowEmptyFieldName()
     * \since QGIS 3.4.6
     */
    bool allowEmptyFieldName() const;

    void setLeftHandButtonStyle( bool isLeft );

    //! currently used filter on list of fields
    QgsFieldProxyModel::Filters filters() const { return mFieldProxyModel->filters(); }

    //! Sets the geometry calculator used in the expression dialog
    void setGeomCalculator( const QgsDistanceArea &da );

    /**
     * \brief currentField returns the currently selected field or expression if allowed
     * \param isExpression determines if the string returned is the name of a field or an expression
     * \param isValid determines if the expression (or field) returned is valid
     */
    QString currentField( bool *isExpression = nullptr, bool *isValid = nullptr ) const;

    /**
      * Returns TRUE if the current expression is valid
      */
    bool isValidExpression( QString *expressionError = nullptr ) const;

    /**
     * If the content is not just a simple field this method will return TRUE.
     */
    bool isExpression() const;

    /**
      * Returns the current text that is set in the expression area
      */
    QString currentText() const;

    /**
     * Returns the currently selected field or expression. If a field is currently selected, the returned
     * value will be converted to a valid expression referencing this field (ie enclosing the field name with
     * appropriate quotations).
     */
    QString asExpression() const;

    /**
     * Returns the currently selected field or expression. If a field is currently selected, the returned
     * value will be converted to a valid expression referencing this field (ie enclosing the field name with
     * appropriate quotations).
     *
     * Alias for asExpression()
     *
     */
    QString expression() const;

    /**
     * Returns the layer currently associated with the widget.
     * \see setLayer()
     */
    QgsVectorLayer *layer() const;

    /**
     * Register an expression context generator class that will be used to retrieve
     * an expression context for the widget.
     * \param generator A QgsExpressionContextGenerator class that will be used to
     *                  create an expression context when required.
     */
    void registerExpressionContextGenerator( const QgsExpressionContextGenerator *generator );

#ifndef SIP_RUN

    /**
     * Sets the widget to run using a custom preview generator.
     *
     * In this mode, the widget will call a callback function to generate a new QgsExpressionContext
     * as the previewed object changes. This can be used to provide custom preview values for different
     * objects (i.e. for objects which aren't vector layer features).
     *
     * \param label The label to display for the combo box presenting choices of objects. This should be a representative name, eg "Band" if the widget is showing choices of raster layer bands
     * \param choices A list of choices to present to the user. Each choice is a pair of a human-readable label and a QVariant representing the object to preview.
     * \param previewContextGenerator A function which takes a QVariant representing the object to preview, and returns a QgsExpressionContext to use for previewing the object.
     *
     * \since QGIS 3.38
     */
    void setCustomPreviewGenerator( const QString &label, const QList<QPair<QString, QVariant>> &choices, const std::function<QgsExpressionContext( const QVariant & )> &previewContextGenerator );
#else

    /**
     * Sets the widget to run using a custom preview generator.
     *
     * In this mode, the widget will call a callback function to generate a new QgsExpressionContext
     * as the previewed object changes. This can be used to provide custom preview values for different
     * objects (i.e. for objects which aren't vector layer features).
     *
     * \param label The label to display for the combo box presenting choices of objects. This should be a representative name, eg "Band" if the widget is showing choices of raster layer bands
     * \param choices A list of choices to present to the user. Each choice is a pair of a human-readable label and a QVariant representing the object to preview.
     * \param previewContextGenerator A function which takes a QVariant representing the object to preview, and returns a QgsExpressionContext to use for previewing the object.
     *
     * \since QGIS 3.38
     */
    void setCustomPreviewGenerator( const QString &label, const QList<QPair<QString, QVariant>> &choices, SIP_PYCALLABLE );
    //%MethodCode
    Py_XINCREF( a2 );
    Py_BEGIN_ALLOW_THREADS
      sipCpp->setCustomPreviewGenerator( *a0, *a1, [a2]( const QVariant &value ) -> QgsExpressionContext {
        QgsExpressionContext res;
        SIP_BLOCK_THREADS
        PyObject *s = sipCallMethod( NULL, a2, "D", &value, sipType_QVariant, NULL );
        int state;
        int sipIsError = 0;
        QgsExpressionContext *t1 = reinterpret_cast<QgsExpressionContext *>( sipConvertToType( s, sipType_QgsExpressionContext, 0, SIP_NOT_NONE, &state, &sipIsError ) );
        if ( sipIsError == 0 )
        {
          res = QgsExpressionContext( *t1 );
        }
        sipReleaseType( t1, sipType_QgsExpressionContext, state );
        SIP_UNBLOCK_THREADS
        return res;
      } );

    Py_END_ALLOW_THREADS
    //%End
#endif

    /**
     * Allow accepting expressions with evaluation errors. This can be useful when we are not able to
     * provide an expression context of which we are sure it's completely populated.
     *
     */
    bool allowEvalErrors() const;

    /**
     * Allow accepting expressions with evaluation errors. This can be useful when we are not able to
     * provide an expression context of which we are sure it's completely populated.
     *
     */
    void setAllowEvalErrors( bool allowEvalErrors );

    /**
     * Returns the visibility of the button
     *
     * If button is hidden, the widget essentially becomes an editable combo box
     *
     * \since QGIS 3.36
     */
    bool buttonVisible() const;

    /**
     * Set the visibility of the button
     *
     * If button is hidden, the widget essentially becomes an editable combo box
     *
     * \since QGIS 3.36
     */
    void setButtonVisible( bool visible );

  signals:
    //! Emitted when the currently selected field changes.
    void fieldChanged( const QString &fieldName );

    //! fieldChanged signal with indication of the validity of the expression
    void fieldChanged( const QString &fieldName, bool isValid );

    /**
     * Allow accepting expressions with evaluation errors. This can be useful when we are not able to
     * provide an expression context of which we are sure it's completely populated.
     *
     */
    void allowEvalErrorsChanged();

    /**
     * Emitted when the button visibility changes
     *
     * \since QGIS 3.36
     */
    void buttonVisibleChanged();

  public slots:

    /**
     * Sets the layer used to display the fields and expression.
     * \see layer()
     */
    void setLayer( QgsMapLayer *layer );

    //! sets the current row in the widget
    void setRow( int row ) { mCombo->setCurrentIndex( row ); }

    //! sets the current field or expression in the widget
    void setField( const QString &fieldName );

    /**
     * Sets the fields used in the widget to \a fields, this allows the widget to work without a layer.
     * \since QGIS 3.14
     */
    void setFields( const QgsFields &fields );

    /**
     * Sets the current expression text and if applicable also the field.
     * Alias for setField.
     *
     */
    void setExpression( const QString &expression );

  protected slots:
    //! open the expression dialog to edit the current or add a new expression
    void editExpression();

    //! when expression is edited by the user in the line edit, it will be checked for validity
    void expressionEdited( const QString &expression );

    //! when expression has been edited (finished) it will be added to the model
    void expressionEditingFinished();

    void currentFieldChanged();

    /**
     * \brief updateLineEditStyle will re-style (color/font) the line edit depending on content and status
     * \param expression if expression is given it will be evaluated for the given string, otherwise it takes
     * current expression from the model
     */
    void updateLineEditStyle( const QString &expression = QString() );

    bool isExpressionValid( const QString &expressionStr );

  protected:
    void changeEvent( QEvent *event ) override;

    bool eventFilter( QObject *watched, QEvent *event ) override;

  private slots:
    void reloadLayer();

    void beforeResetModel();
    void afterResetModel();

  private:
    QComboBox *mCombo = nullptr;
    QToolButton *mButton = nullptr;
    QgsFieldProxyModel *mFieldProxyModel = nullptr;
    QString mExpressionDialogTitle;
    std::shared_ptr<const QgsDistanceArea> mDistanceArea;
    QgsExpressionContext mExpressionContext;
    const QgsExpressionContextGenerator *mExpressionContextGenerator = nullptr;
    QString mBackupExpression;
    bool mAllowEvalErrors = false;

    QString mCustomPreviewLabel;
    QList<QPair<QString, QVariant>> mCustomChoices;
    std::function<QgsExpressionContext( const QVariant & )> mPreviewContextGenerator;

    friend class TestQgsFieldExpressionWidget;
};

#endif // QGSFIELDEXPRESSIONWIDGET_H
