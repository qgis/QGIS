/***************************************************************************
                          qgspointcouldexpression.h
                          -------------------------
    begin                : January 2022
    copyright            : (C) 2022 Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDEXPRESSION_H
#define QGSPOINTCLOUDEXPRESSION_H

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
#include "qgspointcloudexpressionnode.h"
#include "qgspointcloudattribute.h"

class QgsFeature;
class QgsGeometry;
class QgsOgcUtils;
class QgsVectorLayer;
class QgsVectorDataProvider;
class QgsField;
class QgsFields;
class QgsDistanceArea;
class QDomElement;
class QgsPointcloudExpressionPrivate;
class QgsPointcloudExpressionFunction;

/**
 * \ingroup core
 * \brief Class for parsing and evaluation of expressions (formerly called "search strings").
 * The expressions try to follow both syntax and semantics of SQL expressions.
 *
 * Usage:
 * \code{.py}
 *   exp = QgsPointcloudExpression("gid*2 > 10 and type not in ('D','F')")
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
 * It is normally preferable to call `QgsPointcloudExpression( otherExpression )` instead of
 * `QgsPointcloudExpression( otherExpression.expression() )`. A deep copy will only be made
 * when prepare() is called. For usage this means mainly, that you should
 * normally keep an unprepared master copy of a QgsPointcloudExpression and whenever using it
 * with a particular QgsFeatureIterator copy it just before and prepare it using the
 * same context as the iterator.
 *
 * Implicit sharing was added in 2.14
*/
class CORE_EXPORT QgsPointcloudExpression
{
    Q_DECLARE_TR_FUNCTIONS( QgsPointcloudExpression )
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
    QgsPointcloudExpression( const QString &expr );

    /**
     * Create a copy of this expression. This is preferred
     * over recreating an expression from a string since
     * it does not need to be re-parsed.
     */
    QgsPointcloudExpression( const QgsPointcloudExpression &other );

    /**
     * Create a copy of this expression. This is preferred
     * over recreating an expression from a string since
     * it does not need to be re-parsed.
     */
    QgsPointcloudExpression &operator=( const QgsPointcloudExpression &other );

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
    QgsPointcloudExpression();

    ~QgsPointcloudExpression();

    /**
     * Compares two expressions. The operator returns TRUE
     * if the expression string is equal.
     *
     * \since QGIS 3.0
     */
    bool operator==( const QgsPointcloudExpression &other ) const;

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
    QList<QgsPointcloudExpression::ParserError> parserErrors() const;

    /**
     * Returns the root node of the expression.
     *
     * The root node is NULLPTR if parsing has failed.
     */
    const QgsPointcloudExpressionNode *rootNode() const;

    /**
     * Gets the expression ready for evaluation - find out column indexes.
     * \param context context for preparing expression
     * \since QGIS 2.12
     */
    bool prepare( const QgsPointCloudBlock *block );

    /**
     * Gets list of columns referenced by the expression.
     *
     * \note If the returned list contains the QgsFeatureRequest::AllAttributes constant then
     * all attributes from the layer are required for evaluation of the expression.
     * QgsFeatureRequest::setSubsetOfAttributes automatically handles this case.
     *
     * \warning If the expression has been prepared via a call to QgsPointcloudExpression::prepare(),
     * or a call to QgsPointcloudExpressionNode::prepare() for a node has been made, then parts of
     * the expression may have been determined to evaluate to a static pre-calculatable value.
     * In this case the results will omit attribute indices which are used by these
     * pre-calculated nodes, regardless of their actual referenced columns.
     * If you are seeking to use these functions to introspect an expression you must
     * take care to do this with an unprepared expression.
     *
     * \see referencedAttributeIndexes()
     */
    QSet<QString> referencedAttributes() const;

#ifndef SIP_RUN

    /**
     * Returns a list of all nodes which are used in this expression
     *
     * \note not available in Python bindings
     * \since QGIS 3.2
     */
    QList<const QgsPointcloudExpressionNode *> nodes( ) const;

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
      const QList<const QgsPointcloudExpressionNode *> allNodes( nodes() );
      for ( const auto &node : allNodes )
      {
        const T *n = dynamic_cast<const T *>( node );
        if ( n )
          lst << n;
      }
      return lst;
    }
#endif

    // evaluation

    /**
     * Evaluate the feature and return the result.
     * \note this method does not expect that prepare() has been called on this instance
     * \since QGIS 2.12
     */
    double evaluate( int p );

    //! Returns TRUE if an error occurred when evaluating last input
    bool hasEvalError() const;
    //! Returns evaluation error
    QString evalErrorString() const;
    //! Sets evaluation error (used internally by evaluation functions)
    void setEvalErrorString( const QString &str );

    /**
     * Tests whether a string is a valid expression.
     * \param text string to test
     * \param context optional expression context
     * \param errorMessage will be filled with any error message from the validation
     * \returns TRUE if string is a valid expression
     * \since QGIS 2.12
     */
    static bool checkExpression( const QString &text, const QgsPointCloudBlock *block, QString &errorMessage SIP_OUT );

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
     * Returns a quoted column reference (in double quotes)
     * \see quotedString()
     * \see quotedValue()
     */
    static QString quotedAttributeRef( QString name );

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

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsPointcloudExpression: '%1'>" ).arg( sipCpp->expression() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:

    /**
     * Helper for implicit sharing. When called will create
     * a new deep copy of this expression.
     *
     * \note not available in Python bindings
     */
    void detach() SIP_SKIP;

    QgsPointcloudExpressionPrivate *d = nullptr;

};

Q_DECLARE_METATYPE( QgsPointcloudExpression )

#endif // QGSPOINTCLOUDEXPRESSION_H
