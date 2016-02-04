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
    : QgsSqlExpressionCompiler( source->mFields,
                                QgsSqlExpressionCompiler::LikeIsCaseInsensitive | QgsSqlExpressionCompiler::CaseInsensitiveStringMatch )
{

}

QgsSqlExpressionCompiler::Result QgsDb2ExpressionCompiler::compileNode( const QgsExpression::Node* node, QString& result )
{
  if ( node->nodeType() == QgsExpression::ntColumnRef )
  {
     const QgsExpression::NodeColumnRef *bin( static_cast<const QgsExpression::NodeColumnRef*>( node ) );
     QgsDebugMsg(QString("node: ") + bin->dump());
     // TODO - consider escaped names - not sure how to handle
     QString upperName = bin->name().toUpper(); 
     int idx = mFields.indexFromName( upperName);
     if (idx > -1) {
       result = upperName;
       return Complete;
     }
     return Fail;
 }
  if ( node->nodeType() == QgsExpression::ntBinaryOperator )
  {
    const QgsExpression::NodeBinaryOperator *bin( static_cast<const QgsExpression::NodeBinaryOperator*>( node ) );
    QString op1, op2;

    Result result1 = compileNode( bin->opLeft(), op1 );
    Result result2 = compileNode( bin->opRight(), op2 );
    if ( result1 == Fail || result2 == Fail )
      return Fail;

    switch ( bin->op() )
    {
      case QgsExpression::boPow:
        result = QString( "power(%1,%2)" ).arg( op1, op2 );
        return result1 == Partial || result2 == Partial ? Partial : Complete;

      case QgsExpression::boRegexp:
        return Fail; //not supported, regexp syntax is too different to Qt

      case QgsExpression::boConcat:
        result = QString( "%1 + %2" ).arg( op1, op2 );
        return result1 == Partial || result2 == Partial ? Partial : Complete;

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
