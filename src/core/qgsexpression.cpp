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
#include "qgsrelationmanager.h"

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
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgsgeometryengine.h"
#include "qgsgeometryutils.h"
#include "qgslogger.h"
#include "qgsogcutils.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayerutils.h"
#include "qgscolorramp.h"
#include "qgsstyle.h"
#include "qgsexpressioncontext.h"
#include "qgsproject.h"
#include "qgsstringutils.h"
#include "qgsgeometrycollection.h"
#include "qgspointv2.h"
#include "qgspolygon.h"
#include "qgsmultipoint.h"
#include "qgsmultilinestring.h"
#include "qgscurvepolygon.h"
#include "qgsexpressionprivate.h"
#include "qgsexpressionsorter.h"
#include "qgsmaptopixelgeometrysimplifier.h"
#include "qgsmessagelog.h"
#include "qgscsexception.h"
#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"

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
  if ( v.type() == QVariant::Int )
    return true;
  if ( v.type() == QVariant::UInt )
    return true;
  if ( v.type() == QVariant::LongLong )
    return true;
  if ( v.type() == QVariant::ULongLong )
    return true;
  if ( v.type() == QVariant::Double )
    return false;
  if ( v.type() == QVariant::String )
  {
    bool ok;
    v.toString().toInt( &ok );
    return ok;
  }
  return false;
}
inline bool isDoubleSafe( const QVariant& v )
{
  if ( v.type() == QVariant::Double )
    return true;
  if ( v.type() == QVariant::Int )
    return true;
  if ( v.type() == QVariant::UInt )
    return true;
  if ( v.type() == QVariant::LongLong )
    return true;
  if ( v.type() == QVariant::ULongLong )
    return true;
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
  return v.type() == QVariant::DateTime
         || v.type() == QVariant::Date
         || v.type() == QVariant::Time;
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

inline bool isNull( const QVariant& v )
{
  return v.isNull();
}

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

  parent->setEvalErrorString( QStringLiteral( "Cannot convert to QgsGeometry" ) );
  return QgsGeometry();
}

static QgsFeature getFeature( const QVariant& value, QgsExpression* parent )
{
  if ( value.canConvert<QgsFeature>() )
    return value.value<QgsFeature>();

  parent->setEvalErrorString( QStringLiteral( "Cannot convert to QgsFeature" ) );
  return 0;
}

#define FEAT_FROM_CONTEXT(c, f) if (!c || !c->hasVariable(QgsExpressionContext::EXPR_FEATURE)) return QVariant(); \
  QgsFeature f = qvariant_cast<QgsFeature>( c->variable( QgsExpressionContext::EXPR_FEATURE ) );

static QgsExpression::Node* getNode( const QVariant& value, QgsExpression* parent )
{
  if ( value.canConvert<QgsExpression::Node*>() )
    return value.value<QgsExpression::Node*>();

  parent->setEvalErrorString( QStringLiteral( "Cannot convert to Node" ) );
  return nullptr;
}

QgsVectorLayer* getVectorLayer( const QVariant& value, QgsExpression* )
{
  QgsMapLayer* ml = value.value< QPointer<QgsMapLayer> >().data();
  QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( ml );
  if ( !vl )
  {
    QString layerString = value.toString();
    vl = qobject_cast<QgsVectorLayer*>( QgsProject::instance()->mapLayer( layerString ) ); //search by id first

    if ( !vl )
    {
      QList<QgsMapLayer *> layersByName = QgsProject::instance()->mapLayersByName( layerString );
      if ( !layersByName.isEmpty() )
      {
        vl = qobject_cast<QgsVectorLayer*>( layersByName.at( 0 ) );
      }
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

static QVariantList getListValue( const QVariant& value, QgsExpression* parent )
{
  if ( value.type() == QVariant::List || value.type() == QVariant::StringList )
  {
    return value.toList();
  }
  else
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to array" ).arg( value.toString() ) );
    return QVariantList();
  }
}

static QVariantMap getMapValue( const QVariant& value, QgsExpression* parent )
{
  if ( value.type() == QVariant::Map )
  {
    return value.toMap();
  }
  else
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to map" ).arg( value.toString() ) );
    return QVariantMap();
  }
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

static QVariant fcnAggregate( const QVariantList& values, const QgsExpressionContext* context, QgsExpression* parent )
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

  QVariant result;
  if ( context )
  {
    QString cacheKey = QStringLiteral( "aggfcn:%1:%2:%3:%4" ).arg( vl->id(), QString::number( aggregate ), subExpression, parameters.filter );

    QgsExpression subExp( subExpression );
    QgsExpression filterExp( parameters.filter );
    if ( filterExp.referencedVariables().contains( "parent" )
         || filterExp.referencedVariables().contains( QString() )
         || subExp.referencedVariables().contains( "parent" )
         || subExp.referencedVariables().contains( QString() ) )
    {
      cacheKey += ':' + qHash( context->feature() );
    }

    if ( context && context->hasCachedValue( cacheKey ) )
      return context->cachedValue( cacheKey );

    QgsExpressionContext subContext( *context );
    QgsExpressionContextScope* subScope = new QgsExpressionContextScope();
    subScope->setVariable( "parent", context->feature() );
    subContext.appendScope( subScope );
    result = vl->aggregate( aggregate, subExpression, parameters, &subContext, &ok );

    context->setCachedValue( cacheKey, result );
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
  QgsVectorLayer* vl = getVectorLayer( context->variable( "layer" ), parent );
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

  QString cacheKey = QStringLiteral( "relagg:%1:%2:%3:%4" ).arg( vl->id(),
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
  QgsVectorLayer* vl = getVectorLayer( context->variable( "layer" ), parent );
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
      parameters.filter = QStringLiteral( "(%1) AND (%2=%3)" ).arg( parameters.filter, groupBy, QgsExpression::quotedValue( groupByValue ) );
    else
      parameters.filter = QStringLiteral( "(%2 = %3)" ).arg( groupBy, QgsExpression::quotedValue( groupByValue ) );
  }

  QString cacheKey = QStringLiteral( "agg:%1:%2:%3:%4" ).arg( vl->id(),
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

static QVariant fcnAggregateCollectGeometry( const QVariantList& values, const QgsExpressionContext* context, QgsExpression *parent )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::GeometryCollect, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
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
  return QVariant( elems.join( QStringLiteral( " " ) ) );
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
      QRegExp rx;
      QString customdelimiter = getStringValue( values.at( 2 ), parent );
      int delimiterlength;

      if ( customdelimiter.length() > 0 )
      {
        rx.setPatternSyntax( QRegExp::FixedString );
        rx.setPattern( customdelimiter );
        delimiterlength = customdelimiter.length();
      }
      else
      {
        // \x200B is a ZERO-WIDTH SPACE, needed for worwrap to support a number of complex scripts (Indic, Arabic, etc.)
        rx.setPattern( QStringLiteral( "[\\s\\x200B]" ) );
        delimiterlength = 1;
      }


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
            strhit = lines[i].lastIndexOf( rx, strcurrent + wrap );
            if ( strhit == lasthit || strhit == -1 )
            {
              //if no new backward delimiter found, try to locate forward
              strhit = lines[i].indexOf( rx, strcurrent + qAbs( wrap ) );
            }
            lasthit = strhit;
          }
          else
          {
            strhit = lines[i].indexOf( rx, strcurrent + qAbs( wrap ) );
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
    if ( geom.type() != QgsWkbTypes::LineGeometry )
      return QVariant();

    return QVariant( geom.length() );
  }

  //otherwise fall back to string variant
  QString str = getStringValue( values.at( 0 ), parent );
  return QVariant( str.length() );
}

static QVariant fcnReplace( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  if ( values.count() == 2 && values.at( 1 ).type() == QVariant::Map )
  {
    QString str = getStringValue( values.at( 0 ), parent );
    QVariantMap map = getMapValue( values.at( 1 ), parent );

    for ( QVariantMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it )
    {
      str = str.replace( it.key(), it.value().toString() );
    }

    return QVariant( str );
  }
  else if ( values.count() == 3 )
  {
    QString str = getStringValue( values.at( 0 ), parent );
    QVariantList before;
    QVariantList after;
    bool isSingleReplacement = false;

    if ( values.at( 1 ).type() != QVariant::List && values.at( 2 ).type() != QVariant::StringList )
    {
      before = QVariantList() << getStringValue( values.at( 1 ), parent );
    }
    else
    {
      before = getListValue( values.at( 1 ), parent );
    }

    if ( values.at( 2 ).type() != QVariant::List && values.at( 2 ).type() != QVariant::StringList )
    {
      after = QVariantList() << getStringValue( values.at( 2 ), parent );
      isSingleReplacement = true;
    }
    else
    {
      after = getListValue( values.at( 2 ), parent );
    }

    if ( !isSingleReplacement && before.length() != after.length() )
    {
      parent->setEvalErrorString( QObject::tr( "Invalid pair of array, length not identical" ) );
      return QVariant();
    }

    for ( int i = 0; i < before.length(); i++ )
    {
      str = str.replace( before.at( i ).toString(), after.at( isSingleReplacement ? 0 : i ).toString() );
    }

    return QVariant( str );
  }
  else
  {
    parent->setEvalErrorString( QObject::tr( "Function replace requires 2 or 3 arguments" ) );
    return QVariant();
  }
}
static QVariant fcnRegexpReplace( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString str = getStringValue( values.at( 0 ), parent );
  QString regexp = getStringValue( values.at( 1 ), parent );
  QString after = getStringValue( values.at( 2 ), parent );

  QRegularExpression re( regexp );
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

  QRegularExpression re( regexp );
  if ( !re.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Invalid regular expression '%1': %2" ).arg( regexp, re.errorString() ) );
    return QVariant();
  }
  return QVariant(( str.indexOf( re ) + 1 ) );
}

static QVariant fcnRegexpMatches( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString str = getStringValue( values.at( 0 ), parent );
  QString regexp = getStringValue( values.at( 1 ), parent );
  QString empty = getStringValue( values.at( 2 ), parent );

  QRegularExpression re( regexp );
  if ( !re.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Invalid regular expression '%1': %2" ).arg( regexp, re.errorString() ) );
    return QVariant();
  }

  QRegularExpressionMatch matches = re.match( str );
  if ( matches.hasMatch() )
  {
    QVariantList array;
    QStringList list = matches.capturedTexts();

    // Skip the first string to only return captured groups
    for ( QStringList::const_iterator it = ++list.constBegin(); it != list.constEnd(); ++it )
    {
      array += ( *it ).isEmpty() == false ? *it : empty;
    }

    return QVariant( array );
  }
  else
  {
    return QVariant();
  }
}

static QVariant fcnRegexpSubstr( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString str = getStringValue( values.at( 0 ), parent );
  QString regexp = getStringValue( values.at( 1 ), parent );

  QRegularExpression re( regexp );
  if ( !re.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Invalid regular expression '%1': %2" ).arg( regexp, re.errorString() ) );
    return QVariant();
  }

  // extract substring
  QRegularExpressionMatch match = re.match( str );
  if ( match.hasMatch() )
  {
    // return first capture
    return QVariant( match.captured( 0 ) );
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
  if ( !values.at( 0 ).isValid() || !values.at( 1 ).isValid() )
    return QVariant();

  QString str = getStringValue( values.at( 0 ), parent );
  int from = getIntValue( values.at( 1 ), parent );

  int len = 0;
  if ( values.at( 2 ).isValid() )
    len = getIntValue( values.at( 2 ), parent );
  else
    len = str.size();

  if ( from < 0 )
  {
    from = str.size() + from;
    if ( from < 0 )
    {
      from = 0;
    }
  }
  else if ( from > 0 )
  {
    //account for the fact that substr() starts at 1
    from -= 1;
  }

  if ( len < 0 )
  {
    len = str.size() + len - from;
    if ( len < 0 )
    {
      len = 0;
    }
  }

  return QVariant( str.mid( from, len ) );
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

  return context->feature();
}
static QVariant fcnAttribute( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsFeature feat = getFeature( values.at( 0 ), parent );
  QString attr = getStringValue( values.at( 1 ), parent );

  return feat.attribute( attr );
}

static QVariant fcnIsSelected( const QVariantList& values, const QgsExpressionContext* context, QgsExpression* parent )
{
  QgsVectorLayer* layer = nullptr;
  QgsFeature feature;

  if ( values.isEmpty() )
  {
    feature = context->feature();
    layer = getVectorLayer( context->variable( "layer" ), parent );
  }
  else if ( values.size() == 1 )
  {
    layer = getVectorLayer( context->variable( "layer" ), parent );
    feature = getFeature( values.at( 0 ), parent );
  }
  else if ( values.size() == 2 )
  {
    layer = getVectorLayer( values.at( 0 ), parent );
    feature = getFeature( values.at( 1 ), parent );
  }
  else
  {
    parent->setEvalErrorString( QObject::tr( "Function `is_selected` requires no more than two parameters. %1 given." ).arg( values.length() ) );
    return QVariant();
  }

  if ( !layer || !feature.isValid() )
  {
    return QVariant( QVariant::Bool );
  }

  return layer->selectedFeatureIds().contains( feature.id() );
}

static QVariant fcnNumSelected( const QVariantList& values, const QgsExpressionContext* context, QgsExpression* parent )
{
  QgsVectorLayer* layer = nullptr;

  if ( values.isEmpty() )
    layer = getVectorLayer( context->variable( "layer" ), parent );
  else if ( values.count() == 1 )
    layer = getVectorLayer( values.at( 0 ), parent );
  else
  {
    parent->setEvalErrorString( QObject::tr( "Function `num_selected` requires no more than one parameter. %1 given." ).arg( values.length() ) );
    return QVariant();
  }

  if ( !layer )
  {
    return QVariant( QVariant::LongLong );
  }

  return layer->selectedFeatureCount();
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
  return string.indexOf( getStringValue( values.at( 1 ), parent ) ) + 1;
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


#define ENSURE_GEOM_TYPE(f, g, geomtype) \
  if ( !f.hasGeometry() ) \
    return QVariant(); \
  QgsGeometry g = f.geometry(); \
  if ( g.type() != geomtype ) \
    return QVariant();

static QVariant fcnX( const QVariantList&, const QgsExpressionContext* context, QgsExpression* )
{
  FEAT_FROM_CONTEXT( context, f );
  ENSURE_GEOM_TYPE( f, g, QgsWkbTypes::PointGeometry );
  if ( g.isMultipart() )
  {
    return g.asMultiPoint().at( 0 ).x();
  }
  else
  {
    return g.asPoint().x();
  }
}

static QVariant fcnY( const QVariantList&, const QgsExpressionContext* context, QgsExpression* )
{
  FEAT_FROM_CONTEXT( context, f );
  ENSURE_GEOM_TYPE( f, g, QgsWkbTypes::PointGeometry );
  if ( g.isMultipart() )
  {
    return g.asMultiPoint().at( 0 ).y();
  }
  else
  {
    return g.asPoint().y();
  }
}

static QVariant fcnGeomX( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );
  if ( geom.isEmpty() )
    return QVariant();

  //if single point, return the point's x coordinate
  if ( geom.type() == QgsWkbTypes::PointGeometry && !geom.isMultipart() )
  {
    return geom.asPoint().x();
  }

  //otherwise return centroid x
  QgsGeometry centroid = geom.centroid();
  QVariant result( centroid.asPoint().x() );
  return result;
}

static QVariant fcnGeomY( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );
  if ( geom.isEmpty() )
    return QVariant();

  //if single point, return the point's y coordinate
  if ( geom.type() == QgsWkbTypes::PointGeometry && !geom.isMultipart() )
  {
    return geom.asPoint().y();
  }

  //otherwise return centroid y
  QgsGeometry centroid = geom.centroid();
  QVariant result( centroid.asPoint().y() );
  return result;
}

static QVariant fcnGeomZ( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );
  if ( geom.isEmpty() )
    return QVariant(); //or 0?

  //if single point, return the point's z coordinate
  if ( geom.type() == QgsWkbTypes::PointGeometry && !geom.isMultipart() )
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
  if ( geom.type() == QgsWkbTypes::PointGeometry && !geom.isMultipart() )
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

  Q_FOREACH ( const QgsRingSequence &part, geom.geometry()->coordinateSequence() )
  {
    Q_FOREACH ( const QgsPointSequence &ring, part )
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

  QList< QgsLineString* > linesToProcess = QgsGeometryUtils::extractLineStrings( geom.geometry() );

  //ok, now we have a complete list of segmentized lines from the geometry
  QgsMultiLineString* ml = new QgsMultiLineString();
  Q_FOREACH ( QgsLineString* line, linesToProcess )
  {
    for ( int i = 0; i < line->numPoints() - 1; ++i )
    {
      QgsLineString* segment = new QgsLineString();
      segment->setPoints( QgsPointSequence()
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

  QgsCurvePolygon* curvePolygon = dynamic_cast< QgsCurvePolygon* >( geom.geometry() );
  if ( !curvePolygon )
    return QVariant();

  //idx is 1 based
  int idx = getIntValue( values.at( 1 ), parent ) - 1;

  if ( idx >= curvePolygon->numInteriorRings() || idx < 0 )
    return QVariant();

  QgsCurve* curve = static_cast< QgsCurve* >( curvePolygon->interiorRing( idx )->clone() );
  QVariant result = curve ? QVariant::fromValue( QgsGeometry( curve ) ) : QVariant();
  return result;
}

static QVariant fcnGeometryN( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.isEmpty() )
    return QVariant();

  QgsGeometryCollection* collection = dynamic_cast< QgsGeometryCollection* >( geom.geometry() );
  if ( !collection )
    return QVariant();

  //idx is 1 based
  int idx = getIntValue( values.at( 1 ), parent ) - 1;

  if ( idx < 0 || idx >= collection->numGeometries() )
    return QVariant();

  QgsAbstractGeometry* part = collection->geometryN( idx )->clone();
  QVariant result = part ? QVariant::fromValue( QgsGeometry( part ) ) : QVariant();
  return result;
}

static QVariant fcnBoundary( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.isEmpty() )
    return QVariant();

  QgsAbstractGeometry* boundary = geom.geometry()->boundary();
  if ( !boundary )
    return QVariant();

  return QVariant::fromValue( QgsGeometry( boundary ) );
}

static QVariant fcnLineMerge( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.isEmpty() )
    return QVariant();

  QgsGeometry merged = geom.mergeLines();
  if ( merged.isEmpty() )
    return QVariant();

  return QVariant::fromValue( merged );
}

static QVariant fcnSimplify( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.isEmpty() )
    return QVariant();

  double tolerance = getDoubleValue( values.at( 1 ), parent );

  QgsGeometry simplified = geom.simplify( tolerance );
  if ( simplified.isEmpty() )
    return QVariant();

  return simplified;
}

static QVariant fcnSimplifyVW( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.isEmpty() )
    return QVariant();

  double tolerance = getDoubleValue( values.at( 1 ), parent );

  QgsMapToPixelSimplifier simplifier( QgsMapToPixelSimplifier::SimplifyGeometry, tolerance, QgsMapToPixelSimplifier::Visvalingam );

  QgsGeometry simplified = simplifier.simplify( geom );
  if ( simplified.isEmpty() )
    return QVariant();

  return simplified;
}

