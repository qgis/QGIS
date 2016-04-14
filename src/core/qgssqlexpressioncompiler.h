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

#include "qgsexpression.h"
#include "qgsfield.h"

/** \ingroup core
 * \class QgsSqlExpressionCompiler
 * \brief Generic expression compiler for translation to provider specific SQL WHERE clauses.
 *
 * This class is designed to be overridden by providers to take advantage of expression compilation,
 * so that feature requests can take advantage of the provider's native filtering support.
 * \note Added in version 2.14
 * \note Not part of stable API, may change in future versions of QGIS
 * \note Not available in Python bindings
 */

class CORE_EXPORT QgsSqlExpressionCompiler
{
  public:

    /** Possible results from expression compilation */
    enum Result
    {
      None, /*!< No expression */
      Complete, /*!< Expression was successfully compiled and can be completely delegated to provider */
      Partial, /*!< Expression was partially compiled, but provider will return extra records and results must be double-checked using QGIS' expression engine*/
      Fail /*!< Provider cannot handle expression */
    };

    /** Enumeration of flags for how provider handles SQL clauses
     */
    enum Flag
    {
      CaseInsensitiveStringMatch = 0x01,  //!< Provider performs case-insensitive string matching for all strings
      LikeIsCaseInsensitive = 0x02, //!< Provider treats LIKE as case-insensitive
      NoNullInBooleanLogic = 0x04, //!< Provider does not support using NULL with boolean logic, eg "(...) OR NULL"
      NoUnaryMinus = 0x08, //!< Provider does not unary minus, eg " -( 100 * 2 ) = ..."
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /** Constructor for expression compiler.
     * @param fields fields from provider
     * @param flags flags which control how expression is compiled
     */
    explicit QgsSqlExpressionCompiler( const QgsFields& fields, const Flags& flags = Flags() );
    virtual ~QgsSqlExpressionCompiler();

    /** Compiles an expression and returns the result of the compilation.
     */
    virtual Result compile( const QgsExpression* exp );

    /** Returns the compiled expression string for use by the provider.
     */
    virtual QString result() { return mResult; }

  protected:

    /** Returns a quoted column identifier, in the format expected by the provider.
     * Derived classes should override this if special handling of column identifiers
     * is required.
     * @see quotedValue()
     */
    virtual QString quotedIdentifier( const QString& identifier );

    /** Returns a quoted attribute value, in the format expected by the provider.
     * Derived classes should override this if special handling of attribute values is required.
     * @param value value to quote
     * @param ok wil be set to true if value can be compiled
     * @see quotedIdentifier()
     */
    virtual QString quotedValue( const QVariant& value, bool &ok );

    /** Compiles an expression node and returns the result of the compilation.
     * @param node expression node to compile
     * @param str string representing compiled node should be stored in this parameter
     * @returns result of node compilation
     */
    virtual Result compileNode( const QgsExpression::Node* node, QString& str );

    QString mResult;
    QgsFields mFields;

  private:

    Flags mFlags;

    bool nodeIsNullLiteral( const QgsExpression::Node* node ) const;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsSqlExpressionCompiler::Flags )

#endif // QGSSQLEXPRESSIONCOMPILER_H
