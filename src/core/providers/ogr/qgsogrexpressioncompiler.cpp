/***************************************************************************
                             qgsogrexpressioncompiler.cpp
                             ----------------------------
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

#include "qgsogrexpressioncompiler.h"
///@cond PRIVATE

#include "qgsexpressionnodeimpl.h"
#include "qgsogrprovider.h"

QgsOgrExpressionCompiler::QgsOgrExpressionCompiler( QgsOgrFeatureSource *source )
  : QgsSqlExpressionCompiler( source->mFields, QgsSqlExpressionCompiler::CaseInsensitiveStringMatch | QgsSqlExpressionCompiler::NoNullInBooleanLogic
                              | QgsSqlExpressionCompiler::NoUnaryMinus | QgsSqlExpressionCompiler::IntegerDivisionResultsInInteger )
  , mSource( source )
{
}


QgsSqlExpressionCompiler::Result QgsOgrExpressionCompiler::compile( const QgsExpression *exp )
{
  //for certain driver types, OGR forwards SQL through to the underlying provider. In these cases
  //the syntax may differ from OGR SQL, so we don't support compilation for these drivers
  //see http://www.gdal.org/ogr_sql.html
  if ( mSource->mDriverName == QLatin1String( "MySQL" ) )
    return Fail;
  else if ( mSource->mDriverName == QLatin1String( "PostgreSQL" ) )
    return Fail;
  else if ( mSource->mDriverName == QLatin1String( "OCI" ) )
    return Fail;
  else if ( mSource->mDriverName == QLatin1String( "ODBC" ) )
    return Fail;
  else if ( mSource->mDriverName == QLatin1String( "PGeo" ) )
    return Fail;
  else if ( mSource->mDriverName == QLatin1String( "MSSQLSpatial" ) )
    return Fail;

  return QgsSqlExpressionCompiler::compile( exp );
}

QgsSqlExpressionCompiler::Result QgsOgrExpressionCompiler::compileNode( const QgsExpressionNode *node, QString &result )
{
  switch ( node->nodeType() )
  {
    case QgsExpressionNode::ntBinaryOperator:
    {
      switch ( static_cast<const QgsExpressionNodeBinaryOperator *>( node )->op() )
      {
        case QgsExpressionNodeBinaryOperator::boILike:
        case QgsExpressionNodeBinaryOperator::boNotILike:
          return Fail; //disabled until https://trac.osgeo.org/gdal/ticket/5132 is fixed

        case QgsExpressionNodeBinaryOperator::boMod:
        case QgsExpressionNodeBinaryOperator::boConcat:
        case QgsExpressionNodeBinaryOperator::boPow:
        case QgsExpressionNodeBinaryOperator::boRegexp:
          return Fail; //not supported by OGR

        default:
          //fallback to default handling
          return QgsSqlExpressionCompiler::compileNode( node, result );
      }
    }

    case QgsExpressionNode::ntFunction:
    case QgsExpressionNode::ntCondition:
      //not support by OGR
      return Fail;

    case QgsExpressionNode::ntUnaryOperator:
    case QgsExpressionNode::ntColumnRef:
    case QgsExpressionNode::ntInOperator:
    case QgsExpressionNode::ntLiteral:
    case QgsExpressionNode::ntIndexOperator:
      break;
  }

  return QgsSqlExpressionCompiler::compileNode( node, result );
}

QString QgsOgrExpressionCompiler::quotedIdentifier( const QString &identifier )
{
  return QgsOgrProviderUtils::quotedIdentifier( identifier.toUtf8(), mSource->mDriverName );
}

QString QgsOgrExpressionCompiler::quotedValue( const QVariant &value, bool &ok )
{
  ok = true;

  if ( value.type() == QVariant::Bool )
  {
    // No support for boolean literals, so fake them
    return value.toBool() ? "(1=1)" : "(1=0)";
  }

  return QgsOgrProviderUtils::quotedValue( value );
}

QString QgsOgrExpressionCompiler::castToReal( const QString &value ) const
{
  return QStringLiteral( "CAST((%1) AS float)" ).arg( value );
}

QString QgsOgrExpressionCompiler::castToInt( const QString &value ) const
{
  return QStringLiteral( "CAST((%1) AS integer)" ).arg( value );
}

///@endcond