static QVariant fcnSmooth( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.isEmpty() )
    return QVariant();

  int iterations = qMin( getIntValue( values.at( 1 ), parent ), 10 );
  double offset = qBound( 0.0, getDoubleValue( values.at( 2 ), parent ), 0.5 );
  double minLength = getDoubleValue( values.at( 3 ), parent );
  double maxAngle = qBound( 0.0, getDoubleValue( values.at( 4 ), parent ), 180.0 );

  QgsGeometry smoothed = geom.smooth( iterations, offset, minLength, maxAngle );
  if ( smoothed.isEmpty() )
    return QVariant();

  return smoothed;
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
      return QVariant::fromValue( QgsGeometry( new QgsPointV2( QgsWkbTypes::PointZ, x, y, z ) ) );
    case 4:
      return QVariant::fromValue( QgsGeometry( new QgsPointV2( QgsWkbTypes::PointZM, x, y, z, m ) ) );
  }
  return QVariant(); //avoid warning
}

static QVariant fcnMakePointM( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  double x = getDoubleValue( values.at( 0 ), parent );
  double y = getDoubleValue( values.at( 1 ), parent );
  double m = getDoubleValue( values.at( 2 ), parent );
  return QVariant::fromValue( QgsGeometry( new QgsPointV2( QgsWkbTypes::PointM, x, y, 0.0, m ) ) );
}

