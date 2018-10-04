/***************************************************************************
 qgsmssqlexpressioncompiler.cpp
 ------------------------------
 begin                : 9.12.2015
 copyright            : (C) 2015 by Nyall Dawson
 email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmssqlexpressioncompiler.h"
#include "qgsexpressionnodeimpl.h"

QgsMssqlExpressionCompiler::QgsMssqlExpressionCompiler( QgsMssqlFeatureSource *source )
  : QgsSqlExpressionCompiler( source->mFields,
                              QgsSqlExpressionCompiler::LikeIsCaseInsensitive | QgsSqlExpressionCompiler::CaseInsensitiveStringMatch | QgsSqlExpressionCompiler::IntegerDivisionResultsInInteger )
{

}

QgsSqlExpressionCompiler::Result QgsMssqlExpressionCompiler::compileNode( const QgsExpressionNode *node, QString &result )
{
  if ( node->nodeType() == QgsExpressionNode::ntBinaryOperator )
  {
    const QgsExpressionNodeBinaryOperator *bin( static_cast<const QgsExpressionNodeBinaryOperator *>( node ) );
    QString op1, op2;

    Result result1 = compileNode( bin->opLeft(), op1 );
    Result result2 = compileNode( bin->opRight(), op2 );
    if ( result1 == Fail || result2 == Fail )
      return Fail;

    switch ( bin->op() )
    {
      case QgsExpressionNodeBinaryOperator::boPow:
        result = QStringLiteral( "power(%1,%2)" ).arg( op1, op2 );
        return result1 == Partial || result2 == Partial ? Partial : Complete;

      case QgsExpressionNodeBinaryOperator::boRegexp:
        return Fail; //not supported, regexp syntax is too different to Qt

      case QgsExpressionNodeBinaryOperator::boConcat:
        result = QStringLiteral( "%1 + %2" ).arg( op1, op2 );
        return result1 == Partial || result2 == Partial ? Partial : Complete;

      default:
        break;
    }

  }

  //fallback to default handling
  return QgsSqlExpressionCompiler::compileNode( node, result );
}

QString QgsMssqlExpressionCompiler::quotedValue( const QVariant &value, bool &ok )
{
  ok = true;
  if ( value.isNull() )
  {
    //no NULL literal support
    ok = false;
    return QString();
  }

  switch ( value.type() )
  {
    case QVariant::Bool:
      //no boolean literal support in mssql, so fake it
      return value.toBool() ? "(1=1)" : "(1=0)";

    default:
      return QgsSqlExpressionCompiler::quotedValue( value, ok );
  }
}

QString QgsMssqlExpressionCompiler::castToReal( const QString &value ) const
{
  return QStringLiteral( "CAST((%1) AS REAL)" ).arg( value );
}

QString QgsMssqlExpressionCompiler::castToInt( const QString &value ) const
{
  return QStringLiteral( "CAST((%1) AS integer)" ).arg( value );
}
