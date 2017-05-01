/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsexpression.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsExpression
{
%Docstring
Class for parsing and evaluation of expressions (formerly called "search strings").
The expressions try to follow both syntax and semantics of SQL expressions.

Usage:
\code{.cpp}
QgsExpression exp("gid*2 > 10 and type not in ('D','F'));
if (exp.hasParserError())
{
// show error message with parserErrorString() and exit
}
QVariant result = exp.evaluate(feature, fields);
if (exp.hasEvalError())
{
// show error message with evalErrorString()
}
else
{
// examine the result
}
\endcode

Three Value Logic
=================

Similarly to SQL, this class supports three-value logic: true/false/unknown.
Unknown value may be a result of operations with missing data (NULL). Please note
that NULL is different value than zero or an empty string. For example
3 > NULL returns unknown.

There is no special (three-value) 'boolean' type: true/false is represented as
1/0 integer, unknown value is represented the same way as NULL values: invalid QVariant.

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
%Docstring
 Compares two expressions. The operator returns true
 if the expression string is equal.

.. versionadded:: 3.0
 :rtype: bool
%End

    bool isValid() const;
%Docstring
 Checks if this expression is valid.
 A valid expression could be parsed but does not necessarily evaluate properly.

.. versionadded:: 3.0
 :rtype: bool
%End

    bool hasParserError() const;
%Docstring
Returns true if an error occurred when parsing the input expression
 :rtype: bool
%End
    QString parserErrorString() const;
%Docstring
Returns parser error
 :rtype: str
%End


    const QgsExpression::Node *rootNode() const;
%Docstring
Returns root node of the expression. Root node is null is parsing has failed
 :rtype: QgsExpression.Node
%End

    bool prepare( const QgsExpressionContext *context );
%Docstring
 Get the expression ready for evaluation - find out column indexes.
 \param context context for preparing expression
.. versionadded:: 2.12
 :rtype: bool
%End

    QSet<QString> referencedColumns() const;
%Docstring
 Get list of columns referenced by the expression.

.. note::

   If the returned list contains the QgsFeatureRequest.AllAttributes constant then
 all attributes from the layer are required for evaluation of the expression.
 QgsFeatureRequest.setSubsetOfAttributes automatically handles this case.

.. seealso:: referencedAttributeIndexes()
 :rtype: set of str
%End

    QSet<QString> referencedVariables() const;
%Docstring
 Return a list of all variables which are used in this expression.
 If the list contains a NULL QString, there is a variable name used
 which is determined at runtime.

.. versionadded:: 3.0
 :rtype: set of str
%End

    QSet<int> referencedAttributeIndexes( const QgsFields &fields ) const;
%Docstring
 Return a list of field name indexes obtained from the provided fields.

.. versionadded:: 3.0
 :rtype: set of int
%End

    bool needsGeometry() const;
%Docstring
Returns true if the expression uses feature geometry for some computation
 :rtype: bool
%End


    QVariant evaluate();
%Docstring
 Evaluate the feature and return the result.
.. note::

   this method does not expect that prepare() has been called on this instance
.. versionadded:: 2.12
 :rtype: QVariant
%End

    QVariant evaluate( const QgsExpressionContext *context );
%Docstring
 Evaluate the expression against the specified context and return the result.
 \param context context for evaluating expression
.. note::

   prepare() should be called before calling this method.
.. versionadded:: 2.12
 :rtype: QVariant
%End

    bool hasEvalError() const;
%Docstring
Returns true if an error occurred when evaluating last input
 :rtype: bool
%End
    QString evalErrorString() const;
%Docstring
Returns evaluation error
 :rtype: str
%End
    void setEvalErrorString( const QString &str );
%Docstring
Set evaluation error (used internally by evaluation functions)
%End

    bool isField() const;
%Docstring
 Checks whether an expression consists only of a single field reference
.. versionadded:: 2.9
 :rtype: bool
%End

    static bool checkExpression( const QString &text, const QgsExpressionContext *context, QString &errorMessage /Out/ );
%Docstring
 Tests whether a string is a valid expression.
 \param text string to test
 \param context optional expression context
 \param errorMessage will be filled with any error message from the validation
 :return: true if string is a valid expression
.. versionadded:: 2.12
 :rtype: bool
%End

    void setExpression( const QString &expression );
%Docstring
 Set the expression string, will reset the whole internal structure.

.. versionadded:: 3.0
%End

    QString expression() const;
%Docstring
API calls, dump() will be used to create one instead.
 :rtype: str
%End

    QString dump() const;
%Docstring
expression() instead.
 :rtype: str
%End

    QgsDistanceArea *geomCalculator();
%Docstring
 Return calculator used for distance and area calculations
 (used by $length, $area and $perimeter functions only)
.. seealso:: setGeomCalculator()
.. seealso:: distanceUnits()
.. seealso:: areaUnits()
 :rtype: QgsDistanceArea
%End

    void setGeomCalculator( const QgsDistanceArea *calc );
%Docstring
 Sets the geometry calculator used for distance and area calculations in expressions.
 (used by $length, $area and $perimeter functions only). By default, no geometry
 calculator is set and all distance and area calculations are performed using simple
 cartesian methods (ie no ellipsoidal calculations).
 \param calc geometry calculator. Ownership is not transferred. Set to a None to force
 cartesian calculations.
.. seealso:: geomCalculator()
%End

    QgsUnitTypes::DistanceUnit distanceUnits() const;
%Docstring
 Returns the desired distance units for calculations involving geomCalculator(), e.g., "$length" and "$perimeter".
.. note::

   distances are only converted when a geomCalculator() has been set
.. versionadded:: 2.14
.. seealso:: setDistanceUnits()
.. seealso:: areaUnits()
 :rtype: QgsUnitTypes.DistanceUnit
%End

    void setDistanceUnits( QgsUnitTypes::DistanceUnit unit );
%Docstring
 Sets the desired distance units for calculations involving geomCalculator(), e.g., "$length" and "$perimeter".
.. note::

   distances are only converted when a geomCalculator() has been set
.. versionadded:: 2.14
.. seealso:: distanceUnits()
.. seealso:: setAreaUnits()
%End

    QgsUnitTypes::AreaUnit areaUnits() const;
%Docstring
 Returns the desired areal units for calculations involving geomCalculator(), e.g., "$area".
.. note::

   areas are only converted when a geomCalculator() has been set
.. versionadded:: 2.14
.. seealso:: setAreaUnits()
.. seealso:: distanceUnits()
 :rtype: QgsUnitTypes.AreaUnit
%End

    void setAreaUnits( QgsUnitTypes::AreaUnit unit );
%Docstring
 Sets the desired areal units for calculations involving geomCalculator(), e.g., "$area".
.. note::

   areas are only converted when a geomCalculator() has been set
.. versionadded:: 2.14
.. seealso:: areaUnits()
.. seealso:: setDistanceUnits()
%End

    static QString replaceExpressionText( const QString &action, const QgsExpressionContext *context,
                                          const QgsDistanceArea *distanceArea = 0 );
%Docstring
 This function replaces each expression between [% and %]
 in the string with the result of its evaluation with the specified context

 Additional substitutions can be passed through the substitutionMap parameter
 \param action The source string in which placeholders should be replaced.
 \param context Expression context
 \param distanceArea Optional QgsDistanceArea. If specified, the QgsDistanceArea is used for distance
 and area conversion
.. versionadded:: 2.12
 :rtype: str
%End

    static double evaluateToDouble( const QString &text, const double fallbackValue );
%Docstring
 Attempts to evaluate a text string as an expression to a resultant double
 value.
 \param text text to evaluate as expression
 \param fallbackValue value to return if text can not be evaluated as a double
 :return: evaluated double value, or fallback value
.. versionadded:: 2.7
.. note::

   this method is inefficient for bulk evaluation of expressions, it is intended
 for one-off evaluations only.
 :rtype: float
%End

    enum UnaryOperator
    {
      uoNot,
      uoMinus,
    };

    enum BinaryOperator
    {
      // logical
      boOr,
      boAnd,

      // comparison
      boEQ,
      boNE,
      boLE,
      boGE,
      boLT,
      boGT,
      boRegexp,
      boLike,
      boNotLike,
      boILike,
      boNotILike,
      boIs,
      boIsNot,

      // math
      boPlus,
      boMinus,
      boMul,
      boDiv,
      boIntDiv,
      boMod,
      boPow,

      // strings
      boConcat,
    };

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



    class Parameter
{
%Docstring
 Represents a single parameter passed to a function.
.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsexpression.h"
%End
      public:

        Parameter( const QString &name,
                   bool optional = false,
                   const QVariant &defaultValue = QVariant() );
%Docstring
 Constructor for Parameter.
 \param name parameter name, used when named parameter are specified in an expression
 \param optional set to true if parameter should be optional
 \param defaultValue default value to use for optional parameters
%End

        QString name() const;
%Docstring
Returns the name of the parameter.
 :rtype: str
%End

        bool optional() const;
%Docstring
Returns true if the parameter is optional.
 :rtype: bool
%End

        QVariant defaultValue() const;
%Docstring
Returns the default value for the parameter.
 :rtype: QVariant
%End

        bool operator==( const QgsExpression::Parameter &other ) const;
%Docstring
 :rtype: bool
%End

    };

    typedef QList< QgsExpression::Parameter > ParameterList;



    class Function
{
%Docstring
 A abstract base class for defining QgsExpression functions.
%End

%TypeHeaderCode
#include "qgsexpression.h"
%End
      public:

        Function( const QString &fnname,
                  int params,
                  const QString &group,
                  const QString &helpText = QString(),
                  bool lazyEval = false,
                  bool handlesNull = false,
                  bool isContextual = false );
%Docstring
Constructor for function which uses unnamed parameters
%End

        Function( const QString &fnname,
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

        Function( const QString &fnname,
                  const QgsExpression::ParameterList &params,
                  const QString &group,
                  const QString &helpText = QString(),
                  bool lazyEval = false,
                  bool handlesNull = false,
                  bool isContextual = false );
%Docstring
 Constructor for function which uses named parameter list.
.. versionadded:: 2.16
%End

        Function( const QString &fnname,
                  const QgsExpression::ParameterList &params,
                  const QStringList &groups,
                  const QString &helpText = QString(),
                  bool lazyEval = false,
                  bool handlesNull = false,
                  bool isContextual = false );
%Docstring
 Constructor for function which uses named parameter list and group list.
.. versionadded:: 3.0
%End

        virtual ~Function();

        QString name() const;
%Docstring
The name of the function.
 :rtype: str
%End

        int params() const;
%Docstring
The number of parameters this function takes.
 :rtype: int
%End

        int minParams() const;
%Docstring
The minimum number of parameters this function takes.
 :rtype: int
%End

        const QgsExpression::ParameterList &parameters() const;
%Docstring
 Returns the list of named parameters for the function, if set.
.. versionadded:: 2.16
 :rtype: QgsExpression.ParameterList
%End

        virtual bool usesGeometry( const QgsExpression::NodeFunction *node ) const;
%Docstring
Does this function use a geometry object.
 :rtype: bool
%End

        virtual QStringList aliases() const;
%Docstring
 Returns a list of possible aliases for the function. These include
 other permissible names for the function, e.g., deprecated names.
 :return: list of known aliases
.. versionadded:: 2.9
 :rtype: list of str
%End

        bool lazyEval() const;
%Docstring
 True if this function should use lazy evaluation.  Lazy evaluation functions take QgsExpression.Node objects
 rather than the node results when called.  You can use node->eval(parent, feature) to evaluate the node and return the result
 Functions are non lazy default and will be given the node return value when called **
 :rtype: bool
%End

        virtual QSet<QString> referencedColumns( const QgsExpression::NodeFunction *node ) const;
%Docstring
 Returns a set of field names which are required for this function.
 May contain QgsFeatureRequest.AllAttributes to signal that all
 attributes are required.
 If in doubt this will return more fields than strictly required.

.. versionadded:: 3.0
 :rtype: set of str
%End

        bool isContextual() const;
%Docstring
 Returns whether the function is only available if provided by a QgsExpressionContext object.
.. versionadded:: 2.12
 :rtype: bool
%End

        virtual bool isDeprecated() const;
%Docstring
 Returns true if the function is deprecated and should not be presented as a valid option
 to users in expression builders.
.. versionadded:: 3.0
 :rtype: bool
%End

        QString group() const;
%Docstring
 Returns the first group which the function belongs to.
.. note::

   consider using groups() instead, as some functions naturally belong in multiple groups
 :rtype: str
%End

        QStringList groups() const;
%Docstring
 Returns a list of the groups the function belongs to.
.. versionadded:: 3.0
.. seealso:: group()
 :rtype: list of str
%End

        const QString helpText() const;
%Docstring
The help text for the function.
 :rtype: str
%End

        virtual QVariant func( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent ) = 0;
%Docstring
 Returns result of evaluating the function.
 \param values list of values passed to the function
 \param context context expression is being evaluated against
 \param parent parent expression
 :return: result of function
 :rtype: QVariant
%End

        bool operator==( const QgsExpression::Function &other ) const;
%Docstring
 :rtype: bool
%End

        virtual bool handlesNull() const;
%Docstring
 :rtype: bool
%End

    };


    static const QList<QgsExpression::Function *> &Functions();
%Docstring
 :rtype: list of QgsExpression.Function
%End

    static const QStringList &BuiltinFunctions();
%Docstring
 :rtype: list of str
%End

    static bool registerFunction( QgsExpression::Function *function, bool transferOwnership = false );
%Docstring
 Registers a function to the expression engine. This is required to allow expressions to utilize the function.
 \param function function to register
 \param transferOwnership set to true to transfer ownership of function to expression engine
 :return: true on successful registration
.. seealso:: unregisterFunction
 :rtype: bool
%End

    static bool unregisterFunction( const QString &name );
%Docstring
 Unregisters a function from the expression engine. The function will no longer be usable in expressions.
 \param name function name
.. seealso:: registerFunction
 :rtype: bool
%End


    static void cleanRegisteredFunctions();
%Docstring
 Deletes all registered functions whose ownership have been transferred to the expression engine.
.. versionadded:: 2.12
%End

    static bool isFunctionName( const QString &name );
%Docstring
tells whether the identifier is a name of existing function
 :rtype: bool
%End

    static int functionIndex( const QString &name );
%Docstring
return index of the function in Functions array
 :rtype: int
%End

    static int functionCount();
%Docstring
 Returns the number of functions defined in the parser
  :return: The number of function defined in the parser.
 :rtype: int
%End

    static QString quotedColumnRef( QString name );
%Docstring
 Returns a quoted column reference (in double quotes)
.. seealso:: quotedString()
.. seealso:: quotedValue()
 :rtype: str
%End

    static QString quotedString( QString text );
%Docstring
 Returns a quoted version of a string (in single quotes)
.. seealso:: quotedValue()
.. seealso:: quotedColumnRef()
 :rtype: str
%End

    static QString quotedValue( const QVariant &value );
%Docstring
 Returns a string representation of a literal value, including appropriate
 quotations where required.
 \param value value to convert to a string representation
.. versionadded:: 2.14
.. seealso:: quotedString()
.. seealso:: quotedColumnRef()
 :rtype: str
%End

    static QString quotedValue( const QVariant &value, QVariant::Type type );
%Docstring
 Returns a string representation of a literal value, including appropriate
 quotations where required.
 \param value value to convert to a string representation
 \param type value type
.. versionadded:: 2.14
.. seealso:: quotedString()
.. seealso:: quotedColumnRef()
 :rtype: str
%End


    enum NodeType
    {
      ntUnaryOperator,
      ntBinaryOperator,
      ntInOperator,
      ntFunction,
      ntLiteral,
      ntColumnRef,
      ntCondition
    };

    class Node
{
%Docstring

 Abstract base class for all nodes that can appear in an expression.
%End

%TypeHeaderCode
#include "qgsexpression.h"
%End

%ConvertToSubClassCode
        switch ( sipCpp->nodeType() )
        {
          case QgsExpression::ntUnaryOperator:
            sipType = sipType_QgsExpression_NodeUnaryOperator;
            break;
          case QgsExpression::ntBinaryOperator:
            sipType = sipType_QgsExpression_NodeBinaryOperator;
            break;
          case QgsExpression::ntInOperator:
            sipType = sipType_QgsExpression_NodeInOperator;
            break;
          case QgsExpression::ntFunction:
            sipType = sipType_QgsExpression_NodeFunction;
            break;
          case QgsExpression::ntLiteral:
            sipType = sipType_QgsExpression_NodeLiteral;
            break;
          case QgsExpression::ntColumnRef:
            sipType = sipType_QgsExpression_NodeColumnRef;
            break;
          case QgsExpression::ntCondition:
            sipType = sipType_QgsExpression_NodeCondition;
            break;
          default:
            sipType = 0;
            break;
        }
%End
      public:
        virtual ~Node();

        virtual QgsExpression::NodeType nodeType() const = 0;
%Docstring
 Get the type of this node.

 :return: The type of this node
 :rtype: QgsExpression.NodeType
%End

        virtual QString dump() const = 0;
%Docstring
 Dump this node into a serialized (part) of an expression.
 The returned expression does not necessarily literally match
 the original expression, it's just guaranteed to behave the same way.
 :rtype: str
%End

        QVariant eval( QgsExpression *parent, const QgsExpressionContext *context );
%Docstring
 Evaluate this node with the given context and parent.
 This will return a cached value if it has been determined to be static
 during the prepare() execution.

.. versionadded:: 2.12
 :rtype: QVariant
%End

        virtual QgsExpression::Node *clone() const = 0;
%Docstring
 Generate a clone of this node.
 Make sure that the clone does not contain any information which is
 generated in prepare and context related.
 Ownership is transferred to the caller.

 :return: a deep copy of this node.
 :rtype: QgsExpression.Node
%End

        virtual QSet<QString> referencedColumns() const = 0;
%Docstring
 Abstract virtual method which returns a list of columns required to
 evaluate this node.

 When reimplementing this, you need to return any column that is required to
 evaluate this node and in addition recursively collect all the columns required
 to evaluate child nodes.

 :return: A list of columns required to evaluate this expression
 :rtype: set of str
%End

        virtual QSet<QString> referencedVariables() const = 0;
%Docstring
 Return a set of all variables which are used in this expression.
 :rtype: set of str
%End

        virtual bool needsGeometry() const = 0;
%Docstring
 Abstract virtual method which returns if the geometry is required to evaluate
 this expression.

 This needs to call `needsGeometry()` recursively on any child nodes.

 :return: true if a geometry is required to evaluate this expression
 :rtype: bool
%End

        virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const = 0;
%Docstring
 Returns true if this node can be evaluated for a static value. This is used during
 the prepare() step and in case it returns true, the value of this node will already
 be evaluated and the result cached (and therefore not re-evaluated in subsequent calls
 to eval()). In case this returns true, prepareNode() will never be called.

.. versionadded:: 3.0
 :rtype: bool
%End

        bool prepare( QgsExpression *parent, const QgsExpressionContext *context );
%Docstring
 Prepare this node for evaluation.
 This will check if the node content is static and in this case cache the value.
 If it's not static it will call prepareNode() to allow the node to do initialization
 work like for example resolving a column name to an attribute index.

.. versionadded:: 2.12
 :rtype: bool
%End

      private:
        virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) = 0 ;
%Docstring
 Abstract virtual preparation method
 Errors are reported to the parent
.. versionadded:: 3.0
 :rtype: bool
%End
      private:
        virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) = 0 ;
%Docstring
 Abstract virtual eval method
 Errors are reported to the parent
.. versionadded:: 3.0
 :rtype: QVariant
%End
    };

    class NamedNode
{

%TypeHeaderCode
#include "qgsexpression.h"
%End
      public:

        NamedNode( const QString &name, QgsExpression::Node *node );
%Docstring
 Constructor for NamedNode
 \param name node name
 \param node node
%End

        QString name;
%Docstring
Node name
%End

        QgsExpression::Node *node;
%Docstring
Node
%End
    };

    class NodeList
{

%TypeHeaderCode
#include "qgsexpression.h"
%End
      public:
        NodeList();
        virtual ~NodeList();
        void append( QgsExpression::Node *node /Transfer/ );
%Docstring
Takes ownership of the provided node
%End

        void append( QgsExpression::NamedNode *node /Transfer/ );
%Docstring
 Adds a named node. Takes ownership of the provided node.
.. versionadded:: 2.16
%End

        int count() const;
%Docstring
 Returns the number of nodes in the list.
 :rtype: int
%End

        bool hasNamedNodes() const;
%Docstring
.. versionadded:: 2.16
 :rtype: bool
%End

        QList<QgsExpression::Node *> list();
%Docstring
 Get a list of all the nodes.
 :rtype: list of QgsExpression.Node
%End

        QgsExpression::Node *at( int i );
%Docstring
 Get the node at position i in the list.

.. versionadded:: 3.0
 :rtype: QgsExpression.Node
%End

        QStringList names() const;
%Docstring
.. versionadded:: 2.16
 :rtype: list of str
%End

        QgsExpression::NodeList *clone() const;
%Docstring
Creates a deep copy of this list. Ownership is transferred to the caller
 :rtype: QgsExpression.NodeList
%End

        virtual QString dump() const;
%Docstring
 :rtype: str
%End

    };

    class NodeUnaryOperator : QgsExpression::Node
{
%Docstring
 A unary node is either negative as in boolean (not) or as in numbers (minus).
%End

%TypeHeaderCode
#include "qgsexpression.h"
%End
      public:

        NodeUnaryOperator( QgsExpression::UnaryOperator op, QgsExpression::Node *operand /Transfer/ );
%Docstring
 A node unary operator is modifying the value of ``operand`` by negating it with ``op``.
%End
        ~NodeUnaryOperator();

        QgsExpression::UnaryOperator op() const;
%Docstring
 :rtype: QgsExpression.UnaryOperator
%End
        QgsExpression::Node *operand() const;
%Docstring
 :rtype: QgsExpression.Node
%End

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context );
        virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context );
        virtual QString dump() const;

        virtual QSet<QString> referencedColumns() const;
        virtual QSet<QString> referencedVariables() const;
        virtual bool needsGeometry() const;
        virtual QgsExpression::Node *clone() const;

        virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const;

    };

    class NodeBinaryOperator : QgsExpression::Node
{

%TypeHeaderCode
#include "qgsexpression.h"
%End
      public:

