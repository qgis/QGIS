/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsexpressionlineedit.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsExpressionLineEdit : QWidget
{
%Docstring
 The QgsExpressionLineEdit widget includes a line edit for entering expressions
together with a button to open the expression creation dialog.

This widget is designed for use in contexts where no layer fields are available for
use in an expression. In contexts where the expression is directly associated with
a layer and fields can be used, then QgsFieldExpressionWidget is a more appropriate
choice as it gives users direct access to select fields from a drop-down list.

QgsExpressionLineEdit also supports a multiline editor mode which is useful where
more space is available for the widget, but where QgsExpressionBuilderWidget
is too complex or large for use.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsexpressionlineedit.h"
%End
  public:

    explicit QgsExpressionLineEdit( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsExpressionLineEdit.

:param parent: parent widget
%End

    void setExpressionDialogTitle( const QString &title );
%Docstring
Sets the title used in the expression builder dialog

:param title: dialog title

.. seealso:: :py:func:`expressionDialogTitle`
%End

    QString expressionDialogTitle() const;
%Docstring
Returns the title used for the expression dialog.

.. seealso:: :py:func:`setExpressionDialogTitle`
%End

    void setMultiLine( bool multiLine );
%Docstring
Sets whether the widget should show a multiline text editor.

:param multiLine: set to true to show multiline editor, or false
to show single line editor (the default).
%End

    void setGeomCalculator( const QgsDistanceArea &distanceArea );
%Docstring
Set the geometry calculator used in the expression dialog.

:param distanceArea: calculator
%End

    void setLayer( QgsVectorLayer *layer );
%Docstring
Sets a layer associated with the widget. Required in order to get the fields and values
from the layer.
This will also automatically register the layer as expression context generator if
no generator has been set before or the previous layer has been used as generator.

.. seealso:: :py:func:`registerExpressionContextGenerator`
%End

    QString expression() const;
%Docstring
Returns the current expression shown in the widget.

.. seealso:: :py:func:`setExpression`
%End

    bool isValidExpression( QString *expressionError /Out/ = 0 ) const;
%Docstring
Returns true if the current expression is valid.

:param expressionError: will be set to any generated error message if specified
%End

    void registerExpressionContextGenerator( const QgsExpressionContextGenerator *generator );
%Docstring
Register an expression context generator class that will be used to retrieve
an expression context for the widget.

:param generator: A QgsExpressionContextGenerator class that will be used to
create an expression context when required.
%End

  signals:

    void expressionChanged( const QString &expression );
%Docstring
Emitted when the expression is changed.

:param expression: new expression
%End

  public slots:

    void setExpression( const QString &expression );
%Docstring
Sets the current expression to show in the widget.

:param expression: expression string

.. seealso:: :py:func:`expression`
%End

  protected:

    virtual void changeEvent( QEvent *event );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsexpressionlineedit.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
