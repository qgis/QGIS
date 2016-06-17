/***************************************************************************
                              qgsexpression.cpp
                             -------------------
    begin                : August 2011
    copyright            : (C) 2011 Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexpression.h"

#include <QtDebug>
#include <QDomDocument>
#include <QDate>
#include <QRegExp>
#include <QColor>
#include <QUuid>
#include <QMutex>

#include <math.h>
#include <limits>

#include "qgsdistancearea.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsgeometryengine.h"
#include "qgsgeometryutils.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsogcutils.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorcolorrampv2.h"
#include "qgsstylev2.h"
#include "qgsexpressioncontext.h"
#include "qgsproject.h"
#include "qgsstringutils.h"
#include "qgsgeometrycollectionv2.h"
#include "qgspointv2.h"
#include "qgspolygonv2.h"
#include "qgsmultipointv2.h"
#include "qgsmultilinestringv2.h"
#include "qgscurvepolygonv2.h"
#include "qgsexpressionprivate.h"
#include "qgsexpressionsorter.h"
#include "qgscrscache.h"

#if QT_VERSION < 0x050000
#include <qtextdocument.h>
#endif

// from parser
extern QgsExpression::Node* parseExpression( const QString& str, QString& parserErrorMsg );

///////////////////////////////////////////////
// three-value logic

enum TVL
{
  False,
  True,
  Unknown
};

static TVL AND[3][3] =
{
  // false  true    unknown
  { False, False,   False },   // false
  { False, True,    Unknown }, // true
  { False, Unknown, Unknown }  // unknown
};

static TVL OR[3][3] =
{
  { False,   True, Unknown },  // false
  { True,    True, True },     // true
  { Unknown, True, Unknown }   // unknown
};

static TVL NOT[3] = { True, False, Unknown };

static QVariant tvl2variant( TVL v )
{
  switch ( v )
  {
    case False:
      return 0;
    case True:
      return 1;
    case Unknown:
    default:
      return QVariant();
  }
}

#define TVL_True     QVariant(1)
#define TVL_False    QVariant(0)
#define TVL_Unknown  QVariant()

///////////////////////////////////////////////
// QVariant checks and conversions

inline bool isIntSafe( const QVariant& v )
{
  if ( v.type() == QVariant::Int ) return true;
  if ( v.type() == QVariant::UInt ) return true;
  if ( v.type() == QVariant::LongLong ) return true;
  if ( v.type() == QVariant::ULongLong ) return true;
  if ( v.type() == QVariant::Double ) return false;
  if ( v.type() == QVariant::String ) { bool ok; v.toString().toInt( &ok ); return ok; }
  return false;
}
inline bool isDoubleSafe( const QVariant& v )
{
  if ( v.type() == QVariant::Double ) return true;
  if ( v.type() == QVariant::Int ) return true;
  if ( v.type() == QVariant::UInt ) return true;
  if ( v.type() == QVariant::LongLong ) return true;
  if ( v.type() == QVariant::ULongLong ) return true;
  if ( v.type() == QVariant::String )
  {
    bool ok;
    double val = v.toString().toDouble( &ok );
    ok = ok && qIsFinite( val ) && !qIsNaN( val );
    return ok;
  }
  return false;
}

inline bool isDateTimeSafe( const QVariant& v )
{
  return v.type() == QVariant::DateTime || v.type() == QVariant::Date ||
         v.type() == QVariant::Time;
}

inline bool isIntervalSafe( const QVariant& v )
{
  if ( v.canConvert<QgsInterval>() )
  {
    return true;
  }

  if ( v.type() == QVariant::String )
  {
    return QgsInterval::fromString( v.toString() ).isValid();
  }
  return false;
}

inline bool isNull( const QVariant& v ) { return v.isNull(); }

///////////////////////////////////////////////
// evaluation error macros

#define ENSURE_NO_EVAL_ERROR   {  if (parent->hasEvalError()) return QVariant(); }
#define SET_EVAL_ERROR(x)   { parent->setEvalErrorString(x); return QVariant(); }

///////////////////////////////////////////////
// operators

const char* QgsExpression::BinaryOperatorText[] =
{
  // this must correspond (number and order of element) to the declaration of the enum BinaryOperator
  "OR", "AND",
  "=", "<>", "<=", ">=", "<", ">", "~", "LIKE", "NOT LIKE", "ILIKE", "NOT ILIKE", "IS", "IS NOT",
  "+", "-", "*", "/", "//", "%", "^",
  "||"
};

const char* QgsExpression::UnaryOperatorText[] =
{
  // this must correspond (number and order of element) to the declaration of the enum UnaryOperator
  "NOT", "-"
};

///////////////////////////////////////////////
// functions

// implicit conversion to string
static QString getStringValue( const QVariant& value, QgsExpression* )
{
  return value.toString();
}

static double getDoubleValue( const QVariant& value, QgsExpression* parent )
{
  bool ok;
  double x = value.toDouble( &ok );
  if ( !ok || qIsNaN( x ) || !qIsFinite( x ) )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to double" ).arg( value.toString() ) );
    return 0;
  }
  return x;
}

static int getIntValue( const QVariant& value, QgsExpression* parent )
{
  bool ok;
  qint64 x = value.toLongLong( &ok );
  if ( ok && x >= std::numeric_limits<int>::min() && x <= std::numeric_limits<int>::max() )
  {
    return x;
  }
  else
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to int" ).arg( value.toString() ) );
    return 0;
  }
}

static QDateTime getDateTimeValue( const QVariant& value, QgsExpression* parent )
{
  QDateTime d = value.toDateTime();
  if ( d.isValid() )
  {
    return d;
  }
  else
  {
    QTime t = value.toTime();
    if ( t.isValid() )
    {
      return QDateTime( QDate( 1, 1, 1 ), t );
    }

    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to DateTime" ).arg( value.toString() ) );
    return QDateTime();
  }
}

static QDate getDateValue( const QVariant& value, QgsExpression* parent )
{
  QDate d = value.toDate();
  if ( d.isValid() )
  {
    return d;
  }
  else
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to Date" ).arg( value.toString() ) );
    return QDate();
  }
}

static QTime getTimeValue( const QVariant& value, QgsExpression* parent )
{
  QTime t = value.toTime();
  if ( t.isValid() )
  {
    return t;
  }
  else
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to Time" ).arg( value.toString() ) );
    return QTime();
  }
}

static QgsInterval getInterval( const QVariant& value, QgsExpression* parent, bool report_error = false )
{
  if ( value.canConvert<QgsInterval>() )
    return value.value<QgsInterval>();

  QgsInterval inter = QgsInterval::fromString( value.toString() );
  if ( inter.isValid() )
  {
    return inter;
  }
  // If we get here then we can't convert so we just error and return invalid.
  if ( report_error )
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to Interval" ).arg( value.toString() ) );

  return QgsInterval();
}

static QgsGeometry getGeometry( const QVariant& value, QgsExpression* parent )
{
  if ( value.canConvert<QgsGeometry>() )
    return value.value<QgsGeometry>();

  parent->setEvalErrorString( "Cannot convert to QgsGeometry" );
  return QgsGeometry();
}

static QgsFeature getFeature( const QVariant& value, QgsExpression* parent )
{
  if ( value.canConvert<QgsFeature>() )
    return value.value<QgsFeature>();

  parent->setEvalErrorString( "Cannot convert to QgsFeature" );
  return 0;
}

#define FEAT_FROM_CONTEXT(c, f) if (!c || !c->hasVariable(QgsExpressionContext::EXPR_FEATURE)) return QVariant(); \
  QgsFeature f = qvariant_cast<QgsFeature>( c->variable( QgsExpressionContext::EXPR_FEATURE ) );

static QgsExpression::Node* getNode( const QVariant& value, QgsExpression* parent )
{
  if ( value.canConvert<QgsExpression::Node*>() )
    return value.value<QgsExpression::Node*>();

  parent->setEvalErrorString( "Cannot convert to Node" );
  return nullptr;
}

QgsVectorLayer* getVectorLayer( const QVariant& value, QgsExpression* )
{
  QString layerString = value.toString();
  QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerString ) ); //search by id first
  if ( !vl )
  {
    QList<QgsMapLayer *> layersByName = QgsMapLayerRegistry::instance()->mapLayersByName( layerString );
    if ( !layersByName.isEmpty() )
    {
      vl = qobject_cast<QgsVectorLayer*>( layersByName.at( 0 ) );
    }
  }

  return vl;
}


// this handles also NULL values
static TVL getTVLValue( const QVariant& value, QgsExpression* parent )
{
  // we need to convert to TVL
  if ( value.isNull() )
    return Unknown;

  //handle some special cases
  if ( value.canConvert<QgsGeometry>() )
  {
    //geom is false if empty
    QgsGeometry geom = value.value<QgsGeometry>();
    return geom.isEmpty() ? False : True;
  }
  else if ( value.canConvert<QgsFeature>() )
  {
    //feat is false if non-valid
    QgsFeature feat = value.value<QgsFeature>();
    return feat.isValid() ? True : False;
  }

  if ( value.type() == QVariant::Int )
    return value.toInt() != 0 ? True : False;

  bool ok;
  double x = value.toDouble( &ok );
  if ( !ok )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to boolean" ).arg( value.toString() ) );
    return Unknown;
  }
  return !qgsDoubleNear( x, 0.0 ) ? True : False;
}

//////

static QVariant fcnGetVariable( const QVariantList& values, const QgsExpressionContext* context, QgsExpression* parent )
{
  if ( !context )
    return QVariant();

  QString name = getStringValue( values.at( 0 ), parent );
  return context->variable( name );
}

static QVariant fcnEval( const QVariantList& values, const QgsExpressionContext* context, QgsExpression* parent )
{
  if ( !context )
    return QVariant();

  QString expString = getStringValue( values.at( 0 ), parent );
  QgsExpression expression( expString );
  return expression.evaluate( context );
}

static QVariant fcnSqrt( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double x = getDoubleValue( values.at( 0 ), parent );
  return QVariant( sqrt( x ) );
}

static QVariant fcnAbs( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double val = getDoubleValue( values.at( 0 ), parent );
  return QVariant( fabs( val ) );
}

static QVariant fcnRadians( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double deg = getDoubleValue( values.at( 0 ), parent );
  return ( deg * M_PI ) / 180;
}
static QVariant fcnDegrees( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double rad = getDoubleValue( values.at( 0 ), parent );
  return ( 180 * rad ) / M_PI;
}
static QVariant fcnSin( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double x = getDoubleValue( values.at( 0 ), parent );
  return QVariant( sin( x ) );
}
static QVariant fcnCos( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double x = getDoubleValue( values.at( 0 ), parent );
  return QVariant( cos( x ) );
}
static QVariant fcnTan( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double x = getDoubleValue( values.at( 0 ), parent );
  return QVariant( tan( x ) );
}
static QVariant fcnAsin( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double x = getDoubleValue( values.at( 0 ), parent );
  return QVariant( asin( x ) );
}
static QVariant fcnAcos( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double x = getDoubleValue( values.at( 0 ), parent );
  return QVariant( acos( x ) );
}
static QVariant fcnAtan( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double x = getDoubleValue( values.at( 0 ), parent );
  return QVariant( atan( x ) );
}
static QVariant fcnAtan2( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double y = getDoubleValue( values.at( 0 ), parent );
  double x = getDoubleValue( values.at( 1 ), parent );
  return QVariant( atan2( y, x ) );
}
static QVariant fcnExp( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double x = getDoubleValue( values.at( 0 ), parent );
  return QVariant( exp( x ) );
}
static QVariant fcnLn( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double x = getDoubleValue( values.at( 0 ), parent );
  if ( x <= 0 )
    return QVariant();
  return QVariant( log( x ) );
}
static QVariant fcnLog10( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double x = getDoubleValue( values.at( 0 ), parent );
  if ( x <= 0 )
    return QVariant();
  return QVariant( log10( x ) );
}
static QVariant fcnLog( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double b = getDoubleValue( values.at( 0 ), parent );
  double x = getDoubleValue( values.at( 1 ), parent );
  if ( x <= 0 || b <= 0 )
    return QVariant();
  return QVariant( log( x ) / log( b ) );
}
static QVariant fcnRndF( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double min = getDoubleValue( values.at( 0 ), parent );
  double max = getDoubleValue( values.at( 1 ), parent );
  if ( max < min )
    return QVariant();

  // Return a random double in the range [min, max] (inclusive)
  double f = static_cast< double >( qrand() ) / RAND_MAX;
  return QVariant( min + f * ( max - min ) );
}
static QVariant fcnRnd( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  int min = getIntValue( values.at( 0 ), parent );
  int max = getIntValue( values.at( 1 ), parent );
  if ( max < min )
    return QVariant();

  // Return a random integer in the range [min, max] (inclusive)
  return QVariant( min + ( qrand() % static_cast< int >( max - min + 1 ) ) );
}

static QVariant fcnLinearScale( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double val = getDoubleValue( values.at( 0 ), parent );
  double domainMin = getDoubleValue( values.at( 1 ), parent );
  double domainMax = getDoubleValue( values.at( 2 ), parent );
  double rangeMin = getDoubleValue( values.at( 3 ), parent );
  double rangeMax = getDoubleValue( values.at( 4 ), parent );

  if ( domainMin >= domainMax )
  {
    parent->setEvalErrorString( QObject::tr( "Domain max must be greater than domain min" ) );
    return QVariant();
  }

  // outside of domain?
  if ( val >= domainMax )
  {
    return rangeMax;
  }
  else if ( val <= domainMin )
  {
    return rangeMin;
  }

  // calculate linear scale
  double m = ( rangeMax - rangeMin ) / ( domainMax - domainMin );
  double c = rangeMin - ( domainMin * m );

  // Return linearly scaled value
  return QVariant( m * val + c );
}

static QVariant fcnExpScale( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double val = getDoubleValue( values.at( 0 ), parent );
  double domainMin = getDoubleValue( values.at( 1 ), parent );
  double domainMax = getDoubleValue( values.at( 2 ), parent );
  double rangeMin = getDoubleValue( values.at( 3 ), parent );
  double rangeMax = getDoubleValue( values.at( 4 ), parent );
  double exponent = getDoubleValue( values.at( 5 ), parent );

  if ( domainMin >= domainMax )
  {
    parent->setEvalErrorString( QObject::tr( "Domain max must be greater than domain min" ) );
    return QVariant();
  }
  if ( exponent <= 0 )
  {
    parent->setEvalErrorString( QObject::tr( "Exponent must be greater than 0" ) );
    return QVariant();
  }

  // outside of domain?
  if ( val >= domainMax )
  {
    return rangeMax;
  }
  else if ( val <= domainMin )
  {
    return rangeMin;
  }

  // Return exponentially scaled value
  return QVariant((( rangeMax - rangeMin ) / pow( domainMax - domainMin, exponent ) ) * pow( val - domainMin, exponent ) + rangeMin );
}

static QVariant fcnMax( const QVariantList& values, const QgsExpressionContext*, QgsExpression *parent )
{
  //initially set max as first value
  double maxVal = getDoubleValue( values.at( 0 ), parent );

  //check against all other values
  for ( int i = 1; i < values.length(); ++i )
  {
    double testVal = getDoubleValue( values[i], parent );
    if ( testVal > maxVal )
    {
      maxVal = testVal;
    }
  }

  return QVariant( maxVal );
}

static QVariant fcnMin( const QVariantList& values, const QgsExpressionContext*, QgsExpression *parent )
{
  //initially set min as first value
  double minVal = getDoubleValue( values.at( 0 ), parent );

  //check against all other values
  for ( int i = 1; i < values.length(); ++i )
  {
    double testVal = getDoubleValue( values[i], parent );
    if ( testVal < minVal )
    {
      minVal = testVal;
    }
  }

  return QVariant( minVal );
}

static QVariant fcnAggregate( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  //lazy eval, so we need to evaluate nodes now

  //first node is layer id or name
  QgsExpression::Node* node = getNode( values.at( 0 ), parent );
  ENSURE_NO_EVAL_ERROR;
  QVariant value = node->eval( parent, context );
  ENSURE_NO_EVAL_ERROR;
  QgsVectorLayer* vl = getVectorLayer( value, parent );
  if ( !vl )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot find layer with name or ID '%1'" ).arg( value.toString() ) );
    return QVariant();
  }

  // second node is aggregate type
  node = getNode( values.at( 1 ), parent );
  ENSURE_NO_EVAL_ERROR;
  value = node->eval( parent, context );
  ENSURE_NO_EVAL_ERROR;
  bool ok = false;
  QgsAggregateCalculator::Aggregate aggregate = QgsAggregateCalculator::stringToAggregate( getStringValue( value, parent ), &ok );
  if ( !ok )
  {
    parent->setEvalErrorString( QObject::tr( "No such aggregate '%1'" ).arg( value.toString() ) );
    return QVariant();
  }

  // third node is subexpression (or field name)
  node = getNode( values.at( 2 ), parent );
  ENSURE_NO_EVAL_ERROR;
  QString subExpression = node->dump();

  QgsAggregateCalculator::AggregateParameters parameters;
  //optional forth node is filter
  if ( values.count() > 3 )
  {
    node = getNode( values.at( 3 ), parent );
    ENSURE_NO_EVAL_ERROR;
    QgsExpression::NodeLiteral* nl = dynamic_cast< QgsExpression::NodeLiteral* >( node );
    if ( !nl || nl->value().isValid() )
      parameters.filter = node->dump();
  }

  //optional fifth node is concatenator
  if ( values.count() > 4 )
  {
    node = getNode( values.at( 4 ), parent );
    ENSURE_NO_EVAL_ERROR;
    value = node->eval( parent, context );
    ENSURE_NO_EVAL_ERROR;
    parameters.delimiter = value.toString();
  }

  QString cacheKey = QString( "aggfcn:%1:%2:%3:%4" ).arg( vl->id(), QString::number( static_cast< int >( aggregate ) ), subExpression, parameters.filter );
  if ( context && context->hasCachedValue( cacheKey ) )
    return context->cachedValue( cacheKey );

  QVariant result;
  if ( context )
  {
    QgsExpressionContext subContext( *context );
    result = vl->aggregate( aggregate, subExpression, parameters, &subContext, &ok );
  }
  else
  {
    result = vl->aggregate( aggregate, subExpression, parameters, nullptr, &ok );
  }
  if ( !ok )
  {
    parent->setEvalErrorString( QObject::tr( "Could not calculate aggregate for: %1" ).arg( subExpression ) );
    return QVariant();
  }

  // cache value
  if ( context )
    context->setCachedValue( cacheKey, result );
  return result;
}

static QVariant fcnAggregateRelation( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  if ( !context )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot use relation aggregate function in this context" ) );
    return QVariant();
  }

  // first step - find current layer
  QString layerId = context->variable( "layer_id" ).toString();
  QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerId ) );
  if ( !vl )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot use relation aggregate function in this context" ) );
    return QVariant();
  }

  //lazy eval, so we need to evaluate nodes now

  //first node is relation name
  QgsExpression::Node* node = getNode( values.at( 0 ), parent );
  ENSURE_NO_EVAL_ERROR;
  QVariant value = node->eval( parent, context );
  ENSURE_NO_EVAL_ERROR;
  QString relationId = value.toString();
  // check relation exists
  QgsRelation relation = QgsProject::instance()->relationManager()->relation( relationId );
  if ( !relation.isValid() || relation.referencedLayer() != vl )
  {
    // check for relations by name
    QList< QgsRelation > relations = QgsProject::instance()->relationManager()->relationsByName( relationId );
    if ( relations.isEmpty() || relations.at( 0 ).referencedLayer() != vl )
    {
      parent->setEvalErrorString( QObject::tr( "Cannot find relation with id '%1'" ).arg( relationId ) );
      return QVariant();
    }
    else
    {
      relation = relations.at( 0 );
    }
  }

  QgsVectorLayer* childLayer = relation.referencingLayer();

  // second node is aggregate type
  node = getNode( values.at( 1 ), parent );
  ENSURE_NO_EVAL_ERROR;
  value = node->eval( parent, context );
  ENSURE_NO_EVAL_ERROR;
  bool ok = false;
  QgsAggregateCalculator::Aggregate aggregate = QgsAggregateCalculator::stringToAggregate( getStringValue( value, parent ), &ok );
  if ( !ok )
  {
    parent->setEvalErrorString( QObject::tr( "No such aggregate '%1'" ).arg( value.toString() ) );
    return QVariant();
  }

  //third node is subexpression (or field name)
  node = getNode( values.at( 2 ), parent );
  ENSURE_NO_EVAL_ERROR;
  QString subExpression = node->dump();

  //optional fourth node is concatenator
  QgsAggregateCalculator::AggregateParameters parameters;
  if ( values.count() > 3 )
  {
    node = getNode( values.at( 3 ), parent );
    ENSURE_NO_EVAL_ERROR;
    value = node->eval( parent, context );
    ENSURE_NO_EVAL_ERROR;
    parameters.delimiter = value.toString();
  }

  FEAT_FROM_CONTEXT( context, f );
  parameters.filter = relation.getRelatedFeaturesFilter( f );

  QString cacheKey = QString( "relagg:%1:%2:%3:%4" ).arg( vl->id(),
                     QString::number( static_cast< int >( aggregate ) ),
                     subExpression,
                     parameters.filter );
  if ( context && context->hasCachedValue( cacheKey ) )
    return context->cachedValue( cacheKey );

  QVariant result;
  ok = false;


  QgsExpressionContext subContext( *context );
  result = childLayer->aggregate( aggregate, subExpression, parameters, &subContext, &ok );

  if ( !ok )
  {
    parent->setEvalErrorString( QObject::tr( "Could not calculate aggregate for: %1" ).arg( subExpression ) );
    return QVariant();
  }

  // cache value
  if ( context )
    context->setCachedValue( cacheKey, result );
  return result;
}


static QVariant fcnAggregateGeneric( QgsAggregateCalculator::Aggregate aggregate, const QVariantList& values, QgsAggregateCalculator::AggregateParameters parameters, const QgsExpressionContext* context, QgsExpression *parent )
{
  if ( !context )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot use aggregate function in this context" ) );
    return QVariant();
  }

  // first step - find current layer
  QString layerId = context->variable( "layer_id" ).toString();
  QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerId ) );
  if ( !vl )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot use aggregate function in this context" ) );
    return QVariant();
  }

  //lazy eval, so we need to evaluate nodes now

  //first node is subexpression (or field name)
  QgsExpression::Node* node = getNode( values.at( 0 ), parent );
  ENSURE_NO_EVAL_ERROR;
  QString subExpression = node->dump();

  //optional second node is group by
  QString groupBy;
  if ( values.count() > 1 )
  {
    node = getNode( values.at( 1 ), parent );
    ENSURE_NO_EVAL_ERROR;
    QgsExpression::NodeLiteral* nl = dynamic_cast< QgsExpression::NodeLiteral* >( node );
    if ( !nl || nl->value().isValid() )
      groupBy = node->dump();
  }

  //optional third node is filter
  if ( values.count() > 2 )
  {
    node = getNode( values.at( 2 ), parent );
    ENSURE_NO_EVAL_ERROR;
    QgsExpression::NodeLiteral* nl = dynamic_cast< QgsExpression::NodeLiteral* >( node );
    if ( !nl || nl->value().isValid() )
      parameters.filter = node->dump();
  }

  // build up filter with group by

  // find current group by value
  if ( !groupBy.isEmpty() )
  {
    QgsExpression groupByExp( groupBy );
    QVariant groupByValue = groupByExp.evaluate( context );
    if ( !parameters.filter.isEmpty() )
      parameters.filter = QString( "(%1) AND (%2=%3)" ).arg( parameters.filter, groupBy, QgsExpression::quotedValue( groupByValue ) );
    else
      parameters.filter = QString( "(%2 = %3)" ).arg( groupBy, QgsExpression::quotedValue( groupByValue ) );
  }

  QString cacheKey = QString( "agg:%1:%2:%3:%4" ).arg( vl->id(),
                     QString::number( static_cast< int >( aggregate ) ),
                     subExpression,
                     parameters.filter );
  if ( context && context->hasCachedValue( cacheKey ) )
    return context->cachedValue( cacheKey );

  QVariant result;
  bool ok = false;

  QgsExpressionContext subContext( *context );
  result = vl->aggregate( aggregate, subExpression, parameters, &subContext, &ok );

  if ( !ok )
  {
    parent->setEvalErrorString( QObject::tr( "Could not calculate aggregate for: %1" ).arg( subExpression ) );
    return QVariant();
  }

  // cache value
  if ( context )
    context->setCachedValue( cacheKey, result );
  return result;
}


static QVariant fcnAggregateCount( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::Count, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateCountDistinct( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::CountDistinct, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateCountMissing( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::CountMissing, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMin( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::Min, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMax( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::Max, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateSum( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::Sum, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMean( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::Mean, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMedian( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::Median, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateStdev( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::StDevSample, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateRange( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::Range, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMinority( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::Minority, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMajority( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::Majority, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateQ1( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::FirstQuartile, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateQ3( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::ThirdQuartile, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateIQR( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::InterQuartileRange, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMinLength( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::StringMinimumLength, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMaxLength( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::StringMaximumLength, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateStringConcat( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  QgsAggregateCalculator::AggregateParameters parameters;

  //fourth node is concatenator
  if ( values.count() > 3 )
  {
    QgsExpression::Node* node = getNode( values.at( 3 ), parent );
    ENSURE_NO_EVAL_ERROR;
    QVariant value = node->eval( parent, context );
    ENSURE_NO_EVAL_ERROR;
    parameters.delimiter = value.toString();
  }

  return fcnAggregateGeneric( QgsAggregateCalculator::StringConcatenate, values, parameters, context, parent );
}

static QVariant fcnClamp( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double minValue = getDoubleValue( values.at( 0 ), parent );
  double testValue = getDoubleValue( values.at( 1 ), parent );
  double maxValue = getDoubleValue( values.at( 2 ), parent );

  // force testValue to sit inside the range specified by the min and max value
  if ( testValue <= minValue )
  {
    return QVariant( minValue );
  }
  else if ( testValue >= maxValue )
  {
    return QVariant( maxValue );
  }
  else
  {
    return QVariant( testValue );
  }
}

static QVariant fcnFloor( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double x = getDoubleValue( values.at( 0 ), parent );
  return QVariant( floor( x ) );
}

static QVariant fcnCeil( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double x = getDoubleValue( values.at( 0 ), parent );
  return QVariant( ceil( x ) );
}

static QVariant fcnToInt( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  return QVariant( getIntValue( values.at( 0 ), parent ) );
}
static QVariant fcnToReal( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  return QVariant( getDoubleValue( values.at( 0 ), parent ) );
}
static QVariant fcnToString( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  return QVariant( getStringValue( values.at( 0 ), parent ) );
}

static QVariant fcnToDateTime( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  return QVariant( getDateTimeValue( values.at( 0 ), parent ) );
}

static QVariant fcnCoalesce( const QVariantList& values, const QgsExpressionContext*, QgsExpression* )
{
  Q_FOREACH ( const QVariant &value, values )
  {
    if ( value.isNull() )
      continue;
    return value;
  }
  return QVariant();
}
static QVariant fcnLower( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString str = getStringValue( values.at( 0 ), parent );
  return QVariant( str.toLower() );
}
static QVariant fcnUpper( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString str = getStringValue( values.at( 0 ), parent );
  return QVariant( str.toUpper() );
}
static QVariant fcnTitle( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString str = getStringValue( values.at( 0 ), parent );
  QStringList elems = str.split( ' ' );
  for ( int i = 0; i < elems.size(); i++ )
  {
    if ( elems[i].size() > 1 )
      elems[i] = elems[i].at( 0 ).toUpper() + elems[i].mid( 1 ).toLower();
  }
  return QVariant( elems.join( " " ) );
}

static QVariant fcnTrim( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString str = getStringValue( values.at( 0 ), parent );
  return QVariant( str.trimmed() );
}

static QVariant fcnLevenshtein( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString string1 = getStringValue( values.at( 0 ), parent );
  QString string2 = getStringValue( values.at( 1 ), parent );
  return QVariant( QgsStringUtils::levenshteinDistance( string1, string2, true ) );
}

static QVariant fcnLCS( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString string1 = getStringValue( values.at( 0 ), parent );
  QString string2 = getStringValue( values.at( 1 ), parent );
  return QVariant( QgsStringUtils::longestCommonSubstring( string1, string2, true ) );
}

static QVariant fcnHamming( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString string1 = getStringValue( values.at( 0 ), parent );
  QString string2 = getStringValue( values.at( 1 ), parent );
  int dist = QgsStringUtils::hammingDistance( string1, string2 );
  return ( dist < 0 ? QVariant() : QVariant( QgsStringUtils::hammingDistance( string1, string2, true ) ) );
}

static QVariant fcnSoundex( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString string = getStringValue( values.at( 0 ), parent );
  return QVariant( QgsStringUtils::soundex( string ) );
}

static QVariant fcnChar( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QChar character = QChar( getIntValue( values.at( 0 ), parent ) );
  return QVariant( QString( character ) );
}

static QVariant fcnWordwrap( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  if ( values.length() == 2 || values.length() == 3 )
  {
    QString str = getStringValue( values.at( 0 ), parent );
    int wrap = getIntValue( values.at( 1 ), parent );

    if ( !str.isEmpty() && wrap != 0 )
    {
      QString newstr;
      QString delimiterstr;
      if ( values.length() == 3 ) delimiterstr = getStringValue( values.at( 2 ), parent );
      if ( delimiterstr.isEmpty() ) delimiterstr = ' ';
      int delimiterlength = delimiterstr.length();

      QStringList lines = str.split( '\n' );
      int strlength, strcurrent, strhit, lasthit;

      for ( int i = 0; i < lines.size(); i++ )
      {
        strlength = lines[i].length();
        strcurrent = 0;
        strhit = 0;
        lasthit = 0;

        while ( strcurrent < strlength )
        {
          // positive wrap value = desired maximum line width to wrap
          // negative wrap value = desired minimum line width before wrap
          if ( wrap > 0 )
          {
            //first try to locate delimiter backwards
            strhit = lines[i].lastIndexOf( delimiterstr, strcurrent + wrap );
            if ( strhit == lasthit || strhit == -1 )
            {
              //if no new backward delimiter found, try to locate forward
              strhit = lines[i].indexOf( delimiterstr, strcurrent + qAbs( wrap ) );
            }
            lasthit = strhit;
          }
          else
          {
            strhit = lines[i].indexOf( delimiterstr, strcurrent + qAbs( wrap ) );
          }
          if ( strhit > -1 )
          {
            newstr.append( lines[i].midRef( strcurrent, strhit - strcurrent ) );
            newstr.append( '\n' );
            strcurrent = strhit + delimiterlength;
          }
          else
          {
            newstr.append( lines[i].midRef( strcurrent ) );
            strcurrent = strlength;
          }
        }
        if ( i < lines.size() - 1 ) newstr.append( '\n' );
      }

      return QVariant( newstr );
    }
  }

  return QVariant();
}

static QVariant fcnLength( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  // two variants, one for geometry, one for string
  if ( values.at( 0 ).canConvert<QgsGeometry>() )
  {
    //geometry variant
    QgsGeometry geom = getGeometry( values.at( 0 ), parent );
    if ( geom.type() != QGis::Line )
      return QVariant();

    return QVariant( geom.length() );
  }

  //otherwise fall back to string variant
  QString str = getStringValue( values.at( 0 ), parent );
  return QVariant( str.length() );
}

static QVariant fcnReplace( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString str = getStringValue( values.at( 0 ), parent );
  QString before = getStringValue( values.at( 1 ), parent );
  QString after = getStringValue( values.at( 2 ), parent );
  return QVariant( str.replace( before, after ) );
}
static QVariant fcnRegexpReplace( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString str = getStringValue( values.at( 0 ), parent );
  QString regexp = getStringValue( values.at( 1 ), parent );
  QString after = getStringValue( values.at( 2 ), parent );

  QRegExp re( regexp );
  if ( !re.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Invalid regular expression '%1': %2" ).arg( regexp, re.errorString() ) );
    return QVariant();
  }
  return QVariant( str.replace( re, after ) );
}

static QVariant fcnRegexpMatch( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString str = getStringValue( values.at( 0 ), parent );
  QString regexp = getStringValue( values.at( 1 ), parent );

  QRegExp re( regexp );
  if ( !re.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Invalid regular expression '%1': %2" ).arg( regexp, re.errorString() ) );
    return QVariant();
  }
  return QVariant( str.contains( re ) ? 1 : 0 );
}

static QVariant fcnRegexpSubstr( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString str = getStringValue( values.at( 0 ), parent );
  QString regexp = getStringValue( values.at( 1 ), parent );

  QRegExp re( regexp );
  if ( !re.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Invalid regular expression '%1': %2" ).arg( regexp, re.errorString() ) );
    return QVariant();
  }

  // extract substring
  ( void )re.indexIn( str );
  if ( re.captureCount() > 0 )
  {
    // return first capture
    return QVariant( re.capturedTexts().at( 1 ) );
  }
  else
  {
    return QVariant( "" );
  }
}

static QVariant fcnUuid( const QVariantList&, const QgsExpressionContext*, QgsExpression* )
{
  return QUuid::createUuid().toString();
}

static QVariant fcnSubstr( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString str = getStringValue( values.at( 0 ), parent );
  int from = getIntValue( values.at( 1 ), parent );
  int len = getIntValue( values.at( 2 ), parent );
  return QVariant( str.mid( from -1, len ) );
}

static QVariant fcnRowNumber( const QVariantList&, const QgsExpressionContext* context, QgsExpression* parent )
{
  if ( context && context->hasVariable( "row_number" ) )
    return context->variable( "row_number" );

  Q_NOWARN_DEPRECATED_PUSH
  return QVariant( parent->currentRowNumber() );
  Q_NOWARN_DEPRECATED_POP
  //when above is removed - return QVariant()
}

static QVariant fcnMapId( const QVariantList&, const QgsExpressionContext* context, QgsExpression* )
{
  if ( context && context->hasVariable( "map_id" ) )
    return context->variable( "map_id" );

  Q_NOWARN_DEPRECATED_PUSH
  return QgsExpression::specialColumn( "$map" );
  Q_NOWARN_DEPRECATED_POP
}

static QVariant fcnComposerNumPages( const QVariantList&, const QgsExpressionContext* context, QgsExpression* )
{
  if ( context && context->hasVariable( "layout_numpages" ) )
    return context->variable( "layout_numpages" );

  Q_NOWARN_DEPRECATED_PUSH
  return QgsExpression::specialColumn( "$numpages" );
  Q_NOWARN_DEPRECATED_POP
}

static QVariant fcnComposerPage( const QVariantList&, const QgsExpressionContext* context, QgsExpression* )
{
  if ( context && context->hasVariable( "layout_page" ) )
    return context->variable( "layout_page" );

  Q_NOWARN_DEPRECATED_PUSH
  return QgsExpression::specialColumn( "$page" );
  Q_NOWARN_DEPRECATED_POP
}

static QVariant fcnAtlasFeature( const QVariantList&, const QgsExpressionContext* context, QgsExpression* )
{
  if ( context && context->hasVariable( "atlas_featurenumber" ) )
    return context->variable( "atlas_featurenumber" );

  Q_NOWARN_DEPRECATED_PUSH
  return QgsExpression::specialColumn( "$feature" );
  Q_NOWARN_DEPRECATED_POP
}

static QVariant fcnAtlasFeatureId( const QVariantList&, const QgsExpressionContext* context, QgsExpression* )
{
  if ( context && context->hasVariable( "atlas_featureid" ) )
    return context->variable( "atlas_featureid" );

  Q_NOWARN_DEPRECATED_PUSH
  return QgsExpression::specialColumn( "$atlasfeatureid" );
  Q_NOWARN_DEPRECATED_POP
}


static QVariant fcnAtlasCurrentFeature( const QVariantList&, const QgsExpressionContext* context, QgsExpression* )
{
  if ( context && context->hasVariable( "atlas_feature" ) )
    return context->variable( "atlas_feature" );

  Q_NOWARN_DEPRECATED_PUSH
  return QgsExpression::specialColumn( "$atlasfeature" );
  Q_NOWARN_DEPRECATED_POP
}

static QVariant fcnAtlasCurrentGeometry( const QVariantList&, const QgsExpressionContext* context, QgsExpression* )
{
  if ( context && context->hasVariable( "atlas_geometry" ) )
    return context->variable( "atlas_geometry" );

  Q_NOWARN_DEPRECATED_PUSH
  return QgsExpression::specialColumn( "$atlasgeometry" );
  Q_NOWARN_DEPRECATED_POP
}

static QVariant fcnAtlasNumFeatures( const QVariantList&, const QgsExpressionContext* context, QgsExpression* )
{
  if ( context && context->hasVariable( "atlas_totalfeatures" ) )
    return context->variable( "atlas_totalfeatures" );

  Q_NOWARN_DEPRECATED_PUSH
  return QgsExpression::specialColumn( "$numfeatures" );
  Q_NOWARN_DEPRECATED_POP
}

static QVariant fcnFeatureId( const QVariantList&, const QgsExpressionContext* context, QgsExpression* )
{
  FEAT_FROM_CONTEXT( context, f );
  // TODO: handling of 64-bit feature ids?
  return QVariant( static_cast< int >( f.id() ) );
}

static QVariant fcnFeature( const QVariantList&, const QgsExpressionContext* context, QgsExpression* )
{
  if ( !context )
    return QVariant();

  return context->variable( QgsExpressionContext::EXPR_FEATURE );
}
static QVariant fcnAttribute( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsFeature feat = getFeature( values.at( 0 ), parent );
  QString attr = getStringValue( values.at( 1 ), parent );

  return feat.attribute( attr );
}
static QVariant fcnConcat( const QVariantList& values, const QgsExpressionContext*, QgsExpression *parent )
{
  QString concat;
  Q_FOREACH ( const QVariant &value, values )
  {
    concat += getStringValue( value, parent );
  }
  return concat;
}

static QVariant fcnStrpos( const QVariantList& values, const QgsExpressionContext*, QgsExpression *parent )
{
  QString string = getStringValue( values.at( 0 ), parent );
  return string.indexOf( QRegExp( getStringValue( values.at( 1 ), parent ) ) ) + 1;
}

static QVariant fcnRight( const QVariantList& values, const QgsExpressionContext*, QgsExpression *parent )
{
  QString string = getStringValue( values.at( 0 ), parent );
  int pos = getIntValue( values.at( 1 ), parent );
  return string.right( pos );
}

static QVariant fcnLeft( const QVariantList& values, const QgsExpressionContext*, QgsExpression *parent )
{
  QString string = getStringValue( values.at( 0 ), parent );
  int pos = getIntValue( values.at( 1 ), parent );
  return string.left( pos );
}

static QVariant fcnRPad( const QVariantList& values, const QgsExpressionContext*, QgsExpression *parent )
{
  QString string = getStringValue( values.at( 0 ), parent );
  int length = getIntValue( values.at( 1 ), parent );
  QString fill = getStringValue( values.at( 2 ), parent );
  return string.leftJustified( length, fill.at( 0 ), true );
}

static QVariant fcnLPad( const QVariantList& values, const QgsExpressionContext*, QgsExpression *parent )
{
  QString string = getStringValue( values.at( 0 ), parent );
  int length = getIntValue( values.at( 1 ), parent );
  QString fill = getStringValue( values.at( 2 ), parent );
  return string.rightJustified( length, fill.at( 0 ), true );
}

static QVariant fcnFormatString( const QVariantList& values, const QgsExpressionContext*, QgsExpression *parent )
{
  QString string = getStringValue( values.at( 0 ), parent );
  for ( int n = 1; n < values.length(); n++ )
  {
    string = string.arg( getStringValue( values.at( n ), parent ) );
  }
  return string;
}


static QVariant fcnNow( const QVariantList&, const QgsExpressionContext*, QgsExpression * )
{
  return QVariant( QDateTime::currentDateTime() );
}

static QVariant fcnToDate( const QVariantList& values, const QgsExpressionContext*, QgsExpression * parent )
{
  return QVariant( getDateValue( values.at( 0 ), parent ) );
}

static QVariant fcnToTime( const QVariantList& values, const QgsExpressionContext*, QgsExpression * parent )
{
  return QVariant( getTimeValue( values.at( 0 ), parent ) );
}

static QVariant fcnToInterval( const QVariantList& values, const QgsExpressionContext*, QgsExpression * parent )
{
  return QVariant::fromValue( getInterval( values.at( 0 ), parent ) );
}

static QVariant fcnAge( const QVariantList& values, const QgsExpressionContext*, QgsExpression *parent )
{
  QDateTime d1 = getDateTimeValue( values.at( 0 ), parent );
  QDateTime d2 = getDateTimeValue( values.at( 1 ), parent );
  int seconds = d2.secsTo( d1 );
  return QVariant::fromValue( QgsInterval( seconds ) );
}

static QVariant fcnDayOfWeek( const QVariantList& values, const QgsExpressionContext*, QgsExpression *parent )
{
  if ( !values.at( 0 ).canConvert<QDate>() )
    return QVariant();

  QDate date = getDateValue( values.at( 0 ), parent );
  if ( !date.isValid() )
    return QVariant();

  // return dayOfWeek() % 7 so that values range from 0 (sun) to 6 (sat)
  // (to match PostgreSQL behaviour)
  return date.dayOfWeek() % 7;
}

static QVariant fcnDay( const QVariantList& values, const QgsExpressionContext*, QgsExpression *parent )
{
  QVariant value = values.at( 0 );
  QgsInterval inter = getInterval( value, parent, false );
  if ( inter.isValid() )
  {
    return QVariant( inter.days() );
  }
  else
  {
    QDateTime d1 = getDateTimeValue( value, parent );
    return QVariant( d1.date().day() );
  }
}

static QVariant fcnYear( const QVariantList& values, const QgsExpressionContext*, QgsExpression *parent )
{
  QVariant value = values.at( 0 );
  QgsInterval inter = getInterval( value, parent, false );
  if ( inter.isValid() )
  {
    return QVariant( inter.years() );
  }
  else
  {
    QDateTime d1 =  getDateTimeValue( value, parent );
    return QVariant( d1.date().year() );
  }
}

static QVariant fcnMonth( const QVariantList& values, const QgsExpressionContext*, QgsExpression *parent )
{
  QVariant value = values.at( 0 );
  QgsInterval inter = getInterval( value, parent, false );
  if ( inter.isValid() )
  {
    return QVariant( inter.months() );
  }
  else
  {
    QDateTime d1 =  getDateTimeValue( value, parent );
    return QVariant( d1.date().month() );
  }
}

static QVariant fcnWeek( const QVariantList& values, const QgsExpressionContext*, QgsExpression *parent )
{
  QVariant value = values.at( 0 );
  QgsInterval inter = getInterval( value, parent, false );
  if ( inter.isValid() )
  {
    return QVariant( inter.weeks() );
  }
  else
  {
    QDateTime d1 =  getDateTimeValue( value, parent );
    return QVariant( d1.date().weekNumber() );
  }
}

static QVariant fcnHour( const QVariantList& values, const QgsExpressionContext*, QgsExpression *parent )
{
  QVariant value = values.at( 0 );
  QgsInterval inter = getInterval( value, parent, false );
  if ( inter.isValid() )
  {
    return QVariant( inter.hours() );
  }
  else
  {
    QTime t1 = getTimeValue( value, parent );
    return QVariant( t1.hour() );
  }
}

static QVariant fcnMinute( const QVariantList& values, const QgsExpressionContext*, QgsExpression *parent )
{
  QVariant value = values.at( 0 );
  QgsInterval inter = getInterval( value, parent, false );
  if ( inter.isValid() )
  {
    return QVariant( inter.minutes() );
  }
  else
  {
    QTime t1 =  getTimeValue( value, parent );
    return QVariant( t1.minute() );
  }
}

static QVariant fcnSeconds( const QVariantList& values, const QgsExpressionContext*, QgsExpression *parent )
{
  QVariant value = values.at( 0 );
  QgsInterval inter = getInterval( value, parent, false );
  if ( inter.isValid() )
  {
    return QVariant( inter.seconds() );
  }
  else
  {
    QTime t1 =  getTimeValue( value, parent );
    return QVariant( t1.second() );
  }
}


#define ENSURE_GEOM_TYPE(f, g, geomtype) const QgsGeometry* g = f.constGeometry(); \
  if (!g || g->type() != geomtype) return QVariant();

static QVariant fcnX( const QVariantList&, const QgsExpressionContext* context, QgsExpression* )
{
  FEAT_FROM_CONTEXT( context, f );
  ENSURE_GEOM_TYPE( f, g, QGis::Point );
  if ( g->isMultipart() )
  {
    return g->asMultiPoint().at( 0 ).x();
  }
  else
  {
    return g->asPoint().x();
  }
}

static QVariant fcnY( const QVariantList&, const QgsExpressionContext* context, QgsExpression* )
{
  FEAT_FROM_CONTEXT( context, f );
  ENSURE_GEOM_TYPE( f, g, QGis::Point );
  if ( g->isMultipart() )
  {
    return g->asMultiPoint().at( 0 ).y();
  }
  else
  {
    return g->asPoint().y();
  }
}

static QVariant fcnGeomX( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );
  if ( geom.isEmpty() )
    return QVariant();

  //if single point, return the point's x coordinate
  if ( geom.type() == QGis::Point && !geom.isMultipart() )
  {
    return geom.asPoint().x();
  }

  //otherwise return centroid x
  QgsGeometry* centroid = geom.centroid();
  QVariant result( centroid->asPoint().x() );
  delete centroid;
  return result;
}

static QVariant fcnGeomY( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );
  if ( geom.isEmpty() )
    return QVariant();

  //if single point, return the point's y coordinate
  if ( geom.type() == QGis::Point && !geom.isMultipart() )
  {
    return geom.asPoint().y();
  }

  //otherwise return centroid y
  QgsGeometry* centroid = geom.centroid();
  QVariant result( centroid->asPoint().y() );
  delete centroid;
  return result;
}

static QVariant fcnGeomZ( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );
  if ( geom.isEmpty() )
    return QVariant(); //or 0?

  //if single point, return the point's z coordinate
  if ( geom.type() == QGis::Point && !geom.isMultipart() )
  {
    QgsPointV2* point = dynamic_cast< QgsPointV2* >( geom.geometry() );
    if ( point )
      return point->z();
  }

  return QVariant();
}

static QVariant fcnGeomM( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );
  if ( geom.isEmpty() )
    return QVariant(); //or 0?

  //if single point, return the point's m value
  if ( geom.type() == QGis::Point && !geom.isMultipart() )
  {
    QgsPointV2* point = dynamic_cast< QgsPointV2* >( geom.geometry() );
    if ( point )
      return point->m();
  }

  return QVariant();
}

static QVariant fcnPointN( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.isEmpty() )
    return QVariant();

  //idx is 1 based
  int idx = getIntValue( values.at( 1 ), parent ) - 1;

  QgsVertexId vId;
  if ( idx < 0 || !geom.vertexIdFromVertexNr( idx, vId ) )
  {
    parent->setEvalErrorString( QObject::tr( "Point index is out of range" ) );
    return QVariant();
  }

  QgsPointV2 point = geom.geometry()->vertexAt( vId );
  return QVariant::fromValue( QgsGeometry( new QgsPointV2( point ) ) );
}

static QVariant fcnStartPoint( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.isEmpty() )
    return QVariant();

  QgsVertexId vId;
  if ( !geom.vertexIdFromVertexNr( 0, vId ) )
  {
    return QVariant();
  }

  QgsPointV2 point = geom.geometry()->vertexAt( vId );
  return QVariant::fromValue( QgsGeometry( new QgsPointV2( point ) ) );
}

static QVariant fcnEndPoint( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.isEmpty() )
    return QVariant();

  QgsVertexId vId;
  if ( !geom.vertexIdFromVertexNr( geom.geometry()->nCoordinates() - 1, vId ) )
  {
    return QVariant();
  }

  QgsPointV2 point = geom.geometry()->vertexAt( vId );
  return QVariant::fromValue( QgsGeometry( new QgsPointV2( point ) ) );
}

static QVariant fcnNodesToPoints( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.isEmpty() )
    return QVariant();

  bool ignoreClosing = false;
  if ( values.length() > 1 )
  {
    ignoreClosing = getIntValue( values.at( 1 ), parent );
  }

  QgsMultiPointV2* mp = new QgsMultiPointV2();

  Q_FOREACH ( const QgsRingSequenceV2 &part, geom.geometry()->coordinateSequence() )
  {
    Q_FOREACH ( const QgsPointSequenceV2 &ring, part )
    {
      bool skipLast = false;
      if ( ignoreClosing && ring.count() > 2 && ring.first() == ring.last() )
      {
        skipLast = true;
      }

      for ( int i = 0; i < ( skipLast ? ring.count() - 1 : ring.count() ); ++ i )
      {
        mp->addGeometry( ring.at( i ).clone() );
      }
    }
  }

  return QVariant::fromValue( QgsGeometry( mp ) );
}

static QVariant fcnSegmentsToLines( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.isEmpty() )
    return QVariant();

  QList< QgsLineStringV2* > linesToProcess = QgsGeometryUtils::extractLineStrings( geom.geometry() );

  //ok, now we have a complete list of segmentized lines from the geometry
  QgsMultiLineStringV2* ml = new QgsMultiLineStringV2();
  Q_FOREACH ( QgsLineStringV2* line, linesToProcess )
  {
    for ( int i = 0; i < line->numPoints() - 1; ++i )
    {
      QgsLineStringV2* segment = new QgsLineStringV2();
      segment->setPoints( QgsPointSequenceV2()
                          << line->pointN( i )
                          << line->pointN( i + 1 ) );
      ml->addGeometry( segment );
    }
    delete line;
  }

  return QVariant::fromValue( QgsGeometry( ml ) );
}

static QVariant fcnInteriorRingN( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.isEmpty() )
    return QVariant();

  QgsCurvePolygonV2* curvePolygon = dynamic_cast< QgsCurvePolygonV2* >( geom.geometry() );
  if ( !curvePolygon )
    return QVariant();

  //idx is 1 based
  int idx = getIntValue( values.at( 1 ), parent ) - 1;

  if ( idx >= curvePolygon->numInteriorRings() || idx < 0 )
    return QVariant();

  QgsCurveV2* curve = static_cast< QgsCurveV2* >( curvePolygon->interiorRing( idx )->clone() );
  QVariant result = curve ? QVariant::fromValue( QgsGeometry( curve ) ) : QVariant();
  return result;
}

static QVariant fcnGeometryN( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.isEmpty() )
    return QVariant();

  QgsGeometryCollectionV2* collection = dynamic_cast< QgsGeometryCollectionV2* >( geom.geometry() );
  if ( !collection )
    return QVariant();

  //idx is 1 based
  int idx = getIntValue( values.at( 1 ), parent ) - 1;

  if ( idx < 0 || idx >= collection->numGeometries() )
    return QVariant();

  QgsAbstractGeometryV2* part = collection->geometryN( idx )->clone();
  QVariant result = part ? QVariant::fromValue( QgsGeometry( part ) ) : QVariant();
  return result;
}

static QVariant fcnMakePoint( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  if ( values.count() < 2 || values.count() > 4 )
  {
    parent->setEvalErrorString( QObject::tr( "Function make_point requires 2-4 arguments" ) );
    return QVariant();
  }

  double x = getDoubleValue( values.at( 0 ), parent );
  double y = getDoubleValue( values.at( 1 ), parent );
  double z = values.count() >= 3 ? getDoubleValue( values.at( 2 ), parent ) : 0.0;
  double m = values.count() >= 4 ? getDoubleValue( values.at( 3 ), parent ) : 0.0;
  switch ( values.count() )
  {
    case 2:
      return QVariant::fromValue( QgsGeometry( new QgsPointV2( x, y ) ) );
    case 3:
      return QVariant::fromValue( QgsGeometry( new QgsPointV2( QgsWKBTypes::PointZ, x, y, z ) ) );
    case 4:
      return QVariant::fromValue( QgsGeometry( new QgsPointV2( QgsWKBTypes::PointZM, x, y, z, m ) ) );
  }
  return QVariant(); //avoid warning
}

static QVariant fcnMakePointM( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double x = getDoubleValue( values.at( 0 ), parent );
  double y = getDoubleValue( values.at( 1 ), parent );
  double m = getDoubleValue( values.at( 2 ), parent );
  return QVariant::fromValue( QgsGeometry( new QgsPointV2( QgsWKBTypes::PointM, x, y, 0.0, m ) ) );
}

static QVariant fcnMakeLine( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  if ( values.count() < 2 )
  {
    return QVariant();
  }

  QgsLineStringV2* lineString = new QgsLineStringV2();
  lineString->clear();

  Q_FOREACH ( const QVariant& value, values )
  {
    QgsGeometry geom = getGeometry( value, parent );
    if ( geom.isEmpty() )
      continue;

    if ( geom.type() != QGis::Point || geom.isMultipart() )
      continue;

    QgsPointV2* point = dynamic_cast< QgsPointV2* >( geom.geometry() );
    if ( !point )
      continue;

    lineString->addVertex( *point );
  }

  return QVariant::fromValue( QgsGeometry( lineString ) );
}

static QVariant fcnMakePolygon( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  if ( values.count() < 1 )
  {
    parent->setEvalErrorString( QObject::tr( "Function make_polygon requires an argument" ) );
    return QVariant();
  }

  QgsGeometry outerRing = getGeometry( values.at( 0 ), parent );
  if ( outerRing.type() != QGis::Line || outerRing.isMultipart() || outerRing.isEmpty() )
    return QVariant();

  QgsPolygonV2* polygon = new QgsPolygonV2();
  polygon->setExteriorRing( dynamic_cast< QgsCurveV2* >( outerRing.geometry()->clone() ) );

  for ( int i = 1; i < values.count(); ++i )
  {
    QgsGeometry ringGeom = getGeometry( values.at( i ), parent );
    if ( ringGeom.isEmpty() )
      continue;

    if ( ringGeom.type() != QGis::Line || ringGeom.isMultipart() || ringGeom.isEmpty() )
      continue;

    polygon->addInteriorRing( dynamic_cast< QgsCurveV2* >( ringGeom.geometry()->clone() ) );
  }

  return QVariant::fromValue( QgsGeometry( polygon ) );
}

static QVariant pointAt( const QVariantList& values, const QgsExpressionContext* context, QgsExpression* parent ) // helper function
{
  FEAT_FROM_CONTEXT( context, f );
  int idx = getIntValue( values.at( 0 ), parent );
  const QgsGeometry* g = f.constGeometry();
  if ( !g || g->isEmpty() )
    return QVariant();

  if ( idx < 0 )
  {
    idx += g->geometry()->nCoordinates();
  }
  if ( idx < 0 || idx >= g->geometry()->nCoordinates() )
  {
    parent->setEvalErrorString( QObject::tr( "Index is out of range" ) );
    return QVariant();
  }

  QgsPoint p = g->vertexAt( idx );
  return QVariant( QPointF( p.x(), p.y() ) );
}

static QVariant fcnXat( const QVariantList& values, const QgsExpressionContext* f, QgsExpression* parent )
{
  QVariant v = pointAt( values, f, parent );
  if ( v.type() == QVariant::PointF )
    return QVariant( v.toPointF().x() );
  else
    return QVariant();
}
static QVariant fcnYat( const QVariantList& values, const QgsExpressionContext* f, QgsExpression* parent )
{
  QVariant v = pointAt( values, f, parent );
  if ( v.type() == QVariant::PointF )
    return QVariant( v.toPointF().y() );
  else
    return QVariant();
}
static QVariant fcnGeometry( const QVariantList&, const QgsExpressionContext* context, QgsExpression* )
{
  FEAT_FROM_CONTEXT( context, f );
  const QgsGeometry* geom = f.constGeometry();
  if ( geom )
    return  QVariant::fromValue( *geom );
  else
    return QVariant();
}
static QVariant fcnGeomFromWKT( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString wkt = getStringValue( values.at( 0 ), parent );
  QgsGeometry* geom = QgsGeometry::fromWkt( wkt );
  QVariant result = geom ? QVariant::fromValue( *geom ) : QVariant();
  delete geom;
  return result;
}
static QVariant fcnGeomFromGML( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString gml = getStringValue( values.at( 0 ), parent );
  QgsGeometry* geom = QgsOgcUtils::geometryFromGML( gml );
  QVariant result = geom ? QVariant::fromValue( *geom ) : QVariant();
  delete geom;
  return result;
}

static QVariant fcnGeomArea( const QVariantList&, const QgsExpressionContext* context, QgsExpression* parent )
{
  FEAT_FROM_CONTEXT( context, f );
  ENSURE_GEOM_TYPE( f, g, QGis::Polygon );
  QgsDistanceArea* calc = parent->geomCalculator();
  if ( calc )
  {
    double area = calc->measureArea( f.constGeometry() );
    area = calc->convertAreaMeasurement( area, parent->areaUnits() );
    return QVariant( area );
  }
  else
  {
    return QVariant( f.constGeometry()->area() );
  }
}

static QVariant fcnArea( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.type() != QGis::Polygon )
    return QVariant();

  return QVariant( geom.area() );
}

static QVariant fcnGeomLength( const QVariantList&, const QgsExpressionContext* context, QgsExpression* parent )
{
  FEAT_FROM_CONTEXT( context, f );
  ENSURE_GEOM_TYPE( f, g, QGis::Line );
  QgsDistanceArea* calc = parent->geomCalculator();
  if ( calc )
  {
    double len = calc->measureLength( f.constGeometry() );
    len = calc->convertLengthMeasurement( len, parent->distanceUnits() );
    return QVariant( len );
  }
  else
  {
    return QVariant( f.constGeometry()->length() );
  }
}

static QVariant fcnGeomPerimeter( const QVariantList&, const QgsExpressionContext* context, QgsExpression* parent )
{
  FEAT_FROM_CONTEXT( context, f );
  ENSURE_GEOM_TYPE( f, g, QGis::Polygon );
  QgsDistanceArea* calc = parent->geomCalculator();
  if ( calc )
  {
    double len = calc->measurePerimeter( f.constGeometry() );
    len = calc->convertLengthMeasurement( len, parent->distanceUnits() );
    return QVariant( len );
  }
  else
  {
    return f.constGeometry()->isEmpty() ? QVariant( 0 ) : QVariant( f.constGeometry()->geometry()->perimeter() );
  }
}

static QVariant fcnPerimeter( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.type() != QGis::Polygon )
    return QVariant();

  //length for polygons = perimeter
  return QVariant( geom.length() );
}

static QVariant fcnGeomNumPoints( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );
  return QVariant( geom.isEmpty() ? 0 : geom.geometry()->nCoordinates() );
}

static QVariant fcnGeomNumGeometries( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );
  if ( geom.isEmpty() )
    return QVariant();

  return QVariant( geom.geometry()->partCount() );
}

static QVariant fcnGeomNumInteriorRings( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.isEmpty() )
    return QVariant();

  QgsCurvePolygonV2* curvePolygon = dynamic_cast< QgsCurvePolygonV2* >( geom.geometry() );
  if ( curvePolygon )
    return QVariant( curvePolygon->numInteriorRings() );

  QgsGeometryCollectionV2* collection = dynamic_cast< QgsGeometryCollectionV2* >( geom.geometry() );
  if ( collection )
  {
    //find first CurvePolygon in collection
    for ( int i = 0; i < collection->numGeometries(); ++i )
    {
      curvePolygon = dynamic_cast< QgsCurvePolygonV2*>( collection->geometryN( i ) );
      if ( !curvePolygon )
        continue;

      return QVariant( curvePolygon->isEmpty() ? 0 : curvePolygon->numInteriorRings() );
    }
  }

  return QVariant();
}

static QVariant fcnGeomNumRings( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.isEmpty() )
    return QVariant();

  QgsCurvePolygonV2* curvePolygon = dynamic_cast< QgsCurvePolygonV2* >( geom.geometry() );
  if ( curvePolygon )
    return QVariant( curvePolygon->ringCount() );

  bool foundPoly = false;
  int ringCount = 0;
  QgsGeometryCollectionV2* collection = dynamic_cast< QgsGeometryCollectionV2* >( geom.geometry() );
  if ( collection )
  {
    //find CurvePolygons in collection
    for ( int i = 0; i < collection->numGeometries(); ++i )
    {
      curvePolygon = dynamic_cast< QgsCurvePolygonV2*>( collection->geometryN( i ) );
      if ( !curvePolygon )
        continue;

      foundPoly = true;
      ringCount += curvePolygon->ringCount();
    }
  }

  if ( !foundPoly )
    return QVariant();

  return QVariant( ringCount );
}

static QVariant fcnBounds( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );
  QgsGeometry* geomBounds = QgsGeometry::fromRect( geom.boundingBox() );
  QVariant result = geomBounds ? QVariant::fromValue( *geomBounds ) : QVariant();
  delete geomBounds;
  return result;
}

static QVariant fcnBoundsWidth( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );
  return QVariant::fromValue( geom.boundingBox().width() );
}

static QVariant fcnBoundsHeight( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );
  return QVariant::fromValue( geom.boundingBox().height() );
}

static QVariant fcnXMin( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );
  return QVariant::fromValue( geom.boundingBox().xMinimum() );
}

static QVariant fcnXMax( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );
  return QVariant::fromValue( geom.boundingBox().xMaximum() );
}

static QVariant fcnYMin( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );
  return QVariant::fromValue( geom.boundingBox().yMinimum() );
}

static QVariant fcnYMax( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );
  return QVariant::fromValue( geom.boundingBox().yMaximum() );
}

static QVariant fcnIsClosed( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  if ( fGeom.isEmpty() )
    return QVariant();

  QgsCurveV2* curve = dynamic_cast< QgsCurveV2* >( fGeom.geometry() );
  if ( !curve )
    return QVariant();

  return QVariant::fromValue( curve->isClosed() );
}

static QVariant fcnRelate( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  if ( values.length() < 2 || values.length() > 3 )
    return QVariant();

  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );

  if ( fGeom.isEmpty() || sGeom.isEmpty() )
    return QVariant();

  QScopedPointer<QgsGeometryEngine> engine( QgsGeometry::createGeometryEngine( fGeom.geometry() ) );

  if ( values.length() == 2 )
  {
    //two geometry arguments, return relation
    QString result = engine->relate( *sGeom.geometry() );
    return QVariant::fromValue( result );
  }
  else
  {
    //three arguments, test pattern
    QString pattern = getStringValue( values.at( 2 ), parent );
    bool result = engine->relatePattern( *sGeom.geometry(), pattern );
    return QVariant::fromValue( result );
  }
}

static QVariant fcnBbox( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  return fGeom.intersects( sGeom.boundingBox() ) ? TVL_True : TVL_False;
}
static QVariant fcnDisjoint( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  return fGeom.disjoint( &sGeom ) ? TVL_True : TVL_False;
}
static QVariant fcnIntersects( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  return fGeom.intersects( &sGeom ) ? TVL_True : TVL_False;
}
static QVariant fcnTouches( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  return fGeom.touches( &sGeom ) ? TVL_True : TVL_False;
}
static QVariant fcnCrosses( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  return fGeom.crosses( &sGeom ) ? TVL_True : TVL_False;
}
static QVariant fcnContains( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  return fGeom.contains( &sGeom ) ? TVL_True : TVL_False;
}
static QVariant fcnOverlaps( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  return fGeom.overlaps( &sGeom ) ? TVL_True : TVL_False;
}
static QVariant fcnWithin( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  return fGeom.within( &sGeom ) ? TVL_True : TVL_False;
}
static QVariant fcnBuffer( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  if ( values.length() < 2 || values.length() > 3 )
    return QVariant();

  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  double dist = getDoubleValue( values.at( 1 ), parent );
  int seg = 8;
  if ( values.length() == 3 )
    seg = getIntValue( values.at( 2 ), parent );

  QgsGeometry* geom = fGeom.buffer( dist, seg );
  QVariant result = geom ? QVariant::fromValue( *geom ) : QVariant();
  delete geom;
  return result;
}
static QVariant fcnTranslate( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  double dx = getDoubleValue( values.at( 1 ), parent );
  double dy = getDoubleValue( values.at( 2 ), parent );
  fGeom.translate( dx, dy );
  return QVariant::fromValue( fGeom );
}
static QVariant fcnCentroid( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry* geom = fGeom.centroid();
  QVariant result = geom ? QVariant::fromValue( *geom ) : QVariant();
  delete geom;
  return result;
}
static QVariant fcnPointOnSurface( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry* geom = fGeom.pointOnSurface();
  QVariant result = geom ? QVariant::fromValue( *geom ) : QVariant();
  delete geom;
  return result;
}
static QVariant fcnConvexHull( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry* geom = fGeom.convexHull();
  QVariant result = geom ? QVariant::fromValue( *geom ) : QVariant();
  delete geom;
  return result;
}
static QVariant fcnDifference( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  QgsGeometry* geom = fGeom.difference( &sGeom );
  QVariant result = geom ? QVariant::fromValue( *geom ) : QVariant();
  delete geom;
  return result;
}

static QVariant fcnReverse( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  if ( fGeom.isEmpty() )
    return QVariant();

  QgsCurveV2* curve = dynamic_cast< QgsCurveV2* >( fGeom.geometry() );
  if ( !curve )
    return QVariant();

  QgsCurveV2* reversed = curve->reversed();
  QVariant result = reversed ? QVariant::fromValue( QgsGeometry( reversed ) ) : QVariant();
  return result;
}

static QVariant fcnExteriorRing( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  if ( fGeom.isEmpty() )
    return QVariant();

  QgsCurvePolygonV2* curvePolygon = dynamic_cast< QgsCurvePolygonV2* >( fGeom.geometry() );
  if ( !curvePolygon || !curvePolygon->exteriorRing() )
    return QVariant();

  QgsCurveV2* exterior = static_cast< QgsCurveV2* >( curvePolygon->exteriorRing()->clone() );
  QVariant result = exterior ? QVariant::fromValue( QgsGeometry( exterior ) ) : QVariant();
  return result;
}

static QVariant fcnDistance( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  return QVariant( fGeom.distance( sGeom ) );
}
static QVariant fcnIntersection( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  QgsGeometry* geom = fGeom.intersection( &sGeom );
  QVariant result = geom ? QVariant::fromValue( *geom ) : QVariant();
  delete geom;
  return result;
}
static QVariant fcnSymDifference( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  QgsGeometry* geom = fGeom.symDifference( &sGeom );
  QVariant result = geom ? QVariant::fromValue( *geom ) : QVariant();
  delete geom;
  return result;
}
static QVariant fcnCombine( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  QgsGeometry* geom = fGeom.combine( &sGeom );
  QVariant result = geom ? QVariant::fromValue( *geom ) : QVariant();
  delete geom;
  return result;
}
static QVariant fcnGeomToWKT( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  if ( values.length() < 1 || values.length() > 2 )
    return QVariant();

  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  int prec = 8;
  if ( values.length() == 2 )
    prec = getIntValue( values.at( 1 ), parent );
  QString wkt = fGeom.exportToWkt( prec );
  return QVariant( wkt );
}

static QVariant fcnAzimuth( const QVariantList& values, const QgsExpressionContext *, QgsExpression* parent )
{
  if ( values.length() != 2 )
  {
    parent->setEvalErrorString( QObject::tr( "Function `azimuth` requires exactly two parameters. %1 given." ).arg( values.length() ) );
    return QVariant();
  }

  QgsGeometry fGeom1 = getGeometry( values.at( 0 ), parent );
  QgsGeometry fGeom2 = getGeometry( values.at( 1 ), parent );

  const QgsPointV2* pt1 = dynamic_cast<const QgsPointV2*>( fGeom1.geometry() );
  const QgsPointV2* pt2 = dynamic_cast<const QgsPointV2*>( fGeom2.geometry() );

  if ( !pt1 || !pt2 )
  {
    parent->setEvalErrorString( QObject::tr( "Function `azimuth` requires two points as arguments." ) );
    return QVariant();
  }

  // Code from postgis
  if ( pt1->x() == pt2->x() )
  {
    if ( pt1->y() < pt2->y() )
      return 0.0;
    else if ( pt1->y() > pt2->y() )
      return M_PI;
    else
      return 0;
  }

  if ( pt1->y() == pt2->y() )
  {
    if ( pt1->x() < pt2->x() )
      return M_PI / 2;
    else if ( pt1->x() > pt2->x() )
      return M_PI + ( M_PI / 2 );
    else
      return 0;
  }

  if ( pt1->x() < pt2->x() )
  {
    if ( pt1->y() < pt2->y() )
    {
      return atan( fabs( pt1->x() - pt2->x() ) / fabs( pt1->y() - pt2->y() ) );
    }
    else /* ( pt1->y() > pt2->y() )  - equality case handled above */
    {
      return atan( fabs( pt1->y() - pt2->y() ) / fabs( pt1->x() - pt2->x() ) )
             + ( M_PI / 2 );
    }
  }

  else /* ( pt1->x() > pt2->x() ) - equality case handled above */
  {
    if ( pt1->y() > pt2->y() )
    {
      return atan( fabs( pt1->x() - pt2->x() ) / fabs( pt1->y() - pt2->y() ) )
             + M_PI;
    }
    else /* ( pt1->y() < pt2->y() )  - equality case handled above */
    {
      return atan( fabs( pt1->y() - pt2->y() ) / fabs( pt1->x() - pt2->x() ) )
             + ( M_PI + ( M_PI / 2 ) );
    }
  }
}

