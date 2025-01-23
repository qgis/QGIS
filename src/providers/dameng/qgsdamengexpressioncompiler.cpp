/***************************************************************************
    qgsdamengexpressioncompiler.cpp
    -------------------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdamengexpressioncompiler.h"
#include "qgsexpressionutils.h"
#include "qgssqlexpressioncompiler.h"
#include "qgsexpressionnodeimpl.h"

QgsDamengExpressionCompiler::QgsDamengExpressionCompiler( QgsDamengFeatureSource *source, bool ignoreStaticNodes )
  : QgsSqlExpressionCompiler( source->mFields, QgsSqlExpressionCompiler::IntegerDivisionResultsInInteger, ignoreStaticNodes )
  , mGeometryColumn( source->mGeometryColumn )
  , mSpatialColType( source->mSpatialColType )
  , mDetectedGeomType( source->mDetectedGeomType )
  , mRequestedGeomType( source->mRequestedGeomType )
  , mRequestedSrid( source->mRequestedSrid )
  , mDetectedSrid( source->mDetectedSrid )
{
}

QString QgsDamengExpressionCompiler::quotedIdentifier( const QString &identifier )
{
  return QgsDamengConn::quotedIdentifier( identifier );
}

QString QgsDamengExpressionCompiler::quotedValue( const QVariant &value, bool &ok )
{
  ok = true;

  switch ( value.userType() )
  {
    case QMetaType::Type::Bool:
      //no boolean literal support in Dameng, so fake it
      return value.toBool() ? QStringLiteral( "( 1=1 )" ) : QStringLiteral( "( 1=0 )" );

    default:
      QgsGeometry geom = QgsExpressionUtils::getGeometry( value, nullptr );
      if ( geom.isNull() )
        break;
      return QString( "DMGEO2.ST_GeomFromText('%1',%2)" ).arg( geom.asWkt() ).arg( mRequestedSrid.isEmpty() ? mDetectedSrid : mRequestedSrid );
  }

  return QgsDamengConn::quotedValue( value );
}

static const QMap<QString, QString> FUNCTION_NAMES_SQL_FUNCTIONS_MAP {
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
  { "log10", "log10" },
  { "round", "round" },
  { "floor", "floor" },
  { "ceil", "ceil" },
  { "pi", "pi" },
  // geometry functions
  { "x", "DMGEO2.ST_X" },
  { "y", "DMGEO2.ST_Y" },
  { "x_min", "DMGEO2.ST_XMin" },
  { "y_min", "DMGEO2.ST_YMin" },
  { "x_max", "DMGEO2.ST_XMax" },
  { "y_max", "DMGEO2.ST_YMax" },
  { "area", "DMGEO2.ST_Area" },
  { "perimeter", "DMGEO2.ST_Perimeter" },
  { "relate", "DMGEO2.ST_Relate" },
  { "disjoint", "DMGEO2.ST_Disjoint" },
  { "intersects", "DMGEO2.ST_Intersects" },
  { "crosses", "DMGEO2.ST_Crosses" },
  { "contains", "DMGEO2.ST_Contains" },
  { "overlaps", "DMGEO2.ST_Overlaps" },
  { "within", "DMGEO2.ST_Within" },
  { "translate", "DMGEO2.ST_Translate" },
  { "buffer", "DMGEO2.ST_Buffer" },
  { "centroid", "DMGEO2.ST_Centroid" },
  { "point_on_surface", "DMGEO2.ST_PointOnSurface" },
#if 0
  { "reverse", "DMGEO2.ST_Reverse" },
  { "is_closed", "DMGEO2.ST_IsClosed" },
  { "convex_hull", "DMGEO2.ST_ConvexHull" },
  { "difference", "DMGEO2.ST_Difference" },
#endif
  { "distance", "DMGEO2.ST_Distance" },
#if 0
  { "intersection", "DMGEO2.ST_Intersection" },
  { "sym_difference", "DMGEO2.ST_SymDifference" },
  { "combine", "DMGEO2.ST_Union" },
  { "union", "DMGEO2.ST_Union" },
#endif
  { "geom_from_wkt", "DMGEO2.ST_GeomFromText" },
  { "geom_from_gml", "DMGEO2.ST_GeomFromGML" },
  { "char", "chr" },
  { "coalesce", "coalesce" },
  { "lower", "lower" },
  { "trim", "trim" },
  { "upper", "upper" },
  { "make_datetime", "" },
};

QString QgsDamengExpressionCompiler::sqlFunctionFromFunctionName( const QString &fnName ) const
{
  return FUNCTION_NAMES_SQL_FUNCTIONS_MAP.value( fnName, QString() );
}

QStringList QgsDamengExpressionCompiler::sqlArgumentsFromFunctionName( const QString &fnName, const QStringList &fnArgs ) const
{
  QStringList args( fnArgs );
  if ( fnName == QLatin1String( "make_datetime" ) )
  {
    args = QStringList( QStringLiteral( "TIMESTAMP '%1-%2-%3 %4:%5:%6'" ).arg( args[0].rightJustified( 4, '0' ) ).arg( args[1].rightJustified( 2, '0' ) ).arg( args[2].rightJustified( 2, '0' ) ).arg( args[3].rightJustified( 2, '0' ) ).arg( args[4].rightJustified( 2, '0' ) ).arg( args[5].rightJustified( 2, '0' ) ) );
  }
  else if ( fnName == QLatin1String( "geom_from_wkt" ) )
  {
    args << ( mRequestedSrid.isEmpty() ? mDetectedSrid : mRequestedSrid );
  }
  else if ( fnName == QLatin1String( "geom_from_gml" ) )
  {
    args << ( mRequestedSrid.isEmpty() ? mDetectedSrid : mRequestedSrid );
  }
  else if ( fnName == QLatin1String( "x" ) || fnName == QLatin1String( "y" ) )
  {
    args = QStringList( QStringLiteral( "DMGEO2.ST_Centroid(%1)" ).arg( args[0] ) );
  }
  else if ( fnName == QLatin1String( "buffer" ) && args.length() == 2 )
  {
    args << QStringLiteral( "8" );
  }
  else if ( fnName == QLatin1String( "round" ) )
  {
    args[0] = QStringLiteral( "cast( %1 as numeric )" ).arg( args[0] );
  }
  // x and y functions have to be adapted
  return args;
}

QString QgsDamengExpressionCompiler::castToReal( const QString &value ) const
{
  return QStringLiteral( "cast( %1 as real )" ).arg( value );
}

QString QgsDamengExpressionCompiler::castToInt( const QString &value ) const
{
  return QStringLiteral( "cast( %1 as int )" ).arg( value );
}

QString QgsDamengExpressionCompiler::castToText( const QString &value ) const
{
  return QStringLiteral( "cast( %1 as text )" ).arg( value );
}

QgsSqlExpressionCompiler::Result QgsDamengExpressionCompiler::compileNode( const QgsExpressionNode *node, QString &result )
{
  const QgsSqlExpressionCompiler::Result staticRes = replaceNodeByStaticCachedValueIfPossible( node, result );
  if ( staticRes != Fail )
    return staticRes;

  switch ( node->nodeType() )
  {
    case QgsExpressionNode::ntBinaryOperator:
    {
      const QgsExpressionNodeBinaryOperator *bin( static_cast< const QgsExpressionNodeBinaryOperator *>( node ) );

      switch ( bin->op() )
      {
        case QgsExpressionNodeBinaryOperator::boConcat:
          return Fail;

        case QgsExpressionNodeBinaryOperator::boPow:
        case QgsExpressionNodeBinaryOperator::boRegexp:
        case QgsExpressionNodeBinaryOperator::boILike:
        case QgsExpressionNodeBinaryOperator::boNotILike:
        case QgsExpressionNodeBinaryOperator::boMod:
        case QgsExpressionNodeBinaryOperator::boIntDiv:
        {
          QString op1, op2;

          if ( compileNode( bin->opLeft(), op1 ) != Complete || compileNode( bin->opRight(), op2 ) != Complete )
            return Fail;

          switch ( bin->op() )
          {
            case QgsExpressionNodeBinaryOperator::boPow:
              result = QStringLiteral( "power(%1,%2)" ).arg( op1, op2 );
              return Complete;

            case QgsExpressionNodeBinaryOperator::boRegexp:
              result = QStringLiteral( "regexp_like(%1,%2)" ).arg( op1, op2 );
              return Complete;

            case QgsExpressionNodeBinaryOperator::boILike:
              result = QStringLiteral( "lower(%1) LIKE lower(%2) ESCAPE '\\'" ).arg( op1, op2 );
              return Complete;

            case QgsExpressionNodeBinaryOperator::boNotILike:
              result = QStringLiteral( "NOT lower(%1) LIKE lower(%2) ESCAPE '\\'" ).arg( op1, op2 );
              return Complete;

            case QgsExpressionNodeBinaryOperator::boIntDiv:
              result = QStringLiteral( "FLOOR(%1 / %2)" ).arg( op1, op2 );
              return Complete;


            case QgsExpressionNodeBinaryOperator::boMod:
              result = QStringLiteral( "MOD(%1,%2)" ).arg( op1, op2 );
              return Complete;

            default:
              break;
          }
          break; // no warnings
        }

        default:
          break;
      }
      break;
    }

    case QgsExpressionNode::ntFunction:
    {
      const QgsExpressionNodeFunction *n = static_cast< const QgsExpressionNodeFunction *>( node );

      QgsExpressionFunction *fd = QgsExpression::Functions()[n->fnIndex()];
      if ( fd->name() == QLatin1String( "$geometry" ) )
      {
        result = quotedIdentifier( mGeometryColumn );
        return Complete;
      }
      if ( fd->name() == QLatin1String( "make_datetime" ) )
      {
        const auto constList = n->args()->list();
        for ( const QgsExpressionNode *ln : constList )
        {
          if ( ln->nodeType() != QgsExpressionNode::ntLiteral )
            return Fail;
        }
      }
    }

    default:
      break;
  }

  return QgsSqlExpressionCompiler::compileNode( node, result );
}
