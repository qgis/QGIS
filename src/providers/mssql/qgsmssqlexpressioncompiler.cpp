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

QgsMssqlExpressionCompiler::QgsMssqlExpressionCompiler( QgsMssqlFeatureSource *source, bool ignoreStaticNodes )
  : QgsSqlExpressionCompiler( source->mFields, QgsSqlExpressionCompiler::LikeIsCaseInsensitive | QgsSqlExpressionCompiler::CaseInsensitiveStringMatch | QgsSqlExpressionCompiler::IntegerDivisionResultsInInteger, ignoreStaticNodes )
{
}

QgsSqlExpressionCompiler::Result QgsMssqlExpressionCompiler::compileNode( const QgsExpressionNode *node, QString &result )
{
  const QgsSqlExpressionCompiler::Result staticRes = replaceNodeByStaticCachedValueIfPossible( node, result );
  if ( staticRes != Fail )
    return staticRes;

  switch ( node->nodeType() )
  {
    case QgsExpressionNode::ntBinaryOperator:
    {
      const QgsExpressionNodeBinaryOperator *bin( static_cast<const QgsExpressionNodeBinaryOperator *>( node ) );
      switch ( bin->op() )
      {
        // special handling
        case QgsExpressionNodeBinaryOperator::boPow:
        case QgsExpressionNodeBinaryOperator::boRegexp:
        case QgsExpressionNodeBinaryOperator::boConcat:
          break;

        default:
          // fallback to default handling
          return QgsSqlExpressionCompiler::compileNode( node, result );
      }

      QString op1, op2;

      const Result result1 = compileNode( bin->opLeft(), op1 );
      const Result result2 = compileNode( bin->opRight(), op2 );
      if ( result1 == Fail || result2 == Fail )
        return Fail;

      switch ( bin->op() )
      {
        case QgsExpressionNodeBinaryOperator::boPow:
          result = u"power(%1,%2)"_s.arg( op1, op2 );
          return result1 == Partial || result2 == Partial ? Partial : Complete;

        case QgsExpressionNodeBinaryOperator::boRegexp:
          return Fail; //not supported, regexp syntax is too different to Qt

        case QgsExpressionNodeBinaryOperator::boConcat:
          result = u"%1 + %2"_s.arg( op1, op2 );
          return result1 == Partial || result2 == Partial ? Partial : Complete;

        default:
          break;
      }

      break;
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

  //fallback to default handling
  return QgsSqlExpressionCompiler::compileNode( node, result );
}

QString QgsMssqlExpressionCompiler::quotedValue( const QVariant &value, bool &ok )
{
  ok = true;
  if ( QgsVariantUtils::isNull( value ) )
  {
    //no NULL literal support
    ok = false;
    return QString();
  }

  switch ( value.userType() )
  {
    case QMetaType::Type::Bool:
      //no boolean literal support in mssql, so fake it
      return value.toBool() ? u"(1=1)"_s : u"(1=0)"_s;

    default:
      return QgsSqlExpressionCompiler::quotedValue( value, ok );
  }
}

QString QgsMssqlExpressionCompiler::quotedIdentifier( const QString &identifier )
{
  QString quoted = identifier;
  quoted.replace( '[', "[["_L1 );
  quoted.replace( ']', "]]"_L1 );
  quoted = quoted.prepend( '[' ).append( ']' );
  return quoted;
}

QString QgsMssqlExpressionCompiler::castToReal( const QString &value ) const
{
  return u"CAST((%1) AS REAL)"_s.arg( value );
}

QString QgsMssqlExpressionCompiler::castToInt( const QString &value ) const
{
  return u"CAST((%1) AS integer)"_s.arg( value );
}

static const QMap<QString, QString> FUNCTION_NAMES_SQL_FUNCTIONS_MAP {
  { "sqrt", "sqrt" },
  { "abs", "abs" },
  { "cos", "cos" },
  { "sin", "sin" },
  { "tan", "tan" },
  { "radians", "radians" },
  { "degrees", "degrees" },
  { "acos", "acos" },
  { "asin", "asin" },
  { "atan", "atan" },
  { "atan2", "atn2" },
  { "exp", "exp" },
  { "ln", "ln" },
  { "log", "log" },
  { "log10", "log10" },
  { "pi", "pi" },
  { "round", "round" },
  { "floor", "floor" },
  { "ceil", "ceiling" },
  { "char", "char" },
#if 0 // should be possible if/when mssql compiler handles case sensitive string matches
  { "coalesce", "coalesce" },
#endif
  { "trim", "trim" },
  { "lower", "lower" },
  { "upper", "upper" },
  { "make_datetime", "" },
  { "make_date", "" },
  { "make_time", "" },
};

QString QgsMssqlExpressionCompiler::sqlFunctionFromFunctionName( const QString &fnName ) const
{
  return FUNCTION_NAMES_SQL_FUNCTIONS_MAP.value( fnName, QString() );
}

QStringList QgsMssqlExpressionCompiler::sqlArgumentsFromFunctionName( const QString &fnName, const QStringList &fnArgs ) const
{
  QStringList args( fnArgs );
  if ( fnName == "make_datetime"_L1 )
  {
    args = QStringList( u"'%1-%2-%3T%4:%5:%6Z'"_s.arg( args[0].rightJustified( 4, '0' ) ).arg( args[1].rightJustified( 2, '0' ) ).arg( args[2].rightJustified( 2, '0' ) ).arg( args[3].rightJustified( 2, '0' ) ).arg( args[4].rightJustified( 2, '0' ) ).arg( args[5].rightJustified( 2, '0' ) ) );
  }
  else if ( fnName == "make_date"_L1 )
  {
    args = QStringList( u"'%1-%2-%3'"_s.arg( args[0].rightJustified( 4, '0' ) ).arg( args[1].rightJustified( 2, '0' ) ).arg( args[2].rightJustified( 2, '0' ) ) );
  }
  else if ( fnName == "make_time"_L1 )
  {
    args = QStringList( u"'%1:%2:%3'"_s.arg( args[0].rightJustified( 2, '0' ) ).arg( args[1].rightJustified( 2, '0' ) ).arg( args[2].rightJustified( 2, '0' ) ) );
  }
  return args;
}
