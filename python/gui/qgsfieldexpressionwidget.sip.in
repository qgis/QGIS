/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfieldexpressionwidget.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsFieldExpressionWidget : QWidget
{
%Docstring
 The QgsFieldExpressionWidget class reates a widget to choose fields and edit expressions
It contains a combo boxto display the fields and expression and a button to open the expression dialog.
The combo box is editable, allowing expressions to be edited inline.
The validity of the expression is checked live on key press, invalid expressions are displayed in red.
The expression will be added to the model (and the fieldChanged signals emitted)
only when editing in the line edit is finished (focus lost, enter key pressed).
%End

%TypeHeaderCode
#include "qgsfieldexpressionwidget.h"
%End
  public:

    explicit QgsFieldExpressionWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
 QgsFieldExpressionWidget creates a widget with a combo box to display the fields and expression and a button to open the expression dialog
%End

    void setExpressionDialogTitle( const QString &title );
%Docstring
define the title used in the expression dialog
%End

    const QString expressionDialogTitle();
%Docstring
return the title used for the expression dialog
%End

    void setFilters( QgsFieldProxyModel::Filters filters );
%Docstring
setFilters allows fitering according to the type of field
%End

    void setLeftHandButtonStyle( bool isLeft );

    QgsFieldProxyModel::Filters filters() const;
%Docstring
currently used filter on list of fields
%End

    void setGeomCalculator( const QgsDistanceArea &da );
%Docstring
set the geometry calculator used in the expression dialog
%End

    QString currentField( bool *isExpression = 0, bool *isValid = 0 ) const;
%Docstring
 currentField returns the currently selected field or expression if allowed

:param isExpression: determines if the string returned is the name of a field or an expression
:param isValid: determines if the expression (or field) returned is valid
%End

    bool isValidExpression( QString *expressionError = 0 ) const;
%Docstring
Return true if the current expression is valid
%End

    bool isExpression() const;
%Docstring
If the content is not just a simple field this method will return true.
%End

    QString currentText() const;
%Docstring
Return the current text that is set in the expression area
%End

    QString asExpression() const;
%Docstring
Returns the currently selected field or expression. If a field is currently selected, the returned
value will be converted to a valid expression referencing this field (ie enclosing the field name with
appropriate quotations).

.. versionadded:: 2.14
%End

    QString expression() const;
%Docstring
Returns the currently selected field or expression. If a field is currently selected, the returned
value will be converted to a valid expression referencing this field (ie enclosing the field name with
appropriate quotations).

Alias for asExpression()

.. versionadded:: 3.0
%End

    QgsVectorLayer *layer() const;
%Docstring
Returns the layer currently associated with the widget.

.. seealso:: :py:func:`setLayer`
%End

    void registerExpressionContextGenerator( const QgsExpressionContextGenerator *generator );
%Docstring
Register an expression context generator class that will be used to retrieve
an expression context for the widget.

:param generator: A QgsExpressionContextGenerator class that will be used to
create an expression context when required.

.. versionadded:: 3.0
%End

    bool allowEvalErrors() const;
%Docstring
Allow accepting expressions with evaluation errors. This can be useful when we are not able to
provide an expression context of which we are sure it's completely populated.

.. versionadded:: 3.0
%End

    void setAllowEvalErrors( bool allowEvalErrors );
%Docstring
Allow accepting expressions with evaluation errors. This can be useful when we are not able to
provide an expression context of which we are sure it's completely populated.

.. versionadded:: 3.0
%End

  signals:
    void fieldChanged( const QString &fieldName );
%Docstring
the signal is emitted when the currently selected field changes
%End

    void fieldChanged( const QString &fieldName, bool isValid );
%Docstring
fieldChanged signal with indication of the validity of the expression
%End

    void allowEvalErrorsChanged();
%Docstring
Allow accepting expressions with evaluation errors. This can be useful when we are not able to
provide an expression context of which we are sure it's completely populated.

.. versionadded:: 3.0
%End

  public slots:

    void setLayer( QgsMapLayer *layer );
%Docstring
Sets the layer used to display the fields and expression.

.. seealso:: :py:func:`layer`
%End

    void setRow( int row );
%Docstring
sets the current row in the widget
%End

    void setField( const QString &fieldName );
%Docstring
sets the current field or expression in the widget
%End

    void setExpression( const QString &expression );
%Docstring
Sets the current expression text and if applicable also the field.
Alias for setField.

.. versionadded:: 3.0
%End

  protected slots:
    void editExpression();
%Docstring
open the expression dialog to edit the current or add a new expression
%End

    void expressionEdited( const QString &expression );
%Docstring
when expression is edited by the user in the line edit, it will be checked for validity
%End

    void expressionEditingFinished();
%Docstring
when expression has been edited (finished) it will be added to the model
%End

    void currentFieldChanged();

    void updateLineEditStyle( const QString &expression = QString() );
%Docstring
 updateLineEditStyle will re-style (color/font) the line edit depending on content and status

:param expression: if expression is given it will be evaluated for the given string, otherwise it takes
current expression from the model
%End

    bool isExpressionValid( const QString &expressionStr );

  protected:
    virtual void changeEvent( QEvent *event );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfieldexpressionwidget.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
