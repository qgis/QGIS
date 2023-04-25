/***************************************************************************
   qgsredshiftexpressioncompiler.cpp
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsredshiftexpressioncompiler.h"

#include "qgsexpressionnodeimpl.h"
#include "qgssqlexpressioncompiler.h"

QgsRedshiftExpressionCompiler::QgsRedshiftExpressionCompiler( QgsRedshiftFeatureSource *source )
  : QgsSqlExpressionCompiler( source->mFields, QgsSqlExpressionCompiler::IntegerDivisionResultsInInteger ),
    mGeometryColumn( source->mGeometryColumn ), mSpatialColType( source->mSpatialColType ),
    mDetectedGeomType( source->mDetectedGeomType ), mRequestedGeomType( source->mRequestedGeomType ),
    mRequestedSrid( source->mRequestedSrid ), mDetectedSrid( source->mDetectedSrid )
{
}

QString QgsRedshiftExpressionCompiler::quotedIdentifier( const QString &identifier )
{
  return QgsRedshiftConn::quotedIdentifier( identifier );
}

QString QgsRedshiftExpressionCompiler::quotedValue( const QVariant &value, bool &ok )
{
  ok = true;
  return QgsRedshiftConn::quotedValue( value );
}

static const QMap<QString, QString> FUNCTION_NAMES_SQL_FUNCTIONS_MAP
{
  // TODO(marcel): add back some of these functions when Redshift supports them
  {"sqrt", "sqrt"},
  {"radians", "radians"},
  {"degrees", "degrees"},
  {"abs", "abs"},
  {"cos", "cos"},
  {"sin", "sin"},
  {"tan", "tan"},
  {"acos", "acos"},
  {"asin", "asin"},
  {"atan", "atan"},
  {"atan2", "atan2"},
  {"exp", "exp"},
  {"ln", "ln"},
  {"log10", "log"},
  {"round", "round"},
  {"floor", "floor"},
  {"ceil", "ceil"},
  {"pi", "pi"},
  // TODO(marcel): QGIS x and y can accept any kind of
  // geometry, by returning the coordinates of the centroid. Re-add them
  // when st_centroid is added to Redshift
  //{ "x", "ST_X" },
  //{ "y", "ST_Y" },
  //{ "z", "ST_Z" }
  //{ "m", "ST_M" },
  {"x_min", "ST_XMin"},
  {"y_min", "ST_YMin"},
  {"x_max", "ST_XMax"},
  {"y_max", "ST_YMax"},
  {"area", "ST_Area"},
  {"perimeter", "ST_Perimeter"},
  {"disjoint", "ST_Disjoint"},
  {"intersects", "ST_Intersects"},
  {"crosses", "ST_Crosses"},
  {"contains", "ST_Contains"},
  {"bounday", "ST_Boundary"},
  {"within", "ST_Within"},
  {"reverse", "ST_Reverse" },
  {"convex_hull", "ST_ConvexHull" },
  {"distance", "ST_Distance" },
  {"geom_from_wkt", "ST_GeomFromText"},
  {"geom_to_wkt", "ST_AsText"},
  {"char", "chr"}, {"coalesce", "coalesce"},
  {"lower", "lower"},
  {"trim", "trim"},
  {"upper", "upper"},
  {"make_date", "cast"}, // cast to respective date/time/timestamp
  {"make_time", "cast"},
  {"make_datetime", "cast"}
};

QString QgsRedshiftExpressionCompiler::sqlFunctionFromFunctionName( const QString &fnName ) const
{
  return FUNCTION_NAMES_SQL_FUNCTIONS_MAP.value( fnName, QString() );
}

QStringList QgsRedshiftExpressionCompiler::sqlArgumentsFromFunctionName( const QString &fnName,
    const QStringList &fnArgs ) const
{
  QStringList args( fnArgs );
  if ( fnName == QLatin1String( "geom_from_wkt" ) )
  {
    args << ( mRequestedSrid.isEmpty() ? mDetectedSrid : mRequestedSrid );
  }
  else if ( fnName == QLatin1String( "make_date" ) )
  {
    QString date_cast = "'" + fnArgs.join( '-' ) + "' AS date";
    args = QStringList() << date_cast;
  }
  else if ( fnName == QLatin1String( "make_time" ) )
  {
    QString time_cast = "'" + fnArgs.join( ':' ) + "' AS time";
    args = QStringList() << time_cast;
  }
  else if ( fnName == QLatin1String( "make_datetime" ) )
  {
    QString timestamp_cast = "'" + fnArgs.mid( 0, 3 ).join( '-' ) + " " + fnArgs.mid( 3 ).join( ":" ) + "' as timestamp";
    args = QStringList() << timestamp_cast;
  }
  // TODO(reflectored): Redshift ceil and floor do not accept CSTRING for now. We quote double
  // in QgsRedshiftConn so that it will be implicitly converted to the column's type
  // on comparison.
  else if ( ( fnName == QLatin1String( "ceil" ) || fnName == QLatin1String( "floor" ) ) &&
            fnArgs[0].front() == '\'' &&  fnArgs[0].back() == '\'' )
  {
    QString double_cast = fnArgs[0];
    args = QStringList() << double_cast.append( "::float8" );
  }
  return args;
}

QString QgsRedshiftExpressionCompiler::castToReal( const QString &value ) const
{
  return QStringLiteral( "((%1)::real)" ).arg( value );
}

QString QgsRedshiftExpressionCompiler::castToInt( const QString &value ) const
{
  return QStringLiteral( "((%1)::int)" ).arg( value );
}

QString QgsRedshiftExpressionCompiler::castToText( const QString &value ) const
{
  return QStringLiteral( "((%1)::text)" ).arg( value );
}

QgsSqlExpressionCompiler::Result QgsRedshiftExpressionCompiler::compileNode( const QgsExpressionNode *node,
    QString &result )
{
  switch ( node->nodeType() )
  {
    case QgsExpressionNode::ntFunction:
    {
      const QgsExpressionNodeFunction *n = static_cast<const QgsExpressionNodeFunction *>( node );

      QgsExpressionFunction *fd = QgsExpression::Functions()[n->fnIndex()];
      if ( fd->name() == QLatin1String( "$geometry" ) )
      {
        result = quotedIdentifier( mGeometryColumn );
        return Complete;
      }
      break;
    }
    default:
      break;
  }

  return QgsSqlExpressionCompiler::compileNode( node, result );
}
