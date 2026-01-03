/***************************************************************************
   qgshanaexpressioncompiler.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgshanaexpressioncompiler.h"

#include "qgsexpressionnodeimpl.h"
#include "qgshanautils.h"
#include "qgslogger.h"
#include "qgssqlexpressioncompiler.h"

QgsHanaExpressionCompiler::QgsHanaExpressionCompiler( QgsHanaFeatureSource *source, bool ignoreStaticNodes )
  : QgsSqlExpressionCompiler( source->mFields, QgsSqlExpressionCompiler::IntegerDivisionResultsInInteger | QgsSqlExpressionCompiler::NoNullInBooleanLogic, ignoreStaticNodes )
  , mGeometryColumn( source->mGeometryColumn )
{
}

QString QgsHanaExpressionCompiler::quotedIdentifier( const QString &identifier )
{
  return QgsHanaUtils::quotedIdentifier( identifier );
}

QString QgsHanaExpressionCompiler::quotedValue( const QVariant &value, bool &ok )
{
  ok = true;
  return QgsHanaUtils::quotedValue( value );
}

static const QMap<QString, QString> FUNCTION_NAMES_SQL_FUNCTIONS_MAP {
  // mathematical functions
  { "sqrt", "SQRT" },
  { "sign", "SIGN" },
  { "abs", "ABS" },
  { "cos", "COS" },
  { "sin", "SIN" },
  { "tan", "TAN" },
  { "acos", "ACOS" },
  { "asin", "ASIN" },
  { "atan", "ATAN" },
  { "atan2", "ATAN2" },
  { "exp", "EXP" },
  { "ln", "LN" },
  { "log", "LOG" },
  { "round", "ROUND" },
  { "floor", "FLOOR" },
  { "ceil", "CEIL" },
  // geometry functions
  { "geom_from_wkt", "ST_GeomFromWKT" },
  // string functions
  { "char", "CHAR" },
  { "lower", "LOWER" },
  { "upper", "UPPER" },
  { "trim", "TRIM" },
  // date/time functions
  { "make_datetime", "" },
  { "make_date", "" },
  { "make_time", "" },
  // other helper functions
  { "coalesce", "COALESCE" }
};

QString QgsHanaExpressionCompiler::sqlFunctionFromFunctionName( const QString &fnName ) const
{
  return FUNCTION_NAMES_SQL_FUNCTIONS_MAP.value( fnName, QString() );
}


QStringList QgsHanaExpressionCompiler::sqlArgumentsFromFunctionName( const QString &fnName, const QStringList &fnArgs ) const
{
  QStringList args( fnArgs );
  if ( fnName == "make_datetime"_L1 )
  {
    args = QStringList( u"TO_TIMESTAMP('%1-%2-%3 %4:%5:%6', 'YYYY-MM-DD HH24:MI:SS')"_s.arg( args[0].rightJustified( 4, '0' ) ).arg( args[1].rightJustified( 2, '0' ) ).arg( args[2].rightJustified( 2, '0' ) ).arg( args[3].rightJustified( 2, '0' ) ).arg( args[4].rightJustified( 2, '0' ) ).arg( args[5].rightJustified( 2, '0' ) ) );
  }
  else if ( fnName == "make_date"_L1 )
  {
    args = QStringList( u"TO_DATE('%1-%2-%3', 'YYYY-MM-DD')"_s.arg( args[0].rightJustified( 4, '0' ) ).arg( args[1].rightJustified( 2, '0' ) ).arg( args[2].rightJustified( 2, '0' ) ) );
  }
  else if ( fnName == "make_time"_L1 )
  {
    args = QStringList( u"TO_TIME('%1:%2:%3', 'HH24:MI:SS') "_s.arg( args[0].rightJustified( 2, '0' ) ).arg( args[1].rightJustified( 2, '0' ) ).arg( args[2].rightJustified( 2, '0' ) ) );
  }
  return args;
}

QString QgsHanaExpressionCompiler::castToReal( const QString &value ) const
{
  return u"CAST((%1) AS REAL)"_s.arg( value );
}

QString QgsHanaExpressionCompiler::castToInt( const QString &value ) const
{
  return u"CAST((%1) AS INTEGER)"_s.arg( value );
}

QString QgsHanaExpressionCompiler::castToText( const QString &value ) const
{
  return u"CAST((%1) AS NVARCHAR)"_s.arg( value );
}

QgsSqlExpressionCompiler::Result QgsHanaExpressionCompiler::compileNode(
  const QgsExpressionNode *node, QString &result
)
{
  QgsSqlExpressionCompiler::Result staticRes = replaceNodeByStaticCachedValueIfPossible( node, result );
  if ( staticRes != Fail )
    return staticRes;

  switch ( node->nodeType() )
  {
    case QgsExpressionNode::ntFunction:
    {
      const QgsExpressionNodeFunction *nodeFunc = static_cast<const QgsExpressionNodeFunction *>( node );
      QgsExpressionFunction *fd = QgsExpression::Functions()[nodeFunc->fnIndex()];

      if ( fd->name().isEmpty() )
        break;

      if ( fd->name() == "$geometry"_L1 )
      {
        result = quotedIdentifier( mGeometryColumn );
        return Complete;
      }
      else if ( fd->name().toLower() == "pi"_L1 )
      {
        result = u"3.141592653589793238"_s;
        return Complete;
      }
      else if ( fd->name() == "make_datetime"_L1 || fd->name() == "make_date"_L1 || fd->name() == "make_time"_L1 )
      {
        const auto constList = nodeFunc->args()->list();
        for ( const QgsExpressionNode *ln : constList )
        {
          if ( ln->nodeType() != QgsExpressionNode::ntLiteral )
            return Fail;
        }
      }
    }
    break;
    case QgsExpressionNode::ntLiteral:
    {
      const QgsExpressionNodeLiteral *n = static_cast<const QgsExpressionNodeLiteral *>( node );
      switch ( n->value().userType() )
      {
        case QMetaType::Type::Bool:
          result = n->value().toBool() ? u"(1=1)"_s : u"(1=0)"_s;
          return Complete;
        default:
          break;
      }
    }
    break;
    case QgsExpressionNode::ntUnaryOperator:
    {
      const QgsExpressionNodeUnaryOperator *unaryOp = static_cast<const QgsExpressionNodeUnaryOperator *>( node );
      switch ( unaryOp->op() )
      {
        case QgsExpressionNodeUnaryOperator::uoNot:
        {
          Result resRight = compileNode( unaryOp->operand(), result );
          if ( "NULL"_L1 == result.toUpper() )
          {
            result.clear();
            return Fail;
          }
          result = u"NOT "_s + result;
          return resRight;
        }
        case QgsExpressionNodeUnaryOperator::uoMinus:
          break;
      }
    }
    break;
    case QgsExpressionNode::ntBinaryOperator:
    {
      const QgsExpressionNodeBinaryOperator *binOp( static_cast<const QgsExpressionNodeBinaryOperator *>( node ) );

      QString opLeft, opRight;
      Result resLeft = compileNode( binOp->opLeft(), opLeft );
      Result resRight = compileNode( binOp->opRight(), opRight );
      Result compileResult;

      if ( resLeft == Fail || resRight == Fail )
        return Fail;
      // NULL can not appear on the left, only as part of IS NULL or IS NOT NULL
      if ( "NULL"_L1 == opLeft.toUpper() )
        return Fail;

      // NULL can only be on the right for IS and IS NOT
      if ( "NULL"_L1 == opRight.toUpper() && ( binOp->op() != QgsExpressionNodeBinaryOperator::boIs && binOp->op() != QgsExpressionNodeBinaryOperator::boIsNot ) )
        return Fail;

      switch ( binOp->op() )
      {
        case QgsExpressionNodeBinaryOperator::boMod:
          result = u"MOD(%1,%2)"_s.arg( opLeft, opRight );
          compileResult = ( resLeft == Partial || resRight == Partial ) ? Partial : Complete;
          return compileResult;

        case QgsExpressionNodeBinaryOperator::boPow:
          result = u"POWER(%1,%2)"_s.arg( opLeft, opRight );
          compileResult = ( resLeft == Partial || resRight == Partial ) ? Partial : Complete;
          return compileResult;

        case QgsExpressionNodeBinaryOperator::boRegexp:
          result = u"%1 LIKE_REGEXPR %2"_s.arg( opLeft, opRight );
          compileResult = ( resLeft == Partial || resRight == Partial ) ? Partial : Complete;
          return compileResult;

        // We only support IS NULL and IS NOT NULL if the operand on the left is a column
        case QgsExpressionNodeBinaryOperator::boIs:
        case QgsExpressionNodeBinaryOperator::boIsNot:
          if ( "NULL"_L1 == opRight.toUpper() )
          {
            if ( binOp->opLeft()->nodeType() != QgsExpressionNode::ntColumnRef )
            {
              QgsDebugError( "Failing IS NULL/IS NOT NULL with non-column on left: " + opLeft );
              return Fail;
            }
          }
          break;

        case QgsExpressionNodeBinaryOperator::boILike:
        case QgsExpressionNodeBinaryOperator::boNotILike:
        {
          if ( resLeft != Complete || resRight != Complete )
            return Fail;

          switch ( binOp->op() )
          {
            case QgsExpressionNodeBinaryOperator::boILike:
              result = u"LOWER(%1) LIKE LOWER(%2)"_s.arg( opLeft, opRight );
              return Complete;
            case QgsExpressionNodeBinaryOperator::boNotILike:
              result = u"NOT LOWER(%1) LIKE LOWER(%2)"_s.arg( opLeft, opRight );
              return Complete;
            default:
              break;
          }
        }
        break;
        default:
          break;
      }
    }
    break;
    default:
      break;
  }

  return QgsSqlExpressionCompiler::compileNode( node, result );
}
