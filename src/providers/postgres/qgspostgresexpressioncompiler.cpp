/***************************************************************************
              qgspostgresexpressioncompiler.cpp
              ----------------------------------------------------
              date                 : 22.4.2015
              copyright            : (C) 2015 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspostgresexpressioncompiler.h"
#include "qgssqlexpressioncompiler.h"
#include "qgsexpressionnodeimpl.h"

QgsPostgresExpressionCompiler::QgsPostgresExpressionCompiler( QgsPostgresFeatureSource *source, bool ignoreStaticNodes )
  : QgsSqlExpressionCompiler( source->mFields, QgsSqlExpressionCompiler::IntegerDivisionResultsInInteger, ignoreStaticNodes )
  , mGeometryColumn( source->mGeometryColumn )
  , mSpatialColType( source->mSpatialColType )
  , mDetectedGeomType( source->mDetectedGeomType )
  , mRequestedGeomType( source->mRequestedGeomType )
  , mRequestedSrid( source->mRequestedSrid )
  , mDetectedSrid( source->mDetectedSrid )
{
}

QString QgsPostgresExpressionCompiler::quotedIdentifier( const QString &identifier )
{
  return QgsPostgresConn::quotedIdentifier( identifier );
}

QString QgsPostgresExpressionCompiler::quotedValue( const QVariant &value, bool &ok )
{
  ok = true;

  // don't use the default QgsPostgresConn::quotedValue handling for double values -- for
  // various reasons it returns them as string values!
  switch ( value.type() )
  {
    case QVariant::Double:
      return value.toString();

    case QVariant::UserType:
      if ( value.userType() == QMetaType::type( "QgsGeometry" ) )
      {
        const QgsGeometry geom = value.value<QgsGeometry>();
        return QString( "ST_GeomFromText('%1',%2)" ).arg( geom.asWkt() ).arg( mRequestedSrid.isEmpty() ? mDetectedSrid : mRequestedSrid );
      }
      break;

    default:
      break;
  }

  return QgsPostgresConn::quotedValue( value );
}

static const QMap<QString, QString> FUNCTION_NAMES_SQL_FUNCTIONS_MAP
{
  { "sqrt", "sqrt" },
  { "radians", "radians" },
  { "degrees", "degrees" },
  { "abs", "abs" },
  { "cos", "cos" },
  { "sin", "sin" },
  { "tan", "tan" },
  { "acos", "acos" },
  { "asin", "asin" },
  { "atan", "atan" },
  { "atan2", "atan2" },
  { "exp", "exp" },
  { "ln", "ln" },
  { "log", "log" },
  { "log10", "log" },
  { "round", "round" },
  { "floor", "floor" },
  { "ceil", "ceil" },
  { "pi", "pi" },
  // geometry functions
  //{ "azimuth", "ST_Azimuth" },
  { "x", "ST_X" },
  { "y", "ST_Y" },
  //{ "z", "ST_Z" },
  //{ "m", "ST_M" },
  { "x_min", "ST_XMin" },
  { "y_min", "ST_YMin" },
  { "x_max", "ST_XMax" },
  { "y_max", "ST_YMax" },
  { "area", "ST_Area" },
  { "perimeter", "ST_Perimeter" },
  { "relate", "ST_Relate" },
  { "disjoint", "ST_Disjoint" },
  { "intersects", "ST_Intersects" },
  //{ "touches", "ST_Touches" },
  { "crosses", "ST_Crosses" },
  { "contains", "ST_Contains" },
  { "overlaps", "ST_Overlaps" },
  { "within", "ST_Within" },
  { "translate", "ST_Translate" },
  { "buffer", "ST_Buffer" },
  { "centroid", "ST_Centroid" },
  { "point_on_surface", "ST_PointOnSurface" },
#if 0
  { "reverse", "ST_Reverse" },
  { "is_closed", "ST_IsClosed" },
  { "convex_hull", "ST_ConvexHull" },
  { "difference", "ST_Difference" },
#endif
  { "distance", "ST_Distance" },
#if 0
  { "intersection", "ST_Intersection" },
  { "sym_difference", "ST_SymDifference" },
  { "combine", "ST_Union" },
  { "union", "ST_Union" },
#endif
  { "geom_from_wkt", "ST_GeomFromText" },
  { "geom_from_gml", "ST_GeomFromGML" },
  { "char", "chr" },
  { "coalesce", "coalesce" },
  { "lower", "lower" },
  { "trim", "trim" },
  { "upper", "upper" },
  { "make_date", "make_date" },
  { "make_time", "make_time" },
  { "make_datetime", "make_timestamp" },
};

QString QgsPostgresExpressionCompiler::sqlFunctionFromFunctionName( const QString &fnName ) const
{
  return FUNCTION_NAMES_SQL_FUNCTIONS_MAP.value( fnName, QString() );
}

QStringList QgsPostgresExpressionCompiler::sqlArgumentsFromFunctionName( const QString &fnName, const QStringList &fnArgs ) const
{
  QStringList args( fnArgs );
  if ( fnName == QLatin1String( "geom_from_wkt" ) )
  {
    args << ( mRequestedSrid.isEmpty() ? mDetectedSrid : mRequestedSrid );
  }
  else if ( fnName == QLatin1String( "geom_from_gml" ) )
  {
    args << ( mRequestedSrid.isEmpty() ? mDetectedSrid : mRequestedSrid );
  }
  else if ( fnName == QLatin1String( "x" ) || fnName == QLatin1String( "y" ) )
  {
    args = QStringList( QStringLiteral( "ST_Centroid(%1)" ).arg( args[0] ) );
  }
  else if ( fnName == QLatin1String( "buffer" ) && args.length() == 2 )
  {
    args << QStringLiteral( "8" );
  }
  else if ( fnName == QLatin1String( "round" ) )
  {
    args[0] = QStringLiteral( "(%1)::numeric" ).arg( args[0] );
  }
  // x and y functions have to be adapted
  return args;
}

QString QgsPostgresExpressionCompiler::castToReal( const QString &value ) const
{
  return QStringLiteral( "((%1)::real)" ).arg( value );
}

QString QgsPostgresExpressionCompiler::castToInt( const QString &value ) const
{
  return QStringLiteral( "((%1)::int)" ).arg( value );
}

QString QgsPostgresExpressionCompiler::castToText( const QString &value ) const
{
  return QStringLiteral( "((%1)::text)" ).arg( value );
}

QgsSqlExpressionCompiler::Result QgsPostgresExpressionCompiler::compileNode( const QgsExpressionNode *node, QString &result )
{
  const QgsSqlExpressionCompiler::Result staticRes = replaceNodeByStaticCachedValueIfPossible( node, result );
  if ( staticRes != Fail )
    return staticRes;

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
#if 0
      /*
       * These methods are tricky
       * QGIS expression versions of these return ellipsoidal measurements
       * based on the project settings, and also convert the result to the
       * units specified in project properties.
       */
      else if ( fd->name() == "$area" )
      {
        result = QStringLiteral( "ST_Area(%1)" ).arg( quotedIdentifier( mGeometryColumn ) );
        return Complete;
      }
      else if ( fd->name() == "$length" )
      {
        result = QStringLiteral( "ST_Length(%1)" ).arg( quotedIdentifier( mGeometryColumn ) );
        return Complete;
      }
      else if ( fd->name() == "$perimeter" )
      {
        result = QStringLiteral( "ST_Perimeter(%1)" ).arg( quotedIdentifier( mGeometryColumn ) );
        return Complete;
      }
      else if ( fd->name() == "$x" )
      {
        result = QStringLiteral( "ST_X(%1)" ).arg( quotedIdentifier( mGeometryColumn ) );
        return Complete;
      }
      else if ( fd->name() == "$y" )
      {
        result = QStringLiteral( "ST_Y(%1)" ).arg( quotedIdentifier( mGeometryColumn ) );
        return Complete;
      }
#endif
      FALLTHROUGH
    }

    default:
      break;
  }

  return QgsSqlExpressionCompiler::compileNode( node, result );
}
