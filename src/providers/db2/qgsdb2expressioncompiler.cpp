/***************************************************************************
  qgsdb2expressioncompiler.cpp - DB2 expression compiler
  --------------------------------------
  Date      : 2016-01-27
  Copyright : (C) 2016 by David Adler
                          Shirley Xiao, David Nguyen
  Email     : dadler at adtechgeospatial.com
              xiaoshir at us.ibm.com, nguyend at us.ibm.com
/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/

#include "Qgsdb2expressioncompiler.h"

QgsDb2ExpressionCompiler::QgsDb2ExpressionCompiler( QgsDb2FeatureSource* source )
    : QgsSqlExpressionCompiler( source->mFields
                                )
{

}

QgsSqlExpressionCompiler::Result QgsDb2ExpressionCompiler::compileNode( const QgsExpression::Node* node, QString& result )
{
    QgsDebugMsg(QString("nodeType: %1").arg(node->nodeType())); 
  if ( node->nodeType() == QgsExpression::ntColumnRef )
  {
     const QgsExpression::NodeColumnRef *n( static_cast<const QgsExpression::NodeColumnRef*>( node ) );
     QgsDebugMsg(QString("column ref node: ") + n->dump());
     // TODO - consider escaped names - not sure how to handle
     QString upperName = n->name().toUpper(); 
     int idx = mFields.indexFromName( upperName);
     if (idx > -1) {
       result = upperName;
       return Complete;
     }
     return Fail;
 }
  if ( node->nodeType() == QgsExpression::ntLiteral )
  {
     const QgsExpression::NodeLiteral* n = static_cast<const QgsExpression::NodeLiteral*>( node );

     bool ok = false;
     if (n->dump().toUpper() == "NULL") // expression compiler doesn't handle this correctly
     {
       result = "NULL";
       ok = true;
     } else {
       result = quotedValue( n->value(), ok );
     }
     QgsDebugMsg(QString("ok: %1; literal node: ").arg(ok) + n->value().toString() + "; result: " + result);
     QgsDebugMsg(QString("n->dump: ") + n->dump()); 
     QgsDebugMsg( QString( "type: %1; typeName: %2" ).arg( n->value().type() ).arg( n->value().typeName() ) );
     return ok ? Complete : Fail;
 } 
 
  if ( node->nodeType() == QgsExpression::ntBinaryOperator )
  {
    const QgsExpression::NodeBinaryOperator *bin( static_cast<const QgsExpression::NodeBinaryOperator*>( node ) );
    QString left, right;

    Result lr = compileNode( bin->opLeft(), left );
    Result rr = compileNode( bin->opRight(), right );
    Result compileResult;
    QgsDebugMsg("left: '" + left + "'; right: '" + right + 
            QString("'; op: %1; lr: %2; rr: %3").arg(bin->op()).arg(lr).arg(rr));    
    if ( lr == Fail || rr == Fail )
      return Fail;
 // NULL can not appear on the left, only as part of IS NULL or IS NOT NULL
    if ("NULL" == left.toUpper()) return Fail;
// NULL can only be on the right for IS and IS NOT    
    if ("NULL" == right.toUpper() && (bin->op() != QgsExpression::boIs && bin->op() != QgsExpression::boIsNot))
    return Fail;
    
    switch ( bin->op() )
    {
      case QgsExpression::boMod:
        result = QString( "MOD(%1,%2)" ).arg( left, right );
        compileResult = (lr == Partial || rr == Partial) ? Partial : Complete;
        QgsDebugMsg(QString("MOD compile status:  %1").arg(compileResult) + "; " + result);
        return compileResult;    
    
      case QgsExpression::boPow:
        result = QString( "power(%1,%2)" ).arg( left, right );
        compileResult = (lr == Partial || rr == Partial) ? Partial : Complete;
        QgsDebugMsg(QString("POWER compile status:  %1").arg(compileResult) + "; " + result);
        return compileResult;

      case QgsExpression::boRegexp:
        return Fail; //not supported, regexp syntax is too different to Qt

      case QgsExpression::boConcat:
        result = QString( "%1 || %2" ).arg( left, right );
        compileResult = (lr == Partial || rr == Partial) ? Partial : Complete;
        QgsDebugMsg(QString("CONCAT compile status:  %1").arg(compileResult) + "; " + result);
        return compileResult;

      case QgsExpression::boILike:
        QgsDebugMsg("ILIKE is not supported by SQL");
        return Fail;
      /*
        result = QString( "%1 LIKE %2" ).arg( left, right );
        compileResult = (lr == Partial || rr == Partial) ? Partial : Complete;
        QgsDebugMsg(QString("ILIKE compile status:  %1").arg(compileResult) + "; " + result);
        return compileResult;
        */

      case QgsExpression::boNotILike:
        QgsDebugMsg("NOT ILIKE is not supported by SQL");
        return Fail;      
      /*
        result = QString( "%1 NOT LIKE %2" ).arg( left, right );
        compileResult = (lr == Partial || rr == Partial) ? Partial : Complete;
        QgsDebugMsg(QString("NOT ILIKE compile status:  %1").arg(compileResult) + "; " + result);
        return compileResult;
        */

// We only support IS NULL if the operand on the left is a column        
      case QgsExpression::boIs:
        if ("NULL" == right.toUpper())
        {
          if (bin->opLeft()->nodeType() != QgsExpression::ntColumnRef)
          {
          QgsDebugMsg("Failing IS NULL with non-column on left: " + left);
            return Fail;
          }
        }
        break;
// We only support IS NULL if the operand on the left is a column        
      case QgsExpression::boIsNot:
        if ("NULL" == right.toUpper())
        {
          if (bin->opLeft()->nodeType() != QgsExpression::ntColumnRef)
          {
          QgsDebugMsg("Failing IS NOT NULL with non-column on left: " + left);
            return Fail;
          }
        }
        break;        
        
      default:
        break;
    }
  }

  //fallback to default handling
  return QgsSqlExpressionCompiler::compileNode( node, result );
}

QString QgsDb2ExpressionCompiler::quotedValue( const QVariant& value, bool& ok )
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
      //no boolean literal support in db2, so fake it
      return value.toBool() ? "(1=1)" : "(1=0)";

    default:
      return QgsSqlExpressionCompiler::quotedValue( value, ok );
  }
}