static QVariant fcnProject( const QVariantList& values, const QgsExpressionContext *, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.type() != QGis::Point )
  {
    parent->setEvalErrorString( "'project' requires a point geometry" );
    return QVariant();
  }

  double distance = getDoubleValue( values.at( 1 ), parent );
  double bearing = getDoubleValue( values.at( 2 ), parent );

  QgsPoint p = geom.asPoint();
  QgsPoint newPoint = p.project( distance, ( 180 * bearing ) / M_PI );

  return QVariant::fromValue( QgsGeometry( new QgsPointV2( newPoint.x(), newPoint.y() ) ) );
}

static QVariant fcnExtrude( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  if ( values.length() != 3 )
    return QVariant();

  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  double x = getDoubleValue( values.at( 1 ), parent );
  double y = getDoubleValue( values.at( 2 ), parent );

  QgsGeometry geom = fGeom.extrude( x, y );

  QVariant result = geom.geometry() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnOrderParts( const QVariantList& values, const QgsExpressionContext* ctx, QgsExpression* parent )
{
  if ( values.length() < 2 )
    return QVariant();

  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );

  if ( !fGeom.isMultipart() )
    return values.at( 0 );

  QString expString = getStringValue( values.at( 1 ), parent );
  bool asc = values.value( 2 ).toBool();

  QgsExpressionContext* unconstedContext;
  QgsFeature f;
  if ( ctx )
  {
    // ExpressionSorter wants a modifiable expression context, but it will return it in the same shape after
    // so no reason to worry
    unconstedContext = const_cast<QgsExpressionContext*>( ctx );
    f = ctx->feature();
  }
  else
  {
    // If there's no context provided, create a fake one
    unconstedContext = new QgsExpressionContext();
  }

  QgsGeometryCollectionV2* collection = dynamic_cast<QgsGeometryCollectionV2*>( fGeom.geometry() );
  Q_ASSERT( collection ); // Should have failed the multipart check above

  QgsFeatureRequest::OrderBy orderBy;
  orderBy.append( QgsFeatureRequest::OrderByClause( expString, asc ) );
  QgsExpressionSorter sorter( orderBy );

  QList<QgsFeature> partFeatures;
  for ( int i = 0; i < collection->partCount(); ++i )
  {
    f.setGeometry( QgsGeometry( collection->geometryN( i )->clone() ) );
    partFeatures << f;
  }

  sorter.sortFeatures( partFeatures, unconstedContext );

  QgsGeometryCollectionV2* orderedGeom = dynamic_cast<QgsGeometryCollectionV2*>( fGeom.geometry()->clone() );

  Q_ASSERT( orderedGeom );

  while ( orderedGeom->partCount() )
    orderedGeom->removeGeometry( 0 );

  Q_FOREACH ( const QgsFeature& feature, partFeatures )
  {
    orderedGeom->addGeometry( feature.constGeometry()->geometry()->clone() );
  }

  QVariant result = QVariant::fromValue( QgsGeometry( orderedGeom ) );

  if ( !ctx )
    delete unconstedContext;

  return result;
}

