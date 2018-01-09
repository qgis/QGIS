/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/expression/qgsexpressionfunction.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsExpressionFunction
{
%Docstring
A abstract base class for defining QgsExpression functions.
%End

%TypeHeaderCode
#include "qgsexpressionfunction.h"
%End
  public:


    class Parameter
{
%Docstring
Represents a single parameter passed to a function.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsexpressionfunction.h"
%End
      public:

        Parameter( const QString &name,
                   bool optional = false,
                   const QVariant &defaultValue = QVariant() );
%Docstring
Constructor for Parameter.

:param name: parameter name, used when named parameter are specified in an expression
:param optional: set to true if parameter should be optional
:param defaultValue: default value to use for optional parameters
%End

        QString name() const;
%Docstring
Returns the name of the parameter.
%End

        bool optional() const;
%Docstring
Returns true if the parameter is optional.
%End

        QVariant defaultValue() const;
%Docstring
Returns the default value for the parameter.
%End

        bool operator==( const QgsExpressionFunction::Parameter &other ) const;

    };

    typedef QList< QgsExpressionFunction::Parameter > ParameterList;

    QgsExpressionFunction( const QString &fnname,
                           int params,
                           const QString &group,
                           const QString &helpText = QString(),
                           bool lazyEval = false,
                           bool handlesNull = false,
                           bool isContextual = false );
%Docstring
Constructor for function which uses unnamed parameters
%End

    QgsExpressionFunction( const QString &fnname,
                           int params,
                           const QStringList &groups,
                           const QString &helpText = QString(),
                           bool lazyEval = false,
                           bool handlesNull = false,
                           bool isContextual = false );
%Docstring
Constructor for function which uses unnamed parameters and group list

.. versionadded:: 3.0
%End

    QgsExpressionFunction( const QString &fnname,
                           const QgsExpressionFunction::ParameterList &params,
                           const QString &group,
                           const QString &helpText = QString(),
                           bool lazyEval = false,
                           bool handlesNull = false,
                           bool isContextual = false );
%Docstring
Constructor for function which uses named parameter list.

.. versionadded:: 2.16
%End

    QgsExpressionFunction( const QString &fnname,
                           const QgsExpressionFunction::ParameterList &params,
                           const QStringList &groups,
                           const QString &helpText = QString(),
                           bool lazyEval = false,
                           bool handlesNull = false,
                           bool isContextual = false );
%Docstring
Constructor for function which uses named parameter list and group list.

.. versionadded:: 3.0
%End

    virtual ~QgsExpressionFunction();

    QString name() const;
%Docstring
The name of the function.
%End

    int params() const;
%Docstring
The number of parameters this function takes.
%End

    int minParams() const;
%Docstring
The minimum number of parameters this function takes.
%End

    const QgsExpressionFunction::ParameterList &parameters() const;
%Docstring
Returns the list of named parameters for the function, if set.

.. versionadded:: 2.16
%End

    virtual bool usesGeometry( const QgsExpressionNodeFunction *node ) const;
%Docstring
Does this function use a geometry object.
%End

    virtual QStringList aliases() const;
%Docstring
Returns a list of possible aliases for the function. These include
other permissible names for the function, e.g., deprecated names.

:return: list of known aliases

.. versionadded:: 2.9
%End

    bool lazyEval() const;
%Docstring
True if this function should use lazy evaluation.  Lazy evaluation functions take QgsExpression.Node objects
rather than the node results when called.  You can use node->eval(parent, feature) to evaluate the node and return the result
Functions are non lazy default and will be given the node return value when called.
%End

    virtual bool isStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const;
%Docstring
Will be called during prepare to determine if the function is static.
A function is static if it will return the same value for every feature with different
attributes and/or geometry.

By default this will return true, if all arguments that have been passed to the function
are also static.

.. versionadded:: 3.0
%End

    virtual bool prepare( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const;
%Docstring
This will be called during the prepare step() of an expression if it is not static.

This can be used by functions to do any preparation steps that might help to speedup the upcoming
evaluation.

.. versionadded:: 3.0
%End

    virtual QSet<QString> referencedColumns( const QgsExpressionNodeFunction *node ) const;
%Docstring
Returns a set of field names which are required for this function.
May contain QgsFeatureRequest.AllAttributes to signal that all
attributes are required.
If in doubt this will return more fields than strictly required.

.. versionadded:: 3.0
%End

    bool isContextual() const;
%Docstring
Returns whether the function is only available if provided by a QgsExpressionContext object.

.. versionadded:: 2.12
%End

    virtual bool isDeprecated() const;
%Docstring
Returns true if the function is deprecated and should not be presented as a valid option
to users in expression builders.

.. versionadded:: 3.0
%End

    QString group() const;
%Docstring
Returns the first group which the function belongs to.

.. note::

   consider using groups() instead, as some functions naturally belong in multiple groups
%End

    QStringList groups() const;
%Docstring
Returns a list of the groups the function belongs to.

.. versionadded:: 3.0

.. seealso:: :py:func:`group`
%End

    const QString helpText() const;
%Docstring
The help text for the function.
%End

    virtual QVariant func( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node ) = 0;
%Docstring
Returns result of evaluating the function.

:param values: list of values passed to the function
:param context: context expression is being evaluated against
:param parent: parent expression
:param node: expression node

:return: result of function
%End

    virtual QVariant run( QgsExpressionNode::NodeList *args, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node );

    bool operator==( const QgsExpressionFunction &other ) const;

    virtual bool handlesNull() const;

  protected:

    static bool allParamsStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context );
%Docstring
This will return true if all the params for the provided function ``node`` are static within the
constraints imposed by the ``context`` within the given ``parent``.

This can be used as callback for custom implementations of subclasses. It is the default for implementation
for StaticFunction.isStatic.

.. versionadded:: 3.0
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/expression/qgsexpressionfunction.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
