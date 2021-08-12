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
#include "qgsexpressionfunction.h"
#include "qgsexpressionnodeimpl.h"
#include "qgsexpression.h"
#include "qgssqliteutils.h"

QgsSQLiteExpressionCompiler::QgsSQLiteExpressionCompiler( const QgsFields &fields, bool ignoreStaticNodes )
  : QgsSqlExpressionCompiler( fields, QgsSqlExpressionCompiler::LikeIsCaseInsensitive | QgsSqlExpressionCompiler::IntegerDivisionResultsInInteger, ignoreStaticNodes )
{
}

QgsSqlExpressionCompiler::Result QgsSQLiteExpressionCompiler::compileNode( const QgsExpressionNode *node, QString &result )
{
  const QgsSqlExpressionCompiler::Result staticRes = replaceNodeByStaticCachedValueIfPossible( node, result );
  if ( staticRes != Fail )
    return staticRes;

  switch ( node->nodeType() )
  {
    case QgsExpressionNode::ntBinaryOperator:
    {
      const QgsExpressionNodeBinaryOperator *op = static_cast<const QgsExpressionNodeBinaryOperator *>( node );
      switch ( op->op() )
      {
        case QgsExpressionNodeBinaryOperator::boPow:
        case QgsExpressionNodeBinaryOperator::boRegexp:
          return Fail; //not supported by SQLite

        case QgsExpressionNodeBinaryOperator::boILike:
        case QgsExpressionNodeBinaryOperator::boNotILike:
        {
          QString opL, opR;

          if ( compileNode( op->opLeft(), opL ) != Complete ||
               compileNode( op->opRight(), opR ) != Complete )
            return Fail;

          result = QStringLiteral( "lower(%1) %2 lower(%3) ESCAPE '\\'" )
                   .arg( opL )
                   .arg( op->op() == QgsExpressionNodeBinaryOperator::boILike ? QStringLiteral( "LIKE" ) : QStringLiteral( "NOT LIKE" ) )
                   .arg( opR );

          return Complete;
        }

        default:
          //fallback to default handling
          return QgsSqlExpressionCompiler::compileNode( node, result );
      }
    }

    case QgsExpressionNode::ntFunction:
    {
      const QgsExpressionNodeFunction *n = static_cast<const QgsExpressionNodeFunction *>( node );
      QgsExpressionFunction *fd = QgsExpression::Functions()[n->fnIndex()];

      if ( fd->name() == QLatin1String( "make_datetime" ) || fd->name() == QLatin1String( "make_date" ) || fd->name() == QLatin1String( "make_time" ) )
      {
        const auto constList = n->args()->list();
        for ( const QgsExpressionNode *ln : constList )
        {
          if ( ln->nodeType() != QgsExpressionNode::ntLiteral )
            return Fail;
        }
      }

      return QgsSqlExpressionCompiler::compileNode( node, result );
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
    { "make_datetime", "" },
    { "make_date", "" },
    { "make_time", "" },
  };

  return FN_NAMES.value( fnName, QString() );
}

QStringList QgsSQLiteExpressionCompiler::sqlArgumentsFromFunctionName( const QString &fnName, const QStringList &fnArgs ) const
{
  QStringList args( fnArgs );
  if ( fnName == QLatin1String( "make_datetime" ) )
  {
    args = QStringList( QStringLiteral( "'%1-%2-%3T%4:%5:%6Z'" ).arg( args[0].rightJustified( 4, '0' ) )
                        .arg( args[1].rightJustified( 2, '0' ) )
                        .arg( args[2].rightJustified( 2, '0' ) )
                        .arg( args[3].rightJustified( 2, '0' ) )
                        .arg( args[4].rightJustified( 2, '0' ) )
                        .arg( args[5].rightJustified( 2, '0' ) ) );
  }
  else if ( fnName == QLatin1String( "make_date" ) )
  {
    args = QStringList( QStringLiteral( "'%1-%2-%3'" ).arg( args[0].rightJustified( 4, '0' ) )
                        .arg( args[1].rightJustified( 2, '0' ) )
                        .arg( args[2].rightJustified( 2, '0' ) ) );
  }
  else if ( fnName == QLatin1String( "make_time" ) )
  {
    args = QStringList( QStringLiteral( "'%1:%2:%3'" ).arg( args[0].rightJustified( 2, '0' ) )
                        .arg( args[1].rightJustified( 2, '0' ) )
                        .arg( args[2].rightJustified( 2, '0' ) ) );
  }
  return args;
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
