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

QgsOgrExpressionCompiler::QgsOgrExpressionCompiler( QgsOgrFeatureSource *source, bool ignoreStaticNodes )
  : QgsSqlExpressionCompiler( source->mFields, QgsSqlExpressionCompiler::CaseInsensitiveStringMatch | QgsSqlExpressionCompiler::NoNullInBooleanLogic
                              | QgsSqlExpressionCompiler::NoUnaryMinus | QgsSqlExpressionCompiler::IntegerDivisionResultsInInteger, ignoreStaticNodes )
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
  QgsSqlExpressionCompiler::Result staticRes = replaceNodeByStaticCachedValueIfPossible( node, result );
  if ( staticRes != Fail )
    return staticRes;

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
        return QgsSqlExpressionCompiler::compileNode( node, result );
      }
      //generally not support by OGR
      return Fail;
    }

    case QgsExpressionNode::ntCondition:
      //not support by OGR
      return Fail;

    case QgsExpressionNode::ntUnaryOperator:
    case QgsExpressionNode::ntColumnRef:
    case QgsExpressionNode::ntInOperator:
    case QgsExpressionNode::ntLiteral:
    case QgsExpressionNode::ntIndexOperator:
    case QgsExpressionNode::ntBetweenOperator:
      break;
  }

  return QgsSqlExpressionCompiler::compileNode( node, result );
}

QString QgsOgrExpressionCompiler::sqlFunctionFromFunctionName( const QString &fnName ) const
{
  static const QMap<QString, QString> FN_NAMES
  {
    { "make_datetime", "" },
    { "make_date", "" },
    { "make_time", "" },
  };

  return FN_NAMES.value( fnName, QString() );
}

QStringList QgsOgrExpressionCompiler::sqlArgumentsFromFunctionName( const QString &fnName, const QStringList &fnArgs ) const
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