static QVariant fcnClosestPoint( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fromGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry toGeom = getGeometry( values.at( 1 ), parent );

  QgsGeometry geom = fromGeom.nearestPoint( toGeom );

  QVariant result = !geom.isEmpty() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnShortestLine( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fromGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry toGeom = getGeometry( values.at( 1 ), parent );

  QgsGeometry geom = fromGeom.shortestLine( toGeom );

  QVariant result = !geom.isEmpty() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnRound( const QVariantList& values, const QgsExpressionContext *, QgsExpression* parent )
{
  if ( values.length() == 2 )
  {
    double number = getDoubleValue( values.at( 0 ), parent );
    double scaler = pow( 10.0, getIntValue( values.at( 1 ), parent ) );
    return QVariant( qRound( number * scaler ) / scaler );
  }

  if ( values.length() == 1 )
  {
    double number = getIntValue( values.at( 0 ), parent );
    return QVariant( qRound( number ) ).toInt();
  }

  return QVariant();
}

static QVariant fcnPi( const QVariantList& values, const QgsExpressionContext *, QgsExpression* parent )
{
  Q_UNUSED( values );
  Q_UNUSED( parent );
  return M_PI;
}

static QVariant fcnScale( const QVariantList&, const QgsExpressionContext*, QgsExpression* parent )
{
  return QVariant( parent->scale() );
}

static QVariant fcnFormatNumber( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double value = getDoubleValue( values.at( 0 ), parent );
  int places = getIntValue( values.at( 1 ), parent );
  if ( places < 0 )
  {
    parent->setEvalErrorString( QObject::tr( "Number of places must be positive" ) );
    return QVariant();
  }
  return QString( "%L1" ).arg( value, 0, 'f', places );
}

static QVariant fcnFormatDate( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QDateTime dt = getDateTimeValue( values.at( 0 ), parent );
  QString format = getStringValue( values.at( 1 ), parent );
  return dt.toString( format );
}

static QVariant fcnColorRgb( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent )
{
  int red = getIntValue( values.at( 0 ), parent );
  int green = getIntValue( values.at( 1 ), parent );
  int blue = getIntValue( values.at( 2 ), parent );
  QColor color = QColor( red, green, blue );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3' to color" ).arg( red ).arg( green ).arg( blue ) );
    color = QColor( 0, 0, 0 );
  }

  return QString( "%1,%2,%3" ).arg( color.red() ).arg( color.green() ).arg( color.blue() );
}

static QVariant fcnIf( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent )
{
  QgsExpression::Node* node = getNode( values.at( 0 ), parent );
  ENSURE_NO_EVAL_ERROR;
  QVariant value = node->eval( parent, context );
  ENSURE_NO_EVAL_ERROR;
  if ( value.toBool() )
  {
    node = getNode( values.at( 1 ), parent );
    ENSURE_NO_EVAL_ERROR;
    value = node->eval( parent, context );
    ENSURE_NO_EVAL_ERROR;
  }
  else
  {
    node = getNode( values.at( 2 ), parent );
    ENSURE_NO_EVAL_ERROR;
    value = node->eval( parent, context );
    ENSURE_NO_EVAL_ERROR;
  }
  return value;
}

static QVariant fncColorRgba( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent )
{
  int red = getIntValue( values.at( 0 ), parent );
  int green = getIntValue( values.at( 1 ), parent );
  int blue = getIntValue( values.at( 2 ), parent );
  int alpha = getIntValue( values.at( 3 ), parent );
  QColor color = QColor( red, green, blue, alpha );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3:%4' to color" ).arg( red ).arg( green ).arg( blue ).arg( alpha ) );
    color = QColor( 0, 0, 0 );
  }
  return QgsSymbolLayerV2Utils::encodeColor( color );
}

