/***************************************************************************
                             qgssqliteexpressioncompiler.cpp
                             -----------------------------------
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

///@cond PRIVATE

#include "qgssqliteexpressioncompiler.h"
#include "qgssqlexpressioncompiler.h"
#include "qgsexpressionnodeimpl.h"

QgsSQLiteExpressionCompiler::QgsSQLiteExpressionCompiler( const QgsFields &fields )
  : QgsSqlExpressionCompiler( fields, QgsSqlExpressionCompiler::LikeIsCaseInsensitive | QgsSqlExpressionCompiler::IntegerDivisionResultsInInteger )
{
}

QgsSqlExpressionCompiler::Result QgsSQLiteExpressionCompiler::compileNode( const QgsExpressionNode *node, QString &result )
{
  switch ( node->nodeType() )
  {
    case QgsExpressionNode::ntBinaryOperator:
    {
      switch ( static_cast<const QgsExpressionNodeBinaryOperator *>( node )->op() )
      {
        case QgsExpressionNodeBinaryOperator::boPow:
        case QgsExpressionNodeBinaryOperator::boRegexp:
          return Fail; //not supported by SQLite

        default:
          //fallback to default handling
          return QgsSqlExpressionCompiler::compileNode( node, result );
      }
    }

    default:
      break;
  }

  return QgsSqlExpressionCompiler::compileNode( node, result );
}

QString QgsSQLiteExpressionCompiler::quotedIdentifier( const QString &identifier )
{
  QString id( identifier );
  id.replace( '\"', QLatin1String( "\"\"" ) );
  return id.prepend( '\"' ).append( '\"' );
}

QString QgsSQLiteExpressionCompiler::quotedValue( const QVariant &value, bool &ok )
{
  ok = true;

  if ( value.isNull() )
    return QStringLiteral( "NULL" );

  switch ( value.type() )
  {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::Double:
      return value.toString();

    case QVariant::Bool:
      //SQLite has no boolean literals
      return value.toBool() ? "1" : "0";

    default:
    case QVariant::String:
      QString v = value.toString();
      // https://www.sqlite.org/lang_expr.html :
      // """A string constant is formed by enclosing the string in single quotes (').
      // A single quote within the string can be encoded by putting two single quotes
      // in a row - as in Pascal. C-style escapes using the backslash character are not supported because they are not standard SQL. """
      return v.replace( '\'', QLatin1String( "''" ) ).prepend( '\'' ).append( '\'' );
  }
}

QString QgsSQLiteExpressionCompiler::sqlFunctionFromFunctionName( const QString &fnName ) const
{
  static const QMap<QString, QString> FN_NAMES
  {
    { "abs", "abs" },
    { "char", "char" },
    { "coalesce", "coalesce" },
    { "lower", "lower" },
    { "round", "round" },
    { "trim", "trim" },
    { "upper", "upper" },
  };

  return FN_NAMES.value( fnName, QString() );
}

QString QgsSQLiteExpressionCompiler::castToReal( const QString &value ) const
{
  return QStringLiteral( "CAST((%1) AS REAL)" ).arg( value );
}

QString QgsSQLiteExpressionCompiler::castToInt( const QString &value ) const
{
  return QStringLiteral( "CAST((%1) AS INTEGER)" ).arg( value );
}

QString QgsSQLiteExpressionCompiler::castToText( const QString &value ) const
{
  return QStringLiteral( "CAST((%1) AS TEXT)" ).arg( value );
}

///@endcond
