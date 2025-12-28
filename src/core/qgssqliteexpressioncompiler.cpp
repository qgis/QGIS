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

#include "qgsexpression.h"
#include "qgsexpressionfunction.h"
#include "qgsexpressionnodeimpl.h"
#include "qgssqlexpressioncompiler.h"
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

          result = u"lower(%1) %2 lower(%3) ESCAPE '\\'"_s
                   .arg( opL )
                   .arg( op->op() == QgsExpressionNodeBinaryOperator::boILike ? u"LIKE"_s : u"NOT LIKE"_s )
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

      if ( fd->name() == "make_datetime"_L1 || fd->name() == "make_date"_L1 || fd->name() == "make_time"_L1 )
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
  if ( fnName == "make_datetime"_L1 )
  {
    args = QStringList( u"'%1-%2-%3T%4:%5:%6Z'"_s.arg( args[0].rightJustified( 4, '0' ) )
                        .arg( args[1].rightJustified( 2, '0' ) )
                        .arg( args[2].rightJustified( 2, '0' ) )
                        .arg( args[3].rightJustified( 2, '0' ) )
                        .arg( args[4].rightJustified( 2, '0' ) )
                        .arg( args[5].rightJustified( 2, '0' ) ) );
  }
  else if ( fnName == "make_date"_L1 )
  {
    args = QStringList( u"'%1-%2-%3'"_s.arg( args[0].rightJustified( 4, '0' ) )
                        .arg( args[1].rightJustified( 2, '0' ) )
                        .arg( args[2].rightJustified( 2, '0' ) ) );
  }
  else if ( fnName == "make_time"_L1 )
  {
    args = QStringList( u"'%1:%2:%3'"_s.arg( args[0].rightJustified( 2, '0' ) )
                        .arg( args[1].rightJustified( 2, '0' ) )
                        .arg( args[2].rightJustified( 2, '0' ) ) );
  }
  return args;
}

QString QgsSQLiteExpressionCompiler::castToReal( const QString &value ) const
{
  return u"CAST((%1) AS REAL)"_s.arg( value );
}

QString QgsSQLiteExpressionCompiler::castToInt( const QString &value ) const
{
  return u"CAST((%1) AS INTEGER)"_s.arg( value );
}

QString QgsSQLiteExpressionCompiler::castToText( const QString &value ) const
{
  return u"CAST((%1) AS TEXT)"_s.arg( value );
}

///@endcond