QVariant fcnRampColor( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent )
{
  QString rampName = getStringValue( values.at( 0 ), parent );
  const QgsVectorColorRampV2 *mRamp = QgsStyleV2::defaultStyle()->colorRampRef( rampName );
  if ( ! mRamp )
  {
    parent->setEvalErrorString( QObject::tr( "\"%1\" is not a valid color ramp" ).arg( rampName ) );
    return QColor( 0, 0, 0 ).name();
  }
  double value = getDoubleValue( values.at( 1 ), parent );
  QColor color = mRamp->color( value );
  return QgsSymbolLayerV2Utils::encodeColor( color );
}

static QVariant fcnColorHsl( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent )
{
  // Hue ranges from 0 - 360
  double hue = getIntValue( values.at( 0 ), parent ) / 360.0;
  // Saturation ranges from 0 - 100
  double saturation = getIntValue( values.at( 1 ), parent ) / 100.0;
  // Lightness ranges from 0 - 100
  double lightness = getIntValue( values.at( 2 ), parent ) / 100.0;

  QColor color = QColor::fromHslF( hue, saturation, lightness );

  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3' to color" ).arg( hue ).arg( saturation ).arg( lightness ) );
    color = QColor( 0, 0, 0 );
  }

  return QString( "%1,%2,%3" ).arg( color.red() ).arg( color.green() ).arg( color.blue() );
}

static QVariant fncColorHsla( const QVariantList &values, const QgsExpressionContext*, QgsExpression *parent )
{
  // Hue ranges from 0 - 360
  double hue = getIntValue( values.at( 0 ), parent ) / 360.0;
  // Saturation ranges from 0 - 100
  double saturation = getIntValue( values.at( 1 ), parent ) / 100.0;
  // Lightness ranges from 0 - 100
  double lightness = getIntValue( values.at( 2 ), parent ) / 100.0;
  // Alpha ranges from 0 - 255
  double alpha = getIntValue( values.at( 3 ), parent ) / 255.0;

  QColor color = QColor::fromHslF( hue, saturation, lightness, alpha );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3:%4' to color" ).arg( hue ).arg( saturation ).arg( lightness ).arg( alpha ) );
    color = QColor( 0, 0, 0 );
  }
  return QgsSymbolLayerV2Utils::encodeColor( color );
}

static QVariant fcnColorHsv( const QVariantList &values, const QgsExpressionContext*, QgsExpression *parent )
{
  // Hue ranges from 0 - 360
  double hue = getIntValue( values.at( 0 ), parent ) / 360.0;
  // Saturation ranges from 0 - 100
  double saturation = getIntValue( values.at( 1 ), parent ) / 100.0;
  // Value ranges from 0 - 100
  double value = getIntValue( values.at( 2 ), parent ) / 100.0;

  QColor color = QColor::fromHsvF( hue, saturation, value );

  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3' to color" ).arg( hue ).arg( saturation ).arg( value ) );
    color = QColor( 0, 0, 0 );
  }

  return QString( "%1,%2,%3" ).arg( color.red() ).arg( color.green() ).arg( color.blue() );
}

static QVariant fncColorHsva( const QVariantList &values, const QgsExpressionContext*, QgsExpression *parent )
{
  // Hue ranges from 0 - 360
  double hue = getIntValue( values.at( 0 ), parent ) / 360.0;
  // Saturation ranges from 0 - 100
  double saturation = getIntValue( values.at( 1 ), parent ) / 100.0;
  // Value ranges from 0 - 100
  double value = getIntValue( values.at( 2 ), parent ) / 100.0;
  // Alpha ranges from 0 - 255
  double alpha = getIntValue( values.at( 3 ), parent ) / 255.0;

  QColor color = QColor::fromHsvF( hue, saturation, value, alpha );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3:%4' to color" ).arg( hue ).arg( saturation ).arg( value ).arg( alpha ) );
    color = QColor( 0, 0, 0 );
  }
  return QgsSymbolLayerV2Utils::encodeColor( color );
}

static QVariant fcnColorCmyk( const QVariantList &values, const QgsExpressionContext*, QgsExpression *parent )
{
  // Cyan ranges from 0 - 100
  double cyan = getIntValue( values.at( 0 ), parent ) / 100.0;
  // Magenta ranges from 0 - 100
  double magenta = getIntValue( values.at( 1 ), parent ) / 100.0;
  // Yellow ranges from 0 - 100
  double yellow = getIntValue( values.at( 2 ), parent ) / 100.0;
  // Black ranges from 0 - 100
  double black = getIntValue( values.at( 3 ), parent ) / 100.0;

  QColor color = QColor::fromCmykF( cyan, magenta, yellow, black );

  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3:%4' to color" ).arg( cyan ).arg( magenta ).arg( yellow ).arg( black ) );
    color = QColor( 0, 0, 0 );
  }

  return QString( "%1,%2,%3" ).arg( color.red() ).arg( color.green() ).arg( color.blue() );
}

static QVariant fncColorCmyka( const QVariantList &values, const QgsExpressionContext*, QgsExpression *parent )
{
  // Cyan ranges from 0 - 100
  double cyan = getIntValue( values.at( 0 ), parent ) / 100.0;
  // Magenta ranges from 0 - 100
  double magenta = getIntValue( values.at( 1 ), parent ) / 100.0;
  // Yellow ranges from 0 - 100
  double yellow = getIntValue( values.at( 2 ), parent ) / 100.0;
  // Black ranges from 0 - 100
  double black = getIntValue( values.at( 3 ), parent ) / 100.0;
  // Alpha ranges from 0 - 255
  double alpha = getIntValue( values.at( 4 ), parent ) / 255.0;

  QColor color = QColor::fromCmykF( cyan, magenta, yellow, black, alpha );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3:%4:%5' to color" ).arg( cyan ).arg( magenta ).arg( yellow ).arg( black ).arg( alpha ) );
    color = QColor( 0, 0, 0 );
  }
  return QgsSymbolLayerV2Utils::encodeColor( color );
}

static QVariant fncColorPart( const QVariantList &values, const QgsExpressionContext*, QgsExpression *parent )
{
  QColor color = QgsSymbolLayerV2Utils::decodeColor( values.at( 0 ).toString() );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to color" ).arg( values.at( 0 ).toString() ) );
    return QVariant();
  }

  QString part = getStringValue( values.at( 1 ), parent );
  if ( part.compare( QLatin1String( "red" ), Qt::CaseInsensitive ) == 0 )
    return color.red();
  else if ( part.compare( QLatin1String( "green" ), Qt::CaseInsensitive ) == 0 )
    return color.green();
  else if ( part.compare( QLatin1String( "blue" ), Qt::CaseInsensitive ) == 0 )
    return color.blue();
  else if ( part.compare( QLatin1String( "alpha" ), Qt::CaseInsensitive ) == 0 )
    return color.alpha();
  else if ( part.compare( QLatin1String( "hue" ), Qt::CaseInsensitive ) == 0 )
    return color.hsvHueF() * 360;
  else if ( part.compare( QLatin1String( "saturation" ), Qt::CaseInsensitive ) == 0 )
    return color.hsvSaturationF() * 100;
  else if ( part.compare( QLatin1String( "value" ), Qt::CaseInsensitive ) == 0 )
    return color.valueF() * 100;
  else if ( part.compare( QLatin1String( "hsl_hue" ), Qt::CaseInsensitive ) == 0 )
    return color.hslHueF() * 360;
  else if ( part.compare( QLatin1String( "hsl_saturation" ), Qt::CaseInsensitive ) == 0 )
    return color.hslSaturationF() * 100;
  else if ( part.compare( QLatin1String( "lightness" ), Qt::CaseInsensitive ) == 0 )
    return color.lightnessF() * 100;
  else if ( part.compare( QLatin1String( "cyan" ), Qt::CaseInsensitive ) == 0 )
    return color.cyanF() * 100;
  else if ( part.compare( QLatin1String( "magenta" ), Qt::CaseInsensitive ) == 0 )
    return color.magentaF() * 100;
  else if ( part.compare( QLatin1String( "yellow" ), Qt::CaseInsensitive ) == 0 )
    return color.yellowF() * 100;
  else if ( part.compare( QLatin1String( "black" ), Qt::CaseInsensitive ) == 0 )
    return color.blackF() * 100;

  parent->setEvalErrorString( QObject::tr( "Unknown color component '%1'" ).arg( part ) );
  return QVariant();
}

static QVariant fncSetColorPart( const QVariantList &values, const QgsExpressionContext*, QgsExpression *parent )
{
  QColor color = QgsSymbolLayerV2Utils::decodeColor( values.at( 0 ).toString() );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to color" ).arg( values.at( 0 ).toString() ) );
    return QVariant();
  }

  QString part = getStringValue( values.at( 1 ), parent );
  int value = getIntValue( values.at( 2 ), parent );
  if ( part.compare( QLatin1String( "red" ), Qt::CaseInsensitive ) == 0 )
    color.setRed( value );
  else if ( part.compare( QLatin1String( "green" ), Qt::CaseInsensitive ) == 0 )
    color.setGreen( value );
  else if ( part.compare( QLatin1String( "blue" ), Qt::CaseInsensitive ) == 0 )
    color.setBlue( value );
  else if ( part.compare( QLatin1String( "alpha" ), Qt::CaseInsensitive ) == 0 )
    color.setAlpha( value );
  else if ( part.compare( QLatin1String( "hue" ), Qt::CaseInsensitive ) == 0 )
    color.setHsv( value, color.hsvSaturation(), color.value(), color.alpha() );
  else if ( part.compare( QLatin1String( "saturation" ), Qt::CaseInsensitive ) == 0 )
    color.setHsvF( color.hsvHueF(), value / 100.0, color.valueF(), color.alphaF() );
  else if ( part.compare( QLatin1String( "value" ), Qt::CaseInsensitive ) == 0 )
    color.setHsvF( color.hsvHueF(), color.hsvSaturationF(), value / 100.0, color.alphaF() );
  else if ( part.compare( QLatin1String( "hsl_hue" ), Qt::CaseInsensitive ) == 0 )
    color.setHsl( value, color.hslSaturation(), color.lightness(), color.alpha() );
  else if ( part.compare( QLatin1String( "hsl_saturation" ), Qt::CaseInsensitive ) == 0 )
    color.setHslF( color.hslHueF(), value / 100.0, color.lightnessF(), color.alphaF() );
  else if ( part.compare( QLatin1String( "lightness" ), Qt::CaseInsensitive ) == 0 )
    color.setHslF( color.hslHueF(), color.hslSaturationF(), value / 100.0, color.alphaF() );
  else if ( part.compare( QLatin1String( "cyan" ), Qt::CaseInsensitive ) == 0 )
    color.setCmykF( value / 100.0, color.magentaF(), color.yellowF(), color.blackF(), color.alphaF() );
  else if ( part.compare( QLatin1String( "magenta" ), Qt::CaseInsensitive ) == 0 )
    color.setCmykF( color.cyanF(), value / 100.0, color.yellowF(), color.blackF(), color.alphaF() );
  else if ( part.compare( QLatin1String( "yellow" ), Qt::CaseInsensitive ) == 0 )
    color.setCmykF( color.cyanF(), color.magentaF(), value / 100.0, color.blackF(), color.alphaF() );
  else if ( part.compare( QLatin1String( "black" ), Qt::CaseInsensitive ) == 0 )
    color.setCmykF( color.cyanF(), color.magentaF(), color.yellowF(), value / 100.0, color.alphaF() );
  else
  {
    parent->setEvalErrorString( QObject::tr( "Unknown color component '%1'" ).arg( part ) );
    return QVariant();
  }
  return QgsSymbolLayerV2Utils::encodeColor( color );
}

static QVariant fncDarker( const QVariantList &values, const QgsExpressionContext*, QgsExpression *parent )
{
  QColor color = QgsSymbolLayerV2Utils::decodeColor( values.at( 0 ).toString() );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to color" ).arg( values.at( 0 ).toString() ) );
    return QVariant();
  }

  color = color.darker( getIntValue( values.at( 1 ), parent ) );

  return QgsSymbolLayerV2Utils::encodeColor( color );
}

static QVariant fncLighter( const QVariantList &values, const QgsExpressionContext*, QgsExpression *parent )
{
  QColor color = QgsSymbolLayerV2Utils::decodeColor( values.at( 0 ).toString() );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to color" ).arg( values.at( 0 ).toString() ) );
    return QVariant();
  }

  color = color.lighter( getIntValue( values.at( 1 ), parent ) );

  return QgsSymbolLayerV2Utils::encodeColor( color );
}

static QVariant fcnSpecialColumn( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString varName = getStringValue( values.at( 0 ), parent );
  Q_NOWARN_DEPRECATED_PUSH
  return QgsExpression::specialColumn( varName );
  Q_NOWARN_DEPRECATED_POP
}

static QVariant fcnGetGeometry( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsFeature feat = getFeature( values.at( 0 ), parent );
  const QgsGeometry* geom = feat.constGeometry();
  if ( geom )
    return QVariant::fromValue( *geom );
  return QVariant();
}

