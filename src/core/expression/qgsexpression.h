/***************************************************************************
                               qgsexpression.h
                             -------------------
    begin                : August 2011
    copyright            : (C) 2011 Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXPRESSION_H
#define QGSEXPRESSION_H

#include "qgis_core.h"
#include <QMetaType>
#include <QStringList>
#include <QVariant>
#include <QList>
#include <QDomDocument>
#include <QCoreApplication>
#include <QSet>
#include <functional>

#include "qgis.h"
#include "qgsunittypes.h"
#include "qgsinterval.h"
#include "qgsexpressionnode.h"

class QgsFeature;
class QgsGeometry;
class QgsOgcUtils;
class QgsVectorLayer;
class QgsVectorDataProvider;
class QgsField;
class QgsFields;
class QgsDistanceArea;
class QDomElement;
class QgsExpressionContext;
class QgsExpressionPrivate;
class QgsExpressionFunction;

/**
 * \ingroup core
 * \brief Class for parsing and evaluation of expressions (formerly called "search strings").
 * The expressions try to follow both syntax and semantics of SQL expressions.
 *
 * Usage:
 * \code{.py}
 *   exp = QgsExpression("gid*2 > 10 and type not in ('D','F')")
 *   if exp.hasParserError():
 *       # show error message with parserErrorString() and exit
 *
 *   result = exp.evaluate(feature, fields)
 *   if exp.hasEvalError():
 *       # show error message with evalErrorString()
 *   else:
 *       # examine the result
 * \endcode
 *
 * \section value_logic Three Value Logic
 *
 * Similarly to SQL, this class supports three-value logic: true/false/unknown.
 * Unknown value may be a result of operations with missing data (NULL). Please note
 * that NULL is different value than zero or an empty string. For example
 * 3 > NULL returns unknown.
 *
 * There is no special (three-value) 'boolean' type: true/false is represented as
 * 1/0 integer, unknown value is represented the same way as NULL values: NULL QVariant.
 *
 * \section performance Performance
 *
 * For better performance with many evaluations you may first call prepare(fields) function
 * to find out indices of columns and then repeatedly call evaluate(feature).
 *
 * \section type_conversion Type conversion
 *
 * Operators and functions that expect arguments to be of a particular
 * type automatically convert the arguments to that type, e.g. sin('2.1') will convert
 * the argument to a double, length(123) will first convert the number to a string.
 * Explicit conversion can be achieved with to_int, to_real, to_string functions.
 * If implicit or explicit conversion is invalid, the evaluation returns an error.
 * Comparison operators do numeric comparison in case both operators are numeric (int/double)
 * or they can be converted to numeric types.
 *
 * \section implicit_sharing Implicit sharing
 *
 * This class is implicitly shared, copying has a very low overhead.
 * It is normally preferable to call `QgsExpression( otherExpression )` instead of
 * `QgsExpression( otherExpression.expression() )`. A deep copy will only be made
 * when prepare() is called. For usage this means mainly, that you should
 * normally keep an unprepared master copy of a QgsExpression and whenever using it
 * with a particular QgsFeatureIterator copy it just before and prepare it using the
 * same context as the iterator.
 *
 * Implicit sharing was added in 2.14
*/
class CORE_EXPORT QgsExpression
{
    Q_DECLARE_TR_FUNCTIONS( QgsExpression )
  public:

    /**
     * Details about any parser errors that were found when parsing the expression.
     * \since QGIS 3.0
     */
    struct CORE_EXPORT ParserError
    {
      enum ParserErrorType
      {
        Unknown = 0,  //!< Unknown error type.
        FunctionUnknown = 1, //!< Function was unknown.
        FunctionWrongArgs = 2, //!< Function was called with the wrong number of args.
        FunctionInvalidParams = 3, //!< Function was called with invalid args.
        FunctionNamedArgsError = 4 //!< Non named function arg used after named arg.
      };

      /**
       * The type of parser error that was found.
       */
      ParserErrorType errorType = ParserErrorType::Unknown;

      /**
       * The message for the error at this location.
       */
      QString errorMsg;

      /**
       * The first line that contained the error in the parser.
       * Depending on the error sometimes this doesn't mean anything.
       */
      int firstLine = 0;

      /**
       * The first column that contained the error in the parser.
       * Depending on the error sometimes this doesn't mean anything.
       */
      int firstColumn = 0;

      /**
       * The last line that contained the error in the parser.
       */
      int lastLine = 0;

      /**
       * The last column that contained the error in the parser.
       */
      int lastColumn = 0;
    };

