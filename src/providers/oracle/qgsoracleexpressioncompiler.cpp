/***************************************************************************
    qgsoracleexpressioncompiler.cpp
    ----------------------------------------------------
    date                 : December 2015
    copyright            : (C) 2015 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoracleexpressioncompiler.h"
#include "qgssqlexpressioncompiler.h"
#include "qgsexpressionnodeimpl.h"

QgsOracleExpressionCompiler::QgsOracleExpressionCompiler( QgsOracleFeatureSource *source, bool ignoreStaticNodes )
  : QgsSqlExpressionCompiler( source->mFields, Flags(), ignoreStaticNodes )
{
}

QgsSqlExpressionCompiler::Result QgsOracleExpressionCompiler::compileNode( const QgsExpressionNode *node, QString &result )
{
  QgsSqlExpressionCompiler::Result staticRes = replaceNodeByStaticCachedValueIfPossible( node, result );
  if ( staticRes != Fail )
    return staticRes;

  switch ( node->nodeType() )
  {
    case QgsExpressionNode::ntBinaryOperator:
    {
      const QgsExpressionNodeBinaryOperator *bin( static_cast<const QgsExpressionNodeBinaryOperator *>( node ) );

      switch ( bin->op() )
      {
        case QgsExpressionNodeBinaryOperator::boConcat:
          // oracle's handling of || WRT null is not standards compliant
          return Fail;

        case QgsExpressionNodeBinaryOperator::boPow:
        case QgsExpressionNodeBinaryOperator::boRegexp:
        case QgsExpressionNodeBinaryOperator::boILike:
        case QgsExpressionNodeBinaryOperator::boNotILike:
        case QgsExpressionNodeBinaryOperator::boMod:
        case QgsExpressionNodeBinaryOperator::boIntDiv:
        {
          QString op1, op2;

          if ( compileNode( bin->opLeft(), op1 ) != Complete || compileNode( bin->opRight(), op2 ) != Complete )
            return Fail;

          switch ( bin->op() )
          {
            case QgsExpressionNodeBinaryOperator::boPow:
              result = QStringLiteral( "power(%1,%2)" ).arg( op1, op2 );
              return Complete;

            case QgsExpressionNodeBinaryOperator::boRegexp:
              result = QStringLiteral( "regexp_like(%1,%2)" ).arg( op1, op2 );
              return Complete;

            case QgsExpressionNodeBinaryOperator::boILike:
              result = QStringLiteral( "lower(%1) LIKE lower(%2) ESCAPE '\\'" ).arg( op1, op2 );
              return Complete;

            case QgsExpressionNodeBinaryOperator::boNotILike:
              result = QStringLiteral( "NOT lower(%1) LIKE lower(%2) ESCAPE '\\'" ).arg( op1, op2 );
              return Complete;

            case QgsExpressionNodeBinaryOperator::boIntDiv:
              result = QStringLiteral( "FLOOR(%1 / %2)" ).arg( op1, op2 );
              return Complete;


            case QgsExpressionNodeBinaryOperator::boMod:
              result = QStringLiteral( "MOD(%1,%2)" ).arg( op1, op2 );
              return Complete;

            default:
              break;
          }
          break; // no warnings
        }

        default:
          break;
      }
      break;
    }

    case QgsExpressionNode::ntFunction:
    {
      const QgsExpressionNodeFunction *n = static_cast<const QgsExpressionNodeFunction *>( node );
      QgsExpressionFunction *fd = QgsExpression::Functions()[n->fnIndex()];

      if ( fd->name() == QLatin1String( "make_datetime" ) )
      {
        const auto constList = n->args()->list();
        for ( const QgsExpressionNode *ln : constList )
        {
          if ( ln->nodeType() != QgsExpressionNode::ntLiteral )
            return Fail;
        }
      }
      return QgsSqlExpressionCompiler::compileNode( node, result );
      break;
    }

    default:
      break;
  }

  //fallback to default handling
  return QgsSqlExpressionCompiler::compileNode( node, result );
}

QString QgsOracleExpressionCompiler::quotedIdentifier( const QString &identifier )
{
  return QgsOracleConn::quotedIdentifier( identifier );
}

QString QgsOracleExpressionCompiler::quotedValue( const QVariant &value, bool &ok )
{
  ok = true;

  switch ( value.userType() )
  {
    case QMetaType::Type::Bool:
      //no boolean literal support in Oracle, so fake it
      return value.toBool() ? QStringLiteral( "(1=1)" ) : QStringLiteral( "(1=0)" );

    default:
      return QgsOracleConn::quotedValue( value );
  }
}

static const QMap<QString, QString> FUNCTION_NAMES_SQL_FUNCTIONS_MAP {
  { "sqrt", "sqrt" },
  { "abs", "abs" },
  { "cos", "cos" },
  { "sin", "sin" },
  { "tan", "tan" },
  { "acos", "acos" },
  { "asin", "asin" },
  { "atan", "atan" },
  { "exp", "exp" },
  { "ln", "ln" },
  { "log", "log" },
  { "round", "round" },
  { "floor", "floor" },
  { "ceil", "ceil" },
  { "lower", "lower" },
  { "upper", "upper" },
  { "make_datetime", "" },
};

QString QgsOracleExpressionCompiler::sqlFunctionFromFunctionName( const QString &fnName ) const
{
  return FUNCTION_NAMES_SQL_FUNCTIONS_MAP.value( fnName, QString() );
}

QStringList QgsOracleExpressionCompiler::sqlArgumentsFromFunctionName( const QString &fnName, const QStringList &fnArgs ) const
{
  QStringList args( fnArgs );
  if ( fnName == QLatin1String( "make_datetime" ) )
  {
    args = QStringList( QStringLiteral( "TIMESTAMP '%1-%2-%3 %4:%5:%6'" ).arg( args[0].rightJustified( 4, '0' ) ).arg( args[1].rightJustified( 2, '0' ) ).arg( args[2].rightJustified( 2, '0' ) ).arg( args[3].rightJustified( 2, '0' ) ).arg( args[4].rightJustified( 2, '0' ) ).arg( args[5].rightJustified( 2, '0' ) ) );
  }
  return args;
}