static QVariant fcnTransformGeometry( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QString sAuthId = getStringValue( values.at( 1 ), parent );
  QString dAuthId = getStringValue( values.at( 2 ), parent );

  QgsCoordinateReferenceSystem s = QgsCRSCache::instance()->crsByOgcWmsCrs( sAuthId );
  if ( ! s.isValid() )
    return QVariant::fromValue( fGeom );
  QgsCoordinateReferenceSystem d = QgsCRSCache::instance()->crsByOgcWmsCrs( dAuthId );
  if ( ! d.isValid() )
    return QVariant::fromValue( fGeom );

  QgsCoordinateTransform t( s, d );
  if ( fGeom.transform( t ) == 0 )
    return QVariant::fromValue( fGeom );
  return QVariant();
}


static QVariant fcnGetFeature( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  //arguments: 1. layer id / name, 2. key attribute, 3. eq value
  QgsVectorLayer* vl = getVectorLayer( values.at( 0 ), parent );

  //no layer found
  if ( !vl )
  {
    return QVariant();
  }

  QString attribute = getStringValue( values.at( 1 ), parent );
  int attributeId = vl->fieldNameIndex( attribute );
  if ( attributeId == -1 )
  {
    return QVariant();
  }

  const QVariant& attVal = values.at( 2 );
  QgsFeatureRequest req;
  req.setFilterExpression( QString( "%1=%2" ).arg( QgsExpression::quotedColumnRef( attribute ),
                           QgsExpression::quotedString( attVal.toString() ) ) );
  req.setLimit( 1 );
  if ( !parent->needsGeometry() )
  {
    req.setFlags( QgsFeatureRequest::NoGeometry );
  }
  QgsFeatureIterator fIt = vl->getFeatures( req );

  QgsFeature fet;
  if ( fIt.nextFeature( fet ) )
    return QVariant::fromValue( fet );

  return QVariant();
}

static QVariant fcnGetLayerProperty( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString layerIdOrName = getStringValue( values.at( 0 ), parent );

  //try to find a matching layer by name
  QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( layerIdOrName ); //search by id first
  if ( !layer )
  {
    QList<QgsMapLayer *> layersByName = QgsMapLayerRegistry::instance()->mapLayersByName( layerIdOrName );
    if ( !layersByName.isEmpty() )
    {
      layer = layersByName.at( 0 );
    }
  }

  if ( !layer )
    return QVariant();

  QString layerProperty = getStringValue( values.at( 1 ), parent );
  if ( QString::compare( layerProperty, QString( "name" ), Qt::CaseInsensitive ) == 0 )
    return layer->name();
  else if ( QString::compare( layerProperty, QString( "id" ), Qt::CaseInsensitive ) == 0 )
    return layer->id();
  else if ( QString::compare( layerProperty, QString( "title" ), Qt::CaseInsensitive ) == 0 )
    return layer->title();
  else if ( QString::compare( layerProperty, QString( "abstract" ), Qt::CaseInsensitive ) == 0 )
    return layer->abstract();
  else if ( QString::compare( layerProperty, QString( "keywords" ), Qt::CaseInsensitive ) == 0 )
    return layer->keywordList();
  else if ( QString::compare( layerProperty, QString( "data_url" ), Qt::CaseInsensitive ) == 0 )
    return layer->dataUrl();
  else if ( QString::compare( layerProperty, QString( "attribution" ), Qt::CaseInsensitive ) == 0 )
    return layer->attribution();
  else if ( QString::compare( layerProperty, QString( "attribution_url" ), Qt::CaseInsensitive ) == 0 )
    return layer->attributionUrl();
  else if ( QString::compare( layerProperty, QString( "source" ), Qt::CaseInsensitive ) == 0 )
    return layer->publicSource();
  else if ( QString::compare( layerProperty, QString( "min_scale" ), Qt::CaseInsensitive ) == 0 )
    return layer->minimumScale();
  else if ( QString::compare( layerProperty, QString( "max_scale" ), Qt::CaseInsensitive ) == 0 )
    return layer->maximumScale();
  else if ( QString::compare( layerProperty, QString( "crs" ), Qt::CaseInsensitive ) == 0 )
    return layer->crs().authid();
  else if ( QString::compare( layerProperty, QString( "crs_definition" ), Qt::CaseInsensitive ) == 0 )
    return layer->crs().toProj4();
  else if ( QString::compare( layerProperty, QString( "extent" ), Qt::CaseInsensitive ) == 0 )
  {
    QgsGeometry* extentGeom = QgsGeometry::fromRect( layer->extent() );
    QVariant result = QVariant::fromValue( *extentGeom );
    delete extentGeom;
    return result;
  }
  else if ( QString::compare( layerProperty, QString( "type" ), Qt::CaseInsensitive ) == 0 )
  {
    switch ( layer->type() )
    {
      case QgsMapLayer::VectorLayer:
        return QCoreApplication::translate( "expressions", "Vector" );
      case QgsMapLayer::RasterLayer:
        return QCoreApplication::translate( "expressions", "Raster" );
      case QgsMapLayer::PluginLayer:
        return QCoreApplication::translate( "expressions", "Plugin" );
    }
  }
  else
  {
    //vector layer methods
    QgsVectorLayer* vLayer = dynamic_cast< QgsVectorLayer* >( layer );
    if ( vLayer )
    {
      if ( QString::compare( layerProperty, QString( "storage_type" ), Qt::CaseInsensitive ) == 0 )
        return vLayer->storageType();
      else if ( QString::compare( layerProperty, QString( "geometry_type" ), Qt::CaseInsensitive ) == 0 )
        return QGis::vectorGeometryType( vLayer->geometryType() );
      else if ( QString::compare( layerProperty, QString( "feature_count" ), Qt::CaseInsensitive ) == 0 )
        return QVariant::fromValue( vLayer->featureCount() );
    }
  }

  return QVariant();
}

bool QgsExpression::registerFunction( QgsExpression::Function* function, bool transferOwnership )
{
  int fnIdx = functionIndex( function->name() );
  if ( fnIdx != -1 )
  {
    return false;
  }
  QgsExpression::gmFunctions.append( function );
  if ( transferOwnership )
    QgsExpression::gmOwnedFunctions.append( function );
  return true;
}

bool QgsExpression::unregisterFunction( const QString& name )
{
  // You can never override the built in functions.
  if ( QgsExpression::BuiltinFunctions().contains( name ) )
  {
    return false;
  }
  int fnIdx = functionIndex( name );
  if ( fnIdx != -1 )
  {
    QgsExpression::gmFunctions.removeAt( fnIdx );
    return true;
  }
  return false;
}

void QgsExpression::cleanRegisteredFunctions()
{
  qDeleteAll( QgsExpression::gmOwnedFunctions );
  QgsExpression::gmOwnedFunctions.clear();
}

QStringList QgsExpression::gmBuiltinFunctions;

const QStringList& QgsExpression::BuiltinFunctions()
{
  if ( gmBuiltinFunctions.isEmpty() )
  {
    gmBuiltinFunctions
    << "abs" << "sqrt" << "cos" << "sin" << "tan"
    << "asin" << "acos" << "atan" << "atan2"
    << "exp" << "ln" << "log10" << "log"
    << "round" << "rand" << "randf" << "max" << "min" << "clamp"
    << "scale_linear" << "scale_exp" << "floor" << "ceil" << "$pi"
    << "toint" << "to_int" << "toreal" << "to_real" << "tostring" << "to_string"
    << "todatetime" << "to_datetime" << "todate" << "to_date"
    << "totime" << "to_time" << "tointerval" << "to_interval"
    << "coalesce" << "if" << "regexp_match" << "age" << "year"
    << "month" << "week" << "day" << "hour" << "day_of_week"
    << "minute" << "second" << "lower" << "upper"
    << "title" << "length" << "replace" << "trim" << "wordwrap"
    << "regexp_replace" << "regexp_substr"
    << "substr" << "concat" << "strpos" << "left"
    << "right" << "rpad" << "lpad" << "format"
    << "format_number" << "format_date"
    << "color_rgb" << "color_rgba" << "ramp_color"
    << "color_hsl" << "color_hsla" << "color_hsv" << "color_hsva"
    << "color_cmyk" << "color_cmyka" << "color_part" << "set_color_part"
    << "xat" << "yat" << "$area" << "area" << "perimeter"
    << "$length" << "$perimeter" << "x" << "y" << "$x" << "$y" << "z" << "m" << "num_points"
    << "num_interior_rings" << "num_rings" << "num_geometries"
    << "geometry_n" << "interior_ring_n"
    << "point_n" << "start_point" << "end_point" << "make_point" << "make_point_m"
    << "nodes_to_points" << "segments_to_lines"
    << "make_line" << "make_polygon"
    << "$x_at" << "x_at" << "xat" << "$y_at" << "y_at" << "yat" << "x_min" << "xmin" << "x_max" << "xmax"
    << "y_min" << "ymin" << "y_max" << "ymax" << "geom_from_wkt" << "geomFromWKT"
    << "geom_from_gml" << "geomFromGML" << "intersects_bbox" << "bbox"
    << "disjoint" << "intersects" << "touches" << "crosses" << "contains"
    << "relate"
    << "overlaps" << "within" << "buffer" << "centroid" << "bounds" << "reverse" << "exterior_ring"
    << "bounds_width" << "bounds_height" << "is_closed" << "convex_hull" << "difference"
    << "distance" << "intersection" << "sym_difference" << "combine"
    << "extrude" << "azimuth" <<  "project" << "closest_point" << "shortest_line"
    << "union" << "geom_to_wkt" << "geomToWKT" << "geometry"
    << "transform" << "get_feature" << "getFeature"
    << "levenshtein" << "longest_common_substring" << "hamming_distance"
    << "soundex"
    << "aggregate" << "relation_aggregate" << "count" << "count_distinct"
    << "count_missing" << "minimum" << "maximum" << "sum" << "mean"
    << "median" << "stdev" << "range" << "minority" << "majority"
    << "q1" << "q3" << "iqr" << "min_length" << "max_length" << "concatenate"
    << "attribute" << "var" << "layer_property"
    << "$id" << "$scale" << "_specialcol_";
  }
  return gmBuiltinFunctions;
}

QList<QgsExpression::Function*> QgsExpression::gmFunctions;
QList<QgsExpression::Function*> QgsExpression::gmOwnedFunctions;

const QList<QgsExpression::Function*>& QgsExpression::Functions()
{
  // The construction of the list isn't thread-safe, and without the mutex,
  // crashes in the WFS provider may occur, since it can parse expressions
  // in parallel.
  // The mutex needs to be recursive.
  static QMutex sFunctionsMutex( QMutex::Recursive );
  QMutexLocker locker( &sFunctionsMutex );

  if ( gmFunctions.isEmpty() )
  {
    ParameterList aggParams = ParameterList() << Parameter( "expression" )
                              << Parameter( "group_by", true )
                              << Parameter( "filter", true );

    gmFunctions
    << new StaticFunction( "sqrt", ParameterList() << Parameter( "value" ), fcnSqrt, "Math" )
    << new StaticFunction( "radians", ParameterList() << Parameter( "degrees" ), fcnRadians, "Math" )
    << new StaticFunction( "degrees", ParameterList() << Parameter( "radians" ), fcnDegrees, "Math" )
    << new StaticFunction( "azimuth", ParameterList() << Parameter( "point_a" ) << Parameter( "point_b" ), fcnAzimuth, "Math" )
    << new StaticFunction( "project", ParameterList() << Parameter( "point" ) << Parameter( "distance" ) << Parameter( "bearing" ), fcnProject, "GeometryGroup" )
    << new StaticFunction( "abs", ParameterList() << Parameter( "value" ), fcnAbs, "Math" )
    << new StaticFunction( "cos", ParameterList() << Parameter( "angle" ), fcnCos, "Math" )
    << new StaticFunction( "sin", ParameterList() << Parameter( "angle" ), fcnSin, "Math" )
    << new StaticFunction( "tan", ParameterList() << Parameter( "angle" ), fcnTan, "Math" )
    << new StaticFunction( "asin", ParameterList() << Parameter( "value" ), fcnAsin, "Math" )
    << new StaticFunction( "acos", ParameterList() << Parameter( "value" ), fcnAcos, "Math" )
    << new StaticFunction( "atan", ParameterList() << Parameter( "value" ), fcnAtan, "Math" )
    << new StaticFunction( "atan2", ParameterList() << Parameter( "dx" ) << Parameter( "dy" ), fcnAtan2, "Math" )
    << new StaticFunction( "exp", ParameterList() << Parameter( "value" ), fcnExp, "Math" )
    << new StaticFunction( "ln", ParameterList() << Parameter( "value" ), fcnLn, "Math" )
    << new StaticFunction( "log10", ParameterList() << Parameter( "value" ), fcnLog10, "Math" )
    << new StaticFunction( "log", ParameterList() << Parameter( "base" ) << Parameter( "value" ), fcnLog, "Math" )
    << new StaticFunction( "round", ParameterList() << Parameter( "value" ) << Parameter( "places", true, 0 ), fcnRound, "Math" )
    << new StaticFunction( "rand", ParameterList() << Parameter( "min" ) << Parameter( "max" ), fcnRnd, "Math" )
    << new StaticFunction( "randf", ParameterList() << Parameter( "min", true, 0.0 ) << Parameter( "max", true, 1.0 ), fcnRndF, "Math" )
    << new StaticFunction( "max", -1, fcnMax, "Math" )
    << new StaticFunction( "min", -1, fcnMin, "Math" )
    << new StaticFunction( "clamp", ParameterList() << Parameter( "min" ) << Parameter( "value" ) << Parameter( "max" ), fcnClamp, "Math" )
    << new StaticFunction( "scale_linear", 5, fcnLinearScale, "Math" )
    << new StaticFunction( "scale_exp", 6, fcnExpScale, "Math" )
    << new StaticFunction( "floor", 1, fcnFloor, "Math" )
    << new StaticFunction( "ceil", 1, fcnCeil, "Math" )
    << new StaticFunction( "pi", 0, fcnPi, "Math", QString(), false, QStringList(), false, QStringList() << "$pi" )
    << new StaticFunction( "to_int", 1, fcnToInt, "Conversions", QString(), false, QStringList(), false, QStringList() << "toint" )
    << new StaticFunction( "to_real", 1, fcnToReal, "Conversions", QString(), false, QStringList(), false, QStringList() << "toreal" )
    << new StaticFunction( "to_string", 1, fcnToString, "Conversions", QString(), false, QStringList(), false, QStringList() << "tostring" )
    << new StaticFunction( "to_datetime", 1, fcnToDateTime, "Conversions", QString(), false, QStringList(), false, QStringList() << "todatetime" )
    << new StaticFunction( "to_date", 1, fcnToDate, "Conversions", QString(), false, QStringList(), false, QStringList() << "todate" )
    << new StaticFunction( "to_time", 1, fcnToTime, "Conversions", QString(), false, QStringList(), false, QStringList() << "totime" )
    << new StaticFunction( "to_interval", 1, fcnToInterval, "Conversions", QString(), false, QStringList(), false, QStringList() << "tointerval" )
    << new StaticFunction( "coalesce", -1, fcnCoalesce, "Conditionals", QString(), false, QStringList(), false, QStringList(), true )
    << new StaticFunction( "if", 3, fcnIf, "Conditionals", QString(), False, QStringList(), true )
    << new StaticFunction( "aggregate", ParameterList() << Parameter( "layer" ) << Parameter( "aggregate" ) << Parameter( "expression" )
                           << Parameter( "filter", true ) << Parameter( "concatenator", true ), fcnAggregate, "Aggregates", QString(), False, QStringList(), true )
    << new StaticFunction( "relation_aggregate", ParameterList() << Parameter( "relation" ) << Parameter( "aggregate" ) << Parameter( "expression" ) << Parameter( "concatenator", true ),
                           fcnAggregateRelation, "Aggregates", QString(), False, QStringList( QgsFeatureRequest::AllAttributes ), true )

    << new StaticFunction( "count", aggParams, fcnAggregateCount, "Aggregates", QString(), False, QStringList(), true )
    << new StaticFunction( "count_distinct", aggParams, fcnAggregateCountDistinct, "Aggregates", QString(), False, QStringList(), true )
    << new StaticFunction( "count_missing", aggParams, fcnAggregateCountMissing, "Aggregates", QString(), False, QStringList(), true )
    << new StaticFunction( "minimum", aggParams, fcnAggregateMin, "Aggregates", QString(), False, QStringList(), true )
    << new StaticFunction( "maximum", aggParams, fcnAggregateMax, "Aggregates", QString(), False, QStringList(), true )
    << new StaticFunction( "sum", aggParams, fcnAggregateSum, "Aggregates", QString(), False, QStringList(), true )
    << new StaticFunction( "mean", aggParams, fcnAggregateMean, "Aggregates", QString(), False, QStringList(), true )
    << new StaticFunction( "median", aggParams, fcnAggregateMedian, "Aggregates", QString(), False, QStringList(), true )
    << new StaticFunction( "stdev", aggParams, fcnAggregateStdev, "Aggregates", QString(), False, QStringList(), true )
    << new StaticFunction( "range", aggParams, fcnAggregateRange, "Aggregates", QString(), False, QStringList(), true )
    << new StaticFunction( "minority", aggParams, fcnAggregateMinority, "Aggregates", QString(), False, QStringList(), true )
    << new StaticFunction( "majority", aggParams, fcnAggregateMajority, "Aggregates", QString(), False, QStringList(), true )
    << new StaticFunction( "q1", aggParams, fcnAggregateQ1, "Aggregates", QString(), False, QStringList(), true )
    << new StaticFunction( "q3", aggParams, fcnAggregateQ3, "Aggregates", QString(), False, QStringList(), true )
    << new StaticFunction( "iqr", aggParams, fcnAggregateIQR, "Aggregates", QString(), False, QStringList(), true )
    << new StaticFunction( "min_length", aggParams, fcnAggregateMinLength, "Aggregates", QString(), False, QStringList(), true )
    << new StaticFunction( "max_length", aggParams, fcnAggregateMaxLength, "Aggregates", QString(), False, QStringList(), true )
    << new StaticFunction( "concatenate", aggParams << Parameter( "concatenator", true ), fcnAggregateStringConcat, "Aggregates", QString(), False, QStringList(), true )

    << new StaticFunction( "regexp_match", 2, fcnRegexpMatch, "Conditionals" )
    << new StaticFunction( "now", 0, fcnNow, "Date and Time", QString(), false, QStringList(), false, QStringList() << "$now" )
    << new StaticFunction( "age", 2, fcnAge, "Date and Time" )
    << new StaticFunction( "year", 1, fcnYear, "Date and Time" )
    << new StaticFunction( "month", 1, fcnMonth, "Date and Time" )
    << new StaticFunction( "week", 1, fcnWeek, "Date and Time" )
    << new StaticFunction( "day", 1, fcnDay, "Date and Time" )
    << new StaticFunction( "hour", 1, fcnHour, "Date and Time" )
    << new StaticFunction( "minute", 1, fcnMinute, "Date and Time" )
    << new StaticFunction( "second", 1, fcnSeconds, "Date and Time" )
    << new StaticFunction( "day_of_week", 1, fcnDayOfWeek, "Date and Time" )
    << new StaticFunction( "lower", 1, fcnLower, "String" )
    << new StaticFunction( "upper", 1, fcnUpper, "String" )
    << new StaticFunction( "title", 1, fcnTitle, "String" )
    << new StaticFunction( "trim", 1, fcnTrim, "String" )
    << new StaticFunction( "levenshtein", 2, fcnLevenshtein, "Fuzzy Matching" )
    << new StaticFunction( "longest_common_substring", 2, fcnLCS, "Fuzzy Matching" )
    << new StaticFunction( "hamming_distance", 2, fcnHamming, "Fuzzy Matching" )
    << new StaticFunction( "soundex", 1, fcnSoundex, "Fuzzy Matching" )
    << new StaticFunction( "char", 1, fcnChar, "String" )
    << new StaticFunction( "wordwrap", ParameterList() << Parameter( "text" ) << Parameter( "length" ) << Parameter( "delimiter", true, " " ), fcnWordwrap, "String" )
    << new StaticFunction( "length", 1, fcnLength, "String" )
    << new StaticFunction( "replace", 3, fcnReplace, "String" )
    << new StaticFunction( "regexp_replace", 3, fcnRegexpReplace, "String" )
    << new StaticFunction( "regexp_substr", 2, fcnRegexpSubstr, "String" )
    << new StaticFunction( "substr", 3, fcnSubstr, "String" )
    << new StaticFunction( "concat", -1, fcnConcat, "String", QString(), false, QStringList(), false, QStringList(), true )
    << new StaticFunction( "strpos", 2, fcnStrpos, "String" )
    << new StaticFunction( "left", 2, fcnLeft, "String" )
    << new StaticFunction( "right", 2, fcnRight, "String" )
    << new StaticFunction( "rpad", 3, fcnRPad, "String" )
    << new StaticFunction( "lpad", 3, fcnLPad, "String" )
    << new StaticFunction( "format", -1, fcnFormatString, "String" )
    << new StaticFunction( "format_number", 2, fcnFormatNumber, "String" )
    << new StaticFunction( "format_date", 2, fcnFormatDate, "String" )
    << new StaticFunction( "color_rgb", 3, fcnColorRgb, "Color" )
    << new StaticFunction( "color_rgba", 4, fncColorRgba, "Color" )
    << new StaticFunction( "ramp_color", 2, fcnRampColor, "Color" )
    << new StaticFunction( "color_hsl", 3, fcnColorHsl, "Color" )
    << new StaticFunction( "color_hsla", 4, fncColorHsla, "Color" )
    << new StaticFunction( "color_hsv", 3, fcnColorHsv, "Color" )
    << new StaticFunction( "color_hsva", 4, fncColorHsva, "Color" )
    << new StaticFunction( "color_cmyk", 4, fcnColorCmyk, "Color" )
    << new StaticFunction( "color_cmyka", 5, fncColorCmyka, "Color" )
    << new StaticFunction( "color_part", 2, fncColorPart, "Color" )
    << new StaticFunction( "darker", 2, fncDarker, "Color" )
    << new StaticFunction( "lighter", 2, fncLighter, "Color" )
    << new StaticFunction( "set_color_part", 3, fncSetColorPart, "Color" )
    << new StaticFunction( "$geometry", 0, fcnGeometry, "GeometryGroup", QString(), true )
    << new StaticFunction( "$area", 0, fcnGeomArea, "GeometryGroup", QString(), true )
    << new StaticFunction( "area", 1, fcnArea, "GeometryGroup" )
    << new StaticFunction( "$length", 0, fcnGeomLength, "GeometryGroup", QString(), true )
    << new StaticFunction( "$perimeter", 0, fcnGeomPerimeter, "GeometryGroup", QString(), true )
    << new StaticFunction( "perimeter", 1, fcnPerimeter, "GeometryGroup" )
    << new StaticFunction( "$x", 0, fcnX, "GeometryGroup", QString(), true )
    << new StaticFunction( "$y", 0, fcnY, "GeometryGroup", QString(), true )
    << new StaticFunction( "x", 1, fcnGeomX, "GeometryGroup" )
    << new StaticFunction( "y", 1, fcnGeomY, "GeometryGroup" )
    << new StaticFunction( "z", 1, fcnGeomZ, "GeometryGroup" )
    << new StaticFunction( "m", 1, fcnGeomM, "GeometryGroup" )
    << new StaticFunction( "point_n", 2, fcnPointN, "GeometryGroup" )
    << new StaticFunction( "start_point", 1, fcnStartPoint, "GeometryGroup" )
    << new StaticFunction( "end_point", 1, fcnEndPoint, "GeometryGroup" )
    << new StaticFunction( "nodes_to_points", -1, fcnNodesToPoints, "GeometryGroup" )
    << new StaticFunction( "segments_to_lines", 1, fcnSegmentsToLines, "GeometryGroup" )
    << new StaticFunction( "make_point", -1, fcnMakePoint, "GeometryGroup" )
    << new StaticFunction( "make_point_m", 3, fcnMakePointM, "GeometryGroup" )
    << new StaticFunction( "make_line", -1, fcnMakeLine, "GeometryGroup" )
    << new StaticFunction( "make_polygon", -1, fcnMakePolygon, "GeometryGroup" )
    << new StaticFunction( "$x_at", 1, fcnXat, "GeometryGroup", QString(), true, QStringList(), false, QStringList() << "xat" << "x_at" )
    << new StaticFunction( "$y_at", 1, fcnYat, "GeometryGroup", QString(), true, QStringList(), false, QStringList() << "yat" << "y_at" )
    << new StaticFunction( "x_min", 1, fcnXMin, "GeometryGroup", QString(), false, QStringList(), false, QStringList() << "xmin" )
    << new StaticFunction( "x_max", 1, fcnXMax, "GeometryGroup", QString(), false, QStringList(), false, QStringList() << "xmax" )
    << new StaticFunction( "y_min", 1, fcnYMin, "GeometryGroup", QString(), false, QStringList(), false, QStringList() << "ymin" )
    << new StaticFunction( "y_max", 1, fcnYMax, "GeometryGroup", QString(), false, QStringList(), false, QStringList() << "ymax" )
    << new StaticFunction( "geom_from_wkt", 1, fcnGeomFromWKT, "GeometryGroup", QString(), false, QStringList(), false, QStringList() << "geomFromWKT" )
    << new StaticFunction( "geom_from_gml", 1, fcnGeomFromGML, "GeometryGroup", QString(), false, QStringList(), false, QStringList() << "geomFromGML" )
    << new StaticFunction( "relate", -1, fcnRelate, "GeometryGroup" )
    << new StaticFunction( "intersects_bbox", 2, fcnBbox, "GeometryGroup", QString(), false, QStringList(), false, QStringList() << "bbox" )
    << new StaticFunction( "disjoint", 2, fcnDisjoint, "GeometryGroup" )
    << new StaticFunction( "intersects", 2, fcnIntersects, "GeometryGroup" )
    << new StaticFunction( "touches", 2, fcnTouches, "GeometryGroup" )
    << new StaticFunction( "crosses", 2, fcnCrosses, "GeometryGroup" )
    << new StaticFunction( "contains", 2, fcnContains, "GeometryGroup" )
    << new StaticFunction( "overlaps", 2, fcnOverlaps, "GeometryGroup" )
    << new StaticFunction( "within", 2, fcnWithin, "GeometryGroup" )
    << new StaticFunction( "translate", 3, fcnTranslate, "GeometryGroup" )
    << new StaticFunction( "buffer", -1, fcnBuffer, "GeometryGroup" )
    << new StaticFunction( "centroid", 1, fcnCentroid, "GeometryGroup" )
    << new StaticFunction( "point_on_surface", 1, fcnPointOnSurface, "GeometryGroup" )
    << new StaticFunction( "reverse", 1, fcnReverse, "GeometryGroup" )
    << new StaticFunction( "exterior_ring", 1, fcnExteriorRing, "GeometryGroup" )
    << new StaticFunction( "interior_ring_n", 2, fcnInteriorRingN, "GeometryGroup" )
    << new StaticFunction( "geometry_n", 2, fcnGeometryN, "GeometryGroup" )
    << new StaticFunction( "bounds", 1, fcnBounds, "GeometryGroup" )
    << new StaticFunction( "num_points", 1, fcnGeomNumPoints, "GeometryGroup" )
    << new StaticFunction( "num_interior_rings", 1, fcnGeomNumInteriorRings, "GeometryGroup" )
    << new StaticFunction( "num_rings", 1, fcnGeomNumRings, "GeometryGroup" )
    << new StaticFunction( "num_geometries", 1, fcnGeomNumGeometries, "GeometryGroup" )
    << new StaticFunction( "bounds_width", 1, fcnBoundsWidth, "GeometryGroup" )
    << new StaticFunction( "bounds_height", 1, fcnBoundsHeight, "GeometryGroup" )
    << new StaticFunction( "is_closed", 1, fcnIsClosed, "GeometryGroup" )
    << new StaticFunction( "convex_hull", 1, fcnConvexHull, "GeometryGroup", QString(), false, QStringList(), false, QStringList() << "convexHull" )
    << new StaticFunction( "difference", 2, fcnDifference, "GeometryGroup" )
    << new StaticFunction( "distance", 2, fcnDistance, "GeometryGroup" )
    << new StaticFunction( "intersection", 2, fcnIntersection, "GeometryGroup" )
    << new StaticFunction( "sym_difference", 2, fcnSymDifference, "GeometryGroup", QString(), false, QStringList(), false, QStringList() << "symDifference" )
    << new StaticFunction( "combine", 2, fcnCombine, "GeometryGroup" )
    << new StaticFunction( "union", 2, fcnCombine, "GeometryGroup" )
    << new StaticFunction( "geom_to_wkt", -1, fcnGeomToWKT, "GeometryGroup", QString(), false, QStringList(), false, QStringList() << "geomToWKT" )
    << new StaticFunction( "geometry", 1, fcnGetGeometry, "GeometryGroup", QString(), true )
    << new StaticFunction( "transform", 3, fcnTransformGeometry, "GeometryGroup" )
    << new StaticFunction( "extrude", 3, fcnExtrude, "GeometryGroup", QString() )
    << new StaticFunction( "order_parts", 3, fcnOrderParts, "GeometryGroup", QString() )
    << new StaticFunction( "closest_point", 2, fcnClosestPoint, "GeometryGroup" )
    << new StaticFunction( "shortest_line", 2, fcnShortestLine, "GeometryGroup" )
    << new StaticFunction( "$rownum", 0, fcnRowNumber, "deprecated" )
    << new StaticFunction( "$id", 0, fcnFeatureId, "Record" )
    << new StaticFunction( "$currentfeature", 0, fcnFeature, "Record" )
    << new StaticFunction( "$scale", 0, fcnScale, "Record" )
    << new StaticFunction( "$map", 0, fcnMapId, "deprecated" )
    << new StaticFunction( "$numpages", 0, fcnComposerNumPages, "deprecated" )
    << new StaticFunction( "$page", 0, fcnComposerPage, "deprecated" )
    << new StaticFunction( "$feature", 0, fcnAtlasFeature, "deprecated" )
    << new StaticFunction( "$atlasfeatureid", 0, fcnAtlasFeatureId, "deprecated" )
    << new StaticFunction( "$atlasfeature", 0, fcnAtlasCurrentFeature, "deprecated" )
    << new StaticFunction( "$atlasgeometry", 0, fcnAtlasCurrentGeometry, "deprecated" )
    << new StaticFunction( "$numfeatures", 0, fcnAtlasNumFeatures, "deprecated" )
    << new StaticFunction( "uuid", 0, fcnUuid, "Record", QString(), false, QStringList(), false, QStringList() << "$uuid" )
    << new StaticFunction( "get_feature", 3, fcnGetFeature, "Record", QString(), false, QStringList(), false, QStringList() << "getFeature" )
    << new StaticFunction( "layer_property", 2, fcnGetLayerProperty, "General" )
    << new StaticFunction( "var", 1, fcnGetVariable, "General" )

    //return all attributes string for referencedColumns - this is caught by
    // QgsFeatureRequest::setSubsetOfAttributes and causes all attributes to be fetched by the
    // feature request
    << new StaticFunction( "eval", 1, fcnEval, "General", QString(), true, QStringList( QgsFeatureRequest::AllAttributes ) )
    << new StaticFunction( "attribute", 2, fcnAttribute, "Record", QString(), false, QStringList( QgsFeatureRequest::AllAttributes ) )

    << new StaticFunction( "_specialcol_", 1, fcnSpecialColumn, "Special" )
    ;

    QgsExpressionContextUtils::registerContextFunctions();

    //QgsExpression has ownership of all built-in functions
    Q_FOREACH ( QgsExpression::Function* func, gmFunctions )
    {
      gmOwnedFunctions << func;
    }
  }
  return gmFunctions;
}

