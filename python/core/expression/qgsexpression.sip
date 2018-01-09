/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/expression/qgsexpression.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsExpression
{
%Docstring
Class for parsing and evaluation of expressions (formerly called "search strings").
The expressions try to follow both syntax and semantics of SQL expressions.

Usage:

Three Value Logic
=================

Similarly to SQL, this class supports three-value logic: true/false/unknown.
Unknown value may be a result of operations with missing data (NULL). Please note
that NULL is different value than zero or an empty string. For example
3 > NULL returns unknown.

There is no special (three-value) 'boolean' type: true/false is represented as
1/0 integer, unknown value is represented the same way as NULL values: NULL QVariant.

Performance
===========

For better performance with many evaluations you may first call prepare(fields) function
to find out indices of columns and then repeatedly call evaluate(feature).

Type conversion
===============

Operators and functions that expect arguments to be of a particular
type automatically convert the arguments to that type, e.g. sin('2.1') will convert
the argument to a double, length(123) will first convert the number to a string.
Explicit conversion can be achieved with to_int, to_real, to_string functions.
If implicit or explicit conversion is invalid, the evaluation returns an error.
Comparison operators do numeric comparison in case both operators are numeric (int/double)
or they can be converted to numeric types.

Implicit sharing
================

This class is implicitly shared, copying has a very low overhead.
It is normally preferable to call `QgsExpression( otherExpression )` instead of
`QgsExpression( otherExpression.expression() )`. A deep copy will only be made
when prepare() is called. For usage this means mainly, that you should
normally keep an unprepared master copy of a QgsExpression and whenever using it
with a particular QgsFeatureIterator copy it just before and prepare it using the
same context as the iterator.

Implicit sharing was added in 2.14
%End

%TypeHeaderCode
#include "qgsexpression.h"
%End
  public:

    QgsExpression( const QString &expr );
%Docstring
Creates a new expression based on the provided string.
The string will immediately be parsed. For optimization
prepare() should always be called before every
loop in which this expression is used.
%End

    QgsExpression( const QgsExpression &other );
%Docstring
Create a copy of this expression. This is preferred
over recreating an expression from a string since
it does not need to be re-parsed.
%End



    QgsExpression();
%Docstring
Create an empty expression.

.. versionadded:: 3.0
%End

    ~QgsExpression();

    bool operator==( const QgsExpression &other ) const;

    bool isValid() const;
%Docstring
Checks if this expression is valid.
A valid expression could be parsed but does not necessarily evaluate properly.

.. versionadded:: 3.0
%End

    bool hasParserError() const;
%Docstring
Returns true if an error occurred when parsing the input expression
%End
    QString parserErrorString() const;
%Docstring
Returns parser error
%End

    const QgsExpressionNode *rootNode() const;
%Docstring
Returns root node of the expression. Root node is null is parsing has failed
%End

    bool prepare( const QgsExpressionContext *context );
%Docstring
Get the expression ready for evaluation - find out column indexes.

:param context: context for preparing expression

.. versionadded:: 2.12
%End

    QSet<QString> referencedColumns() const;
%Docstring
Get list of columns referenced by the expression.

.. note::

   If the returned list contains the QgsFeatureRequest.AllAttributes constant then
all attributes from the layer are required for evaluation of the expression.
QgsFeatureRequest.setSubsetOfAttributes automatically handles this case.

.. seealso:: :py:func:`referencedAttributeIndexes`
%End

    QSet<QString> referencedVariables() const;
%Docstring
Return a list of all variables which are used in this expression.
If the list contains a NULL QString, there is a variable name used
which is determined at runtime.

.. versionadded:: 3.0
%End

    QSet<int> referencedAttributeIndexes( const QgsFields &fields ) const;
%Docstring
Return a list of field name indexes obtained from the provided fields.

.. versionadded:: 3.0
%End

    bool needsGeometry() const;
%Docstring
Returns true if the expression uses feature geometry for some computation
%End


    QVariant evaluate();
%Docstring
Evaluate the feature and return the result.

.. note::

   this method does not expect that prepare() has been called on this instance

.. versionadded:: 2.12
%End

    QVariant evaluate( const QgsExpressionContext *context );
%Docstring
Evaluate the expression against the specified context and return the result.

:param context: context for evaluating expression

.. note::

   prepare() should be called before calling this method.

.. versionadded:: 2.12
%End

    bool hasEvalError() const;
%Docstring
Returns true if an error occurred when evaluating last input
%End
    QString evalErrorString() const;
%Docstring
Returns evaluation error
%End
    void setEvalErrorString( const QString &str );
%Docstring
Set evaluation error (used internally by evaluation functions)
%End

    bool isField() const;
%Docstring
Checks whether an expression consists only of a single field reference

.. versionadded:: 2.9
%End

    static bool checkExpression( const QString &text, const QgsExpressionContext *context, QString &errorMessage /Out/ );
%Docstring
Tests whether a string is a valid expression.

:param text: string to test
:param context: optional expression context
:param errorMessage: will be filled with any error message from the validation

:return: true if string is a valid expression

.. versionadded:: 2.12
%End

    void setExpression( const QString &expression );
%Docstring
Set the expression string, will reset the whole internal structure.

.. versionadded:: 3.0
%End

    QString expression() const;
%Docstring
Return the original, unmodified expression string.
If there was none supplied because it was constructed by sole
API calls, dump() will be used to create one instead.
%End

    QString dump() const;
%Docstring
Return an expression string, constructed from the internal
abstract syntax tree. This does not contain any nice whitespace
formatting or comments. In general it is preferable to use
expression() instead.
%End

    QgsDistanceArea *geomCalculator();
%Docstring
Return calculator used for distance and area calculations
(used by $length, $area and $perimeter functions only)

.. seealso:: :py:func:`setGeomCalculator`

.. seealso:: :py:func:`distanceUnits`

.. seealso:: :py:func:`areaUnits`
%End

    void setGeomCalculator( const QgsDistanceArea *calc );
%Docstring
Sets the geometry calculator used for distance and area calculations in expressions.
(used by $length, $area and $perimeter functions only). By default, no geometry
calculator is set and all distance and area calculations are performed using simple
Cartesian methods (ie no ellipsoidal calculations).

:param calc: geometry calculator. Ownership is not transferred. Set to a None to force
Cartesian calculations.

.. seealso:: :py:func:`geomCalculator`
%End

    QgsUnitTypes::DistanceUnit distanceUnits() const;
%Docstring
Returns the desired distance units for calculations involving geomCalculator(), e.g., "$length" and "$perimeter".

.. note::

   distances are only converted when a geomCalculator() has been set

.. versionadded:: 2.14

.. seealso:: :py:func:`setDistanceUnits`

.. seealso:: :py:func:`areaUnits`
%End

    void setDistanceUnits( QgsUnitTypes::DistanceUnit unit );
%Docstring
Sets the desired distance units for calculations involving geomCalculator(), e.g., "$length" and "$perimeter".

.. note::

   distances are only converted when a geomCalculator() has been set

.. versionadded:: 2.14

.. seealso:: :py:func:`distanceUnits`

.. seealso:: :py:func:`setAreaUnits`
%End

    QgsUnitTypes::AreaUnit areaUnits() const;
%Docstring
Returns the desired areal units for calculations involving geomCalculator(), e.g., "$area".

.. note::

   areas are only converted when a geomCalculator() has been set

.. versionadded:: 2.14

.. seealso:: :py:func:`setAreaUnits`

.. seealso:: :py:func:`distanceUnits`
%End

    void setAreaUnits( QgsUnitTypes::AreaUnit unit );
%Docstring
Sets the desired areal units for calculations involving geomCalculator(), e.g., "$area".

.. note::

   areas are only converted when a geomCalculator() has been set

.. versionadded:: 2.14

.. seealso:: :py:func:`areaUnits`

.. seealso:: :py:func:`setDistanceUnits`
%End

    static QString replaceExpressionText( const QString &action, const QgsExpressionContext *context,
                                          const QgsDistanceArea *distanceArea = 0 );
%Docstring
This function replaces each expression between [% and %]
in the string with the result of its evaluation with the specified context

Additional substitutions can be passed through the substitutionMap parameter

:param action: The source string in which placeholders should be replaced.
:param context: Expression context
:param distanceArea: Optional :py:class:`QgsDistanceArea`. If specified, the QgsDistanceArea is used for distance
and area conversion

.. versionadded:: 2.12
%End

    static double evaluateToDouble( const QString &text, const double fallbackValue );
%Docstring
Attempts to evaluate a text string as an expression to a resultant double
value.

:param text: text to evaluate as expression
:param fallbackValue: value to return if text can not be evaluated as a double

:return: evaluated double value, or fallback value

.. versionadded:: 2.7

.. note::

   this method is inefficient for bulk evaluation of expressions, it is intended
for one-off evaluations only.
%End

    enum SpatialOperator
    {
      soBbox,
      soIntersects,
      soContains,
      soCrosses,
      soEquals,
      soDisjoint,
      soOverlaps,
      soTouches,
      soWithin,
    };

    static const QList<QgsExpressionFunction *> &Functions();

    static const QStringList &BuiltinFunctions();

    static bool registerFunction( QgsExpressionFunction *function, bool transferOwnership = false );
%Docstring
Registers a function to the expression engine. This is required to allow expressions to utilize the function.

:param function: function to register
:param transferOwnership: set to true to transfer ownership of function to expression engine

:return: true on successful registration

.. seealso:: :py:func:`unregisterFunction`
%End

    static bool unregisterFunction( const QString &name );
%Docstring
Unregisters a function from the expression engine. The function will no longer be usable in expressions.

:param name: function name

.. seealso:: :py:func:`registerFunction`
%End


    static void cleanRegisteredFunctions();
%Docstring
Deletes all registered functions whose ownership have been transferred to the expression engine.

.. versionadded:: 2.12
%End

    static bool isFunctionName( const QString &name );
%Docstring
tells whether the identifier is a name of existing function
%End

    static int functionIndex( const QString &name );
%Docstring
return index of the function in Functions array
%End

    static int functionCount();
%Docstring
Returns the number of functions defined in the parser

:return: The number of function defined in the parser.
%End

    static QString quotedColumnRef( QString name );
%Docstring
Returns a quoted column reference (in double quotes)

.. seealso:: :py:func:`quotedString`

.. seealso:: :py:func:`quotedValue`
%End

    static QString quotedString( QString text );
%Docstring
Returns a quoted version of a string (in single quotes)

.. seealso:: :py:func:`quotedValue`

.. seealso:: :py:func:`quotedColumnRef`
%End

    static QString quotedValue( const QVariant &value );
%Docstring
Returns a string representation of a literal value, including appropriate
quotations where required.

:param value: value to convert to a string representation

.. versionadded:: 2.14

.. seealso:: :py:func:`quotedString`

.. seealso:: :py:func:`quotedColumnRef`
%End

    static QString quotedValue( const QVariant &value, QVariant::Type type );
%Docstring
Returns a string representation of a literal value, including appropriate
quotations where required.

:param value: value to convert to a string representation
:param type: value type

.. versionadded:: 2.14

.. seealso:: :py:func:`quotedString`

.. seealso:: :py:func:`quotedColumnRef`
%End


    static QString helpText( QString name );
%Docstring
Returns the help text for a specified function.

:param name: function name

.. seealso:: :py:func:`variableHelpText`

.. seealso:: :py:func:`formatVariableHelp`
%End

    static QString variableHelpText( const QString &variableName );
%Docstring
Returns the help text for a specified variable.

:param variableName: name of variable
:param showValue: set to true to include current value of variable in help text
:param value: current value of variable to show in help text

.. seealso:: :py:func:`helpText`

.. versionadded:: 2.12
%End

    static QString formatVariableHelp( const QString &description, bool showValue = true, const QVariant &value = QVariant() );
%Docstring
Returns formatted help text for a variable.

:param description: translated description of variable
:param showValue: set to true to include current value of variable in help text
:param value: current value of variable to show in help text

.. seealso:: :py:func:`helpText`

.. seealso:: :py:func:`variableHelpText`

.. versionadded:: 3.0
%End

    static QString group( const QString &group );
%Docstring
Returns the translated name for a function group.

:param group: untranslated group name
%End

    static QString formatPreviewString( const QVariant &value );
%Docstring
Formats an expression result for friendly display to the user. Truncates the result to a sensible
length, and presents text representations of non numeric/text types (e.g., geometries and features).

:param value: expression result to format

:return: formatted string, may contain HTML formatting characters

.. versionadded:: 2.14
%End

    static QString createFieldEqualityExpression( const QString &fieldName, const QVariant &value );
%Docstring
Create an expression allowing to evaluate if a field is equal to a
value. The value may be null.

:param fieldName: the name of the field
:param value: the value of the field

:return: the expression to evaluate field equality

.. versionadded:: 3.0
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/expression/qgsexpression.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