static QVariant fcnMakeLine( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  if ( values.count() < 2 )
  {
    return QVariant();
  }

  QgsLineString* lineString = new QgsLineString();
  lineString->clear();

  Q_FOREACH ( const QVariant& value, values )
  {
    QgsGeometry geom = getGeometry( value, parent );
    if ( geom.isEmpty() )
      continue;

    if ( geom.type() != QgsWkbTypes::PointGeometry || geom.isMultipart() )
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
  if ( outerRing.type() != QgsWkbTypes::LineGeometry || outerRing.isMultipart() || outerRing.isEmpty() )
    return QVariant();

  QgsPolygonV2* polygon = new QgsPolygonV2();
  polygon->setExteriorRing( dynamic_cast< QgsCurve* >( outerRing.geometry()->clone() ) );

  for ( int i = 1; i < values.count(); ++i )
  {
    QgsGeometry ringGeom = getGeometry( values.at( i ), parent );
    if ( ringGeom.isEmpty() )
      continue;

    if ( ringGeom.type() != QgsWkbTypes::LineGeometry || ringGeom.isMultipart() || ringGeom.isEmpty() )
      continue;

    polygon->addInteriorRing( dynamic_cast< QgsCurve* >( ringGeom.geometry()->clone() ) );
  }

  return QVariant::fromValue( QgsGeometry( polygon ) );
}

static QVariant pointAt( const QVariantList& values, const QgsExpressionContext* context, QgsExpression* parent ) // helper function
{
  FEAT_FROM_CONTEXT( context, f );
  int idx = getIntValue( values.at( 0 ), parent );
  QgsGeometry g = f.geometry();
  if ( g.isEmpty() )
    return QVariant();

  if ( idx < 0 )
  {
    idx += g.geometry()->nCoordinates();
  }
  if ( idx < 0 || idx >= g.geometry()->nCoordinates() )
  {
    parent->setEvalErrorString( QObject::tr( "Index is out of range" ) );
    return QVariant();
  }

  QgsPoint p = g.vertexAt( idx );
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
  QgsGeometry geom = f.geometry();
  if ( !geom.isEmpty() )
    return  QVariant::fromValue( geom );
  else
    return QVariant();
}
static QVariant fcnGeomFromWKT( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString wkt = getStringValue( values.at( 0 ), parent );
  QgsGeometry geom = QgsGeometry::fromWkt( wkt );
  QVariant result = !geom.isEmpty() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}
static QVariant fcnGeomFromGML( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString gml = getStringValue( values.at( 0 ), parent );
  QgsGeometry geom = QgsOgcUtils::geometryFromGML( gml );
  QVariant result = !geom.isEmpty() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnGeomArea( const QVariantList&, const QgsExpressionContext* context, QgsExpression* parent )
{
  FEAT_FROM_CONTEXT( context, f );
  ENSURE_GEOM_TYPE( f, g, QgsWkbTypes::PolygonGeometry );
  QgsDistanceArea* calc = parent->geomCalculator();
  if ( calc )
  {
    double area = calc->measureArea( f.geometry() );
    area = calc->convertAreaMeasurement( area, parent->areaUnits() );
    return QVariant( area );
  }
  else
  {
    return QVariant( f.geometry().area() );
  }
}

static QVariant fcnArea( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.type() != QgsWkbTypes::PolygonGeometry )
    return QVariant();

  return QVariant( geom.area() );
}

static QVariant fcnGeomLength( const QVariantList&, const QgsExpressionContext* context, QgsExpression* parent )
{
  FEAT_FROM_CONTEXT( context, f );
  ENSURE_GEOM_TYPE( f, g, QgsWkbTypes::LineGeometry );
  QgsDistanceArea* calc = parent->geomCalculator();
  if ( calc )
  {
    double len = calc->measureLength( f.geometry() );
    len = calc->convertLengthMeasurement( len, parent->distanceUnits() );
    return QVariant( len );
  }
  else
  {
    return QVariant( f.geometry().length() );
  }
}

static QVariant fcnGeomPerimeter( const QVariantList&, const QgsExpressionContext* context, QgsExpression* parent )
{
  FEAT_FROM_CONTEXT( context, f );
  ENSURE_GEOM_TYPE( f, g, QgsWkbTypes::PolygonGeometry );
  QgsDistanceArea* calc = parent->geomCalculator();
  if ( calc )
  {
    double len = calc->measurePerimeter( f.geometry() );
    len = calc->convertLengthMeasurement( len, parent->distanceUnits() );
    return QVariant( len );
  }
  else
  {
    return f.geometry().isEmpty() ? QVariant( 0 ) : QVariant( f.geometry().geometry()->perimeter() );
  }
}

static QVariant fcnPerimeter( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );

  if ( geom.type() != QgsWkbTypes::PolygonGeometry )
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

  QgsCurvePolygon* curvePolygon = dynamic_cast< QgsCurvePolygon* >( geom.geometry() );
  if ( curvePolygon )
    return QVariant( curvePolygon->numInteriorRings() );

  QgsGeometryCollection* collection = dynamic_cast< QgsGeometryCollection* >( geom.geometry() );
  if ( collection )
  {
    //find first CurvePolygon in collection
    for ( int i = 0; i < collection->numGeometries(); ++i )
    {
      curvePolygon = dynamic_cast< QgsCurvePolygon*>( collection->geometryN( i ) );
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

  QgsCurvePolygon* curvePolygon = dynamic_cast< QgsCurvePolygon* >( geom.geometry() );
  if ( curvePolygon )
    return QVariant( curvePolygon->ringCount() );

  bool foundPoly = false;
  int ringCount = 0;
  QgsGeometryCollection* collection = dynamic_cast< QgsGeometryCollection* >( geom.geometry() );
  if ( collection )
  {
    //find CurvePolygons in collection
    for ( int i = 0; i < collection->numGeometries(); ++i )
    {
      curvePolygon = dynamic_cast< QgsCurvePolygon*>( collection->geometryN( i ) );
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
  QgsGeometry geomBounds = QgsGeometry::fromRect( geom.boundingBox() );
  QVariant result = !geomBounds.isEmpty() ? QVariant::fromValue( geomBounds ) : QVariant();
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

  QgsCurve* curve = dynamic_cast< QgsCurve* >( fGeom.geometry() );
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
  return fGeom.disjoint( sGeom ) ? TVL_True : TVL_False;
}
static QVariant fcnIntersects( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  return fGeom.intersects( sGeom ) ? TVL_True : TVL_False;
}
static QVariant fcnTouches( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  return fGeom.touches( sGeom ) ? TVL_True : TVL_False;
}
static QVariant fcnCrosses( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  return fGeom.crosses( sGeom ) ? TVL_True : TVL_False;
}
static QVariant fcnContains( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  return fGeom.contains( sGeom ) ? TVL_True : TVL_False;
}
static QVariant fcnOverlaps( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  return fGeom.overlaps( sGeom ) ? TVL_True : TVL_False;
}
static QVariant fcnWithin( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  return fGeom.within( sGeom ) ? TVL_True : TVL_False;
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

  QgsGeometry geom = fGeom.buffer( dist, seg );
  QVariant result = !geom.isEmpty() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnOffsetCurve( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  double dist = getDoubleValue( values.at( 1 ), parent );
  int segments = getIntValue( values.at( 2 ), parent );
  QgsGeometry::JoinStyle join = static_cast< QgsGeometry::JoinStyle >( getIntValue( values.at( 3 ), parent ) );
  if ( join < QgsGeometry::JoinStyleRound || join > QgsGeometry::JoinStyleBevel )
    return QVariant();
  double mitreLimit = getDoubleValue( values.at( 3 ), parent );

  QgsGeometry geom = fGeom.offsetCurve( dist, segments, join, mitreLimit );
  QVariant result = !geom.isEmpty() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnSingleSidedBuffer( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  double dist = getDoubleValue( values.at( 1 ), parent );
  int segments = getIntValue( values.at( 2 ), parent );
  QgsGeometry::JoinStyle join = static_cast< QgsGeometry::JoinStyle >( getIntValue( values.at( 3 ), parent ) );
  if ( join < QgsGeometry::JoinStyleRound || join > QgsGeometry::JoinStyleBevel )
    return QVariant();
  double mitreLimit = getDoubleValue( values.at( 3 ), parent );

  QgsGeometry geom = fGeom.singleSidedBuffer( dist, segments, QgsGeometry::SideLeft, join, mitreLimit );
  QVariant result = !geom.isEmpty() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnExtend( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  double distStart = getDoubleValue( values.at( 1 ), parent );
  double distEnd = getDoubleValue( values.at( 2 ), parent );

  QgsGeometry geom = fGeom.extendLine( distStart, distEnd );
  QVariant result = !geom.isEmpty() ? QVariant::fromValue( geom ) : QVariant();
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
  QgsGeometry geom = fGeom.centroid();
  QVariant result = !geom.isEmpty() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}
static QVariant fcnPointOnSurface( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry geom = fGeom.pointOnSurface();
  QVariant result = !geom.isEmpty() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnPoleOfInaccessibility( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  double tolerance = getDoubleValue( values.at( 1 ), parent );
  QgsGeometry geom = fGeom.poleOfInaccessibility( tolerance );
  QVariant result = !geom.isEmpty() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnConvexHull( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry geom = fGeom.convexHull();
  QVariant result = !geom.isEmpty() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}
static QVariant fcnDifference( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  QgsGeometry geom = fGeom.difference( sGeom );
  QVariant result = !geom.isEmpty() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnReverse( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  if ( fGeom.isEmpty() )
    return QVariant();

  QgsCurve* curve = dynamic_cast< QgsCurve* >( fGeom.geometry() );
  if ( !curve )
    return QVariant();

  QgsCurve* reversed = curve->reversed();
  QVariant result = reversed ? QVariant::fromValue( QgsGeometry( reversed ) ) : QVariant();
  return result;
}

static QVariant fcnExteriorRing( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  if ( fGeom.isEmpty() )
    return QVariant();

  QgsCurvePolygon* curvePolygon = dynamic_cast< QgsCurvePolygon* >( fGeom.geometry() );
  if ( !curvePolygon || !curvePolygon->exteriorRing() )
    return QVariant();

  QgsCurve* exterior = static_cast< QgsCurve* >( curvePolygon->exteriorRing()->clone() );
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
  QgsGeometry geom = fGeom.intersection( sGeom );
  QVariant result = !geom.isEmpty() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}
static QVariant fcnSymDifference( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  QgsGeometry geom = fGeom.symDifference( sGeom );
  QVariant result = !geom.isEmpty() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}
static QVariant fcnCombine( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = getGeometry( values.at( 1 ), parent );
  QgsGeometry geom = fGeom.combine( sGeom );
  QVariant result = !geom.isEmpty() ? QVariant::fromValue( geom ) : QVariant();
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

  if ( geom.type() != QgsWkbTypes::PointGeometry )
  {
    parent->setEvalErrorString( QStringLiteral( "'project' requires a point geometry" ) );
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

  QgsGeometryCollection* collection = dynamic_cast<QgsGeometryCollection*>( fGeom.geometry() );
  Q_ASSERT( collection ); // Should have failed the multipart check above

  QgsFeatureRequest::OrderBy orderBy;
  orderBy.append( QgsFeatureRequest::OrderByClause( expString, asc ) );
  QgsExpressionSorter sorter( orderBy );

  QList<QgsFeature> partFeatures;
  partFeatures.reserve( collection->partCount() );
  for ( int i = 0; i < collection->partCount(); ++i )
  {
    f.setGeometry( QgsGeometry( collection->geometryN( i )->clone() ) );
    partFeatures << f;
  }

  sorter.sortFeatures( partFeatures, unconstedContext );

  QgsGeometryCollection* orderedGeom = dynamic_cast<QgsGeometryCollection*>( fGeom.geometry()->clone() );

  Q_ASSERT( orderedGeom );

  while ( orderedGeom->partCount() )
    orderedGeom->removeGeometry( 0 );

  Q_FOREACH ( const QgsFeature& feature, partFeatures )
  {
    orderedGeom->addGeometry( feature.geometry().geometry()->clone() );
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

static QVariant fcnLineInterpolatePoint( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry lineGeom = getGeometry( values.at( 0 ), parent );
  double distance = getDoubleValue( values.at( 1 ), parent );

  QgsGeometry geom = lineGeom.interpolate( distance );

  QVariant result = !geom.isEmpty() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnLineInterpolateAngle( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry lineGeom = getGeometry( values.at( 0 ), parent );
  double distance = getDoubleValue( values.at( 1 ), parent );

  return lineGeom.interpolateAngle( distance ) * 180.0 / M_PI;
}

static QVariant fcnAngleAtVertex( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );
  int vertex = getIntValue( values.at( 1 ), parent );

  return geom.angleAtVertex( vertex ) * 180.0 / M_PI;
}

static QVariant fcnDistanceToVertex( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry geom = getGeometry( values.at( 0 ), parent );
  int vertex = getIntValue( values.at( 1 ), parent );

  return geom.distanceToVertex( vertex );
}

static QVariant fcnLineLocatePoint( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry lineGeom = getGeometry( values.at( 0 ), parent );
  QgsGeometry pointGeom = getGeometry( values.at( 1 ), parent );

  double distance = lineGeom.lineLocatePoint( pointGeom );

  return distance >= 0 ? distance : QVariant();
}

static QVariant fcnRound( const QVariantList& values, const QgsExpressionContext *, QgsExpression* parent )
{
  if ( values.length() == 2 && values.at( 1 ).toInt() != 0 )
  {
    double number = getDoubleValue( values.at( 0 ), parent );
    double scaler = pow( 10.0, getIntValue( values.at( 1 ), parent ) );
    return QVariant( qRound( number * scaler ) / scaler );
  }

  if ( values.length() >= 1 )
  {
    double number = getIntValue( values.at( 0 ), parent );
    return QVariant( qRound( number ) );
  }

  return QVariant();
}

static QVariant fcnPi( const QVariantList& values, const QgsExpressionContext *, QgsExpression* parent )
{
  Q_UNUSED( values );
  Q_UNUSED( parent );
  return M_PI;
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
  return QStringLiteral( "%L1" ).arg( value, 0, 'f', places );
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

  return QStringLiteral( "%1,%2,%3" ).arg( color.red() ).arg( color.green() ).arg( color.blue() );
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
  return QgsSymbolLayerUtils::encodeColor( color );
}

QVariant fcnRampColor( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent )
{
  QString rampName = getStringValue( values.at( 0 ), parent );
  const QgsColorRamp *mRamp = QgsStyle::defaultStyle()->colorRampRef( rampName );
  if ( ! mRamp )
  {
    parent->setEvalErrorString( QObject::tr( "\"%1\" is not a valid color ramp" ).arg( rampName ) );
    return QColor( 0, 0, 0 ).name();
  }
  double value = getDoubleValue( values.at( 1 ), parent );
  QColor color = mRamp->color( value );
  return QgsSymbolLayerUtils::encodeColor( color );
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

  return QStringLiteral( "%1,%2,%3" ).arg( color.red() ).arg( color.green() ).arg( color.blue() );
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
  return QgsSymbolLayerUtils::encodeColor( color );
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

  return QStringLiteral( "%1,%2,%3" ).arg( color.red() ).arg( color.green() ).arg( color.blue() );
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
  return QgsSymbolLayerUtils::encodeColor( color );
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

  return QStringLiteral( "%1,%2,%3" ).arg( color.red() ).arg( color.green() ).arg( color.blue() );
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
  return QgsSymbolLayerUtils::encodeColor( color );
}

static QVariant fncColorPart( const QVariantList &values, const QgsExpressionContext*, QgsExpression *parent )
{
  QColor color = QgsSymbolLayerUtils::decodeColor( values.at( 0 ).toString() );
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
  QColor color = QgsSymbolLayerUtils::decodeColor( values.at( 0 ).toString() );
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
  return QgsSymbolLayerUtils::encodeColor( color );
}

static QVariant fncDarker( const QVariantList &values, const QgsExpressionContext*, QgsExpression *parent )
{
  QColor color = QgsSymbolLayerUtils::decodeColor( values.at( 0 ).toString() );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to color" ).arg( values.at( 0 ).toString() ) );
    return QVariant();
  }

  color = color.darker( getIntValue( values.at( 1 ), parent ) );

  return QgsSymbolLayerUtils::encodeColor( color );
}

static QVariant fncLighter( const QVariantList &values, const QgsExpressionContext*, QgsExpression *parent )
{
  QColor color = QgsSymbolLayerUtils::decodeColor( values.at( 0 ).toString() );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to color" ).arg( values.at( 0 ).toString() ) );
    return QVariant();
  }

  color = color.lighter( getIntValue( values.at( 1 ), parent ) );

  return QgsSymbolLayerUtils::encodeColor( color );
}

static QVariant fcnGetGeometry( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsFeature feat = getFeature( values.at( 0 ), parent );
  QgsGeometry geom = feat.geometry();
  if ( !geom.isEmpty() )
    return QVariant::fromValue( geom );
  return QVariant();
}

static QVariant fcnTransformGeometry( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QgsGeometry fGeom = getGeometry( values.at( 0 ), parent );
  QString sAuthId = getStringValue( values.at( 1 ), parent );
  QString dAuthId = getStringValue( values.at( 2 ), parent );

  QgsCoordinateReferenceSystem s = QgsCoordinateReferenceSystem::fromOgcWmsCrs( sAuthId );
  if ( ! s.isValid() )
    return QVariant::fromValue( fGeom );
  QgsCoordinateReferenceSystem d = QgsCoordinateReferenceSystem::fromOgcWmsCrs( dAuthId );
  if ( ! d.isValid() )
    return QVariant::fromValue( fGeom );

  QgsCoordinateTransform t( s, d );
  try
  {
    if ( fGeom.transform( t ) == 0 )
      return QVariant::fromValue( fGeom );
  }
  catch ( QgsCsException &cse )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Transform error caught in transform() function: %1" ).arg( cse.what() ) );
    return QVariant();
  }
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
  int attributeId = vl->fields().lookupField( attribute );
  if ( attributeId == -1 )
  {
    return QVariant();
  }

  const QVariant& attVal = values.at( 2 );
  QgsFeatureRequest req;
  req.setFilterExpression( QStringLiteral( "%1=%2" ).arg( QgsExpression::quotedColumnRef( attribute ),
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
  QgsMapLayer* layer = QgsProject::instance()->mapLayer( layerIdOrName ); //search by id first
  if ( !layer )
  {
    QList<QgsMapLayer *> layersByName = QgsProject::instance()->mapLayersByName( layerIdOrName );
    if ( !layersByName.isEmpty() )
    {
      layer = layersByName.at( 0 );
    }
  }

  if ( !layer )
    return QVariant();

  QString layerProperty = getStringValue( values.at( 1 ), parent );
  if ( QString::compare( layerProperty, QStringLiteral( "name" ), Qt::CaseInsensitive ) == 0 )
    return layer->name();
  else if ( QString::compare( layerProperty, QStringLiteral( "id" ), Qt::CaseInsensitive ) == 0 )
    return layer->id();
  else if ( QString::compare( layerProperty, QStringLiteral( "title" ), Qt::CaseInsensitive ) == 0 )
    return layer->title();
  else if ( QString::compare( layerProperty, QStringLiteral( "abstract" ), Qt::CaseInsensitive ) == 0 )
    return layer->abstract();
  else if ( QString::compare( layerProperty, QStringLiteral( "keywords" ), Qt::CaseInsensitive ) == 0 )
    return layer->keywordList();
  else if ( QString::compare( layerProperty, QStringLiteral( "data_url" ), Qt::CaseInsensitive ) == 0 )
    return layer->dataUrl();
  else if ( QString::compare( layerProperty, QStringLiteral( "attribution" ), Qt::CaseInsensitive ) == 0 )
    return layer->attribution();
  else if ( QString::compare( layerProperty, QStringLiteral( "attribution_url" ), Qt::CaseInsensitive ) == 0 )
    return layer->attributionUrl();
  else if ( QString::compare( layerProperty, QStringLiteral( "source" ), Qt::CaseInsensitive ) == 0 )
    return layer->publicSource();
  else if ( QString::compare( layerProperty, QStringLiteral( "min_scale" ), Qt::CaseInsensitive ) == 0 )
    return layer->minimumScale();
  else if ( QString::compare( layerProperty, QStringLiteral( "max_scale" ), Qt::CaseInsensitive ) == 0 )
    return layer->maximumScale();
  else if ( QString::compare( layerProperty, QStringLiteral( "crs" ), Qt::CaseInsensitive ) == 0 )
    return layer->crs().authid();
  else if ( QString::compare( layerProperty, QStringLiteral( "crs_definition" ), Qt::CaseInsensitive ) == 0 )
    return layer->crs().toProj4();
  else if ( QString::compare( layerProperty, QStringLiteral( "extent" ), Qt::CaseInsensitive ) == 0 )
  {
    QgsGeometry extentGeom = QgsGeometry::fromRect( layer->extent() );
    QVariant result = QVariant::fromValue( extentGeom );
    return result;
  }
  else if ( QString::compare( layerProperty, QStringLiteral( "type" ), Qt::CaseInsensitive ) == 0 )
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
      if ( QString::compare( layerProperty, QStringLiteral( "storage_type" ), Qt::CaseInsensitive ) == 0 )
        return vLayer->storageType();
      else if ( QString::compare( layerProperty, QStringLiteral( "geometry_type" ), Qt::CaseInsensitive ) == 0 )
        return QgsWkbTypes::geometryDisplayString( vLayer->geometryType() );
      else if ( QString::compare( layerProperty, QStringLiteral( "feature_count" ), Qt::CaseInsensitive ) == 0 )
        return QVariant::fromValue( vLayer->featureCount() );
    }
  }

  return QVariant();
}

static QVariant fcnGetRasterBandStat( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString layerIdOrName = getStringValue( values.at( 0 ), parent );

  //try to find a matching layer by name
  QgsMapLayer* layer = QgsProject::instance()->mapLayer( layerIdOrName ); //search by id first
  if ( !layer )
  {
    QList<QgsMapLayer *> layersByName = QgsProject::instance()->mapLayersByName( layerIdOrName );
    if ( !layersByName.isEmpty() )
    {
      layer = layersByName.at( 0 );
    }
  }

  if ( !layer )
    return QVariant();

  QgsRasterLayer* rl = qobject_cast< QgsRasterLayer* >( layer );
  if ( !rl )
    return QVariant();

  int band = getIntValue( values.at( 1 ), parent );
  if ( band < 1 || band > rl->bandCount() )
  {
    parent->setEvalErrorString( QObject::tr( "Invalid band number %1 for layer %2" ).arg( band ).arg( layerIdOrName ) );
    return QVariant();
  }

  QString layerProperty = getStringValue( values.at( 2 ), parent );
  int stat = 0;

  if ( QString::compare( layerProperty, QStringLiteral( "avg" ), Qt::CaseInsensitive ) == 0 )
    stat = QgsRasterBandStats::Mean;
  else if ( QString::compare( layerProperty, QStringLiteral( "stdev" ), Qt::CaseInsensitive ) == 0 )
    stat = QgsRasterBandStats::StdDev;
  else if ( QString::compare( layerProperty, QStringLiteral( "min" ), Qt::CaseInsensitive ) == 0 )
    stat = QgsRasterBandStats::Min;
  else if ( QString::compare( layerProperty, QStringLiteral( "max" ), Qt::CaseInsensitive ) == 0 )
    stat = QgsRasterBandStats::Max;
  else if ( QString::compare( layerProperty, QStringLiteral( "range" ), Qt::CaseInsensitive ) == 0 )
    stat = QgsRasterBandStats::Range;
  else if ( QString::compare( layerProperty, QStringLiteral( "sum" ), Qt::CaseInsensitive ) == 0 )
    stat = QgsRasterBandStats::Sum;
  else
  {
    parent->setEvalErrorString( QObject::tr( "Invalid raster statistic: '%1'" ).arg( layerProperty ) );
    return QVariant();
  }

  QgsRasterBandStats stats = rl->dataProvider()->bandStatistics( band, stat );
  switch ( stat )
  {
    case QgsRasterBandStats::Mean:
      return stats.mean;
    case QgsRasterBandStats::StdDev:
      return stats.stdDev;
    case QgsRasterBandStats::Min:
      return stats.minimumValue;
    case QgsRasterBandStats::Max:
      return stats.maximumValue;
    case QgsRasterBandStats::Range:
      return stats.range;
    case QgsRasterBandStats::Sum:
      return stats.sum;
  }
  return QVariant();
}

static QVariant fcnArray( const QVariantList& values, const QgsExpressionContext*, QgsExpression* )
{
  return values;
}

static QVariant fcnArrayLength( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  return getListValue( values.at( 0 ), parent ).length();
}

static QVariant fcnArrayContains( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  return QVariant( getListValue( values.at( 0 ), parent ).contains( values.at( 1 ) ) );
}

static QVariant fcnArrayFind( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  return getListValue( values.at( 0 ), parent ).indexOf( values.at( 1 ) );
}

static QVariant fcnArrayGet( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  const QVariantList list = getListValue( values.at( 0 ), parent );
  const int pos = getIntValue( values.at( 1 ), parent );
  if ( pos < 0 || pos >= list.length() ) return QVariant();
  return list.at( pos );
}

static QVariant convertToSameType( const QVariant& value, QVariant::Type type )
{
  QVariant result = value;
  result.convert( type );
  return result;
}

static QVariant fcnArrayAppend( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QVariantList list = getListValue( values.at( 0 ), parent );
  list.append( values.at( 1 ) );
  return convertToSameType( list , values.at( 0 ).type() );
}

static QVariant fcnArrayPrepend( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QVariantList list = getListValue( values.at( 0 ), parent );
  list.prepend( values.at( 1 ) );
  return convertToSameType( list , values.at( 0 ).type() );
}

static QVariant fcnArrayInsert( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QVariantList list = getListValue( values.at( 0 ), parent );
  list.insert( getIntValue( values.at( 1 ), parent ), values.at( 2 ) );
  return convertToSameType( list , values.at( 0 ).type() );
}

static QVariant fcnArrayRemoveAt( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QVariantList list = getListValue( values.at( 0 ), parent );
  list.removeAt( getIntValue( values.at( 1 ), parent ) );
  return convertToSameType( list , values.at( 0 ).type() );
}

static QVariant fcnArrayRemoveAll( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QVariantList list = getListValue( values.at( 0 ), parent );
  list.removeAll( values.at( 1 ) );
  return convertToSameType( list , values.at( 0 ).type() );
}

static QVariant fcnArrayCat( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QVariantList list;
  Q_FOREACH ( const QVariant& cur, values )
  {
    list += getListValue( cur, parent );
  }
  return convertToSameType( list , values.at( 0 ).type() );
}

static QVariant fcnArrayIntersect( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  const QVariantList array1 = getListValue( values.at( 0 ), parent );
  Q_FOREACH ( const QVariant& cur, getListValue( values.at( 1 ), parent ) )
  {
    if ( array1.contains( cur ) )
      return QVariant( true );
  }
  return QVariant( false );
}


static QVariant fcnArrayDistinct( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QVariantList array = getListValue( values.at( 0 ), parent );

  QVariantList distinct;

  for ( QVariantList::const_iterator it = array.constBegin(); it != array.constEnd(); ++it )
  {
    if ( !distinct.contains( *it ) )
    {
      distinct += ( *it );
    }
  }

  return distinct;
}

static QVariant fcnArrayToString( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QVariantList array = getListValue( values.at( 0 ), parent );
  QString delimiter = getStringValue( values.at( 1 ), parent );
  QString empty = getStringValue( values.at( 2 ), parent );

  QString str;

  for ( QVariantList::const_iterator it = array.constBegin(); it != array.constEnd(); ++it )
  {
    str += ( *it ).toString().isEmpty() == false ? ( *it ).toString() : empty;
    if ( it != ( array.constEnd() - 1 ) )
    {
      str += delimiter;
    }
  }

  return QVariant( str );
}

static QVariant fcnStringToArray( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QString str = getStringValue( values.at( 0 ), parent );
  QString delimiter = getStringValue( values.at( 1 ), parent );
  QString empty = getStringValue( values.at( 2 ), parent );

  QStringList list = str.split( delimiter );
  QVariantList array;

  for ( QStringList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it )
  {
    array += ( *it ).isEmpty() == false ? *it : empty;
  }

  return array;
}

static QVariant fcnMap( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QVariantMap result;
  for ( int i = 0; i + 1 < values.length(); i += 2 )
  {
    result.insert( getStringValue( values.at( i ), parent ), values.at( i + 1 ) );
  }
  return result;
}

static QVariant fcnMapGet( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  return getMapValue( values.at( 0 ), parent ).value( values.at( 1 ).toString() );
}

static QVariant fcnMapExist( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  return getMapValue( values.at( 0 ), parent ).contains( values.at( 1 ).toString() );
}

static QVariant fcnMapDelete( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QVariantMap map = getMapValue( values.at( 0 ), parent );
  map.remove( values.at( 1 ).toString() );
  return map;
}

static QVariant fcnMapInsert( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QVariantMap map = getMapValue( values.at( 0 ), parent );
  map.insert( values.at( 1 ).toString(),  values.at( 2 ) );
  return map;
}

static QVariant fcnMapConcat( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  QVariantMap result;
  Q_FOREACH ( const QVariant& cur, values )
  {
    const QVariantMap curMap = getMapValue( cur, parent );
    for ( QVariantMap::const_iterator it = curMap.constBegin(); it != curMap.constEnd(); ++it )
      result.insert( it.key(), it.value() );
  }
  return result;
}

static QVariant fcnMapAKeys( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  return QStringList( getMapValue( values.at( 0 ), parent ).keys() );
}

static QVariant fcnMapAVals( const QVariantList& values, const QgsExpressionContext*, QgsExpression* parent )
{
  return getMapValue( values.at( 0 ), parent ).values();
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
    Functions();  // this method builds the gmBuiltinFunctions as well
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
    ParameterList aggParams = ParameterList() << Parameter( QStringLiteral( "expression" ) )
                              << Parameter( QStringLiteral( "group_by" ), true )
                              << Parameter( QStringLiteral( "filter" ), true );

    gmFunctions
    << new StaticFunction( QStringLiteral( "sqrt" ), ParameterList() << Parameter( QStringLiteral( "value" ) ), fcnSqrt, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "radians" ), ParameterList() << Parameter( QStringLiteral( "degrees" ) ), fcnRadians, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "degrees" ), ParameterList() << Parameter( QStringLiteral( "radians" ) ), fcnDegrees, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "azimuth" ), ParameterList() << Parameter( QStringLiteral( "point_a" ) ) << Parameter( QStringLiteral( "point_b" ) ), fcnAzimuth, QStringList() << QStringLiteral( "Math" ) << QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "project" ), ParameterList() << Parameter( QStringLiteral( "point" ) ) << Parameter( QStringLiteral( "distance" ) ) << Parameter( QStringLiteral( "bearing" ) ), fcnProject, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "abs" ), ParameterList() << Parameter( QStringLiteral( "value" ) ), fcnAbs, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "cos" ), ParameterList() << Parameter( QStringLiteral( "angle" ) ), fcnCos, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "sin" ), ParameterList() << Parameter( QStringLiteral( "angle" ) ), fcnSin, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "tan" ), ParameterList() << Parameter( QStringLiteral( "angle" ) ), fcnTan, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "asin" ), ParameterList() << Parameter( QStringLiteral( "value" ) ), fcnAsin, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "acos" ), ParameterList() << Parameter( QStringLiteral( "value" ) ), fcnAcos, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "atan" ), ParameterList() << Parameter( QStringLiteral( "value" ) ), fcnAtan, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "atan2" ), ParameterList() << Parameter( QStringLiteral( "dx" ) ) << Parameter( QStringLiteral( "dy" ) ), fcnAtan2, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "exp" ), ParameterList() << Parameter( QStringLiteral( "value" ) ), fcnExp, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "ln" ), ParameterList() << Parameter( QStringLiteral( "value" ) ), fcnLn, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "log10" ), ParameterList() << Parameter( QStringLiteral( "value" ) ), fcnLog10, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "log" ), ParameterList() << Parameter( QStringLiteral( "base" ) ) << Parameter( QStringLiteral( "value" ) ), fcnLog, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "round" ), ParameterList() << Parameter( QStringLiteral( "value" ) ) << Parameter( QStringLiteral( "places" ), true, 0 ), fcnRound, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "rand" ), ParameterList() << Parameter( QStringLiteral( "min" ) ) << Parameter( QStringLiteral( "max" ) ), fcnRnd, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "randf" ), ParameterList() << Parameter( QStringLiteral( "min" ), true, 0.0 ) << Parameter( QStringLiteral( "max" ), true, 1.0 ), fcnRndF, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "max" ), -1, fcnMax, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "min" ), -1, fcnMin, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "clamp" ), ParameterList() << Parameter( QStringLiteral( "min" ) ) << Parameter( QStringLiteral( "value" ) ) << Parameter( QStringLiteral( "max" ) ), fcnClamp, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "scale_linear" ), 5, fcnLinearScale, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "scale_exp" ), 6, fcnExpScale, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "floor" ), 1, fcnFloor, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "ceil" ), 1, fcnCeil, QStringLiteral( "Math" ) )
    << new StaticFunction( QStringLiteral( "pi" ), 0, fcnPi, QStringLiteral( "Math" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "$pi" ) )
    << new StaticFunction( QStringLiteral( "to_int" ), ParameterList() << Parameter( QStringLiteral( "value" ) ), fcnToInt, QStringLiteral( "Conversions" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "toint" ) )
    << new StaticFunction( QStringLiteral( "to_real" ), ParameterList() << Parameter( QStringLiteral( "value" ) ), fcnToReal, QStringLiteral( "Conversions" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "toreal" ) )
    << new StaticFunction( QStringLiteral( "to_string" ), ParameterList() << Parameter( QStringLiteral( "value" ) ), fcnToString, QStringList() << QStringLiteral( "Conversions" ) << QStringLiteral( "String" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "tostring" ) )
    << new StaticFunction( QStringLiteral( "to_datetime" ), ParameterList() << Parameter( QStringLiteral( "value" ) ), fcnToDateTime, QStringList() << QStringLiteral( "Conversions" ) << QStringLiteral( "Date and Time" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "todatetime" ) )
    << new StaticFunction( QStringLiteral( "to_date" ), ParameterList() << Parameter( QStringLiteral( "value" ) ), fcnToDate, QStringList() << QStringLiteral( "Conversions" ) << QStringLiteral( "Date and Time" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "todate" ) )
    << new StaticFunction( QStringLiteral( "to_time" ), ParameterList() << Parameter( QStringLiteral( "value" ) ), fcnToTime, QStringList() << QStringLiteral( "Conversions" ) << QStringLiteral( "Date and Time" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "totime" ) )
    << new StaticFunction( QStringLiteral( "to_interval" ), ParameterList() << Parameter( QStringLiteral( "value" ) ), fcnToInterval, QStringList() << QStringLiteral( "Conversions" ) << QStringLiteral( "Date and Time" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "tointerval" ) )
    << new StaticFunction( QStringLiteral( "coalesce" ), -1, fcnCoalesce, QStringLiteral( "Conditionals" ), QString(), false, QSet<QString>(), false, QStringList(), true )
    << new StaticFunction( QStringLiteral( "if" ), 3, fcnIf, QStringLiteral( "Conditionals" ), QString(), False, QSet<QString>(), true )

    << new StaticFunction( QStringLiteral( "aggregate" ),
                           ParameterList()
                           << Parameter( QStringLiteral( "layer" ) )
                           << Parameter( QStringLiteral( "aggregate" ) )
                           << Parameter( QStringLiteral( "expression" ) )
                           << Parameter( QStringLiteral( "filter" ), true )
                           << Parameter( QStringLiteral( "concatenator" ), true ),
                           fcnAggregate,
                           QStringLiteral( "Aggregates" ),
                           QString(),
                           []( const QgsExpression::NodeFunction* node )
    {
      // usesGeometry callback: return true if @parent variable is referenced

      if ( !node )
        return true;

      if ( !node->args() )
        return false;

      QSet<QString> referencedVars;
      if ( node->args()->count() > 2 )
      {
        QgsExpression::Node* subExpressionNode = node->args()->at( 2 );
        referencedVars = subExpressionNode->referencedVariables();
      }

      if ( node->args()->count() > 3 )
      {
        QgsExpression::Node* filterNode = node->args()->at( 3 );
        referencedVars.unite( filterNode->referencedVariables() );
      }
      return referencedVars.contains( "parent" ) || referencedVars.contains( QString() );
    },
    []( const QgsExpression::NodeFunction* node )
    {
      // referencedColumns callback: return AllAttributes if @parent variable is referenced

      if ( !node )
        return QSet<QString>() << QgsFeatureRequest::AllAttributes;

      if ( !node->args() )
        return QSet<QString>();

      QSet<QString> referencedCols;
      QSet<QString> referencedVars;

      if ( node->args()->count() > 2 )
      {
        QgsExpression::Node* subExpressionNode = node->args()->at( 2 );
        referencedVars = subExpressionNode->referencedVariables();
        referencedCols = subExpressionNode->referencedColumns();
      }
      if ( node->args()->count() > 3 )
      {
        QgsExpression::Node* filterNode = node->args()->at( 3 );
        referencedVars = filterNode->referencedVariables();
        referencedCols.unite( filterNode->referencedColumns() );
      }

      if ( referencedVars.contains( "parent" ) || referencedVars.contains( QString() ) )
        return QSet<QString>() << QgsFeatureRequest::AllAttributes;
      else
        return referencedCols;
    },
    true
                         )

    << new StaticFunction( QStringLiteral( "relation_aggregate" ), ParameterList() << Parameter( QStringLiteral( "relation" ) ) << Parameter( QStringLiteral( "aggregate" ) ) << Parameter( QStringLiteral( "expression" ) ) << Parameter( QStringLiteral( "concatenator" ), true ),
                           fcnAggregateRelation, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>() << QgsFeatureRequest::AllAttributes, true )

    << new StaticFunction( QStringLiteral( "count" ), aggParams, fcnAggregateCount, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>(), true )
    << new StaticFunction( QStringLiteral( "count_distinct" ), aggParams, fcnAggregateCountDistinct, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>(), true )
    << new StaticFunction( QStringLiteral( "count_missing" ), aggParams, fcnAggregateCountMissing, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>(), true )
    << new StaticFunction( QStringLiteral( "minimum" ), aggParams, fcnAggregateMin, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>(), true )
    << new StaticFunction( QStringLiteral( "maximum" ), aggParams, fcnAggregateMax, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>(), true )
    << new StaticFunction( QStringLiteral( "sum" ), aggParams, fcnAggregateSum, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>(), true )
    << new StaticFunction( QStringLiteral( "mean" ), aggParams, fcnAggregateMean, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>(), true )
    << new StaticFunction( QStringLiteral( "median" ), aggParams, fcnAggregateMedian, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>(), true )
    << new StaticFunction( QStringLiteral( "stdev" ), aggParams, fcnAggregateStdev, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>(), true )
    << new StaticFunction( QStringLiteral( "range" ), aggParams, fcnAggregateRange, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>(), true )
    << new StaticFunction( QStringLiteral( "minority" ), aggParams, fcnAggregateMinority, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>(), true )
    << new StaticFunction( QStringLiteral( "majority" ), aggParams, fcnAggregateMajority, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>(), true )
    << new StaticFunction( QStringLiteral( "q1" ), aggParams, fcnAggregateQ1, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>(), true )
    << new StaticFunction( QStringLiteral( "q3" ), aggParams, fcnAggregateQ3, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>(), true )
    << new StaticFunction( QStringLiteral( "iqr" ), aggParams, fcnAggregateIQR, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>(), true )
    << new StaticFunction( QStringLiteral( "min_length" ), aggParams, fcnAggregateMinLength, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>(), true )
    << new StaticFunction( QStringLiteral( "max_length" ), aggParams, fcnAggregateMaxLength, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>(), true )
    << new StaticFunction( QStringLiteral( "collect" ), aggParams, fcnAggregateCollectGeometry, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>(), true )
    << new StaticFunction( QStringLiteral( "concatenate" ), aggParams << Parameter( QStringLiteral( "concatenator" ), true ), fcnAggregateStringConcat, QStringLiteral( "Aggregates" ), QString(), False, QSet<QString>(), true )

    << new StaticFunction( QStringLiteral( "regexp_match" ), ParameterList() << Parameter( QStringLiteral( "string" ) ) << Parameter( QStringLiteral( "regex" ) ), fcnRegexpMatch, QStringList() << QStringLiteral( "Conditionals" ) << QStringLiteral( "String" ) )
    << new StaticFunction( QStringLiteral( "regexp_matches" ), ParameterList() << Parameter( QStringLiteral( "string" ) ) << Parameter( QStringLiteral( "regex" ) ) << Parameter( QStringLiteral( "emptyvalue" ), true, "" ), fcnRegexpMatches, QStringLiteral( "Arrays" ) )

    << new StaticFunction( QStringLiteral( "now" ), 0, fcnNow, QStringLiteral( "Date and Time" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "$now" ) )
    << new StaticFunction( QStringLiteral( "age" ), 2, fcnAge, QStringLiteral( "Date and Time" ) )
    << new StaticFunction( QStringLiteral( "year" ), 1, fcnYear, QStringLiteral( "Date and Time" ) )
    << new StaticFunction( QStringLiteral( "month" ), 1, fcnMonth, QStringLiteral( "Date and Time" ) )
    << new StaticFunction( QStringLiteral( "week" ), 1, fcnWeek, QStringLiteral( "Date and Time" ) )
    << new StaticFunction( QStringLiteral( "day" ), 1, fcnDay, QStringLiteral( "Date and Time" ) )
    << new StaticFunction( QStringLiteral( "hour" ), 1, fcnHour, QStringLiteral( "Date and Time" ) )
    << new StaticFunction( QStringLiteral( "minute" ), 1, fcnMinute, QStringLiteral( "Date and Time" ) )
    << new StaticFunction( QStringLiteral( "second" ), 1, fcnSeconds, QStringLiteral( "Date and Time" ) )
    << new StaticFunction( QStringLiteral( "day_of_week" ), 1, fcnDayOfWeek, QStringLiteral( "Date and Time" ) )
    << new StaticFunction( QStringLiteral( "lower" ), 1, fcnLower, QStringLiteral( "String" ) )
    << new StaticFunction( QStringLiteral( "upper" ), 1, fcnUpper, QStringLiteral( "String" ) )
    << new StaticFunction( QStringLiteral( "title" ), 1, fcnTitle, QStringLiteral( "String" ) )
    << new StaticFunction( QStringLiteral( "trim" ), 1, fcnTrim, QStringLiteral( "String" ) )
    << new StaticFunction( QStringLiteral( "levenshtein" ), 2, fcnLevenshtein, QStringLiteral( "Fuzzy Matching" ) )
    << new StaticFunction( QStringLiteral( "longest_common_substring" ), 2, fcnLCS, QStringLiteral( "Fuzzy Matching" ) )
    << new StaticFunction( QStringLiteral( "hamming_distance" ), 2, fcnHamming, QStringLiteral( "Fuzzy Matching" ) )
    << new StaticFunction( QStringLiteral( "soundex" ), 1, fcnSoundex, QStringLiteral( "Fuzzy Matching" ) )
    << new StaticFunction( QStringLiteral( "char" ), 1, fcnChar, QStringLiteral( "String" ) )
    << new StaticFunction( QStringLiteral( "wordwrap" ), ParameterList() << Parameter( QStringLiteral( "text" ) ) << Parameter( QStringLiteral( "length" ) ) << Parameter( QStringLiteral( "delimiter" ), true, "" ), fcnWordwrap, QStringLiteral( "String" ) )
    << new StaticFunction( QStringLiteral( "length" ), ParameterList() << Parameter( QStringLiteral( "text" ), true, "" ), fcnLength, QStringList() << QStringLiteral( "String" ) << QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "replace" ), -1, fcnReplace, QStringLiteral( "String" ) )
    << new StaticFunction( QStringLiteral( "regexp_replace" ), 3, fcnRegexpReplace, QStringLiteral( "String" ) )
    << new StaticFunction( QStringLiteral( "regexp_substr" ), 2, fcnRegexpSubstr, QStringLiteral( "String" ) )
    << new StaticFunction( QStringLiteral( "substr" ), ParameterList() << Parameter( QStringLiteral( "string" ) ) << Parameter( QStringLiteral( "start " ) ) << Parameter( QStringLiteral( "length" ), true ), fcnSubstr, QStringLiteral( "String" ), QString(),
                           false, QSet< QString >(), false, QStringList(), true )
    << new StaticFunction( QStringLiteral( "concat" ), -1, fcnConcat, QStringLiteral( "String" ), QString(), false, QSet<QString>(), false, QStringList(), true )
    << new StaticFunction( QStringLiteral( "strpos" ), 2, fcnStrpos, QStringLiteral( "String" ) )
    << new StaticFunction( QStringLiteral( "left" ), 2, fcnLeft, QStringLiteral( "String" ) )
    << new StaticFunction( QStringLiteral( "right" ), 2, fcnRight, QStringLiteral( "String" ) )
    << new StaticFunction( QStringLiteral( "rpad" ), 3, fcnRPad, QStringLiteral( "String" ) )
    << new StaticFunction( QStringLiteral( "lpad" ), 3, fcnLPad, QStringLiteral( "String" ) )
    << new StaticFunction( QStringLiteral( "format" ), -1, fcnFormatString, QStringLiteral( "String" ) )
    << new StaticFunction( QStringLiteral( "format_number" ), 2, fcnFormatNumber, QStringLiteral( "String" ) )
    << new StaticFunction( QStringLiteral( "format_date" ), ParameterList() << Parameter( QStringLiteral( "date" ) ) << Parameter( QStringLiteral( "format" ) ), fcnFormatDate, QStringList() << QStringLiteral( "String" ) << QStringLiteral( "Date and Time" ) )
    << new StaticFunction( QStringLiteral( "color_rgb" ), 3, fcnColorRgb, QStringLiteral( "Color" ) )
    << new StaticFunction( QStringLiteral( "color_rgba" ), 4, fncColorRgba, QStringLiteral( "Color" ) )
    << new StaticFunction( QStringLiteral( "ramp_color" ), 2, fcnRampColor, QStringLiteral( "Color" ) )
    << new StaticFunction( QStringLiteral( "color_hsl" ), 3, fcnColorHsl, QStringLiteral( "Color" ) )
    << new StaticFunction( QStringLiteral( "color_hsla" ), 4, fncColorHsla, QStringLiteral( "Color" ) )
    << new StaticFunction( QStringLiteral( "color_hsv" ), 3, fcnColorHsv, QStringLiteral( "Color" ) )
    << new StaticFunction( QStringLiteral( "color_hsva" ), 4, fncColorHsva, QStringLiteral( "Color" ) )
    << new StaticFunction( QStringLiteral( "color_cmyk" ), 4, fcnColorCmyk, QStringLiteral( "Color" ) )
    << new StaticFunction( QStringLiteral( "color_cmyka" ), 5, fncColorCmyka, QStringLiteral( "Color" ) )
    << new StaticFunction( QStringLiteral( "color_part" ), 2, fncColorPart, QStringLiteral( "Color" ) )
    << new StaticFunction( QStringLiteral( "darker" ), 2, fncDarker, QStringLiteral( "Color" ) )
    << new StaticFunction( QStringLiteral( "lighter" ), 2, fncLighter, QStringLiteral( "Color" ) )
    << new StaticFunction( QStringLiteral( "set_color_part" ), 3, fncSetColorPart, QStringLiteral( "Color" ) )
    << new StaticFunction( QStringLiteral( "$geometry" ), 0, fcnGeometry, QStringLiteral( "GeometryGroup" ), QString(), true )
    << new StaticFunction( QStringLiteral( "$area" ), 0, fcnGeomArea, QStringLiteral( "GeometryGroup" ), QString(), true )
    << new StaticFunction( QStringLiteral( "area" ), 1, fcnArea, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "$length" ), 0, fcnGeomLength, QStringLiteral( "GeometryGroup" ), QString(), true )
    << new StaticFunction( QStringLiteral( "$perimeter" ), 0, fcnGeomPerimeter, QStringLiteral( "GeometryGroup" ), QString(), true )
    << new StaticFunction( QStringLiteral( "perimeter" ), 1, fcnPerimeter, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "$x" ), 0, fcnX, QStringLiteral( "GeometryGroup" ), QString(), true )
    << new StaticFunction( QStringLiteral( "$y" ), 0, fcnY, QStringLiteral( "GeometryGroup" ), QString(), true )
    << new StaticFunction( QStringLiteral( "x" ), 1, fcnGeomX, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "y" ), 1, fcnGeomY, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "z" ), 1, fcnGeomZ, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "m" ), 1, fcnGeomM, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "point_n" ), 2, fcnPointN, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "start_point" ), 1, fcnStartPoint, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "end_point" ), 1, fcnEndPoint, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "nodes_to_points" ), -1, fcnNodesToPoints, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "segments_to_lines" ), 1, fcnSegmentsToLines, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "make_point" ), -1, fcnMakePoint, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "make_point_m" ), 3, fcnMakePointM, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "make_line" ), -1, fcnMakeLine, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "make_polygon" ), -1, fcnMakePolygon, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "$x_at" ), 1, fcnXat, QStringLiteral( "GeometryGroup" ), QString(), true, QSet<QString>(), false, QStringList() << QStringLiteral( "xat" ) << QStringLiteral( "x_at" ) )
    << new StaticFunction( QStringLiteral( "$y_at" ), 1, fcnYat, QStringLiteral( "GeometryGroup" ), QString(), true, QSet<QString>(), false, QStringList() << QStringLiteral( "yat" ) << QStringLiteral( "y_at" ) )
    << new StaticFunction( QStringLiteral( "x_min" ), 1, fcnXMin, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "xmin" ) )
    << new StaticFunction( QStringLiteral( "x_max" ), 1, fcnXMax, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "xmax" ) )
    << new StaticFunction( QStringLiteral( "y_min" ), 1, fcnYMin, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "ymin" ) )
    << new StaticFunction( QStringLiteral( "y_max" ), 1, fcnYMax, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "ymax" ) )
    << new StaticFunction( QStringLiteral( "geom_from_wkt" ), 1, fcnGeomFromWKT, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "geomFromWKT" ) )
    << new StaticFunction( QStringLiteral( "geom_from_gml" ), 1, fcnGeomFromGML, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "geomFromGML" ) )
    << new StaticFunction( QStringLiteral( "relate" ), -1, fcnRelate, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "intersects_bbox" ), 2, fcnBbox, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "bbox" ) )
    << new StaticFunction( QStringLiteral( "disjoint" ), 2, fcnDisjoint, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "intersects" ), 2, fcnIntersects, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "touches" ), 2, fcnTouches, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "crosses" ), 2, fcnCrosses, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "contains" ), 2, fcnContains, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "overlaps" ), 2, fcnOverlaps, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "within" ), 2, fcnWithin, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "translate" ), 3, fcnTranslate, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "buffer" ), -1, fcnBuffer, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "offset_curve" ), ParameterList() << Parameter( QStringLiteral( "geometry" ) )
                           << Parameter( QStringLiteral( "distance" ) )
                           << Parameter( QStringLiteral( "segments" ), true, 8.0 )
                           << Parameter( QStringLiteral( "join" ), true, QgsGeometry::JoinStyleRound )
                           << Parameter( QStringLiteral( "mitre_limit" ), true, 2.0 ),
                           fcnOffsetCurve, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "single_sided_buffer" ), ParameterList() << Parameter( QStringLiteral( "geometry" ) )
                           << Parameter( QStringLiteral( "distance" ) )
                           << Parameter( QStringLiteral( "segments" ), true, 8.0 )
                           << Parameter( QStringLiteral( "join" ), true, QgsGeometry::JoinStyleRound )
                           << Parameter( QStringLiteral( "mitre_limit" ), true, 2.0 ),
                           fcnSingleSidedBuffer, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "extend" ), ParameterList() << Parameter( QStringLiteral( "geometry" ) )
                           << Parameter( QStringLiteral( "start_distance" ) )
                           << Parameter( QStringLiteral( "end_distance" ) ),
                           fcnExtend, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "centroid" ), 1, fcnCentroid, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "point_on_surface" ), 1, fcnPointOnSurface, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "pole_of_inaccessibility" ), ParameterList() << Parameter( QStringLiteral( "geometry" ) )
                           << Parameter( QStringLiteral( "tolerance" ) ), fcnPoleOfInaccessibility, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "reverse" ), 1, fcnReverse, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "exterior_ring" ), 1, fcnExteriorRing, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "interior_ring_n" ), 2, fcnInteriorRingN, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "geometry_n" ), 2, fcnGeometryN, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "boundary" ), ParameterList() << Parameter( QStringLiteral( "geometry" ) ), fcnBoundary, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "line_merge" ), ParameterList() << Parameter( QStringLiteral( "geometry" ) ), fcnLineMerge, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "bounds" ), 1, fcnBounds, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "simplify" ), ParameterList() << Parameter( QStringLiteral( "geometry" ) ) << Parameter( QStringLiteral( "tolerance" ) ), fcnSimplify, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "simplify_vw" ), ParameterList() << Parameter( QStringLiteral( "geometry" ) ) << Parameter( QStringLiteral( "tolerance" ) ), fcnSimplifyVW, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "smooth" ), ParameterList() << Parameter( QStringLiteral( "geometry" ) ) << Parameter( QStringLiteral( "iterations" ), true, 1 )
                           << Parameter( QStringLiteral( "offset" ), true, 0.25 )
                           << Parameter( QStringLiteral( "min_length" ), true, -1 )
                           << Parameter( QStringLiteral( "max_angle" ), true, 180 ), fcnSmooth, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "num_points" ), 1, fcnGeomNumPoints, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "num_interior_rings" ), 1, fcnGeomNumInteriorRings, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "num_rings" ), 1, fcnGeomNumRings, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "num_geometries" ), 1, fcnGeomNumGeometries, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "bounds_width" ), 1, fcnBoundsWidth, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "bounds_height" ), 1, fcnBoundsHeight, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "is_closed" ), 1, fcnIsClosed, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "convex_hull" ), 1, fcnConvexHull, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "convexHull" ) )
    << new StaticFunction( QStringLiteral( "difference" ), 2, fcnDifference, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "distance" ), 2, fcnDistance, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "intersection" ), 2, fcnIntersection, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "sym_difference" ), 2, fcnSymDifference, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "symDifference" ) )
    << new StaticFunction( QStringLiteral( "combine" ), 2, fcnCombine, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "union" ), 2, fcnCombine, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "geom_to_wkt" ), -1, fcnGeomToWKT, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "geomToWKT" ) )
    << new StaticFunction( QStringLiteral( "geometry" ), 1, fcnGetGeometry, QStringLiteral( "GeometryGroup" ), QString(), true )
    << new StaticFunction( QStringLiteral( "transform" ), 3, fcnTransformGeometry, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "extrude" ), 3, fcnExtrude, QStringLiteral( "GeometryGroup" ), QString() )
    << new StaticFunction( QStringLiteral( "order_parts" ), 3, fcnOrderParts, QStringLiteral( "GeometryGroup" ), QString() )
    << new StaticFunction( QStringLiteral( "closest_point" ), 2, fcnClosestPoint, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "shortest_line" ), 2, fcnShortestLine, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "line_interpolate_point" ), ParameterList() << Parameter( QStringLiteral( "geometry" ) )
                           << Parameter( QStringLiteral( "distance" ) ), fcnLineInterpolatePoint, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "line_interpolate_angle" ), ParameterList() << Parameter( QStringLiteral( "geometry" ) )
                           << Parameter( QStringLiteral( "distance" ) ), fcnLineInterpolateAngle, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "line_locate_point" ), ParameterList() << Parameter( QStringLiteral( "geometry" ) )
                           << Parameter( QStringLiteral( "point" ) ), fcnLineLocatePoint, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "angle_at_vertex" ), ParameterList() << Parameter( QStringLiteral( "geometry" ) )
                           << Parameter( QStringLiteral( "vertex" ) ), fcnAngleAtVertex, QStringLiteral( "GeometryGroup" ) )
    << new StaticFunction( QStringLiteral( "distance_to_vertex" ), ParameterList() << Parameter( QStringLiteral( "geometry" ) )
                           << Parameter( QStringLiteral( "vertex" ) ), fcnDistanceToVertex, QStringLiteral( "GeometryGroup" ) )


    // **Record** functions

    << new StaticFunction( QStringLiteral( "$id" ), 0, fcnFeatureId, QStringLiteral( "Record" ) )
    << new StaticFunction( QStringLiteral( "$currentfeature" ), 0, fcnFeature, QStringLiteral( "Record" ) )
    << new StaticFunction( QStringLiteral( "uuid" ), 0, fcnUuid, QStringLiteral( "Record" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "$uuid" ) )
    << new StaticFunction( QStringLiteral( "get_feature" ), 3, fcnGetFeature, QStringLiteral( "Record" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "getFeature" ) )

    << new StaticFunction(
      QStringLiteral( "is_selected" ),
      -1,
      fcnIsSelected,
      QStringLiteral( "Record" ),
      QString(),
      false,
      QSet<QString>()
    )

    << new StaticFunction(
      QStringLiteral( "num_selected" ),
      -1,
      fcnNumSelected,
      QStringLiteral( "Record" ),
      QString(),
      false,
      QSet<QString>()
    )

    // **General** functions

    << new StaticFunction( QStringLiteral( "layer_property" ), 2, fcnGetLayerProperty, QStringLiteral( "General" ) )
    << new StaticFunction( QStringLiteral( "raster_statistic" ), ParameterList() << Parameter( QStringLiteral( "layer" ) )
                           << Parameter( QStringLiteral( "band" ) )
                           << Parameter( QStringLiteral( "statistic" ) ), fcnGetRasterBandStat, QStringLiteral( "General" ) )
    << new StaticFunction( QStringLiteral( "var" ), 1, fcnGetVariable, QStringLiteral( "General" ) )

    //return all attributes string for referencedColumns - this is caught by
    // QgsFeatureRequest::setSubsetOfAttributes and causes all attributes to be fetched by the
    // feature request
    << new StaticFunction( QStringLiteral( "eval" ), 1, fcnEval, QStringLiteral( "General" ), QString(), true, QSet<QString>() << QgsFeatureRequest::AllAttributes )
    << new StaticFunction( QStringLiteral( "attribute" ), 2, fcnAttribute, QStringLiteral( "Record" ), QString(), false, QSet<QString>() << QgsFeatureRequest::AllAttributes )

    // functions for arrays
    << new StaticFunction( QStringLiteral( "array" ), -1, fcnArray, QStringLiteral( "Arrays" ) )
    << new StaticFunction( QStringLiteral( "array_length" ), 1, fcnArrayLength, QStringLiteral( "Arrays" ) )
    << new StaticFunction( QStringLiteral( "array_contains" ), ParameterList() << Parameter( QStringLiteral( "array" ) ) << Parameter( QStringLiteral( "value" ) ), fcnArrayContains, QStringLiteral( "Arrays" ) )
    << new StaticFunction( QStringLiteral( "array_find" ), ParameterList() << Parameter( QStringLiteral( "array" ) ) << Parameter( QStringLiteral( "value" ) ), fcnArrayFind, QStringLiteral( "Arrays" ) )
    << new StaticFunction( QStringLiteral( "array_get" ), ParameterList() << Parameter( QStringLiteral( "array" ) ) << Parameter( QStringLiteral( "pos" ) ), fcnArrayGet, QStringLiteral( "Arrays" ) )
    << new StaticFunction( QStringLiteral( "array_append" ), ParameterList() << Parameter( QStringLiteral( "array" ) ) << Parameter( QStringLiteral( "value" ) ), fcnArrayAppend, QStringLiteral( "Arrays" ) )
    << new StaticFunction( QStringLiteral( "array_prepend" ), ParameterList() << Parameter( QStringLiteral( "array" ) ) << Parameter( QStringLiteral( "value" ) ), fcnArrayPrepend, QStringLiteral( "Arrays" ) )
    << new StaticFunction( QStringLiteral( "array_insert" ), ParameterList() << Parameter( QStringLiteral( "array" ) ) << Parameter( QStringLiteral( "pos" ) ) << Parameter( QStringLiteral( "value" ) ), fcnArrayInsert, QStringLiteral( "Arrays" ) )
    << new StaticFunction( QStringLiteral( "array_remove_at" ), ParameterList() << Parameter( QStringLiteral( "array" ) ) << Parameter( QStringLiteral( "pos" ) ), fcnArrayRemoveAt, QStringLiteral( "Arrays" ) )
    << new StaticFunction( QStringLiteral( "array_remove_all" ), ParameterList() << Parameter( QStringLiteral( "array" ) ) << Parameter( QStringLiteral( "value" ) ), fcnArrayRemoveAll, QStringLiteral( "Arrays" ) )
    << new StaticFunction( QStringLiteral( "array_cat" ), -1, fcnArrayCat, QStringLiteral( "Arrays" ) )
    << new StaticFunction( QStringLiteral( "array_intersect" ), ParameterList() << Parameter( QStringLiteral( "array1" ) ) << Parameter( QStringLiteral( "array2" ) ), fcnArrayIntersect, QStringLiteral( "Arrays" ) )
    << new StaticFunction( QStringLiteral( "array_distinct" ), 1, fcnArrayDistinct, QStringLiteral( "Arrays" ) )
    << new StaticFunction( QStringLiteral( "array_to_string" ), ParameterList() << Parameter( QStringLiteral( "array" ) ) << Parameter( QStringLiteral( "delimiter" ), true, "," ) << Parameter( QStringLiteral( "emptyvalue" ), true, "" ), fcnArrayToString, QStringLiteral( "Arrays" ) )
    << new StaticFunction( QStringLiteral( "string_to_array" ), ParameterList() << Parameter( QStringLiteral( "string" ) ) << Parameter( QStringLiteral( "delimiter" ), true, "," ) << Parameter( QStringLiteral( "emptyvalue" ), true, "" ), fcnStringToArray, QStringLiteral( "Arrays" ) )

    //functions for maps
    << new StaticFunction( QStringLiteral( "map" ), -1, fcnMap, QStringLiteral( "Maps" ) )
    << new StaticFunction( QStringLiteral( "map_get" ), ParameterList() << Parameter( QStringLiteral( "map" ) ) << Parameter( QStringLiteral( "key" ) ), fcnMapGet, QStringLiteral( "Maps" ) )
    << new StaticFunction( QStringLiteral( "map_exist" ), ParameterList() << Parameter( QStringLiteral( "map" ) ) << Parameter( QStringLiteral( "key" ) ), fcnMapExist, QStringLiteral( "Maps" ) )
    << new StaticFunction( QStringLiteral( "map_delete" ), ParameterList() << Parameter( QStringLiteral( "map" ) ) << Parameter( QStringLiteral( "key" ) ), fcnMapDelete, QStringLiteral( "Maps" ) )
    << new StaticFunction( QStringLiteral( "map_insert" ), ParameterList() << Parameter( QStringLiteral( "map" ) ) << Parameter( QStringLiteral( "key" ) ) << Parameter( QStringLiteral( "value" ) ), fcnMapInsert, QStringLiteral( "Maps" ) )
    << new StaticFunction( QStringLiteral( "map_concat" ), -1, fcnMapConcat, QStringLiteral( "Maps" ) )
    << new StaticFunction( QStringLiteral( "map_akeys" ), ParameterList() << Parameter( QStringLiteral( "map" ) ), fcnMapAKeys, QStringLiteral( "Maps" ) )
    << new StaticFunction( QStringLiteral( "map_avals" ), ParameterList() << Parameter( QStringLiteral( "map" ) ), fcnMapAVals, QStringLiteral( "Maps" ) )
    ;

    QgsExpressionContextUtils::registerContextFunctions();

    //QgsExpression has ownership of all built-in functions
    Q_FOREACH ( QgsExpression::Function* func, gmFunctions )
    {
      gmOwnedFunctions << func;
      gmBuiltinFunctions << func->name();
      gmBuiltinFunctions.append( func->aliases() );
    }
  }
  return gmFunctions;
}