        NodeBinaryOperator( QgsExpression::BinaryOperator op, QgsExpression::Node *opLeft /Transfer/, QgsExpression::Node *opRight /Transfer/ );
%Docstring
 Binary combination of the left and the right with op.
%End
        ~NodeBinaryOperator();

        QgsExpression::BinaryOperator op() const;
%Docstring
 :rtype: QgsExpression.BinaryOperator
%End
        QgsExpression::Node *opLeft() const;
%Docstring
 :rtype: QgsExpression.Node
%End
        QgsExpression::Node *opRight() const;
%Docstring
 :rtype: QgsExpression.Node
%End

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context );
        virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context );
        virtual QString dump() const;

        virtual QSet<QString> referencedColumns() const;
        virtual QSet<QString> referencedVariables() const;
        virtual bool needsGeometry() const;
        virtual QgsExpression::Node *clone() const;
        virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const;

        int precedence() const;
%Docstring
 :rtype: int
%End
        bool leftAssociative() const;
%Docstring
 :rtype: bool
%End

    };

    class NodeInOperator : QgsExpression::Node
{

%TypeHeaderCode
#include "qgsexpression.h"
%End
      public:

        NodeInOperator( QgsExpression::Node *node /Transfer/, QgsExpression::NodeList *list /Transfer/, bool notin = false );
%Docstring
 This node tests if the result of ``node`` is in the result of ``list``. Optionally it can be inverted with ``notin`` which by default is false.
%End
        virtual ~NodeInOperator();