    /**
     * Creates a new expression based on the provided string.
     * The string will immediately be parsed. For optimization
     * prepare() should always be called before every
     * loop in which this expression is used.
     */
    QgsExpression( const QString &expr );

    /**
     * Create a copy of this expression. This is preferred
     * over recreating an expression from a string since
     * it does not need to be re-parsed.
     */
    QgsExpression( const QgsExpression &other );

    /**
     * Create a copy of this expression. This is preferred
     * over recreating an expression from a string since
     * it does not need to be re-parsed.
     */
    QgsExpression &operator=( const QgsExpression &other );

    /**
     * Automatically convert this expression to a string where requested.
     *
     * \since QGIS 3.0
     */
    operator QString() const SIP_SKIP;

    /**
     * Create an empty expression.
     *
     * \since QGIS 3.0
     */
    QgsExpression();

    ~QgsExpression();

    /**
     * Compares two expressions. The operator returns TRUE
     * if the expression string is equal.
     *
     * \since QGIS 3.0
     */
    bool operator==( const QgsExpression &other ) const;

    /**
     * Checks if this expression is valid.
     * A valid expression could be parsed but does not necessarily evaluate properly.
     *
     * \since QGIS 3.0
     */
    bool isValid() const;

    //! Returns TRUE if an error occurred when parsing the input expression
    bool hasParserError() const;
    //! Returns parser error
    QString parserErrorString() const;

    /**
     * Returns parser error details including location of error.
     * \since QGIS 3.0
     */
    QList<QgsExpression::ParserError> parserErrors() const;

    /**
     * Returns the root node of the expression.
     *
     * The root node is NULLPTR if parsing has failed.
     */
    const QgsExpressionNode *rootNode() const;

    /**
     * Gets the expression ready for evaluation - find out column indexes.
     * \param context context for preparing expression
     * \since QGIS 2.12
     */
    bool prepare( const QgsExpressionContext *context );

    /**
     * Gets list of columns referenced by the expression.
     *
     * \note If the returned list contains the QgsFeatureRequest::AllAttributes constant then
     * all attributes from the layer are required for evaluation of the expression.
     * QgsFeatureRequest::setSubsetOfAttributes automatically handles this case.
     *
     * \warning If the expression has been prepared via a call to QgsExpression::prepare(),
     * or a call to QgsExpressionNode::prepare() for a node has been made, then parts of
     * the expression may have been determined to evaluate to a static pre-calculatable value.
     * In this case the results will omit attribute indices which are used by these
     * pre-calculated nodes, regardless of their actual referenced columns.
     * If you are seeking to use these functions to introspect an expression you must
     * take care to do this with an unprepared expression.
     *
     * \see referencedAttributeIndexes()
     */
    QSet<QString> referencedColumns() const;

    /**
     * Returns a list of all variables which are used in this expression.
     * If the list contains a NULL QString, there is a variable name used
     * which is determined at runtime.
     *
     * \note In contrast to the referencedColumns() function this method
     * is not affected by any previous calls to QgsExpression::prepare(),
     * or QgsExpressionNode::prepare().
     *
     * \since QGIS 3.0
     */
    QSet<QString> referencedVariables() const;

    /**
     * Returns a list of the names of all functions which are used in this expression.
     *
     * \note In contrast to the referencedColumns() function this method
     * is not affected by any previous calls to QgsExpression::prepare(),
     * or QgsExpressionNode::prepare().
     *
     * \since QGIS 3.2
     */
    QSet<QString> referencedFunctions() const;

#ifndef SIP_RUN

    /**
     * Returns a list of all nodes which are used in this expression
     *
     * \note not available in Python bindings
     * \since QGIS 3.2
     */
    QList<const QgsExpressionNode *> nodes( ) const;

    /**
     * Returns a list of all nodes of the given class which are used in this expression
     *
     * \note not available in Python bindings
     * \since QGIS 3.2
     */
    template <class T>
    QList<const T *> findNodes( ) const
    {
      QList<const T *> lst;
      const QList<const QgsExpressionNode *> allNodes( nodes() );
      for ( const auto &node : allNodes )
      {
        const T *n = dynamic_cast<const T *>( node );
        if ( n )
          lst << n;
      }
      return lst;
    }
#endif

