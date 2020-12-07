/***************************************************************************
  qgsdb2expressioncompiler.cpp - DB2 expression compiler
  --------------------------------------
  Date      : 2016-01-27
  Copyright : (C) 2016 by David Adler
                          Shirley Xiao, David Nguyen
  Email     : dadler at adtechgeospatial.com
              xshirley2012 at yahoo.com, davidng0123 at gmail.com
  Adapted from MSSQL provider by Tamas Szekeres
****************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/

#include "qgsdb2expressioncompiler.h"
#include "qgsexpressionnodeimpl.h"
#include "qgslogger.h"

QgsDb2ExpressionCompiler::QgsDb2ExpressionCompiler( QgsDb2FeatureSource *source )
  : QgsSqlExpressionCompiler( source->mFields
                            )
{

}

QString nodeType( const QgsExpressionNode *node )
{
  QString opString = QStringLiteral( "?" );
  if ( node->nodeType() == QgsExpressionNode::ntUnaryOperator ) opString = QStringLiteral( "ntUnaryOperator" );
  if ( node->nodeType() == QgsExpressionNode::ntBinaryOperator ) opString = QStringLiteral( "ntBinaryOperator" );
  if ( node->nodeType() == QgsExpressionNode::ntInOperator ) opString = QStringLiteral( "ntInOperator" );
  if ( node->nodeType() == QgsExpressionNode::ntFunction ) opString = QStringLiteral( "ntFunction" );
  if ( node->nodeType() == QgsExpressionNode::ntLiteral ) opString = QStringLiteral( "ntLiteral" );
  if ( node->nodeType() == QgsExpressionNode::ntColumnRef ) opString = QStringLiteral( "ntColumnRef" );
  if ( node->nodeType() == QgsExpressionNode::ntCondition ) opString = QStringLiteral( "ntCondition" );
  QString result = QStringLiteral( "%1 - " ).arg( node->nodeType() ) + opString;
  return result;

}

QString resultType( QgsSqlExpressionCompiler::Result result )
{
  if ( result == QgsSqlExpressionCompiler::None ) return QStringLiteral( "None" );
  if ( result == QgsSqlExpressionCompiler::Complete ) return QStringLiteral( "Complete" );
  if ( result == QgsSqlExpressionCompiler::Partial ) return QStringLiteral( "Partial" );
  if ( result == QgsSqlExpressionCompiler::Fail ) return QStringLiteral( "Fail" );
  return QStringLiteral( "Other result" );

}

QgsSqlExpressionCompiler::Result QgsDb2ExpressionCompiler::compileNode( const QgsExpressionNode *node, QString &result )
{
  QgsDebugMsg( QStringLiteral( "nodeType: %1" ).arg( nodeType( node ) ) );
  if ( node->nodeType() == QgsExpressionNode::ntColumnRef )
  {
    const QgsExpressionNodeColumnRef *n( static_cast<const QgsExpressionNodeColumnRef *>( node ) );
    QgsDebugMsg( QStringLiteral( "column ref node: " ) + n->dump() );
    // TODO - consider escaped names - not sure how to handle
    QString upperName = n->name().toUpper();
    int idx = mFields.indexFromName( upperName );
    QgsDebugMsg( QStringLiteral( "%1 - %2" ).arg( idx ).arg( upperName ) );
    if ( idx > -1 )
    {
      result = upperName;
      QgsDebugMsg( QStringLiteral( "return Complete" ) );
      return Complete;
    }
    QgsDebugMsg( QStringLiteral( "return Fail" ) );
    return Fail;
  }
// Seemed necessary in initial Python testing but can't identify failing case now
#if 0
  if ( node->nodeType() == QgsExpressionNode::ntLiteral )
  {
    const QgsExpression::NodeLiteral *n = static_cast<const QgsExpression::NodeLiteral *>( node );

    bool ok = false;
    if ( n->dump().toUpper() == "NULL" ) // expression compiler doesn't handle this correctly
    {
      result = "NULL";
      ok = true;
    }
    else
    {
      result = quotedValue( n->value(), ok );
    }
    QgsDebugMsg( QStringLiteral( "ok: %1; literal node: " ).arg( ok ) + n->value().toString() + "; result: " + result );
    QgsDebugMsg( QStringLiteral( "n->dump: " ) + n->dump() );
    QgsDebugMsg( QStringLiteral( "type: %1; typeName: %2" ).arg( n->value().type() ).arg( n->value().typeName() ) );
    if ( ok )
    {
      QgsDebugMsg( QStringLiteral( "return Complete" ) );
      return Complete;
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "return Fail" ) );
      return Fail;
    }

  }
#endif
  if ( node->nodeType() == QgsExpressionNode::ntUnaryOperator )
  {
    const QgsExpressionNodeUnaryOperator *n = static_cast<const QgsExpressionNodeUnaryOperator *>( node );
    Result rr = Fail;
    switch ( n->op() )
    {
      case QgsExpressionNodeUnaryOperator::uoNot:
        rr = compileNode( n->operand(), result );
        if ( "NULL" == result.toUpper() )
        {
          result.clear();
          return Fail;
        }

        result = "NOT " + result;
        QgsDebugMsg( QStringLiteral( "NOT; result: %1; right: %2" ).arg( resultType( rr ), result ) );
        return rr;

      case QgsExpressionNodeUnaryOperator::uoMinus:
        break;
    }
  }

  if ( node->nodeType() == QgsExpressionNode::ntBinaryOperator )
  {
    const QgsExpressionNodeBinaryOperator *bin( static_cast<const QgsExpressionNodeBinaryOperator *>( node ) );
    QString left, right;

    Result lr = compileNode( bin->opLeft(), left );
    Result rr = compileNode( bin->opRight(), right );
    Result compileResult;
    QgsDebugMsg( "left: '" + left + "'; right: '" + right +
                 QString( "'; op: %1; lr: %2; rr: %3" ).arg( bin->op() ).arg( lr ).arg( rr ) );
    if ( lr == Fail || rr == Fail )
      return Fail;
// NULL can not appear on the left, only as part of IS NULL or IS NOT NULL
    if ( "NULL" == left.toUpper() ) return Fail;
// NULL can only be on the right for IS and IS NOT
    if ( "NULL" == right.toUpper() && ( bin->op() != QgsExpressionNodeBinaryOperator::boIs && bin->op() != QgsExpressionNodeBinaryOperator::boIsNot ) )
      return Fail;

    switch ( bin->op() )
    {
      case QgsExpressionNodeBinaryOperator::boMod:
        result = QStringLiteral( "MOD(%1,%2)" ).arg( left, right );
        compileResult = ( lr == Partial || rr == Partial ) ? Partial : Complete;
        QgsDebugMsg( QStringLiteral( "MOD compile status:  %1" ).arg( compileResult ) + "; " + result );
        return compileResult;

      case QgsExpressionNodeBinaryOperator::boPow:
        result = QStringLiteral( "power(%1,%2)" ).arg( left, right );
        compileResult = ( lr == Partial || rr == Partial ) ? Partial : Complete;
        QgsDebugMsg( QStringLiteral( "POWER compile status:  %1" ).arg( compileResult ) + "; " + result );
        return compileResult;

      case QgsExpressionNodeBinaryOperator::boRegexp:
        return Fail; //not supported, regexp syntax is too different to Qt

      case QgsExpressionNodeBinaryOperator::boConcat:
        result = QStringLiteral( "%1 || %2" ).arg( left, right );
        compileResult = ( lr == Partial || rr == Partial ) ? Partial : Complete;
        QgsDebugMsg( QStringLiteral( "CONCAT compile status:  %1" ).arg( compileResult ) + "; " + result );
        return compileResult;

      case QgsExpressionNodeBinaryOperator::boILike:
        QgsDebugMsg( QStringLiteral( "ILIKE is not supported by DB2" ) );
        return Fail;
      /*
        result = QString( "%1 LIKE %2" ).arg( left, right );
        compileResult = (lr == Partial || rr == Partial) ? Partial : Complete;
        QgsDebugMsg(QString("ILIKE compile status:  %1").arg(compileResult) + "; " + result);
        return compileResult;
        */

      case QgsExpressionNodeBinaryOperator::boNotILike:
        QgsDebugMsg( QStringLiteral( "NOT ILIKE is not supported by DB2" ) );
        return Fail;
      /*
        result = QString( "%1 NOT LIKE %2" ).arg( left, right );
        compileResult = (lr == Partial || rr == Partial) ? Partial : Complete;
        QgsDebugMsg(QString("NOT ILIKE compile status:  %1").arg(compileResult) + "; " + result);
        return compileResult;
        */