bool QgsExpression::checkExpression( const QString &text, const QgsExpressionContext *context, QString &errorMessage )
{
  QgsExpression exp( text );
  exp.prepare( context );
  errorMessage = exp.parserErrorString();
  return !exp.hasParserError();
}

void QgsExpression::setExpression( const QString& expression )
{
  detach();
  d->mRootNode = ::parseExpression( expression, d->mParserErrorString );
  d->mEvalErrorString = QString();
  d->mExp = expression;
}

QString QgsExpression::expression() const
{
  if ( !d->mExp.isNull() )
    return d->mExp;
  else
    return dump();
}

QString QgsExpression::quotedColumnRef( QString name )
{
  return QStringLiteral( "\"%1\"" ).arg( name.replace( '\"', QLatin1String( "\"\"" ) ) );
}

QString QgsExpression::quotedString( QString text )
{
  text.replace( '\'', QLatin1String( "''" ) );
  text.replace( '\\', QLatin1String( "\\\\" ) );
  text.replace( '\n', QLatin1String( "\\n" ) );
  text.replace( '\t', QLatin1String( "\\t" ) );
  return QStringLiteral( "'%1'" ).arg( text );
}

QString QgsExpression::quotedValue( const QVariant &value )
{
  return quotedValue( value, value.type() );
}