    /**
     * Returns a list of field name indexes obtained from the provided fields.
     *
     * \warning If the expression has been prepared via a call to QgsExpression::prepare(),
     * or a call to QgsExpressionNode::prepare() for a node has been made, then parts of
     * the expression may have been determined to evaluate to a static pre-calculatable value.
     * In this case the results will omit attribute indices which are used by these
     * pre-calculated nodes, regardless of their actual referenced columns.
     * If you are seeking to use these functions to introspect an expression you must
     * take care to do this with an unprepared expression.
     *
     * \since QGIS 3.0
     */
    QSet<int> referencedAttributeIndexes( const QgsFields &fields ) const;

    //! Returns TRUE if the expression uses feature geometry for some computation
    bool needsGeometry() const;

    // evaluation

    /**
     * Evaluate the feature and return the result.
     * \note this method does not expect that prepare() has been called on this instance
     * \since QGIS 2.12
     */
    QVariant evaluate();

    /**
     * Evaluate the expression against the specified context and return the result.
     * \param context context for evaluating expression
     * \note prepare() should be called before calling this method.
     * \since QGIS 2.12
     */
    QVariant evaluate( const QgsExpressionContext *context );

    //! Returns TRUE if an error occurred when evaluating last input
    bool hasEvalError() const;
    //! Returns evaluation error
    QString evalErrorString() const;
    //! Sets evaluation error (used internally by evaluation functions)
    void setEvalErrorString( const QString &str );

    /**
     * Checks whether an expression consists only of a single field reference
     *
     * \see expressionToLayerFieldIndex()
     * \since QGIS 2.9
     */
    bool isField() const;

    /**
     * Attempts to resolve an expression to a field index from the given \a layer.
     *
     * Given a string which may either directly match a field name from a layer, OR may
     * be an expression which consists only of a single field reference for that layer, this
     * method will return the corresponding field index.
     *
     * \returns field index if found, or -1 if \a expression does not represent a field from the layer.
     *
     * \see isField()
     * \since QGIS 3.22
     */
    static int expressionToLayerFieldIndex( const QString &expression, const QgsVectorLayer *layer );

    /**
     * Validate if the expression is a field in the \a layer and ensure it is quoted.
     *
     * Given a string which may either directly match a field name from a layer, OR may
     * be an expression which consists only of a single field reference for that layer, this
     * method will return the quoted field.
     *
     * \returns the \a expression if not a field or quotes are not required, otherwise returns a quoted field.
     *
     * \see expressionToLayerFieldIndex()
     * \since QGIS 3.24
     */
    static QString quoteFieldExpression( const QString &expression, const QgsVectorLayer *layer );

    /**
     * Tests whether a string is a valid expression.
     * \param text string to test
     * \param context optional expression context
     * \param errorMessage will be filled with any error message from the validation
     * \returns TRUE if string is a valid expression
     * \since QGIS 2.12
     */
    static bool checkExpression( const QString &text, const QgsExpressionContext *context, QString &errorMessage SIP_OUT );

    /**
     * Set the expression string, will reset the whole internal structure.
     *
     * \since QGIS 3.0
     */
    void setExpression( const QString &expression );

    /**
     * Returns the original, unmodified expression string.
     * If there was none supplied because it was constructed by sole
     * API calls, dump() will be used to create one instead.
     */
    QString expression() const;

    /**
     * Returns an expression string, constructed from the internal
     * abstract syntax tree. This does not contain any nice whitespace
     * formatting or comments. In general it is preferable to use
     * expression() instead.
     */
    QString dump() const;

    /**
     * Returns calculator used for distance and area calculations
     * (used by $length, $area and $perimeter functions only)
     * \see setGeomCalculator()
     * \see distanceUnits()
     * \see areaUnits()
     */
    QgsDistanceArea *geomCalculator();

    /**
     * Sets the geometry calculator used for distance and area calculations in expressions.
     * (used by $length, $area and $perimeter functions only).
     * If the geometry calculator is set to NULLPTR (default), prepare() will read variables
     * from the expression context ("project_ellipsoid", "_project_transform_context" and
     * "_layer_crs") to build a geometry calculator.
     * If these variables does not exist and if setGeomCalculator() is not called,
     * all distance and area calculations are performed using simple
     * Cartesian methods (ie no ellipsoidal calculations).
     * \param calc geometry calculator. Ownership is not transferred. Set to NULLPTR to force
     * Cartesian calculations.
     * \see geomCalculator()
     */
    void setGeomCalculator( const QgsDistanceArea *calc );

    /**
     * Returns the desired distance units for calculations involving geomCalculator(), e.g., "$length" and "$perimeter".
     * \note distances are only converted when a geomCalculator() has been set
     * \see setDistanceUnits()
     * \see areaUnits()
     * \since QGIS 2.14
     */
    QgsUnitTypes::DistanceUnit distanceUnits() const;

