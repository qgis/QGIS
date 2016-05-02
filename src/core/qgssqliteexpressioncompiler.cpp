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

QgsSQLiteExpressionCompiler::QgsSQLiteExpressionCompiler( const QgsFields& fields )
    : QgsSqlExpressionCompiler( fields, QgsSqlExpressionCompiler::LikeIsCaseInsensitive )
{
}

QgsSqlExpressionCompiler::Result QgsSQLiteExpressionCompiler::compileNode( const QgsExpression::Node* node, QString& result )
{
  switch ( node->nodeType() )
  {
    case QgsExpression::ntBinaryOperator:
    {
      switch ( static_cast<const QgsExpression::NodeBinaryOperator*>( node )->op() )
      {
        case QgsExpression::boPow:
        case QgsExpression::boRegexp:
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

QString QgsSQLiteExpressionCompiler::quotedIdentifier( const QString& identifier )
{
  QString id( identifier );
  id.replace( '\"', "\"\"" );
  return id.prepend( '\"' ).append( '\"' );
}

QString QgsSQLiteExpressionCompiler::quotedValue( const QVariant& value, bool& ok )
{
  ok = true;

  if ( value.isNull() )
    return "NULL";

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
      return v.replace( '\'', "''" ).prepend( '\'' ).append( '\'' );
  }
}

///@endcond
