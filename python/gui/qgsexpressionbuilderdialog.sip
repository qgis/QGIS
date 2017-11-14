/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsexpressionbuilderdialog.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsExpressionBuilderDialog : QDialog
{
%Docstring
 A generic dialog for building expression strings
 @remarks This class also shows an example on how to use QgsExpressionBuilderWidget
%End

%TypeHeaderCode
#include "qgsexpressionbuilderdialog.h"
%End
  public:
    QgsExpressionBuilderDialog( QgsVectorLayer *layer,
                                const QString &startText = QString(),
                                QWidget *parent /TransferThis/ = 0,
                                const QString &key = "generic",
                                const QgsExpressionContext &context = QgsExpressionContext() );

    QgsExpressionBuilderWidget *expressionBuilder();
%Docstring
The builder widget that is used by the dialog
 :rtype: QgsExpressionBuilderWidget
%End

    void setExpressionText( const QString &text );

    QString expressionText();
%Docstring
 :rtype: str
%End

    QgsExpressionContext expressionContext() const;
%Docstring
 Returns the expression context for the dialog. The context is used for the expression
 preview result and for populating the list of available functions and variables.
.. seealso:: setExpressionContext
.. versionadded:: 2.12
 :rtype: QgsExpressionContext
%End

    void setExpressionContext( const QgsExpressionContext &context );
%Docstring
 Sets the expression context for the dialog. The context is used for the expression
 preview result and for populating the list of available functions and variables.
 \param context expression context
.. seealso:: expressionContext
.. versionadded:: 2.12
%End

    void setGeomCalculator( const QgsDistanceArea &da );
%Docstring
Sets geometry calculator used in distance/area calculations.
%End

  protected:

    virtual void done( int r );
%Docstring
 Is called when the dialog get accepted or rejected
 Used to save geometry

 \param r result value (unused)
%End

    virtual void accept();

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsexpressionbuilderdialog.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
