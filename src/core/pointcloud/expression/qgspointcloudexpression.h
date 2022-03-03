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
#include <QList>
#include <QCoreApplication>
#include <QSet>

#include "qgspointcloudexpressionnode.h"
#include "qgspointcloudblock.h"

class QgsPointCloudExpressionPrivate;
class QgsExpression;

/**
 * \ingroup core
 * \brief Class for parsing and evaluation of expressions for pointcloud filtering.
 *
 * Usage:
 * \code{.py}
 *   exp = QgsPointCloudExpression("Z > 10 and Classification in (1, 3, 5)")
 *   block = QgsPointCloudBlock( ... )
 *   if exp.hasParserError():
 *       # show error message with parserErrorString() and exit
 *   exp.prepare( block )
 *   result = exp.evaluate( pointNumber )
 *   if exp.hasEvalError():
 *       # show error message with evalErrorString()
 *   else:
 *       # examine the result
 * \endcode
 *
 * \section evaluation Evaluation result
 *
 * All expression evaluations return doubles
 * 0.0 is considered as False, while any non-zero value is considered as True
 * NaN is returned on evaluation error
 *
 * \section implicit_sharing Implicit sharing
 *
 * This class is implicitly shared, copying has a very low overhead.
 * It is normally preferable to call `QgsPointCloudExpression( otherExpression )` instead of
 * `QgsPointCloudExpression( otherExpression.expression() )`. A deep copy will only be made
 * when prepare() is called.
 *
 * \since QGIS 3.26
*/
class CORE_EXPORT QgsPointCloudExpression
{
    Q_DECLARE_TR_FUNCTIONS( QgsPointCloudExpression )
  public:

    /**
     * Details about any parser errors that were found when parsing the expression.
     * \since QGIS 3.26
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

    QgsPointCloudExpression( const QgsExpression &expression );

    /**
     * Create a copy of this expression. This is preferred
     * over recreating an expression from a string since
     * it does not need to be re-parsed.
     * \since QGIS 3.26
     */
    QgsPointCloudExpression( const QgsPointCloudExpression &other );

    /**
     * Create a copy of this expression. This is preferred
     * over recreating an expression from a string since
     * it does not need to be re-parsed.
     * \since QGIS 3.26
     */
    QgsPointCloudExpression &operator=( const QgsPointCloudExpression &other );

    /**
     * Automatically convert this expression to a string where requested.
     *
     * \since QGIS 3.26
     */
    operator QString() const SIP_SKIP;

    /**
     * Create an empty expression.
     *
     * \since QGIS 3.26
     */
    QgsPointCloudExpression();

    ~QgsPointCloudExpression();

    /**
     * Compares two expressions. The operator returns TRUE
     * if the expression string is equal.
     *
     * \since QGIS 3.26
     */
    bool operator==( const QgsPointCloudExpression &other ) const;

    /**
     * Checks if this expression is valid.
     * A valid expression could be parsed but does not necessarily evaluate properly.
     *
     * \since QGIS 3.26
     */
    bool isValid() const;

    /**
     * Returns TRUE if an error occurred when parsing the input expression
     *
     * \since QGIS 3.26
     */
    bool hasParserError() const;

    /**
     * Returns parser error
     *
     * \since QGIS 3.26
     */
    QString parserErrorString() const;

    /**
     * Returns parser error details including location of error.
     *
     * \since QGIS 3.26
     */
    QList<QgsPointCloudExpression::ParserError> parserErrors() const;

    /**
     * Returns the root node of the expression.
     *
     * The root node is NULLPTR if parsing has failed.
     * \since QGIS 3.26
     */
    const QgsPointCloudExpressionNode *rootNode() const;

    /**
     * Gets the expression ready for evaluation.
     * \param block pointer to the QgsPointCloudBlock that will be filtered
     * \since QGIS 3.26
     */
    bool prepare( const QgsPointCloudBlock *block );

    /**
     * Gets list of attributes referenced by the expression.
     *
     * \since QGIS 3.26
     */
    QSet<QString> referencedAttributes() const;

#ifndef SIP_RUN

    /**
     * Returns a list of all nodes which are used in this expression
     *
     * \note not available in Python bindings
     * \since QGIS 3.26
     */
    QList<const QgsPointCloudExpressionNode *> nodes( ) const;

    /**
     * Returns a list of all nodes of the given class which are used in this expression
     *
     * \note not available in Python bindings
     * \since QGIS 3.26
     */
    template <class T>
    QList<const T *> findNodes( ) const
    {
      QList<const T *> lst;
      const QList<const QgsPointCloudExpressionNode *> allNodes( nodes() );
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
     * Evaluate the expression for one point.
     * \returns 0.0 for false or 1.0 for true.
     * \param p point number within the block to evaluate
     * \since QGIS 3.26
     */
    double evaluate( int p );

    /**
     * Returns TRUE if an error occurred when evaluating last input
     *
     * \since QGIS 3.26
     */
    bool hasEvalError() const;

    /**
     * Returns evaluation error
     *
     * \since QGIS 3.26
     */
    QString evalErrorString() const;

    /**
     * Sets evaluation error (used internally by evaluation functions)
     *
     * \since QGIS 3.26
     */
    void setEvalErrorString( const QString &str );

    /**
     * Tests whether a string is a valid expression.
     * \param expression the QgsExpression to test
     * \param block QgsPointCloudBlock to be filtered
     * \param errorMessage will be filled with any error message from the validation
     * \returns TRUE if string is a valid expression
     * \since QGIS 3.26
     */
    static bool checkExpression( const QgsExpression &expression, const QgsPointCloudBlock *block, QString &errorMessage SIP_OUT );

    /**
     * Set the expression string, will reset the whole internal structure.
     *
     * \since QGIS 3.26
     */
    void setExpression( const QgsExpression &expression );

    /**
     * Returns the original, unmodified expression string.
     * If there was none supplied because it was constructed by sole
     * API calls, dump() will be used to create one instead.
     *
     * \since QGIS 3.26
     */
    QString expression() const;

    /**
     * Returns an expression string, constructed from the internal
     * abstract syntax tree. This does not contain any nice whitespace
     * formatting or comments. In general it is preferable to use
     * expression() instead.
     *
     * \since QGIS 3.26
     */
    QString dump() const;

    //////

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsPointCloudExpression: '%1'>" ).arg( sipCpp->expression() );
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

    QgsPointCloudExpressionPrivate *d = nullptr;

};

Q_DECLARE_METATYPE( QgsPointCloudExpression )

#endif // QGSPOINTCLOUDEXPRESSION_H
