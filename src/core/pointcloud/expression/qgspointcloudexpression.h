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

#define SIP_NO_FILE

#ifndef QGSPOINTCLOUDEXPRESSION_H
#define QGSPOINTCLOUDEXPRESSION_H

#include "qgis_core.h"
#include <QMetaType>
#include <QList>
#include <QSet>

#include "qgspointcloudexpressionnode.h"
#include "qgsexpression.h"

class QgsPointCloudExpressionPrivate;
class QgsPointCloudBlock;

/**
 * \ingroup core
 * \brief Class for parsing and evaluation of expressions for pointcloud filtering.
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
     * Create an expression using a string.
     */
    QgsPointCloudExpression( const QString &subset );

    /**
     * Create a copy of this expression. This is preferred
     * over recreating an expression from a string since
     * it does not need to be re-parsed.
     */
    QgsPointCloudExpression( const QgsPointCloudExpression &other );

    /**
     * Create a copy of this expression. This is preferred
     * over recreating an expression from a string since
     * it does not need to be re-parsed.
     */
    QgsPointCloudExpression &operator=( const QgsPointCloudExpression &other );

    /**
     * Automatically convert this expression to a string where requested.
     */
    operator QString() const;

    /**
     * Create an empty expression.
     */
    QgsPointCloudExpression();

    ~QgsPointCloudExpression();

    /**
     * Compares two expressions. The operator returns TRUE
     * if the expression string is equal.
     */
    bool operator==( const QgsPointCloudExpression &other ) const;

    /**
     * Checks if this expression is valid.
     * A valid expression could be parsed but does not necessarily evaluate properly.
     */
    bool isValid() const;

    /**
     * Returns TRUE if an error occurred when parsing the input expression
     */
    bool hasParserError() const;

    /**
     * Returns parser error
     */
    QString parserErrorString() const;

    /**
     * Returns parser error details including location of error.
     */
    QList<QgsExpression::ParserError> parserErrors() const;

    /**
     * Returns the root node of the expression.
     *
     * The root node is NULLPTR if parsing has failed.
     */
    const QgsPointCloudExpressionNode *rootNode() const;

    /**
     * Gets the expression ready for evaluation.
     * \param block pointer to the QgsPointCloudBlock that will be filtered
     */
    bool prepare( const QgsPointCloudBlock *block );

    /**
     * Gets list of attributes referenced by the expression.
     */
    QSet<QString> referencedAttributes() const;

    /**
     * Returns a list of all nodes which are used in this expression
     */
    QList<const QgsPointCloudExpressionNode *> nodes( ) const;

    /**
     * Returns a list of all nodes of the given class which are used in this expression
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

    // evaluation

    /**
     * Evaluate the expression for one point.
     * \returns 0.0 for false or 1.0 for true.
     * \param pointIndex point number within the block to evaluate
     * \note prepare() must me called for a specific block before this function can be used
     */
    double evaluate( int pointIndex );

    /**
     * Returns TRUE if an error occurred when evaluating last input
     */
    bool hasEvalError() const;

    /**
     * Returns evaluation error
     */
    QString evalErrorString() const;

    /**
     * Sets evaluation error (used internally by evaluation functions)
     */
    void setEvalErrorString( const QString &str );

    /**
     * Set the expression string
     */
    void setExpression( const QString &subset );

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
     * Tests whether a string is a valid expression.
     * \param expression the QgsExpression to test
     * \param block QgsPointCloudBlock to be filtered
     * \param errorMessage will be filled with any error message from the validation
     * \returns TRUE if string is a valid expression
     */
    static bool checkExpression( const QgsExpression &expression, const QgsPointCloudBlock *block, QString &errorMessage );

  private:

    /**
     * Helper for implicit sharing. When called will create
     * a new deep copy of this expression.
     */
    void detach();

    QgsPointCloudExpressionPrivate *d = nullptr;

};

Q_DECLARE_METATYPE( QgsPointCloudExpression )

#endif // QGSPOINTCLOUDEXPRESSION_H