QMap<QString, QVariant> QgsExpression::gmSpecialColumns;
QMap<QString, QString> QgsExpression::gmSpecialColumnGroups;

void QgsExpression::setSpecialColumn( const QString& name, const QVariant& variant )
{
  int fnIdx = functionIndex( name );
  if ( fnIdx != -1 )
  {
    // function of the same name already exists
    return;
  }
  gmSpecialColumns[ name ] = variant;
}

void QgsExpression::unsetSpecialColumn( const QString& name )
{
  QMap<QString, QVariant>::iterator fit = gmSpecialColumns.find( name );
  if ( fit != gmSpecialColumns.end() )
  {
    gmSpecialColumns.erase( fit );
  }
}

QVariant QgsExpression::specialColumn( const QString& name )
{
  int fnIdx = functionIndex( name );
  if ( fnIdx != -1 )
  {
    // function of the same name already exists
    return QVariant();
  }
  QMap<QString, QVariant>::const_iterator it = gmSpecialColumns.constFind( name );
  if ( it == gmSpecialColumns.constEnd() )
  {
    return QVariant();
  }
  return it.value();
}

bool QgsExpression::hasSpecialColumn( const QString& name )
{
  if ( functionIndex( name ) != -1 )
    return false;
  return gmSpecialColumns.contains( name );
}

bool QgsExpression::isValid( const QString &text, const QgsFields &fields, QString &errorMessage )
{
  QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( 0, fields );
  return isValid( text, &context, errorMessage );
}

bool QgsExpression::isValid( const QString &text, const QgsExpressionContext *context, QString &errorMessage )
{
  QgsExpression exp( text );
  exp.prepare( context );
  errorMessage = exp.parserErrorString();
  return !exp.hasParserError();
}

void QgsExpression::setScale( double scale ) { d->mScale = scale; }

double QgsExpression::scale() { return d->mScale; }

QString QgsExpression::expression() const
{
  if ( !d->mExp.isNull() )
    return d->mExp;
  else
    return dump();
}

QList<QgsExpression::Function*> QgsExpression::specialColumns()
{
  QList<Function*> defs;
  for ( QMap<QString, QVariant>::const_iterator it = gmSpecialColumns.constBegin(); it != gmSpecialColumns.constEnd(); ++it )
  {
    //check for special column group name
    QString group = gmSpecialColumnGroups.value( it.key(), "Record" );
    defs << new StaticFunction( it.key(), 0, static_cast< FcnEvalContext >( nullptr ), group );
  }
  return defs;
}

QString QgsExpression::quotedColumnRef( QString name )
{
  return QString( "\"%1\"" ).arg( name.replace( '\"', "\"\"" ) );
}

QString QgsExpression::quotedString( QString text )
{
  text.replace( '\'', "''" );
  text.replace( '\\', "\\\\" );
  text.replace( '\n', "\\n" );
  text.replace( '\t', "\\t" );
  return QString( "'%1'" ).arg( text );
}

QString QgsExpression::quotedValue( const QVariant &value )
{
  return quotedValue( value, value.type() );
}

QString QgsExpression::quotedValue( const QVariant& value, QVariant::Type type )
{
  if ( value.isNull() )
    return "NULL";

  switch ( type )
  {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::Double:
      return value.toString();

    case QVariant::Bool:
      return value.toBool() ? "TRUE" : "FALSE";

    default:
    case QVariant::String:
      return quotedString( value.toString() );
  }

}

bool QgsExpression::isFunctionName( const QString &name )
{
  return functionIndex( name ) != -1;
}

int QgsExpression::functionIndex( const QString &name )
{
  int count = functionCount();
  for ( int i = 0; i < count; i++ )
  {
    if ( QString::compare( name, Functions()[i]->name(), Qt::CaseInsensitive ) == 0 )
      return i;
    Q_FOREACH ( const QString& alias, Functions()[i]->aliases() )
    {
      if ( QString::compare( name, alias, Qt::CaseInsensitive ) == 0 )
        return i;
    }
  }
  return -1;
}

int QgsExpression::functionCount()
{
  return Functions().size();
}


QgsExpression::QgsExpression( const QString& expr )
    : d( new QgsExpressionPrivate )
{
  d->mRootNode = ::parseExpression( expr, d->mParserErrorString );
  d->mExp = expr;
  Q_ASSERT( !d->mParserErrorString.isNull() || d->mRootNode );
}

QgsExpression::QgsExpression( const QgsExpression& other )
    : d( other.d )
{
  d->ref.ref();
}

QgsExpression& QgsExpression::operator=( const QgsExpression & other )
{
  d = other.d;
  d->ref.ref();
  return *this;
}

QgsExpression::QgsExpression()
    : d( new QgsExpressionPrivate )
{
}

QgsExpression::~QgsExpression()
{
  Q_ASSERT( d );
  if ( !d->ref.deref() )
    delete d;
}

bool QgsExpression::hasParserError() const { return !d->mParserErrorString.isNull(); }

QString QgsExpression::parserErrorString() const { return d->mParserErrorString; }

QStringList QgsExpression::referencedColumns() const
{
  if ( !d->mRootNode )
    return QStringList();

  QStringList columns = d->mRootNode->referencedColumns();

  // filter out duplicates
  for ( int i = 0; i < columns.count(); i++ )
  {
    QString col = columns.at( i );
    for ( int j = i + 1; j < columns.count(); j++ )
    {
      if ( QString::compare( col, columns[j], Qt::CaseInsensitive ) == 0 )
      {
        // this column is repeated: remove it!
        columns.removeAt( j-- );
      }
    }
  }

  return columns;
}

bool QgsExpression::needsGeometry() const
{
  if ( !d->mRootNode )
    return false;
  return d->mRootNode->needsGeometry();
}

void QgsExpression::initGeomCalculator()
{
  if ( d->mCalc.data() )
    return;

  // Use planimetric as default
  d->mCalc = QSharedPointer<QgsDistanceArea>( new QgsDistanceArea() );
  d->mCalc->setEllipsoidalMode( false );
}

void QgsExpression::detach()
{
  Q_ASSERT( d );

  if ( d->ref > 1 )
  {
    ( void )d->ref.deref();

    d = new QgsExpressionPrivate( *d );
  }
}

void QgsExpression::setGeomCalculator( const QgsDistanceArea &calc )
{
  d->mCalc = QSharedPointer<QgsDistanceArea>( new QgsDistanceArea( calc ) );
}

bool QgsExpression::prepare( const QgsFields& fields )
{
  detach();
  QgsExpressionContext fc = QgsExpressionContextUtils::createFeatureBasedContext( 0, fields );
  return prepare( &fc );
}

bool QgsExpression::prepare( const QgsExpressionContext *context )
{
  detach();
  d->mEvalErrorString = QString();
  if ( !d->mRootNode )
  {
    //re-parse expression. Creation of QgsExpressionContexts may have added extra
    //known functions since this expression was created, so we have another try
    //at re-parsing it now that the context must have been created
    d->mRootNode = ::parseExpression( d->mExp, d->mParserErrorString );
  }

  if ( !d->mRootNode )
  {
    d->mEvalErrorString = tr( "No root node! Parsing failed?" );
    return false;
  }

  return d->mRootNode->prepare( this, context );
}

QVariant QgsExpression::evaluate( const QgsFeature* f )
{
  d->mEvalErrorString = QString();
  if ( !d->mRootNode )
  {
    d->mEvalErrorString = tr( "No root node! Parsing failed?" );
    return QVariant();
  }

  QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f ? *f : QgsFeature(), QgsFields() );
  return d->mRootNode->eval( this, &context );
}

QVariant QgsExpression::evaluate( const QgsFeature &f )
{
  Q_NOWARN_DEPRECATED_PUSH
  return evaluate( &f );
  Q_NOWARN_DEPRECATED_POP
}

QVariant QgsExpression::evaluate( const QgsFeature* f, const QgsFields& fields )
{
  // first prepare
  QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( f ? *f : QgsFeature() , fields );
  bool res = prepare( &context );
  if ( !res )
    return QVariant();

  // then evaluate
  return evaluate( &context );
}

inline QVariant QgsExpression::evaluate( const QgsFeature& f, const QgsFields& fields )
{
  Q_NOWARN_DEPRECATED_PUSH
  return evaluate( &f, fields );
  Q_NOWARN_DEPRECATED_POP
}

QVariant QgsExpression::evaluate()
{
  d->mEvalErrorString = QString();
  if ( !d->mRootNode )
  {
    d->mEvalErrorString = tr( "No root node! Parsing failed?" );
    return QVariant();
  }

  return d->mRootNode->eval( this, static_cast<const QgsExpressionContext*>( nullptr ) );
}

QVariant QgsExpression::evaluate( const QgsExpressionContext *context )
{
  d->mEvalErrorString = QString();
  if ( !d->mRootNode )
  {
    d->mEvalErrorString = tr( "No root node! Parsing failed?" );
    return QVariant();
  }

  return d->mRootNode->eval( this, context );
}

bool QgsExpression::hasEvalError() const
{
  return !d->mEvalErrorString.isNull();
}

QString QgsExpression::evalErrorString() const
{
  return d->mEvalErrorString;
}

void QgsExpression::setEvalErrorString( const QString& str )
{
  d->mEvalErrorString = str;
}

void QgsExpression::setCurrentRowNumber( int rowNumber )
{
  d->mRowNumber = rowNumber;
}

int QgsExpression::currentRowNumber() { return d->mRowNumber; }

QString QgsExpression::dump() const
{
  if ( !d->mRootNode )
    return tr( "(no root)" );

  return d->mRootNode->dump();
}

QgsDistanceArea* QgsExpression::geomCalculator()
{
  return d->mCalc.data();
}

QGis::UnitType QgsExpression::distanceUnits() const
{
  return d->mDistanceUnit;
}

void QgsExpression::setDistanceUnits( QGis::UnitType unit )
{
  d->mDistanceUnit = unit;
}

QgsUnitTypes::AreaUnit QgsExpression::areaUnits() const
{
  return d->mAreaUnit;
}

void QgsExpression::setAreaUnits( QgsUnitTypes::AreaUnit unit )
{
  d->mAreaUnit = unit;
}

void QgsExpression::acceptVisitor( QgsExpression::Visitor& v ) const
{
  if ( d->mRootNode )
    d->mRootNode->accept( v );
}

QString QgsExpression::replaceExpressionText( const QString &action, const QgsFeature *feat,
    QgsVectorLayer *layer,
    const QMap<QString, QVariant> *substitutionMap, const QgsDistanceArea *distanceArea )
{
  QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( feat ? *feat : QgsFeature(), layer ? layer->fields() : QgsFields() );
  return replaceExpressionText( action, &context, substitutionMap, distanceArea );
}

QString QgsExpression::replaceExpressionText( const QString &action, const QgsExpressionContext *context, const QMap<QString, QVariant> *substitutionMap, const QgsDistanceArea *distanceArea )
{
  QString expr_action;

  QMap<QString, QVariant> savedValues;
  if ( substitutionMap )
  {
    // variables with a local scope (must be restored after evaluation)
    for ( QMap<QString, QVariant>::const_iterator sit = substitutionMap->begin(); sit != substitutionMap->end(); ++sit )
    {
      Q_NOWARN_DEPRECATED_PUSH
      QVariant oldValue = QgsExpression::specialColumn( sit.key() );
      if ( !oldValue.isNull() )
        savedValues.insert( sit.key(), oldValue );

      // set the new value
      QgsExpression::setSpecialColumn( sit.key(), sit.value() );
      Q_NOWARN_DEPRECATED_POP
    }
  }

  int index = 0;
  while ( index < action.size() )
  {
    QRegExp rx = QRegExp( "\\[%([^\\]]+)%\\]" );

    int pos = rx.indexIn( action, index );
    if ( pos < 0 )
      break;

    int start = index;
    index = pos + rx.matchedLength();
    QString to_replace = rx.cap( 1 ).trimmed();
    QgsDebugMsg( "Found expression: " + to_replace );

    QgsExpression exp( to_replace );
    if ( exp.hasParserError() )
    {
      QgsDebugMsg( "Expression parser error: " + exp.parserErrorString() );
      expr_action += action.midRef( start, index - start );
      continue;
    }

    if ( distanceArea )
    {
      //if QgsDistanceArea specified for area/distance conversion, use it
      exp.setGeomCalculator( *distanceArea );
    }

    QVariant result = exp.evaluate( context );

    if ( exp.hasEvalError() )
    {
      QgsDebugMsg( "Expression parser eval error: " + exp.evalErrorString() );
      expr_action += action.midRef( start, index - start );
      continue;
    }

    QgsDebugMsg( "Expression result is: " + result.toString() );
    expr_action += action.mid( start, pos - start ) + result.toString();
  }

  expr_action += action.midRef( index );

  // restore overwritten local values
  Q_NOWARN_DEPRECATED_PUSH
  for ( QMap<QString, QVariant>::const_iterator sit = savedValues.begin(); sit != savedValues.end(); ++sit )
  {
    QgsExpression::setSpecialColumn( sit.key(), sit.value() );
  }
  Q_NOWARN_DEPRECATED_POP

  return expr_action;
}