    /**
     * Sets the desired distance units for calculations involving geomCalculator(), e.g., "$length" and "$perimeter".
     * If distance units are set to QgsUnitTypes::DistanceUnknownUnit (default), prepare() will read
     * variables from the expression context ("project_distance_units") to determine distance units.
     * \note distances are only converted when a geomCalculator() has been set
     * \see distanceUnits()
     * \see setAreaUnits()
     * \since QGIS 2.14
     */
    void setDistanceUnits( QgsUnitTypes::DistanceUnit unit );

    /**
     * Returns the desired areal units for calculations involving geomCalculator(), e.g., "$area".
     * \note areas are only converted when a geomCalculator() has been set
     * \see setAreaUnits()
     * \see distanceUnits()
     * \since QGIS 2.14
     */
    QgsUnitTypes::AreaUnit areaUnits() const;

    /**
     * Sets the desired areal units for calculations involving geomCalculator(), e.g., "$area".
     * If distance units are set to QgsUnitTypes::AreaUnknownUnit (default), prepare() will read
     * variables from the expression context ("project_distance_units") to determine distance units.
     * \note areas are only converted when a geomCalculator() has been set
     * \see areaUnits()
     * \see setDistanceUnits()
     * \since QGIS 2.14
     */
    void setAreaUnits( QgsUnitTypes::AreaUnit unit );

    /**
     * This function replaces each expression between [% and %]
     * in the string with the result of its evaluation with the specified context
     *
     * Additional substitutions can be passed through the substitutionMap parameter
     * \param action The source string in which placeholders should be replaced.
     * \param context Expression context
     * \param distanceArea Optional QgsDistanceArea. If specified, the QgsDistanceArea is used for distance
     * and area conversion
     * \since QGIS 2.12
     */
    static QString replaceExpressionText( const QString &action, const QgsExpressionContext *context,
                                          const QgsDistanceArea *distanceArea = nullptr );

    /**
     * This function returns variables in each expression between [% and %].
     *
     * \param text The source string in which variables should be searched.
     *
     * \since QGIS 3.2
     */
    static QSet<QString> referencedVariables( const QString &text );

    /**
     * Attempts to evaluate a text string as an expression to a resultant double
     * value.
     * \param text text to evaluate as expression
     * \param fallbackValue value to return if text can not be evaluated as a double
     * \returns evaluated double value, or fallback value
     * \note this method is inefficient for bulk evaluation of expressions, it is intended
     * for one-off evaluations only.
     * \since QGIS 2.7
     */
    static double evaluateToDouble( const QString &text, double fallbackValue );

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

    /**
     * Registers a function to the expression engine. This is required to allow expressions to utilize the function.
     * \param function function to register
     * \param transferOwnership set to TRUE to transfer ownership of function to expression engine
     * \returns TRUE on successful registration
     * \see unregisterFunction
     */
    static bool registerFunction( QgsExpressionFunction *function, bool transferOwnership = false );

    /**
     * Unregisters a function from the expression engine. The function will no longer be usable in expressions.
     * \param name function name
     * \see registerFunction
     */
    static bool unregisterFunction( const QString &name );

    /**
     * Deletes all registered functions whose ownership have been transferred to the expression engine.
     * \since QGIS 2.12
     */
    static void cleanRegisteredFunctions();

    //! tells whether the identifier is a name of existing function
    static bool isFunctionName( const QString &name );

    //! Returns index of the function in Functions array
    static int functionIndex( const QString &name );

    /**
     * Returns the number of functions defined in the parser
     * \returns The number of function defined in the parser.
     */
    static int functionCount();

    /**
     * Returns a quoted column reference (in double quotes)
     * \see quotedString()
     * \see quotedValue()
     */
    static QString quotedColumnRef( QString name );

    /**
     * Returns a quoted version of a string (in single quotes)
     * \see quotedValue()
     * \see quotedColumnRef()
     */
    static QString quotedString( QString text );

    /**
     * Returns a string representation of a literal value, including appropriate
     * quotations where required.
     * \param value value to convert to a string representation
     * \see quotedString()
     * \see quotedColumnRef()
     * \since QGIS 2.14
     */
    static QString quotedValue( const QVariant &value );

    /**
     * Returns a string representation of a literal value, including appropriate
     * quotations where required.
     * \param value value to convert to a string representation
     * \param type value type
     * \see quotedString()
     * \see quotedColumnRef()
     * \since QGIS 2.14
     */
    static QString quotedValue( const QVariant &value, QVariant::Type type );

