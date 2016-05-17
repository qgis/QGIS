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
#include "qgsogrprovider.h"

QgsOgrExpressionCompiler::QgsOgrExpressionCompiler( QgsOgrFeatureSource* source )
    : QgsSqlExpressionCompiler( source->mFields, QgsSqlExpressionCompiler::CaseInsensitiveStringMatch | QgsSqlExpressionCompiler::NoNullInBooleanLogic
                                | QgsSqlExpressionCompiler::NoUnaryMinus )
    , mSource( source )
{
}


QgsSqlExpressionCompiler::Result QgsOgrExpressionCompiler::compile( const QgsExpression* exp )
{
  //for certain driver types, OGR forwards SQL through to the underlying provider. In these cases
  //the syntax may differ from OGR SQL, so we don't support compilation for these drivers
  //see http://www.gdal.org/ogr_sql.html
  if ( mSource->mDriverName == "MySQL" )
    return Fail;
  else if ( mSource->mDriverName == "PostgreSQL" )
    return Fail;
  else if ( mSource->mDriverName == "OCI" )
    return Fail;
  else if ( mSource->mDriverName == "ODBC" )
    return Fail;
  else if ( mSource->mDriverName == "PGeo" )
    return Fail;
  else if ( mSource->mDriverName == "MSSQLSpatial" )
    return Fail;

  return QgsSqlExpressionCompiler::compile( exp );
}

QgsSqlExpressionCompiler::Result QgsOgrExpressionCompiler::compileNode( const QgsExpression::Node* node, QString& result )
{
  switch ( node->nodeType() )
  {
    case QgsExpression::ntBinaryOperator:
    {
      switch ( static_cast<const QgsExpression::NodeBinaryOperator*>( node )->op() )
      {
        case QgsExpression::boILike:
        case QgsExpression::boNotILike:
          return Fail; //disabled until https://trac.osgeo.org/gdal/ticket/5132 is fixed

        case QgsExpression::boDiv:
        case QgsExpression::boMod:
        case QgsExpression::boConcat:
        case QgsExpression::boIntDiv:
        case QgsExpression::boPow:
        case QgsExpression::boRegexp:
          return Fail; //not supported by OGR

        default:
          //fallback to default handling
          return QgsSqlExpressionCompiler::compileNode( node, result );
      }
    }

    case QgsExpression::ntFunction:
    case QgsExpression::ntCondition:
      //not support by OGR
      return Fail;

    case QgsExpression::ntUnaryOperator:
    case QgsExpression::ntColumnRef:
    case QgsExpression::ntInOperator:
    case QgsExpression::ntLiteral:
      break;
  }

  return QgsSqlExpressionCompiler::compileNode( node, result );
}

QString QgsOgrExpressionCompiler::quotedIdentifier( const QString& identifier )
{
  return mSource->mProvider->quotedIdentifier( identifier.toUtf8() );
}

QString QgsOgrExpressionCompiler::quotedValue( const QVariant& value, bool& ok )
{
  ok = true;

  if ( value.type() == QVariant::Bool )
  {
    // No support for boolean literals, so fake them
    return value.toBool() ? "(1=1)" : "(1=0)";
  }

  return QgsOgrProviderUtils::quotedValue( value );
}
