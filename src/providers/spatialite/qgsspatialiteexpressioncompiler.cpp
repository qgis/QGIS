/***************************************************************************
                             qgsspatialiteexpressioncompiler.cpp
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

#include "qgsspatialiteexpressioncompiler.h"
#include "qgssqlexpressioncompiler.h"
#include "qgsspatialiteprovider.h"

QgsSpatiaLiteExpressionCompiler::QgsSpatiaLiteExpressionCompiler( QgsSpatiaLiteFeatureSource* source )
    : QgsSqlExpressionCompiler( source->mFields, QgsSqlExpressionCompiler::LikeIsCaseInsensitive )
{
}

QgsSqlExpressionCompiler::Result QgsSpatiaLiteExpressionCompiler::compileNode( const QgsExpression::Node* node, QString& result )
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

QString QgsSpatiaLiteExpressionCompiler::quotedIdentifier( const QString& identifier )
{
  return QgsSpatiaLiteProvider::quotedIdentifier( identifier );
}

QString QgsSpatiaLiteExpressionCompiler::quotedValue( const QVariant& value, bool& ok )
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
      v.replace( '\'', "''" );
      if ( v.contains( '\\' ) )
        return v.replace( '\\', "\\\\" ).prepend( "E'" ).append( '\'' );
      else
        return v.prepend( '\'' ).append( '\'' );
  }
}