        QgsExpression::Node *node() const;
%Docstring
 :rtype: QgsExpression.Node
%End
        bool isNotIn() const;
%Docstring
 :rtype: bool
%End
        QgsExpression::NodeList *list() const;
%Docstring
 :rtype: QgsExpression.NodeList
%End

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context );
        virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context );
        virtual QString dump() const;

        virtual QSet<QString> referencedColumns() const;
        virtual QSet<QString> referencedVariables() const;
        virtual bool needsGeometry() const;
        virtual QgsExpression::Node *clone() const;
        virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const;

    };

    class NodeFunction : QgsExpression::Node
{

%TypeHeaderCode
#include "qgsexpression.h"
%End
      public:

        NodeFunction( int fnIndex, QgsExpression::NodeList *args /Transfer/ );
%Docstring
 A function node consists of an index of the function in the global function array and
 a list of arguments that will be passed to it.
%End

        virtual ~NodeFunction();

        int fnIndex() const;
%Docstring
 :rtype: int
%End
        QgsExpression::NodeList *args() const;
%Docstring
 :rtype: QgsExpression.NodeList
%End

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context );
        virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context );
        virtual QString dump() const;

        virtual QSet<QString> referencedColumns() const;
        virtual QSet<QString> referencedVariables() const;
        virtual bool needsGeometry() const;
        virtual QgsExpression::Node *clone() const;
        virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const;

        static bool validateParams( int fnIndex, QgsExpression::NodeList *args, QString &error );