// We only support IS NULL if the operand on the left is a column
      case QgsExpressionNodeBinaryOperator::boIs:
        if ( "NULL" == right.toUpper() )
        {
          if ( bin->opLeft()->nodeType() != QgsExpressionNode::ntColumnRef )
          {
            QgsDebugMsg( "Failing IS NULL with non-column on left: " + left );
            return Fail;
          }
        }
        break;
// We only support IS NULL if the operand on the left is a column
      case QgsExpressionNodeBinaryOperator::boIsNot:
        if ( "NULL" == right.toUpper() )
        {
          if ( bin->opLeft()->nodeType() != QgsExpressionNode::ntColumnRef )
          {
            QgsDebugMsg( "Failing IS NOT NULL with non-column on left: " + left );
            return Fail;
          }
        }
        break;

      default:
        break;
    }
  }

  //fallback to default handling
  QgsDebugMsg( QStringLiteral( "fallback: %1 - " ).arg( nodeType( node ) ) );
  QgsSqlExpressionCompiler::Result rc = QgsSqlExpressionCompiler::compileNode( node, result );
  QgsDebugMsg( QStringLiteral( "fallback: %1 - " ).arg( resultType( rc ) ) + result );
  return rc;
}

QString QgsDb2ExpressionCompiler::quotedValue( const QVariant &value, bool &ok )
{
  ok = true;
// Seemed necessary in initial Python testing but can't identify failing case now
#if 0
  if ( value.isNull() )
  {
    //no NULL literal support
    ok = false;
    return QString();
  }
#endif
  switch ( value.type() )
  {
    case QVariant::Bool:
      //no boolean literal support in db2, so fake it
      return value.toBool() ? "(1=1)" : "(1=0)";

    default:
      return QgsSqlExpressionCompiler::quotedValue( value, ok );
  }
}
