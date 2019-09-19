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
#include "qgssqliteutils.h"

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
  return QgsSqliteUtils::quotedIdentifier( identifier );
}

QString QgsSQLiteExpressionCompiler::quotedValue( const QVariant &value, bool &ok )
{
  ok = true;
  return QgsSqliteUtils::quotedValue( value );
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
