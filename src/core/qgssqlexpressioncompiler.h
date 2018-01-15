/***************************************************************************
                             qgssqlexpressioncompiler.h
                             --------------------------
    begin                : November 2015
    copyright            : (C) 2015 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSQLEXPRESSIONCOMPILER_H
#define QGSSQLEXPRESSIONCOMPILER_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgsfields.h"

class QgsExpression;
class QgsExpressionNode;

/**
 * \ingroup core
 * \class QgsSqlExpressionCompiler
 * \brief Generic expression compiler for translation to provider specific SQL WHERE clauses.
 *
 * This class is designed to be overridden by providers to take advantage of expression compilation,
 * so that feature requests can take advantage of the provider's native filtering support.
 * \since QGIS 2.14
 * \note Not part of stable API, may change in future versions of QGIS
 * \note Not available in Python bindings
 */

class CORE_EXPORT QgsSqlExpressionCompiler
{
  public:

    //! Possible results from expression compilation
    enum Result
    {
      None, //!< No expression
      Complete, //!< Expression was successfully compiled and can be completely delegated to provider
      Partial, //!< Expression was partially compiled, but provider will return extra records and results must be double-checked using QGIS' expression engine
      Fail //!< Provider cannot handle expression
    };

    /**
     * Enumeration of flags for how provider handles SQL clauses
     */
    enum Flag
    {
      CaseInsensitiveStringMatch = 1,  //!< Provider performs case-insensitive string matching for all strings
      LikeIsCaseInsensitive = 1 << 1, //!< Provider treats LIKE as case-insensitive
      NoNullInBooleanLogic = 1 << 2, //!< Provider does not support using NULL with boolean logic, e.g., "(...) OR NULL"
      NoUnaryMinus = 1 << 3, //!< Provider does not unary minus, e.g., " -( 100 * 2 ) = ..."
      IntegerDivisionResultsInInteger = 1 << 4, //!< Dividing int by int results in int on provider. Subclass must implement the castToReal() function to allow compilation of division.
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for expression compiler.
     * \param fields fields from provider
     * \param flags flags which control how expression is compiled
     */
    explicit QgsSqlExpressionCompiler( const QgsFields &fields, QgsSqlExpressionCompiler::Flags flags = Flags() );
    virtual ~QgsSqlExpressionCompiler() = default;

    /**
     * Compiles an expression and returns the result of the compilation.
     */
    virtual Result compile( const QgsExpression *exp );

    /**
     * Returns the compiled expression string for use by the provider.
     */
    virtual QString result();

  protected:

    /**
     * Returns a quoted column identifier, in the format expected by the provider.
     * Derived classes should override this if special handling of column identifiers
     * is required.
     * \see quotedValue()
     */
    virtual QString quotedIdentifier( const QString &identifier );

    /**
     * Returns a quoted attribute value, in the format expected by the provider.
     * Derived classes should override this if special handling of attribute values is required.
     * \param value value to quote
     * \param ok wil be set to true if value can be compiled
     * \see quotedIdentifier()
     */
    virtual QString quotedValue( const QVariant &value, bool &ok );

    /**
     * Compiles an expression node and returns the result of the compilation.
     * \param node expression node to compile
     * \param str string representing compiled node should be stored in this parameter
     * \returns result of node compilation
     */
    virtual Result compileNode( const QgsExpressionNode *node, QString &str );

    /**
     * Return the SQL function for the expression function.
     * Derived classes should override this to help compile functions
     * \param fnName expression function name
     * \returns the SQL function name
     */
    virtual QString sqlFunctionFromFunctionName( const QString &fnName ) const;

    /**
     * Return the Arguments for SQL function for the expression function.
     * Derived classes should override this to help compile functions
     * \param fnName expression function name
     * \param fnArgs arguments from expression
     * \returns the arguments updated for SQL Function
     */
    virtual QStringList sqlArgumentsFromFunctionName( const QString &fnName, const QStringList &fnArgs ) const;

    /**
     * Casts a value to a real result. Subclasses which indicate the IntegerDivisionResultsInInteger
     * flag must reimplement this to cast a numeric value to a real type value so that division results
     * in a real value result instead of integer.
     * \since QGIS 3.0
     */
    virtual QString castToReal( const QString &value ) const;

    /**
     * Casts a value to a integer result. Subclasses must reimplement this to cast a numeric value to a integer
     * type value so that integer division results in a integer value result instead of real.
     * \since QGIS 3.0
     */
    virtual QString castToInt( const QString &value ) const;

    QString mResult;
    QgsFields mFields;

  private:

    Flags mFlags;

    bool nodeIsNullLiteral( const QgsExpressionNode *node ) const;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsSqlExpressionCompiler::Flags )

#endif // QGSSQLEXPRESSIONCOMPILER_H