    //////

    /**
     * Returns the help text for a specified function.
     * \param name function name
     * \see variableHelpText()
     * \see formatVariableHelp()
     */
    static QString helpText( QString name );

    /**
     * Returns a string list of search tags for a specified function.
     * \param name function name
     * \since QGIS 3.12
     */
    static QStringList tags( const QString &name );

    /**
     * Adds a help string for a custom variable.
     *
     * The specified variable \a name should not have an existing help string set. If a help string is already present then
     * FALSE will be returned and no changes will occur.
     *
     * \param name variable name
     * \param description the help string to add. This is user visible, and should be a translated string.
     * \returns TRUE if the help string was successfully added
     * \see variableHelpText()
     * \since QGIS 3.22
     */
    static bool addVariableHelpText( const QString name, const QString &description );

    /**
     * Returns the help text for a specified variable.
     * \param variableName name of variable
     * \see helpText()
     * \see addVariableHelpText()
     * \since QGIS 2.12
     */
    static QString variableHelpText( const QString &variableName );

    /**
     * Returns formatted help text for a variable.
     * \param description translated description of variable
     * \param showValue set to TRUE to include current value of variable in help text
     * \param value current value of variable to show in help text
     * \see helpText()
     * \see variableHelpText()
     * \since QGIS 3.0
     */
    static QString formatVariableHelp( const QString &description, bool showValue = true, const QVariant &value = QVariant() );

    /**
     * Returns the translated name for a function group.
     * \param group untranslated group name
     */
    static QString group( const QString &group );

    /**
     * Formats an expression result for friendly display to the user. Truncates the result to a sensible
     * length, and presents text representations of non numeric/text types (e.g., geometries and features).
     * \param value expression result to format
     * \param htmlOutput set to TRUE to allow HTML formatting, or FALSE for plain text output
     * \param maximumPreviewLength define the maximum character length of the preview
     * \returns formatted string, may contain HTML formatting characters if \a htmlOutput is TRUE
     * \since QGIS 2.14
     */
    static QString formatPreviewString( const QVariant &value, bool htmlOutput = true, int maximumPreviewLength = 60 );

    /**
     * Create an expression allowing to evaluate if a field is equal to a
     *  value. The value may be null.
     * \param fieldName the name of the field
     * \param value the value of the field
     * \param fieldType the type of the field on the left side used to quote the value. If not given, the value type is used instead
     * \returns the expression to evaluate field equality
     * \since QGIS 3.0
     */
    static QString createFieldEqualityExpression( const QString &fieldName, const QVariant &value, QVariant::Type fieldType = QVariant::Type::Invalid );

    /**
     * Returns TRUE if the given \a expression is a simple "field=value" type expression.
     *
     * \param expression expression to test
     * \param field will be set to the field name if the expression is a field equality expression
     * \param value will be set to the value if the expression is a field equality expression
     * \returns TRUE if the expression is a field equality expression
     *
     * \since QGIS 3.18
     */
    static bool isFieldEqualityExpression( const QString &expression, QString &field SIP_OUT, QVariant &value SIP_OUT );

    /**
     * Attempts to reduce a list of expressions to a single "field IN (val1, val2, ... )" type expression.
     *
     * This will only be possible if all the input expressions form simple "field=value" OR "field IN (value1, value2)" expressions, and all
     * reference the same field name.
     *
     * Returns TRUE if the given \a expressions could be converted to an IN type expression.
     *
     * \param expressions expressions to test
     * \param result will be set to the calculated "field IN (...)" expression, wherever possible
     * \returns TRUE if the expression was converted to a field IN type expression
     *
     * \since QGIS 3.18
     */
    static bool attemptReduceToInClause( const QStringList &expressions, QString &result SIP_OUT );

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsExpression: '%1'>" ).arg( sipCpp->expression() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:
    void initGeomCalculator( const QgsExpressionContext *context );

    /**
     * Helper for implicit sharing. When called will create
     * a new deep copy of this expression.
     *
     * \note not available in Python bindings
     */
    void detach() SIP_SKIP;

    QgsExpressionPrivate *d = nullptr;

    //! \note not available in Python bindings
    static void initFunctionHelp() SIP_SKIP;
    //! \note not available in Python bindings
    static void initVariableHelp() SIP_SKIP;

    friend class QgsOgcUtils;
};

Q_DECLARE_METATYPE( QgsExpression )

#endif // QGSEXPRESSION_H