%Docstring
Tests whether the provided argument list is valid for the matching function
 :rtype: bool
%End

    };

    class NodeLiteral : QgsExpression::Node
{

%TypeHeaderCode
#include "qgsexpression.h"
%End
      public:
        NodeLiteral( const QVariant &value );

        QVariant value() const;
%Docstring
The value of the literal.
 :rtype: QVariant
%End

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context );
        virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context );
        virtual QString dump() const;

        virtual QSet<QString> referencedColumns() const;
        virtual QSet<QString> referencedVariables() const;
        virtual bool needsGeometry() const;
        virtual QgsExpression::Node *clone() const;
        virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const;

    };

    class NodeColumnRef : QgsExpression::Node
{

%TypeHeaderCode
#include "qgsexpression.h"
%End
      public:
        NodeColumnRef( const QString &name );

        QString name() const;
%Docstring
The name of the column.
 :rtype: str
%End

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context );
        virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context );
        virtual QString dump() const;

        virtual QSet<QString> referencedColumns() const;
        virtual QSet<QString> referencedVariables() const;
        virtual bool needsGeometry() const;

        virtual QgsExpression::Node *clone() const;
        virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const;

    };


    class WhenThen
{

%TypeHeaderCode
#include "qgsexpression.h"
%End
      public:

        WhenThen( QgsExpression::Node *whenExp, QgsExpression::Node *thenExp );
%Docstring
 A combination of when and then. Simple as that.
%End
        ~WhenThen();


        QgsExpression::WhenThen *clone() const;
%Docstring
 Get a deep copy of this WhenThen combination.
 :rtype: QgsExpression.WhenThen
%End

      private:
        WhenThen( const QgsExpression::WhenThen &rh );
    };
    typedef QList<QgsExpression::WhenThen *> WhenThenList;

    class NodeCondition : QgsExpression::Node
{

%TypeHeaderCode
#include "qgsexpression.h"
%End
      public:

        NodeCondition( QgsExpression::WhenThenList *conditions, QgsExpression::Node *elseExp = 0 );
%Docstring
 Create a new node with the given list of ``conditions`` and an optional ``elseExp`` expression.
%End


        ~NodeCondition();

        virtual QgsExpression::NodeType nodeType() const;
        virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context );
        virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context );
        virtual QString dump() const;

        virtual QSet<QString> referencedColumns() const;
        virtual QSet<QString> referencedVariables() const;
        virtual bool needsGeometry() const;
        virtual QgsExpression::Node *clone() const;
        virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const;

    };

    static QString helpText( QString name );
%Docstring
 Returns the help text for a specified function.
 \param name function name
.. seealso:: variableHelpText()
 :rtype: str
%End

    static QString variableHelpText( const QString &variableName, bool showValue = true, const QVariant &value = QVariant() );
%Docstring
 Returns the help text for a specified variable.
 \param variableName name of variable
 \param showValue set to true to include current value of variable in help text
 \param value current value of variable to show in help text
.. seealso:: helpText()
.. versionadded:: 2.12
 :rtype: str
%End

    static QString group( const QString &group );
%Docstring
 Returns the translated name for a function group.
 \param group untranslated group name
 :rtype: str
%End

    static QString formatPreviewString( const QVariant &value );
%Docstring
 Formats an expression result for friendly display to the user. Truncates the result to a sensible
 length, and presents text representations of non numeric/text types (e.g., geometries and features).
 \param value expression result to format
 :return: formatted string, may contain HTML formatting characters
.. versionadded:: 2.14
 :rtype: str
%End

  protected:
    void initGeomCalculator();









};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsexpression.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