double QgsExpression::evaluateToDouble( const QString &text, const double fallbackValue )
{
  bool ok;
  //first test if text is directly convertible to double
  double convertedValue = text.toDouble( &ok );
  if ( ok )
  {
    return convertedValue;
  }

  //otherwise try to evalute as expression
  QgsExpression expr( text );

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope();

  QVariant result = expr.evaluate( &context );
  convertedValue = result.toDouble( &ok );
  if ( expr.hasEvalError() || !ok )
  {
    return fallbackValue;
  }
  return convertedValue;
}


///////////////////////////////////////////////
// nodes

QgsExpression::NodeList* QgsExpression::NodeList::clone() const
{
  NodeList* nl = new NodeList;
  Q_FOREACH ( Node* node, mList )
  {
    nl->mList.append( node->clone() );
  }
  nl->mNameList = mNameList;

  return nl;
}

QString QgsExpression::NodeList::dump() const
{
  QString msg;
  bool first = true;
  Q_FOREACH ( Node* n, mList )
  {
    if ( !first ) msg += ", ";
    else first = false;
    msg += n->dump();
  }
  return msg;
}


//

QVariant QgsExpression::NodeUnaryOperator::eval( QgsExpression *parent, const QgsExpressionContext *context )
{
  QVariant val = mOperand->eval( parent, context );
  ENSURE_NO_EVAL_ERROR;

  switch ( mOp )
  {
    case uoNot:
    {
      TVL tvl = getTVLValue( val, parent );
      ENSURE_NO_EVAL_ERROR;
      return tvl2variant( NOT[tvl] );
    }

    case uoMinus:
      if ( isIntSafe( val ) )
        return QVariant( - getIntValue( val, parent ) );
      else if ( isDoubleSafe( val ) )
        return QVariant( - getDoubleValue( val, parent ) );
      else
        SET_EVAL_ERROR( tr( "Unary minus only for numeric values." ) );
    default:
      Q_ASSERT( 0 && "unknown unary operation" );
  }
  return QVariant();
}

bool QgsExpression::NodeUnaryOperator::prepare( QgsExpression *parent, const QgsExpressionContext *context )
{
  return mOperand->prepare( parent, context );
}

QString QgsExpression::NodeUnaryOperator::dump() const
{
  return QString( "%1 %2" ).arg( UnaryOperatorText[mOp], mOperand->dump() );
}

QgsExpression::Node*QgsExpression::NodeUnaryOperator::clone() const
{
  return new NodeUnaryOperator( mOp, mOperand->clone() );
}

//

QVariant QgsExpression::NodeBinaryOperator::eval( QgsExpression *parent, const QgsExpressionContext *context )
{
  QVariant vL = mOpLeft->eval( parent, context );
  ENSURE_NO_EVAL_ERROR;
  QVariant vR = mOpRight->eval( parent, context );
  ENSURE_NO_EVAL_ERROR;

  switch ( mOp )
  {
    case boPlus:
      if ( vL.type() == QVariant::String && vR.type() == QVariant::String )
      {
        QString sL = isNull( vL ) ? QString() : getStringValue( vL, parent );
        ENSURE_NO_EVAL_ERROR;
        QString sR = isNull( vR ) ? QString() : getStringValue( vR, parent );
        ENSURE_NO_EVAL_ERROR;
        return QVariant( sL + sR );
      }
      //intentional fall-through
      FALLTHROUGH;
    case boMinus:
    case boMul:
    case boDiv:
    case boMod:
    {
      if ( isNull( vL ) || isNull( vR ) )
        return QVariant();
      else if ( mOp != boDiv && isIntSafe( vL ) && isIntSafe( vR ) )
      {
        // both are integers - let's use integer arithmetics
        int iL = getIntValue( vL, parent );
        ENSURE_NO_EVAL_ERROR;
        int iR = getIntValue( vR, parent );
        ENSURE_NO_EVAL_ERROR;

        if ( mOp == boMod && iR == 0 )
          return QVariant();

        return QVariant( computeInt( iL, iR ) );
      }
      else if ( isDateTimeSafe( vL ) && isIntervalSafe( vR ) )
      {
        QDateTime dL = getDateTimeValue( vL, parent );
        ENSURE_NO_EVAL_ERROR;
        QgsInterval iL = getInterval( vR, parent );
        ENSURE_NO_EVAL_ERROR;
        if ( mOp == boDiv || mOp == boMul || mOp == boMod )
        {
          parent->setEvalErrorString( tr( "Can't preform /, *, or % on DateTime and Interval" ) );
          return QVariant();
        }
        return QVariant( computeDateTimeFromInterval( dL, &iL ) );
      }
      else if ( mOp == boPlus && (( vL.type() == QVariant::Date && vR.type() == QVariant::Time ) ||
                                  ( vR.type() == QVariant::Date && vL.type() == QVariant::Time ) ) )
      {
        QDate date = getDateValue( vL.type() == QVariant::Date ? vL : vR, parent );
        ENSURE_NO_EVAL_ERROR;
        QTime time = getTimeValue( vR.type() == QVariant::Time ? vR : vL, parent );
        ENSURE_NO_EVAL_ERROR;
        QDateTime dt = QDateTime( date, time );
        return QVariant( dt );
      }
      else if ( mOp == boMinus && vL.type() == QVariant::Date && vR.type() == QVariant::Date )
      {
        QDate date1 = getDateValue( vL, parent );
        ENSURE_NO_EVAL_ERROR;
        QDate date2 = getDateValue( vR, parent );
        ENSURE_NO_EVAL_ERROR;
        return date1 - date2;
      }
      else if ( mOp == boMinus && vL.type() == QVariant::Time && vR.type() == QVariant::Time )
      {
        QTime time1 = getTimeValue( vL, parent );
        ENSURE_NO_EVAL_ERROR;
        QTime time2 = getTimeValue( vR, parent );
        ENSURE_NO_EVAL_ERROR;
        return time1 - time2;
      }
      else if ( mOp == boMinus && vL.type() == QVariant::DateTime && vR.type() == QVariant::DateTime )
      {
        QDateTime datetime1 = getDateTimeValue( vL, parent );
        ENSURE_NO_EVAL_ERROR;
        QDateTime datetime2 = getDateTimeValue( vR, parent );
        ENSURE_NO_EVAL_ERROR;
        return datetime1 - datetime2;
      }
      else
      {
        // general floating point arithmetic
        double fL = getDoubleValue( vL, parent );
        ENSURE_NO_EVAL_ERROR;
        double fR = getDoubleValue( vR, parent );
        ENSURE_NO_EVAL_ERROR;
        if (( mOp == boDiv || mOp == boMod ) && fR == 0. )
          return QVariant(); // silently handle division by zero and return NULL
        return QVariant( computeDouble( fL, fR ) );
      }
    }
    case boIntDiv:
    {
      //integer division
      double fL = getDoubleValue( vL, parent );
      ENSURE_NO_EVAL_ERROR;
      double fR = getDoubleValue( vR, parent );
      ENSURE_NO_EVAL_ERROR;
      if ( fR == 0. )
        return QVariant(); // silently handle division by zero and return NULL
      return QVariant( qFloor( fL / fR ) );
    }
    case boPow:
      if ( isNull( vL ) || isNull( vR ) )
        return QVariant();
      else
      {
        double fL = getDoubleValue( vL, parent );
        ENSURE_NO_EVAL_ERROR;
        double fR = getDoubleValue( vR, parent );
        ENSURE_NO_EVAL_ERROR;
        return QVariant( pow( fL, fR ) );
      }

    case boAnd:
    {
      TVL tvlL = getTVLValue( vL, parent ), tvlR = getTVLValue( vR, parent );
      ENSURE_NO_EVAL_ERROR;
      return tvl2variant( AND[tvlL][tvlR] );
    }

    case boOr:
    {
      TVL tvlL = getTVLValue( vL, parent ), tvlR = getTVLValue( vR, parent );
      ENSURE_NO_EVAL_ERROR;
      return tvl2variant( OR[tvlL][tvlR] );
    }

    case boEQ:
    case boNE:
    case boLT:
    case boGT:
    case boLE:
    case boGE:
      if ( isNull( vL ) || isNull( vR ) )
      {
        return TVL_Unknown;
      }
      else if ( isDoubleSafe( vL ) && isDoubleSafe( vR ) &&
                ( vL.type() != QVariant::String || vR.type() != QVariant::String ) )
      {
        // do numeric comparison if both operators can be converted to numbers,
        // and they aren't both string
        double fL = getDoubleValue( vL, parent );
        ENSURE_NO_EVAL_ERROR;
        double fR = getDoubleValue( vR, parent );
        ENSURE_NO_EVAL_ERROR;
        return compare( fL - fR ) ? TVL_True : TVL_False;
      }
      else
      {
        // do string comparison otherwise
        QString sL = getStringValue( vL, parent );
        ENSURE_NO_EVAL_ERROR;
        QString sR = getStringValue( vR, parent );
        ENSURE_NO_EVAL_ERROR;
        int diff = QString::compare( sL, sR );
        return compare( diff ) ? TVL_True : TVL_False;
      }

    case boIs:
    case boIsNot:
      if ( isNull( vL ) && isNull( vR ) ) // both operators null
        return ( mOp == boIs ? TVL_True : TVL_False );
      else if ( isNull( vL ) || isNull( vR ) ) // one operator null
        return ( mOp == boIs ? TVL_False : TVL_True );
      else // both operators non-null
      {
        bool equal = false;
        if ( isDoubleSafe( vL ) && isDoubleSafe( vR ) &&
             ( vL.type() != QVariant::String || vR.type() != QVariant::String ) )
        {
          double fL = getDoubleValue( vL, parent );
          ENSURE_NO_EVAL_ERROR;
          double fR = getDoubleValue( vR, parent );
          ENSURE_NO_EVAL_ERROR;
          equal = qgsDoubleNear( fL, fR );
        }
        else
        {
          QString sL = getStringValue( vL, parent );
          ENSURE_NO_EVAL_ERROR;
          QString sR = getStringValue( vR, parent );
          ENSURE_NO_EVAL_ERROR;
          equal = QString::compare( sL, sR ) == 0;
        }
        if ( equal )
          return mOp == boIs ? TVL_True : TVL_False;
        else
          return mOp == boIs ? TVL_False : TVL_True;
      }

    case boRegexp:
    case boLike:
    case boNotLike:
    case boILike:
    case boNotILike:
      if ( isNull( vL ) || isNull( vR ) )
        return TVL_Unknown;
      else
      {
        QString str    = getStringValue( vL, parent );
        ENSURE_NO_EVAL_ERROR;
        QString regexp = getStringValue( vR, parent );
        ENSURE_NO_EVAL_ERROR;
        // TODO: cache QRegExp in case that regexp is a literal string (i.e. it will stay constant)
        bool matches;
        if ( mOp == boLike || mOp == boILike || mOp == boNotLike || mOp == boNotILike ) // change from LIKE syntax to regexp
        {
          QString esc_regexp = QRegExp::escape( regexp );
          // XXX escape % and _  ???
          esc_regexp.replace( '%', ".*" );
          esc_regexp.replace( '_', '.' );
          matches = QRegExp( esc_regexp, mOp == boLike || mOp == boNotLike ? Qt::CaseSensitive : Qt::CaseInsensitive ).exactMatch( str );
        }
        else
        {
          matches = QRegExp( regexp ).indexIn( str ) != -1;
        }

        if ( mOp == boNotLike || mOp == boNotILike )
        {
          matches = !matches;
        }

        return matches ? TVL_True : TVL_False;
      }

    case boConcat:
      if ( isNull( vL ) || isNull( vR ) )
        return QVariant();
      else
      {
        QString sL = getStringValue( vL, parent );
        ENSURE_NO_EVAL_ERROR;
        QString sR = getStringValue( vR, parent );
        ENSURE_NO_EVAL_ERROR;
        return QVariant( sL + sR );
      }

    default:
      break;
  }
  Q_ASSERT( false );
  return QVariant();
}

bool QgsExpression::NodeBinaryOperator::compare( double diff )
{
  switch ( mOp )
  {
    case boEQ:
      return qgsDoubleNear( diff, 0.0 );
    case boNE:
      return !qgsDoubleNear( diff, 0.0 );
    case boLT:
      return diff < 0;
    case boGT:
      return diff > 0;
    case boLE:
      return diff <= 0;
    case boGE:
      return diff >= 0;
    default:
      Q_ASSERT( false );
      return false;
  }
}

int QgsExpression::NodeBinaryOperator::computeInt( int x, int y )
{
  switch ( mOp )
  {
    case boPlus:
      return x + y;
    case boMinus:
      return x -y;
    case boMul:
      return x*y;
    case boDiv:
      return x / y;
    case boMod:
      return x % y;
    default:
      Q_ASSERT( false );
      return 0;
  }
}

QDateTime QgsExpression::NodeBinaryOperator::computeDateTimeFromInterval( const QDateTime& d, QgsInterval *i )
{
  switch ( mOp )
  {
    case boPlus:
      return d.addSecs( i->seconds() );
    case boMinus:
      return d.addSecs( -i->seconds() );
    default:
      Q_ASSERT( false );
      return QDateTime();
  }
}

double QgsExpression::NodeBinaryOperator::computeDouble( double x, double y )
{
  switch ( mOp )
  {
    case boPlus:
      return x + y;
    case boMinus:
      return x -y;
    case boMul:
      return x*y;
    case boDiv:
      return x / y;
    case boMod:
      return fmod( x, y );
    default:
      Q_ASSERT( false );
      return 0;
  }
}

bool QgsExpression::NodeBinaryOperator::prepare( QgsExpression *parent, const QgsExpressionContext *context )
{
  bool resL = mOpLeft->prepare( parent, context );
  bool resR = mOpRight->prepare( parent, context );
  return resL && resR;
}

int QgsExpression::NodeBinaryOperator::precedence() const
{
  // see left/right in qgsexpressionparser.yy
  switch ( mOp )
  {
    case boOr:
      return 1;

    case boAnd:
      return 2;

    case boEQ:
    case boNE:
    case boLE:
    case boGE:
    case boLT:
    case boGT:
    case boRegexp:
    case boLike:
    case boILike:
    case boNotLike:
    case boNotILike:
    case boIs:
    case boIsNot:
      return 3;

    case boPlus:
    case boMinus:
      return 4;

    case boMul:
    case boDiv:
    case boIntDiv:
    case boMod:
      return 5;

    case boPow:
      return 6;

    case boConcat:
      return 7;
  }
  Q_ASSERT( 0 && "unexpected binary operator" );
  return -1;
}

bool QgsExpression::NodeBinaryOperator::leftAssociative() const
{
  // see left/right in qgsexpressionparser.yy
  switch ( mOp )
  {
    case boOr:
    case boAnd:
    case boEQ:
    case boNE:
    case boLE:
    case boGE:
    case boLT:
    case boGT:
    case boRegexp:
    case boLike:
    case boILike:
    case boNotLike:
    case boNotILike:
    case boIs:
    case boIsNot:
    case boPlus:
    case boMinus:
    case boMul:
    case boDiv:
    case boIntDiv:
    case boMod:
    case boConcat:
      return true;

    case boPow:
      return false;
  }
  Q_ASSERT( 0 && "unexpected binary operator" );
  return false;
}

QString QgsExpression::NodeBinaryOperator::dump() const
{
  QgsExpression::NodeBinaryOperator *lOp = dynamic_cast<QgsExpression::NodeBinaryOperator *>( mOpLeft );
  QgsExpression::NodeBinaryOperator *rOp = dynamic_cast<QgsExpression::NodeBinaryOperator *>( mOpRight );
  QgsExpression::NodeUnaryOperator *ruOp = dynamic_cast<QgsExpression::NodeUnaryOperator *>( mOpRight );

  QString rdump( mOpRight->dump() );

  // avoid dumping "IS (NOT ...)" as "IS NOT ..."
  if ( mOp == boIs && ruOp && ruOp->op() == uoNot )
  {
    rdump.prepend( '(' ).append( ')' );
  }

  QString fmt;
  if ( leftAssociative() )
  {
    fmt += lOp && ( lOp->precedence() < precedence() ) ? "(%1)" : "%1";
    fmt += " %2 ";
    fmt += rOp && ( rOp->precedence() <= precedence() ) ? "(%3)" : "%3";
  }
  else
  {
    fmt += lOp && ( lOp->precedence() <= precedence() ) ? "(%1)" : "%1";
    fmt += " %2 ";
    fmt += rOp && ( rOp->precedence() < precedence() ) ? "(%3)" : "%3";
  }

  return fmt.arg( mOpLeft->dump(), BinaryOperatorText[mOp], rdump );
}

QgsExpression::Node* QgsExpression::NodeBinaryOperator::clone() const
{
  return new NodeBinaryOperator( mOp, mOpLeft->clone(), mOpRight->clone() );
}

//

QVariant QgsExpression::NodeInOperator::eval( QgsExpression *parent, const QgsExpressionContext *context )
{
  if ( mList->count() == 0 )
    return mNotIn ? TVL_True : TVL_False;
  QVariant v1 = mNode->eval( parent, context );
  ENSURE_NO_EVAL_ERROR;
  if ( isNull( v1 ) )
    return TVL_Unknown;

  bool listHasNull = false;

  Q_FOREACH ( Node* n, mList->list() )
  {
    QVariant v2 = n->eval( parent, context );
    ENSURE_NO_EVAL_ERROR;
    if ( isNull( v2 ) )
      listHasNull = true;
    else
    {
      bool equal = false;
      // check whether they are equal
      if ( isDoubleSafe( v1 ) && isDoubleSafe( v2 ) )
      {
        double f1 = getDoubleValue( v1, parent );
        ENSURE_NO_EVAL_ERROR;
        double f2 = getDoubleValue( v2, parent );
        ENSURE_NO_EVAL_ERROR;
        equal = qgsDoubleNear( f1, f2 );
      }
      else
      {
        QString s1 = getStringValue( v1, parent );
        ENSURE_NO_EVAL_ERROR;
        QString s2 = getStringValue( v2, parent );
        ENSURE_NO_EVAL_ERROR;
        equal = QString::compare( s1, s2 ) == 0;
      }

      if ( equal ) // we know the result
        return mNotIn ? TVL_False : TVL_True;
    }
  }

  // item not found
  if ( listHasNull )
    return TVL_Unknown;
  else
    return mNotIn ? TVL_True : TVL_False;
}

bool QgsExpression::NodeInOperator::prepare( QgsExpression *parent, const QgsExpressionContext *context )
{
  bool res = mNode->prepare( parent, context );
  Q_FOREACH ( Node* n, mList->list() )
  {
    res = res && n->prepare( parent, context );
  }
  return res;
}

QString QgsExpression::NodeInOperator::dump() const
{
  return QString( "%1 %2 IN (%3)" ).arg( mNode->dump(), mNotIn ? "NOT" : "", mList->dump() );
}

QgsExpression::Node*QgsExpression::NodeInOperator::clone() const
{
  return new NodeInOperator( mNode->clone(), mList->clone(), mNotIn );
}

//

QVariant QgsExpression::NodeFunction::eval( QgsExpression *parent, const QgsExpressionContext *context )
{
  QString name = Functions()[mFnIndex]->name();
  Function* fd = context && context->hasFunction( name ) ? context->function( name ) : Functions()[mFnIndex];

  // evaluate arguments
  QVariantList argValues;
  if ( mArgs )
  {
    Q_FOREACH ( Node* n, mArgs->list() )
    {
      QVariant v;
      if ( fd->lazyEval() )
      {
        // Pass in the node for the function to eval as it needs.
        v = QVariant::fromValue( n );
      }
      else
      {
        v = n->eval( parent, context );
        ENSURE_NO_EVAL_ERROR;
        if ( isNull( v ) && !fd->handlesNull() )
          return QVariant(); // all "normal" functions return NULL, when any parameter is NULL (so coalesce is abnormal)
      }
      argValues.append( v );
    }
  }

  // run the function
  QVariant res = fd->func( argValues, context, parent );
  ENSURE_NO_EVAL_ERROR;

  // everything went fine
  return res;
}

bool QgsExpression::NodeFunction::prepare( QgsExpression *parent, const QgsExpressionContext *context )
{
  Function* fd = Functions()[mFnIndex];

  bool res = true;
  if ( mArgs && !fd->lazyEval() )
  {
    Q_FOREACH ( Node* n, mArgs->list() )
    {
      res = res && n->prepare( parent, context );
    }
  }
  return res;
}

QString QgsExpression::NodeFunction::dump() const
{
  Function* fd = Functions()[mFnIndex];
  if ( fd->params() == 0 )
    return QString( "%1%2" ).arg( fd->name(), fd->name().startsWith( '$' ) ? "" : "()" ); // special column
  else
    return QString( "%1(%2)" ).arg( fd->name(), mArgs ? mArgs->dump() : QString() ); // function
}

QStringList QgsExpression::NodeFunction::referencedColumns() const
{
  Function* fd = Functions()[mFnIndex];
  QStringList functionColumns = fd->referencedColumns();

  if ( !mArgs )
  {
    //no referenced columns in arguments, just return function's referenced columns
    return functionColumns;
  }

  Q_FOREACH ( Node* n, mArgs->list() )
  {
    functionColumns.append( n->referencedColumns() );
  }

  //remove duplicates and return
  return functionColumns.toSet().toList();
}

QgsExpression::Node* QgsExpression::NodeFunction::clone() const
{
  return new NodeFunction( mFnIndex, mArgs ? mArgs->clone() : nullptr );
}

//

QVariant QgsExpression::NodeLiteral::eval( QgsExpression *parent, const QgsExpressionContext *context )
{
  Q_UNUSED( context );
  Q_UNUSED( parent );
  return mValue;
}

bool QgsExpression::NodeLiteral::prepare( QgsExpression *parent, const QgsExpressionContext *context )
{
  Q_UNUSED( parent );
  Q_UNUSED( context );
  return true;
}


QString QgsExpression::NodeLiteral::dump() const
{
  if ( mValue.isNull() )
    return "NULL";

  switch ( mValue.type() )
  {
    case QVariant::Int:
      return QString::number( mValue.toInt() );
    case QVariant::Double:
      return QString::number( mValue.toDouble() );
    case QVariant::String:
      return quotedString( mValue.toString() );
    case QVariant::Bool:
      return mValue.toBool() ? "TRUE" : "FALSE";
    default:
      return tr( "[unsupported type;%1; value:%2]" ).arg( mValue.typeName(), mValue.toString() );
  }
}

QgsExpression::Node*QgsExpression::NodeLiteral::clone() const
{
  return new NodeLiteral( mValue );
}