QString QgsExpression::quotedValue( const QVariant& value, QVariant::Type type )
{
  if ( value.isNull() )
    return QStringLiteral( "NULL" );

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

bool QgsExpression::operator==( const QgsExpression& other ) const
{
  if ( d == other.d || d->mExp == other.d->mExp )
    return true;
  return false;
}

bool QgsExpression::isValid() const
{
  return d->mRootNode;
}

bool QgsExpression::hasParserError() const { return !d->mParserErrorString.isNull(); }

QString QgsExpression::parserErrorString() const { return d->mParserErrorString; }

QSet<QString> QgsExpression::referencedColumns() const
{
  if ( !d->mRootNode )
    return QSet<QString>();

  return d->mRootNode->referencedColumns();
}

QSet<QString> QgsExpression::referencedVariables() const
{
  if ( !d->mRootNode )
    return QSet<QString>();

  return d->mRootNode->referencedVariables();
}

bool QgsExpression::NodeInOperator::needsGeometry() const
{
  bool needs = false;
  Q_FOREACH ( Node* n, mList->list() )
    needs |= n->needsGeometry();
  return needs;
}

QSet<int> QgsExpression::referencedAttributeIndexes( const QgsFields& fields ) const
{
  if ( !d->mRootNode )
    return QSet<int>();

  const QSet<QString> referencedFields = d->mRootNode->referencedColumns();
  QSet<int> referencedIndexes;

for ( const QString& fieldName : referencedFields )
  {
    if ( fieldName == QgsFeatureRequest::AllAttributes )
    {
      referencedIndexes = fields.allAttributesList().toSet();
      break;
    }
    referencedIndexes << fields.lookupField( fieldName );
  }

  return referencedIndexes;
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

void QgsExpression::setGeomCalculator( const QgsDistanceArea *calc )
{
  detach();
  if ( calc )
    d->mCalc = QSharedPointer<QgsDistanceArea>( new QgsDistanceArea( *calc ) );
  else
    d->mCalc.clear();
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

QString QgsExpression::dump() const
{
  if ( !d->mRootNode )
    return QString();

  return d->mRootNode->dump();
}

QgsDistanceArea* QgsExpression::geomCalculator()
{
  return d->mCalc.data();
}

QgsUnitTypes::DistanceUnit QgsExpression::distanceUnits() const
{
  return d->mDistanceUnit;
}

void QgsExpression::setDistanceUnits( QgsUnitTypes::DistanceUnit unit )
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

QString QgsExpression::replaceExpressionText( const QString &action, const QgsExpressionContext *context, const QgsDistanceArea *distanceArea )
{
  QString expr_action;

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
      exp.setGeomCalculator( distanceArea );
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

void QgsExpression::NodeList::append( QgsExpression::NamedNode* node )
{
  mList.append( node->node );
  mNameList.append( node->name.toLower() );
  mHasNamedNodes = true;
}

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
    if ( !first ) msg += QLatin1String( ", " );
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
  return QStringLiteral( "%1 %2" ).arg( UnaryOperatorText[mOp], mOperand->dump() );
}

QSet<QString> QgsExpression::NodeUnaryOperator::referencedColumns() const
{
  return mOperand->referencedColumns();
}

QSet<QString> QgsExpression::NodeUnaryOperator::referencedVariables() const
{
  return mOperand->referencedVariables();
}

QgsExpression::Node* QgsExpression::NodeUnaryOperator::clone() const
{
  return new NodeUnaryOperator( mOp, mOperand->clone() );
}

//

QVariant QgsExpression::NodeBinaryOperator::eval( QgsExpression* parent, const QgsExpressionContext* context )
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
          // manage escape % and _
          if ( esc_regexp.startsWith( '%' ) )
          {
            esc_regexp.replace( 0, 1, QStringLiteral( ".*" ) );
          }
          QRegExp rx( "[^\\\\](%)" );
          int pos = 0;
          while (( pos = rx.indexIn( esc_regexp, pos ) ) != -1 )
          {
            esc_regexp.replace( pos + 1, 1, QStringLiteral( ".*" ) );
            pos += 1;
          }
          rx.setPattern( QStringLiteral( "\\\\%" ) );
          esc_regexp.replace( rx, QStringLiteral( "%" ) );
          if ( esc_regexp.startsWith( '_' ) )
          {
            esc_regexp.replace( 0, 1, QStringLiteral( "." ) );
          }
          rx.setPattern( QStringLiteral( "[^\\\\](_)" ) );
          pos = 0;
          while (( pos = rx.indexIn( esc_regexp, pos ) ) != -1 )
          {
            esc_regexp.replace( pos + 1, 1, '.' );
            pos += 1;
          }
          rx.setPattern( QStringLiteral( "\\\\_" ) );
          esc_regexp.replace( rx, QStringLiteral( "_" ) );
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
    fmt += QLatin1String( " %2 " );
    fmt += rOp && ( rOp->precedence() <= precedence() ) ? "(%3)" : "%3";
  }
  else
  {
    fmt += lOp && ( lOp->precedence() <= precedence() ) ? "(%1)" : "%1";
    fmt += QLatin1String( " %2 " );
    fmt += rOp && ( rOp->precedence() < precedence() ) ? "(%3)" : "%3";
  }

  return fmt.arg( mOpLeft->dump(), BinaryOperatorText[mOp], rdump );
}

QSet<QString> QgsExpression::NodeBinaryOperator::referencedColumns() const
{
  return mOpLeft->referencedColumns() + mOpRight->referencedColumns();
}

QSet<QString> QgsExpression::NodeBinaryOperator::referencedVariables() const
{
  return mOpLeft->referencedVariables() + mOpRight->referencedVariables();
}

bool QgsExpression::NodeBinaryOperator::needsGeometry() const
{
  return mOpLeft->needsGeometry() || mOpRight->needsGeometry();
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
  return QStringLiteral( "%1 %2 IN (%3)" ).arg( mNode->dump(), mNotIn ? "NOT" : "", mList->dump() );
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

QgsExpression::NodeFunction::NodeFunction( int fnIndex, QgsExpression::NodeList* args )
    : mFnIndex( fnIndex )
{
  const ParameterList& functionParams = Functions()[mFnIndex]->parameters();
  if ( !args || functionParams.isEmpty() )
  {
    // no parameters, or function does not support them
    mArgs = args;
  }
  else
  {
    mArgs = new NodeList();

    int idx = 0;
    //first loop through unnamed arguments
    while ( idx < args->names().size() && args->names().at( idx ).isEmpty() )
    {
      mArgs->append( args->list().at( idx )->clone() );
      idx++;
    }

    //next copy named parameters in order expected by function
    for ( ; idx < functionParams.count(); ++idx )
    {
      int nodeIdx = args->names().indexOf( functionParams.at( idx ).name().toLower() );
      if ( nodeIdx < 0 )
      {
        //parameter not found - insert default value for parameter
        mArgs->append( new NodeLiteral( functionParams.at( idx ).defaultValue() ) );
      }
      else
      {
        mArgs->append( args->list().at( nodeIdx )->clone() );
      }
    }

    delete args;
  }
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
    return QStringLiteral( "%1%2" ).arg( fd->name(), fd->name().startsWith( '$' ) ? "" : "()" ); // special column
  else
    return QStringLiteral( "%1(%2)" ).arg( fd->name(), mArgs ? mArgs->dump() : QString() ); // function
}

QSet<QString> QgsExpression::NodeFunction::referencedColumns() const
{
  Function* fd = Functions()[mFnIndex];
  QSet<QString> functionColumns = fd->referencedColumns( this );

  if ( !mArgs )
  {
    //no referenced columns in arguments, just return function's referenced columns
    return functionColumns;
  }

  Q_FOREACH ( Node* n, mArgs->list() )
  {
    functionColumns.unite( n->referencedColumns() );
  }

  return functionColumns;
}

QSet<QString> QgsExpression::NodeFunction::referencedVariables() const
{
  Function* fd = Functions()[mFnIndex];
  if ( fd->name() == "var" )
  {
    if ( !mArgs->list().isEmpty() )
    {
      QgsExpression::NodeLiteral* var = dynamic_cast<QgsExpression::NodeLiteral*>( mArgs->list().first() );
      if ( var )
        return QSet<QString>() << var->value().toString();
    }
    return QSet<QString>() << QString();
  }
  else
  {
    QSet<QString> functionVariables = QSet<QString>();

    if ( !mArgs )
      return functionVariables;

    Q_FOREACH ( Node* n, mArgs->list() )
    {
      functionVariables.unite( n->referencedVariables() );
    }

    return functionVariables;
  }
}

bool QgsExpression::NodeFunction::needsGeometry() const
{
  bool needs = Functions()[mFnIndex]->usesGeometry( this );
  if ( mArgs )
  {
    Q_FOREACH ( Node* n, mArgs->list() )
      needs |= n->needsGeometry();
  }
  return needs;
}

QgsExpression::Node* QgsExpression::NodeFunction::clone() const
{
  return new NodeFunction( mFnIndex, mArgs ? mArgs->clone() : nullptr );
}

bool QgsExpression::NodeFunction::validateParams( int fnIndex, QgsExpression::NodeList* args, QString& error )
{
  if ( !args || !args->hasNamedNodes() )
    return true;

  const ParameterList& functionParams = Functions()[fnIndex]->parameters();
  if ( functionParams.isEmpty() )
  {
    error = QStringLiteral( "%1 does not supported named parameters" ).arg( Functions()[fnIndex]->name() );
    return false;
  }
  else
  {
    QSet< int > providedArgs;
    QSet< int > handledArgs;
    int idx = 0;
    //first loop through unnamed arguments
    while ( args->names().at( idx ).isEmpty() )
    {
      providedArgs << idx;
      handledArgs << idx;
      idx++;
    }

    //next check named parameters
    for ( ; idx < functionParams.count(); ++idx )
    {
      int nodeIdx = args->names().indexOf( functionParams.at( idx ).name().toLower() );
      if ( nodeIdx < 0 )
      {
        if ( !functionParams.at( idx ).optional() )
        {
          error = QStringLiteral( "No value specified for parameter '%1' for %2" ).arg( functionParams.at( idx ).name(), Functions()[fnIndex]->name() );
          return false;
        }
      }
      else
      {
        if ( providedArgs.contains( idx ) )
        {
          error = QStringLiteral( "Duplicate parameter specified for '%1' for %2" ).arg( functionParams.at( idx ).name(), Functions()[fnIndex]->name() );
          return false;
        }
      }
      providedArgs << idx;
      handledArgs << nodeIdx;
    }

    //last check for bad names
    idx = 0;
    Q_FOREACH ( const QString& name, args->names() )
    {
      if ( !name.isEmpty() && !functionParams.contains( name ) )
      {
        error = QStringLiteral( "Invalid parameter name '%1' for %2" ).arg( name, Functions()[fnIndex]->name() );
        return false;
      }
      if ( !name.isEmpty() && !handledArgs.contains( idx ) )
      {
        int functionIdx = functionParams.indexOf( name );
        if ( providedArgs.contains( functionIdx ) )
        {
          error = QStringLiteral( "Duplicate parameter specified for '%1' for %2" ).arg( functionParams.at( functionIdx ).name(), Functions()[fnIndex]->name() );
          return false;
        }
      }
      idx++;
    }

  }
  return true;
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
    return QStringLiteral( "NULL" );

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

QSet<QString> QgsExpression::NodeLiteral::referencedColumns() const
{
  return QSet<QString>();
}

QSet<QString> QgsExpression::NodeLiteral::referencedVariables() const
{
  return QSet<QString>();
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
      index = fields.lookupField( mName );
    }
  }

  if ( context && context->hasVariable( QgsExpressionContext::EXPR_FEATURE ) )
  {
    QgsFeature feature = context->feature();
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

  mIndex = fields.lookupField( mName );
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

QSet<QString> QgsExpression::NodeColumnRef::referencedColumns() const
{
  return QSet<QString>() << mName;
}

QSet<QString> QgsExpression::NodeColumnRef::referencedVariables() const
{
  return QSet<QString>();
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
  QString msg( QStringLiteral( "CASE" ) );
  Q_FOREACH ( WhenThen* cond, mConditions )
  {
    msg += QStringLiteral( " WHEN %1 THEN %2" ).arg( cond->mWhenExp->dump(), cond->mThenExp->dump() );
  }
  if ( mElseExp )
    msg += QStringLiteral( " ELSE %1" ).arg( mElseExp->dump() );
  msg += QStringLiteral( " END" );
  return msg;
}

QSet<QString> QgsExpression::NodeCondition::referencedColumns() const
{
  QSet<QString> lst;
  Q_FOREACH ( WhenThen* cond, mConditions )
  {
    lst += cond->mWhenExp->referencedColumns() + cond->mThenExp->referencedColumns();
  }

  if ( mElseExp )
    lst += mElseExp->referencedColumns();

  return lst;
}

QSet<QString> QgsExpression::NodeCondition::referencedVariables() const
{
  QSet<QString> lst;
  Q_FOREACH ( WhenThen* cond, mConditions )
  {
    lst += cond->mWhenExp->referencedVariables() + cond->mThenExp->referencedVariables();
  }

  if ( mElseExp )
    lst += mElseExp->referencedVariables();

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


QString QgsExpression::helpText( QString name )
{
  QgsExpression::initFunctionHelp();

  if ( !gFunctionHelpTexts.contains( name ) )
    return tr( "function help for %1 missing" ).arg( name );

  const Help &f = gFunctionHelpTexts[ name ];

  name = f.mName;
  if ( f.mType == tr( "group" ) )
    name = group( name );

  name = name.toHtmlEscaped();

  QString helpContents( QStringLiteral( "<h3>%1</h3>\n<div class=\"description\"><p>%2</p></div>" )
                        .arg( tr( "%1 %2" ).arg( f.mType, name ),
                              f.mDescription ) );

  Q_FOREACH ( const HelpVariant &v, f.mVariants )
  {
    if ( f.mVariants.size() > 1 )
    {
      helpContents += QStringLiteral( "<h3>%1</h3>\n<div class=\"description\">%2</p></div>" ).arg( v.mName, v.mDescription );
    }

    if ( f.mType != tr( "group" ) && f.mType != tr( "expression" ) )
      helpContents += QStringLiteral( "<h4>%1</h4>\n<div class=\"syntax\">\n" ).arg( tr( "Syntax" ) );

    if ( f.mType == tr( "operator" ) )
    {
      if ( v.mArguments.size() == 1 )
      {
        helpContents += QStringLiteral( "<code><span class=\"functionname\">%1</span> <span class=\"argument\">%2</span></code>" )
                        .arg( name, v.mArguments[0].mArg );
      }
      else if ( v.mArguments.size() == 2 )
      {
        helpContents += QStringLiteral( "<code><span class=\"argument\">%1</span> <span class=\"functionname\">%2</span> <span class=\"argument\">%3</span></code>" )
                        .arg( v.mArguments[0].mArg, name, v.mArguments[1].mArg );
      }
    }
    else if ( f.mType != tr( "group" ) && f.mType != tr( "expression" ) )
    {
      helpContents += QStringLiteral( "<code><span class=\"functionname\">%1</span>" ).arg( name );

      if ( f.mType == tr( "function" ) && ( f.mName[0] != '$' || !v.mArguments.isEmpty() || v.mVariableLenArguments ) )
      {
        helpContents += '(';

        QString delim;
        Q_FOREACH ( const HelpArg &a, v.mArguments )
        {
          helpContents += delim;
          delim = QStringLiteral( ", " );
          if ( !a.mDescOnly )
          {
            helpContents += QStringLiteral( "<span class=\"argument %1\">%2%3</span>" ).arg( a.mOptional ? "optional" : "", a.mArg,
                            a.mDefaultVal.isEmpty() ? QLatin1String( "" ) : '=' + a.mDefaultVal );
          }
        }

        if ( v.mVariableLenArguments )
        {
          helpContents += QLatin1String( "..." );
        }

        helpContents += ')';
      }

      helpContents += QLatin1String( "</code>" );
    }

    if ( !v.mArguments.isEmpty() )
    {
      helpContents += QStringLiteral( "<h4>%1</h4>\n<div class=\"arguments\">\n<table>" ).arg( tr( "Arguments" ) );

      Q_FOREACH ( const HelpArg &a, v.mArguments )
      {
        if ( a.mSyntaxOnly )
          continue;

        helpContents += QStringLiteral( "<tr><td class=\"argument\">%1</td><td>%2</td></tr>" ).arg( a.mArg, a.mDescription );
      }

      helpContents += QLatin1String( "</table>\n</div>\n" );
    }

    if ( !v.mExamples.isEmpty() )
    {
      helpContents += QStringLiteral( "<h4>%1</h4>\n<div class=\"examples\">\n<ul>\n" ).arg( tr( "Examples" ) );

      Q_FOREACH ( const HelpExample &e, v.mExamples )
      {
        helpContents += "<li><code>" + e.mExpression + "</code> &rarr; <code>" + e.mReturns + "</code>";

        if ( !e.mNote.isEmpty() )
          helpContents += QStringLiteral( " (%1)" ).arg( e.mNote );

        helpContents += QLatin1String( "</li>\n" );
      }

      helpContents += QLatin1String( "</ul>\n</div>\n" );
    }

    if ( !v.mNotes.isEmpty() )
    {
      helpContents += QStringLiteral( "<h4>%1</h4>\n<div class=\"notes\"><p>%2</p></div>\n" ).arg( tr( "Notes" ), v.mNotes );
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
  gVariableHelpTexts.insert( QStringLiteral( "qgis_version" ), QCoreApplication::translate( "variable_help", "Current QGIS version string." ) );
  gVariableHelpTexts.insert( QStringLiteral( "qgis_version_no" ), QCoreApplication::translate( "variable_help", "Current QGIS version number." ) );
  gVariableHelpTexts.insert( QStringLiteral( "qgis_release_name" ), QCoreApplication::translate( "variable_help", "Current QGIS release name." ) );
  gVariableHelpTexts.insert( QStringLiteral( "qgis_os_name" ), QCoreApplication::translate( "variable_help", "Operating system name, e.g., 'windows', 'linux' or 'osx'." ) );
  gVariableHelpTexts.insert( QStringLiteral( "qgis_platform" ), QCoreApplication::translate( "variable_help", "QGIS platform, e.g., 'desktop' or 'server'." ) );
  gVariableHelpTexts.insert( QStringLiteral( "user_account_name" ), QCoreApplication::translate( "variable_help", "Current user's operating system account name." ) );
  gVariableHelpTexts.insert( QStringLiteral( "user_full_name" ), QCoreApplication::translate( "variable_help", "Current user's operating system user name (if available)." ) );

  //project variables
  gVariableHelpTexts.insert( QStringLiteral( "project_title" ), QCoreApplication::translate( "variable_help", "Title of current project." ) );
  gVariableHelpTexts.insert( QStringLiteral( "project_path" ), QCoreApplication::translate( "variable_help", "Full path (including file name) of current project." ) );
  gVariableHelpTexts.insert( QStringLiteral( "project_folder" ), QCoreApplication::translate( "variable_help", "Folder for current project." ) );
  gVariableHelpTexts.insert( QStringLiteral( "project_filename" ), QCoreApplication::translate( "variable_help", "Filename of current project." ) );
  gVariableHelpTexts.insert( QStringLiteral( "project_crs" ), QCoreApplication::translate( "variable_help", "Coordinate reference system of project (e.g., 'EPSG:4326')." ) );
  gVariableHelpTexts.insert( QStringLiteral( "project_crs_definition" ), QCoreApplication::translate( "variable_help", "Coordinate reference system of project (full definition)." ) );

  //layer variables
  gVariableHelpTexts.insert( QStringLiteral( "layer_name" ), QCoreApplication::translate( "variable_help", "Name of current layer." ) );
  gVariableHelpTexts.insert( QStringLiteral( "layer_id" ), QCoreApplication::translate( "variable_help", "ID of current layer." ) );
  gVariableHelpTexts.insert( QStringLiteral( "layer" ), QCoreApplication::translate( "variable_help", "The current layer." ) );

  //composition variables
  gVariableHelpTexts.insert( QStringLiteral( "layout_numpages" ), QCoreApplication::translate( "variable_help", "Number of pages in composition." ) );
  gVariableHelpTexts.insert( QStringLiteral( "layout_page" ), QCoreApplication::translate( "variable_help", "Current page number in composition." ) );
  gVariableHelpTexts.insert( QStringLiteral( "layout_pageheight" ), QCoreApplication::translate( "variable_help", "Composition page height in mm." ) );
  gVariableHelpTexts.insert( QStringLiteral( "layout_pagewidth" ), QCoreApplication::translate( "variable_help", "Composition page width in mm." ) );
  gVariableHelpTexts.insert( QStringLiteral( "layout_dpi" ), QCoreApplication::translate( "variable_help", "Composition resolution (DPI)." ) );

  //atlas variables
  gVariableHelpTexts.insert( QStringLiteral( "atlas_totalfeatures" ), QCoreApplication::translate( "variable_help", "Total number of features in atlas." ) );
  gVariableHelpTexts.insert( QStringLiteral( "atlas_featurenumber" ), QCoreApplication::translate( "variable_help", "Current atlas feature number." ) );
  gVariableHelpTexts.insert( QStringLiteral( "atlas_filename" ), QCoreApplication::translate( "variable_help", "Current atlas file name." ) );
  gVariableHelpTexts.insert( QStringLiteral( "atlas_pagename" ), QCoreApplication::translate( "variable_help", "Current atlas page name." ) );
  gVariableHelpTexts.insert( QStringLiteral( "atlas_feature" ), QCoreApplication::translate( "variable_help", "Current atlas feature (as feature object)." ) );
  gVariableHelpTexts.insert( QStringLiteral( "atlas_featureid" ), QCoreApplication::translate( "variable_help", "Current atlas feature ID." ) );
  gVariableHelpTexts.insert( QStringLiteral( "atlas_geometry" ), QCoreApplication::translate( "variable_help", "Current atlas feature geometry." ) );

  //composer item variables
  gVariableHelpTexts.insert( QStringLiteral( "item_id" ), QCoreApplication::translate( "variable_help", "Composer item user ID (not necessarily unique)." ) );
  gVariableHelpTexts.insert( QStringLiteral( "item_uuid" ), QCoreApplication::translate( "variable_help", "Composer item unique ID." ) );
  gVariableHelpTexts.insert( QStringLiteral( "item_left" ), QCoreApplication::translate( "variable_help", "Left position of composer item (in mm)." ) );
  gVariableHelpTexts.insert( QStringLiteral( "item_top" ), QCoreApplication::translate( "variable_help", "Top position of composer item (in mm)." ) );
  gVariableHelpTexts.insert( QStringLiteral( "item_width" ), QCoreApplication::translate( "variable_help", "Width of composer item (in mm)." ) );
  gVariableHelpTexts.insert( QStringLiteral( "item_height" ), QCoreApplication::translate( "variable_help", "Height of composer item (in mm)." ) );

  //map settings item variables
  gVariableHelpTexts.insert( QStringLiteral( "map_id" ), QCoreApplication::translate( "variable_help", "ID of current map destination. This will be 'canvas' for canvas renders, and the item ID for composer map renders." ) );
  gVariableHelpTexts.insert( QStringLiteral( "map_rotation" ), QCoreApplication::translate( "variable_help", "Current rotation of map." ) );
  gVariableHelpTexts.insert( QStringLiteral( "map_scale" ), QCoreApplication::translate( "variable_help", "Current scale of map." ) );
  gVariableHelpTexts.insert( QStringLiteral( "map_extent" ), QCoreApplication::translate( "variable_help", "Geometry representing the current extent of the map." ) );
  gVariableHelpTexts.insert( QStringLiteral( "map_extent_center" ), QCoreApplication::translate( "variable_help", "Center of map." ) );
  gVariableHelpTexts.insert( QStringLiteral( "map_extent_width" ), QCoreApplication::translate( "variable_help", "Width of map." ) );
  gVariableHelpTexts.insert( QStringLiteral( "map_extent_height" ), QCoreApplication::translate( "variable_help", "Height of map." ) );

  gVariableHelpTexts.insert( QStringLiteral( "row_number" ), QCoreApplication::translate( "variable_help", "Stores the number of the current row." ) );
  gVariableHelpTexts.insert( QStringLiteral( "grid_number" ), QCoreApplication::translate( "variable_help", "Current grid annotation value." ) );
  gVariableHelpTexts.insert( QStringLiteral( "grid_axis" ), QCoreApplication::translate( "variable_help", "Current grid annotation axis (e.g., 'x' for longitude, 'y' for latitude)." ) );

  //symbol variables
  gVariableHelpTexts.insert( QStringLiteral( "geometry_part_count" ), QCoreApplication::translate( "variable_help", "Number of parts in rendered feature's geometry." ) );
  gVariableHelpTexts.insert( QStringLiteral( "geometry_part_num" ), QCoreApplication::translate( "variable_help", "Current geometry part number for feature being rendered." ) );
  gVariableHelpTexts.insert( QStringLiteral( "geometry_point_count" ), QCoreApplication::translate( "variable_help", "Number of points in the rendered geometry's part. It is only meaningful for line geometries and for symbol layers that set this variable." ) );
  gVariableHelpTexts.insert( QStringLiteral( "geometry_point_num" ), QCoreApplication::translate( "variable_help", "Current point number in the rendered geometry's part. It is only meaningful for line geometries and for symbol layers that set this variable." ) );

  gVariableHelpTexts.insert( QStringLiteral( "symbol_color" ), QCoreApplication::translate( "symbol_color", "Color of symbol used to render the feature." ) );
  gVariableHelpTexts.insert( QStringLiteral( "symbol_angle" ), QCoreApplication::translate( "symbol_angle", "Angle of symbol used to render the feature (valid for marker symbols only)." ) );

  //cluster variables
  gVariableHelpTexts.insert( QStringLiteral( "cluster_color" ), QCoreApplication::translate( "cluster_color", "Color of symbols within a cluster, or NULL if symbols have mixed colors." ) );
  gVariableHelpTexts.insert( QStringLiteral( "cluster_size" ), QCoreApplication::translate( "cluster_size", "Number of symbols contained within a cluster." ) );
}

QString QgsExpression::variableHelpText( const QString &variableName, bool showValue, const QVariant &value )
{
  QgsExpression::initVariableHelp();
  QString text = gVariableHelpTexts.contains( variableName ) ? QStringLiteral( "<p>%1</p>" ).arg( gVariableHelpTexts.value( variableName ) ) : QString();
  if ( showValue )
  {
    QString valueString;
    if ( !value.isValid() )
    {
      valueString = QCoreApplication::translate( "variable_help", "not set" );
    }
    else
    {
      valueString = QStringLiteral( "<b>%1</b>" ).arg( formatPreviewString( value ) );
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
    gGroups.insert( QStringLiteral( "General" ), tr( "General" ) );
    gGroups.insert( QStringLiteral( "Operators" ), tr( "Operators" ) );
    gGroups.insert( QStringLiteral( "Conditionals" ), tr( "Conditionals" ) );
    gGroups.insert( QStringLiteral( "Fields and Values" ), tr( "Fields and Values" ) );
    gGroups.insert( QStringLiteral( "Math" ), tr( "Math" ) );
    gGroups.insert( QStringLiteral( "Conversions" ), tr( "Conversions" ) );
    gGroups.insert( QStringLiteral( "Date and Time" ), tr( "Date and Time" ) );
    gGroups.insert( QStringLiteral( "String" ), tr( "String" ) );
    gGroups.insert( QStringLiteral( "Color" ), tr( "Color" ) );
    gGroups.insert( QStringLiteral( "GeometryGroup" ), tr( "Geometry" ) );
    gGroups.insert( QStringLiteral( "Record" ), tr( "Record" ) );
    gGroups.insert( QStringLiteral( "Variables" ), tr( "Variables" ) );
    gGroups.insert( QStringLiteral( "Fuzzy Matching" ), tr( "Fuzzy Matching" ) );
    gGroups.insert( QStringLiteral( "Recent (%1)" ), tr( "Recent (%1)" ) );
  }

  //return the translated name for this group. If group does not
  //have a translated name in the gGroups hash, return the name
  //unchanged
  return gGroups.value( name, name );
}

QString QgsExpression::formatPreviewString( const QVariant& value )
{
  static const int MAX_PREVIEW = 60;

  if ( value.canConvert<QgsGeometry>() )
  {
    //result is a geometry
    QgsGeometry geom = value.value<QgsGeometry>();
    if ( geom.isEmpty() )
      return tr( "<i>&lt;empty geometry&gt;</i>" );
    else
      return tr( "<i>&lt;geometry: %1&gt;</i>" ).arg( QgsWkbTypes::displayString( geom.geometry()->wkbType() ) );
  }
  else if ( !value.isValid() )
  {
    return tr( "<i>NULL</i>" );
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
    return tr( "<i>&lt;date: %1&gt;</i>" ).arg( dt.toString( QStringLiteral( "yyyy-MM-dd" ) ) );
  }
  else if ( value.type() == QVariant::Time )
  {
    QTime tm = value.toTime();
    return tr( "<i>&lt;time: %1&gt;</i>" ).arg( tm.toString( QStringLiteral( "hh:mm:ss" ) ) );
  }
  else if ( value.type() == QVariant::DateTime )
  {
    QDateTime dt = value.toDateTime();
    return tr( "<i>&lt;datetime: %1&gt;</i>" ).arg( dt.toString( QStringLiteral( "yyyy-MM-dd hh:mm:ss" ) ) );
  }
  else if ( value.type() == QVariant::String )
  {
    QString previewString = value.toString();
    if ( previewString.length() > MAX_PREVIEW + 3 )
    {
      return QString( tr( "'%1...'" ) ).arg( previewString.left( MAX_PREVIEW ) );
    }
    else
    {
      return previewString.prepend( '\'' ).append( '\'' );
    }
  }
  else if ( value.type() == QVariant::Map )
  {
    QString mapStr;
    const QVariantMap map = value.toMap();
    for ( QVariantMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it )
    {
      if ( !mapStr.isEmpty() ) mapStr.append( ", " );
      mapStr.append( it.key() ).append( ": " ).append( formatPreviewString( it.value() ) );
      if ( mapStr.length() > MAX_PREVIEW + 3 )
      {
        mapStr = QString( tr( "%1..." ) ).arg( mapStr.left( MAX_PREVIEW ) );
        break;
      }
    }
    return tr( "<i>&lt;map: %1&gt;</i>" ).arg( mapStr );
  }
  else if ( value.type() == QVariant::List || value.type() == QVariant::StringList )
  {
    QString listStr;
    const QVariantList list = value.toList();
    for ( QVariantList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it )
    {
      if ( !listStr.isEmpty() ) listStr.append( ", " );
      listStr.append( formatPreviewString( *it ) );
      if ( listStr.length() > MAX_PREVIEW + 3 )
      {
        listStr = QString( tr( "%1..." ) ).arg( listStr.left( MAX_PREVIEW ) );
        break;
      }
    }
    return tr( "<i>&lt;array: %1&gt;</i>" ).arg( listStr );
  }
  else
  {
    return value.toString();
  }
}

const QgsExpression::Node* QgsExpression::rootNode() const
{
  return d->mRootNode;
}

QSet<QString> QgsExpression::NodeInOperator::referencedColumns() const
{
  QSet<QString> lst( mNode->referencedColumns() );
  Q_FOREACH ( const Node* n, mList->list() )
    lst.unite( n->referencedColumns() );
  return lst;
}

QSet<QString> QgsExpression::NodeInOperator::referencedVariables() const
{
  QSet<QString> lst( mNode->referencedVariables() );
  Q_FOREACH ( const Node* n, mList->list() )
    lst.unite( n->referencedVariables() );
  return lst;
}

bool QgsExpression::Function::usesGeometry( const QgsExpression::NodeFunction* node ) const
{
  Q_UNUSED( node )
  return true;
}

QSet<QString> QgsExpression::Function::referencedColumns( const NodeFunction* node ) const
{
  Q_UNUSED( node )
  return QSet<QString>() << QgsFeatureRequest::AllAttributes;
}

bool QgsExpression::Function::operator==( const QgsExpression::Function& other ) const
{
  if ( QString::compare( mName, other.mName, Qt::CaseInsensitive ) == 0 )
    return true;

  return false;
}

QgsExpression::StaticFunction::StaticFunction( const QString& fnname, const QgsExpression::ParameterList& params, QgsExpression::FcnEval fcn, const QString& group, const QString& helpText, std::function < bool ( const NodeFunction* node ) > usesGeometry, std::function < QSet<QString>( const NodeFunction* node ) > referencedColumns, bool lazyEval, const QStringList& aliases, bool handlesNull )
    : Function( fnname, params, group, helpText, lazyEval, handlesNull )
    , mFnc( fcn )
    , mAliases( aliases )
    , mUsesGeometry( false )
    , mUsesGeometryFunc( usesGeometry )
    , mReferencedColumnsFunc( referencedColumns )
{
}

bool QgsExpression::StaticFunction::usesGeometry( const NodeFunction* node ) const
{
  if ( mUsesGeometryFunc )
    return mUsesGeometryFunc( node );
  else
    return mUsesGeometry;
}

QSet<QString> QgsExpression::StaticFunction::referencedColumns( const NodeFunction* node ) const
{
  if ( mReferencedColumnsFunc )
    return mReferencedColumnsFunc( node );
  else
    return mReferencedColumns;
}