//

QVariant QgsExpression::NodeColumnRef::eval( QgsExpression *parent, const QgsExpressionContext *context )
{
  Q_UNUSED( parent );
  int index = mIndex;

  if ( index < 0 )
  {
    // have not yet found field index - first check explicitly set fields collection
    if ( context && context->hasVariable( QgsExpressionContext::EXPR_FIELDS ) )
    {
      QgsFields fields = qvariant_cast<QgsFields>( context->variable( QgsExpressionContext::EXPR_FIELDS ) );
      index = fields.fieldNameIndex( mName );
    }
  }

  if ( context && context->hasVariable( QgsExpressionContext::EXPR_FEATURE ) )
  {
    QgsFeature feature = qvariant_cast<QgsFeature>( context->variable( QgsExpressionContext::EXPR_FEATURE ) );
    if ( index >= 0 )
      return feature.attribute( index );
    else
      return feature.attribute( mName );
  }
  return QVariant( '[' + mName + ']' );
}

bool QgsExpression::NodeColumnRef::prepare( QgsExpression *parent, const QgsExpressionContext *context )
{
  if ( !context || !context->hasVariable( QgsExpressionContext::EXPR_FIELDS ) )
    return false;

  QgsFields fields = qvariant_cast<QgsFields>( context->variable( QgsExpressionContext::EXPR_FIELDS ) );

  mIndex = fields.fieldNameIndex( mName );
  if ( mIndex >= 0 )
  {
    return true;
  }
  else
  {
    parent->d->mEvalErrorString = tr( "Column '%1' not found" ).arg( mName );
    mIndex = -1;
    return false;
  }
}

QString QgsExpression::NodeColumnRef::dump() const
{
  return QRegExp( "^[A-Za-z_\x80-\xff][A-Za-z0-9_\x80-\xff]*$" ).exactMatch( mName ) ? mName : quotedColumnRef( mName );
}

QgsExpression::Node*QgsExpression::NodeColumnRef::clone() const
{
  return new NodeColumnRef( mName );
}

//

QVariant QgsExpression::NodeCondition::eval( QgsExpression *parent, const QgsExpressionContext *context )
{
  Q_FOREACH ( WhenThen* cond, mConditions )
  {
    QVariant vWhen = cond->mWhenExp->eval( parent, context );
    TVL tvl = getTVLValue( vWhen, parent );
    ENSURE_NO_EVAL_ERROR;
    if ( tvl == True )
    {
      QVariant vRes = cond->mThenExp->eval( parent, context );
      ENSURE_NO_EVAL_ERROR;
      return vRes;
    }
  }

  if ( mElseExp )
  {
    QVariant vElse = mElseExp->eval( parent, context );
    ENSURE_NO_EVAL_ERROR;
    return vElse;
  }

  // return NULL if no condition is matching
  return QVariant();
}

bool QgsExpression::NodeCondition::prepare( QgsExpression *parent, const QgsExpressionContext *context )
{
  bool res;
  Q_FOREACH ( WhenThen* cond, mConditions )
  {
    res = cond->mWhenExp->prepare( parent, context )
          & cond->mThenExp->prepare( parent, context );
    if ( !res ) return false;
  }

  if ( mElseExp )
    return mElseExp->prepare( parent, context );

  return true;
}

QString QgsExpression::NodeCondition::dump() const
{
  QString msg( "CASE" );
  Q_FOREACH ( WhenThen* cond, mConditions )
  {
    msg += QString( " WHEN %1 THEN %2" ).arg( cond->mWhenExp->dump(), cond->mThenExp->dump() );
  }
  if ( mElseExp )
    msg += QString( " ELSE %1" ).arg( mElseExp->dump() );
  msg += QString( " END" );
  return msg;
}

QStringList QgsExpression::NodeCondition::referencedColumns() const
{
  QStringList lst;
  Q_FOREACH ( WhenThen* cond, mConditions )
  {
    lst += cond->mWhenExp->referencedColumns() + cond->mThenExp->referencedColumns();
  }

  if ( mElseExp )
    lst += mElseExp->referencedColumns();

  return lst;
}

bool QgsExpression::NodeCondition::needsGeometry() const
{
  Q_FOREACH ( WhenThen* cond, mConditions )
  {
    if ( cond->mWhenExp->needsGeometry() ||
         cond->mThenExp->needsGeometry() )
      return true;
  }

  if ( mElseExp && mElseExp->needsGeometry() )
    return true;

  return false;
}

QgsExpression::Node* QgsExpression::NodeCondition::clone() const
{
  WhenThenList conditions;
  Q_FOREACH ( WhenThen* wt, mConditions )
    conditions.append( new WhenThen( wt->mWhenExp->clone(), wt->mThenExp->clone() ) );
  return new NodeCondition( conditions, mElseExp ? mElseExp->clone() : nullptr );
}


QString QgsExpression::helptext( QString name )
{
  QgsExpression::initFunctionHelp();

  if ( !gFunctionHelpTexts.contains( name ) )
    return tr( "function help for %1 missing" ).arg( name );

  const Help &f = gFunctionHelpTexts[ name ];

  name = f.mName;
  if ( f.mType == tr( "group" ) )
    name = group( name );

#if QT_VERSION < 0x050000
  name = Qt::escape( name );
#else
  name = name.toHtmlEscaped();
#endif

  QString helpContents( QString( "<h3>%1</h3>\n<div class=\"description\"><p>%2</p></div>" )
                        .arg( tr( "%1 %2" ).arg( f.mType, name ),
                              f.mDescription ) );

  Q_FOREACH ( const HelpVariant &v, f.mVariants )
  {
    if ( f.mVariants.size() > 1 )
    {
      helpContents += QString( "<h3>%1</h3>\n<div class=\"description\">%2</p></div>" ).arg( v.mName, v.mDescription );
    }

    if ( f.mType != tr( "group" ) && f.mType != tr( "expression" ) )
      helpContents += QString( "<h4>%1</h4>\n<div class=\"syntax\">\n" ).arg( tr( "Syntax" ) );

    if ( f.mType == tr( "operator" ) )
    {
      if ( v.mArguments.size() == 1 )
      {
        helpContents += QString( "<code><span class=\"functionname\">%1</span> <span class=\"argument\">%2</span></code>" )
                        .arg( name, v.mArguments[0].mArg );
      }
      else if ( v.mArguments.size() == 2 )
      {
        helpContents += QString( "<code><span class=\"argument\">%1</span> <span class=\"functionname\">%2</span> <span class=\"argument\">%3</span></code>" )
                        .arg( v.mArguments[0].mArg, name, v.mArguments[1].mArg );
      }
    }
    else if ( f.mType != tr( "group" ) && f.mType != tr( "expression" ) )
    {
      helpContents += QString( "<code><span class=\"functionname\">%1</span>" ).arg( name );

      if ( f.mType == tr( "function" ) && ( f.mName[0] != '$' || !v.mArguments.isEmpty() || v.mVariableLenArguments ) )
      {
        helpContents += '(';

        QString delim;
        Q_FOREACH ( const HelpArg &a, v.mArguments )
        {
          helpContents += delim;
          delim = ", ";
          if ( !a.mDescOnly )
          {
            helpContents += QString( "<span class=\"argument %1\">%2%3</span>" ).arg( a.mOptional ? "optional" : "", a.mArg,
                            a.mDefaultVal.isEmpty() ? "" : '=' + a.mDefaultVal );
          }
        }

        if ( v.mVariableLenArguments )
        {
          helpContents += "...";
        }

        helpContents += ')';
      }

      helpContents += "</code>";
    }

    if ( !v.mArguments.isEmpty() )
    {
      helpContents += QString( "<h4>%1</h4>\n<div class=\"arguments\">\n<table>" ).arg( tr( "Arguments" ) );

      Q_FOREACH ( const HelpArg &a, v.mArguments )
      {
        if ( a.mSyntaxOnly )
          continue;

        helpContents += QString( "<tr><td class=\"argument\">%1</td><td>%2</td></tr>" ).arg( a.mArg, a.mDescription );
      }

      helpContents += "</table>\n</div>\n";
    }

    if ( !v.mExamples.isEmpty() )
    {
      helpContents += QString( "<h4>%1</h4>\n<div class=\"examples\">\n<ul>\n" ).arg( tr( "Examples" ) );

      Q_FOREACH ( const HelpExample &e, v.mExamples )
      {
        helpContents += "<li><code>" + e.mExpression + "</code> &rarr; <code>" + e.mReturns + "</code>";

        if ( !e.mNote.isEmpty() )
          helpContents += QString( " (%1)" ).arg( e.mNote );

        helpContents += "</li>\n";
      }

      helpContents += "</ul>\n</div>\n";
    }

    if ( !v.mNotes.isEmpty() )
    {
      helpContents += QString( "<h4>%1</h4>\n<div class=\"notes\"><p>%2</p></div>\n" ).arg( tr( "Notes" ), v.mNotes );
    }
  }

  return helpContents;
}

QHash<QString, QString> QgsExpression::gVariableHelpTexts;

void QgsExpression::initVariableHelp()
{
  if ( !gVariableHelpTexts.isEmpty() )
    return;

  //global variables
  gVariableHelpTexts.insert( "qgis_version", QCoreApplication::translate( "variable_help", "Current QGIS version string." ) );
  gVariableHelpTexts.insert( "qgis_version_no", QCoreApplication::translate( "variable_help", "Current QGIS version number." ) );
  gVariableHelpTexts.insert( "qgis_release_name", QCoreApplication::translate( "variable_help", "Current QGIS release name." ) );
  gVariableHelpTexts.insert( "qgis_os_name", QCoreApplication::translate( "variable_help", "Operating system name, eg 'windows', 'linux' or 'osx'." ) );
  gVariableHelpTexts.insert( "qgis_platform", QCoreApplication::translate( "variable_help", "QGIS platform, eg 'desktop' or 'server'." ) );
  gVariableHelpTexts.insert( "user_account_name", QCoreApplication::translate( "variable_help", "Current user's operating system account name." ) );
  gVariableHelpTexts.insert( "user_full_name", QCoreApplication::translate( "variable_help", "Current user's operating system user name (if available)." ) );

  //project variables
  gVariableHelpTexts.insert( "project_title", QCoreApplication::translate( "variable_help", "Title of current project." ) );
  gVariableHelpTexts.insert( "project_path", QCoreApplication::translate( "variable_help", "Full path (including file name) of current project." ) );
  gVariableHelpTexts.insert( "project_folder", QCoreApplication::translate( "variable_help", "Folder for current project." ) );
  gVariableHelpTexts.insert( "project_filename", QCoreApplication::translate( "variable_help", "Filename of current project." ) );

  //layer variables
  gVariableHelpTexts.insert( "layer_name", QCoreApplication::translate( "variable_help", "Name of current layer." ) );
  gVariableHelpTexts.insert( "layer_id", QCoreApplication::translate( "variable_help", "ID of current layer." ) );

  //composition variables
  gVariableHelpTexts.insert( "layout_numpages", QCoreApplication::translate( "variable_help", "Number of pages in composition." ) );
  gVariableHelpTexts.insert( "layout_pageheight", QCoreApplication::translate( "variable_help", "Composition page height in mm." ) );
  gVariableHelpTexts.insert( "layout_pagewidth", QCoreApplication::translate( "variable_help", "Composition page width in mm." ) );
  gVariableHelpTexts.insert( "layout_dpi", QCoreApplication::translate( "variable_help", "Composition resolution (DPI)." ) );

  //atlas variables
  gVariableHelpTexts.insert( "atlas_totalfeatures", QCoreApplication::translate( "variable_help", "Total number of features in atlas." ) );
  gVariableHelpTexts.insert( "atlas_featurenumber", QCoreApplication::translate( "variable_help", "Current atlas feature number." ) );
  gVariableHelpTexts.insert( "atlas_filename", QCoreApplication::translate( "variable_help", "Current atlas file name." ) );
  gVariableHelpTexts.insert( "atlas_pagename", QCoreApplication::translate( "variable_help", "Current atlas page name." ) );
  gVariableHelpTexts.insert( "atlas_feature", QCoreApplication::translate( "variable_help", "Current atlas feature (as feature object)." ) );
  gVariableHelpTexts.insert( "atlas_featureid", QCoreApplication::translate( "variable_help", "Current atlas feature ID." ) );
  gVariableHelpTexts.insert( "atlas_geometry", QCoreApplication::translate( "variable_help", "Current atlas feature geometry." ) );

  //composer item variables
  gVariableHelpTexts.insert( "item_id", QCoreApplication::translate( "variable_help", "Composer item user ID (not necessarily unique)." ) );
  gVariableHelpTexts.insert( "item_uuid", QCoreApplication::translate( "variable_help", "Composer item unique ID." ) );
  gVariableHelpTexts.insert( "item_left", QCoreApplication::translate( "variable_help", "Left position of composer item (in mm)." ) );
  gVariableHelpTexts.insert( "item_top", QCoreApplication::translate( "variable_help", "Top position of composer item (in mm)." ) );
  gVariableHelpTexts.insert( "item_width", QCoreApplication::translate( "variable_help", "Width of composer item (in mm)." ) );
  gVariableHelpTexts.insert( "item_height", QCoreApplication::translate( "variable_help", "Height of composer item (in mm)." ) );

  //map settings item variables
  gVariableHelpTexts.insert( "map_id", QCoreApplication::translate( "variable_help", "ID of current map destination. This will be 'canvas' for canvas renders, and the item ID for composer map renders." ) );
  gVariableHelpTexts.insert( "map_rotation", QCoreApplication::translate( "variable_help", "Current rotation of map." ) );
  gVariableHelpTexts.insert( "map_scale", QCoreApplication::translate( "variable_help", "Current scale of map." ) );
  gVariableHelpTexts.insert( "map_extent_center", QCoreApplication::translate( "variable_help", "Center of map." ) );
  gVariableHelpTexts.insert( "map_extent_width", QCoreApplication::translate( "variable_help", "Width of map." ) );
  gVariableHelpTexts.insert( "map_extent_height", QCoreApplication::translate( "variable_help", "Height of map." ) );

  gVariableHelpTexts.insert( "row_number", QCoreApplication::translate( "variable_help", "Stores the number of the current row." ) );
  gVariableHelpTexts.insert( "grid_number", QCoreApplication::translate( "variable_help", "Current grid annotation value." ) );
  gVariableHelpTexts.insert( "grid_axis", QCoreApplication::translate( "variable_help", "Current grid annotation axis (eg, 'x' for longitude, 'y' for latitude)." ) );

  //symbol variables
  gVariableHelpTexts.insert( "geometry_part_count", QCoreApplication::translate( "variable_help", "Number of parts in rendered feature's geometry." ) );
  gVariableHelpTexts.insert( "geometry_part_num", QCoreApplication::translate( "variable_help", "Current geometry part number for feature being rendered." ) );
  gVariableHelpTexts.insert( "geometry_point_count", QCoreApplication::translate( "variable_help", "Number of points in the rendered geometry's part. It is only meaningful for line geometries and for symbol layers that set this variable." ) );
  gVariableHelpTexts.insert( "geometry_point_num", QCoreApplication::translate( "variable_help", "Current point number in the rendered geometry's part. It is only meaningful for line geometries and for symbol layers that set this variable." ) );

  gVariableHelpTexts.insert( "symbol_color", QCoreApplication::translate( "symbol_color", "Color of symbol used to render the feature." ) );
  gVariableHelpTexts.insert( "symbol_angle", QCoreApplication::translate( "symbol_angle", "Angle of symbol used to render the feature (valid for marker symbols only)." ) );
}

QString QgsExpression::variableHelpText( const QString &variableName, bool showValue, const QVariant &value )
{
  QgsExpression::initVariableHelp();
  QString text = gVariableHelpTexts.contains( variableName ) ? QString( "<p>%1</p>" ).arg( gVariableHelpTexts.value( variableName ) ) : QString();
  if ( showValue )
  {
    QString valueString;
    if ( !value.isValid() )
    {
      valueString = QCoreApplication::translate( "variable_help", "not set" );
    }
    else
    {
      valueString = QString( "<b>%1</b>" ).arg( formatPreviewString( value ) );
    }
    text.append( QCoreApplication::translate( "variable_help", "<p>Current value: %1</p>" ).arg( valueString ) );
  }
  return text;
}

QHash<QString, QString> QgsExpression::gGroups;

QString QgsExpression::group( const QString& name )
{
  if ( gGroups.isEmpty() )
  {
    gGroups.insert( "General", tr( "General" ) );
    gGroups.insert( "Operators", tr( "Operators" ) );
    gGroups.insert( "Conditionals", tr( "Conditionals" ) );
    gGroups.insert( "Fields and Values", tr( "Fields and Values" ) );
    gGroups.insert( "Math", tr( "Math" ) );
    gGroups.insert( "Conversions", tr( "Conversions" ) );
    gGroups.insert( "Date and Time", tr( "Date and Time" ) );
    gGroups.insert( "String", tr( "String" ) );
    gGroups.insert( "Color", tr( "Color" ) );
    gGroups.insert( "GeometryGroup", tr( "Geometry" ) );
    gGroups.insert( "Record", tr( "Record" ) );
    gGroups.insert( "Variables", tr( "Variables" ) );
    gGroups.insert( "Fuzzy Matching", tr( "Fuzzy Matching" ) );
    gGroups.insert( "Recent (%1)", tr( "Recent (%1)" ) );
  }

  //return the translated name for this group. If group does not
  //have a translated name in the gGroups hash, return the name
  //unchanged
  return gGroups.value( name, name );
}

QString QgsExpression::formatPreviewString( const QVariant& value )
{
  if ( value.canConvert<QgsGeometry>() )
  {
    //result is a geometry
    QgsGeometry geom = value.value<QgsGeometry>();
    if ( geom.isEmpty() )
      return tr( "<i>&lt;empty geometry&gt;</i>" );
    else
      return tr( "<i>&lt;geometry: %1&gt;</i>" ).arg( QgsWKBTypes::displayString( geom.geometry()->wkbType() ) );
  }
  else if ( value.canConvert< QgsFeature >() )
  {
    //result is a feature
    QgsFeature feat = value.value<QgsFeature>();
    return tr( "<i>&lt;feature: %1&gt;</i>" ).arg( feat.id() );
  }
  else if ( value.canConvert< QgsInterval >() )
  {
    //result is a feature
    QgsInterval interval = value.value<QgsInterval>();
    return tr( "<i>&lt;interval: %1 days&gt;</i>" ).arg( interval.days() );
  }
  else if ( value.type() == QVariant::Date )
  {
    QDate dt = value.toDate();
    return tr( "<i>&lt;date: %1&gt;</i>" ).arg( dt.toString( "yyyy-MM-dd" ) );
  }
  else if ( value.type() == QVariant::Time )
  {
    QTime tm = value.toTime();
    return tr( "<i>&lt;time: %1&gt;</i>" ).arg( tm.toString( "hh:mm:ss" ) );
  }
  else if ( value.type() == QVariant::DateTime )
  {
    QDateTime dt = value.toDateTime();
    return tr( "<i>&lt;datetime: %1&gt;</i>" ).arg( dt.toString( "yyyy-MM-dd hh:mm:ss" ) );
  }
  else if ( value.type() == QVariant::String )
  {
    QString previewString = value.toString();
    if ( previewString.length() > 63 )
    {
      return QString( tr( "'%1...'" ) ).arg( previewString.left( 60 ) );
    }
    else
    {
      return previewString.prepend( '\'' ).append( '\'' );
    }
  }
  else
  {
    return value.toString();
  }
}

QVariant QgsExpression::Function::func( const QVariantList& values, const QgsFeature* feature, QgsExpression* parent )
{
  //default implementation creates a QgsFeatureBasedExpressionContext
  QgsExpressionContext c = QgsExpressionContextUtils::createFeatureBasedContext( feature ? *feature : QgsFeature(), QgsFields() );
  return func( values, &c, parent );
}

QVariant QgsExpression::Function::func( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent )
{
  //base implementation calls deprecated func to avoid API breakage
  QgsFeature f;
  if ( context && context->hasVariable( QgsExpressionContext::EXPR_FEATURE ) )
    f = qvariant_cast<QgsFeature>( context->variable( QgsExpressionContext::EXPR_FEATURE ) );

  Q_NOWARN_DEPRECATED_PUSH
  return func( values, &f, parent );
  Q_NOWARN_DEPRECATED_POP
}

QVariant QgsExpression::Node::eval( QgsExpression* parent, const QgsFeature* feature )
{
  //default implementation creates a QgsFeatureBasedExpressionContext
  QgsExpressionContext c = QgsExpressionContextUtils::createFeatureBasedContext( feature ? *feature : QgsFeature(), QgsFields() );
  return eval( parent, &c );
}

QVariant QgsExpression::Node::eval( QgsExpression *parent, const QgsExpressionContext *context )
{
  //base implementation calls deprecated eval to avoid API breakage
  QgsFeature f;
  if ( context && context->hasVariable( QgsExpressionContext::EXPR_FEATURE ) )
    f = qvariant_cast<QgsFeature>( context->variable( QgsExpressionContext::EXPR_FEATURE ) );

  Q_NOWARN_DEPRECATED_PUSH
  return eval( parent, &f );
  Q_NOWARN_DEPRECATED_POP
}

bool QgsExpression::Node::prepare( QgsExpression* parent, const QgsFields &fields )
{
  QgsExpressionContext c = QgsExpressionContextUtils::createFeatureBasedContext( 0, fields );
  return prepare( parent, &c );
}

bool QgsExpression::Node::prepare( QgsExpression* parent, const QgsExpressionContext *context )
{
  //base implementation calls deprecated prepare to avoid API breakage
  QgsFields f;
  if ( context && context->hasVariable( QgsExpressionContext::EXPR_FIELDS ) )
    f = qvariant_cast<QgsFields>( context->variable( QgsExpressionContext::EXPR_FIELDS ) );

  Q_NOWARN_DEPRECATED_PUSH
  return prepare( parent, f );
  Q_NOWARN_DEPRECATED_POP
}

QVariant QgsExpression::StaticFunction::func( const QVariantList &values, const QgsFeature* f, QgsExpression* parent )
{
  Q_NOWARN_DEPRECATED_PUSH
  return mFnc ? mFnc( values, f, parent ) : QVariant();
  Q_NOWARN_DEPRECATED_POP
}

const QgsExpression::Node* QgsExpression::rootNode() const
{
  return d->mRootNode;
}
