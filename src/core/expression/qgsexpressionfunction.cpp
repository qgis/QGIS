/***************************************************************************
                               qgsexpressionfunction.cpp
                             -------------------
    begin                : May 2017
    copyright            : (C) 2017 Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsexpressionfunction.h"

#include <random>
#include <sqlite3.h>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgscolorramp.h"
#include "qgscolorrampimpl.h"
#include "qgscoordinateformatter.h"
#include "qgscoordinateutils.h"
#include "qgscurve.h"
#include "qgscurvepolygon.h"
#include "qgsdistancearea.h"
#include "qgsexception.h"
#include "qgsexiftools.h"
#include "qgsexpressioncontextutils.h"
#include "qgsexpressionnodeimpl.h"
#include "qgsexpressionsorter_p.h"
#include "qgsexpressionutils.h"
#include "qgsfeaturerequest.h"
#include "qgsfieldformatter.h"
#include "qgsfieldformatterregistry.h"
#include "qgsgeometryengine.h"
#include "qgsgeometryutils.h"
#include "qgsgeos.h"
#include "qgshstoreutils.h"
#include "qgslinestring.h"
#include "qgsmaptopixelgeometrysimplifier.h"
#include "qgsmessagelog.h"
#include "qgsmultilinestring.h"
#include "qgsmultipoint.h"
#include "qgsogcutils.h"
#include "qgspolygon.h"
#include "qgsproviderregistry.h"
#include "qgsquadrilateral.h"
#include "qgsrasterbandstats.h"
#include "qgsrasterlayer.h"
#include "qgsregularpolygon.h"
#include "qgsspatialindex.h"
#include "qgsstringutils.h"
#include "qgsstyle.h"
#include "qgssymbollayerutils.h"
#include "qgsthreadingutils.h"
#include "qgstransaction.h"
#include "qgstriangle.h"
#include "qgsunittypes.h"
#include "qgsvariantutils.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsvectorlayerutils.h"

#include <QCryptographicHash>
#include <QMimeDatabase>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QUrlQuery>
#include <QUuid>

typedef QList<QgsExpressionFunction *> ExpressionFunctionList;

Q_GLOBAL_STATIC( ExpressionFunctionList, sOwnedFunctions )
Q_GLOBAL_STATIC( QStringList, sBuiltinFunctions )
Q_GLOBAL_STATIC( ExpressionFunctionList, sFunctions )

Q_DECLARE_METATYPE( QgsSpatialIndex )
Q_DECLARE_METATYPE( QgsExpressionContext )
Q_DECLARE_METATYPE( std::shared_ptr<QgsVectorLayer> )

const QString QgsExpressionFunction::helpText() const
{
  return mHelpText.isEmpty() ? QgsExpression::helpText( mName ) : mHelpText;
}

QVariant QgsExpressionFunction::run( QgsExpressionNode::NodeList *args, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node )
{
  Q_UNUSED( node )
  // evaluate arguments
  QVariantList argValues;
  if ( args )
  {
    int arg = 0;
    const QList< QgsExpressionNode * > argList = args->list();
    argValues.reserve( argList.size() );
    for ( QgsExpressionNode *n : argList )
    {
      QVariant v;
      if ( lazyEval() )
      {
        // Pass in the node for the function to eval as it needs.
        v = QVariant::fromValue( n );
      }
      else
      {
        v = n->eval( parent, context );
        ENSURE_NO_EVAL_ERROR
        bool defaultParamIsNull = mParameterList.count() > arg && mParameterList.at( arg ).optional() && !mParameterList.at( arg ).defaultValue().isValid();
        if ( QgsExpressionUtils::isNull( v ) && !defaultParamIsNull && !handlesNull() )
          return QVariant(); // all "normal" functions return NULL, when any QgsExpressionFunction::Parameter is NULL (so coalesce is abnormal)
      }
      argValues.append( v );
      arg++;
    }
  }

  return func( argValues, context, parent, node );
}

bool QgsExpressionFunction::usesGeometry( const QgsExpressionNodeFunction *node ) const
{
  Q_UNUSED( node )
  return true;
}

QStringList QgsExpressionFunction::aliases() const
{
  return QStringList();
}

bool QgsExpressionFunction::isStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const
{
  Q_UNUSED( parent )
  Q_UNUSED( context )
  Q_UNUSED( node )
  return false;
}

bool QgsExpressionFunction::prepare( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const
{
  Q_UNUSED( parent )
  Q_UNUSED( context )
  Q_UNUSED( node )
  return true;
}

QSet<QString> QgsExpressionFunction::referencedColumns( const QgsExpressionNodeFunction *node ) const
{
  Q_UNUSED( node )
  return QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES;
}

bool QgsExpressionFunction::isDeprecated() const
{
  return mGroups.isEmpty() ? false : mGroups.contains( u"deprecated"_s );
}

bool QgsExpressionFunction::operator==( const QgsExpressionFunction &other ) const
{
  return ( QString::compare( mName, other.mName, Qt::CaseInsensitive ) == 0 );
}

bool QgsExpressionFunction::handlesNull() const
{
  return mHandlesNull;
}

// doxygen doesn't like this constructor for some reason (maybe the function arguments?)
///@cond PRIVATE
QgsStaticExpressionFunction::QgsStaticExpressionFunction( const QString &fnname, const QgsExpressionFunction::ParameterList &params,
    FcnEval fcn,
    const QString &group,
    const QString &helpText,
    const std::function < bool ( const QgsExpressionNodeFunction *node ) > &usesGeometry,
    const std::function < QSet<QString>( const QgsExpressionNodeFunction *node ) > &referencedColumns,
    bool lazyEval,
    const QStringList &aliases,
    bool handlesNull )
  : QgsExpressionFunction( fnname, params, group, helpText, lazyEval, handlesNull, false )
  , mFnc( fcn )
  , mAliases( aliases )
  , mUsesGeometry( false )
  , mUsesGeometryFunc( usesGeometry )
  , mReferencedColumnsFunc( referencedColumns )
{
}
///@endcond

QStringList QgsStaticExpressionFunction::aliases() const
{
  return mAliases;
}

bool QgsStaticExpressionFunction::usesGeometry( const QgsExpressionNodeFunction *node ) const
{
  if ( mUsesGeometryFunc )
    return mUsesGeometryFunc( node );
  else
    return mUsesGeometry;
}

void QgsStaticExpressionFunction::setUsesGeometryFunction( const std::function<bool ( const QgsExpressionNodeFunction * )> &usesGeometry )
{
  mUsesGeometryFunc = usesGeometry;
}

QSet<QString> QgsStaticExpressionFunction::referencedColumns( const QgsExpressionNodeFunction *node ) const
{
  if ( mReferencedColumnsFunc )
    return mReferencedColumnsFunc( node );
  else
    return mReferencedColumns;
}

bool QgsStaticExpressionFunction::isStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const
{
  if ( mIsStaticFunc )
    return mIsStaticFunc( node, parent, context );
  else
    return mIsStatic;
}

bool QgsStaticExpressionFunction::prepare( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const
{
  if ( mPrepareFunc )
    return mPrepareFunc( node, parent, context );

  return true;
}

void QgsStaticExpressionFunction::setIsStaticFunction( const std::function<bool ( const QgsExpressionNodeFunction *, QgsExpression *, const QgsExpressionContext * )> &isStatic )
{
  mIsStaticFunc = isStatic;
}

void QgsStaticExpressionFunction::setIsStatic( bool isStatic )
{
  mIsStaticFunc = nullptr;
  mIsStatic = isStatic;
}

void QgsStaticExpressionFunction::setPrepareFunction( const std::function<bool ( const QgsExpressionNodeFunction *, QgsExpression *, const QgsExpressionContext * )> &prepareFunc )
{
  mPrepareFunc = prepareFunc;
}

bool QgsExpressionFunction::allParamsStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context )
{
  if ( node && node->args() )
  {
    const QList< QgsExpressionNode * > argList = node->args()->list();
    for ( QgsExpressionNode *argNode : argList )
    {
      if ( !argNode->isStatic( parent, context ) )
        return false;
    }
  }

  return true;
}

static QVariant fcnGenerateSeries( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double start = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  double stop = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  double step = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );

  if ( step == 0.0  || ( step > 0.0 && start > stop ) || ( step < 0.0 && start < stop ) )
    return QVariant();

  QVariantList array;
  int length = 1;

  array << start;
  double current = start + step;
  while ( ( ( step > 0.0 && current <= stop ) || ( step < 0.0 && current >= stop ) ) && length <= 1000000 )
  {
    array << current;
    current += step;
    length++;
  }

  return array;
}

static QVariant fcnGetVariable( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( !context )
    return QVariant();

  const QString name = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );

  if ( name == "feature"_L1 )
  {
    return context->hasFeature() ? QVariant::fromValue( context->feature() ) : QVariant();
  }
  else if ( name == "id"_L1 )
  {
    return context->hasFeature() ? QVariant::fromValue( context->feature().id() ) : QVariant();
  }
  else if ( name == "geometry"_L1 )
  {
    if ( !context->hasFeature() )
      return QVariant();

    const QgsFeature feature = context->feature();
    return feature.hasGeometry() ? QVariant::fromValue( feature.geometry() ) : QVariant();
  }
  else
  {
    return context->variable( name );
  }
}

static QVariant fcnEvalTemplate( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString templateString = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  return QgsExpression::replaceExpressionText( templateString, context );
}

static QVariant fcnEval( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( !context )
    return QVariant();

  QString expString = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  QgsExpression expression( expString );
  return expression.evaluate( context );
}

static QVariant fcnSqrt( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double x = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  return QVariant( std::sqrt( x ) );
}

static QVariant fcnAbs( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double val = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  return QVariant( std::fabs( val ) );
}

static QVariant fcnRadians( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double deg = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  return ( deg * M_PI ) / 180;
}
static QVariant fcnDegrees( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double rad = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  return ( 180 * rad ) / M_PI;
}
static QVariant fcnSin( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double x = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  return QVariant( std::sin( x ) );
}
static QVariant fcnCos( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double x = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  return QVariant( std::cos( x ) );
}
static QVariant fcnTan( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double x = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  return QVariant( std::tan( x ) );
}
static QVariant fcnAsin( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double x = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  return QVariant( std::asin( x ) );
}
static QVariant fcnAcos( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double x = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  return QVariant( std::acos( x ) );
}
static QVariant fcnAtan( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double x = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  return QVariant( std::atan( x ) );
}
static QVariant fcnAtan2( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double y = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  double x = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  return QVariant( std::atan2( y, x ) );
}
static QVariant fcnExp( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double x = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  return QVariant( std::exp( x ) );
}
static QVariant fcnLn( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double x = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  if ( x <= 0 )
    return QVariant();
  return QVariant( std::log( x ) );
}
static QVariant fcnLog10( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double x = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  if ( x <= 0 )
    return QVariant();
  return QVariant( log10( x ) );
}
static QVariant fcnLog( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double b = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  double x = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  if ( x <= 0 || b <= 0 )
    return QVariant();
  return QVariant( std::log( x ) / std::log( b ) );
}
static QVariant fcnRndF( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double min = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  double max = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  if ( max < min )
    return QVariant();

  std::random_device rd;
  std::mt19937_64 generator( rd() );

  if ( !QgsExpressionUtils::isNull( values.at( 2 ) ) )
  {
    quint32 seed;
    if ( QgsExpressionUtils::isIntSafe( values.at( 2 ) ) )
    {
      // if seed can be converted to int, we use as is
      seed = QgsExpressionUtils::getIntValue( values.at( 2 ), parent );
    }
    else
    {
      // if not, we hash string representation to int
      QString seedStr = QgsExpressionUtils::getStringValue( values.at( 2 ), parent );
      std::hash<std::string> hasher;
      seed = hasher( seedStr.toStdString() );
    }
    generator.seed( seed );
  }

  // Return a random double in the range [min, max] (inclusive)
  double f = static_cast< double >( generator() ) / static_cast< double >( std::mt19937_64::max() );
  return QVariant( min + f * ( max - min ) );
}
static QVariant fcnRnd( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  qlonglong min = QgsExpressionUtils::getIntValue( values.at( 0 ), parent );
  qlonglong max = QgsExpressionUtils::getIntValue( values.at( 1 ), parent );
  if ( max < min )
    return QVariant();

  std::random_device rd;
  std::mt19937_64 generator( rd() );

  if ( !QgsExpressionUtils::isNull( values.at( 2 ) ) )
  {
    quint32 seed;
    if ( QgsExpressionUtils::isIntSafe( values.at( 2 ) ) )
    {
      // if seed can be converted to int, we use as is
      seed = QgsExpressionUtils::getIntValue( values.at( 2 ), parent );
    }
    else
    {
      // if not, we hash string representation to int
      QString seedStr = QgsExpressionUtils::getStringValue( values.at( 2 ), parent );
      std::hash<std::string> hasher;
      seed = hasher( seedStr.toStdString() );
    }
    generator.seed( seed );
  }

  qint64 randomInteger = min + ( generator() % ( max - min + 1 ) );
  if ( randomInteger  > std::numeric_limits<int>::max() || randomInteger < -std::numeric_limits<int>::max() )
    return QVariant( randomInteger );

  // Prevent wrong conversion of QVariant. See #36412
  return QVariant( int( randomInteger ) );
}

static QVariant fcnLinearScale( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double val = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  double domainMin = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  double domainMax = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
  double rangeMin = QgsExpressionUtils::getDoubleValue( values.at( 3 ), parent );
  double rangeMax = QgsExpressionUtils::getDoubleValue( values.at( 4 ), parent );

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

static QVariant fcnPolynomialScale( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double val = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  double domainMin = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  double domainMax = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
  double rangeMin = QgsExpressionUtils::getDoubleValue( values.at( 3 ), parent );
  double rangeMax = QgsExpressionUtils::getDoubleValue( values.at( 4 ), parent );
  double exponent = QgsExpressionUtils::getDoubleValue( values.at( 5 ), parent );

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

  // Return polynomially scaled value
  return QVariant( ( ( rangeMax - rangeMin ) / std::pow( domainMax - domainMin, exponent ) ) * std::pow( val - domainMin, exponent ) + rangeMin );
}

static QVariant fcnExponentialScale( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double val = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  double domainMin = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  double domainMax = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
  double rangeMin = QgsExpressionUtils::getDoubleValue( values.at( 3 ), parent );
  double rangeMax = QgsExpressionUtils::getDoubleValue( values.at( 4 ), parent );
  double exponent = QgsExpressionUtils::getDoubleValue( values.at( 5 ), parent );

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
  double ratio = ( std::pow( exponent, val - domainMin ) - 1 ) / ( std::pow( exponent, domainMax - domainMin ) - 1 );
  return QVariant( ( rangeMax - rangeMin ) * ratio + rangeMin );
}

static QVariant fcnMax( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariant result = QgsVariantUtils::createNullVariant( QMetaType::Type::Double );
  double maxVal = std::numeric_limits<double>::quiet_NaN();
  for ( const QVariant &val : values )
  {
    double testVal = QgsVariantUtils::isNull( val ) ? std::numeric_limits<double>::quiet_NaN() : QgsExpressionUtils::getDoubleValue( val, parent );
    if ( std::isnan( maxVal ) )
    {
      maxVal = testVal;
    }
    else if ( !std::isnan( testVal ) )
    {
      maxVal = std::max( maxVal, testVal );
    }
  }

  if ( !std::isnan( maxVal ) )
  {
    result = QVariant( maxVal );
  }
  return result;
}

static QVariant fcnMin( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariant result = QgsVariantUtils::createNullVariant( QMetaType::Type::Double );
  double minVal = std::numeric_limits<double>::quiet_NaN();
  for ( const QVariant &val : values )
  {
    double testVal = QgsVariantUtils::isNull( val ) ? std::numeric_limits<double>::quiet_NaN() : QgsExpressionUtils::getDoubleValue( val, parent );
    if ( std::isnan( minVal ) )
    {
      minVal = testVal;
    }
    else if ( !std::isnan( testVal ) )
    {
      minVal = std::min( minVal, testVal );
    }
  }

  if ( !std::isnan( minVal ) )
  {
    result = QVariant( minVal );
  }
  return result;
}

static QVariant fcnAggregate( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  //lazy eval, so we need to evaluate nodes now

  //first node is layer id or name
  QgsExpressionNode *node = QgsExpressionUtils::getNode( values.at( 0 ), parent );
  ENSURE_NO_EVAL_ERROR
  QVariant value = node->eval( parent, context );
  ENSURE_NO_EVAL_ERROR

  // TODO this expression function is NOT thread safe
  Q_NOWARN_DEPRECATED_PUSH
  QgsVectorLayer *vl = QgsExpressionUtils::getVectorLayer( value, context, parent );
  Q_NOWARN_DEPRECATED_POP
  if ( !vl )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot find layer with name or ID '%1'" ).arg( value.toString() ) );
    return QVariant();
  }

  // second node is aggregate type
  node = QgsExpressionUtils::getNode( values.at( 1 ), parent );
  ENSURE_NO_EVAL_ERROR
  value = node->eval( parent, context );
  ENSURE_NO_EVAL_ERROR
  bool ok = false;
  Qgis::Aggregate aggregate = QgsAggregateCalculator::stringToAggregate( QgsExpressionUtils::getStringValue( value, parent ), &ok );
  if ( !ok )
  {
    parent->setEvalErrorString( QObject::tr( "No such aggregate '%1'" ).arg( value.toString() ) );
    return QVariant();
  }

  // third node is subexpression (or field name)
  node = QgsExpressionUtils::getNode( values.at( 2 ), parent );
  ENSURE_NO_EVAL_ERROR
  QString subExpression = node->dump();

  QgsAggregateCalculator::AggregateParameters parameters;
  //optional forth node is filter
  if ( values.count() > 3 )
  {
    node = QgsExpressionUtils::getNode( values.at( 3 ), parent );
    ENSURE_NO_EVAL_ERROR
    QgsExpressionNodeLiteral *nl = dynamic_cast< QgsExpressionNodeLiteral * >( node );
    if ( !nl || nl->value().isValid() )
      parameters.filter = node->dump();
  }

  //optional fifth node is concatenator
  if ( values.count() > 4 )
  {
    node = QgsExpressionUtils::getNode( values.at( 4 ), parent );
    ENSURE_NO_EVAL_ERROR
    value = node->eval( parent, context );
    ENSURE_NO_EVAL_ERROR
    parameters.delimiter = value.toString();
  }

  //optional sixth node is order by
  QString orderBy;
  if ( values.count() > 5 )
  {
    node = QgsExpressionUtils::getNode( values.at( 5 ), parent );
    ENSURE_NO_EVAL_ERROR
    QgsExpressionNodeLiteral *nl = dynamic_cast< QgsExpressionNodeLiteral * >( node );
    if ( !nl || nl->value().isValid() )
    {
      orderBy = node->dump();
      parameters.orderBy << QgsFeatureRequest::OrderByClause( orderBy );
    }
  }

  QString aggregateError;
  QVariant result;
  if ( context )
  {
    QString cacheKey;
    QgsExpression subExp( subExpression );
    QgsExpression filterExp( parameters.filter );

    const QSet< QString > filterVars = filterExp.referencedVariables();
    const QSet< QString > subExpVars = subExp.referencedVariables();
    QSet<QString> allVars = filterVars + subExpVars;

    bool isStatic = true;
    if ( filterVars.contains( u"parent"_s )
         || filterVars.contains( QString() )
         || subExpVars.contains( u"parent"_s )
         || subExpVars.contains( QString() ) )
    {
      isStatic = false;
    }
    else
    {
      for ( const QString &varName : allVars )
      {
        const QgsExpressionContextScope *scope = context->activeScopeForVariable( varName );
        if ( scope && !scope->isStatic( varName ) )
        {
          isStatic = false;
          break;
        }
      }
    }

    if ( isStatic && ! parameters.orderBy.isEmpty() )
    {
      for ( const auto &orderByClause : std::as_const( parameters.orderBy ) )
      {
        const QgsExpression &orderByExpression { orderByClause.expression() };
        if ( orderByExpression.referencedVariables().contains( u"parent"_s ) || orderByExpression.referencedVariables().contains( QString() ) )
        {
          isStatic = false;
          break;
        }
      }
    }

    if ( !isStatic )
    {
      bool ok = false;
      const QString contextHash = context->uniqueHash( ok, allVars );
      if ( ok )
      {
        cacheKey = u"aggfcn:%1:%2:%3:%4:%5:%6"_s.arg( vl->id(), QString::number( static_cast< int >( aggregate ) ), subExpression, parameters.filter,
                   orderBy, contextHash );
      }
    }
    else
    {
      cacheKey = u"aggfcn:%1:%2:%3:%4:%5"_s.arg( vl->id(), QString::number( static_cast< int >( aggregate ) ), subExpression, parameters.filter, orderBy );
    }

    if ( !cacheKey.isEmpty() && context->hasCachedValue( cacheKey ) )
    {
      return context->cachedValue( cacheKey );
    }

    QgsExpressionContext subContext( *context );
    QgsExpressionContextScope *subScope = new QgsExpressionContextScope();
    subScope->setVariable( u"parent"_s, context->feature(), true );
    subContext.appendScope( subScope );
    result = vl->aggregate( aggregate, subExpression, parameters, &subContext, &ok, nullptr, context->feedback(), &aggregateError );

    if ( ok && !cacheKey.isEmpty() )
    {
      // important -- we should only store cached values when the expression is successfully calculated. Otherwise subsequent
      // use of the expression context will happily grab the invalid QVariant cached value without realising that there was actually an error
      // associated with it's calculation!
      context->setCachedValue( cacheKey, result );
    }
  }
  else
  {
    result = vl->aggregate( aggregate, subExpression, parameters, nullptr, &ok, nullptr, nullptr, &aggregateError );
  }
  if ( !ok )
  {
    if ( !aggregateError.isEmpty() )
      parent->setEvalErrorString( QObject::tr( "Could not calculate aggregate for: %1 (%2)" ).arg( subExpression, aggregateError ) );
    else
      parent->setEvalErrorString( QObject::tr( "Could not calculate aggregate for: %1" ).arg( subExpression ) );
    return QVariant();
  }

  return result;
}

static QVariant fcnAggregateRelation( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( !context )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot use relation aggregate function in this context" ) );
    return QVariant();
  }

  // first step - find current layer

  // TODO this expression function is NOT thread safe
  Q_NOWARN_DEPRECATED_PUSH
  QgsVectorLayer *vl = QgsExpressionUtils::getVectorLayer( context->variable( u"layer"_s ), context, parent );
  Q_NOWARN_DEPRECATED_POP
  if ( !vl )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot use relation aggregate function in this context" ) );
    return QVariant();
  }

  //lazy eval, so we need to evaluate nodes now

  //first node is relation name
  QgsExpressionNode *node = QgsExpressionUtils::getNode( values.at( 0 ), parent );
  ENSURE_NO_EVAL_ERROR
  QVariant value = node->eval( parent, context );
  ENSURE_NO_EVAL_ERROR
  QString relationId = value.toString();
  // check relation exists
  QgsRelation relation = QgsProject::instance()->relationManager()->relation( relationId ); // skip-keyword-check
  if ( !relation.isValid() || relation.referencedLayer() != vl )
  {
    // check for relations by name
    QList< QgsRelation > relations = QgsProject::instance()->relationManager()->relationsByName( relationId ); // skip-keyword-check
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

  QgsVectorLayer *childLayer = relation.referencingLayer();

  // second node is aggregate type
  node = QgsExpressionUtils::getNode( values.at( 1 ), parent );
  ENSURE_NO_EVAL_ERROR
  value = node->eval( parent, context );
  ENSURE_NO_EVAL_ERROR
  bool ok = false;
  Qgis::Aggregate aggregate = QgsAggregateCalculator::stringToAggregate( QgsExpressionUtils::getStringValue( value, parent ), &ok );
  if ( !ok )
  {
    parent->setEvalErrorString( QObject::tr( "No such aggregate '%1'" ).arg( value.toString() ) );
    return QVariant();
  }

  //third node is subexpression (or field name)
  node = QgsExpressionUtils::getNode( values.at( 2 ), parent );
  ENSURE_NO_EVAL_ERROR
  QString subExpression = node->dump();

  //optional fourth node is concatenator
  QgsAggregateCalculator::AggregateParameters parameters;
  if ( values.count() > 3 )
  {
    node = QgsExpressionUtils::getNode( values.at( 3 ), parent );
    ENSURE_NO_EVAL_ERROR
    value = node->eval( parent, context );
    ENSURE_NO_EVAL_ERROR
    parameters.delimiter = value.toString();
  }

  //optional fifth node is order by
  QString orderBy;
  if ( values.count() > 4 )
  {
    node = QgsExpressionUtils::getNode( values.at( 4 ), parent );
    ENSURE_NO_EVAL_ERROR
    QgsExpressionNodeLiteral *nl = dynamic_cast< QgsExpressionNodeLiteral * >( node );
    if ( !nl || nl->value().isValid() )
    {
      orderBy = node->dump();
      parameters.orderBy << QgsFeatureRequest::OrderByClause( orderBy );
    }
  }

  if ( !context->hasFeature() )
    return QVariant();
  QgsFeature f = context->feature();

  parameters.filter = relation.getRelatedFeaturesFilter( f );

  const QString cacheKey = u"relagg:%1%:%2:%3:%4:%5:%6"_s.arg( relationId, vl->id(),
                           QString::number( static_cast< int >( aggregate ) ),
                           subExpression,
                           parameters.filter,
                           orderBy );
  if ( context->hasCachedValue( cacheKey ) )
    return context->cachedValue( cacheKey );

  QVariant result;
  ok = false;


  QgsExpressionContext subContext( *context );
  QString error;
  result = childLayer->aggregate( aggregate, subExpression, parameters, &subContext, &ok, nullptr, context->feedback(), &error );

  if ( !ok )
  {
    if ( !error.isEmpty() )
      parent->setEvalErrorString( QObject::tr( "Could not calculate aggregate for: %1 (%2)" ).arg( subExpression, error ) );
    else
      parent->setEvalErrorString( QObject::tr( "Could not calculate aggregate for: %1" ).arg( subExpression ) );
    return QVariant();
  }

  // cache value
  context->setCachedValue( cacheKey, result );
  return result;
}


static QVariant fcnAggregateGeneric( Qgis::Aggregate aggregate, const QVariantList &values, QgsAggregateCalculator::AggregateParameters parameters, const QgsExpressionContext *context, QgsExpression *parent, int orderByPos = -1 )
{
  if ( !context )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot use aggregate function in this context" ) );
    return QVariant();
  }

  // first step - find current layer

  // TODO this expression function is NOT thread safe
  Q_NOWARN_DEPRECATED_PUSH
  QgsVectorLayer *vl = QgsExpressionUtils::getVectorLayer( context->variable( u"layer"_s ), context, parent );
  Q_NOWARN_DEPRECATED_POP
  if ( !vl )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot use aggregate function in this context" ) );
    return QVariant();
  }

  //lazy eval, so we need to evaluate nodes now

  //first node is subexpression (or field name)
  QgsExpressionNode *node = QgsExpressionUtils::getNode( values.at( 0 ), parent );
  ENSURE_NO_EVAL_ERROR
  QString subExpression = node->dump();

  //optional second node is group by
  QString groupBy;
  if ( values.count() > 1 )
  {
    node = QgsExpressionUtils::getNode( values.at( 1 ), parent );
    ENSURE_NO_EVAL_ERROR
    QgsExpressionNodeLiteral *nl = dynamic_cast< QgsExpressionNodeLiteral * >( node );
    if ( !nl || nl->value().isValid() )
      groupBy = node->dump();
  }

  //optional third node is filter
  if ( values.count() > 2 )
  {
    node = QgsExpressionUtils::getNode( values.at( 2 ), parent );
    ENSURE_NO_EVAL_ERROR
    QgsExpressionNodeLiteral *nl = dynamic_cast< QgsExpressionNodeLiteral * >( node );
    if ( !nl || nl->value().isValid() )
      parameters.filter = node->dump();
  }

  //optional order by node, if supported
  QString orderBy;
  if ( orderByPos >= 0 && values.count() > orderByPos )
  {
    node = QgsExpressionUtils::getNode( values.at( orderByPos ), parent );
    ENSURE_NO_EVAL_ERROR
    QgsExpressionNodeLiteral *nl = dynamic_cast< QgsExpressionNodeLiteral * >( node );
    if ( !nl || nl->value().isValid() )
    {
      orderBy = node->dump();
      parameters.orderBy << QgsFeatureRequest::OrderByClause( orderBy );
    }
  }

  // build up filter with group by

  // find current group by value
  if ( !groupBy.isEmpty() )
  {
    QgsExpression groupByExp( groupBy );
    QVariant groupByValue = groupByExp.evaluate( context );
    QString groupByClause = u"%1 %2 %3"_s.arg( groupBy,
                            QgsVariantUtils::isNull( groupByValue ) ? u"is"_s : u"="_s,
                            QgsExpression::quotedValue( groupByValue ) );
    if ( !parameters.filter.isEmpty() )
      parameters.filter = u"(%1) AND (%2)"_s.arg( parameters.filter, groupByClause );
    else
      parameters.filter = groupByClause;
  }

  QgsExpression subExp( subExpression );
  QgsExpression filterExp( parameters.filter );

  bool isStatic = true;
  const QSet<QString> refVars = filterExp.referencedVariables() + subExp.referencedVariables();
  for ( const QString &varName : refVars )
  {
    const QgsExpressionContextScope *scope = context->activeScopeForVariable( varName );
    if ( scope && !scope->isStatic( varName ) )
    {
      isStatic = false;
      break;
    }
  }

  QString cacheKey;
  if ( !isStatic )
  {
    bool ok = false;
    const QString contextHash = context->uniqueHash( ok, refVars );
    if ( ok )
    {
      cacheKey = u"agg:%1:%2:%3:%4:%5:%6"_s.arg( vl->id(), QString::number( static_cast< int >( aggregate ) ), subExpression, parameters.filter,
                 orderBy, contextHash );
    }
  }
  else
  {
    cacheKey = u"agg:%1:%2:%3:%4:%5"_s.arg( vl->id(), QString::number( static_cast< int >( aggregate ) ), subExpression, parameters.filter, orderBy );
  }

  if ( context->hasCachedValue( cacheKey ) )
    return context->cachedValue( cacheKey );

  QVariant result;
  bool ok = false;

  QgsExpressionContext subContext( *context );
  QgsExpressionContextScope *subScope = new QgsExpressionContextScope();
  subScope->setVariable( u"parent"_s, context->feature(), true );
  subContext.appendScope( subScope );
  QString error;
  result = vl->aggregate( aggregate, subExpression, parameters, &subContext, &ok, nullptr, context->feedback(), &error );

  if ( !ok )
  {
    if ( !error.isEmpty() )
      parent->setEvalErrorString( QObject::tr( "Could not calculate aggregate for: %1 (%2)" ).arg( subExpression, error ) );
    else
      parent->setEvalErrorString( QObject::tr( "Could not calculate aggregate for: %1" ).arg( subExpression ) );
    return QVariant();
  }

  // cache value
  context->setCachedValue( cacheKey, result );
  return result;
}


static QVariant fcnAggregateCount( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( Qgis::Aggregate::Count, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateCountDistinct( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( Qgis::Aggregate::CountDistinct, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateCountMissing( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( Qgis::Aggregate::CountMissing, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMin( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( Qgis::Aggregate::Min, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMax( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( Qgis::Aggregate::Max, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateSum( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( Qgis::Aggregate::Sum, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMean( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( Qgis::Aggregate::Mean, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMedian( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( Qgis::Aggregate::Median, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateStdev( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( Qgis::Aggregate::StDevSample, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateRange( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( Qgis::Aggregate::Range, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMinority( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( Qgis::Aggregate::Minority, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMajority( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( Qgis::Aggregate::Majority, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateQ1( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( Qgis::Aggregate::FirstQuartile, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateQ3( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( Qgis::Aggregate::ThirdQuartile, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateIQR( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( Qgis::Aggregate::InterQuartileRange, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMinLength( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( Qgis::Aggregate::StringMinimumLength, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMaxLength( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( Qgis::Aggregate::StringMaximumLength, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateCollectGeometry( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( Qgis::Aggregate::GeometryCollect, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateStringConcat( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsAggregateCalculator::AggregateParameters parameters;

  //fourth node is concatenator
  if ( values.count() > 3 )
  {
    QgsExpressionNode *node = QgsExpressionUtils::getNode( values.at( 3 ), parent );
    ENSURE_NO_EVAL_ERROR
    QVariant value = node->eval( parent, context );
    ENSURE_NO_EVAL_ERROR
    parameters.delimiter = value.toString();
  }

  return fcnAggregateGeneric( Qgis::Aggregate::StringConcatenate, values, parameters, context, parent, 4 );
}

static QVariant fcnAggregateStringConcatUnique( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsAggregateCalculator::AggregateParameters parameters;

  //fourth node is concatenator
  if ( values.count() > 3 )
  {
    QgsExpressionNode *node = QgsExpressionUtils::getNode( values.at( 3 ), parent );
    ENSURE_NO_EVAL_ERROR
    QVariant value = node->eval( parent, context );
    ENSURE_NO_EVAL_ERROR
    parameters.delimiter = value.toString();
  }

  return fcnAggregateGeneric( Qgis::Aggregate::StringConcatenateUnique, values, parameters, context, parent, 4 );
}

static QVariant fcnAggregateArray( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( Qgis::Aggregate::ArrayAggregate, values, QgsAggregateCalculator::AggregateParameters(), context, parent, 3 );
}

static QVariant fcnMapScale( const QVariantList &, const QgsExpressionContext *context, QgsExpression *, const QgsExpressionNodeFunction * )
{
  if ( !context )
    return QVariant();

  QVariant scale = context->variable( u"map_scale"_s );
  bool ok = false;
  if ( QgsVariantUtils::isNull( scale ) )
    return QVariant();

  const double v = scale.toDouble( &ok );
  if ( ok )
    return v;
  return QVariant();
}

static QVariant fcnClamp( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double minValue = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  double testValue = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  double maxValue = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );

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

static QVariant fcnFloor( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double x = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  return QVariant( std::floor( x ) );
}

static QVariant fcnCeil( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double x = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  return QVariant( std::ceil( x ) );
}

static QVariant fcnToBool( const QVariantList &values, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * )
{
  const QVariant value = values.at( 0 );
  if ( QgsExpressionUtils::isNull( value.isValid() ) )
  {
    return QVariant( false );
  }
  else if ( value.userType() == QMetaType::QString )
  {
    // Capture strings to avoid a '0' string value casted to 0 and wrongly returning false
    return QVariant( !value.toString().isEmpty() );
  }
  else if ( QgsExpressionUtils::isList( value ) )
  {
    return !value.toList().isEmpty();
  }
  return QVariant( value.toBool() );
}
static QVariant fcnToInt( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return QVariant( QgsExpressionUtils::getIntValue( values.at( 0 ), parent ) );
}
static QVariant fcnToReal( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return QVariant( QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent ) );
}
static QVariant fcnToString( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return QVariant( QgsExpressionUtils::getStringValue( values.at( 0 ), parent ) );
}

static QVariant fcnToDateTime( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString format = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  QString language = QgsExpressionUtils::getStringValue( values.at( 2 ), parent );
  if ( format.isEmpty() && !language.isEmpty() )
  {
    parent->setEvalErrorString( QObject::tr( "A format is required to convert to DateTime when the language is specified" ) );
    return QVariant( QDateTime() );
  }

  if ( format.isEmpty() && language.isEmpty() )
    return QVariant( QgsExpressionUtils::getDateTimeValue( values.at( 0 ), parent ) );

  QString datetimestring = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  QLocale locale = QLocale();
  if ( !language.isEmpty() )
  {
    locale = QLocale( language );
  }

  QDateTime datetime = locale.toDateTime( datetimestring, format );
  if ( !datetime.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to DateTime" ).arg( datetimestring ) );
    datetime = QDateTime();
  }
  return QVariant( datetime );
}

static QVariant fcnMakeDate( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const int year = QgsExpressionUtils::getIntValue( values.at( 0 ), parent );
  const int month = QgsExpressionUtils::getIntValue( values.at( 1 ), parent );
  const int day = QgsExpressionUtils::getIntValue( values.at( 2 ), parent );

  const QDate date( year, month, day );
  if ( !date.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "'%1-%2-%3' is not a valid date" ).arg( year ).arg( month ).arg( day ) );
    return QVariant();
  }
  return QVariant( date );
}

static QVariant fcnMakeTime( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const int hours = QgsExpressionUtils::getIntValue( values.at( 0 ), parent );
  const int minutes = QgsExpressionUtils::getIntValue( values.at( 1 ), parent );
  const double seconds = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );

  const QTime time( hours, minutes, std::floor( seconds ), ( seconds - std::floor( seconds ) ) * 1000 );
  if ( !time.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "'%1-%2-%3' is not a valid time" ).arg( hours ).arg( minutes ).arg( seconds ) );
    return QVariant();
  }
  return QVariant( time );
}

static QVariant fcnMakeDateTime( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const int year = QgsExpressionUtils::getIntValue( values.at( 0 ), parent );
  const int month = QgsExpressionUtils::getIntValue( values.at( 1 ), parent );
  const int day = QgsExpressionUtils::getIntValue( values.at( 2 ), parent );
  const int hours = QgsExpressionUtils::getIntValue( values.at( 3 ), parent );
  const int minutes = QgsExpressionUtils::getIntValue( values.at( 4 ), parent );
  const double seconds = QgsExpressionUtils::getDoubleValue( values.at( 5 ), parent );

  const QDate date( year, month, day );
  if ( !date.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "'%1-%2-%3' is not a valid date" ).arg( year ).arg( month ).arg( day ) );
    return QVariant();
  }
  const QTime time( hours, minutes, std::floor( seconds ), ( seconds - std::floor( seconds ) ) * 1000 );
  if ( !time.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "'%1-%2-%3' is not a valid time" ).arg( hours ).arg( minutes ).arg( seconds ) );
    return QVariant();
  }
  return QVariant( QDateTime( date, time ) );
}

static QVariant fcnTimeZoneFromId( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString timeZoneId = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );

  QTimeZone tz;

#if QT_FEATURE_timezone > 0
  if ( !timeZoneId.isEmpty() )
  {
    tz = QTimeZone( timeZoneId.toUtf8() );
  }

  if ( !tz.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "'%1' is not a valid time zone ID" ).arg( timeZoneId ) );
    return QVariant();
  }

#else
  parent->setEvalErrorString( QObject::tr( "Qt is built without Qt timezone support, cannot use fcnTimeZoneFromId" ) );
#endif
  return QVariant::fromValue( tz );
}

static QVariant fcnGetTimeZone( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
#if QT_FEATURE_timezone > 0
  const QDateTime datetime = QgsExpressionUtils::getDateTimeValue( values.at( 0 ), parent );
  if ( datetime.isValid() )
  {
    return QVariant::fromValue( datetime.timeZone() );
  }
  return QVariant();
#else
  parent->setEvalErrorString( QObject::tr( "Qt is built without Qt timezone support, cannot use fcnGetTimeZone" ) );
  return QVariant();
#endif
}

static QVariant fcnSetTimeZone( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
#if QT_FEATURE_timezone > 0
  QDateTime datetime = QgsExpressionUtils::getDateTimeValue( values.at( 0 ), parent );
  const QTimeZone tz = QgsExpressionUtils::getTimeZoneValue( values.at( 1 ), parent );
  if ( datetime.isValid() && tz.isValid() )
  {
    datetime.setTimeZone( tz );
    return QVariant::fromValue( datetime );
  }
  return QVariant();
#else
  parent->setEvalErrorString( QObject::tr( "Qt is built without Qt timezone support, cannot use fcnSetTimeZone" ) );
  return QVariant();
#endif
}

static QVariant fcnConvertTimeZone( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
#if QT_FEATURE_timezone > 0
  const QDateTime datetime = QgsExpressionUtils::getDateTimeValue( values.at( 0 ), parent );
  const QTimeZone tz = QgsExpressionUtils::getTimeZoneValue( values.at( 1 ), parent );
  if ( datetime.isValid() && tz.isValid() )
  {
    return QVariant::fromValue( datetime.toTimeZone( tz ) );
  }
  return QVariant();
#else
  parent->setEvalErrorString( QObject::tr( "Qt is built without Qt timezone support, cannot use fcnConvertTimeZone" ) );
  return QVariant();
#endif
}

static QVariant fcnTimeZoneToId( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
#if QT_FEATURE_timezone > 0
  const QTimeZone timeZone = QgsExpressionUtils::getTimeZoneValue( values.at( 0 ), parent );
  if ( timeZone.isValid() )
  {
    return QString( timeZone.id() );
  }
  return QVariant();
#else
  parent->setEvalErrorString( QObject::tr( "Qt is built without Qt timezone support, cannot use fcnTimeZoneToId" ) );
  return QVariant();
#endif
}

static QVariant fcnMakeInterval( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const double years = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  const double months = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  const double weeks = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
  const double days = QgsExpressionUtils::getDoubleValue( values.at( 3 ), parent );
  const double hours = QgsExpressionUtils::getDoubleValue( values.at( 4 ), parent );
  const double minutes = QgsExpressionUtils::getDoubleValue( values.at( 5 ), parent );
  const double seconds = QgsExpressionUtils::getDoubleValue( values.at( 6 ), parent );

  return QVariant::fromValue( QgsInterval( years, months, weeks, days, hours, minutes, seconds ) );
}

static QVariant fcnCoalesce( const QVariantList &values, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * )
{
  for ( const QVariant &value : values )
  {
    if ( QgsVariantUtils::isNull( value ) )
      continue;
    return value;
  }
  return QVariant();
}

static QVariant fcnNullIf( const QVariantList &values, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * )
{
  const QVariant val1 = values.at( 0 );
  const QVariant val2 = values.at( 1 );

  if ( val1 == val2 )
    return QVariant();
  else
    return val1;
}

static QVariant fcnLower( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  return QVariant( str.toLower() );
}
static QVariant fcnUpper( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  return QVariant( str.toUpper() );
}
static QVariant fcnTitle( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  QStringList elems = str.split( ' ' );
  for ( int i = 0; i < elems.size(); i++ )
  {
    if ( elems[i].size() > 1 )
      elems[i] = elems[i].at( 0 ).toUpper() + elems[i].mid( 1 ).toLower();
  }
  return QVariant( elems.join( ' '_L1 ) );
}

static QVariant fcnTrim( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  return QVariant( str.trimmed() );
}

static QVariant fcnLTrim( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );

  const QString characters = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );

  const QRegularExpression re( u"^([%1]*)"_s.arg( QRegularExpression::escape( characters ) ) );
  str.replace( re, QString() );
  return QVariant( str );
}

static QVariant fcnRTrim( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );

  const QString characters = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );

  const QRegularExpression re( u"([%1]*)$"_s.arg( QRegularExpression::escape( characters ) ) );
  str.replace( re, QString() );
  return QVariant( str );
}

static QVariant fcnLevenshtein( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString string1 = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  QString string2 = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  return QVariant( QgsStringUtils::levenshteinDistance( string1, string2, true ) );
}

static QVariant fcnLCS( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString string1 = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  QString string2 = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  return QVariant( QgsStringUtils::longestCommonSubstring( string1, string2, true ) );
}

static QVariant fcnHamming( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString string1 = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  QString string2 = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  int dist = QgsStringUtils::hammingDistance( string1, string2 );
  return ( dist < 0 ? QVariant() : QVariant( QgsStringUtils::hammingDistance( string1, string2, true ) ) );
}

static QVariant fcnSoundex( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString string = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  return QVariant( QgsStringUtils::soundex( string ) );
}

static QVariant fcnChar( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QChar character = QChar( QgsExpressionUtils::getNativeIntValue( values.at( 0 ), parent ) );
  return QVariant( QString( character ) );
}

static QVariant fcnAscii( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString value = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );

  if ( value.isEmpty() )
  {
    return QVariant();
  }

  int res = value.at( 0 ).unicode();
  return QVariant( res );
}

static QVariant fcnWordwrap( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( values.length() == 2 || values.length() == 3 )
  {
    QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
    qlonglong wrap = QgsExpressionUtils::getIntValue( values.at( 1 ), parent );

    QString customdelimiter = QgsExpressionUtils::getStringValue( values.at( 2 ), parent );

    return QgsStringUtils::wordWrap( str, static_cast< int >( wrap ), wrap > 0, customdelimiter );
  }

  return QVariant();
}

static QVariant fcnLength( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  // two variants, one for geometry, one for string

  //geometry variant
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent, true );
  if ( !geom.isNull() )
  {
    if ( geom.type() == Qgis::GeometryType::Line )
      return QVariant( geom.length() );
    else
      return QVariant();
  }

  //otherwise fall back to string variant
  QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  return QVariant( str.length() );
}

static QVariant fcnLength3D( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.type() != Qgis::GeometryType::Line )
    return QVariant();

  double totalLength = 0;
  for ( auto it = geom.const_parts_begin(); it != geom.const_parts_end(); ++it )
  {
    if ( const QgsLineString *line = qgsgeometry_cast< const QgsLineString * >( *it ) )
    {
      totalLength += line->length3D();
    }
    else
    {
      std::unique_ptr< QgsLineString > segmentized( qgsgeometry_cast< const QgsCurve * >( *it )->curveToLine() );
      totalLength += segmentized->length3D();
    }
  }

  return totalLength;
}


static QVariant fcnRepeat( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString string = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  const qlonglong number = QgsExpressionUtils::getIntValue( values.at( 1 ), parent );
  return string.repeated( std::max( static_cast< int >( number ), 0 ) );
}

static QVariant fcnReplace( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( values.count() == 2 && values.at( 1 ).userType() == QMetaType::Type::QVariantMap )
  {
    QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
    QVariantMap map = QgsExpressionUtils::getMapValue( values.at( 1 ), parent );
    QVector< QPair< QString, QString > > mapItems;

    for ( QVariantMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it )
    {
      mapItems.append( qMakePair( it.key(), it.value().toString() ) );
    }

    // larger keys should be replaced first since they may contain whole smaller keys
    std::sort( mapItems.begin(),
               mapItems.end(),
               []( const QPair< QString, QString > &pair1,
                   const QPair< QString, QString > &pair2 )
    {
      return ( pair1.first.length() > pair2.first.length() );
    } );

    for ( auto it = mapItems.constBegin(); it != mapItems.constEnd(); ++it )
    {
      str = str.replace( it->first, it->second );
    }

    return QVariant( str );
  }
  else if ( values.count() == 3 )
  {
    QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
    QVariantList before;
    QVariantList after;
    bool isSingleReplacement = false;

    if ( !QgsExpressionUtils::isList( values.at( 1 ) ) && values.at( 2 ).userType() != QMetaType::Type::QStringList )
    {
      before = QVariantList() << QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
    }
    else
    {
      before = QgsExpressionUtils::getListValue( values.at( 1 ), parent );
    }

    if ( !QgsExpressionUtils::isList( values.at( 2 ) ) )
    {
      after = QVariantList() << QgsExpressionUtils::getStringValue( values.at( 2 ), parent );
      isSingleReplacement = true;
    }
    else
    {
      after = QgsExpressionUtils::getListValue( values.at( 2 ), parent );
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

static QVariant fcnRegexpReplace( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  QString regexp = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  QString after = QgsExpressionUtils::getStringValue( values.at( 2 ), parent );

  QRegularExpression re( regexp, QRegularExpression::UseUnicodePropertiesOption );
  if ( !re.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Invalid regular expression '%1': %2" ).arg( regexp, re.errorString() ) );
    return QVariant();
  }
  return QVariant( str.replace( re, after ) );
}

static QVariant fcnRegexpMatch( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  QString regexp = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );

  QRegularExpression re( regexp, QRegularExpression::UseUnicodePropertiesOption );
  if ( !re.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Invalid regular expression '%1': %2" ).arg( regexp, re.errorString() ) );
    return QVariant();
  }
  return QVariant( ( str.indexOf( re ) + 1 ) );
}

static QVariant fcnRegexpMatches( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  QString regexp = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  QString empty = QgsExpressionUtils::getStringValue( values.at( 2 ), parent );

  QRegularExpression re( regexp, QRegularExpression::UseUnicodePropertiesOption );
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
      array += ( !( *it ).isEmpty() ) ? *it : empty;
    }

    return QVariant( array );
  }
  else
  {
    return QVariant();
  }
}

static QVariant fcnRegexpSubstr( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  QString regexp = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );

  QRegularExpression re( regexp, QRegularExpression::UseUnicodePropertiesOption );
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
    if ( match.lastCapturedIndex() > 0 )
    {
      // a capture group was present, so use that
      return QVariant( match.captured( 1 ) );
    }
    else
    {
      // no capture group, so using all match
      return QVariant( match.captured( 0 ) );
    }
  }
  else
  {
    return QVariant( "" );
  }
}

static QVariant fcnUuid( const QVariantList &values, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * )
{
  QString uuid = QUuid::createUuid().toString();
  if ( values.at( 0 ).toString().compare( u"WithoutBraces"_s, Qt::CaseInsensitive ) == 0 )
    uuid = QUuid::createUuid().toString( QUuid::StringFormat::WithoutBraces );
  else if ( values.at( 0 ).toString().compare( u"Id128"_s, Qt::CaseInsensitive ) == 0 )
    uuid = QUuid::createUuid().toString( QUuid::StringFormat::Id128 );
  return uuid;
}

static QVariant fcnSubstr( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( !values.at( 0 ).isValid() || !values.at( 1 ).isValid() )
    return QVariant();

  QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  int from = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );

  int len = 0;
  if ( values.at( 2 ).isValid() )
    len = QgsExpressionUtils::getNativeIntValue( values.at( 2 ), parent );
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
static QVariant fcnFeatureId( const QVariantList &, const QgsExpressionContext *context, QgsExpression *, const QgsExpressionNodeFunction * )
{
  FEAT_FROM_CONTEXT( context, f )
  return QVariant( f.id() );
}

static QVariant fcnRasterValue( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const int bandNb = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  const QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 2 ), parent );
  bool foundLayer = false;
  const QVariant res = QgsExpressionUtils::runMapLayerFunctionThreadSafe( values.at( 0 ), context, parent, [parent, bandNb, geom]( QgsMapLayer * mapLayer )
  {
    QgsRasterLayer *layer = qobject_cast< QgsRasterLayer * >( mapLayer );
    if ( !layer || !layer->dataProvider() )
    {
      parent->setEvalErrorString( QObject::tr( "Function `raster_value` requires a valid raster layer." ) );
      return QVariant();
    }

    if ( bandNb < 1 || bandNb > layer->bandCount() )
    {
      parent->setEvalErrorString( QObject::tr( "Function `raster_value` requires a valid raster band number." ) );
      return QVariant();
    }

    if ( geom.isNull() || geom.type() != Qgis::GeometryType::Point )
    {
      parent->setEvalErrorString( QObject::tr( "Function `raster_value` requires a valid point geometry." ) );
      return QVariant();
    }

    QgsPointXY point = geom.asPoint();
    if ( geom.isMultipart() )
    {
      QgsMultiPointXY multiPoint = geom.asMultiPoint();
      if ( multiPoint.count() == 1 )
      {
        point = multiPoint[0];
      }
      else
      {
        // if the geometry contains more than one part, return an undefined value
        return QVariant();
      }
    }

    double value = layer->dataProvider()->sample( point, bandNb );
    return std::isnan( value ) ? QVariant() : value;
  },
  foundLayer );

  if ( !foundLayer )
  {
    parent->setEvalErrorString( QObject::tr( "Function `raster_value` requires a valid raster layer." ) );
    return QVariant();
  }
  else
  {
    return res;
  }
}

static QVariant fcnRasterAttributes( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const int bandNb = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  const double value = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );

  bool foundLayer = false;
  const QVariant res = QgsExpressionUtils::runMapLayerFunctionThreadSafe( values.at( 0 ), context, parent, [parent, bandNb, value]( QgsMapLayer * mapLayer )-> QVariant
  {
    QgsRasterLayer *layer = qobject_cast< QgsRasterLayer *>( mapLayer );
    if ( !layer || !layer->dataProvider() )
    {
      parent->setEvalErrorString( QObject::tr( "Function `raster_attributes` requires a valid raster layer." ) );
      return QVariant();
    }

    if ( bandNb < 1 || bandNb > layer->bandCount() )
    {
      parent->setEvalErrorString( QObject::tr( "Function `raster_attributes` requires a valid raster band number." ) );
      return QVariant();
    }

    if ( std::isnan( value ) )
    {
      parent->setEvalErrorString( QObject::tr( "Function `raster_attributes` requires a valid raster value." ) );
      return QVariant();
    }

    if ( ! layer->dataProvider()->attributeTable( bandNb ) )
    {
      return QVariant();
    }

    const QVariantList data = layer->dataProvider()->attributeTable( bandNb )->row( value );
    if ( data.isEmpty() )
    {
      return QVariant();
    }

    QVariantMap result;
    const QList<QgsRasterAttributeTable::Field> fields { layer->dataProvider()->attributeTable( bandNb )->fields() };
    for ( int idx = 0; idx < static_cast<int>( fields.count( ) ) && idx < static_cast<int>( data.count() ); ++idx )
    {
      const QgsRasterAttributeTable::Field field { fields.at( idx ) };
      if ( field.isColor() || field.isRamp() )
      {
        continue;
      }
      result.insert( fields.at( idx ).name, data.at( idx ) );
    }

    return result;
  }, foundLayer );

  if ( !foundLayer )
  {
    parent->setEvalErrorString( QObject::tr( "Function `raster_attributes` requires a valid raster layer." ) );
    return QVariant();
  }
  else
  {
    return res;
  }
}

static QVariant fcnFeature( const QVariantList &, const QgsExpressionContext *context, QgsExpression *, const QgsExpressionNodeFunction * )
{
  if ( !context )
    return QVariant();

  return context->feature();
}

static QVariant fcnAttribute( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsFeature feature;
  QString attr;
  if ( values.size() == 1 )
  {
    attr = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
    feature = context->feature();
  }
  else if ( values.size() == 2 )
  {
    feature = QgsExpressionUtils::getFeature( values.at( 0 ), parent );
    attr = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  }
  else
  {
    parent->setEvalErrorString( QObject::tr( "Function `attribute` requires one or two parameters. %n given.", nullptr, values.length() ) );
    return QVariant();
  }

  return feature.attribute( attr );
}

static QVariant fcnMapToHtmlTable( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString table { R"html(
  <table>
    <thead>
      <tr><th>%1</th></tr>
    </thead>
    <tbody>
      <tr><td>%2</td></tr>
    </tbody>
  </table>)html" };
  QVariantMap dict;
  if ( values.size() == 1 )
  {
    dict = QgsExpressionUtils::getMapValue( values.at( 0 ), parent );
  }
  else
  {
    parent->setEvalErrorString( QObject::tr( "Function `map_to_html_table` requires one parameter. %n given.", nullptr, values.length() ) );
    return QVariant();
  }

  if ( dict.isEmpty() )
  {
    return QVariant();
  }

  QStringList headers;
  QStringList cells;

  for ( auto it = dict.cbegin(); it != dict.cend(); ++it )
  {
    headers.push_back( it.key().toHtmlEscaped() );
    cells.push_back( it.value().toString( ).toHtmlEscaped() );
  }

  return table.arg( headers.join( "</th><th>"_L1 ), cells.join( "</td><td>"_L1 ) );
}

static QVariant fcnMapToHtmlDefinitionList( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString table { R"html(
  <dl>
    %1
  </dl>)html" };
  QVariantMap dict;
  if ( values.size() == 1 )
  {
    dict = QgsExpressionUtils::getMapValue( values.at( 0 ), parent );
  }
  else
  {
    parent->setEvalErrorString( QObject::tr( "Function `map_to_html_dl` requires one parameter. %n given.", nullptr, values.length() ) );
    return QVariant();
  }

  if ( dict.isEmpty() )
  {
    return QVariant();
  }

  QString rows;

  for ( auto it = dict.cbegin(); it != dict.cend(); ++it )
  {
    rows.append( u"<dt>%1</dt><dd>%2</dd>"_s.arg( it.key().toHtmlEscaped(), it.value().toString().toHtmlEscaped() ) );
  }

  return table.arg( rows );
}

static QVariant fcnValidateFeature( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariant layer;
  if ( values.size() < 1 || QgsVariantUtils::isNull( values.at( 0 ) ) )
  {
    layer = context->variable( u"layer"_s );
  }
  else
  {
    //first node is layer id or name
    QgsExpressionNode *node = QgsExpressionUtils::getNode( values.at( 0 ), parent );
    ENSURE_NO_EVAL_ERROR
    layer = node->eval( parent, context );
    ENSURE_NO_EVAL_ERROR
  }

  QgsFeature feature;
  if ( values.size() < 2 || QgsVariantUtils::isNull( values.at( 1 ) ) )
  {
    feature = context->feature();
  }
  else
  {
    feature = QgsExpressionUtils::getFeature( values.at( 1 ), parent );
  }

  QgsFieldConstraints::ConstraintStrength constraintStrength = QgsFieldConstraints::ConstraintStrengthNotSet;
  const QString strength = QgsExpressionUtils::getStringValue( values.at( 2 ), parent ).toLower();
  if ( strength == "hard"_L1 )
  {
    constraintStrength = QgsFieldConstraints::ConstraintStrengthHard;
  }
  else if ( strength == "soft"_L1 )
  {
    constraintStrength = QgsFieldConstraints::ConstraintStrengthSoft;
  }

  bool foundLayer = false;
  const QVariant res = QgsExpressionUtils::runMapLayerFunctionThreadSafe( layer, context, parent, [parent, feature, constraintStrength]( QgsMapLayer * mapLayer ) -> QVariant
  {
    QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( mapLayer );
    if ( !layer )
    {
      parent->setEvalErrorString( QObject::tr( "No layer provided to conduct constraints checks" ) );
      return QVariant();
    }

    const QgsFields fields = layer->fields();
    bool valid = true;
    for ( int i = 0; i < fields.size(); i++ )
    {
      QStringList errors;
      valid = QgsVectorLayerUtils::validateAttribute( layer, feature, i, errors, constraintStrength );
      if ( !valid )
      {
        break;
      }
    }

    return valid;
  }, foundLayer );

  if ( !foundLayer )
  {
    parent->setEvalErrorString( QObject::tr( "No layer provided to conduct constraints checks" ) );
    return QVariant();
  }

  return res;
}

static QVariant fcnValidateAttribute( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariant layer;
  if ( values.size() < 2 || QgsVariantUtils::isNull( values.at( 1 ) ) )
  {
    layer = context->variable( u"layer"_s );
  }
  else
  {
    //first node is layer id or name
    QgsExpressionNode *node = QgsExpressionUtils::getNode( values.at( 1 ), parent );
    ENSURE_NO_EVAL_ERROR
    layer = node->eval( parent, context );
    ENSURE_NO_EVAL_ERROR
  }

  QgsFeature feature;
  if ( values.size() < 3 || QgsVariantUtils::isNull( values.at( 2 ) ) )
  {
    feature = context->feature();
  }
  else
  {
    feature = QgsExpressionUtils::getFeature( values.at( 2 ), parent );
  }

  QgsFieldConstraints::ConstraintStrength constraintStrength = QgsFieldConstraints::ConstraintStrengthNotSet;
  const QString strength = QgsExpressionUtils::getStringValue( values.at( 3 ), parent ).toLower();
  if ( strength == "hard"_L1 )
  {
    constraintStrength = QgsFieldConstraints::ConstraintStrengthHard;
  }
  else if ( strength == "soft"_L1 )
  {
    constraintStrength = QgsFieldConstraints::ConstraintStrengthSoft;
  }

  const QString attributeName = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );

  bool foundLayer = false;
  const QVariant res = QgsExpressionUtils::runMapLayerFunctionThreadSafe( layer, context, parent, [parent, feature, attributeName, constraintStrength]( QgsMapLayer * mapLayer ) -> QVariant
  {
    QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( mapLayer );
    if ( !layer )
    {
      return QVariant();
    }

    const int fieldIndex = layer->fields().indexFromName( attributeName );
    if ( fieldIndex == -1 )
    {
      parent->setEvalErrorString( QObject::tr( "The attribute name did not match any field for the given feature" ) );
      return QVariant();
    }

    QStringList errors;
    bool valid = QgsVectorLayerUtils::validateAttribute( layer, feature, fieldIndex, errors, constraintStrength );
    return valid;
  }, foundLayer );

  if ( !foundLayer )
  {
    parent->setEvalErrorString( QObject::tr( "No layer provided to conduct constraints checks" ) );
    return QVariant();
  }

  return res;
}

static QVariant fcnAttributes( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsFeature feature;
  if ( values.size() == 0 || QgsVariantUtils::isNull( values.at( 0 ) ) )
  {
    feature = context->feature();
  }
  else
  {
    feature = QgsExpressionUtils::getFeature( values.at( 0 ), parent );
  }

  const QgsFields fields = feature.fields();
  QVariantMap result;
  for ( int i = 0; i < fields.count(); ++i )
  {
    result.insert( fields.at( i ).name(), feature.attribute( i ) );
  }
  return result;
}

static QVariant fcnRepresentAttributes( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsVectorLayer *layer = nullptr;
  QgsFeature feature;

  // TODO this expression function is NOT thread safe
  Q_NOWARN_DEPRECATED_PUSH
  if ( values.isEmpty() )
  {
    feature = context->feature();
    layer = QgsExpressionUtils::getVectorLayer( context->variable( u"layer"_s ), context, parent );
  }
  else if ( values.size() == 1 )
  {
    layer = QgsExpressionUtils::getVectorLayer( context->variable( u"layer"_s ), context, parent );
    feature = QgsExpressionUtils::getFeature( values.at( 0 ), parent );
  }
  else if ( values.size() == 2 )
  {
    layer = QgsExpressionUtils::getVectorLayer( values.at( 0 ), context,  parent );
    feature = QgsExpressionUtils::getFeature( values.at( 1 ), parent );
  }
  else
  {
    parent->setEvalErrorString( QObject::tr( "Function `represent_attributes` requires no more than two parameters. %n given.", nullptr, values.length() ) );
    return QVariant();
  }
  Q_NOWARN_DEPRECATED_POP

  if ( !layer )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot use represent attributes function: layer could not be resolved." ) );
    return QVariant();
  }

  if ( !feature.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot use represent attributes function: feature could not be resolved." ) );
    return QVariant();
  }

  const QgsFields fields = feature.fields();
  QVariantMap result;
  for ( int fieldIndex = 0; fieldIndex < fields.count(); ++fieldIndex )
  {
    const QString fieldName { fields.at( fieldIndex ).name() };
    const QVariant attributeVal = feature.attribute( fieldIndex );
    const QString cacheValueKey = u"repvalfcnval:%1:%2:%3"_s.arg( layer->id(), fieldName, attributeVal.toString() );
    if ( context && context->hasCachedValue( cacheValueKey ) )
    {
      result.insert( fieldName, context->cachedValue( cacheValueKey ) );
    }
    else
    {
      const QgsEditorWidgetSetup setup = layer->editorWidgetSetup( fieldIndex );
      QgsFieldFormatter *fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );
      QVariant cache;
      if ( context )
      {
        const QString cacheKey = u"repvalfcn:%1:%2"_s.arg( layer->id(), fieldName );

        if ( !context->hasCachedValue( cacheKey ) )
        {
          cache = fieldFormatter->createCache( layer, fieldIndex, setup.config() );
          context->setCachedValue( cacheKey, cache );
        }
        else
        {
          cache = context->cachedValue( cacheKey );
        }
      }
      QString value( fieldFormatter->representValue( layer, fieldIndex, setup.config(), cache, attributeVal ) );

      result.insert( fields.at( fieldIndex ).name(), value );

      if ( context )
      {
        context->setCachedValue( cacheValueKey, value );
      }

    }
  }
  return result;
}

static QVariant fcnCoreFeatureMaptipDisplay( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const bool isMaptip )
{
  QgsVectorLayer *layer = nullptr;
  QgsFeature feature;
  bool evaluate = true;

  // TODO this expression function is NOT thread safe
  Q_NOWARN_DEPRECATED_PUSH
  if ( values.isEmpty() )
  {
    feature = context->feature();
    layer = QgsExpressionUtils::getVectorLayer( context->variable( u"layer"_s ), context, parent );
  }
  else if ( values.size() == 1 )
  {
    layer = QgsExpressionUtils::getVectorLayer( context->variable( u"layer"_s ), context, parent );
    feature = QgsExpressionUtils::getFeature( values.at( 0 ), parent );
  }
  else if ( values.size() == 2 )
  {
    layer = QgsExpressionUtils::getVectorLayer( values.at( 0 ), context, parent );
    feature = QgsExpressionUtils::getFeature( values.at( 1 ), parent );
  }
  else if ( values.size() == 3 )
  {
    layer = QgsExpressionUtils::getVectorLayer( values.at( 0 ), context, parent );
    feature = QgsExpressionUtils::getFeature( values.at( 1 ), parent );
    evaluate = values.value( 2 ).toBool();
  }
  else
  {
    if ( isMaptip )
    {
      parent->setEvalErrorString( QObject::tr( "Function `maptip` requires no more than three parameters. %n given.", nullptr, values.length() ) );
    }
    else
    {
      parent->setEvalErrorString( QObject::tr( "Function `display` requires no more than three parameters. %n given.", nullptr, values.length() ) );
    }
    return QVariant();
  }

  if ( !layer )
  {
    parent->setEvalErrorString( QObject::tr( "The layer is not valid." ) );
    return QVariant( );
  }
  Q_NOWARN_DEPRECATED_POP

  if ( !feature.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "The feature is not valid." ) );
    return QVariant( );
  }

  if ( ! evaluate )
  {
    if ( isMaptip )
    {
      return layer->mapTipTemplate();
    }
    else
    {
      return layer->displayExpression();
    }
  }

  QgsExpressionContext subContext( *context );
  subContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
  subContext.setFeature( feature );

  if ( isMaptip )
  {
    return QgsExpression::replaceExpressionText( layer->mapTipTemplate(), &subContext );
  }
  else
  {
    QgsExpression exp( layer->displayExpression() );
    exp.prepare( &subContext );
    return exp.evaluate( &subContext ).toString();
  }
}

static QVariant fcnFeatureDisplayExpression( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnCoreFeatureMaptipDisplay( values, context, parent, false );
}

static QVariant fcnFeatureMaptip( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnCoreFeatureMaptipDisplay( values, context, parent, true );
}

static QVariant fcnIsSelected( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsFeature feature;
  QVariant layer;
  if ( values.isEmpty() )
  {
    feature = context->feature();
    layer = context->variable( u"layer"_s );
  }
  else if ( values.size() == 1 )
  {
    feature = QgsExpressionUtils::getFeature( values.at( 0 ), parent );
    layer = context->variable( u"layer"_s );
  }
  else if ( values.size() == 2 )
  {
    feature = QgsExpressionUtils::getFeature( values.at( 1 ), parent );
    layer = values.at( 0 );
  }
  else
  {
    parent->setEvalErrorString( QObject::tr( "Function `is_selected` requires no more than two parameters. %n given.", nullptr, values.length() ) );
    return QVariant();
  }

  bool foundLayer = false;
  const QVariant res = QgsExpressionUtils::runMapLayerFunctionThreadSafe( layer, context, parent, [feature]( QgsMapLayer * mapLayer ) -> QVariant
  {
    QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( mapLayer );
    if ( !layer || !feature.isValid() )
    {
      return QgsVariantUtils::createNullVariant( QMetaType::Type::Bool );
    }

    return layer->selectedFeatureIds().contains( feature.id() );
  }, foundLayer );
  if ( !foundLayer )
    return  QgsVariantUtils::createNullVariant( QMetaType::Type::Bool );
  else
    return res;
}

static QVariant fcnNumSelected( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariant layer;

  if ( values.isEmpty() )
    layer = context->variable( u"layer"_s );
  else if ( values.count() == 1 )
    layer = values.at( 0 );
  else
  {
    parent->setEvalErrorString( QObject::tr( "Function `num_selected` requires no more than one parameter. %n given.", nullptr, values.length() ) );
    return QVariant();
  }

  bool foundLayer = false;
  const QVariant res = QgsExpressionUtils::runMapLayerFunctionThreadSafe( layer, context, parent, []( QgsMapLayer * mapLayer ) -> QVariant
  {
    QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( mapLayer );
    if ( !layer )
    {
      return QgsVariantUtils::createNullVariant( QMetaType::Type::LongLong );
    }

    return layer->selectedFeatureCount();
  }, foundLayer );
  if ( !foundLayer )
    return  QgsVariantUtils::createNullVariant( QMetaType::Type::LongLong );
  else
    return res;
}

static QVariant fcnSqliteFetchAndIncrement( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  static QMap<QString, qlonglong> counterCache;
  QVariant functionResult;

  auto fetchAndIncrementFunc = [ values, parent, &functionResult ]( QgsMapLayer * mapLayer, const QString & databaseArgument )
  {
    QString database;

    const QgsVectorLayer *layer = qobject_cast< QgsVectorLayer *>( mapLayer );

    if ( layer )
    {
      const QVariantMap decodedUri = QgsProviderRegistry::instance()->decodeUri( layer->providerType(), layer->dataProvider()->dataSourceUri() );
      database = decodedUri.value( u"path"_s ).toString();
      if ( database.isEmpty() )
      {
        parent->setEvalErrorString( QObject::tr( "Could not extract file path from layer `%1`." ).arg( layer->name() ) );
      }
    }
    else
    {
      database = databaseArgument;
    }

    const QString table = values.at( 1 ).toString();
    const QString idColumn = values.at( 2 ).toString();
    const QString filterAttribute = values.at( 3 ).toString();
    const QVariant filterValue = values.at( 4 ).toString();
    const QVariantMap defaultValues = values.at( 5 ).toMap();

    // read from database
    sqlite3_database_unique_ptr sqliteDb;
    sqlite3_statement_unique_ptr sqliteStatement;

    if ( sqliteDb.open_v2( database, SQLITE_OPEN_READWRITE, nullptr ) != SQLITE_OK )
    {
      parent->setEvalErrorString( QObject::tr( "Could not open sqlite database %1. Error %2. " ).arg( database, sqliteDb.errorMessage() ) );
      functionResult = QVariant();
      return;
    }

    QString errorMessage;
    QString currentValSql;

    qlonglong nextId = 0;
    bool cachedMode = false;
    bool valueRetrieved = false;

    QString cacheString = u"%1:%2:%3:%4:%5"_s.arg( database, table, idColumn, filterAttribute, filterValue.toString() );

    // Running in transaction mode, check for cached value first
    if ( layer && layer->dataProvider() && layer->dataProvider()->transaction() )
    {
      cachedMode = true;

      auto cachedCounter = counterCache.find( cacheString );

      if ( cachedCounter != counterCache.end() )
      {
        qlonglong &cachedValue = cachedCounter.value();
        nextId = cachedValue;
        nextId += 1;
        cachedValue = nextId;
        valueRetrieved = true;
      }
    }

    // Either not in cached mode or no cached value found, obtain from DB
    if ( !cachedMode || !valueRetrieved )
    {
      int result = SQLITE_ERROR;

      currentValSql = u"SELECT %1 FROM %2"_s.arg( QgsSqliteUtils::quotedIdentifier( idColumn ), QgsSqliteUtils::quotedIdentifier( table ) );
      if ( !filterAttribute.isNull() )
      {
        currentValSql += u" WHERE %1 = %2"_s.arg( QgsSqliteUtils::quotedIdentifier( filterAttribute ), QgsSqliteUtils::quotedValue( filterValue ) );
      }

      sqliteStatement = sqliteDb.prepare( currentValSql, result );

      if ( result == SQLITE_OK )
      {
        nextId = 0;
        if ( sqliteStatement.step() == SQLITE_ROW )
        {
          nextId = sqliteStatement.columnAsInt64( 0 ) + 1;
        }

        // If in cached mode: add value to cache and connect to transaction
        if ( cachedMode && result == SQLITE_OK )
        {
          counterCache.insert( cacheString, nextId );

          QObject::connect( layer->dataProvider()->transaction(), &QgsTransaction::destroyed, [cacheString]()
          {
            counterCache.remove( cacheString );
          } );
        }
        valueRetrieved = true;
      }
    }

    if ( valueRetrieved )
    {
      QString upsertSql;
      upsertSql = u"INSERT OR REPLACE INTO %1"_s.arg( QgsSqliteUtils::quotedIdentifier( table ) );
      QStringList cols;
      QStringList vals;
      cols << QgsSqliteUtils::quotedIdentifier( idColumn );
      vals << QgsSqliteUtils::quotedValue( nextId );

      if ( !filterAttribute.isNull() )
      {
        cols << QgsSqliteUtils::quotedIdentifier( filterAttribute );
        vals << QgsSqliteUtils::quotedValue( filterValue );
      }

      for ( QVariantMap::const_iterator iter = defaultValues.constBegin(); iter != defaultValues.constEnd(); ++iter )
      {
        cols << QgsSqliteUtils::quotedIdentifier( iter.key() );
        vals << iter.value().toString();
      }

      upsertSql += " ("_L1 + cols.join( ',' ) + ')';
      upsertSql += " VALUES "_L1;
      upsertSql += '(' + vals.join( ',' ) + ')';

      int result = SQLITE_ERROR;
      if ( layer && layer->dataProvider() && layer->dataProvider()->transaction() )
      {
        QgsTransaction *transaction = layer->dataProvider()->transaction();
        if ( transaction->executeSql( upsertSql, errorMessage ) )
        {
          result = SQLITE_OK;
        }
      }
      else
      {
        result = sqliteDb.exec( upsertSql, errorMessage );
      }
      if ( result == SQLITE_OK )
      {
        functionResult = QVariant( nextId );
        return;
      }
      else
      {
        parent->setEvalErrorString( u"Could not increment value: SQLite error: \"%1\" (%2)."_s.arg( errorMessage, QString::number( result ) ) );
        functionResult = QVariant();
        return;
      }
    }

    functionResult = QVariant();
  };

  bool foundLayer = false;
  QgsExpressionUtils::executeLambdaForMapLayer( values.at( 0 ), context, parent, [&fetchAndIncrementFunc]( QgsMapLayer * layer )
  {
    fetchAndIncrementFunc( layer, QString() );
  }, foundLayer );
  if ( !foundLayer )
  {
    const QString databasePath = values.at( 0 ).toString();
    QgsThreadingUtils::runOnMainThread( [&fetchAndIncrementFunc, databasePath]
    {
      fetchAndIncrementFunc( nullptr, databasePath );
    } );
  }

  return functionResult;
}

static QVariant fcnCrsToAuthid( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsCoordinateReferenceSystem crs = QgsExpressionUtils::getCrsValue( values.at( 0 ), parent );
  if ( !crs.isValid() )
    return QVariant();
  return crs.authid();
}

static QVariant fcnCrsFromText( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString definition = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  QgsCoordinateReferenceSystem crs( definition );

  if ( !crs.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to coordinate reference system" ).arg( definition ) );
  }

  return QVariant::fromValue( crs );
}

static QVariant fcnConcat( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString concat;
  for ( const QVariant &value : values )
  {
    if ( !QgsVariantUtils::isNull( value ) )
      concat += QgsExpressionUtils::getStringValue( value, parent );
  }
  return concat;
}

static QVariant fcnStrpos( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString string = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  return string.indexOf( QgsExpressionUtils::getStringValue( values.at( 1 ), parent ) ) + 1;
}

static QVariant fcnUnaccent(
  const QVariantList &values,
  const QgsExpressionContext *context,
  QgsExpression *,
  const QgsExpressionNodeFunction *node
)
{
  Q_UNUSED( context )
  Q_UNUSED( node )

  if ( values.isEmpty() || values[0].isNull() )
    return QVariant();

  return QgsStringUtils::unaccent( values[0].toString() );
}


static QVariant fcnRight( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString string = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  int pos = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  return string.right( pos );
}

static QVariant fcnSubstrCount( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( values.length() < 2 || values.length() > 3 )
    return QVariant();

  const QString input = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  const QString substring = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );

  bool overlapping = false;
  if ( values.length() == 3 )
  {
    overlapping = values.at( 2 ).toBool();
  }

  if ( substring.isEmpty() )
    return QVariant( 0 );

  int count = 0;
  if ( overlapping )
  {
    count = input.count( substring );
  }
  else
  {
    int pos = 0;
    while ( ( pos = input.indexOf( substring, pos ) ) != -1 )
    {
      count++;
      pos += substring.length();
    }
  }

  return QVariant( count );
}

static QVariant fcnLeft( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString string = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  int pos = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  return string.left( pos );
}

static QVariant fcnRPad( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString string = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  int length = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  QString fill = QgsExpressionUtils::getStringValue( values.at( 2 ), parent );
  return string.leftJustified( length, fill.at( 0 ), true );
}

static QVariant fcnLPad( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString string = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  int length = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  QString fill = QgsExpressionUtils::getStringValue( values.at( 2 ), parent );
  return string.rightJustified( length, fill.at( 0 ), true );
}

static QVariant fcnFormatString( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( values.size() < 1 )
  {
    parent->setEvalErrorString( QObject::tr( "Function format requires at least 1 argument" ) );
    return QVariant();
  }

  QString string = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  for ( int n = 1; n < values.length(); n++ )
  {
    string = string.arg( QgsExpressionUtils::getStringValue( values.at( n ), parent ) );
  }
  return string;
}


static QVariant fcnNow( const QVariantList &, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * )
{
  return QVariant( QDateTime::currentDateTime() );
}

static QVariant fcnToDate( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString format = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  QString language = QgsExpressionUtils::getStringValue( values.at( 2 ), parent );
  if ( format.isEmpty() && !language.isEmpty() )
  {
    parent->setEvalErrorString( QObject::tr( "A format is required to convert to Date when the language is specified" ) );
    return QVariant( QDate() );
  }

  if ( format.isEmpty() && language.isEmpty() )
    return QVariant( QgsExpressionUtils::getDateValue( values.at( 0 ), parent ) );

  QString datestring = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  QLocale locale = QLocale();
  if ( !language.isEmpty() )
  {
    locale = QLocale( language );
  }

  QDate date = locale.toDate( datestring, format );
  if ( !date.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to Date" ).arg( datestring ) );
    date = QDate();
  }
  return QVariant( date );
}

static QVariant fcnToTime( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString format = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  QString language = QgsExpressionUtils::getStringValue( values.at( 2 ), parent );
  if ( format.isEmpty() && !language.isEmpty() )
  {
    parent->setEvalErrorString( QObject::tr( "A format is required to convert to Time when the language is specified" ) );
    return QVariant( QTime() );
  }

  if ( format.isEmpty() && language.isEmpty() )
    return QVariant( QgsExpressionUtils::getTimeValue( values.at( 0 ), parent ) );

  QString timestring = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  QLocale locale = QLocale();
  if ( !language.isEmpty() )
  {
    locale = QLocale( language );
  }

  QTime time = locale.toTime( timestring, format );
  if ( !time.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to Time" ).arg( timestring ) );
    time = QTime();
  }
  return QVariant( time );
}

static QVariant fcnToInterval( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return QVariant::fromValue( QgsExpressionUtils::getInterval( values.at( 0 ), parent ) );
}

/*
 * DMS functions
 */

static QVariant floatToDegreeFormat( const QgsCoordinateFormatter::Format format, const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double value = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  QString axis = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  int precision = QgsExpressionUtils::getNativeIntValue( values.at( 2 ), parent );

  QString formatString;
  if ( values.count() > 3 )
    formatString = QgsExpressionUtils::getStringValue( values.at( 3 ), parent );

  QgsCoordinateFormatter::FormatFlags flags = QgsCoordinateFormatter::FormatFlags();
  if ( formatString.compare( "suffix"_L1, Qt::CaseInsensitive ) == 0 )
  {
    flags = QgsCoordinateFormatter::FlagDegreesUseStringSuffix;
  }
  else if ( formatString.compare( "aligned"_L1, Qt::CaseInsensitive ) == 0 )
  {
    flags = QgsCoordinateFormatter::FlagDegreesUseStringSuffix | QgsCoordinateFormatter::FlagDegreesPadMinutesSeconds;
  }
  else if ( ! formatString.isEmpty() )
  {
    parent->setEvalErrorString( QObject::tr( "Invalid formatting parameter: '%1'. It must be empty, or 'suffix' or 'aligned'." ).arg( formatString ) );
    return QVariant();
  }

  if ( axis.compare( 'x'_L1, Qt::CaseInsensitive ) == 0 )
  {
    return QVariant::fromValue( QgsCoordinateFormatter::formatX( value, format, precision, flags ) );
  }
  else if ( axis.compare( 'y'_L1, Qt::CaseInsensitive ) == 0 )
  {
    return QVariant::fromValue( QgsCoordinateFormatter::formatY( value, format, precision, flags ) );
  }
  else
  {
    parent->setEvalErrorString( QObject::tr( "Invalid axis name: '%1'. It must be either 'x' or 'y'." ).arg( axis ) );
    return QVariant();
  }
}

static QVariant fcnToDegreeMinute( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node )
{
  QgsCoordinateFormatter::Format format = QgsCoordinateFormatter::FormatDegreesMinutes;
  return floatToDegreeFormat( format, values, context, parent, node );
}

static QVariant fcnToDecimal( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double value = 0.0;
  bool ok = false;
  value = QgsCoordinateUtils::dmsToDecimal( QgsExpressionUtils::getStringValue( values.at( 0 ), parent ), &ok );

  return ok ? QVariant( value ) : QVariant();
}

static QVariant fcnToDegreeMinuteSecond( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node )
{
  QgsCoordinateFormatter::Format format = QgsCoordinateFormatter::FormatDegreesMinutesSeconds;
  return floatToDegreeFormat( format, values, context, parent, node );
}

static QVariant fcnExtractDegrees( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const double decimalDegrees = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  return static_cast< int >( decimalDegrees );
}

static QVariant fcnExtractMinutes( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const double absoluteDecimalDegrees = std::abs( QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent ) );
  const double remainder = absoluteDecimalDegrees - static_cast<int>( absoluteDecimalDegrees );
  return static_cast< int >( remainder * 60 );
}

static QVariant fcnExtractSeconds( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const double absoluteDecimalDegrees = std::abs( QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent ) );
  const double remainder = absoluteDecimalDegrees - static_cast<int>( absoluteDecimalDegrees );
  const double remainderInMinutes = remainder * 60;
  const double remainderSecondsFraction = remainderInMinutes - static_cast< int >( remainderInMinutes );
  // do not truncate to int, this function returns decimal seconds!
  return remainderSecondsFraction * 60;
}

static QVariant fcnAge( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QDateTime d1 = QgsExpressionUtils::getDateTimeValue( values.at( 0 ), parent );
  QDateTime d2 = QgsExpressionUtils::getDateTimeValue( values.at( 1 ), parent );
  qint64 seconds = d2.secsTo( d1 );
  return QVariant::fromValue( QgsInterval( seconds ) );
}

static QVariant fcnDayOfWeek( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( !values.at( 0 ).canConvert<QDate>() )
    return QVariant();

  QDate date = QgsExpressionUtils::getDateValue( values.at( 0 ), parent );
  if ( !date.isValid() )
    return QVariant();

  // return dayOfWeek() % 7 so that values range from 0 (sun) to 6 (sat)
  // (to match PostgreSQL behavior)
  return date.dayOfWeek() % 7;
}

static QVariant fcnDay( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariant value = values.at( 0 );
  QgsInterval inter = QgsExpressionUtils::getInterval( value, parent, false );
  if ( inter.isValid() )
  {
    return QVariant( inter.days() );
  }
  else
  {
    QDateTime d1 = QgsExpressionUtils::getDateTimeValue( value, parent );
    return QVariant( d1.date().day() );
  }
}

static QVariant fcnYear( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariant value = values.at( 0 );
  QgsInterval inter = QgsExpressionUtils::getInterval( value, parent, false );
  if ( inter.isValid() )
  {
    return QVariant( inter.years() );
  }
  else
  {
    QDateTime d1 = QgsExpressionUtils::getDateTimeValue( value, parent );
    return QVariant( d1.date().year() );
  }
}

static QVariant fcnMonth( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariant value = values.at( 0 );
  QgsInterval inter = QgsExpressionUtils::getInterval( value, parent, false );
  if ( inter.isValid() )
  {
    return QVariant( inter.months() );
  }
  else
  {
    QDateTime d1 = QgsExpressionUtils::getDateTimeValue( value, parent );
    return QVariant( d1.date().month() );
  }
}

static QVariant fcnWeek( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariant value = values.at( 0 );
  QgsInterval inter = QgsExpressionUtils::getInterval( value, parent, false );
  if ( inter.isValid() )
  {
    return QVariant( inter.weeks() );
  }
  else
  {
    QDateTime d1 = QgsExpressionUtils::getDateTimeValue( value, parent );
    return QVariant( d1.date().weekNumber() );
  }
}

static QVariant fcnHour( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariant value = values.at( 0 );
  QgsInterval inter = QgsExpressionUtils::getInterval( value, parent, false );
  if ( inter.isValid() )
  {
    return QVariant( inter.hours() );
  }
  else
  {
    QTime t1 = QgsExpressionUtils::getTimeValue( value, parent );
    return QVariant( t1.hour() );
  }
}

static QVariant fcnMinute( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariant value = values.at( 0 );
  QgsInterval inter = QgsExpressionUtils::getInterval( value, parent, false );
  if ( inter.isValid() )
  {
    return QVariant( inter.minutes() );
  }
  else
  {
    QTime t1 = QgsExpressionUtils::getTimeValue( value, parent );
    return QVariant( t1.minute() );
  }
}

static QVariant fcnSeconds( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariant value = values.at( 0 );
  QgsInterval inter = QgsExpressionUtils::getInterval( value, parent, false );
  if ( inter.isValid() )
  {
    return QVariant( inter.seconds() );
  }
  else
  {
    QTime t1 = QgsExpressionUtils::getTimeValue( value, parent );
    return QVariant( t1.second() );
  }
}

static QVariant fcnEpoch( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QDateTime dt = QgsExpressionUtils::getDateTimeValue( values.at( 0 ), parent );
  if ( dt.isValid() )
  {
    return QVariant( dt.toMSecsSinceEpoch() );
  }
  else
  {
    return QVariant();
  }
}

static QVariant fcnDateTimeFromEpoch( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  long long millisecs_since_epoch = QgsExpressionUtils::getIntValue( values.at( 0 ), parent );
  // no sense to check for strange values, as Qt behavior is undefined anyway (see docs)
  return QVariant( QDateTime::fromMSecsSinceEpoch( millisecs_since_epoch ) );
}

static QVariant fcnExif( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString filepath = QgsExpressionUtils::getFilePathValue( values.at( 0 ), context, parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( "exif"_L1 ) );
    return QVariant();
  }
  QString tag = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  return !tag.isNull() ? QgsExifTools::readTag( filepath, tag ) : QVariant( QgsExifTools::readTags( filepath ) );
}

static QVariant fcnExifGeoTag( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString filepath = QgsExpressionUtils::getFilePathValue( values.at( 0 ), context, parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( "exif_geotag"_L1 ) );
    return QVariant();
  }
  bool ok;
  return QVariant::fromValue( QgsGeometry( new QgsPoint( QgsExifTools::getGeoTag( filepath, ok ) ) ) );
}

#define ENSURE_GEOM_TYPE(f, g, geomtype) \
  if ( !(f).hasGeometry() ) \
    return QVariant(); \
  QgsGeometry g = (f).geometry(); \
  if ( (g).type() != (geomtype) ) \
    return QVariant();

static QVariant fcnX( const QVariantList &, const QgsExpressionContext *context, QgsExpression *, const QgsExpressionNodeFunction * )
{
  FEAT_FROM_CONTEXT( context, f )
  ENSURE_GEOM_TYPE( f, g, Qgis::GeometryType::Point )
  if ( g.isMultipart() )
  {
    return g.asMultiPoint().at( 0 ).x();
  }
  else
  {
    return g.asPoint().x();
  }
}

static QVariant fcnY( const QVariantList &, const QgsExpressionContext *context, QgsExpression *, const QgsExpressionNodeFunction * )
{
  FEAT_FROM_CONTEXT( context, f )
  ENSURE_GEOM_TYPE( f, g, Qgis::GeometryType::Point )
  if ( g.isMultipart() )
  {
    return g.asMultiPoint().at( 0 ).y();
  }
  else
  {
    return g.asPoint().y();
  }
}

static QVariant fcnZ( const QVariantList &, const QgsExpressionContext *context, QgsExpression *, const QgsExpressionNodeFunction * )
{
  FEAT_FROM_CONTEXT( context, f )
  ENSURE_GEOM_TYPE( f, g, Qgis::GeometryType::Point )

  if ( g.isEmpty() )
    return QVariant();

  const QgsAbstractGeometry *abGeom = g.constGet();

  if ( g.isEmpty() || !abGeom->is3D() )
    return QVariant();

  if ( g.type() == Qgis::GeometryType::Point && !g.isMultipart() )
  {
    const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( g.constGet() );
    if ( point )
      return point->z();
  }
  else if ( g.type() == Qgis::GeometryType::Point && g.isMultipart() )
  {
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( g.constGet() ) )
    {
      if ( collection->numGeometries() > 0 )
      {
        if ( const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( collection->geometryN( 0 ) ) )
          return point->z();
      }
    }
  }

  return QVariant();
}

static QVariant fcnGeomIsValid( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
    return QVariant();

  bool isValid = geom.isGeosValid();

  return QVariant( isValid );
}

static QVariant fcnGeomMakeValid( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
    return QVariant();

  const QString methodString = QgsExpressionUtils::getStringValue( values.at( 1 ), parent ).trimmed();
#if GEOS_VERSION_MAJOR==3 && GEOS_VERSION_MINOR<10
  Qgis::MakeValidMethod method = Qgis::MakeValidMethod::Linework;
#else
  Qgis::MakeValidMethod method = Qgis::MakeValidMethod::Structure;
#endif
  if ( methodString.compare( "linework"_L1, Qt::CaseInsensitive ) == 0 )
    method = Qgis::MakeValidMethod::Linework;
  else if ( methodString.compare( "structure"_L1, Qt::CaseInsensitive ) == 0 )
    method = Qgis::MakeValidMethod::Structure;

  const bool keepCollapsed = values.value( 2 ).toBool();

  QgsGeometry valid;
  try
  {
    valid = geom.makeValid( method, keepCollapsed );
  }
  catch ( QgsNotSupportedException & )
  {
    parent->setEvalErrorString( QObject::tr( "The make_valid parameters require a newer GEOS library version" ) );
    return QVariant();
  }

  return QVariant::fromValue( valid );
}

static QVariant fcnGeometryCollectionAsArray( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
    return QVariant();

  QVector<QgsGeometry> multiGeom = geom.asGeometryCollection();
  QVariantList array;
  for ( int i = 0; i < multiGeom.size(); ++i )
  {
    array += QVariant::fromValue( multiGeom.at( i ) );
  }

  return array;
}

static QVariant fcnGeomX( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
    return QVariant();

  //if single point, return the point's x coordinate
  if ( geom.type() == Qgis::GeometryType::Point && !geom.isMultipart() )
  {
    return geom.asPoint().x();
  }

  //otherwise return centroid x
  QgsGeometry centroid = geom.centroid();
  QVariant result( centroid.asPoint().x() );
  return result;
}

static QVariant fcnGeomY( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
    return QVariant();

  //if single point, return the point's y coordinate
  if ( geom.type() == Qgis::GeometryType::Point && !geom.isMultipart() )
  {
    return geom.asPoint().y();
  }

  //otherwise return centroid y
  QgsGeometry centroid = geom.centroid();
  QVariant result( centroid.asPoint().y() );
  return result;
}

static QVariant fcnGeomZ( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
    return QVariant(); //or 0?

  if ( !geom.constGet()->is3D() )
    return QVariant();

  //if single point, return the point's z coordinate
  if ( geom.type() == Qgis::GeometryType::Point && !geom.isMultipart() )
  {
    const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( geom.constGet() );
    if ( point )
      return point->z();
  }
  else if ( geom.type() == Qgis::GeometryType::Point && geom.isMultipart() )
  {
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( geom.constGet() ) )
    {
      if ( collection->numGeometries() == 1 )
      {
        if ( const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( collection->geometryN( 0 ) ) )
          return point->z();
      }
    }
  }

  return QVariant();
}

static QVariant fcnGeomM( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
    return QVariant(); //or 0?

  if ( !geom.constGet()->isMeasure() )
    return QVariant();

  //if single point, return the point's m value
  if ( geom.type() == Qgis::GeometryType::Point && !geom.isMultipart() )
  {
    const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( geom.constGet() );
    if ( point )
      return point->m();
  }
  else if ( geom.type() == Qgis::GeometryType::Point && geom.isMultipart() )
  {
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( geom.constGet() ) )
    {
      if ( collection->numGeometries() == 1 )
      {
        if ( const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( collection->geometryN( 0 ) ) )
          return point->m();
      }
    }
  }

  return QVariant();
}

static QVariant fcnPointN( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  int idx = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );

  if ( idx < 0 )
  {
    //negative idx
    int count = geom.constGet()->nCoordinates();
    idx = count + idx;
  }
  else
  {
    //positive idx is 1 based
    idx -= 1;
  }

  QgsVertexId vId;
  if ( idx < 0 || !geom.vertexIdFromVertexNr( idx, vId ) )
  {
    parent->setEvalErrorString( QObject::tr( "Point index is out of range" ) );
    return QVariant();
  }

  QgsPoint point = geom.constGet()->vertexAt( vId );
  return QVariant::fromValue( QgsGeometry( new QgsPoint( point ) ) );
}

static QVariant fcnStartPoint( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  QgsVertexId vId;
  if ( !geom.vertexIdFromVertexNr( 0, vId ) )
  {
    return QVariant();
  }

  QgsPoint point = geom.constGet()->vertexAt( vId );
  return QVariant::fromValue( QgsGeometry( new QgsPoint( point ) ) );
}

static QVariant fcnEndPoint( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  QgsVertexId vId;
  if ( !geom.vertexIdFromVertexNr( geom.constGet()->nCoordinates() - 1, vId ) )
  {
    return QVariant();
  }

  QgsPoint point = geom.constGet()->vertexAt( vId );
  return QVariant::fromValue( QgsGeometry( new QgsPoint( point ) ) );
}

static QVariant fcnNodesToPoints( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  bool ignoreClosing = false;
  if ( values.length() > 1 )
  {
    ignoreClosing = QgsExpressionUtils::getIntValue( values.at( 1 ), parent );
  }

  QgsMultiPoint *mp = new QgsMultiPoint();

  const QgsCoordinateSequence sequence = geom.constGet()->coordinateSequence();
  for ( const QgsRingSequence &part : sequence )
  {
    for ( const QgsPointSequence &ring : part )
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

static QVariant fcnSegmentsToLines( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  const QVector< QgsLineString * > linesToProcess = QgsGeometryUtils::extractLineStrings( geom.constGet() );

  //OK, now we have a complete list of segmentized lines from the geometry
  QgsMultiLineString *ml = new QgsMultiLineString();
  for ( QgsLineString *line : linesToProcess )
  {
    for ( int i = 0; i < line->numPoints() - 1; ++i )
    {
      QgsLineString *segment = new QgsLineString();
      segment->setPoints( QgsPointSequence()
                          << line->pointN( i )
                          << line->pointN( i + 1 ) );
      ml->addGeometry( segment );
    }
    delete line;
  }

  return QVariant::fromValue( QgsGeometry( ml ) );
}

static QVariant fcnInteriorRingN( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  const QgsCurvePolygon *curvePolygon = qgsgeometry_cast< const QgsCurvePolygon * >( geom.constGet() );
  if ( !curvePolygon && geom.isMultipart() )
  {
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( geom.constGet() ) )
    {
      if ( collection->numGeometries() == 1 )
      {
        curvePolygon = qgsgeometry_cast< const QgsCurvePolygon * >( collection->geometryN( 0 ) );
      }
    }
  }

  if ( !curvePolygon )
    return QVariant();

  //idx is 1 based
  qlonglong idx = QgsExpressionUtils::getIntValue( values.at( 1 ), parent ) - 1;

  if ( idx >= curvePolygon->numInteriorRings() || idx < 0 )
    return QVariant();

  QgsCurve *curve = static_cast< QgsCurve * >( curvePolygon->interiorRing( static_cast< int >( idx ) )->clone() );
  QVariant result = curve ? QVariant::fromValue( QgsGeometry( curve ) ) : QVariant();
  return result;
}

static QVariant fcnGeometryN( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( geom.constGet() );
  if ( !collection )
    return QVariant();

  //idx is 1 based
  qlonglong idx = QgsExpressionUtils::getIntValue( values.at( 1 ), parent ) - 1;

  if ( idx < 0 || idx >= collection->numGeometries() )
    return QVariant();

  QgsAbstractGeometry *part = collection->geometryN( static_cast< int >( idx ) )->clone();
  QVariant result = part ? QVariant::fromValue( QgsGeometry( part ) ) : QVariant();
  return result;
}

static QVariant fcnBoundary( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  QgsAbstractGeometry *boundary = geom.constGet()->boundary();
  if ( !boundary )
    return QVariant();

  return QVariant::fromValue( QgsGeometry( boundary ) );
}

static QVariant fcnLineMerge( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  QgsGeometry merged = geom.mergeLines();
  if ( merged.isNull() )
    return QVariant();

  return QVariant::fromValue( merged );
}

static QVariant fcnSharedPaths( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
    return QVariant();

  const QgsGeometry geom2 = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  if ( geom2.isNull() )
    return QVariant();

  const QgsGeometry sharedPaths = geom.sharedPaths( geom2 );
  if ( sharedPaths.isNull() )
    return QVariant();

  return QVariant::fromValue( sharedPaths );
}


static QVariant fcnSimplify( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  double tolerance = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );

  QgsGeometry simplified = geom.simplify( tolerance );
  if ( simplified.isNull() )
    return QVariant();

  return simplified;
}

static QVariant fcnSimplifyVW( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  double tolerance = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );

  QgsMapToPixelSimplifier simplifier( QgsMapToPixelSimplifier::SimplifyGeometry, tolerance, Qgis::VectorSimplificationAlgorithm::Visvalingam );

  QgsGeometry simplified = simplifier.simplify( geom );
  if ( simplified.isNull() )
    return QVariant();

  return simplified;
}

static QVariant fcnSmooth( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  int iterations = std::min( QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent ), 10 );
  double offset = std::clamp( QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent ), 0.0, 0.5 );
  double minLength = QgsExpressionUtils::getDoubleValue( values.at( 3 ), parent );
  double maxAngle = std::clamp( QgsExpressionUtils::getDoubleValue( values.at( 4 ), parent ), 0.0, 180.0 );

  QgsGeometry smoothed = geom.smooth( static_cast<unsigned int>( iterations ), offset, minLength, maxAngle );
  if ( smoothed.isNull() )
    return QVariant();

  return smoothed;
}

static QVariant fcnTriangularWave( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  const double wavelength = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  const double amplitude = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
  const bool strict = QgsExpressionUtils::getIntValue( values.at( 3 ), parent );

  const QgsGeometry waved = geom.triangularWaves( wavelength, amplitude, strict );
  if ( waved.isNull() )
    return QVariant();

  return waved;
}

static QVariant fcnTriangularWaveRandomized( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  const double minWavelength = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  const double maxWavelength = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
  const double minAmplitude = QgsExpressionUtils::getDoubleValue( values.at( 3 ), parent );
  const double maxAmplitude = QgsExpressionUtils::getDoubleValue( values.at( 4 ), parent );
  const long long seed = QgsExpressionUtils::getIntValue( values.at( 5 ), parent );

  const QgsGeometry waved = geom.triangularWavesRandomized( minWavelength, maxWavelength,
                            minAmplitude, maxAmplitude, seed );
  if ( waved.isNull() )
    return QVariant();

  return waved;
}

static QVariant fcnSquareWave( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  const double wavelength = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  const double amplitude = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
  const bool strict = QgsExpressionUtils::getIntValue( values.at( 3 ), parent );

  const QgsGeometry waved = geom.squareWaves( wavelength, amplitude, strict );
  if ( waved.isNull() )
    return QVariant();

  return waved;
}

static QVariant fcnSquareWaveRandomized( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  const double minWavelength = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  const double maxWavelength = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
  const double minAmplitude = QgsExpressionUtils::getDoubleValue( values.at( 3 ), parent );
  const double maxAmplitude = QgsExpressionUtils::getDoubleValue( values.at( 4 ), parent );
  const long long seed = QgsExpressionUtils::getIntValue( values.at( 5 ), parent );

  const QgsGeometry waved = geom.squareWavesRandomized( minWavelength, maxWavelength,
                            minAmplitude, maxAmplitude, seed );
  if ( waved.isNull() )
    return QVariant();

  return waved;
}

static QVariant fcnRoundWave( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  const double wavelength = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  const double amplitude = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
  const bool strict = QgsExpressionUtils::getIntValue( values.at( 3 ), parent );

  const QgsGeometry waved = geom.roundWaves( wavelength, amplitude, strict );
  if ( waved.isNull() )
    return QVariant();

  return waved;
}

static QVariant fcnRoundWaveRandomized( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  const double minWavelength = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  const double maxWavelength = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
  const double minAmplitude = QgsExpressionUtils::getDoubleValue( values.at( 3 ), parent );
  const double maxAmplitude = QgsExpressionUtils::getDoubleValue( values.at( 4 ), parent );
  const long long seed = QgsExpressionUtils::getIntValue( values.at( 5 ), parent );

  const QgsGeometry waved = geom.roundWavesRandomized( minWavelength, maxWavelength,
                            minAmplitude, maxAmplitude, seed );
  if ( waved.isNull() )
    return QVariant();

  return waved;
}

static QVariant fcnApplyDashPattern( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  const QVariantList pattern = QgsExpressionUtils::getListValue( values.at( 1 ), parent );
  QVector< double > dashPattern;
  dashPattern.reserve( pattern.size() );
  for ( const QVariant &value : std::as_const( pattern ) )
  {
    bool ok = false;
    double v = value.toDouble( &ok );
    if ( ok )
    {
      dashPattern << v;
    }
    else
    {
      parent->setEvalErrorString( u"Dash pattern must be an array of numbers"_s );
      return QgsGeometry();
    }
  }

  if ( dashPattern.size() % 2 != 0 )
  {
    parent->setEvalErrorString( u"Dash pattern must contain an even number of elements"_s );
    return QgsGeometry();
  }

  const QString startRuleString = QgsExpressionUtils::getStringValue( values.at( 2 ), parent ).trimmed();
  Qgis::DashPatternLineEndingRule startRule = Qgis::DashPatternLineEndingRule::NoRule;
  if ( startRuleString.compare( "no_rule"_L1, Qt::CaseInsensitive ) == 0 )
    startRule = Qgis::DashPatternLineEndingRule::NoRule;
  else if ( startRuleString.compare( "full_dash"_L1, Qt::CaseInsensitive ) == 0 )
    startRule = Qgis::DashPatternLineEndingRule::FullDash;
  else if ( startRuleString.compare( "half_dash"_L1, Qt::CaseInsensitive ) == 0 )
    startRule = Qgis::DashPatternLineEndingRule::HalfDash;
  else if ( startRuleString.compare( "full_gap"_L1, Qt::CaseInsensitive ) == 0 )
    startRule = Qgis::DashPatternLineEndingRule::FullGap;
  else if ( startRuleString.compare( "half_gap"_L1, Qt::CaseInsensitive ) == 0 )
    startRule = Qgis::DashPatternLineEndingRule::HalfGap;
  else
  {
    parent->setEvalErrorString( u"'%1' is not a valid dash pattern rule"_s.arg( startRuleString ) );
    return QgsGeometry();
  }

  const QString endRuleString = QgsExpressionUtils::getStringValue( values.at( 3 ), parent ).trimmed();
  Qgis::DashPatternLineEndingRule endRule = Qgis::DashPatternLineEndingRule::NoRule;
  if ( endRuleString.compare( "no_rule"_L1, Qt::CaseInsensitive ) == 0 )
    endRule = Qgis::DashPatternLineEndingRule::NoRule;
  else if ( endRuleString.compare( "full_dash"_L1, Qt::CaseInsensitive ) == 0 )
    endRule = Qgis::DashPatternLineEndingRule::FullDash;
  else if ( endRuleString.compare( "half_dash"_L1, Qt::CaseInsensitive ) == 0 )
    endRule = Qgis::DashPatternLineEndingRule::HalfDash;
  else if ( endRuleString.compare( "full_gap"_L1, Qt::CaseInsensitive ) == 0 )
    endRule = Qgis::DashPatternLineEndingRule::FullGap;
  else if ( endRuleString.compare( "half_gap"_L1, Qt::CaseInsensitive ) == 0 )
    endRule = Qgis::DashPatternLineEndingRule::HalfGap;
  else
  {
    parent->setEvalErrorString( u"'%1' is not a valid dash pattern rule"_s.arg( endRuleString ) );
    return QgsGeometry();
  }

  const QString adjustString = QgsExpressionUtils::getStringValue( values.at( 4 ), parent ).trimmed();
  Qgis::DashPatternSizeAdjustment adjustment = Qgis::DashPatternSizeAdjustment::ScaleBothDashAndGap;
  if ( adjustString.compare( "both"_L1, Qt::CaseInsensitive ) == 0 )
    adjustment = Qgis::DashPatternSizeAdjustment::ScaleBothDashAndGap;
  else if ( adjustString.compare( "dash"_L1, Qt::CaseInsensitive ) == 0 )
    adjustment = Qgis::DashPatternSizeAdjustment::ScaleDashOnly;
  else if ( adjustString.compare( "gap"_L1, Qt::CaseInsensitive ) == 0 )
    adjustment = Qgis::DashPatternSizeAdjustment::ScaleGapOnly;
  else
  {
    parent->setEvalErrorString( u"'%1' is not a valid dash pattern size adjustment"_s.arg( adjustString ) );
    return QgsGeometry();
  }

  const double patternOffset = QgsExpressionUtils::getDoubleValue( values.at( 5 ), parent );

  const QgsGeometry result = geom.applyDashPattern( dashPattern, startRule, endRule, adjustment, patternOffset );
  if ( result.isNull() )
    return QVariant();

  return result;
}

static QVariant fcnDensifyByCount( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  const long long count = QgsExpressionUtils::getIntValue( values.at( 1 ), parent );
  const QgsGeometry densified = geom.densifyByCount( static_cast< int >( count ) );
  if ( densified.isNull() )
    return QVariant();

  return densified;
}

static QVariant fcnDensifyByDistance( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  const double distance = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  const QgsGeometry densified = geom.densifyByDistance( distance );
  if ( densified.isNull() )
    return QVariant();

  return densified;
}

static QVariant fcnCollectGeometries( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantList list;
  if ( values.size() == 1 && QgsExpressionUtils::isList( values.at( 0 ) ) )
  {
    list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  }
  else
  {
    list = values;
  }

  QVector< QgsGeometry > parts;
  parts.reserve( list.size() );
  for ( const QVariant &value : std::as_const( list ) )
  {
    QgsGeometry part = QgsExpressionUtils::getGeometry( value, parent );
    if ( part.isNull() )
      return QgsGeometry();
    parts << part;
  }

  return QgsGeometry::collectGeometry( parts );
}

static QVariant fcnMakePoint( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( values.count() < 2 || values.count() > 4 )
  {
    parent->setEvalErrorString( QObject::tr( "Function make_point requires 2-4 arguments" ) );
    return QVariant();
  }

  double x = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  double y = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  double z = values.count() >= 3 ? QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent ) : 0.0;
  double m = values.count() >= 4 ? QgsExpressionUtils::getDoubleValue( values.at( 3 ), parent ) : 0.0;
  switch ( values.count() )
  {
    case 2:
      return QVariant::fromValue( QgsGeometry( new QgsPoint( x, y ) ) );
    case 3:
      return QVariant::fromValue( QgsGeometry( new QgsPoint( Qgis::WkbType::PointZ, x, y, z ) ) );
    case 4:
      return QVariant::fromValue( QgsGeometry( new QgsPoint( Qgis::WkbType::PointZM, x, y, z, m ) ) );
  }
  return QVariant(); //avoid warning
}

static QVariant fcnMakePointM( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double x = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  double y = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  double m = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
  return QVariant::fromValue( QgsGeometry( new QgsPoint( Qgis::WkbType::PointM, x, y, 0.0, m ) ) );
}

static QVariant fcnMakeLine( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( values.empty() )
  {
    return QVariant();
  }

  QVector<QgsPoint> points;
  points.reserve( values.count() );

  auto addPoint = [&points]( const QgsGeometry & geom )
  {
    if ( geom.isNull() )
      return;

    if ( geom.type() != Qgis::GeometryType::Point || geom.isMultipart() )
      return;

    const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( geom.constGet() );
    if ( !point )
      return;

    points << *point;
  };

  for ( const QVariant &value : values )
  {
    if ( value.userType() == QMetaType::Type::QVariantList )
    {
      const QVariantList list = value.toList();
      for ( const QVariant &v : list )
      {
        addPoint( QgsExpressionUtils::getGeometry( v, parent ) );
      }
    }
    else
    {
      addPoint( QgsExpressionUtils::getGeometry( value, parent ) );
    }
  }

  if ( points.count() < 2 )
    return QVariant();

  return QgsGeometry( new QgsLineString( points ) );
}

static QVariant fcnMakePolygon( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( values.count() < 1 )
  {
    parent->setEvalErrorString( QObject::tr( "Function make_polygon requires an argument" ) );
    return QVariant();
  }

  QgsGeometry outerRing = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( outerRing.type() == Qgis::GeometryType::Polygon )
    return outerRing; // if it's already a polygon we have nothing to do

  if ( outerRing.type() != Qgis::GeometryType::Line || outerRing.isNull() )
    return QVariant();

  auto polygon = std::make_unique< QgsPolygon >();

  const QgsCurve *exteriorRing = qgsgeometry_cast< const QgsCurve * >( outerRing.constGet() );
  if ( !exteriorRing && outerRing.isMultipart() )
  {
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( outerRing.constGet() ) )
    {
      if ( collection->numGeometries() == 1 )
      {
        exteriorRing = qgsgeometry_cast< const QgsCurve * >( collection->geometryN( 0 ) );
      }
    }
  }

  if ( !exteriorRing )
    return QVariant();

  polygon->setExteriorRing( exteriorRing->segmentize() );


  for ( int i = 1; i < values.count(); ++i )
  {
    QgsGeometry ringGeom = QgsExpressionUtils::getGeometry( values.at( i ), parent );
    if ( ringGeom.isNull() )
      continue;

    if ( ringGeom.type() != Qgis::GeometryType::Line || ringGeom.isNull() )
      continue;

    const QgsCurve *ring = qgsgeometry_cast< const QgsCurve * >( ringGeom.constGet() );
    if ( !ring && ringGeom.isMultipart() )
    {
      if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( ringGeom.constGet() ) )
      {
        if ( collection->numGeometries() == 1 )
        {
          ring = qgsgeometry_cast< const QgsCurve * >( collection->geometryN( 0 ) );
        }
      }
    }

    if ( !ring )
      continue;

    polygon->addInteriorRing( ring->segmentize() );
  }

  return QVariant::fromValue( QgsGeometry( std::move( polygon ) ) );
}

static QVariant fcnMakeTriangle( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  auto tr = std::make_unique<QgsTriangle>();
  auto lineString = std::make_unique<QgsLineString>();
  lineString->clear();

  for ( const QVariant &value : values )
  {
    QgsGeometry geom = QgsExpressionUtils::getGeometry( value, parent );
    if ( geom.isNull() )
      return QVariant();

    if ( geom.type() != Qgis::GeometryType::Point || geom.isMultipart() )
      return QVariant();

    const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( geom.constGet() );
    if ( !point && geom.isMultipart() )
    {
      if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( geom.constGet() ) )
      {
        if ( collection->numGeometries() == 1 )
        {
          point = qgsgeometry_cast< const QgsPoint * >( collection->geometryN( 0 ) );
        }
      }
    }

    if ( !point )
      return QVariant();

    lineString->addVertex( *point );
  }

  tr->setExteriorRing( lineString.release() );

  return QVariant::fromValue( QgsGeometry( tr.release() ) );
}

static QVariant fcnMakeCircle( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
    return QVariant();

  if ( geom.type() != Qgis::GeometryType::Point || geom.isMultipart() )
    return QVariant();

  double radius = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  int segment = QgsExpressionUtils::getNativeIntValue( values.at( 2 ), parent );

  if ( segment < 3 )
  {
    parent->setEvalErrorString( QObject::tr( "Segment must be greater than 2" ) );
    return QVariant();
  }
  const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( geom.constGet() );
  if ( !point && geom.isMultipart() )
  {
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( geom.constGet() ) )
    {
      if ( collection->numGeometries() == 1 )
      {
        point = qgsgeometry_cast< const QgsPoint * >( collection->geometryN( 0 ) );
      }
    }
  }
  if ( !point )
    return QVariant();

  QgsCircle circ( *point, radius );
  return QVariant::fromValue( QgsGeometry( circ.toPolygon( static_cast<unsigned int>( segment ) ) ) );
}

static QVariant fcnMakeEllipse( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
    return QVariant();

  if ( geom.type() != Qgis::GeometryType::Point || geom.isMultipart() )
    return QVariant();

  double majorAxis = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  double minorAxis = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
  double azimuth = QgsExpressionUtils::getDoubleValue( values.at( 3 ), parent );
  int segment = QgsExpressionUtils::getNativeIntValue( values.at( 4 ), parent );
  if ( segment < 3 )
  {
    parent->setEvalErrorString( QObject::tr( "Segment must be greater than 2" ) );
    return QVariant();
  }
  const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( geom.constGet() );
  if ( !point && geom.isMultipart() )
  {
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( geom.constGet() ) )
    {
      if ( collection->numGeometries() == 1 )
      {
        point = qgsgeometry_cast< const QgsPoint * >( collection->geometryN( 0 ) );
      }
    }
  }
  if ( !point )
    return QVariant();

  QgsEllipse elp( *point, majorAxis,  minorAxis, azimuth );
  return QVariant::fromValue( QgsGeometry( elp.toPolygon( static_cast<unsigned int>( segment ) ) ) );
}

static QVariant fcnMakeRegularPolygon( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{

  QgsGeometry pt1 = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( pt1.isNull() )
    return QVariant();

  if ( pt1.type() != Qgis::GeometryType::Point || pt1.isMultipart() )
    return QVariant();

  QgsGeometry pt2 = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  if ( pt2.isNull() )
    return QVariant();

  if ( pt2.type() != Qgis::GeometryType::Point || pt2.isMultipart() )
    return QVariant();

  unsigned int nbEdges = static_cast<unsigned int>( QgsExpressionUtils::getIntValue( values.at( 2 ), parent ) );
  if ( nbEdges < 3 )
  {
    parent->setEvalErrorString( QObject::tr( "Number of edges/sides must be greater than 2" ) );
    return QVariant();
  }

  QgsRegularPolygon::ConstructionOption option = static_cast< QgsRegularPolygon::ConstructionOption >( QgsExpressionUtils::getIntValue( values.at( 3 ), parent ) );
  if ( ( option < QgsRegularPolygon::InscribedCircle ) || ( option > QgsRegularPolygon::CircumscribedCircle ) )
  {
    parent->setEvalErrorString( QObject::tr( "Option can be 0 (inscribed) or 1 (circumscribed)" ) );
    return QVariant();
  }

  const QgsPoint *center = qgsgeometry_cast< const QgsPoint * >( pt1.constGet() );
  if ( !center && pt1.isMultipart() )
  {
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( pt1.constGet() ) )
    {
      if ( collection->numGeometries() == 1 )
      {
        center = qgsgeometry_cast< const QgsPoint * >( collection->geometryN( 0 ) );
      }
    }
  }
  if ( !center )
    return QVariant();

  const QgsPoint *corner = qgsgeometry_cast< const QgsPoint * >( pt2.constGet() );
  if ( !corner && pt2.isMultipart() )
  {
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( pt2.constGet() ) )
    {
      if ( collection->numGeometries() == 1 )
      {
        corner = qgsgeometry_cast< const QgsPoint * >( collection->geometryN( 0 ) );
      }
    }
  }
  if ( !corner )
    return QVariant();

  QgsRegularPolygon rp = QgsRegularPolygon( *center, *corner, nbEdges, option );

  return QVariant::fromValue( QgsGeometry( rp.toPolygon() ) );

}

static QVariant fcnMakeSquare( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry pt1 = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( pt1.isNull() )
    return QVariant();
  if ( pt1.type() != Qgis::GeometryType::Point || pt1.isMultipart() )
    return QVariant();

  QgsGeometry pt2 = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  if ( pt2.isNull() )
    return QVariant();
  if ( pt2.type() != Qgis::GeometryType::Point || pt2.isMultipart() )
    return QVariant();

  const QgsPoint *point1 = qgsgeometry_cast< const QgsPoint *>( pt1.constGet() );
  const QgsPoint *point2 = qgsgeometry_cast< const QgsPoint *>( pt2.constGet() );
  QgsQuadrilateral square = QgsQuadrilateral::squareFromDiagonal( *point1, *point2 );

  return QVariant::fromValue( QgsGeometry( square.toPolygon() ) );
}

static QVariant fcnMakeRectangleFrom3Points( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry pt1 = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( pt1.isNull() )
    return QVariant();
  if ( pt1.type() != Qgis::GeometryType::Point || pt1.isMultipart() )
    return QVariant();

  QgsGeometry pt2 = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  if ( pt2.isNull() )
    return QVariant();
  if ( pt2.type() != Qgis::GeometryType::Point || pt2.isMultipart() )
    return QVariant();

  QgsGeometry pt3 = QgsExpressionUtils::getGeometry( values.at( 2 ), parent );
  if ( pt3.isNull() )
    return QVariant();
  if ( pt3.type() != Qgis::GeometryType::Point || pt3.isMultipart() )
    return QVariant();

  QgsQuadrilateral::ConstructionOption option = static_cast< QgsQuadrilateral::ConstructionOption >( QgsExpressionUtils::getIntValue( values.at( 3 ), parent ) );
  if ( ( option < QgsQuadrilateral::Distance ) || ( option > QgsQuadrilateral::Projected ) )
  {
    parent->setEvalErrorString( QObject::tr( "Option can be 0 (distance) or 1 (projected)" ) );
    return QVariant();
  }
  const QgsPoint *point1 = qgsgeometry_cast< const QgsPoint *>( pt1.constGet() );
  const QgsPoint *point2 = qgsgeometry_cast< const QgsPoint *>( pt2.constGet() );
  const QgsPoint *point3 = qgsgeometry_cast< const QgsPoint *>( pt3.constGet() );
  QgsQuadrilateral rect = QgsQuadrilateral::rectangleFrom3Points( *point1, *point2, *point3, option );
  return QVariant::fromValue( QgsGeometry( rect.toPolygon() ) );
}

static QVariant pointAt( const QgsGeometry &geom, int idx, QgsExpression *parent ) // helper function
{
  if ( geom.isNull() )
    return QVariant();

  if ( idx < 0 )
  {
    idx += geom.constGet()->nCoordinates();
  }
  if ( idx < 0 || idx >= geom.constGet()->nCoordinates() )
  {
    parent->setEvalErrorString( QObject::tr( "Index is out of range" ) );
    return QVariant();
  }
  return QVariant::fromValue( geom.vertexAt( idx ) );
}

// function used for the old $ style
static QVariant fcnOldXat( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  FEAT_FROM_CONTEXT( context, feature )
  const QgsGeometry geom = feature.geometry();
  const int idx = QgsExpressionUtils::getNativeIntValue( values.at( 0 ), parent );

  const QVariant v = pointAt( geom, idx, parent );

  if ( !v.isNull() )
    return QVariant( v.value<QgsPoint>().x() );
  else
    return QVariant();
}
static QVariant fcnXat( const QVariantList &values, const QgsExpressionContext *f, QgsExpression *parent, const QgsExpressionNodeFunction *node )
{
  if ( values.at( 1 ).isNull() && !values.at( 0 ).isNull() ) // the case where the alias x_at function is called like a $ function (x_at(i))
  {
    return fcnOldXat( values, f, parent, node );
  }
  else if ( values.at( 0 ).isNull() && !values.at( 1 ).isNull() ) // same as above with x_at(i:=0) (vertex value is at the second position)
  {
    return fcnOldXat( QVariantList() << values[1], f, parent, node );
  }

  const QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
  {
    return QVariant();
  }

  const int vertexNumber = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );

  const QVariant v = pointAt( geom, vertexNumber, parent );
  if ( !v.isNull() )
    return QVariant( v.value<QgsPoint>().x() );
  else
    return QVariant();
}

// function used for the old $ style
static QVariant fcnOldYat( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  FEAT_FROM_CONTEXT( context, feature )
  const QgsGeometry geom = feature.geometry();
  const int idx = QgsExpressionUtils::getNativeIntValue( values.at( 0 ), parent );

  const QVariant v = pointAt( geom, idx, parent );

  if ( !v.isNull() )
    return QVariant( v.value<QgsPoint>().y() );
  else
    return QVariant();
}
static QVariant fcnYat( const QVariantList &values, const QgsExpressionContext *f, QgsExpression *parent, const QgsExpressionNodeFunction *node )
{
  if ( values.at( 1 ).isNull() && !values.at( 0 ).isNull() ) // the case where the alias y_at function is called like a $ function (y_at(i))
  {
    return fcnOldYat( values, f, parent, node );
  }
  else if ( values.at( 0 ).isNull() && !values.at( 1 ).isNull() ) // same as above with x_at(i:=0) (vertex value is at the second position)
  {
    return fcnOldYat( QVariantList() << values[1], f, parent, node );
  }

  const QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
  {
    return QVariant();
  }

  const int vertexNumber = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );

  const QVariant v = pointAt( geom, vertexNumber, parent );
  if ( !v.isNull() )
    return QVariant( v.value<QgsPoint>().y() );
  else
    return QVariant();
}

static QVariant fcnZat( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
  {
    return QVariant();
  }

  const int vertexNumber = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );

  const QVariant v = pointAt( geom, vertexNumber, parent );
  if ( !v.isNull() && v.value<QgsPoint>().is3D() )
    return QVariant( v.value<QgsPoint>().z() );
  else
    return QVariant();
}

static QVariant fcnMat( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
  {
    return QVariant();
  }

  const int vertexNumber = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );

  const QVariant v = pointAt( geom, vertexNumber, parent );
  if ( !v.isNull() && v.value<QgsPoint>().isMeasure() )
    return QVariant( v.value<QgsPoint>().m() );
  else
    return QVariant();
}


static QVariant fcnGeometry( const QVariantList &, const QgsExpressionContext *context, QgsExpression *, const QgsExpressionNodeFunction * )
{
  if ( !context )
    return QVariant();

  // prefer geometry from context if it's present, otherwise fallback to context's feature's geometry
  if ( context->hasGeometry() )
    return context->geometry();
  else
  {
    FEAT_FROM_CONTEXT( context, f )
    QgsGeometry geom = f.geometry();
    if ( !geom.isNull() )
      return  QVariant::fromValue( geom );
    else
      return QVariant();
  }
}

static QVariant fcnGeomFromWKT( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString wkt = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  QgsGeometry geom = QgsGeometry::fromWkt( wkt );
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnGeomFromWKB( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QByteArray wkb = QgsExpressionUtils::getBinaryValue( values.at( 0 ), parent );
  if ( wkb.isNull() )
    return QVariant();

  QgsGeometry geom;
  geom.fromWkb( wkb );
  return !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
}

static QVariant fcnGeomFromGML( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString gml = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  QgsOgcUtils::Context ogcContext;
  if ( context )
  {
    QgsWeakMapLayerPointer mapLayerPtr {context->variable( u"layer"_s ).value<QgsWeakMapLayerPointer>() };
    if ( mapLayerPtr )
    {
      ogcContext.layer = mapLayerPtr.data();
      ogcContext.transformContext = context->variable( u"_project_transform_context"_s ).value<QgsCoordinateTransformContext>();
    }
  }
  QgsGeometry geom = QgsOgcUtils::geometryFromGML( gml, ogcContext );
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnGeomArea( const QVariantList &, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  FEAT_FROM_CONTEXT( context, f )
  ENSURE_GEOM_TYPE( f, g, Qgis::GeometryType::Polygon )
  QgsDistanceArea *calc = parent->geomCalculator();
  if ( calc )
  {
    try
    {
      double area = calc->measureArea( f.geometry() );
      area = calc->convertAreaMeasurement( area, parent->areaUnits() );
      return QVariant( area );
    }
    catch ( QgsCsException & )
    {
      parent->setEvalErrorString( QObject::tr( "An error occurred while calculating area" ) );
      return QVariant();
    }
  }
  else
  {
    return QVariant( f.geometry().area() );
  }
}

static QVariant fcnArea( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.type() != Qgis::GeometryType::Polygon )
    return QVariant();

  return QVariant( geom.area() );
}

static QVariant fcnGeomLength( const QVariantList &, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  FEAT_FROM_CONTEXT( context, f )
  ENSURE_GEOM_TYPE( f, g, Qgis::GeometryType::Line )
  QgsDistanceArea *calc = parent->geomCalculator();
  if ( calc )
  {
    try
    {
      double len = calc->measureLength( f.geometry() );
      len = calc->convertLengthMeasurement( len, parent->distanceUnits() );
      return QVariant( len );
    }
    catch ( QgsCsException & )
    {
      parent->setEvalErrorString( QObject::tr( "An error occurred while calculating length" ) );
      return QVariant();
    }
  }
  else
  {
    return QVariant( f.geometry().length() );
  }
}

static QVariant fcnGeomPerimeter( const QVariantList &, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  FEAT_FROM_CONTEXT( context, f )
  ENSURE_GEOM_TYPE( f, g, Qgis::GeometryType::Polygon )
  QgsDistanceArea *calc = parent->geomCalculator();
  if ( calc )
  {
    try
    {
      double len = calc->measurePerimeter( f.geometry() );
      len = calc->convertLengthMeasurement( len, parent->distanceUnits() );
      return QVariant( len );
    }
    catch ( QgsCsException & )
    {
      parent->setEvalErrorString( QObject::tr( "An error occurred while calculating perimeter" ) );
      return QVariant();
    }
  }
  else
  {
    return f.geometry().isNull() ? QVariant( 0 ) : QVariant( f.geometry().constGet()->perimeter() );
  }
}

static QVariant fcnPerimeter( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.type() != Qgis::GeometryType::Polygon )
    return QVariant();

  //length for polygons = perimeter
  return QVariant( geom.length() );
}

static QVariant fcnGeomNumPoints( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  return QVariant( geom.isNull() ? 0 : geom.constGet()->nCoordinates() );
}

static QVariant fcnGeomNumGeometries( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
    return QVariant();

  return QVariant( geom.constGet()->partCount() );
}

static QVariant fcnGeomIsMultipart( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
    return QVariant();

  return QVariant( geom.isMultipart() );
}

static QVariant fcnGeomNumInteriorRings( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  const QgsCurvePolygon *curvePolygon = qgsgeometry_cast< const QgsCurvePolygon * >( geom.constGet() );
  if ( curvePolygon )
    return QVariant( curvePolygon->numInteriorRings() );

  const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( geom.constGet() );
  if ( collection )
  {
    //find first CurvePolygon in collection
    for ( int i = 0; i < collection->numGeometries(); ++i )
    {
      curvePolygon = qgsgeometry_cast< const QgsCurvePolygon *>( collection->geometryN( i ) );
      if ( !curvePolygon )
        continue;

      return QVariant( curvePolygon->isEmpty() ? 0 : curvePolygon->numInteriorRings() );
    }
  }

  return QVariant();
}

static QVariant fcnGeomNumRings( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  const QgsCurvePolygon *curvePolygon = qgsgeometry_cast< const QgsCurvePolygon * >( geom.constGet() );
  if ( curvePolygon )
    return QVariant( curvePolygon->ringCount() );

  bool foundPoly = false;
  int ringCount = 0;
  const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( geom.constGet() );
  if ( collection )
  {
    //find CurvePolygons in collection
    for ( int i = 0; i < collection->numGeometries(); ++i )
    {
      curvePolygon = qgsgeometry_cast< const QgsCurvePolygon *>( collection->geometryN( i ) );
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

static QVariant fcnBounds( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry geomBounds = QgsGeometry::fromRect( geom.boundingBox() );
  QVariant result = !geomBounds.isNull() ? QVariant::fromValue( geomBounds ) : QVariant();
  return result;
}

static QVariant fcnBoundsWidth( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  return QVariant::fromValue( geom.boundingBox().width() );
}

static QVariant fcnBoundsHeight( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  return QVariant::fromValue( geom.boundingBox().height() );
}

static QVariant fcnGeometryType( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
    return QVariant();

  return QgsWkbTypes::geometryDisplayString( geom.type() );
}

static QVariant fcnXMin( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  return QVariant::fromValue( geom.boundingBox().xMinimum() );
}

static QVariant fcnXMax( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  return QVariant::fromValue( geom.boundingBox().xMaximum() );
}

static QVariant fcnYMin( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  return QVariant::fromValue( geom.boundingBox().yMinimum() );
}

static QVariant fcnYMax( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  return QVariant::fromValue( geom.boundingBox().yMaximum() );
}

static QVariant fcnZMax( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() || geom.isEmpty( ) )
    return QVariant();

  if ( !geom.constGet()->is3D() )
    return QVariant();

  double max = std::numeric_limits< double >::lowest();

  for ( auto it = geom.vertices_begin(); it != geom.vertices_end(); ++it )
  {
    double z = ( *it ).z();

    if ( max < z )
      max = z;
  }

  if ( max == std::numeric_limits< double >::lowest() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::Double );

  return QVariant( max );
}

static QVariant fcnZMin( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() || geom.isEmpty() )
    return QVariant();

  if ( !geom.constGet()->is3D() )
    return QVariant();

  double min = std::numeric_limits< double >::max();

  for ( auto it = geom.vertices_begin(); it != geom.vertices_end(); ++it )
  {
    double z = ( *it ).z();

    if ( z < min )
      min = z;
  }

  if ( min == std::numeric_limits< double >::max() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::Double );

  return QVariant( min );
}

static QVariant fcnMMin( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() || geom.isEmpty() )
    return QVariant();

  if ( !geom.constGet()->isMeasure() )
    return QVariant();

  double min = std::numeric_limits< double >::max();

  for ( auto it = geom.vertices_begin(); it != geom.vertices_end(); ++it )
  {
    double m = ( *it ).m();

    if ( m < min )
      min = m;
  }

  if ( min == std::numeric_limits< double >::max() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::Double );

  return QVariant( min );
}

static QVariant fcnMMax( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() || geom.isEmpty() )
    return QVariant();

  if ( !geom.constGet()->isMeasure() )
    return QVariant();

  double max = std::numeric_limits< double >::lowest();

  for ( auto it = geom.vertices_begin(); it != geom.vertices_end(); ++it )
  {
    double m = ( *it ).m();

    if ( max < m )
      max = m;
  }

  if ( max == std::numeric_limits< double >::lowest() )
    return QgsVariantUtils::createNullVariant( QMetaType::Type::Double );

  return QVariant( max );
}

static QVariant fcnSinuosity( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  const QgsCurve *curve = qgsgeometry_cast< const QgsCurve * >( geom.constGet() );
  if ( !curve )
  {
    parent->setEvalErrorString( QObject::tr( "Function `sinuosity` requires a line geometry." ) );
    return QVariant();
  }

  return QVariant( curve->sinuosity() );
}

static QVariant fcnStraightDistance2d( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  const QgsCurve *curve = geom.constGet() ? qgsgeometry_cast< const QgsCurve * >( geom.constGet()->simplifiedTypeRef() ) : nullptr;
  if ( !curve )
  {
    parent->setEvalErrorString( QObject::tr( "Function `straight_distance_2d` requires a line geometry or a multi line geometry with a single part." ) );
    return QVariant();
  }

  return QVariant( curve->straightDistance2d() );
}

static QVariant fcnRoundness( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  const QgsCurvePolygon *poly = geom.constGet() ? qgsgeometry_cast< const QgsCurvePolygon * >( geom.constGet()->simplifiedTypeRef() ) : nullptr;

  if ( !poly )
  {
    parent->setEvalErrorString( QObject::tr( "Function `roundness` requires a polygon geometry or a multi polygon geometry with a single part." ) );
    return QVariant();
  }

  return QVariant( poly->roundness() );
}



static QVariant fcnFlipCoordinates( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
    return QVariant();

  std::unique_ptr< QgsAbstractGeometry > flipped( geom.constGet()->clone() );
  flipped->swapXy();
  return QVariant::fromValue( QgsGeometry( std::move( flipped ) ) );
}

static QVariant fcnIsClosed( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( fGeom.isNull() )
    return QVariant();

  const QgsCurve *curve = qgsgeometry_cast< const QgsCurve * >( fGeom.constGet() );
  if ( !curve && fGeom.isMultipart() )
  {
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( fGeom.constGet() ) )
    {
      if ( collection->numGeometries() == 1 )
      {
        curve = qgsgeometry_cast< const QgsCurve * >( collection->geometryN( 0 ) );
      }
    }
  }

  if ( !curve )
    return QVariant();

  return QVariant::fromValue( curve->isClosed() );
}

static QVariant fcnCloseLine( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.isNull() )
    return QVariant();

  QVariant result;
  if ( !geom.isMultipart() )
  {
    const QgsLineString *line = qgsgeometry_cast<const QgsLineString * >( geom.constGet() );

    if ( !line )
      return QVariant();

    std::unique_ptr< QgsLineString > closedLine( line->clone() );
    closedLine->close();

    result = QVariant::fromValue( QgsGeometry( std::move( closedLine ) ) );
  }
  else
  {
    const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection *>( geom.constGet() );
    if ( !collection )
      return QVariant();

    std::unique_ptr< QgsGeometryCollection > closed( collection->createEmptyWithSameType() );

    for ( int i = 0; i < collection->numGeometries(); ++i )
    {
      if ( const QgsLineString *line = qgsgeometry_cast<const QgsLineString * >( collection->geometryN( i ) ) )
      {
        std::unique_ptr< QgsLineString > closedLine( line->clone() );
        closedLine->close();

        closed->addGeometry( closedLine.release() );
      }
    }
    result = QVariant::fromValue( QgsGeometry( std::move( closed ) ) );
  }

  return result;
}

static QVariant fcnIsEmpty( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( fGeom.isNull() )
    return QVariant();

  return QVariant::fromValue( fGeom.isEmpty() );
}

static QVariant fcnIsEmptyOrNull( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( QgsVariantUtils::isNull( values.at( 0 ) ) )
    return QVariant::fromValue( true );

  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  return QVariant::fromValue( fGeom.isNull() || fGeom.isEmpty() );
}

static QVariant fcnRelate( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( values.length() < 2 || values.length() > 3 )
    return QVariant();

  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );

  if ( fGeom.isNull() || sGeom.isNull() )
    return QVariant();

  std::unique_ptr<QgsGeometryEngine> engine( QgsGeometry::createGeometryEngine( fGeom.constGet() ) );

  if ( values.length() == 2 )
  {
    //two geometry arguments, return relation
    QString result = engine->relate( sGeom.constGet() );
    return QVariant::fromValue( result );
  }
  else
  {
    //three arguments, test pattern
    QString pattern = QgsExpressionUtils::getStringValue( values.at( 2 ), parent );
    bool result = engine->relatePattern( sGeom.constGet(), pattern );
    return QVariant::fromValue( result );
  }
}

static QVariant fcnBbox( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  return fGeom.intersects( sGeom.boundingBox() ) ? TVL_True : TVL_False;
}
static QVariant fcnDisjoint( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  return fGeom.disjoint( sGeom ) ? TVL_True : TVL_False;
}
static QVariant fcnIntersects( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  return fGeom.intersects( sGeom ) ? TVL_True : TVL_False;
}
static QVariant fcnTouches( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  return fGeom.touches( sGeom ) ? TVL_True : TVL_False;
}
static QVariant fcnCrosses( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  return fGeom.crosses( sGeom ) ? TVL_True : TVL_False;
}
static QVariant fcnContains( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  return fGeom.contains( sGeom ) ? TVL_True : TVL_False;
}
static QVariant fcnOverlaps( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  return fGeom.overlaps( sGeom ) ? TVL_True : TVL_False;
}
static QVariant fcnWithin( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  return fGeom.within( sGeom ) ? TVL_True : TVL_False;
}

static QVariant fcnBuffer( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  const double dist = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  const int seg = QgsExpressionUtils::getNativeIntValue( values.at( 2 ), parent );
  const QString endCapString = QgsExpressionUtils::getStringValue( values.at( 3 ), parent ).trimmed();
  const QString joinString = QgsExpressionUtils::getStringValue( values.at( 4 ), parent ).trimmed();
  const double miterLimit = QgsExpressionUtils::getDoubleValue( values.at( 5 ), parent );

  Qgis::EndCapStyle capStyle = Qgis::EndCapStyle::Round;
  if ( endCapString.compare( "flat"_L1, Qt::CaseInsensitive ) == 0 )
    capStyle = Qgis::EndCapStyle::Flat;
  else if ( endCapString.compare( "square"_L1, Qt::CaseInsensitive ) == 0 )
    capStyle = Qgis::EndCapStyle::Square;

  Qgis::JoinStyle joinStyle = Qgis::JoinStyle::Round;
  if ( joinString.compare( "miter"_L1, Qt::CaseInsensitive ) == 0 )
    joinStyle = Qgis::JoinStyle::Miter;
  else if ( joinString.compare( "bevel"_L1, Qt::CaseInsensitive ) == 0 )
    joinStyle = Qgis::JoinStyle::Bevel;

  QgsGeometry geom = fGeom.buffer( dist, seg, capStyle, joinStyle, miterLimit );
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnForceRHR( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  const QgsGeometry reoriented = fGeom.forceRHR();
  return !reoriented.isNull() ? QVariant::fromValue( reoriented ) : QVariant();
}

static QVariant fcnForcePolygonCW( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  const QgsGeometry reoriented = fGeom.forcePolygonClockwise();
  return !reoriented.isNull() ? QVariant::fromValue( reoriented ) : QVariant();
}

static QVariant fcnForcePolygonCCW( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  const QgsGeometry reoriented = fGeom.forcePolygonCounterClockwise();
  return !reoriented.isNull() ? QVariant::fromValue( reoriented ) : QVariant();
}

static QVariant fcnWedgeBuffer( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  const QgsPoint *pt = qgsgeometry_cast<const QgsPoint *>( fGeom.constGet() );
  if ( !pt && fGeom.isMultipart() )
  {
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( fGeom.constGet() ) )
    {
      if ( collection->numGeometries() == 1 )
      {
        pt = qgsgeometry_cast< const QgsPoint * >( collection->geometryN( 0 ) );
      }
    }
  }

  if ( !pt )
  {
    parent->setEvalErrorString( QObject::tr( "Function `wedge_buffer` requires a point value for the center." ) );
    return QVariant();
  }

  double azimuth = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  double width = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
  double outerRadius = QgsExpressionUtils::getDoubleValue( values.at( 3 ), parent );
  double innerRadius = QgsExpressionUtils::getDoubleValue( values.at( 4 ), parent );

  QgsGeometry geom = QgsGeometry::createWedgeBuffer( *pt, azimuth, width, outerRadius, innerRadius );
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnTaperedBuffer( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( fGeom.type() != Qgis::GeometryType::Line )
  {
    parent->setEvalErrorString( QObject::tr( "Function `tapered_buffer` requires a line geometry." ) );
    return QVariant();
  }

  double startWidth = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  double endWidth = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
  int segments = static_cast< int >( QgsExpressionUtils::getIntValue( values.at( 3 ), parent ) );

  QgsGeometry geom = fGeom.taperedBuffer( startWidth, endWidth, segments );
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnBufferByM( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( fGeom.type() != Qgis::GeometryType::Line )
  {
    parent->setEvalErrorString( QObject::tr( "Function `buffer_by_m` requires a line geometry." ) );
    return QVariant();
  }

  int segments = static_cast< int >( QgsExpressionUtils::getIntValue( values.at( 1 ), parent ) );

  QgsGeometry geom = fGeom.variableWidthBufferByM( segments );
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnOffsetCurve( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  double dist = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  int segments = QgsExpressionUtils::getNativeIntValue( values.at( 2 ), parent );
  const int joinInt = QgsExpressionUtils::getIntValue( values.at( 3 ), parent );
  if ( joinInt < 1 || joinInt > 3 )
    return QVariant();
  const Qgis::JoinStyle join = static_cast< Qgis::JoinStyle >( joinInt );

  double miterLimit = QgsExpressionUtils::getDoubleValue( values.at( 3 ), parent );

  QgsGeometry geom = fGeom.offsetCurve( dist, segments, join, miterLimit );
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnSingleSidedBuffer( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  double dist = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  int segments = QgsExpressionUtils::getNativeIntValue( values.at( 2 ), parent );

  const int joinInt = QgsExpressionUtils::getIntValue( values.at( 3 ), parent );
  if ( joinInt < 1 || joinInt > 3 )
    return QVariant();
  const Qgis::JoinStyle join = static_cast< Qgis::JoinStyle >( joinInt );

  double miterLimit = QgsExpressionUtils::getDoubleValue( values.at( 3 ), parent );

  QgsGeometry geom = fGeom.singleSidedBuffer( dist, segments, Qgis::BufferSide::Left, join, miterLimit );
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnExtend( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  double distStart = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  double distEnd = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );

  QgsGeometry geom = fGeom.extendLine( distStart, distEnd );
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnTranslate( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  double dx = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  double dy = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
  fGeom.translate( dx, dy );
  return QVariant::fromValue( fGeom );
}

static QVariant fcnRotate( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  const double rotation = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  const QgsGeometry center = values.at( 2 ).isValid() ? QgsExpressionUtils::getGeometry( values.at( 2 ), parent )
                             : QgsGeometry();
  const bool perPart = values.value( 3 ).toBool();

  if ( center.isNull() && perPart && fGeom.isMultipart() )
  {
    // no explicit center, rotating per part
    // (note that we only do this branch for multipart geometries -- for singlepart geometries
    // the result is equivalent to setting perPart as false anyway)
    std::unique_ptr< QgsGeometryCollection > collection( qgsgeometry_cast< QgsGeometryCollection * >( fGeom.constGet()->clone() ) );
    for ( auto it = collection->parts_begin(); it != collection->parts_end(); ++it )
    {
      const QgsPointXY partCenter = ( *it )->boundingBox().center();
      QTransform t = QTransform::fromTranslate( partCenter.x(), partCenter.y() );
      t.rotate( -rotation );
      t.translate( -partCenter.x(), -partCenter.y() );
      ( *it )->transform( t );
    }
    return QVariant::fromValue( QgsGeometry( std::move( collection ) ) );
  }
  else
  {
    QgsPointXY pt;
    if ( center.isEmpty() )
    {
      // if center wasn't specified, use bounding box centroid
      pt = fGeom.boundingBox().center();
    }
    else if ( QgsWkbTypes::flatType( center.constGet()->simplifiedTypeRef()->wkbType() ) != Qgis::WkbType::Point )
    {
      parent->setEvalErrorString( QObject::tr( "Function 'rotate' requires a point value for the center" ) );
      return QVariant();
    }
    else
    {
      pt = QgsPointXY( *qgsgeometry_cast< const QgsPoint * >( center.constGet()->simplifiedTypeRef() ) );
    }

    fGeom.rotate( rotation, pt );
    return QVariant::fromValue( fGeom );
  }
}

static QVariant fcnScale( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  const double xScale = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  const double yScale = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
  const QgsGeometry center = values.at( 3 ).isValid() ? QgsExpressionUtils::getGeometry( values.at( 3 ), parent )
                             : QgsGeometry();

  QgsPointXY pt;
  if ( center.isNull() )
  {
    // if center wasn't specified, use bounding box centroid
    pt = fGeom.boundingBox().center();
  }
  else if ( QgsWkbTypes::flatType( center.constGet()->simplifiedTypeRef()->wkbType() ) != Qgis::WkbType::Point )
  {
    parent->setEvalErrorString( QObject::tr( "Function 'scale' requires a point value for the center" ) );
    return QVariant();
  }
  else
  {
    pt = center.asPoint();
  }

  QTransform t = QTransform::fromTranslate( pt.x(), pt.y() );
  t.scale( xScale, yScale );
  t.translate( -pt.x(), -pt.y() );
  fGeom.transform( t );
  return QVariant::fromValue( fGeom );
}

static QVariant fcnAffineTransform( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( fGeom.isNull() )
  {
    return QVariant();
  }

  const double deltaX = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  const double deltaY = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );

  const double rotationZ = QgsExpressionUtils::getDoubleValue( values.at( 3 ), parent );

  const double scaleX = QgsExpressionUtils::getDoubleValue( values.at( 4 ), parent );
  const double scaleY = QgsExpressionUtils::getDoubleValue( values.at( 5 ), parent );

  const double deltaZ = QgsExpressionUtils::getDoubleValue( values.at( 6 ), parent );
  const double deltaM = QgsExpressionUtils::getDoubleValue( values.at( 7 ), parent );
  const double scaleZ = QgsExpressionUtils::getDoubleValue( values.at( 8 ), parent );
  const double scaleM = QgsExpressionUtils::getDoubleValue( values.at( 9 ), parent );

  if ( deltaZ != 0.0 && !fGeom.constGet()->is3D() )
  {
    fGeom.get()->addZValue( 0 );
  }
  if ( deltaM != 0.0 && !fGeom.constGet()->isMeasure() )
  {
    fGeom.get()->addMValue( 0 );
  }

  QTransform transform;
  transform.translate( deltaX, deltaY );
  transform.rotate( rotationZ );
  transform.scale( scaleX, scaleY );
  fGeom.transform( transform, deltaZ, scaleZ, deltaM, scaleM );

  return QVariant::fromValue( fGeom );
}


static QVariant fcnCentroid( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry geom = fGeom.centroid();
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}
static QVariant fcnPointOnSurface( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry geom = fGeom.pointOnSurface();
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnPoleOfInaccessibility( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  double tolerance = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  QgsGeometry geom = fGeom.poleOfInaccessibility( tolerance );
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnConvexHull( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry geom = fGeom.convexHull();
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=11 )
static QVariant fcnConcaveHull( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  try
  {
    QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
    const double targetPercent = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
    const bool allowHoles = values.value( 2 ).toBool();
    QgsGeometry geom = fGeom.concaveHull( targetPercent, allowHoles );
    QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
    return result;
  }
  catch ( QgsCsException &cse )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error caught in concave_hull() function: %1" ).arg( cse.what() ) );
    return QVariant();
  }
}
#endif

static QVariant fcnMinimalCircle( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  int segments = 36;
  if ( values.length() == 2 )
    segments = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  if ( segments < 0 )
  {
    parent->setEvalErrorString( QObject::tr( "Parameter can not be negative." ) );
    return QVariant();
  }

  QgsGeometry geom = fGeom.minimalEnclosingCircle( static_cast<unsigned int>( segments ) );
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnOrientedBBox( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry geom = fGeom.orientedMinimumBoundingBox( );
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnMainAngle( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  // we use the angle of the oriented minimum bounding box to calculate the polygon main angle.
  // While ArcGIS uses a different approach ("the angle of longest collection of segments that have similar orientation"), this
  // yields similar results to OMBB approach under the same constraints ("this tool is meant for primarily orthogonal polygons rather than organically shaped ones.")

  double area, angle, width, height;
  const QgsGeometry geom = fGeom.orientedMinimumBoundingBox( area, angle, width, height );

  if ( geom.isNull() )
  {
    parent->setEvalErrorString( QObject::tr( "Error calculating polygon main angle: %1" ).arg( geom.lastError() ) );
    return QVariant();
  }
  return angle;
}

static QVariant fcnDifference( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  QgsGeometry geom = fGeom.difference( sGeom );
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnReverse( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( QgsVariantUtils::isNull( values.at( 0 ) ) )
    return QVariant();

  // two variants, one for geometry, one for string

  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent, true );
  if ( !fGeom.isNull() )
  {
    QVariant result;
    if ( !fGeom.isMultipart() )
    {
      const QgsCurve *curve = qgsgeometry_cast<const QgsCurve * >( fGeom.constGet() );
      if ( !curve )
        return QVariant();

      QgsCurve *reversed = curve->reversed();
      result = reversed ? QVariant::fromValue( QgsGeometry( reversed ) ) : QVariant();
    }
    else
    {
      const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection *>( fGeom.constGet() );
      std::unique_ptr< QgsGeometryCollection > reversed( collection->createEmptyWithSameType() );
      for ( int i = 0; i < collection->numGeometries(); ++i )
      {
        if ( const QgsCurve *curve = qgsgeometry_cast<const QgsCurve * >( collection->geometryN( i ) ) )
        {
          reversed->addGeometry( curve->reversed() );
        }
        else
        {
          reversed->addGeometry( collection->geometryN( i )->clone() );
        }
      }
      result = reversed ? QVariant::fromValue( QgsGeometry( std::move( reversed ) ) ) : QVariant();
    }
    return result;
  }

  //fall back to string variant
  QString string = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  std::reverse( string.begin(), string.end() );
  return string;
}

static QVariant fcnExteriorRing( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( fGeom.isNull() )
    return QVariant();

  const QgsCurvePolygon *curvePolygon = qgsgeometry_cast< const QgsCurvePolygon * >( fGeom.constGet() );
  if ( !curvePolygon && fGeom.isMultipart() )
  {
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( fGeom.constGet() ) )
    {
      if ( collection->numGeometries() == 1 )
      {
        curvePolygon = qgsgeometry_cast< const QgsCurvePolygon * >( collection->geometryN( 0 ) );
      }
    }
  }

  if ( !curvePolygon || !curvePolygon->exteriorRing() )
    return QVariant();

  QgsCurve *exterior = static_cast< QgsCurve * >( curvePolygon->exteriorRing()->clone() );
  QVariant result = exterior ? QVariant::fromValue( QgsGeometry( exterior ) ) : QVariant();
  return result;
}

static QVariant fcnDistance( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  return QVariant( fGeom.distance( sGeom ) );
}

static QVariant fcnHausdorffDistance( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry g1 = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry g2 = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );

  double res = -1;
  if ( values.length() == 3 && values.at( 2 ).isValid() )
  {
    double densify = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
    densify = std::clamp( densify, 0.0, 1.0 );
    res = g1.hausdorffDistanceDensify( g2, densify );
  }
  else
  {
    res = g1.hausdorffDistance( g2 );
  }

  return res > -1 ? QVariant( res ) : QVariant();
}

static QVariant fcnIntersection( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  QgsGeometry geom = fGeom.intersection( sGeom );
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}
static QVariant fcnSymDifference( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  QgsGeometry geom = fGeom.symDifference( sGeom );
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}
static QVariant fcnCombine( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry sGeom = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  QgsGeometry geom = fGeom.combine( sGeom );
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnGeomToWKT( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( values.length() < 1 || values.length() > 2 )
    return QVariant();

  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  int prec = 8;
  if ( values.length() == 2 )
    prec = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  QString wkt = fGeom.asWkt( prec );
  return QVariant( wkt );
}

static QVariant fcnGeomToWKB( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  return fGeom.isNull() ? QVariant() : QVariant( fGeom.asWkb() );
}

static QVariant fcnAzimuth( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( values.length() != 2 )
  {
    parent->setEvalErrorString( QObject::tr( "Function `azimuth` requires exactly two parameters. %n given.", nullptr, values.length() ) );
    return QVariant();
  }

  QgsGeometry fGeom1 = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry fGeom2 = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );

  const QgsPoint *pt1 = qgsgeometry_cast<const QgsPoint *>( fGeom1.constGet() );
  if ( !pt1 && fGeom1.isMultipart() )
  {
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( fGeom1.constGet() ) )
    {
      if ( collection->numGeometries() == 1 )
      {
        pt1 = qgsgeometry_cast< const QgsPoint * >( collection->geometryN( 0 ) );
      }
    }
  }

  const QgsPoint *pt2 = qgsgeometry_cast<const QgsPoint *>( fGeom2.constGet() );
  if ( !pt2 && fGeom2.isMultipart() )
  {
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( fGeom2.constGet() ) )
    {
      if ( collection->numGeometries() == 1 )
      {
        pt2 = qgsgeometry_cast< const QgsPoint * >( collection->geometryN( 0 ) );
      }
    }
  }

  if ( !pt1 || !pt2 )
  {
    parent->setEvalErrorString( QObject::tr( "Function `azimuth` requires two points as arguments." ) );
    return QVariant();
  }

  // Code from PostGIS
  if ( qgsDoubleNear( pt1->x(), pt2->x() ) )
  {
    if ( pt1->y() < pt2->y() )
      return 0.0;
    else if ( pt1->y() > pt2->y() )
      return M_PI;
    else
      return 0;
  }

  if ( qgsDoubleNear( pt1->y(), pt2->y() ) )
  {
    if ( pt1->x() < pt2->x() )
      return M_PI_2;
    else if ( pt1->x() > pt2->x() )
      return M_PI + ( M_PI_2 );
    else
      return 0;
  }

  if ( pt1->x() < pt2->x() )
  {
    if ( pt1->y() < pt2->y() )
    {
      return std::atan( std::fabs( pt1->x() - pt2->x() ) / std::fabs( pt1->y() - pt2->y() ) );
    }
    else /* ( pt1->y() > pt2->y() )  - equality case handled above */
    {
      return std::atan( std::fabs( pt1->y() - pt2->y() ) / std::fabs( pt1->x() - pt2->x() ) )
             + ( M_PI_2 );
    }
  }

  else /* ( pt1->x() > pt2->x() ) - equality case handled above */
  {
    if ( pt1->y() > pt2->y() )
    {
      return std::atan( std::fabs( pt1->x() - pt2->x() ) / std::fabs( pt1->y() - pt2->y() ) )
             + M_PI;
    }
    else /* ( pt1->y() < pt2->y() )  - equality case handled above */
    {
      return std::atan( std::fabs( pt1->y() - pt2->y() ) / std::fabs( pt1->x() - pt2->x() ) )
             + ( M_PI + ( M_PI_2 ) );
    }
  }
}

static QVariant fcnBearing( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry geom1 = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  const QgsGeometry geom2 = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  QgsCoordinateReferenceSystem sourceCrs = QgsExpressionUtils::getCrsValue( values.at( 2 ), parent );
  QString ellipsoid = QgsExpressionUtils::getStringValue( values.at( 3 ), parent );

  if ( geom1.isNull() || geom2.isNull() || geom1.type() != Qgis::GeometryType::Point || geom2.type() != Qgis::GeometryType::Point )
  {
    parent->setEvalErrorString( QObject::tr( "Function `bearing` requires two valid point geometries." ) );
    return QVariant();
  }

  const QgsPointXY point1 = geom1.asPoint();
  const QgsPointXY point2 = geom2.asPoint();
  if ( point1.isEmpty() || point2.isEmpty() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `bearing` requires point geometries or multi point geometries with a single part." ) );
    return QVariant();
  }

  QgsCoordinateTransformContext tContext;
  if ( context )
  {
    tContext = context->variable( u"_project_transform_context"_s ).value<QgsCoordinateTransformContext>();

    if ( !sourceCrs.isValid() )
    {
      sourceCrs = context->variable( u"_layer_crs"_s ).value<QgsCoordinateReferenceSystem>();
    }

    if ( ellipsoid.isEmpty() )
    {
      ellipsoid = context->variable( u"project_ellipsoid"_s ).toString();
    }
  }

  if ( !sourceCrs.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `bearing` requires a valid source CRS." ) );
    return QVariant();
  }

  QgsDistanceArea da;
  da.setSourceCrs( sourceCrs, tContext );
  if ( !da.setEllipsoid( ellipsoid ) )
  {
    parent->setEvalErrorString( QObject::tr( "Function `bearing` requires a valid ellipsoid acronym or ellipsoid authority ID." ) );
    return QVariant();
  }

  try
  {
    const double bearing = da.bearing( point1, point2 );
    if ( std::isfinite( bearing ) )
    {
      return std::fmod( bearing + 2 * M_PI, 2 * M_PI );
    }
  }
  catch ( QgsCsException &cse )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error caught in bearing() function: %1" ).arg( cse.what() ) );
    return QVariant();
  }
  return QVariant();
}

static QVariant fcnProject( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( ! geom.constGet() || QgsWkbTypes::flatType( geom.constGet()->simplifiedTypeRef( )->wkbType() ) != Qgis::WkbType::Point )
  {
    parent->setEvalErrorString( u"'project' requires a point geometry"_s );
    return QVariant();
  }

  double distance = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  double azimuth = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
  double inclination = QgsExpressionUtils::getDoubleValue( values.at( 3 ), parent );

  const QgsPoint *p = static_cast<const QgsPoint *>( geom.constGet()->simplifiedTypeRef( ) );
  QgsPoint newPoint = p->project( distance,  180.0 * azimuth / M_PI, 180.0 * inclination / M_PI );

  return QVariant::fromValue( QgsGeometry( new QgsPoint( newPoint ) ) );
}

static QVariant fcnInclination( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom1 = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry fGeom2 = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );

  const QgsPoint *pt1 = qgsgeometry_cast<const QgsPoint *>( fGeom1.constGet() );
  if ( !pt1 && fGeom1.isMultipart() )
  {
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( fGeom1.constGet() ) )
    {
      if ( collection->numGeometries() == 1 )
      {
        pt1 = qgsgeometry_cast< const QgsPoint * >( collection->geometryN( 0 ) );
      }
    }
  }
  const QgsPoint *pt2 = qgsgeometry_cast<const QgsPoint *>( fGeom2.constGet() );
  if ( !pt2 && fGeom2.isMultipart() )
  {
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( fGeom2.constGet() ) )
    {
      if ( collection->numGeometries() == 1 )
      {
        pt2 = qgsgeometry_cast< const QgsPoint * >( collection->geometryN( 0 ) );
      }
    }
  }

  if ( ( fGeom1.type() != Qgis::GeometryType::Point ) || ( fGeom2.type() != Qgis::GeometryType::Point ) ||
       !pt1 || !pt2 )
  {
    parent->setEvalErrorString( u"Function 'inclination' requires two points as arguments."_s );
    return QVariant();
  }

  return pt1->inclination( *pt2 );

}

static QVariant fcnExtrude( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( values.length() != 3 )
    return QVariant();

  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  double x = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  double y = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );

  QgsGeometry geom = fGeom.extrude( x, y );

  QVariant result = geom.constGet() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnOrderParts( const QVariantList &values, const QgsExpressionContext *ctx, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( values.length() < 2 )
    return QVariant();

  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( !fGeom.isMultipart() )
    return values.at( 0 );

  QString expString = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  QVariant cachedExpression;
  if ( ctx )
    cachedExpression = ctx->cachedValue( expString );
  QgsExpression expression;

  if ( cachedExpression.isValid() )
  {
    expression = cachedExpression.value<QgsExpression>();
  }
  else
    expression = QgsExpression( expString );

  bool asc = values.value( 2 ).toBool();

  QgsExpressionContext *unconstedContext = nullptr;
  QgsFeature f;
  if ( ctx )
  {
    // ExpressionSorter wants a modifiable expression context, but it will return it in the same shape after
    // so no reason to worry
    unconstedContext = const_cast<QgsExpressionContext *>( ctx );
    f = ctx->feature();
  }
  else
  {
    // If there's no context provided, create a fake one
    unconstedContext = new QgsExpressionContext();
  }

  const QgsGeometryCollection *collection = qgsgeometry_cast<const QgsGeometryCollection *>( fGeom.constGet() );
  Q_ASSERT( collection ); // Should have failed the multipart check above

  QgsFeatureRequest::OrderBy orderBy;
  orderBy.append( QgsFeatureRequest::OrderByClause( expression, asc ) );
  QgsExpressionSorter sorter( orderBy );

  QList<QgsFeature> partFeatures;
  partFeatures.reserve( collection->partCount() );
  for ( int i = 0; i < collection->partCount(); ++i )
  {
    f.setGeometry( QgsGeometry( collection->geometryN( i )->clone() ) );
    partFeatures << f;
  }

  sorter.sortFeatures( partFeatures, unconstedContext );

  QgsGeometryCollection *orderedGeom = qgsgeometry_cast<QgsGeometryCollection *>( fGeom.constGet()->clone() );

  Q_ASSERT( orderedGeom );

  while ( orderedGeom->partCount() )
    orderedGeom->removeGeometry( 0 );

  for ( const QgsFeature &feature : std::as_const( partFeatures ) )
  {
    orderedGeom->addGeometry( feature.geometry().constGet()->clone() );
  }

  QVariant result = QVariant::fromValue( QgsGeometry( orderedGeom ) );

  if ( !ctx )
    delete unconstedContext;

  return result;
}

static QVariant fcnClosestPoint( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fromGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry toGeom = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );

  QgsGeometry geom = fromGeom.nearestPoint( toGeom );

  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnShortestLine( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fromGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry toGeom = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );

  QgsGeometry geom = fromGeom.shortestLine( toGeom );

  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnLineInterpolatePoint( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry lineGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  double distance = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );

  QgsGeometry geom = lineGeom.interpolate( distance );

  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnLineInterpolatePointByM( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry lineGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  const double m = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  const bool use3DDistance = values.at( 2 ).toBool();

  double x, y, z, distance;

  const QgsLineString *line = qgsgeometry_cast<const QgsLineString *>( lineGeom.constGet() );
  if ( !line )
  {
    return QVariant();
  }

  if ( line->lineLocatePointByM( m, x, y, z, distance, use3DDistance ) )
  {
    QgsPoint point( x, y );
    if ( use3DDistance && QgsWkbTypes::hasZ( lineGeom.wkbType() ) )
    {
      point.addZValue( z );
    }
    return QVariant::fromValue( QgsGeometry( point.clone() ) );
  }

  return QVariant();
}

static QVariant fcnLineSubset( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry lineGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( lineGeom.type() != Qgis::GeometryType::Line )
  {
    parent->setEvalErrorString( QObject::tr( "line_substring requires a curve geometry input" ) );
    return QVariant();
  }

  const QgsCurve *curve = nullptr;
  if ( !lineGeom.isMultipart() )
    curve = qgsgeometry_cast< const QgsCurve * >( lineGeom.constGet() );
  else
  {
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( lineGeom.constGet() ) )
    {
      if ( collection->numGeometries() > 0 )
      {
        curve = qgsgeometry_cast< const QgsCurve * >( collection->geometryN( 0 ) );
      }
    }
  }
  if ( !curve )
    return QVariant();

  double startDistance = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  double endDistance = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );

  std::unique_ptr< QgsCurve > substring( curve->curveSubstring( startDistance, endDistance ) );
  QgsGeometry result( std::move( substring ) );
  return !result.isNull() ? QVariant::fromValue( result ) : QVariant();
}

static QVariant fcnLineInterpolateAngle( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry lineGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  double distance = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );

  return lineGeom.interpolateAngle( distance ) * 180.0 / M_PI;
}

static QVariant fcnAngleAtVertex( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  int vertex = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  if ( vertex < 0 )
  {
    //negative idx
    int count = geom.constGet()->nCoordinates();
    vertex = count + vertex;
  }

  return geom.angleAtVertex( vertex ) * 180.0 / M_PI;
}

static QVariant fcnDistanceToVertex( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  int vertex = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  if ( vertex < 0 )
  {
    //negative idx
    int count = geom.constGet()->nCoordinates();
    vertex = count + vertex;
  }

  return geom.distanceToVertex( vertex );
}

static QVariant fcnLineLocatePoint( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry lineGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsGeometry pointGeom = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );

  double distance = lineGeom.lineLocatePoint( pointGeom );

  return distance >= 0 ? distance : QVariant();
}

static QVariant fcnLineLocateM( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry lineGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  const double m = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  const bool use3DDistance = values.at( 2 ).toBool();

  double x, y, z, distance;

  const QgsLineString *line = qgsgeometry_cast<const QgsLineString *>( lineGeom.constGet() );
  if ( !line )
  {
    return QVariant();
  }

  const bool found = line->lineLocatePointByM( m, x, y, z, distance, use3DDistance );
  return found ? distance : QVariant();
}

static QVariant fcnRound( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( values.length() == 2 && values.at( 1 ).toInt() != 0 )
  {
    double number = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
    return qgsRound( number, QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent ) );
  }

  if ( values.length() >= 1 )
  {
    double number = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
    return QVariant( qlonglong( std::round( number ) ) );
  }

  return QVariant();
}

static QVariant fcnPi( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  Q_UNUSED( values )
  Q_UNUSED( parent )
  return M_PI;
}

static QVariant fcnFormatNumber( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const double value = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  const int places = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  const QString language = QgsExpressionUtils::getStringValue( values.at( 2 ), parent );
  if ( places < 0 )
  {
    parent->setEvalErrorString( QObject::tr( "Number of places must be positive" ) );
    return QVariant();
  }

  const bool omitGroupSeparator = values.value( 3 ).toBool();
  const bool trimTrailingZeros = values.value( 4 ).toBool();

  QLocale locale = !language.isEmpty() ? QLocale( language ) : QLocale();
  if ( !omitGroupSeparator )
    locale.setNumberOptions( locale.numberOptions() & ~QLocale::NumberOption::OmitGroupSeparator );
  else
    locale.setNumberOptions( locale.numberOptions() | QLocale::NumberOption::OmitGroupSeparator );

  QString res = locale.toString( value, 'f', places );

  if ( trimTrailingZeros )
  {
    const QChar decimal = locale.decimalPoint().at( 0 );
    const QChar zeroDigit = locale.zeroDigit().at( 0 );

    if ( res.contains( decimal ) )
    {
      int trimPoint = res.length() - 1;

      while ( res.at( trimPoint ) == zeroDigit )
        trimPoint--;

      if ( res.at( trimPoint ) == decimal )
        trimPoint--;

      res.truncate( trimPoint + 1 );
    }
  }

  return res;
}

static QVariant fcnFormatDate( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QDateTime datetime = QgsExpressionUtils::getDateTimeValue( values.at( 0 ), parent );
  const QString format = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  const QString language = QgsExpressionUtils::getStringValue( values.at( 2 ), parent );

  // Convert to UTC if the format string includes a Z, as QLocale::toString() doesn't do it
  if ( format.indexOf( "Z" ) > 0 )
    datetime = datetime.toUTC();

  QLocale locale = !language.isEmpty() ? QLocale( language ) : QLocale();
  return locale.toString( datetime, format );
}

static QVariant fcnColorGrayscaleAverage( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QVariant variant = values.at( 0 );
  bool isQColor;
  QColor color = QgsExpressionUtils::getColorValue( variant, parent, isQColor );
  if ( !color.isValid() )
    return QVariant();

  const float alpha = color.alphaF(); // NOLINT(bugprone-narrowing-conversions): TODO QGIS 5 remove the nolint instructions, QColor was qreal (double) and is now float
  if ( color.spec() == QColor::Spec::Cmyk )
  {
    const float avg = ( color.cyanF() + color.magentaF() + color.yellowF() ) / 3; // NOLINT(bugprone-narrowing-conversions): TODO QGIS 5 remove the nolint instructions, QColor was qreal (double) and is now float
    color = QColor::fromCmykF( avg, avg, avg, color.blackF(), alpha );
  }
  else
  {
    const float avg = ( color.redF() + color.greenF() + color.blueF() ) / 3; // NOLINT(bugprone-narrowing-conversions): TODO QGIS 5 remove the nolint instructions, QColor was qreal (double) and is now float
    color.setRgbF( avg, avg, avg, alpha );
  }

  return isQColor ? QVariant( color ) : QVariant( QgsSymbolLayerUtils::encodeColor( color ) );
}

static QVariant fcnColorMixRgb( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QColor color1 = QgsSymbolLayerUtils::decodeColor( values.at( 0 ).toString() );
  QColor color2 = QgsSymbolLayerUtils::decodeColor( values.at( 1 ).toString() );
  double ratio = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
  if ( ratio > 1 )
  {
    ratio = 1;
  }
  else if ( ratio < 0 )
  {
    ratio = 0;
  }

  int red = static_cast<int>( color1.red() * ( 1 - ratio ) + color2.red() * ratio );
  int green = static_cast<int>( color1.green() * ( 1 - ratio ) + color2.green() * ratio );
  int blue = static_cast<int>( color1.blue() * ( 1 - ratio ) + color2.blue() * ratio );
  int alpha = static_cast<int>( color1.alpha() * ( 1 - ratio ) + color2.alpha() * ratio );

  QColor newColor( red, green, blue, alpha );

  return QgsSymbolLayerUtils::encodeColor( newColor );
}

static QVariant fcnColorMix( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QVariant variant1 = values.at( 0 );
  const QVariant variant2 = values.at( 1 );

  if ( variant1.userType() != variant2.userType() )
  {
    parent->setEvalErrorString( QObject::tr( "Both color arguments must have the same type (string or color object)" ) );
    return QVariant();
  }

  bool isQColor;
  const QColor color1 = QgsExpressionUtils::getColorValue( variant1, parent, isQColor );
  if ( !color1.isValid() )
    return QVariant();

  const QColor color2 = QgsExpressionUtils::getColorValue( variant2, parent, isQColor );
  if ( !color2.isValid() )
    return QVariant();

  if ( ( color1.spec() == QColor::Cmyk ) != ( color2.spec() == QColor::Cmyk ) )
  {
    parent->setEvalErrorString( QObject::tr( "Both color arguments must have compatible color type (CMYK or RGB/HSV/HSL)" ) );
    return QVariant();
  }

  const float ratio = static_cast<float>( std::clamp( QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent ), 0., 1. ) );

  // TODO QGIS 5 remove the nolint instructions, QColor was qreal (double) and is now float
  // NOLINTBEGIN(bugprone-narrowing-conversions)

  QColor newColor;
  const float alpha = color1.alphaF() * ( 1 - ratio ) + color2.alphaF() * ratio;
  if ( color1.spec() == QColor::Spec::Cmyk )
  {
    float cyan = color1.cyanF() * ( 1 - ratio ) + color2.cyanF() * ratio;
    float magenta = color1.magentaF() * ( 1 - ratio ) + color2.magentaF() * ratio;
    float yellow = color1.yellowF() * ( 1 - ratio ) + color2.yellowF() * ratio;
    float black = color1.blackF() * ( 1 - ratio ) + color2.blackF() * ratio;
    newColor = QColor::fromCmykF( cyan, magenta, yellow, black, alpha );
  }
  else
  {
    float red = color1.redF() * ( 1 - ratio ) + color2.redF() * ratio;
    float green = color1.greenF() * ( 1 - ratio ) + color2.greenF() * ratio;
    float blue = color1.blueF() * ( 1 - ratio ) + color2.blueF() * ratio;
    newColor = QColor::fromRgbF( red, green, blue, alpha );
  }

  // NOLINTEND(bugprone-narrowing-conversions)

  return isQColor ? QVariant( newColor ) : QVariant( QgsSymbolLayerUtils::encodeColor( newColor ) );
}

static QVariant fcnColorRgb( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  int red = QgsExpressionUtils::getNativeIntValue( values.at( 0 ), parent );
  int green = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  int blue = QgsExpressionUtils::getNativeIntValue( values.at( 2 ), parent );
  QColor color = QColor( red, green, blue );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3' to color" ).arg( red ).arg( green ).arg( blue ) );
    color = QColor( 0, 0, 0 );
  }

  return u"%1,%2,%3"_s.arg( color.red() ).arg( color.green() ).arg( color.blue() );
}

static QVariant fcnColorRgbF( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const float red = std::clamp( static_cast<float>( QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent ) ), 0.f, 1.f );
  const float green = std::clamp( static_cast<float>( QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent ) ), 0.f, 1.f );
  const float blue = std::clamp( static_cast<float>( QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent ) ), 0.f, 1.f );
  const float alpha = std::clamp( static_cast<float>( QgsExpressionUtils::getDoubleValue( values.at( 3 ), parent ) ), 0.f, 1.f );
  QColor color = QColor::fromRgbF( red, green, blue, alpha );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3:%4' to color" ).arg( red ).arg( green ).arg( blue ).arg( alpha ) );
    return QVariant();
  }

  return color;
}

static QVariant fcnTry( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsExpressionNode *node = QgsExpressionUtils::getNode( values.at( 0 ), parent );
  QVariant value = node->eval( parent, context );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QString() );
    node = QgsExpressionUtils::getNode( values.at( 1 ), parent );
    ENSURE_NO_EVAL_ERROR
    value = node->eval( parent, context );
    ENSURE_NO_EVAL_ERROR
  }
  return value;
}

static QVariant fcnIf( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsExpressionNode *node = QgsExpressionUtils::getNode( values.at( 0 ), parent );
  ENSURE_NO_EVAL_ERROR
  QVariant value = node->eval( parent, context );
  ENSURE_NO_EVAL_ERROR
  if ( value.toBool() )
  {
    node = QgsExpressionUtils::getNode( values.at( 1 ), parent );
    ENSURE_NO_EVAL_ERROR
    value = node->eval( parent, context );
    ENSURE_NO_EVAL_ERROR
  }
  else
  {
    node = QgsExpressionUtils::getNode( values.at( 2 ), parent );
    ENSURE_NO_EVAL_ERROR
    value = node->eval( parent, context );
    ENSURE_NO_EVAL_ERROR
  }
  return value;
}

static QVariant fncColorRgba( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  int red = QgsExpressionUtils::getNativeIntValue( values.at( 0 ), parent );
  int green = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  int blue = QgsExpressionUtils::getNativeIntValue( values.at( 2 ), parent );
  int alpha = QgsExpressionUtils::getNativeIntValue( values.at( 3 ), parent );
  QColor color = QColor( red, green, blue, alpha );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3:%4' to color" ).arg( red ).arg( green ).arg( blue ).arg( alpha ) );
    color = QColor( 0, 0, 0 );
  }
  return QgsSymbolLayerUtils::encodeColor( color );
}

QVariant fcnRampColorObject( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGradientColorRamp expRamp;
  const QgsColorRamp *ramp = nullptr;
  if ( values.at( 0 ).userType() == qMetaTypeId< QgsGradientColorRamp>() )
  {
    expRamp = QgsExpressionUtils::getRamp( values.at( 0 ), parent );
    ramp = &expRamp;
  }
  else
  {
    QString rampName = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
    ramp = QgsStyle::defaultStyle()->colorRampRef( rampName );
    if ( ! ramp )
    {
      parent->setEvalErrorString( QObject::tr( "\"%1\" is not a valid color ramp" ).arg( rampName ) );
      return QVariant();
    }
  }

  double value = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  QColor color = ramp->color( value );
  return color;
}

QVariant fcnRampColor( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node )
{
  QColor color = fcnRampColorObject( values, context, parent, node ).value<QColor>();
  return color.isValid() ? QgsSymbolLayerUtils::encodeColor( color ) : QVariant();
}

static QVariant fcnColorHsl( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  // Hue ranges from 0 - 360
  double hue = QgsExpressionUtils::getIntValue( values.at( 0 ), parent ) / 360.0;
  // Saturation ranges from 0 - 100
  double saturation = QgsExpressionUtils::getIntValue( values.at( 1 ), parent ) / 100.0;
  // Lightness ranges from 0 - 100
  double lightness = QgsExpressionUtils::getIntValue( values.at( 2 ), parent ) / 100.0;

  QColor color = QColor::fromHslF( hue, saturation, lightness );

  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3' to color" ).arg( hue ).arg( saturation ).arg( lightness ) );
    color = QColor( 0, 0, 0 );
  }

  return u"%1,%2,%3"_s.arg( color.red() ).arg( color.green() ).arg( color.blue() );
}

static QVariant fncColorHsla( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  // Hue ranges from 0 - 360
  double hue = QgsExpressionUtils::getIntValue( values.at( 0 ), parent ) / 360.0;
  // Saturation ranges from 0 - 100
  double saturation = QgsExpressionUtils::getIntValue( values.at( 1 ), parent ) / 100.0;
  // Lightness ranges from 0 - 100
  double lightness = QgsExpressionUtils::getIntValue( values.at( 2 ), parent ) / 100.0;
  // Alpha ranges from 0 - 255
  double alpha = QgsExpressionUtils::getIntValue( values.at( 3 ), parent ) / 255.0;

  QColor color = QColor::fromHslF( hue, saturation, lightness, alpha );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3:%4' to color" ).arg( hue ).arg( saturation ).arg( lightness ).arg( alpha ) );
    color = QColor( 0, 0, 0 );
  }
  return QgsSymbolLayerUtils::encodeColor( color );
}

static QVariant fcnColorHslF( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  float hue = std::clamp( static_cast<float>( QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent ) ), 0.f, 1.f );
  float saturation = std::clamp( static_cast<float>( QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent ) ), 0.f, 1.f );
  float lightness = std::clamp( static_cast<float>( QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent ) ), 0.f, 1.f );
  float alpha = std::clamp( static_cast<float>( QgsExpressionUtils::getDoubleValue( values.at( 3 ), parent ) ), 0.f, 1.f );

  QColor color = QColor::fromHslF( hue, saturation, lightness, alpha );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3:%4' to color" ).arg( hue ).arg( saturation ).arg( lightness ).arg( alpha ) );
    return QVariant();
  }

  return color;
}

static QVariant fcnColorHsv( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  // Hue ranges from 0 - 360
  double hue = QgsExpressionUtils::getIntValue( values.at( 0 ), parent ) / 360.0;
  // Saturation ranges from 0 - 100
  double saturation = QgsExpressionUtils::getIntValue( values.at( 1 ), parent ) / 100.0;
  // Value ranges from 0 - 100
  double value = QgsExpressionUtils::getIntValue( values.at( 2 ), parent ) / 100.0;

  QColor color = QColor::fromHsvF( hue, saturation, value );

  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3' to color" ).arg( hue ).arg( saturation ).arg( value ) );
    color = QColor( 0, 0, 0 );
  }

  return u"%1,%2,%3"_s.arg( color.red() ).arg( color.green() ).arg( color.blue() );
}

static QVariant fncColorHsva( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  // Hue ranges from 0 - 360
  double hue = QgsExpressionUtils::getIntValue( values.at( 0 ), parent ) / 360.0;
  // Saturation ranges from 0 - 100
  double saturation = QgsExpressionUtils::getIntValue( values.at( 1 ), parent ) / 100.0;
  // Value ranges from 0 - 100
  double value = QgsExpressionUtils::getIntValue( values.at( 2 ), parent ) / 100.0;
  // Alpha ranges from 0 - 255
  double alpha = QgsExpressionUtils::getIntValue( values.at( 3 ), parent ) / 255.0;

  QColor color = QColor::fromHsvF( hue, saturation, value, alpha );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3:%4' to color" ).arg( hue ).arg( saturation ).arg( value ).arg( alpha ) );
    color = QColor( 0, 0, 0 );
  }
  return QgsSymbolLayerUtils::encodeColor( color );
}

static QVariant fcnColorHsvF( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  float hue = std::clamp( static_cast<float>( QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent ) ), 0.f, 1.f );
  float saturation = std::clamp( static_cast<float>( QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent ) ), 0.f, 1.f );
  float value = std::clamp( static_cast<float>( QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent ) ), 0.f, 1.f );
  float alpha = std::clamp( static_cast<float>( QgsExpressionUtils::getDoubleValue( values.at( 3 ), parent ) ), 0.f, 1.f );
  QColor color = QColor::fromHsvF( hue, saturation, value, alpha );

  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3:%4' to color" ).arg( hue ).arg( saturation ).arg( value ).arg( alpha ) );
    return QVariant();
  }

  return color;
}

static QVariant fcnColorCmykF( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const float cyan = std::clamp( static_cast<float>( QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent ) ), 0.f, 1.f );
  const float magenta = std::clamp( static_cast<float>( QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent ) ), 0.f, 1.f );
  const float yellow = std::clamp( static_cast<float>( QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent ) ), 0.f, 1.f );
  const float black = std::clamp( static_cast<float>( QgsExpressionUtils::getDoubleValue( values.at( 3 ), parent ) ), 0.f, 1.f );
  const float alpha = std::clamp( static_cast<float>( QgsExpressionUtils::getDoubleValue( values.at( 4 ), parent ) ), 0.f, 1.f );

  QColor color = QColor::fromCmykF( cyan, magenta, yellow, black, alpha );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3:%4:%5' to color" ).arg( cyan ).arg( magenta ).arg( yellow ).arg( black ).arg( alpha ) );
    return QVariant();
  }

  return color;
}

static QVariant fcnColorCmyk( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  // Cyan ranges from 0 - 100
  double cyan = QgsExpressionUtils::getIntValue( values.at( 0 ), parent ) / 100.0;
  // Magenta ranges from 0 - 100
  double magenta = QgsExpressionUtils::getIntValue( values.at( 1 ), parent ) / 100.0;
  // Yellow ranges from 0 - 100
  double yellow = QgsExpressionUtils::getIntValue( values.at( 2 ), parent ) / 100.0;
  // Black ranges from 0 - 100
  double black = QgsExpressionUtils::getIntValue( values.at( 3 ), parent ) / 100.0;

  QColor color = QColor::fromCmykF( cyan, magenta, yellow, black );

  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3:%4' to color" ).arg( cyan ).arg( magenta ).arg( yellow ).arg( black ) );
    color = QColor( 0, 0, 0 );
  }

  return u"%1,%2,%3"_s.arg( color.red() ).arg( color.green() ).arg( color.blue() );
}

static QVariant fncColorCmyka( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  // Cyan ranges from 0 - 100
  double cyan = QgsExpressionUtils::getIntValue( values.at( 0 ), parent ) / 100.0;
  // Magenta ranges from 0 - 100
  double magenta = QgsExpressionUtils::getIntValue( values.at( 1 ), parent ) / 100.0;
  // Yellow ranges from 0 - 100
  double yellow = QgsExpressionUtils::getIntValue( values.at( 2 ), parent ) / 100.0;
  // Black ranges from 0 - 100
  double black = QgsExpressionUtils::getIntValue( values.at( 3 ), parent ) / 100.0;
  // Alpha ranges from 0 - 255
  double alpha = QgsExpressionUtils::getIntValue( values.at( 4 ), parent ) / 255.0;

  QColor color = QColor::fromCmykF( cyan, magenta, yellow, black, alpha );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1:%2:%3:%4:%5' to color" ).arg( cyan ).arg( magenta ).arg( yellow ).arg( black ).arg( alpha ) );
    color = QColor( 0, 0, 0 );
  }
  return QgsSymbolLayerUtils::encodeColor( color );
}

static QVariant fncColorPart( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QVariant variant = values.at( 0 );
  bool isQColor;
  const QColor color = QgsExpressionUtils::getColorValue( variant, parent, isQColor );
  if ( !color.isValid() )
    return QVariant();

  QString part = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  if ( part.compare( "red"_L1, Qt::CaseInsensitive ) == 0 )
    return color.red();
  else if ( part.compare( "green"_L1, Qt::CaseInsensitive ) == 0 )
    return color.green();
  else if ( part.compare( "blue"_L1, Qt::CaseInsensitive ) == 0 )
    return color.blue();
  else if ( part.compare( "alpha"_L1, Qt::CaseInsensitive ) == 0 )
    return color.alpha();
  else if ( part.compare( "hue"_L1, Qt::CaseInsensitive ) == 0 )
    return static_cast< double >( color.hsvHueF() * 360 );
  else if ( part.compare( "saturation"_L1, Qt::CaseInsensitive ) == 0 )
    return static_cast< double >( color.hsvSaturationF() * 100 );
  else if ( part.compare( "value"_L1, Qt::CaseInsensitive ) == 0 )
    return static_cast< double >( color.valueF() * 100 );
  else if ( part.compare( "hsl_hue"_L1, Qt::CaseInsensitive ) == 0 )
    return static_cast< double >( color.hslHueF() * 360 );
  else if ( part.compare( "hsl_saturation"_L1, Qt::CaseInsensitive ) == 0 )
    return static_cast< double >( color.hslSaturationF() * 100 );
  else if ( part.compare( "lightness"_L1, Qt::CaseInsensitive ) == 0 )
    return static_cast< double >( color.lightnessF() * 100 );
  else if ( part.compare( "cyan"_L1, Qt::CaseInsensitive ) == 0 )
    return static_cast< double >( color.cyanF() * 100 );
  else if ( part.compare( "magenta"_L1, Qt::CaseInsensitive ) == 0 )
    return static_cast< double >( color.magentaF() * 100 );
  else if ( part.compare( "yellow"_L1, Qt::CaseInsensitive ) == 0 )
    return static_cast< double >( color.yellowF() * 100 );
  else if ( part.compare( "black"_L1, Qt::CaseInsensitive ) == 0 )
    return static_cast< double >( color.blackF() * 100 );

  parent->setEvalErrorString( QObject::tr( "Unknown color component '%1'" ).arg( part ) );
  return QVariant();
}

static QVariant fcnCreateRamp( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QVariantMap map = QgsExpressionUtils::getMapValue( values.at( 0 ), parent );
  if ( map.empty() )
  {
    parent->setEvalErrorString( QObject::tr( "A minimum of two colors is required to create a ramp" ) );
    return QVariant();
  }

  QList< QColor > colors;
  QgsGradientStopsList stops;
  for ( QVariantMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it )
  {
    colors << QgsSymbolLayerUtils::decodeColor( it.value().toString() );
    if ( !colors.last().isValid() )
    {
      parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to color" ).arg( it.value().toString() ) );
      return QVariant();
    }

    double step = it.key().toDouble();
    if ( it == map.constBegin() )
    {
      if ( step != 0.0 )
        stops << QgsGradientStop( step, colors.last() );
    }
    else if ( it == map.constEnd() )
    {
      if ( step != 1.0 )
        stops << QgsGradientStop( step, colors.last() );
    }
    else
    {
      stops << QgsGradientStop( step, colors.last() );
    }
  }
  bool discrete = values.at( 1 ).toBool();

  if ( colors.empty() )
    return QVariant();

  return QVariant::fromValue( QgsGradientColorRamp( colors.first(), colors.last(), discrete, stops ) );
}

static QVariant fncSetColorPart( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QVariant variant = values.at( 0 );
  bool isQColor;
  QColor color = QgsExpressionUtils::getColorValue( variant, parent, isQColor );
  if ( !color.isValid() )
    return QVariant();

  QString part = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  int value = QgsExpressionUtils::getNativeIntValue( values.at( 2 ), parent );
  if ( part.compare( "red"_L1, Qt::CaseInsensitive ) == 0 )
    color.setRed( std::clamp( value, 0, 255 ) );
  else if ( part.compare( "green"_L1, Qt::CaseInsensitive ) == 0 )
    color.setGreen( std::clamp( value, 0, 255 ) );
  else if ( part.compare( "blue"_L1, Qt::CaseInsensitive ) == 0 )
    color.setBlue( std::clamp( value, 0, 255 ) );
  else if ( part.compare( "alpha"_L1, Qt::CaseInsensitive ) == 0 )
    color.setAlpha( std::clamp( value, 0, 255 ) );
  else if ( part.compare( "hue"_L1, Qt::CaseInsensitive ) == 0 )
    color.setHsv( std::clamp( value, 0, 359 ), color.hsvSaturation(), color.value(), color.alpha() );
  else if ( part.compare( "saturation"_L1, Qt::CaseInsensitive ) == 0 )
    color.setHsvF( color.hsvHueF(), std::clamp( value, 0, 100 ) / 100.0, color.valueF(), color.alphaF() );
  else if ( part.compare( "value"_L1, Qt::CaseInsensitive ) == 0 )
    color.setHsvF( color.hsvHueF(), color.hsvSaturationF(), std::clamp( value, 0, 100 ) / 100.0, color.alphaF() );
  else if ( part.compare( "hsl_hue"_L1, Qt::CaseInsensitive ) == 0 )
    color.setHsl( std::clamp( value, 0, 359 ), color.hslSaturation(), color.lightness(), color.alpha() );
  else if ( part.compare( "hsl_saturation"_L1, Qt::CaseInsensitive ) == 0 )
    color.setHslF( color.hslHueF(), std::clamp( value, 0, 100 ) / 100.0, color.lightnessF(), color.alphaF() );
  else if ( part.compare( "lightness"_L1, Qt::CaseInsensitive ) == 0 )
    color.setHslF( color.hslHueF(), color.hslSaturationF(), std::clamp( value, 0, 100 ) / 100.0, color.alphaF() );
  else if ( part.compare( "cyan"_L1, Qt::CaseInsensitive ) == 0 )
    color.setCmykF( std::clamp( value, 0, 100 ) / 100.0, color.magentaF(), color.yellowF(), color.blackF(), color.alphaF() );
  else if ( part.compare( "magenta"_L1, Qt::CaseInsensitive ) == 0 )
    color.setCmykF( color.cyanF(), std::clamp( value, 0, 100 ) / 100.0, color.yellowF(), color.blackF(), color.alphaF() );
  else if ( part.compare( "yellow"_L1, Qt::CaseInsensitive ) == 0 )
    color.setCmykF( color.cyanF(), color.magentaF(), std::clamp( value, 0, 100 ) / 100.0, color.blackF(), color.alphaF() );
  else if ( part.compare( "black"_L1, Qt::CaseInsensitive ) == 0 )
    color.setCmykF( color.cyanF(), color.magentaF(), color.yellowF(), std::clamp( value, 0, 100 ) / 100.0, color.alphaF() );
  else
  {
    parent->setEvalErrorString( QObject::tr( "Unknown color component '%1'" ).arg( part ) );
    return QVariant();
  }
  return isQColor ? QVariant( color ) : QVariant( QgsSymbolLayerUtils::encodeColor( color ) );
}

static QVariant fncDarker( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QVariant variant = values.at( 0 );
  bool isQColor;
  QColor color = QgsExpressionUtils::getColorValue( variant, parent, isQColor );
  if ( !color.isValid() )
    return QVariant();

  color = color.darker( QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent ) );

  return isQColor ? QVariant( color ) : QVariant( QgsSymbolLayerUtils::encodeColor( color ) );
}

static QVariant fncLighter( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QVariant variant = values.at( 0 );
  bool isQColor;
  QColor color = QgsExpressionUtils::getColorValue( variant, parent, isQColor );
  if ( !color.isValid() )
    return QVariant();

  color = color.lighter( QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent ) );

  return isQColor ? QVariant( color ) : QVariant( QgsSymbolLayerUtils::encodeColor( color ) );
}

static QVariant fcnGetGeometry( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsFeature feat = QgsExpressionUtils::getFeature( values.at( 0 ), parent );
  QgsGeometry geom = feat.geometry();
  if ( !geom.isNull() )
    return QVariant::fromValue( geom );
  return QVariant();
}

static QVariant fcnGetFeatureId( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsFeature feat = QgsExpressionUtils::getFeature( values.at( 0 ), parent );
  if ( !feat.isValid() )
    return QVariant();
  return feat.id();
}

static QVariant fcnTransformGeometry( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QgsCoordinateReferenceSystem sCrs = QgsExpressionUtils::getCrsValue( values.at( 1 ), parent );
  QgsCoordinateReferenceSystem dCrs = QgsExpressionUtils::getCrsValue( values.at( 2 ), parent );

  if ( !sCrs.isValid() )
    return QVariant::fromValue( fGeom );

  if ( !dCrs.isValid() )
    return QVariant::fromValue( fGeom );

  QgsCoordinateTransformContext tContext;
  if ( context )
    tContext = context->variable( u"_project_transform_context"_s ).value<QgsCoordinateTransformContext>();
  QgsCoordinateTransform t( sCrs, dCrs, tContext );
  try
  {
    if ( fGeom.transform( t ) == Qgis::GeometryOperationResult::Success )
      return QVariant::fromValue( fGeom );
  }
  catch ( QgsCsException &cse )
  {
    QgsMessageLog::logMessage( QObject::tr( "Transform error caught in transform() function: %1" ).arg( cse.what() ) );
    return QVariant();
  }
  return QVariant();
}


static QVariant fcnGetFeatureById( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  bool foundLayer = false;
  std::unique_ptr<QgsVectorLayerFeatureSource> featureSource = QgsExpressionUtils::getFeatureSource( values.at( 0 ), context, parent, foundLayer );

  //no layer found
  if ( !featureSource || !foundLayer )
  {
    return QVariant();
  }

  const QgsFeatureId fid = QgsExpressionUtils::getIntValue( values.at( 1 ), parent );

  QgsFeatureRequest req;
  req.setFilterFid( fid );
  req.setTimeout( 10000 );
  req.setRequestMayBeNested( true );
  if ( context )
    req.setFeedback( context->feedback() );
  QgsFeatureIterator fIt = featureSource->getFeatures( req );

  QgsFeature fet;
  QVariant result;
  if ( fIt.nextFeature( fet ) )
    result = QVariant::fromValue( fet );

  return result;
}

static QVariant fcnGetFeature( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  //arguments: 1. layer id / name, 2. key attribute, 3. eq value
  bool foundLayer = false;
  std::unique_ptr<QgsVectorLayerFeatureSource> featureSource = QgsExpressionUtils::getFeatureSource( values.at( 0 ), context, parent, foundLayer );

  //no layer found
  if ( !featureSource || !foundLayer )
  {
    return QVariant();
  }
  QgsFeatureRequest req;
  QString cacheValueKey;
  if ( values.at( 1 ).userType() == QMetaType::Type::QVariantMap )
  {
    QVariantMap attributeMap = QgsExpressionUtils::getMapValue( values.at( 1 ), parent );

    QMap <QString, QVariant>::const_iterator i = attributeMap.constBegin();
    QString filterString;
    for ( ; i != attributeMap.constEnd(); ++i )
    {
      if ( !filterString.isEmpty() )
      {
        filterString.append( " AND " );
      }
      filterString.append( QgsExpression::createFieldEqualityExpression( i.key(), i.value() ) );
    }
    cacheValueKey = u"getfeature:%1:%2"_s.arg( featureSource->id(), filterString );
    if ( context && context->hasCachedValue( cacheValueKey ) )
    {
      return context->cachedValue( cacheValueKey );
    }
    req.setFilterExpression( filterString );
  }
  else
  {
    QString attribute = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
    int attributeId = featureSource->fields().lookupField( attribute );
    if ( attributeId == -1 )
    {
      return QVariant();
    }

    const QVariant &attVal = values.at( 2 );

    cacheValueKey = u"getfeature:%1:%2:%3"_s.arg( featureSource->id(), QString::number( attributeId ), attVal.toString() );
    if ( context && context->hasCachedValue( cacheValueKey ) )
    {
      return context->cachedValue( cacheValueKey );
    }

    req.setFilterExpression( QgsExpression::createFieldEqualityExpression( attribute, attVal ) );
  }
  req.setLimit( 1 );
  req.setTimeout( 10000 );
  req.setRequestMayBeNested( true );
  if ( context )
    req.setFeedback( context->feedback() );
  if ( !parent->needsGeometry() )
  {
    req.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
  }
  QgsFeatureIterator fIt = featureSource->getFeatures( req );

  QgsFeature fet;
  QVariant res;
  if ( fIt.nextFeature( fet ) )
  {
    res = QVariant::fromValue( fet );
  }

  if ( context )
    context->setCachedValue( cacheValueKey, res );
  return res;
}

static QVariant fcnRepresentValue( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node )
{
  QVariant result;
  QString fieldName;

  if ( context )
  {
    if ( !values.isEmpty() )
    {
      QgsExpressionNodeColumnRef *col = dynamic_cast<QgsExpressionNodeColumnRef *>( node->args()->at( 0 ) );
      if ( col && ( values.size() == 1 || !values.at( 1 ).isValid() ) )
        fieldName = col->name();
      else if ( values.size() == 2 )
        fieldName = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
    }

    QVariant value = values.at( 0 );

    const QgsFields fields = context->fields();
    int fieldIndex = fields.lookupField( fieldName );

    if ( fieldIndex == -1 )
    {
      parent->setEvalErrorString( QCoreApplication::translate( "expression", "%1: Field not found %2" ).arg( u"represent_value"_s, fieldName ) );
    }
    else
    {
      // TODO this function is NOT thread safe
      Q_NOWARN_DEPRECATED_PUSH
      QgsVectorLayer *layer = QgsExpressionUtils::getVectorLayer( context->variable( u"layer"_s ), context, parent );
      Q_NOWARN_DEPRECATED_POP

      const QString cacheValueKey = u"repvalfcnval:%1:%2:%3"_s.arg( layer ? layer->id() : u"[None]"_s, fieldName, value.toString() );
      if ( context->hasCachedValue( cacheValueKey ) )
      {
        return context->cachedValue( cacheValueKey );
      }

      const QgsEditorWidgetSetup setup = fields.at( fieldIndex ).editorWidgetSetup();
      const QgsFieldFormatter *formatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );

      const QString cacheKey = u"repvalfcn:%1:%2"_s.arg( layer ? layer->id() : u"[None]"_s, fieldName );

      QVariant cache;
      if ( !context->hasCachedValue( cacheKey ) )
      {
        cache = formatter->createCache( layer, fieldIndex, setup.config() );
        context->setCachedValue( cacheKey, cache );
      }
      else
        cache = context->cachedValue( cacheKey );

      result = formatter->representValue( layer, fieldIndex, setup.config(), cache, value );

      context->setCachedValue( cacheValueKey, result );
    }
  }
  else
  {
    parent->setEvalErrorString( QCoreApplication::translate( "expression", "%1: function cannot be evaluated without a context." ).arg( u"represent_value"_s, fieldName ) );
  }

  return result;
}

static QVariant fcnMimeType( const QVariantList &values, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * )
{
  const QVariant data = values.at( 0 );
  const QMimeDatabase db;
  return db.mimeTypeForData( data.toByteArray() ).name();
}

static QVariant fcnGetLayerProperty( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString layerProperty = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );

  bool foundLayer = false;
  const QVariant res = QgsExpressionUtils::runMapLayerFunctionThreadSafe( values.at( 0 ), context, parent, [layerProperty]( QgsMapLayer * layer )-> QVariant
  {
    if ( !layer )
      return QVariant();

    // here, we always prefer the layer metadata values over the older server-specific published values
    if ( QString::compare( layerProperty, u"name"_s, Qt::CaseInsensitive ) == 0 )
      return layer->name();
    else if ( QString::compare( layerProperty, u"id"_s, Qt::CaseInsensitive ) == 0 )
      return layer->id();
    else if ( QString::compare( layerProperty, u"title"_s, Qt::CaseInsensitive ) == 0 )
      return !layer->metadata().title().isEmpty() ? layer->metadata().title() : layer->serverProperties()->title();
    else if ( QString::compare( layerProperty, u"abstract"_s, Qt::CaseInsensitive ) == 0 )
      return !layer->metadata().abstract().isEmpty() ? layer->metadata().abstract() : layer->serverProperties()->abstract();
    else if ( QString::compare( layerProperty, u"keywords"_s, Qt::CaseInsensitive ) == 0 )
    {
      QStringList keywords;
      const QgsAbstractMetadataBase::KeywordMap keywordMap = layer->metadata().keywords();
      for ( auto it = keywordMap.constBegin(); it != keywordMap.constEnd(); ++it )
      {
        keywords.append( it.value() );
      }
      if ( !keywords.isEmpty() )
        return keywords;
      return layer->serverProperties()->keywordList();
    }
    else if ( QString::compare( layerProperty, u"data_url"_s, Qt::CaseInsensitive ) == 0 )
      return layer->serverProperties()->dataUrl();
    else if ( QString::compare( layerProperty, u"attribution"_s, Qt::CaseInsensitive ) == 0 )
    {
      return !layer->metadata().rights().isEmpty() ? QVariant( layer->metadata().rights() ) : QVariant( layer->serverProperties()->attribution() );
    }
    else if ( QString::compare( layerProperty, u"attribution_url"_s, Qt::CaseInsensitive ) == 0 )
      return layer->serverProperties()->attributionUrl();
    else if ( QString::compare( layerProperty, u"source"_s, Qt::CaseInsensitive ) == 0 )
      return layer->publicSource();
    else if ( QString::compare( layerProperty, u"min_scale"_s, Qt::CaseInsensitive ) == 0 )
      return layer->minimumScale();
    else if ( QString::compare( layerProperty, u"max_scale"_s, Qt::CaseInsensitive ) == 0 )
      return layer->maximumScale();
    else if ( QString::compare( layerProperty, u"is_editable"_s, Qt::CaseInsensitive ) == 0 )
      return layer->isEditable();
    else if ( QString::compare( layerProperty, u"crs"_s, Qt::CaseInsensitive ) == 0 )
      return layer->crs().authid();
    else if ( QString::compare( layerProperty, u"crs_definition"_s, Qt::CaseInsensitive ) == 0 )
      return layer->crs().toProj();
    else if ( QString::compare( layerProperty, u"crs_description"_s, Qt::CaseInsensitive ) == 0 )
      return layer->crs().description();
    else if ( QString::compare( layerProperty, u"crs_ellipsoid"_s, Qt::CaseInsensitive ) == 0 )
      return layer->crs().ellipsoidAcronym();
    else if ( QString::compare( layerProperty, u"extent"_s, Qt::CaseInsensitive ) == 0 )
    {
      QgsGeometry extentGeom = QgsGeometry::fromRect( layer->extent() );
      QVariant result = QVariant::fromValue( extentGeom );
      return result;
    }
    else if ( QString::compare( layerProperty, u"distance_units"_s, Qt::CaseInsensitive ) == 0 )
      return QgsUnitTypes::encodeUnit( layer->crs().mapUnits() );
    else if ( QString::compare( layerProperty, u"path"_s, Qt::CaseInsensitive ) == 0 )
    {
      const QVariantMap decodedUri = QgsProviderRegistry::instance()->decodeUri( layer->providerType(), layer->source() );
      return decodedUri.value( u"path"_s );
    }
    else if ( QString::compare( layerProperty, u"type"_s, Qt::CaseInsensitive ) == 0 )
    {
      switch ( layer->type() )
      {
        case Qgis::LayerType::Vector:
          return QCoreApplication::translate( "expressions", "Vector" );
        case Qgis::LayerType::Raster:
          return QCoreApplication::translate( "expressions", "Raster" );
        case Qgis::LayerType::Mesh:
          return QCoreApplication::translate( "expressions", "Mesh" );
        case Qgis::LayerType::VectorTile:
          return QCoreApplication::translate( "expressions", "Vector Tile" );
        case Qgis::LayerType::Plugin:
          return QCoreApplication::translate( "expressions", "Plugin" );
        case Qgis::LayerType::Annotation:
          return QCoreApplication::translate( "expressions", "Annotation" );
        case Qgis::LayerType::PointCloud:
          return QCoreApplication::translate( "expressions", "Point Cloud" );
        case Qgis::LayerType::Group:
          return QCoreApplication::translate( "expressions", "Group" );
        case Qgis::LayerType::TiledScene:
          return QCoreApplication::translate( "expressions", "Tiled Scene" );
      }
    }
    else
    {
      //vector layer methods
      QgsVectorLayer *vLayer = qobject_cast< QgsVectorLayer * >( layer );
      if ( vLayer )
      {
        if ( QString::compare( layerProperty, u"storage_type"_s, Qt::CaseInsensitive ) == 0 )
          return vLayer->storageType();
        else if ( QString::compare( layerProperty, u"geometry_type"_s, Qt::CaseInsensitive ) == 0 )
          return QgsWkbTypes::geometryDisplayString( vLayer->geometryType() );
        else if ( QString::compare( layerProperty, u"feature_count"_s, Qt::CaseInsensitive ) == 0 )
          return QVariant::fromValue( vLayer->featureCount() );
      }
    }

    return QVariant();
  }, foundLayer );

  if ( !foundLayer )
    return QVariant();
  else
    return res;
}

static QVariant fcnDecodeUri( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString uriPart = values.at( 1 ).toString();

  bool foundLayer = false;

  const QVariant res = QgsExpressionUtils::runMapLayerFunctionThreadSafe( values.at( 0 ), context, parent, [parent, uriPart]( QgsMapLayer * layer )-> QVariant
  {
    if ( !layer->dataProvider() )
    {
      parent->setEvalErrorString( QObject::tr( "Layer %1 has invalid data provider" ).arg( layer->name() ) );
      return QVariant();
    }

    const QVariantMap decodedUri = QgsProviderRegistry::instance()->decodeUri( layer->providerType(), layer->dataProvider()->dataSourceUri() );

    if ( !uriPart.isNull() )
    {
      return decodedUri.value( uriPart );
    }
    else
    {
      return decodedUri;
    }
  }, foundLayer );

  if ( !foundLayer )
  {
    parent->setEvalErrorString( QObject::tr( "Function `decode_uri` requires a valid layer." ) );
    return QVariant();
  }
  else
  {
    return res;
  }
}

static QVariant fcnGetRasterBandStat( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const int band = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  const QString layerProperty = QgsExpressionUtils::getStringValue( values.at( 2 ), parent );

  bool foundLayer = false;
  const QVariant res = QgsExpressionUtils::runMapLayerFunctionThreadSafe( values.at( 0 ), context, parent, [parent, band, layerProperty]( QgsMapLayer * layer )-> QVariant
  {
    QgsRasterLayer *rl = qobject_cast< QgsRasterLayer * >( layer );
    if ( !rl )
      return QVariant();

    if ( band < 1 || band > rl->bandCount() )
    {
      parent->setEvalErrorString( QObject::tr( "Invalid band number %1 for layer" ).arg( band ) );
      return QVariant();
    }

    Qgis::RasterBandStatistic stat = Qgis::RasterBandStatistic::NoStatistic;

    if ( QString::compare( layerProperty, u"avg"_s, Qt::CaseInsensitive ) == 0 )
      stat = Qgis::RasterBandStatistic::Mean;
    else if ( QString::compare( layerProperty, u"stdev"_s, Qt::CaseInsensitive ) == 0 )
      stat = Qgis::RasterBandStatistic::StdDev;
    else if ( QString::compare( layerProperty, u"min"_s, Qt::CaseInsensitive ) == 0 )
      stat = Qgis::RasterBandStatistic::Min;
    else if ( QString::compare( layerProperty, u"max"_s, Qt::CaseInsensitive ) == 0 )
      stat = Qgis::RasterBandStatistic::Max;
    else if ( QString::compare( layerProperty, u"range"_s, Qt::CaseInsensitive ) == 0 )
      stat = Qgis::RasterBandStatistic::Range;
    else if ( QString::compare( layerProperty, u"sum"_s, Qt::CaseInsensitive ) == 0 )
      stat = Qgis::RasterBandStatistic::Sum;
    else
    {
      parent->setEvalErrorString( QObject::tr( "Invalid raster statistic: '%1'" ).arg( layerProperty ) );
      return QVariant();
    }

    QgsRasterBandStats stats = rl->dataProvider()->bandStatistics( band, stat );
    switch ( stat )
    {
      case Qgis::RasterBandStatistic::Mean:
        return stats.mean;
      case Qgis::RasterBandStatistic::StdDev:
        return stats.stdDev;
      case Qgis::RasterBandStatistic::Min:
        return stats.minimumValue;
      case Qgis::RasterBandStatistic::Max:
        return stats.maximumValue;
      case Qgis::RasterBandStatistic::Range:
        return stats.range;
      case Qgis::RasterBandStatistic::Sum:
        return stats.sum;
      default:
        break;
    }
    return QVariant();
  }, foundLayer );

  if ( !foundLayer )
  {
#if 0 // for consistency with other functions we should raise an error here, but for compatibility with old projects we don't
    parent->setEvalErrorString( QObject::tr( "Function `raster_statistic` requires a valid raster layer." ) );
#endif
    return QVariant();
  }
  else
  {
    return res;
  }
}

static QVariant fcnArray( const QVariantList &values, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * )
{
  return values;
}

static QVariant fcnArraySort( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  bool ascending = values.value( 1 ).toBool();
  std::sort( list.begin(), list.end(), [ascending]( QVariant a, QVariant b ) -> bool { return ( !ascending ? qgsVariantLessThan( b, a ) : qgsVariantLessThan( a, b ) ); } );
  return list;
}

static QVariant fcnArrayLength( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return QgsExpressionUtils::getListValue( values.at( 0 ), parent ).length();
}

static QVariant fcnArrayContains( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return QVariant( QgsExpressionUtils::getListValue( values.at( 0 ), parent ).contains( values.at( 1 ) ) );
}

static QVariant fcnArrayCount( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return QVariant( QgsExpressionUtils::getListValue( values.at( 0 ), parent ).count( values.at( 1 ) ) );
}

static QVariant fcnArrayAll( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantList listA = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  QVariantList listB = QgsExpressionUtils::getListValue( values.at( 1 ), parent );
  int match = 0;
  for ( const auto &item : listB )
  {
    if ( listA.contains( item ) )
      match++;
  }

  return QVariant( match == listB.count() );
}

static QVariant fcnArrayFind( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return QgsExpressionUtils::getListValue( values.at( 0 ), parent ).indexOf( values.at( 1 ) );
}

static QVariant fcnArrayGet( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  const int pos = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  if ( pos < list.length() && pos >= 0 ) return list.at( pos );
  else if ( pos < 0 && ( list.length() + pos ) >= 0 )
    return list.at( list.length() + pos );
  return QVariant();
}

static QVariant fcnArrayFirst( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  return list.value( 0 );
}

static QVariant fcnArrayLast( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  return list.value( list.size() - 1 );
}

static QVariant fcnArrayMinimum( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  return list.isEmpty() ? QVariant() : *std::min_element( list.constBegin(), list.constEnd(), []( QVariant a, QVariant b ) -> bool { return ( qgsVariantLessThan( a, b ) ); } );
}

static QVariant fcnArrayMaximum( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  return list.isEmpty() ? QVariant() : *std::max_element( list.constBegin(), list.constEnd(), []( QVariant a, QVariant b ) -> bool { return ( qgsVariantLessThan( a, b ) ); } );
}

static QVariant fcnArrayMean( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  int i = 0;
  double total = 0.0;
  for ( const QVariant &item : list )
  {
    switch ( item.userType() )
    {
      case QMetaType::Int:
      case QMetaType::UInt:
      case QMetaType::LongLong:
      case QMetaType::ULongLong:
      case QMetaType::Float:
      case QMetaType::Double:
        total += item.toDouble();
        ++i;
        break;
    }
  }
  return i == 0 ? QVariant() : total / i;
}

static QVariant fcnArrayMedian( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  QVariantList numbers;
  for ( const auto &item : list )
  {
    switch ( item.userType() )
    {
      case QMetaType::Int:
      case QMetaType::UInt:
      case QMetaType::LongLong:
      case QMetaType::ULongLong:
      case QMetaType::Float:
      case QMetaType::Double:
        numbers.append( item );
        break;
    }
  }
  std::sort( numbers.begin(), numbers.end(), []( QVariant a, QVariant b ) -> bool { return ( qgsVariantLessThan( a, b ) ); } );
  const int count = numbers.count();
  if ( count == 0 )
  {
    return QVariant();
  }
  else if ( count % 2 )
  {
    return numbers.at( count / 2 );
  }
  else
  {
    return ( numbers.at( count / 2 - 1 ).toDouble() + numbers.at( count / 2 ).toDouble() ) / 2;
  }
}

static QVariant fcnArraySum( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  int i = 0;
  double total = 0.0;
  for ( const QVariant &item : list )
  {
    switch ( item.userType() )
    {
      case QMetaType::Int:
      case QMetaType::UInt:
      case QMetaType::LongLong:
      case QMetaType::ULongLong:
      case QMetaType::Float:
      case QMetaType::Double:
        total += item.toDouble();
        ++i;
        break;
    }
  }
  return i == 0 ? QVariant() : total;
}

static QVariant convertToSameType( const QVariant &value, QMetaType::Type type )
{
  QVariant result = value;
  ( void )result.convert( static_cast<int>( type ) );
  return result;
}

static QVariant fcnArrayMajority( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node )
{
  const QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  QHash< QVariant, int > hash;
  for ( const auto &item : list )
  {
    ++hash[item];
  }
  const QList< int > occurrences = hash.values();
  if ( occurrences.empty() )
    return QVariantList();

  const int maxValue = *std::max_element( occurrences.constBegin(), occurrences.constEnd() );

  const QString option = values.at( 1 ).toString();
  if ( option.compare( "all"_L1, Qt::CaseInsensitive ) == 0 )
  {
    return convertToSameType( hash.keys( maxValue ), static_cast<QMetaType::Type>( values.at( 0 ).userType() ) );
  }
  else if ( option.compare( "any"_L1, Qt::CaseInsensitive ) == 0 )
  {
    if ( hash.isEmpty() )
      return QVariant();

    return QVariant( hash.key( maxValue ) );
  }
  else if ( option.compare( "median"_L1, Qt::CaseInsensitive ) == 0 )
  {
    return fcnArrayMedian( QVariantList() << convertToSameType( hash.keys( maxValue ), static_cast<QMetaType::Type>( values.at( 0 ).userType() ) ), context, parent, node );
  }
  else if ( option.compare( "real_majority"_L1, Qt::CaseInsensitive ) == 0 )
  {
    if ( maxValue * 2 <= list.size() )
      return QVariant();

    return QVariant( hash.key( maxValue ) );
  }
  else
  {
    parent->setEvalErrorString( QObject::tr( "No such option '%1'" ).arg( option ) );
    return QVariant();
  }
}

static QVariant fcnArrayMinority( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node )
{
  const QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  QHash< QVariant, int > hash;
  for ( const auto &item : list )
  {
    ++hash[item];
  }
  const QList< int > occurrences = hash.values();
  if ( occurrences.empty() )
    return QVariantList();

  const int minValue = *std::min_element( occurrences.constBegin(), occurrences.constEnd() );

  const QString option = values.at( 1 ).toString();
  if ( option.compare( "all"_L1, Qt::CaseInsensitive ) == 0 )
  {
    return convertToSameType( hash.keys( minValue ), static_cast<QMetaType::Type>( values.at( 0 ).userType() ) );
  }
  else if ( option.compare( "any"_L1, Qt::CaseInsensitive ) == 0 )
  {
    if ( hash.isEmpty() )
      return QVariant();

    return QVariant( hash.key( minValue ) );
  }
  else if ( option.compare( "median"_L1, Qt::CaseInsensitive ) == 0 )
  {
    return fcnArrayMedian( QVariantList() << convertToSameType( hash.keys( minValue ), static_cast<QMetaType::Type>( values.at( 0 ).userType() ) ), context, parent, node );
  }
  else if ( option.compare( "real_minority"_L1, Qt::CaseInsensitive ) == 0 )
  {
    if ( hash.isEmpty() )
      return QVariant();

    // Remove the majority, all others are minority
    const int maxValue = *std::max_element( occurrences.constBegin(), occurrences.constEnd() );
    if ( maxValue * 2 > list.size() )
      hash.remove( hash.key( maxValue ) );

    return convertToSameType( hash.keys(), static_cast<QMetaType::Type>( values.at( 0 ).userType() ) );
  }
  else
  {
    parent->setEvalErrorString( QObject::tr( "No such option '%1'" ).arg( option ) );
    return QVariant();
  }
}

static QVariant fcnArrayAppend( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  list.append( values.at( 1 ) );
  return convertToSameType( list, static_cast<QMetaType::Type>( values.at( 0 ).userType() ) );
}

static QVariant fcnArrayPrepend( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  list.prepend( values.at( 1 ) );
  return convertToSameType( list, static_cast<QMetaType::Type>( values.at( 0 ).userType() ) );
}

static QVariant fcnArrayInsert( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  list.insert( QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent ), values.at( 2 ) );
  return convertToSameType( list, static_cast<QMetaType::Type>( values.at( 0 ).userType() ) );
}

static QVariant fcnArrayRemoveAt( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  int position = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  if ( position < 0 )
    position = position + list.length();
  if ( position >= 0 && position < list.length() )
    list.removeAt( position );
  return convertToSameType( list, static_cast<QMetaType::Type>( values.at( 0 ).userType() ) );
}

static QVariant fcnArrayRemoveAll( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( QgsVariantUtils::isNull( values.at( 0 ) ) )
    return QVariant();

  QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );

  const QVariant toRemove = values.at( 1 );
  if ( QgsVariantUtils::isNull( toRemove ) )
  {
    list.erase( std::remove_if( list.begin(), list.end(), []( const QVariant & element )
    {
      return QgsVariantUtils::isNull( element );
    } ), list.end() );
  }
  else
  {
    list.removeAll( toRemove );
  }
  return convertToSameType( list, static_cast<QMetaType::Type>( values.at( 0 ).userType() ) );
}

static QVariant fcnArrayReplace( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( values.count() == 2 && values.at( 1 ).userType() == QMetaType::Type::QVariantMap )
  {
    QVariantMap map = QgsExpressionUtils::getMapValue( values.at( 1 ), parent );

    QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
    for ( QVariantMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it )
    {
      int index = list.indexOf( it.key() );
      while ( index >= 0 )
      {
        list.replace( index, it.value() );
        index = list.indexOf( it.key() );
      }
    }

    return convertToSameType( list, static_cast<QMetaType::Type>( values.at( 0 ).userType() ) );
  }
  else if ( values.count() == 3 )
  {
    QVariantList before;
    QVariantList after;
    bool isSingleReplacement = false;

    if ( !QgsExpressionUtils::isList( values.at( 1 ) ) && values.at( 2 ).userType() != QMetaType::Type::QStringList )
    {
      before = QVariantList() << values.at( 1 );
    }
    else
    {
      before = QgsExpressionUtils::getListValue( values.at( 1 ), parent );
    }

    if ( !QgsExpressionUtils::isList( values.at( 2 ) ) )
    {
      after = QVariantList() << values.at( 2 );
      isSingleReplacement = true;
    }
    else
    {
      after = QgsExpressionUtils::getListValue( values.at( 2 ), parent );
    }

    if ( !isSingleReplacement && before.length() != after.length() )
    {
      parent->setEvalErrorString( QObject::tr( "Invalid pair of array, length not identical" ) );
      return QVariant();
    }

    QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
    for ( int i = 0; i < before.length(); i++ )
    {
      int index = list.indexOf( before.at( i ) );
      while ( index >= 0 )
      {
        list.replace( index, after.at( isSingleReplacement ? 0 : i ) );
        index = list.indexOf( before.at( i ) );
      }
    }

    return convertToSameType( list, static_cast<QMetaType::Type>( values.at( 0 ).userType() ) );
  }
  else
  {
    parent->setEvalErrorString( QObject::tr( "Function array_replace requires 2 or 3 arguments" ) );
    return QVariant();
  }
}

static QVariant fcnArrayPrioritize( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  QVariantList list_new;

  for ( const QVariant &cur : QgsExpressionUtils::getListValue( values.at( 1 ), parent ) )
  {
    while ( list.removeOne( cur ) )
    {
      list_new.append( cur );
    }
  }

  list_new.append( list );

  return convertToSameType( list_new, static_cast<QMetaType::Type>( values.at( 0 ).userType() ) );
}

static QVariant fcnArrayCat( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantList list;
  for ( const QVariant &cur : values )
  {
    list += QgsExpressionUtils::getListValue( cur, parent );
  }
  return convertToSameType( list, static_cast<QMetaType::Type>( values.at( 0 ).userType() ) );
}

static QVariant fcnArraySlice( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  int start_pos = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  const int end_pos = QgsExpressionUtils::getNativeIntValue( values.at( 2 ), parent );
  int slice_length = 0;
  // negative positions means positions taken relative to the end of the array
  if ( start_pos < 0 )
  {
    start_pos = list.length() + start_pos;
  }
  if ( end_pos >= 0 )
  {
    slice_length = end_pos - start_pos + 1;
  }
  else
  {
    slice_length = list.length() + end_pos - start_pos + 1;
  }
  //avoid negative lengths in QList.mid function
  if ( slice_length < 0 )
  {
    slice_length = 0;
  }
  list = list.mid( start_pos, slice_length );
  return list;
}

static QVariant fcnArrayReverse( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  std::reverse( list.begin(), list.end() );
  return list;
}

static QVariant fcnArrayIntersect( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QVariantList array1 = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  const QVariantList array2 = QgsExpressionUtils::getListValue( values.at( 1 ), parent );
  for ( const QVariant &cur : array2 )
  {
    if ( array1.contains( cur ) )
      return QVariant( true );
  }
  return QVariant( false );
}

static QVariant fcnArrayDistinct( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantList array = QgsExpressionUtils::getListValue( values.at( 0 ), parent );

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

static QVariant fcnArrayToString( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantList array = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  QString delimiter = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  QString empty = QgsExpressionUtils::getStringValue( values.at( 2 ), parent );

  QString str;

  for ( QVariantList::const_iterator it = array.constBegin(); it != array.constEnd(); ++it )
  {
    str += ( !( *it ).toString().isEmpty() ) ? ( *it ).toString() : empty;
    if ( it != ( array.constEnd() - 1 ) )
    {
      str += delimiter;
    }
  }

  return QVariant( str );
}

static QVariant fcnStringToArray( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  QString delimiter = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  QString empty = QgsExpressionUtils::getStringValue( values.at( 2 ), parent );

  QStringList list = str.split( delimiter );
  QVariantList array;

  for ( QStringList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it )
  {
    array += ( !( *it ).isEmpty() ) ? *it : empty;
  }

  return array;
}

static QVariant fcnLoadJson( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  QJsonDocument document = QJsonDocument::fromJson( str.toUtf8() );
  if ( document.isNull() )
    return QVariant();

  return document.toVariant();
}

static QVariant fcnWriteJson( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  Q_UNUSED( parent )
  QJsonDocument document = QJsonDocument::fromVariant( values.at( 0 ) );
  return QString( document.toJson( QJsonDocument::Compact ) );
}

static QVariant fcnHstoreToMap( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  if ( str.isEmpty() )
    return QVariantMap();
  str = str.trimmed();

  return QgsHstoreUtils::parse( str );
}

static QVariant fcnMapToHstore( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantMap map = QgsExpressionUtils::getMapValue( values.at( 0 ), parent );
  return QgsHstoreUtils::build( map );
}

static QVariant fcnMap( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantMap result;
  for ( int i = 0; i + 1 < values.length(); i += 2 )
  {
    result.insert( QgsExpressionUtils::getStringValue( values.at( i ), parent ), values.at( i + 1 ) );
  }
  return result;
}

static QVariant fcnMapPrefixKeys( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QVariantMap map = QgsExpressionUtils::getMapValue( values.at( 0 ), parent );
  const QString prefix = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  QVariantMap resultMap;

  for ( auto it = map.cbegin(); it != map.cend(); it++ )
  {
    resultMap.insert( QString( it.key() ).prepend( prefix ), it.value() );
  }

  return resultMap;
}

static QVariant fcnMapGet( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return QgsExpressionUtils::getMapValue( values.at( 0 ), parent ).value( values.at( 1 ).toString() );
}

static QVariant fcnMapExist( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return QgsExpressionUtils::getMapValue( values.at( 0 ), parent ).contains( values.at( 1 ).toString() );
}

static QVariant fcnMapDelete( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantMap map = QgsExpressionUtils::getMapValue( values.at( 0 ), parent );
  map.remove( values.at( 1 ).toString() );
  return map;
}

static QVariant fcnMapInsert( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantMap map = QgsExpressionUtils::getMapValue( values.at( 0 ), parent );
  map.insert( values.at( 1 ).toString(),  values.at( 2 ) );
  return map;
}

static QVariant fcnMapConcat( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantMap result;
  for ( const QVariant &cur : values )
  {
    const QVariantMap curMap = QgsExpressionUtils::getMapValue( cur, parent );
    for ( QVariantMap::const_iterator it = curMap.constBegin(); it != curMap.constEnd(); ++it )
      result.insert( it.key(), it.value() );
  }
  return result;
}

static QVariant fcnMapAKeys( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return QStringList( QgsExpressionUtils::getMapValue( values.at( 0 ), parent ).keys() );
}

static QVariant fcnMapAVals( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return QgsExpressionUtils::getMapValue( values.at( 0 ), parent ).values();
}

static QVariant fcnEnvVar( const QVariantList &values, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * )
{
  const QString envVarName = values.at( 0 ).toString();
  if ( !QProcessEnvironment::systemEnvironment().contains( envVarName ) )
    return QVariant();

  return QProcessEnvironment::systemEnvironment().value( envVarName );
}

static QVariant fcnBaseFileName( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString file = QgsExpressionUtils::getFilePathValue( values.at( 0 ), context, parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( "base_file_name"_L1 ) );
    return QVariant();
  }
  return QFileInfo( file ).completeBaseName();
}

static QVariant fcnFileSuffix( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString file = QgsExpressionUtils::getFilePathValue( values.at( 0 ), context, parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( "file_suffix"_L1 ) );
    return QVariant();
  }
  return QFileInfo( file ).completeSuffix();
}

static QVariant fcnFileExists( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString file = QgsExpressionUtils::getFilePathValue( values.at( 0 ), context, parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( "file_exists"_L1 ) );
    return QVariant();
  }
  return QFileInfo::exists( file );
}

static QVariant fcnFileName( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString file = QgsExpressionUtils::getFilePathValue( values.at( 0 ), context, parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( "file_name"_L1 ) );
    return QVariant();
  }
  return QFileInfo( file ).fileName();
}

static QVariant fcnPathIsFile( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString file = QgsExpressionUtils::getFilePathValue( values.at( 0 ), context, parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( "is_file"_L1 ) );
    return QVariant();
  }
  return QFileInfo( file ).isFile();
}

static QVariant fcnPathIsDir( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString file = QgsExpressionUtils::getFilePathValue( values.at( 0 ), context, parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( "is_directory"_L1 ) );
    return QVariant();
  }
  return QFileInfo( file ).isDir();
}

static QVariant fcnFilePath( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString file = QgsExpressionUtils::getFilePathValue( values.at( 0 ), context, parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( "file_path"_L1 ) );
    return QVariant();
  }
  return QDir::toNativeSeparators( QFileInfo( file ).path() );
}

static QVariant fcnFileSize( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString file = QgsExpressionUtils::getFilePathValue( values.at( 0 ), context, parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( "file_size"_L1 ) );
    return QVariant();
  }
  return QFileInfo( file ).size();
}

static QVariant fcnHash( const QString &str, const QCryptographicHash::Algorithm algorithm )
{
  return QString( QCryptographicHash::hash( str.toUtf8(), algorithm ).toHex() );
}

static QVariant fcnGenericHash( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariant hash;
  QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  QString method = QgsExpressionUtils::getStringValue( values.at( 1 ), parent ).toLower();

  if ( method == "md4"_L1 )
  {
    hash = fcnHash( str, QCryptographicHash::Md4 );
  }
  else if ( method == "md5"_L1 )
  {
    hash = fcnHash( str, QCryptographicHash::Md5 );
  }
  else if ( method == "sha1"_L1 )
  {
    hash = fcnHash( str, QCryptographicHash::Sha1 );
  }
  else if ( method == "sha224"_L1 )
  {
    hash = fcnHash( str, QCryptographicHash::Sha224 );
  }
  else if ( method == "sha256"_L1 )
  {
    hash = fcnHash( str, QCryptographicHash::Sha256 );
  }
  else if ( method == "sha384"_L1 )
  {
    hash = fcnHash( str, QCryptographicHash::Sha384 );
  }
  else if ( method == "sha512"_L1 )
  {
    hash = fcnHash( str, QCryptographicHash::Sha512 );
  }
  else if ( method == "sha3_224"_L1 )
  {
    hash = fcnHash( str, QCryptographicHash::Sha3_224 );
  }
  else if ( method == "sha3_256"_L1 )
  {
    hash = fcnHash( str, QCryptographicHash::Sha3_256 );
  }
  else if ( method == "sha3_384"_L1 )
  {
    hash = fcnHash( str, QCryptographicHash::Sha3_384 );
  }
  else if ( method == "sha3_512"_L1 )
  {
    hash = fcnHash( str, QCryptographicHash::Sha3_512 );
  }
  else if ( method == "keccak_224"_L1 )
  {
    hash = fcnHash( str, QCryptographicHash::Keccak_224 );
  }
  else if ( method == "keccak_256"_L1 )
  {
    hash = fcnHash( str, QCryptographicHash::Keccak_256 );
  }
  else if ( method == "keccak_384"_L1 )
  {
    hash = fcnHash( str, QCryptographicHash::Keccak_384 );
  }
  else if ( method == "keccak_512"_L1 )
  {
    hash = fcnHash( str, QCryptographicHash::Keccak_512 );
  }
  else
  {
    parent->setEvalErrorString( QObject::tr( "Hash method %1 is not available on this system." ).arg( str ) );
  }
  return hash;
}

static QVariant fcnHashMd5( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnHash( QgsExpressionUtils::getStringValue( values.at( 0 ), parent ), QCryptographicHash::Md5 );
}

static QVariant fcnHashSha256( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnHash( QgsExpressionUtils::getStringValue( values.at( 0 ), parent ), QCryptographicHash::Sha256 );
}

static QVariant fcnToBase64( const QVariantList &values, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * )
{
  const QByteArray input = values.at( 0 ).toByteArray();
  return QVariant( QString( input.toBase64() ) );
}

static QVariant fcnToFormUrlEncode( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QVariantMap map = QgsExpressionUtils::getMapValue( values.at( 0 ), parent );
  QUrlQuery query;
  for ( auto it = map.cbegin(); it != map.cend(); it++ )
  {
    query.addQueryItem( it.key(), it.value().toString() );
  }
  return query.toString( QUrl::ComponentFormattingOption::FullyEncoded );
}

static QVariant fcnFromBase64( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString value = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  const QByteArray base64 = value.toLocal8Bit();
  const QByteArray decoded = QByteArray::fromBase64( base64 );
  return QVariant( decoded );
}

typedef bool ( QgsGeometry::*RelationFunction )( const QgsGeometry &geometry ) const;

static QVariant executeGeomOverlay( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const RelationFunction &relationFunction, bool invert = false, double bboxGrow = 0, bool isNearestFunc = false, bool isIntersectsFunc = false )
{

  if ( ! context )
  {
    parent->setEvalErrorString( u"This function was called without an expression context."_s );
    return QVariant();
  }

  const QVariant sourceLayerRef = context->variable( u"layer"_s ); //used to detect if sourceLayer and targetLayer are the same
  // TODO this function is NOT thread safe
  Q_NOWARN_DEPRECATED_PUSH
  QgsVectorLayer *sourceLayer = QgsExpressionUtils::getVectorLayer( sourceLayerRef, context, parent );
  Q_NOWARN_DEPRECATED_POP

  QgsFeatureRequest request;
  request.setTimeout( 10000 );
  request.setRequestMayBeNested( true );
  request.setFeedback( context->feedback() );

  // First parameter is the overlay layer
  QgsExpressionNode *node = QgsExpressionUtils::getNode( values.at( 0 ), parent );
  ENSURE_NO_EVAL_ERROR

  const bool layerCanBeCached = node->isStatic( parent, context );
  QVariant targetLayerValue = node->eval( parent, context );
  ENSURE_NO_EVAL_ERROR

  // Second parameter is the expression to evaluate (or null for testonly)
  node = QgsExpressionUtils::getNode( values.at( 1 ), parent );
  ENSURE_NO_EVAL_ERROR
  QString subExpString = node->dump();

  bool testOnly = ( subExpString == "NULL" );
  // TODO this function is NOT thread safe
  Q_NOWARN_DEPRECATED_PUSH
  QgsVectorLayer *targetLayer = QgsExpressionUtils::getVectorLayer( targetLayerValue, context, parent );
  Q_NOWARN_DEPRECATED_POP
  if ( !targetLayer ) // No layer, no joy
  {
    parent->setEvalErrorString( QObject::tr( "Layer '%1' could not be loaded." ).arg( targetLayerValue.toString() ) );
    return QVariant();
  }

  // Third parameter is the filtering expression
  node = QgsExpressionUtils::getNode( values.at( 2 ), parent );
  ENSURE_NO_EVAL_ERROR
  QString filterString = node->dump();
  if ( filterString != "NULL" )
  {
    request.setFilterExpression( filterString ); //filter cached features
  }

  // Fourth parameter is the limit
  node = QgsExpressionUtils::getNode( values.at( 3 ), parent ); //in expressions overlay functions throw the exception: Eval Error: Cannot convert '' to int
  ENSURE_NO_EVAL_ERROR
  QVariant limitValue = node->eval( parent, context );
  ENSURE_NO_EVAL_ERROR
  qlonglong limit = QgsExpressionUtils::getIntValue( limitValue, parent );

  // Fifth parameter (for nearest only) is the max distance
  double max_distance = 0;
  if ( isNearestFunc )   //maxdistance param handling
  {
    node = QgsExpressionUtils::getNode( values.at( 4 ), parent );
    ENSURE_NO_EVAL_ERROR
    QVariant distanceValue = node->eval( parent, context );
    ENSURE_NO_EVAL_ERROR
    max_distance = QgsExpressionUtils::getDoubleValue( distanceValue, parent );
  }

  // Fifth or sixth (for nearest only) parameter is the cache toggle
  node = QgsExpressionUtils::getNode( values.at( isNearestFunc ? 5 : 4 ), parent );
  ENSURE_NO_EVAL_ERROR
  QVariant cacheValue = node->eval( parent, context );
  ENSURE_NO_EVAL_ERROR
  bool cacheEnabled = cacheValue.toBool();

  // Sixth parameter (for intersects only) is the min overlap (area or length)
  // Seventh parameter (for intersects only) is the min inscribed circle radius
  // Eighth parameter (for intersects only) is the return_details
  // Ninth parameter (for intersects only) is the sort_by_intersection_size flag
  double minOverlap { -1 };
  double minInscribedCircleRadius { -1 };
  bool returnDetails = false;  //#spellok
  bool sortByMeasure = false;
  bool sortAscending = false;
  bool requireMeasures = false;
  bool overlapOrRadiusFilter = false;
  if ( isIntersectsFunc )
  {

    node = QgsExpressionUtils::getNode( values.at( 5 ), parent ); //in expressions overlay functions throw the exception: Eval Error: Cannot convert '' to int
    ENSURE_NO_EVAL_ERROR
    const QVariant minOverlapValue = node->eval( parent, context );
    ENSURE_NO_EVAL_ERROR
    minOverlap = QgsExpressionUtils::getDoubleValue( minOverlapValue, parent );
    node = QgsExpressionUtils::getNode( values.at( 6 ), parent ); //in expressions overlay functions throw the exception: Eval Error: Cannot convert '' to int
    ENSURE_NO_EVAL_ERROR
    const QVariant minInscribedCircleRadiusValue = node->eval( parent, context );
    ENSURE_NO_EVAL_ERROR
    minInscribedCircleRadius = QgsExpressionUtils::getDoubleValue( minInscribedCircleRadiusValue, parent );
    node = QgsExpressionUtils::getNode( values.at( 7 ), parent );
    // Return measures is only effective when an expression is set
    returnDetails = !testOnly && node->eval( parent, context ).toBool();  //#spellok
    node = QgsExpressionUtils::getNode( values.at( 8 ), parent );
    // Sort by measures is only effective when an expression is set
    const QString sorting { node->eval( parent, context ).toString().toLower() };
    sortByMeasure = !testOnly && ( sorting.startsWith( "asc" ) || sorting.startsWith( "des" ) );
    sortAscending = sorting.startsWith( "asc" );
    requireMeasures = sortByMeasure || returnDetails;  //#spellok
    overlapOrRadiusFilter = minInscribedCircleRadius != -1 || minOverlap != -1;
  }


  FEAT_FROM_CONTEXT( context, feat )
  const QgsGeometry geometry = feat.geometry();

  if ( sourceLayer && targetLayer->crs() != sourceLayer->crs() )
  {
    QgsCoordinateTransformContext TransformContext = context->variable( u"_project_transform_context"_s ).value<QgsCoordinateTransformContext>();
    request.setDestinationCrs( sourceLayer->crs(), TransformContext ); //if crs are not the same, cached target will be reprojected to source crs
  }

  bool sameLayers = ( sourceLayer && sourceLayer->id() == targetLayer->id() );

  QgsRectangle intDomain = geometry.boundingBox();
  if ( bboxGrow != 0 )
  {
    intDomain.grow( bboxGrow ); //optional parameter to enlarge boundary context for touches and equals methods
  }

  const QString cacheBase { u"%1:%2:%3"_s.arg( targetLayer->id(), subExpString, filterString ) };

  // Cache (a local spatial index) is always enabled for nearest function (as we need QgsSpatialIndex::nearestNeighbor)
  // Otherwise, it can be toggled by the user
  QgsSpatialIndex spatialIndex;
  QgsVectorLayer *cachedTarget;
  QList<QgsFeature> features;
  if ( isNearestFunc || ( layerCanBeCached && cacheEnabled ) )
  {
    // If the cache (local spatial index) is enabled, we materialize the whole
    // layer, then do the request on that layer instead.
    const QString cacheLayer { u"ovrlaylyr:%1"_s.arg( cacheBase ) };
    const QString cacheIndex { u"ovrlayidx:%1"_s.arg( cacheBase ) };

    if ( !context->hasCachedValue( cacheLayer ) ) // should check for same crs. if not the same we could think to reproject target layer before charging cache
    {
      cachedTarget = targetLayer->materialize( request );
      if ( layerCanBeCached )
        context->setCachedValue( cacheLayer, QVariant::fromValue( cachedTarget ) );
    }
    else
    {
      cachedTarget = context->cachedValue( cacheLayer ).value<QgsVectorLayer *>();
    }

    if ( !context->hasCachedValue( cacheIndex ) )
    {
      spatialIndex = QgsSpatialIndex( cachedTarget->getFeatures(), nullptr, QgsSpatialIndex::FlagStoreFeatureGeometries );
      if ( layerCanBeCached )
        context->setCachedValue( cacheIndex, QVariant::fromValue( spatialIndex ) );
    }
    else
    {
      spatialIndex = context->cachedValue( cacheIndex ).value<QgsSpatialIndex>();
    }

    QList<QgsFeatureId> fidsList;
    if ( isNearestFunc )
    {
      fidsList = spatialIndex.nearestNeighbor( geometry, sameLayers ? limit + 1 : limit, max_distance );
    }
    else
    {
      fidsList = spatialIndex.intersects( intDomain );
    }

    QListIterator<QgsFeatureId> i( fidsList );
    while ( i.hasNext() )
    {
      QgsFeatureId fId2 = i.next();
      if ( sameLayers && feat.id() == fId2 )
        continue;
      features.append( cachedTarget->getFeature( fId2 ) );
    }

  }
  else
  {
    // If the cache (local spatial index) is not enabled, we directly
    // get the features from the target layer
    request.setFilterRect( intDomain );
    QgsFeatureIterator fit = targetLayer->getFeatures( request );
    QgsFeature feat2;
    while ( fit.nextFeature( feat2 ) )
    {
      if ( sameLayers && feat.id() == feat2.id() )
        continue;
      features.append( feat2 );
    }
  }

  QgsExpression subExpression;
  QgsExpressionContext subContext;
  if ( !testOnly )
  {
    const QString expCacheKey { u"exp:%1"_s.arg( cacheBase ) };
    const QString ctxCacheKey { u"ctx:%1"_s.arg( cacheBase ) };

    if ( !context->hasCachedValue( expCacheKey ) || !context->hasCachedValue( ctxCacheKey ) )
    {
      subExpression = QgsExpression( subExpString );
      subContext = QgsExpressionContext( QgsExpressionContextUtils::globalProjectLayerScopes( targetLayer ) );
      subExpression.prepare( &subContext );
    }
    else
    {
      subExpression = context->cachedValue( expCacheKey ).value<QgsExpression>();
      subContext = context->cachedValue( ctxCacheKey ).value<QgsExpressionContext>();
    }
  }

  // //////////////////////////////////////////////////////////////////
  // Helper functions for geometry tests

  // Test function for linestring geometries, returns TRUE if test passes
  auto testLinestring = [minOverlap, requireMeasures]( const QgsGeometry intersection, double & overlapValue ) -> bool
  {
    bool testResult { false };
    // For return measures:
    QVector<double> overlapValues;
    const QgsGeometry merged { intersection.mergeLines() };
    for ( auto it = merged.const_parts_begin(); ! testResult && it != merged.const_parts_end(); ++it )
    {
      const QgsCurve *geom = qgsgeometry_cast< const QgsCurve * >( *it );
      // Check min overlap for intersection (if set)
      if ( minOverlap != -1  || requireMeasures )
      {
        overlapValue = geom->length();
        overlapValues.append( overlapValue );
        if ( minOverlap != -1 )
        {
          if ( overlapValue >= minOverlap )
          {
            testResult = true;
          }
          else
          {
            continue;
          }
        }
      }
    }

    if ( ! overlapValues.isEmpty() )
    {
      overlapValue = *std::max_element( overlapValues.cbegin(), overlapValues.cend() );
    }

    return testResult;
  };

  // Test function for polygon geometries, returns TRUE if test passes
  auto testPolygon = [minOverlap, requireMeasures, minInscribedCircleRadius]( const QgsGeometry intersection, double & radiusValue, double & overlapValue ) -> bool
  {
    // overlap and inscribed circle tests must be checked both (if the values are != -1)
    bool testResult { false };
    // For return measures:
    QVector<double> overlapValues;
    QVector<double> radiusValues;
    for ( auto it = intersection.const_parts_begin(); ( ! testResult || requireMeasures ) && it != intersection.const_parts_end(); ++it )
    {
      const QgsCurvePolygon *geom = qgsgeometry_cast< const QgsCurvePolygon * >( *it );
      // Check min overlap for intersection (if set)
      if ( minOverlap != -1 || requireMeasures )
      {
        overlapValue = geom->area();
        overlapValues.append( geom->area() );
        if ( minOverlap != - 1 )
        {
          if ( overlapValue >= minOverlap )
          {
            testResult = true;
          }
          else
          {
            continue;
          }
        }
      }

      // Check min inscribed circle radius for intersection (if set)
      if ( minInscribedCircleRadius != -1 || requireMeasures )
      {
        const QgsRectangle bbox = geom->boundingBox();
        const double width = bbox.width();
        const double height = bbox.height();
        const double size = width > height ? width : height;
        const double tolerance = size / 100.0;
        radiusValue = QgsGeos( geom ).maximumInscribedCircle( tolerance )->length();
        testResult = radiusValue >= minInscribedCircleRadius;
        radiusValues.append( radiusValues );
      }
    } // end for parts

    // Get the max values
    if ( !radiusValues.isEmpty() )
    {
      radiusValue = *std::max_element( radiusValues.cbegin(), radiusValues.cend() );
    }

    if ( ! overlapValues.isEmpty() )
    {
      overlapValue = *std::max_element( overlapValues.cbegin(), overlapValues.cend() );
    }

    return testResult;

  };


  bool found = false;
  int foundCount = 0;
  QVariantList results;

  QListIterator<QgsFeature> i( features );
  while ( i.hasNext() && ( sortByMeasure || limit == -1 || foundCount < limit ) )
  {

    QgsFeature feat2 = i.next();


    if ( ! relationFunction || ( geometry.*relationFunction )( feat2.geometry() ) ) // Calls the method provided as template argument for the function (e.g. QgsGeometry::intersects)
    {

      double overlapValue = -1;
      double radiusValue = -1;

      if ( isIntersectsFunc && ( requireMeasures || overlapOrRadiusFilter ) )
      {

        QgsGeometry intersection { geometry.intersection( feat2.geometry() ) };

        // Pre-process collections: if the tested geometry is a polygon we take the polygons from the collection
        if ( intersection.wkbType() == Qgis::WkbType::GeometryCollection )
        {
          const QVector<QgsGeometry> geometries { intersection.asGeometryCollection() };
          intersection = QgsGeometry();
          QgsMultiPolygonXY poly;
          QgsMultiPolylineXY line;
          QgsMultiPointXY point;
          for ( const auto &geom : std::as_const( geometries ) )
          {
            switch ( geom.type() )
            {
              case Qgis::GeometryType::Polygon:
              {
                poly.append( geom.asPolygon() );
                break;
              }
              case Qgis::GeometryType::Line:
              {
                line.append( geom.asPolyline() );
                break;
              }
              case Qgis::GeometryType::Point:
              {
                point.append( geom.asPoint() );
                break;
              }
              case Qgis::GeometryType::Unknown:
              case Qgis::GeometryType::Null:
              {
                break;
              }
            }
          }

          switch ( geometry.type() )
          {
            case Qgis::GeometryType::Polygon:
            {
              intersection = QgsGeometry::fromMultiPolygonXY( poly );
              break;
            }
            case Qgis::GeometryType::Line:
            {
              intersection = QgsGeometry::fromMultiPolylineXY( line );
              break;
            }
            case Qgis::GeometryType::Point:
            {
              intersection = QgsGeometry::fromMultiPointXY( point );
              break;
            }
            case Qgis::GeometryType::Unknown:
            case Qgis::GeometryType::Null:
            {
              break;
            }
          }
        }

        // Depending on the intersection geometry type and on the geometry type of
        // the tested geometry we can run different tests and collect different measures
        // that can be used for sorting (if required).
        switch ( intersection.type() )
        {

          case Qgis::GeometryType::Polygon:
          {

            // Overlap and inscribed circle tests must be checked both (if the values are != -1)
            bool testResult { testPolygon( intersection, radiusValue, overlapValue ) };

            if ( ! testResult && overlapOrRadiusFilter )
            {
              continue;
            }

            break;
          }

          case Qgis::GeometryType::Line:
          {

            // If the intersection is a linestring and a minimum circle is required
            // we can discard this result immediately.
            if ( minInscribedCircleRadius != -1 )
            {
              continue;
            }

            // Otherwise a test for the overlap value is performed.
            const bool testResult { testLinestring( intersection, overlapValue ) };

            if ( ! testResult && overlapOrRadiusFilter )
            {
              continue;
            }

            break;
          }

          case Qgis::GeometryType::Point:
          {

            // If the intersection is a point and a minimum circle is required
            // we can discard this result immediately.
            if ( minInscribedCircleRadius != -1 )
            {
              continue;
            }

            bool testResult { false };
            if ( minOverlap != -1 || requireMeasures )
            {
              // Initially set this to 0 because it's a point intersection...
              overlapValue = 0;
              // ... but if the target geometry is not a point and the source
              // geometry is a point, we must record the length or the area
              // of the intersected geometry and use that as a measure for
              // sorting or reporting.
              if ( geometry.type() == Qgis::GeometryType::Point )
              {
                switch ( feat2.geometry().type() )
                {
                  case Qgis::GeometryType::Unknown:
                  case Qgis::GeometryType::Null:
                  case Qgis::GeometryType::Point:
                  {
                    break;
                  }
                  case Qgis::GeometryType::Line:
                  {
                    testResult = testLinestring( feat2.geometry(), overlapValue );
                    break;
                  }
                  case Qgis::GeometryType::Polygon:
                  {
                    testResult = testPolygon( feat2.geometry(), radiusValue, overlapValue );
                    break;
                  }
                }
              }

              if ( ! testResult && overlapOrRadiusFilter )
              {
                continue;
              }

            }
            break;
          }

          case Qgis::GeometryType::Null:
          case Qgis::GeometryType::Unknown:
          {
            continue;
          }
        }
      }

      found = true;
      foundCount++;

      // We just want a single boolean result if there is any intersect: finish and return true
      if ( testOnly )
        break;

      if ( !invert )
      {
        // We want a list of attributes / geometries / other expression values, evaluate now
        subContext.setFeature( feat2 );
        const QVariant expResult = subExpression.evaluate( &subContext );

        if ( requireMeasures )
        {
          QVariantMap resultRecord;
          resultRecord.insert( u"id"_s, feat2.id() );
          resultRecord.insert( u"result"_s, expResult );
          // Overlap is always added because return measures was set
          resultRecord.insert( u"overlap"_s, overlapValue );
          // Radius is only added when is different than -1 (because for linestrings is not set)
          if ( radiusValue != -1 )
          {
            resultRecord.insert( u"radius"_s, radiusValue );
          }
          results.append( resultRecord );
        }
        else
        {
          results.append( expResult );
        }
      }
      else
      {
        // If not, results is a list of found ids, which we'll inverse and evaluate below
        results.append( feat2.id() );
      }
    }
  }

  if ( testOnly )
  {
    if ( invert )
      found = !found;//for disjoint condition
    return found;
  }

  if ( !invert )
  {
    if ( requireMeasures )
    {
      if ( sortByMeasure )
      {
        std::sort( results.begin(), results.end(), [ sortAscending ]( const QVariant & recordA, const QVariant & recordB ) -> bool
        {
          return sortAscending ?
          recordB.toMap().value( u"overlap"_s ).toDouble() > recordA.toMap().value( u"overlap"_s ).toDouble()
          : recordA.toMap().value( u"overlap"_s ).toDouble() > recordB.toMap().value( u"overlap"_s ).toDouble();
        } );
      }
      // Resize
      if ( limit > 0 && results.size() > limit )
      {
        results.erase( results.begin() + limit );
      }

      if ( ! returnDetails )  //#spellok
      {
        QVariantList expResults;
        for ( auto it = results.constBegin(); it != results.constEnd(); ++it )
        {
          expResults.append( it->toMap().value( u"result"_s ) );
        }
        return expResults;
      }
    }

    return results;
  }

  // for disjoint condition returns the results for cached layers not intersected feats
  QVariantList disjoint_results;
  QgsFeature feat2;
  QgsFeatureRequest request2;
  request2.setLimit( limit );
  if ( context )
    request2.setFeedback( context->feedback() );
  QgsFeatureIterator fi = targetLayer->getFeatures( request2 );
  while ( fi.nextFeature( feat2 ) )
  {
    if ( !results.contains( feat2.id() ) )
    {
      subContext.setFeature( feat2 );
      disjoint_results.append( subExpression.evaluate( &subContext ) );
    }
  }
  return disjoint_results;

}

// Intersect functions:

static QVariant fcnGeomOverlayIntersects( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return executeGeomOverlay( values, context, parent, &QgsGeometry::intersects, false, 0, false, true );
}

static QVariant fcnGeomOverlayContains( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return executeGeomOverlay( values, context, parent, &QgsGeometry::contains );
}

static QVariant fcnGeomOverlayCrosses( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return executeGeomOverlay( values, context, parent, &QgsGeometry::crosses );
}

static QVariant fcnGeomOverlayEquals( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return executeGeomOverlay( values, context, parent, &QgsGeometry::equals, false, 0.01 );  //grow amount should adapt to current units
}

static QVariant fcnGeomOverlayTouches( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return executeGeomOverlay( values, context, parent, &QgsGeometry::touches, false, 0.01 ); //grow amount should adapt to current units
}

static QVariant fcnGeomOverlayWithin( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return executeGeomOverlay( values, context, parent, &QgsGeometry::within );
}

static QVariant fcnGeomOverlayDisjoint( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return executeGeomOverlay( values, context, parent, &QgsGeometry::intersects, true, 0, false, true );
}

static QVariant fcnGeomOverlayNearest( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return executeGeomOverlay( values, context, parent, nullptr, false, 0, true );
}

const QList<QgsExpressionFunction *> &QgsExpression::Functions()
{
  // The construction of the list isn't thread-safe, and without the mutex,
  // crashes in the WFS provider may occur, since it can parse expressions
  // in parallel.
  // The mutex needs to be recursive.
  QMutexLocker locker( &sFunctionsMutex );

  QList<QgsExpressionFunction *> &functions = *sFunctions();

  if ( functions.isEmpty() )
  {
    QgsExpressionFunction::ParameterList aggParams = QgsExpressionFunction::ParameterList()
        << QgsExpressionFunction::Parameter( u"expression"_s )
        << QgsExpressionFunction::Parameter( u"group_by"_s, true )
        << QgsExpressionFunction::Parameter( u"filter"_s, true );

    QgsExpressionFunction::ParameterList aggParamsConcat = aggParams;
    aggParamsConcat <<  QgsExpressionFunction::Parameter( u"concatenator"_s, true )
                    << QgsExpressionFunction::Parameter( u"order_by"_s, true, QVariant(), true );

    QgsExpressionFunction::ParameterList aggParamsArray = aggParams;
    aggParamsArray << QgsExpressionFunction::Parameter( u"order_by"_s, true, QVariant(), true );

    functions
        << new QgsStaticExpressionFunction( u"sqrt"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ), fcnSqrt, u"Math"_s )
        << new QgsStaticExpressionFunction( u"radians"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"degrees"_s ), fcnRadians, u"Math"_s )
        << new QgsStaticExpressionFunction( u"degrees"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"radians"_s ), fcnDegrees, u"Math"_s )
        << new QgsStaticExpressionFunction( u"azimuth"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"point_a"_s ) << QgsExpressionFunction::Parameter( u"point_b"_s ), fcnAzimuth, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"bearing"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"point_a"_s ) << QgsExpressionFunction::Parameter( u"point_b"_s ) << QgsExpressionFunction::Parameter( u"source_crs"_s, true, QVariant() ) << QgsExpressionFunction::Parameter( u"ellipsoid"_s, true, QVariant() ), fcnBearing, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"inclination"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"point_a"_s ) << QgsExpressionFunction::Parameter( u"point_b"_s ), fcnInclination, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"project"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"point"_s ) << QgsExpressionFunction::Parameter( u"distance"_s ) << QgsExpressionFunction::Parameter( u"azimuth"_s ) << QgsExpressionFunction::Parameter( u"elevation"_s, true, M_PI_2 ), fcnProject, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"abs"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ), fcnAbs, u"Math"_s )
        << new QgsStaticExpressionFunction( u"cos"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"angle"_s ), fcnCos, u"Math"_s )
        << new QgsStaticExpressionFunction( u"sin"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"angle"_s ), fcnSin, u"Math"_s )
        << new QgsStaticExpressionFunction( u"tan"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"angle"_s ), fcnTan, u"Math"_s )
        << new QgsStaticExpressionFunction( u"asin"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ), fcnAsin, u"Math"_s )
        << new QgsStaticExpressionFunction( u"acos"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ), fcnAcos, u"Math"_s )
        << new QgsStaticExpressionFunction( u"atan"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ), fcnAtan, u"Math"_s )
        << new QgsStaticExpressionFunction( u"atan2"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"dx"_s ) << QgsExpressionFunction::Parameter( u"dy"_s ), fcnAtan2, u"Math"_s )
        << new QgsStaticExpressionFunction( u"exp"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ), fcnExp, u"Math"_s )
        << new QgsStaticExpressionFunction( u"ln"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ), fcnLn, u"Math"_s )
        << new QgsStaticExpressionFunction( u"log10"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ), fcnLog10, u"Math"_s )
        << new QgsStaticExpressionFunction( u"log"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"base"_s ) << QgsExpressionFunction::Parameter( u"value"_s ), fcnLog, u"Math"_s )
        << new QgsStaticExpressionFunction( u"round"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ) << QgsExpressionFunction::Parameter( u"places"_s, true, 0 ), fcnRound, u"Math"_s );

    QgsStaticExpressionFunction *randFunc = new QgsStaticExpressionFunction( u"rand"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"min"_s ) << QgsExpressionFunction::Parameter( u"max"_s ) << QgsExpressionFunction::Parameter( u"seed"_s, true ), fcnRnd, u"Math"_s );
    randFunc->setIsStatic( false );
    functions << randFunc;

    QgsStaticExpressionFunction *randfFunc = new QgsStaticExpressionFunction( u"randf"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"min"_s, true, 0.0 ) << QgsExpressionFunction::Parameter( u"max"_s, true, 1.0 ) << QgsExpressionFunction::Parameter( u"seed"_s, true ), fcnRndF, u"Math"_s );
    randfFunc->setIsStatic( false );
    functions << randfFunc;

    functions
        << new QgsStaticExpressionFunction( u"max"_s, -1, fcnMax, u"Math"_s, QString(), false, QSet<QString>(), false, QStringList(), /* handlesNull = */ true )
        << new QgsStaticExpressionFunction( u"min"_s, -1, fcnMin, u"Math"_s, QString(), false, QSet<QString>(), false, QStringList(), /* handlesNull = */ true )
        << new QgsStaticExpressionFunction( u"clamp"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"min"_s ) << QgsExpressionFunction::Parameter( u"value"_s ) << QgsExpressionFunction::Parameter( u"max"_s ), fcnClamp, u"Math"_s )
        << new QgsStaticExpressionFunction( u"scale_linear"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s )  << QgsExpressionFunction::Parameter( u"domain_min"_s ) << QgsExpressionFunction::Parameter( u"domain_max"_s ) << QgsExpressionFunction::Parameter( u"range_min"_s ) << QgsExpressionFunction::Parameter( u"range_max"_s ), fcnLinearScale, u"Math"_s )
        << new QgsStaticExpressionFunction( u"scale_polynomial"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s )  << QgsExpressionFunction::Parameter( u"domain_min"_s ) << QgsExpressionFunction::Parameter( u"domain_max"_s ) << QgsExpressionFunction::Parameter( u"range_min"_s ) << QgsExpressionFunction::Parameter( u"range_max"_s ) << QgsExpressionFunction::Parameter( u"exponent"_s ), fcnPolynomialScale, u"Math"_s, QString(), false, QSet<QString>(), false, QStringList() << u"scale_exp"_s )
        << new QgsStaticExpressionFunction( u"scale_exponential"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s )  << QgsExpressionFunction::Parameter( u"domain_min"_s ) << QgsExpressionFunction::Parameter( u"domain_max"_s ) << QgsExpressionFunction::Parameter( u"range_min"_s ) << QgsExpressionFunction::Parameter( u"range_max"_s ) << QgsExpressionFunction::Parameter( u"exponent"_s ), fcnExponentialScale, u"Math"_s )
        << new QgsStaticExpressionFunction( u"floor"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ), fcnFloor, u"Math"_s )
        << new QgsStaticExpressionFunction( u"ceil"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ), fcnCeil, u"Math"_s )
        << new QgsStaticExpressionFunction( u"pi"_s, 0, fcnPi, u"Math"_s, QString(), false, QSet<QString>(), false, QStringList() << u"$pi"_s )
        << new QgsStaticExpressionFunction( u"to_bool"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ), fcnToBool, u"Conversions"_s, QString(), false, QSet<QString>(), false, QStringList() << u"tobool"_s, /* handlesNull = */ true )
        << new QgsStaticExpressionFunction( u"to_int"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ), fcnToInt, u"Conversions"_s, QString(), false, QSet<QString>(), false, QStringList() << u"toint"_s )
        << new QgsStaticExpressionFunction( u"to_real"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ), fcnToReal, u"Conversions"_s, QString(), false, QSet<QString>(), false, QStringList() << u"toreal"_s )
        << new QgsStaticExpressionFunction( u"to_string"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ), fcnToString, QStringList() << u"Conversions"_s << u"String"_s, QString(), false, QSet<QString>(), false, QStringList() << u"tostring"_s )
        << new QgsStaticExpressionFunction( u"to_datetime"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ) << QgsExpressionFunction::Parameter( u"format"_s, true, QVariant() ) << QgsExpressionFunction::Parameter( u"language"_s, true, QVariant() ), fcnToDateTime, QStringList() << u"Conversions"_s << u"Date and Time"_s, QString(), false, QSet<QString>(), false, QStringList() << u"todatetime"_s )
        << new QgsStaticExpressionFunction( u"to_date"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ) << QgsExpressionFunction::Parameter( u"format"_s, true, QVariant() ) << QgsExpressionFunction::Parameter( u"language"_s, true, QVariant() ), fcnToDate, QStringList() << u"Conversions"_s << u"Date and Time"_s, QString(), false, QSet<QString>(), false, QStringList() << u"todate"_s )
        << new QgsStaticExpressionFunction( u"to_time"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ) << QgsExpressionFunction::Parameter( u"format"_s, true, QVariant() ) << QgsExpressionFunction::Parameter( u"language"_s, true, QVariant() ), fcnToTime, QStringList() << u"Conversions"_s << u"Date and Time"_s, QString(), false, QSet<QString>(), false, QStringList() << u"totime"_s )
        << new QgsStaticExpressionFunction( u"to_interval"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ), fcnToInterval, QStringList() << u"Conversions"_s << u"Date and Time"_s, QString(), false, QSet<QString>(), false, QStringList() << u"tointerval"_s )
        << new QgsStaticExpressionFunction( u"to_dm"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ) << QgsExpressionFunction::Parameter( u"axis"_s ) << QgsExpressionFunction::Parameter( u"precision"_s ) << QgsExpressionFunction::Parameter( u"formatting"_s, true ), fcnToDegreeMinute, u"Conversions"_s, QString(), false, QSet<QString>(), false, QStringList() << u"todm"_s )
        << new QgsStaticExpressionFunction( u"to_dms"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ) << QgsExpressionFunction::Parameter( u"axis"_s ) << QgsExpressionFunction::Parameter( u"precision"_s ) << QgsExpressionFunction::Parameter( u"formatting"_s, true ), fcnToDegreeMinuteSecond, u"Conversions"_s, QString(), false, QSet<QString>(), false, QStringList() << u"todms"_s )
        << new QgsStaticExpressionFunction( u"to_decimal"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ), fcnToDecimal, u"Conversions"_s, QString(), false, QSet<QString>(), false, QStringList() << u"todecimal"_s )
        << new QgsStaticExpressionFunction( u"extract_degrees"_s, { QgsExpressionFunction::Parameter{ u"value"_s } }, fcnExtractDegrees, u"Conversions"_s )
        << new QgsStaticExpressionFunction( u"extract_minutes"_s, { QgsExpressionFunction::Parameter{ u"value"_s } }, fcnExtractMinutes, u"Conversions"_s )
        << new QgsStaticExpressionFunction( u"extract_seconds"_s, { QgsExpressionFunction::Parameter{ u"value"_s } }, fcnExtractSeconds, u"Conversions"_s )
        << new QgsStaticExpressionFunction( u"coalesce"_s, -1, fcnCoalesce, u"Conditionals"_s, QString(), false, QSet<QString>(), false, QStringList(), true )
        << new QgsStaticExpressionFunction( u"nullif"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value1"_s ) << QgsExpressionFunction::Parameter( u"value2"_s ), fcnNullIf, u"Conditionals"_s )
        << new QgsStaticExpressionFunction( u"if"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"condition"_s ) << QgsExpressionFunction::Parameter( u"result_when_true"_s )  << QgsExpressionFunction::Parameter( u"result_when_false"_s ), fcnIf, u"Conditionals"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"try"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"expression"_s ) << QgsExpressionFunction::Parameter( u"alternative"_s, true, QVariant() ), fcnTry, u"Conditionals"_s, QString(), false, QSet<QString>(), true )

        << new QgsStaticExpressionFunction( u"aggregate"_s,
                                            QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( u"layer"_s )
                                            << QgsExpressionFunction::Parameter( u"aggregate"_s )
                                            << QgsExpressionFunction::Parameter( u"expression"_s, false, QVariant(), true )
                                            << QgsExpressionFunction::Parameter( u"filter"_s, true, QVariant(), true )
                                            << QgsExpressionFunction::Parameter( u"concatenator"_s, true )
                                            << QgsExpressionFunction::Parameter( u"order_by"_s, true, QVariant(), true ),
                                            fcnAggregate,
                                            u"Aggregates"_s,
                                            QString(),
                                            []( const QgsExpressionNodeFunction * node )
    {
      // usesGeometry callback: return true if @parent variable is referenced

      if ( !node )
        return true;

      if ( !node->args() )
        return false;

      QSet<QString> referencedVars;
      if ( node->args()->count() > 2 )
      {
        QgsExpressionNode *subExpressionNode = node->args()->at( 2 );
        referencedVars = subExpressionNode->referencedVariables();
      }

      if ( node->args()->count() > 3 )
      {
        QgsExpressionNode *filterNode = node->args()->at( 3 );
        referencedVars.unite( filterNode->referencedVariables() );
      }
      return referencedVars.contains( u"parent"_s ) || referencedVars.contains( QString() );
    },
    []( const QgsExpressionNodeFunction * node )
    {
      // referencedColumns callback: return AllAttributes if @parent variable is referenced

      if ( !node )
        return QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES;

      if ( !node->args() )
        return QSet<QString>();

      QSet<QString> referencedCols;
      QSet<QString> referencedVars;

      if ( node->args()->count() > 2 )
      {
        QgsExpressionNode *subExpressionNode = node->args()->at( 2 );
        referencedVars = subExpressionNode->referencedVariables();
        referencedCols = subExpressionNode->referencedColumns();
      }
      if ( node->args()->count() > 3 )
      {
        QgsExpressionNode *filterNode = node->args()->at( 3 );
        referencedVars = filterNode->referencedVariables();
        referencedCols.unite( filterNode->referencedColumns() );
      }

      if ( referencedVars.contains( u"parent"_s ) || referencedVars.contains( QString() ) )
        return QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES;
      else
        return referencedCols;
    },
    true
                                          )

        << new QgsStaticExpressionFunction( u"relation_aggregate"_s, QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( u"relation"_s )
                                            << QgsExpressionFunction::Parameter( u"aggregate"_s )
                                            << QgsExpressionFunction::Parameter( u"expression"_s, false, QVariant(), true )
                                            << QgsExpressionFunction::Parameter( u"concatenator"_s, true )
                                            << QgsExpressionFunction::Parameter( u"order_by"_s, true, QVariant(), true ),
                                            fcnAggregateRelation, u"Aggregates"_s, QString(), false, QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES, true )

        << new QgsStaticExpressionFunction( u"count"_s, aggParams, fcnAggregateCount, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"count_distinct"_s, aggParams, fcnAggregateCountDistinct, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"count_missing"_s, aggParams, fcnAggregateCountMissing, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"minimum"_s, aggParams, fcnAggregateMin, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"maximum"_s, aggParams, fcnAggregateMax, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"sum"_s, aggParams, fcnAggregateSum, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"mean"_s, aggParams, fcnAggregateMean, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"median"_s, aggParams, fcnAggregateMedian, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"stdev"_s, aggParams, fcnAggregateStdev, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"range"_s, aggParams, fcnAggregateRange, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"minority"_s, aggParams, fcnAggregateMinority, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"majority"_s, aggParams, fcnAggregateMajority, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"q1"_s, aggParams, fcnAggregateQ1, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"q3"_s, aggParams, fcnAggregateQ3, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"iqr"_s, aggParams, fcnAggregateIQR, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"min_length"_s, aggParams, fcnAggregateMinLength, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"max_length"_s, aggParams, fcnAggregateMaxLength, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"collect"_s, aggParams, fcnAggregateCollectGeometry, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"concatenate"_s, aggParamsConcat, fcnAggregateStringConcat, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"concatenate_unique"_s, aggParamsConcat, fcnAggregateStringConcatUnique, u"Aggregates"_s, QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( u"array_agg"_s, aggParamsArray, fcnAggregateArray, u"Aggregates"_s, QString(), false, QSet<QString>(), true )

        << new QgsStaticExpressionFunction( u"regexp_match"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string"_s ) << QgsExpressionFunction::Parameter( u"regex"_s ), fcnRegexpMatch, QStringList() << u"Conditionals"_s << u"String"_s )
        << new QgsStaticExpressionFunction( u"regexp_matches"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string"_s ) << QgsExpressionFunction::Parameter( u"regex"_s ) << QgsExpressionFunction::Parameter( u"emptyvalue"_s, true, "" ), fcnRegexpMatches, u"Arrays"_s )

        << new QgsStaticExpressionFunction( u"now"_s, 0, fcnNow, u"Date and Time"_s, QString(), false, QSet<QString>(), false, QStringList() << u"$now"_s )
        << new QgsStaticExpressionFunction( u"age"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"datetime1"_s )
                                            << QgsExpressionFunction::Parameter( u"datetime2"_s ),
                                            fcnAge, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"year"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"date"_s ), fcnYear, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"month"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"date"_s ), fcnMonth, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"week"_s,  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"date"_s ), fcnWeek, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"day"_s,  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"date"_s ), fcnDay, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"hour"_s,  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"datetime"_s ), fcnHour, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"minute"_s,  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"datetime"_s ), fcnMinute, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"second"_s,  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"datetime"_s ), fcnSeconds, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"epoch"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"date"_s ), fcnEpoch, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"datetime_from_epoch"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"long"_s ), fcnDateTimeFromEpoch, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"day_of_week"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"date"_s ), fcnDayOfWeek, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"make_date"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"year"_s )
                                            << QgsExpressionFunction::Parameter( u"month"_s )
                                            << QgsExpressionFunction::Parameter( u"day"_s ),
                                            fcnMakeDate, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"make_time"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"hour"_s )
                                            << QgsExpressionFunction::Parameter( u"minute"_s )
                                            << QgsExpressionFunction::Parameter( u"second"_s ),
                                            fcnMakeTime, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"make_datetime"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"year"_s )
                                            << QgsExpressionFunction::Parameter( u"month"_s )
                                            << QgsExpressionFunction::Parameter( u"day"_s )
                                            << QgsExpressionFunction::Parameter( u"hour"_s )
                                            << QgsExpressionFunction::Parameter( u"minute"_s )
                                            << QgsExpressionFunction::Parameter( u"second"_s ),
                                            fcnMakeDateTime, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"make_interval"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"years"_s, true, 0 )
                                            << QgsExpressionFunction::Parameter( u"months"_s, true, 0 )
                                            << QgsExpressionFunction::Parameter( u"weeks"_s, true, 0 )
                                            << QgsExpressionFunction::Parameter( u"days"_s, true, 0 )
                                            << QgsExpressionFunction::Parameter( u"hours"_s, true, 0 )
                                            << QgsExpressionFunction::Parameter( u"minutes"_s, true, 0 )
                                            << QgsExpressionFunction::Parameter( u"seconds"_s, true, 0 ),
                                            fcnMakeInterval, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"timezone_from_id"_s, { QgsExpressionFunction::Parameter( u"id"_s ) }, fcnTimeZoneFromId, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"timezone_id"_s, { QgsExpressionFunction::Parameter( u"timezone"_s ) }, fcnTimeZoneToId, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"get_timezone"_s, { QgsExpressionFunction::Parameter( u"datetime"_s ) }, fcnGetTimeZone, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"set_timezone"_s, { QgsExpressionFunction::Parameter( u"datetime"_s ), QgsExpressionFunction::Parameter( u"timezone"_s ) }, fcnSetTimeZone, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"convert_timezone"_s, { QgsExpressionFunction::Parameter( u"datetime"_s ), QgsExpressionFunction::Parameter( u"timezone"_s ) }, fcnConvertTimeZone, u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"lower"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string"_s ), fcnLower, u"String"_s )
        << new QgsStaticExpressionFunction( u"substr_count"_s, QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( u"string"_s )
                                            << QgsExpressionFunction::Parameter( u"substring"_s )
                                            << QgsExpressionFunction::Parameter( u"overlapping"_s, true, false ),  // Optional parameter with default value of false
                                            fcnSubstrCount,
                                            u"String"_s )
        << new QgsStaticExpressionFunction( u"upper"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string"_s ), fcnUpper, u"String"_s )
        << new QgsStaticExpressionFunction( u"title"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string"_s ), fcnTitle, u"String"_s )
        << new QgsStaticExpressionFunction( u"trim"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string"_s ), fcnTrim, u"String"_s )
        << new QgsStaticExpressionFunction( u"unaccent"_s, { QgsExpressionFunction::Parameter( u"string"_s ) }, fcnUnaccent, u"String"_s )
        << new QgsStaticExpressionFunction( u"ltrim"_s, QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( u"string"_s )
                                            << QgsExpressionFunction::Parameter( u"characters"_s, true, u" "_s ), fcnLTrim, u"String"_s )
        << new QgsStaticExpressionFunction( u"rtrim"_s, QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( u"string"_s )
                                            << QgsExpressionFunction::Parameter( u"characters"_s, true, u" "_s ), fcnRTrim, u"String"_s )
        << new QgsStaticExpressionFunction( u"levenshtein"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string1"_s ) << QgsExpressionFunction::Parameter( u"string2"_s ), fcnLevenshtein, u"Fuzzy Matching"_s )
        << new QgsStaticExpressionFunction( u"longest_common_substring"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string1"_s ) << QgsExpressionFunction::Parameter( u"string2"_s ), fcnLCS, u"Fuzzy Matching"_s )
        << new QgsStaticExpressionFunction( u"hamming_distance"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string1"_s ) << QgsExpressionFunction::Parameter( u"string2"_s ), fcnHamming, u"Fuzzy Matching"_s )
        << new QgsStaticExpressionFunction( u"soundex"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string"_s ), fcnSoundex, u"Fuzzy Matching"_s )
        << new QgsStaticExpressionFunction( u"char"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"code"_s ), fcnChar, u"String"_s )
        << new QgsStaticExpressionFunction( u"ascii"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string"_s ), fcnAscii, u"String"_s )
        << new QgsStaticExpressionFunction( u"wordwrap"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"text"_s ) << QgsExpressionFunction::Parameter( u"length"_s ) << QgsExpressionFunction::Parameter( u"delimiter"_s, true, "" ), fcnWordwrap, u"String"_s )
        << new QgsStaticExpressionFunction( u"length"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"text"_s, true, "" ), fcnLength, QStringList() << u"String"_s << u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"length3D"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnLength3D, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"repeat"_s, { QgsExpressionFunction::Parameter( u"text"_s ), QgsExpressionFunction::Parameter( u"number"_s )}, fcnRepeat, u"String"_s )
        << new QgsStaticExpressionFunction( u"replace"_s, -1, fcnReplace, u"String"_s )
        << new QgsStaticExpressionFunction( u"regexp_replace"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"input_string"_s ) << QgsExpressionFunction::Parameter( u"regex"_s )
                                            << QgsExpressionFunction::Parameter( u"replacement"_s ), fcnRegexpReplace, u"String"_s )
        << new QgsStaticExpressionFunction( u"regexp_substr"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"input_string"_s ) << QgsExpressionFunction::Parameter( u"regex"_s ), fcnRegexpSubstr, u"String"_s )
        << new QgsStaticExpressionFunction( u"substr"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string"_s ) << QgsExpressionFunction::Parameter( u"start"_s ) << QgsExpressionFunction::Parameter( u"length"_s, true ), fcnSubstr, u"String"_s, QString(),
                                            false, QSet< QString >(), false, QStringList(), true )
        << new QgsStaticExpressionFunction( u"concat"_s, -1, fcnConcat, u"String"_s, QString(), false, QSet<QString>(), false, QStringList(), true )
        << new QgsStaticExpressionFunction( u"strpos"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"haystack"_s ) << QgsExpressionFunction::Parameter( u"needle"_s ), fcnStrpos, u"String"_s )
        << new QgsStaticExpressionFunction( u"left"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string"_s ) << QgsExpressionFunction::Parameter( u"length"_s ), fcnLeft, u"String"_s )
        << new QgsStaticExpressionFunction( u"right"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string"_s ) << QgsExpressionFunction::Parameter( u"length"_s ), fcnRight, u"String"_s )
        << new QgsStaticExpressionFunction( u"rpad"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string"_s ) << QgsExpressionFunction::Parameter( u"width"_s ) << QgsExpressionFunction::Parameter( u"fill"_s ), fcnRPad, u"String"_s )
        << new QgsStaticExpressionFunction( u"lpad"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string"_s ) << QgsExpressionFunction::Parameter( u"width"_s ) << QgsExpressionFunction::Parameter( u"fill"_s ), fcnLPad, u"String"_s )
        << new QgsStaticExpressionFunction( u"format"_s, -1, fcnFormatString, u"String"_s )
        << new QgsStaticExpressionFunction( u"format_number"_s, QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( u"number"_s )
                                            << QgsExpressionFunction::Parameter( u"places"_s, true, 0 )
                                            << QgsExpressionFunction::Parameter( u"language"_s, true, QVariant() )
                                            << QgsExpressionFunction::Parameter( u"omit_group_separators"_s, true, false )
                                            << QgsExpressionFunction::Parameter( u"trim_trailing_zeroes"_s, true, false ), fcnFormatNumber, u"String"_s )
        << new QgsStaticExpressionFunction( u"format_date"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"datetime"_s ) << QgsExpressionFunction::Parameter( u"format"_s ) << QgsExpressionFunction::Parameter( u"language"_s, true, QVariant() ), fcnFormatDate, QStringList() << u"String"_s << u"Date and Time"_s )
        << new QgsStaticExpressionFunction( u"color_grayscale_average"_s,  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"color"_s ), fcnColorGrayscaleAverage, u"Color"_s )
        << new QgsStaticExpressionFunction( u"color_mix_rgb"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"color1"_s )
                                            << QgsExpressionFunction::Parameter( u"color2"_s )
                                            << QgsExpressionFunction::Parameter( u"ratio"_s ),
                                            fcnColorMixRgb, u"Color"_s )
        << new QgsStaticExpressionFunction( u"color_mix"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"color1"_s )
                                            << QgsExpressionFunction::Parameter( u"color2"_s )
                                            << QgsExpressionFunction::Parameter( u"ratio"_s ),
                                            fcnColorMix, u"Color"_s )
        << new QgsStaticExpressionFunction( u"color_rgb"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"red"_s )
                                            << QgsExpressionFunction::Parameter( u"green"_s )
                                            << QgsExpressionFunction::Parameter( u"blue"_s ),
                                            fcnColorRgb, u"Color"_s )
        << new QgsStaticExpressionFunction( u"color_rgbf"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"red"_s )
                                            << QgsExpressionFunction::Parameter( u"green"_s )
                                            << QgsExpressionFunction::Parameter( u"blue"_s )
                                            << QgsExpressionFunction::Parameter( u"alpha"_s, true, 1. ),
                                            fcnColorRgbF, u"Color"_s )
        << new QgsStaticExpressionFunction( u"color_rgba"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"red"_s )
                                            << QgsExpressionFunction::Parameter( u"green"_s )
                                            << QgsExpressionFunction::Parameter( u"blue"_s )
                                            << QgsExpressionFunction::Parameter( u"alpha"_s ),
                                            fncColorRgba, u"Color"_s )
        << new QgsStaticExpressionFunction( u"ramp_color"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"ramp_name"_s )
                                            << QgsExpressionFunction::Parameter( u"value"_s ),
                                            fcnRampColor, u"Color"_s )
        << new QgsStaticExpressionFunction( u"ramp_color_object"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"ramp_name"_s )
                                            << QgsExpressionFunction::Parameter( u"value"_s ),
                                            fcnRampColorObject, u"Color"_s )
        << new QgsStaticExpressionFunction( u"create_ramp"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"map"_s )
                                            << QgsExpressionFunction::Parameter( u"discrete"_s, true, false ),
                                            fcnCreateRamp, u"Color"_s )
        << new QgsStaticExpressionFunction( u"color_hsl"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"hue"_s )
                                            << QgsExpressionFunction::Parameter( u"saturation"_s )
                                            << QgsExpressionFunction::Parameter( u"lightness"_s ),
                                            fcnColorHsl, u"Color"_s )
        << new QgsStaticExpressionFunction( u"color_hsla"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"hue"_s )
                                            << QgsExpressionFunction::Parameter( u"saturation"_s )
                                            << QgsExpressionFunction::Parameter( u"lightness"_s )
                                            << QgsExpressionFunction::Parameter( u"alpha"_s ),
                                            fncColorHsla, u"Color"_s )
        << new QgsStaticExpressionFunction( u"color_hslf"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"hue"_s )
                                            << QgsExpressionFunction::Parameter( u"saturation"_s )
                                            << QgsExpressionFunction::Parameter( u"lightness"_s )
                                            << QgsExpressionFunction::Parameter( u"alpha"_s, true, 1. ),
                                            fcnColorHslF, u"Color"_s )
        << new QgsStaticExpressionFunction( u"color_hsv"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"hue"_s )
                                            << QgsExpressionFunction::Parameter( u"saturation"_s )
                                            << QgsExpressionFunction::Parameter( u"value"_s ),
                                            fcnColorHsv, u"Color"_s )
        << new QgsStaticExpressionFunction( u"color_hsva"_s,  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"hue"_s )
                                            << QgsExpressionFunction::Parameter( u"saturation"_s )
                                            << QgsExpressionFunction::Parameter( u"value"_s )
                                            << QgsExpressionFunction::Parameter( u"alpha"_s ),
                                            fncColorHsva, u"Color"_s )
        << new QgsStaticExpressionFunction( u"color_hsvf"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"hue"_s )
                                            << QgsExpressionFunction::Parameter( u"saturation"_s )
                                            << QgsExpressionFunction::Parameter( u"value"_s )
                                            << QgsExpressionFunction::Parameter( u"alpha"_s, true, 1. ),
                                            fcnColorHsvF, u"Color"_s )
        << new QgsStaticExpressionFunction( u"color_cmyk"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"cyan"_s )
                                            << QgsExpressionFunction::Parameter( u"magenta"_s )
                                            << QgsExpressionFunction::Parameter( u"yellow"_s )
                                            << QgsExpressionFunction::Parameter( u"black"_s ),
                                            fcnColorCmyk, u"Color"_s )
        << new QgsStaticExpressionFunction( u"color_cmyka"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"cyan"_s )
                                            << QgsExpressionFunction::Parameter( u"magenta"_s )
                                            << QgsExpressionFunction::Parameter( u"yellow"_s )
                                            << QgsExpressionFunction::Parameter( u"black"_s )
                                            << QgsExpressionFunction::Parameter( u"alpha"_s ),
                                            fncColorCmyka, u"Color"_s )
        << new QgsStaticExpressionFunction( u"color_cmykf"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"cyan"_s )
                                            << QgsExpressionFunction::Parameter( u"magenta"_s )
                                            << QgsExpressionFunction::Parameter( u"yellow"_s )
                                            << QgsExpressionFunction::Parameter( u"black"_s )
                                            << QgsExpressionFunction::Parameter( u"alpha"_s, true, 1. ),
                                            fcnColorCmykF, u"Color"_s )
        << new QgsStaticExpressionFunction( u"color_part"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"color"_s )
                                            << QgsExpressionFunction::Parameter( u"component"_s ),
                                            fncColorPart, u"Color"_s )
        << new QgsStaticExpressionFunction( u"darker"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"color"_s )
                                            << QgsExpressionFunction::Parameter( u"factor"_s ),
                                            fncDarker, u"Color"_s )
        << new QgsStaticExpressionFunction( u"lighter"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"color"_s )
                                            << QgsExpressionFunction::Parameter( u"factor"_s ),
                                            fncLighter, u"Color"_s )
        << new QgsStaticExpressionFunction( u"set_color_part"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"color"_s ) << QgsExpressionFunction::Parameter( u"component"_s ) << QgsExpressionFunction::Parameter( u"value"_s ), fncSetColorPart, u"Color"_s )

        // file info
        << new QgsStaticExpressionFunction( u"base_file_name"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"path"_s ),
                                            fcnBaseFileName, u"Files and Paths"_s )
        << new QgsStaticExpressionFunction( u"file_suffix"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"path"_s ),
                                            fcnFileSuffix, u"Files and Paths"_s )
        << new QgsStaticExpressionFunction( u"file_exists"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"path"_s ),
                                            fcnFileExists, u"Files and Paths"_s )
        << new QgsStaticExpressionFunction( u"file_name"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"path"_s ),
                                            fcnFileName, u"Files and Paths"_s )
        << new QgsStaticExpressionFunction( u"is_file"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"path"_s ),
                                            fcnPathIsFile, u"Files and Paths"_s )
        << new QgsStaticExpressionFunction( u"is_directory"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"path"_s ),
                                            fcnPathIsDir, u"Files and Paths"_s )
        << new QgsStaticExpressionFunction( u"file_path"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"path"_s ),
                                            fcnFilePath, u"Files and Paths"_s )
        << new QgsStaticExpressionFunction( u"file_size"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"path"_s ),
                                            fcnFileSize, u"Files and Paths"_s )

        << new QgsStaticExpressionFunction( u"exif"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"path"_s ) << QgsExpressionFunction::Parameter( u"tag"_s, true ),
                                            fcnExif, u"Files and Paths"_s )
        << new QgsStaticExpressionFunction( u"exif_geotag"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"path"_s ),
                                            fcnExifGeoTag, u"GeometryGroup"_s )

        // hash
        << new QgsStaticExpressionFunction( u"hash"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string"_s ) << QgsExpressionFunction::Parameter( u"method"_s ),
                                            fcnGenericHash, u"Conversions"_s )
        << new QgsStaticExpressionFunction( u"md5"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string"_s ),
                                            fcnHashMd5, u"Conversions"_s )
        << new QgsStaticExpressionFunction( u"sha256"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string"_s ),
                                            fcnHashSha256, u"Conversions"_s )

        //base64
        << new QgsStaticExpressionFunction( u"to_base64"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ),
                                            fcnToBase64, u"Conversions"_s )
        << new QgsStaticExpressionFunction( u"from_base64"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string"_s ),
                                            fcnFromBase64, u"Conversions"_s )

        // deprecated stuff - hidden from users
        << new QgsStaticExpressionFunction( u"$scale"_s, QgsExpressionFunction::ParameterList(), fcnMapScale, u"deprecated"_s );

    QgsStaticExpressionFunction *geomFunc = new QgsStaticExpressionFunction( u"$geometry"_s, 0, fcnGeometry, u"GeometryGroup"_s, QString(), true );
    geomFunc->setIsStatic( false );
    functions << geomFunc;

    QgsStaticExpressionFunction *areaFunc = new QgsStaticExpressionFunction( u"$area"_s, 0, fcnGeomArea, u"GeometryGroup"_s, QString(), true );
    areaFunc->setIsStatic( false );
    functions << areaFunc;

    functions << new QgsStaticExpressionFunction( u"area"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnArea, u"GeometryGroup"_s );

    QgsStaticExpressionFunction *lengthFunc = new QgsStaticExpressionFunction( u"$length"_s, 0, fcnGeomLength, u"GeometryGroup"_s, QString(), true );
    lengthFunc->setIsStatic( false );
    functions << lengthFunc;

    QgsStaticExpressionFunction *perimeterFunc = new QgsStaticExpressionFunction( u"$perimeter"_s, 0, fcnGeomPerimeter, u"GeometryGroup"_s, QString(), true );
    perimeterFunc->setIsStatic( false );
    functions << perimeterFunc;

    functions << new QgsStaticExpressionFunction( u"perimeter"_s,  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnPerimeter, u"GeometryGroup"_s );

    functions << new QgsStaticExpressionFunction( u"roundness"_s,
              QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ),
              fcnRoundness, u"GeometryGroup"_s );

    QgsStaticExpressionFunction *xFunc = new QgsStaticExpressionFunction( u"$x"_s, 0, fcnX, u"GeometryGroup"_s, QString(), true );
    xFunc->setIsStatic( false );
    functions << xFunc;

    QgsStaticExpressionFunction *yFunc = new QgsStaticExpressionFunction( u"$y"_s, 0, fcnY, u"GeometryGroup"_s, QString(), true );
    yFunc->setIsStatic( false );
    functions << yFunc;

    QgsStaticExpressionFunction *zFunc = new QgsStaticExpressionFunction( u"$z"_s, 0, fcnZ, u"GeometryGroup"_s, QString(), true );
    zFunc->setIsStatic( false );
    functions << zFunc;

    QMap< QString, QgsExpressionFunction::FcnEval > geometry_overlay_definitions
    {
      { u"overlay_intersects"_s, fcnGeomOverlayIntersects },
      { u"overlay_contains"_s, fcnGeomOverlayContains },
      { u"overlay_crosses"_s, fcnGeomOverlayCrosses },
      { u"overlay_equals"_s, fcnGeomOverlayEquals },
      { u"overlay_touches"_s, fcnGeomOverlayTouches },
      { u"overlay_disjoint"_s, fcnGeomOverlayDisjoint },
      { u"overlay_within"_s, fcnGeomOverlayWithin },
    };
    QMapIterator< QString, QgsExpressionFunction::FcnEval > i( geometry_overlay_definitions );
    while ( i.hasNext() )
    {
      i.next();
      QgsStaticExpressionFunction *fcnGeomOverlayFunc = new QgsStaticExpressionFunction( i.key(), QgsExpressionFunction::ParameterList()
          << QgsExpressionFunction::Parameter( u"layer"_s )
          << QgsExpressionFunction::Parameter( u"expression"_s, true, QVariant(), true )
          << QgsExpressionFunction::Parameter( u"filter"_s, true, QVariant(), true )
          << QgsExpressionFunction::Parameter( u"limit"_s, true, QVariant( -1 ), true )
          << QgsExpressionFunction::Parameter( u"cache"_s, true, QVariant( false ), false )
          << QgsExpressionFunction::Parameter( u"min_overlap"_s, true, QVariant( -1 ), false )
          << QgsExpressionFunction::Parameter( u"min_inscribed_circle_radius"_s, true, QVariant( -1 ), false )
          << QgsExpressionFunction::Parameter( u"return_details"_s, true, false, false )
          << QgsExpressionFunction::Parameter( u"sort_by_intersection_size"_s, true, QString(), false ),
          i.value(), u"GeometryGroup"_s, QString(), true, QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES, true );

      // The current feature is accessed for the geometry, so this should not be cached
      fcnGeomOverlayFunc->setIsStatic( false );
      functions << fcnGeomOverlayFunc;
    }

    QgsStaticExpressionFunction *fcnGeomOverlayNearestFunc   = new QgsStaticExpressionFunction( u"overlay_nearest"_s, QgsExpressionFunction::ParameterList()
        << QgsExpressionFunction::Parameter( u"layer"_s )
        << QgsExpressionFunction::Parameter( u"expression"_s, true, QVariant(), true )
        << QgsExpressionFunction::Parameter( u"filter"_s, true, QVariant(), true )
        << QgsExpressionFunction::Parameter( u"limit"_s, true, QVariant( 1 ), true )
        << QgsExpressionFunction::Parameter( u"max_distance"_s, true, 0 )
        << QgsExpressionFunction::Parameter( u"cache"_s, true, QVariant( false ), false ),
        fcnGeomOverlayNearest, u"GeometryGroup"_s, QString(), true, QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES, true );
    // The current feature is accessed for the geometry, so this should not be cached
    fcnGeomOverlayNearestFunc->setIsStatic( false );
    functions << fcnGeomOverlayNearestFunc;

    functions
        << new QgsStaticExpressionFunction( u"is_valid"_s,  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnGeomIsValid, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"x"_s,  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnGeomX, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"y"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnGeomY, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"z"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnGeomZ, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"m"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnGeomM, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"point_n"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ) << QgsExpressionFunction::Parameter( u"index"_s ), fcnPointN, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"start_point"_s,  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnStartPoint, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"end_point"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnEndPoint, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"nodes_to_points"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"ignore_closing_nodes"_s, true, false ),
                                            fcnNodesToPoints, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"segments_to_lines"_s,  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnSegmentsToLines, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"collect_geometries"_s, -1, fcnCollectGeometries, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"make_point"_s, -1, fcnMakePoint, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"make_point_m"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"x"_s )
                                            << QgsExpressionFunction::Parameter( u"y"_s )
                                            << QgsExpressionFunction::Parameter( u"m"_s ),
                                            fcnMakePointM, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"make_line"_s, -1, fcnMakeLine, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"make_polygon"_s, -1, fcnMakePolygon, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"make_triangle"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"point1"_s )
                                            << QgsExpressionFunction::Parameter( u"point2"_s )
                                            << QgsExpressionFunction::Parameter( u"point3"_s ),
                                            fcnMakeTriangle, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"make_circle"_s, QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( u"center"_s )
                                            << QgsExpressionFunction::Parameter( u"radius"_s )
                                            << QgsExpressionFunction::Parameter( u"segments"_s, true, 36 ),
                                            fcnMakeCircle, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"make_ellipse"_s, QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( u"center"_s )
                                            << QgsExpressionFunction::Parameter( u"semi_major_axis"_s )
                                            << QgsExpressionFunction::Parameter( u"semi_minor_axis"_s )
                                            << QgsExpressionFunction::Parameter( u"azimuth"_s )
                                            << QgsExpressionFunction::Parameter( u"segments"_s, true, 36 ),
                                            fcnMakeEllipse, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"make_regular_polygon"_s, QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( u"center"_s )
                                            << QgsExpressionFunction::Parameter( u"radius"_s )
                                            << QgsExpressionFunction::Parameter( u"number_sides"_s )
                                            << QgsExpressionFunction::Parameter( u"circle"_s, true, 0 ),
                                            fcnMakeRegularPolygon, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"make_square"_s, QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( u"point1"_s )
                                            << QgsExpressionFunction::Parameter( u"point2"_s ),
                                            fcnMakeSquare, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"make_rectangle_3points"_s, QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( u"point1"_s )
                                            << QgsExpressionFunction::Parameter( u"point2"_s )
                                            << QgsExpressionFunction::Parameter( u"point3"_s )
                                            << QgsExpressionFunction::Parameter( u"option"_s, true, 0 ),
                                            fcnMakeRectangleFrom3Points, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"make_valid"_s,  QgsExpressionFunction::ParameterList
    {
      QgsExpressionFunction::Parameter( u"geometry"_s ),
#if GEOS_VERSION_MAJOR==3 && GEOS_VERSION_MINOR<10
      QgsExpressionFunction::Parameter( u"method"_s, true, u"linework"_s ),
#else
      QgsExpressionFunction::Parameter( u"method"_s, true, u"structure"_s ),
#endif
      QgsExpressionFunction::Parameter( u"keep_collapsed"_s, true, false )
    }, fcnGeomMakeValid, u"GeometryGroup"_s );

    functions << new QgsStaticExpressionFunction( u"x_at"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s, true ) << QgsExpressionFunction::Parameter( u"vertex"_s, true ), fcnXat, u"GeometryGroup"_s );
    functions << new QgsStaticExpressionFunction( u"y_at"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s, true ) << QgsExpressionFunction::Parameter( u"vertex"_s, true ), fcnYat, u"GeometryGroup"_s );
    functions << new QgsStaticExpressionFunction( u"z_at"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ) << QgsExpressionFunction::Parameter( u"vertex"_s, true ), fcnZat, u"GeometryGroup"_s );
    functions << new QgsStaticExpressionFunction( u"m_at"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ) << QgsExpressionFunction::Parameter( u"vertex"_s, true ), fcnMat, u"GeometryGroup"_s );

    QgsStaticExpressionFunction *xAtFunc = new QgsStaticExpressionFunction( u"$x_at"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"vertex"_s ), fcnOldXat, u"GeometryGroup"_s, QString(), true, QSet<QString>(), false, QStringList() << u"xat"_s );
    xAtFunc->setIsStatic( false );
    functions << xAtFunc;


    QgsStaticExpressionFunction *yAtFunc = new QgsStaticExpressionFunction( u"$y_at"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"vertex"_s ), fcnOldYat, u"GeometryGroup"_s, QString(), true, QSet<QString>(), false, QStringList() << u"yat"_s );
    yAtFunc->setIsStatic( false );
    functions << yAtFunc;

    functions
        << new QgsStaticExpressionFunction( u"geometry_type"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnGeometryType, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"x_min"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnXMin, u"GeometryGroup"_s, QString(), false, QSet<QString>(), false, QStringList() << u"xmin"_s )
        << new QgsStaticExpressionFunction( u"x_max"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnXMax, u"GeometryGroup"_s, QString(), false, QSet<QString>(), false, QStringList() << u"xmax"_s )
        << new QgsStaticExpressionFunction( u"y_min"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnYMin, u"GeometryGroup"_s, QString(), false, QSet<QString>(), false, QStringList() << u"ymin"_s )
        << new QgsStaticExpressionFunction( u"y_max"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnYMax, u"GeometryGroup"_s, QString(), false, QSet<QString>(), false, QStringList() << u"ymax"_s )
        << new QgsStaticExpressionFunction( u"geom_from_wkt"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"text"_s ), fcnGeomFromWKT, u"GeometryGroup"_s, QString(), false, QSet<QString>(), false, QStringList() << u"geomFromWKT"_s )
        << new QgsStaticExpressionFunction( u"geom_from_wkb"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"binary"_s ), fcnGeomFromWKB, u"GeometryGroup"_s, QString(), false, QSet<QString>(), false )
        << new QgsStaticExpressionFunction( u"geom_from_gml"_s,  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"gml"_s ), fcnGeomFromGML, u"GeometryGroup"_s, QString(), false, QSet<QString>(), false, QStringList() << u"geomFromGML"_s )
        << new QgsStaticExpressionFunction( u"flip_coordinates"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnFlipCoordinates, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"relate"_s, -1, fcnRelate, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"intersects_bbox"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry1"_s ) << QgsExpressionFunction::Parameter( u"geometry2"_s ), fcnBbox, u"GeometryGroup"_s, QString(), false, QSet<QString>(), false, QStringList() << u"bbox"_s )
        << new QgsStaticExpressionFunction( u"disjoint"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry1"_s )
                                            << QgsExpressionFunction::Parameter( u"geometry2"_s ),
                                            fcnDisjoint, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"intersects"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry1"_s )
                                            << QgsExpressionFunction::Parameter( u"geometry2"_s ),
                                            fcnIntersects, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"touches"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry1"_s )
                                            << QgsExpressionFunction::Parameter( u"geometry2"_s ),
                                            fcnTouches, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"crosses"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry1"_s )
                                            << QgsExpressionFunction::Parameter( u"geometry2"_s ),
                                            fcnCrosses, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"contains"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry1"_s )
                                            << QgsExpressionFunction::Parameter( u"geometry2"_s ),
                                            fcnContains, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"overlaps"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry1"_s )
                                            << QgsExpressionFunction::Parameter( u"geometry2"_s ),
                                            fcnOverlaps, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"within"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry1"_s )
                                            << QgsExpressionFunction::Parameter( u"geometry2"_s ),
                                            fcnWithin, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"translate"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"dx"_s )
                                            << QgsExpressionFunction::Parameter( u"dy"_s ),
                                            fcnTranslate, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"rotate"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"rotation"_s )
                                            << QgsExpressionFunction::Parameter( u"center"_s, true )
                                            << QgsExpressionFunction::Parameter( u"per_part"_s, true, false ),
                                            fcnRotate, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"scale"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"x_scale"_s )
                                            << QgsExpressionFunction::Parameter( u"y_scale"_s )
                                            << QgsExpressionFunction::Parameter( u"center"_s, true ),
                                            fcnScale, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"affine_transform"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"delta_x"_s )
                                            << QgsExpressionFunction::Parameter( u"delta_y"_s )
                                            << QgsExpressionFunction::Parameter( u"rotation_z"_s )
                                            << QgsExpressionFunction::Parameter( u"scale_x"_s )
                                            << QgsExpressionFunction::Parameter( u"scale_y"_s )
                                            << QgsExpressionFunction::Parameter( u"delta_z"_s, true, 0 )
                                            << QgsExpressionFunction::Parameter( u"delta_m"_s, true, 0 )
                                            << QgsExpressionFunction::Parameter( u"scale_z"_s, true, 1 )
                                            << QgsExpressionFunction::Parameter( u"scale_m"_s, true, 1 ),
                                            fcnAffineTransform, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"buffer"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"distance"_s )
                                            << QgsExpressionFunction::Parameter( u"segments"_s, true, 8 )
                                            << QgsExpressionFunction::Parameter( u"cap"_s, true, u"round"_s )
                                            << QgsExpressionFunction::Parameter( u"join"_s, true, u"round"_s )
                                            << QgsExpressionFunction::Parameter( u"miter_limit"_s, true, 2 ),
                                            fcnBuffer, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"force_rhr"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ),
                                            fcnForceRHR, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"force_polygon_cw"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ),
                                            fcnForcePolygonCW, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"force_polygon_ccw"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ),
                                            fcnForcePolygonCCW, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"wedge_buffer"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"center"_s )
                                            << QgsExpressionFunction::Parameter( u"azimuth"_s )
                                            << QgsExpressionFunction::Parameter( u"width"_s )
                                            << QgsExpressionFunction::Parameter( u"outer_radius"_s )
                                            << QgsExpressionFunction::Parameter( u"inner_radius"_s, true, 0.0 ), fcnWedgeBuffer, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"tapered_buffer"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"start_width"_s )
                                            << QgsExpressionFunction::Parameter( u"end_width"_s )
                                            << QgsExpressionFunction::Parameter( u"segments"_s, true, 8.0 )
                                            , fcnTaperedBuffer, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"buffer_by_m"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"segments"_s, true, 8.0 )
                                            , fcnBufferByM, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"offset_curve"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"distance"_s )
                                            << QgsExpressionFunction::Parameter( u"segments"_s, true, 8.0 )
                                            << QgsExpressionFunction::Parameter( u"join"_s, true, static_cast< int >( Qgis::JoinStyle::Round ) )
                                            << QgsExpressionFunction::Parameter( u"miter_limit"_s, true, 2.0 ),
                                            fcnOffsetCurve, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"single_sided_buffer"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"distance"_s )
                                            << QgsExpressionFunction::Parameter( u"segments"_s, true, 8.0 )
                                            << QgsExpressionFunction::Parameter( u"join"_s, true, static_cast< int >( Qgis::JoinStyle::Round ) )
                                            << QgsExpressionFunction::Parameter( u"miter_limit"_s, true, 2.0 ),
                                            fcnSingleSidedBuffer, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"extend"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"start_distance"_s )
                                            << QgsExpressionFunction::Parameter( u"end_distance"_s ),
                                            fcnExtend, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"centroid"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnCentroid, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"point_on_surface"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnPointOnSurface, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"pole_of_inaccessibility"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"tolerance"_s ), fcnPoleOfInaccessibility, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"reverse"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnReverse, { u"String"_s, u"GeometryGroup"_s } )
        << new QgsStaticExpressionFunction( u"exterior_ring"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnExteriorRing, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"interior_ring_n"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"index"_s ),
                                            fcnInteriorRingN, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"geometry_n"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"index"_s ),
                                            fcnGeometryN, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"boundary"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnBoundary, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"line_merge"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnLineMerge, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"shared_paths"_s, QgsExpressionFunction::ParameterList
    {
      QgsExpressionFunction::Parameter( u"geometry1"_s ),
      QgsExpressionFunction::Parameter( u"geometry2"_s )
    }, fcnSharedPaths, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"bounds"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnBounds, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"simplify"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ) << QgsExpressionFunction::Parameter( u"tolerance"_s ), fcnSimplify, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"simplify_vw"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ) << QgsExpressionFunction::Parameter( u"tolerance"_s ), fcnSimplifyVW, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"smooth"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ) << QgsExpressionFunction::Parameter( u"iterations"_s, true, 1 )
                                            << QgsExpressionFunction::Parameter( u"offset"_s, true, 0.25 )
                                            << QgsExpressionFunction::Parameter( u"min_length"_s, true, -1 )
                                            << QgsExpressionFunction::Parameter( u"max_angle"_s, true, 180 ), fcnSmooth, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"triangular_wave"_s,
    {
      QgsExpressionFunction::Parameter( u"geometry"_s ),
      QgsExpressionFunction::Parameter( u"wavelength"_s ),
      QgsExpressionFunction::Parameter( u"amplitude"_s ),
      QgsExpressionFunction::Parameter( u"strict"_s, true, false )
    }, fcnTriangularWave, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"triangular_wave_randomized"_s,
    {
      QgsExpressionFunction::Parameter( u"geometry"_s ),
      QgsExpressionFunction::Parameter( u"min_wavelength"_s ),
      QgsExpressionFunction::Parameter( u"max_wavelength"_s ),
      QgsExpressionFunction::Parameter( u"min_amplitude"_s ),
      QgsExpressionFunction::Parameter( u"max_amplitude"_s ),
      QgsExpressionFunction::Parameter( u"seed"_s, true, 0 )
    }, fcnTriangularWaveRandomized, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"square_wave"_s,
    {
      QgsExpressionFunction::Parameter( u"geometry"_s ),
      QgsExpressionFunction::Parameter( u"wavelength"_s ),
      QgsExpressionFunction::Parameter( u"amplitude"_s ),
      QgsExpressionFunction::Parameter( u"strict"_s, true, false )
    }, fcnSquareWave, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"square_wave_randomized"_s,
    {
      QgsExpressionFunction::Parameter( u"geometry"_s ),
      QgsExpressionFunction::Parameter( u"min_wavelength"_s ),
      QgsExpressionFunction::Parameter( u"max_wavelength"_s ),
      QgsExpressionFunction::Parameter( u"min_amplitude"_s ),
      QgsExpressionFunction::Parameter( u"max_amplitude"_s ),
      QgsExpressionFunction::Parameter( u"seed"_s, true, 0 )
    }, fcnSquareWaveRandomized, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"wave"_s,
    {
      QgsExpressionFunction::Parameter( u"geometry"_s ),
      QgsExpressionFunction::Parameter( u"wavelength"_s ),
      QgsExpressionFunction::Parameter( u"amplitude"_s ),
      QgsExpressionFunction::Parameter( u"strict"_s, true, false )
    }, fcnRoundWave, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"wave_randomized"_s,
    {
      QgsExpressionFunction::Parameter( u"geometry"_s ),
      QgsExpressionFunction::Parameter( u"min_wavelength"_s ),
      QgsExpressionFunction::Parameter( u"max_wavelength"_s ),
      QgsExpressionFunction::Parameter( u"min_amplitude"_s ),
      QgsExpressionFunction::Parameter( u"max_amplitude"_s ),
      QgsExpressionFunction::Parameter( u"seed"_s, true, 0 )
    }, fcnRoundWaveRandomized, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"apply_dash_pattern"_s,
    {
      QgsExpressionFunction::Parameter( u"geometry"_s ),
      QgsExpressionFunction::Parameter( u"pattern"_s ),
      QgsExpressionFunction::Parameter( u"start_rule"_s, true, u"no_rule"_s ),
      QgsExpressionFunction::Parameter( u"end_rule"_s, true, u"no_rule"_s ),
      QgsExpressionFunction::Parameter( u"adjustment"_s, true, u"both"_s ),
      QgsExpressionFunction::Parameter( u"pattern_offset"_s, true, 0 ),
    }, fcnApplyDashPattern, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"densify_by_count"_s,
    {
      QgsExpressionFunction::Parameter( u"geometry"_s ),
      QgsExpressionFunction::Parameter( u"vertices"_s )
    }, fcnDensifyByCount, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"densify_by_distance"_s,
    {
      QgsExpressionFunction::Parameter( u"geometry"_s ),
      QgsExpressionFunction::Parameter( u"distance"_s )
    }, fcnDensifyByDistance, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"num_points"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnGeomNumPoints, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"num_interior_rings"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnGeomNumInteriorRings, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"num_rings"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnGeomNumRings, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"num_geometries"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnGeomNumGeometries, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"bounds_width"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnBoundsWidth, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"bounds_height"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnBoundsHeight, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"is_closed"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnIsClosed, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"close_line"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnCloseLine, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"is_empty"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnIsEmpty, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"is_empty_or_null"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnIsEmptyOrNull, u"GeometryGroup"_s, QString(), false, QSet<QString>(), false, QStringList(), true )
        << new QgsStaticExpressionFunction( u"convex_hull"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ), fcnConvexHull, u"GeometryGroup"_s, QString(), false, QSet<QString>(), false, QStringList() << u"convexHull"_s )
#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=11 )
        << new QgsStaticExpressionFunction( u"concave_hull"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"target_percent"_s )
                                            << QgsExpressionFunction::Parameter( u"allow_holes"_s, true, false ), fcnConcaveHull, u"GeometryGroup"_s )
#endif
        << new QgsStaticExpressionFunction( u"oriented_bbox"_s, QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( u"geometry"_s ),
                                            fcnOrientedBBox, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"main_angle"_s, QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( u"geometry"_s ),
                                            fcnMainAngle, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"minimal_circle"_s, QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"segments"_s, true, 36 ),
                                            fcnMinimalCircle, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"difference"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry1"_s )
                                            << QgsExpressionFunction::Parameter( u"geometry2"_s ),
                                            fcnDifference, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"distance"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry1"_s )
                                            << QgsExpressionFunction::Parameter( u"geometry2"_s ),
                                            fcnDistance, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"hausdorff_distance"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry1"_s ) << QgsExpressionFunction::Parameter( u"geometry2"_s )
                                            << QgsExpressionFunction::Parameter( u"densify_fraction"_s, true ),
                                            fcnHausdorffDistance, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"intersection"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry1"_s )
                                            << QgsExpressionFunction::Parameter( u"geometry2"_s ),
                                            fcnIntersection, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"sym_difference"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry1"_s )
                                            << QgsExpressionFunction::Parameter( u"geometry2"_s ),
                                            fcnSymDifference, u"GeometryGroup"_s, QString(), false, QSet<QString>(), false, QStringList() << u"symDifference"_s )
        << new QgsStaticExpressionFunction( u"combine"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry1"_s )
                                            << QgsExpressionFunction::Parameter( u"geometry2"_s ),
                                            fcnCombine, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"union"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry1"_s )
                                            << QgsExpressionFunction::Parameter( u"geometry2"_s ),
                                            fcnCombine, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"geom_to_wkt"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"precision"_s, true, 8.0 ),
                                            fcnGeomToWKT, u"GeometryGroup"_s, QString(), false, QSet<QString>(), false, QStringList() << u"geomToWKT"_s )
        << new QgsStaticExpressionFunction( u"geom_to_wkb"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ),
                                            fcnGeomToWKB, u"GeometryGroup"_s, QString(), false, QSet<QString>(), false )
        << new QgsStaticExpressionFunction( u"geometry"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"feature"_s ), fcnGetGeometry, u"GeometryGroup"_s, QString(), true )
        << new QgsStaticExpressionFunction( u"transform"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"source_auth_id"_s )
                                            << QgsExpressionFunction::Parameter( u"dest_auth_id"_s ),
                                            fcnTransformGeometry, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"extrude"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"x"_s )
                                            << QgsExpressionFunction::Parameter( u"y"_s ),
                                            fcnExtrude, u"GeometryGroup"_s, QString() )
        << new QgsStaticExpressionFunction( u"is_multipart"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ),
                                            fcnGeomIsMultipart, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"z_max"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ),
                                            fcnZMax, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"z_min"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ),
                                            fcnZMin, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"m_max"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ),
                                            fcnMMax, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"m_min"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ),
                                            fcnMMin, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"sinuosity"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ),
                                            fcnSinuosity, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"straight_distance_2d"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s ),
                                            fcnStraightDistance2d, u"GeometryGroup"_s );


    QgsStaticExpressionFunction *orderPartsFunc = new QgsStaticExpressionFunction( u"order_parts"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
        << QgsExpressionFunction::Parameter( u"orderby"_s )
        << QgsExpressionFunction::Parameter( u"ascending"_s, true, true ),
        fcnOrderParts, u"GeometryGroup"_s, QString() );

    orderPartsFunc->setIsStaticFunction(
      []( const QgsExpressionNodeFunction * node, QgsExpression * parent, const QgsExpressionContext * context )
    {
      const QList< QgsExpressionNode *> argList = node->args()->list();
      for ( QgsExpressionNode *argNode : argList )
      {
        if ( !argNode->isStatic( parent, context ) )
          return false;
      }

      if ( node->args()->count() > 1 )
      {
        QgsExpressionNode *argNode = node->args()->at( 1 );

        QString expString = argNode->eval( parent, context ).toString();

        QgsExpression e( expString );

        if ( e.rootNode() && e.rootNode()->isStatic( parent, context ) )
          return true;
      }

      return true;
    } );

    orderPartsFunc->setPrepareFunction( []( const QgsExpressionNodeFunction * node, QgsExpression * parent, const QgsExpressionContext * context )
    {
      if ( node->args()->count() > 1 )
      {
        QgsExpressionNode *argNode = node->args()->at( 1 );
        QString expression = argNode->eval( parent, context ).toString();
        QgsExpression e( expression );
        e.prepare( context );
        context->setCachedValue( expression, QVariant::fromValue( e ) );
      }
      return true;
    }
                                      );
    functions << orderPartsFunc;

    functions
        << new QgsStaticExpressionFunction( u"closest_point"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry1"_s )
                                            << QgsExpressionFunction::Parameter( u"geometry2"_s ),
                                            fcnClosestPoint, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"shortest_line"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry1"_s )
                                            << QgsExpressionFunction::Parameter( u"geometry2"_s ),
                                            fcnShortestLine, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"line_interpolate_point"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"distance"_s ), fcnLineInterpolatePoint, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"line_interpolate_point_by_m"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"m"_s ) << QgsExpressionFunction::Parameter( u"use_3d_distance"_s, true, false ),
                                            fcnLineInterpolatePointByM, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"line_interpolate_angle"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"distance"_s ), fcnLineInterpolateAngle, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"line_locate_point"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"point"_s ), fcnLineLocatePoint, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"line_locate_m"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"m"_s )  << QgsExpressionFunction::Parameter( u"use_3d_distance"_s, true, false ),
                                            fcnLineLocateM, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"angle_at_vertex"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"vertex"_s ), fcnAngleAtVertex, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"distance_to_vertex"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"vertex"_s ), fcnDistanceToVertex, u"GeometryGroup"_s )
        << new QgsStaticExpressionFunction( u"line_substring"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometry"_s )
                                            << QgsExpressionFunction::Parameter( u"start_distance"_s ) << QgsExpressionFunction::Parameter( u"end_distance"_s ), fcnLineSubset, u"GeometryGroup"_s );


    // **Record** functions

    QgsStaticExpressionFunction *idFunc = new QgsStaticExpressionFunction( u"$id"_s, 0, fcnFeatureId, u"Record and Attributes"_s );
    idFunc->setIsStatic( false );
    functions << idFunc;

    QgsStaticExpressionFunction *currentFeatureFunc = new QgsStaticExpressionFunction( u"$currentfeature"_s, 0, fcnFeature, u"Record and Attributes"_s );
    currentFeatureFunc->setIsStatic( false );
    functions << currentFeatureFunc;

    QgsStaticExpressionFunction *uuidFunc = new QgsStaticExpressionFunction( u"uuid"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"format"_s, true, u"WithBraces"_s ), fcnUuid, u"Record and Attributes"_s, QString(), false, QSet<QString>(), false, QStringList() << u"$uuid"_s );
    uuidFunc->setIsStatic( false );
    functions << uuidFunc;

    functions
        << new QgsStaticExpressionFunction( u"feature_id"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"feature"_s ), fcnGetFeatureId, u"Record and Attributes"_s, QString(), true )
        << new QgsStaticExpressionFunction( u"get_feature"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"layer"_s )
                                            << QgsExpressionFunction::Parameter( u"attribute"_s )
                                            << QgsExpressionFunction::Parameter( u"value"_s, true ),
                                            fcnGetFeature, u"Record and Attributes"_s, QString(), false, QSet<QString>(), false, QStringList() << u"QgsExpressionUtils::getFeature"_s )
        << new QgsStaticExpressionFunction( u"get_feature_by_id"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"layer"_s )
                                            << QgsExpressionFunction::Parameter( u"feature_id"_s ),
                                            fcnGetFeatureById, u"Record and Attributes"_s, QString(), false, QSet<QString>(), false );

    QgsStaticExpressionFunction *attributesFunc = new QgsStaticExpressionFunction( u"attributes"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"feature"_s, true ),
        fcnAttributes, u"Record and Attributes"_s, QString(), false, QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES );
    attributesFunc->setIsStatic( false );
    functions << attributesFunc;
    QgsStaticExpressionFunction *representAttributesFunc = new QgsStaticExpressionFunction( u"represent_attributes"_s, -1,
        fcnRepresentAttributes, u"Record and Attributes"_s, QString(), false, QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES );
    representAttributesFunc->setIsStatic( false );
    functions << representAttributesFunc;

    QgsStaticExpressionFunction *validateFeature = new QgsStaticExpressionFunction( u"is_feature_valid"_s,
        QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"layer"_s, true )
        << QgsExpressionFunction::Parameter( u"feature"_s, true )
        << QgsExpressionFunction::Parameter( u"strength"_s, true ),
        fcnValidateFeature, u"Record and Attributes"_s, QString(), false, QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES );
    validateFeature->setIsStatic( false );
    functions << validateFeature;

    QgsStaticExpressionFunction *validateAttribute = new QgsStaticExpressionFunction( u"is_attribute_valid"_s,
        QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"attribute"_s, false )
        << QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"layer"_s, true )
        << QgsExpressionFunction::Parameter( u"feature"_s, true )
        << QgsExpressionFunction::Parameter( u"strength"_s, true ),
        fcnValidateAttribute, u"Record and Attributes"_s, QString(), false, QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES );
    validateAttribute->setIsStatic( false );
    functions << validateAttribute;

    QgsStaticExpressionFunction *maptipFunc = new QgsStaticExpressionFunction(
      u"maptip"_s,
      -1,
      fcnFeatureMaptip,
      u"Record and Attributes"_s,
      QString(),
      false,
      QSet<QString>()
    );
    maptipFunc->setIsStatic( false );
    functions << maptipFunc;

    QgsStaticExpressionFunction *displayFunc = new QgsStaticExpressionFunction(
      u"display_expression"_s,
      -1,
      fcnFeatureDisplayExpression,
      u"Record and Attributes"_s,
      QString(),
      false,
      QSet<QString>()
    );
    displayFunc->setIsStatic( false );
    functions << displayFunc;

    QgsStaticExpressionFunction *isSelectedFunc = new QgsStaticExpressionFunction(
      u"is_selected"_s,
      -1,
      fcnIsSelected,
      u"Record and Attributes"_s,
      QString(),
      false,
      QSet<QString>()
    );
    isSelectedFunc->setIsStatic( false );
    functions << isSelectedFunc;

    functions
        << new QgsStaticExpressionFunction(
          u"num_selected"_s,
          -1,
          fcnNumSelected,
          u"Record and Attributes"_s,
          QString(),
          false,
          QSet<QString>()
        );

    functions
        << new QgsStaticExpressionFunction(
          u"sqlite_fetch_and_increment"_s,
          QgsExpressionFunction::ParameterList()
          << QgsExpressionFunction::Parameter( u"database"_s )
          << QgsExpressionFunction::Parameter( u"table"_s )
          << QgsExpressionFunction::Parameter( u"id_field"_s )
          << QgsExpressionFunction::Parameter( u"filter_attribute"_s )
          << QgsExpressionFunction::Parameter( u"filter_value"_s )
          << QgsExpressionFunction::Parameter( u"default_values"_s, true ),
          fcnSqliteFetchAndIncrement,
          u"Record and Attributes"_s
        );

    // **CRS** functions
    functions
        << new QgsStaticExpressionFunction( u"crs_to_authid"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"crs"_s ), fcnCrsToAuthid, u"CRS"_s, QString(), true )
        << new QgsStaticExpressionFunction( u"crs_from_text"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"definition"_s ), fcnCrsFromText, u"CRS"_s );


    // **Fields and Values** functions
    QgsStaticExpressionFunction *representValueFunc = new QgsStaticExpressionFunction( u"represent_value"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"attribute"_s ) << QgsExpressionFunction::Parameter( u"field_name"_s, true ), fcnRepresentValue, u"Record and Attributes"_s );

    representValueFunc->setPrepareFunction( []( const QgsExpressionNodeFunction * node, QgsExpression * parent, const QgsExpressionContext * context )
    {
      Q_UNUSED( context )
      if ( node->args()->count() == 1 )
      {
        QgsExpressionNodeColumnRef *colRef = dynamic_cast<QgsExpressionNodeColumnRef *>( node->args()->at( 0 ) );
        if ( colRef )
        {
          return true;
        }
        else
        {
          parent->setEvalErrorString( tr( "If represent_value is called with 1 parameter, it must be an attribute." ) );
          return false;
        }
      }
      else if ( node->args()->count() == 2 )
      {
        return true;
      }
      else
      {
        parent->setEvalErrorString( tr( "represent_value must be called with exactly 1 or 2 parameters." ) );
        return false;
      }
    }
                                          );

    functions << representValueFunc;

    // **General** functions
    functions
        << new QgsStaticExpressionFunction( u"layer_property"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"layer"_s )
                                            << QgsExpressionFunction::Parameter( u"property"_s ),
                                            fcnGetLayerProperty, u"Map Layers"_s )
        << new QgsStaticExpressionFunction( u"decode_uri"_s,
                                            QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( u"layer"_s )
                                            << QgsExpressionFunction::Parameter( u"part"_s, true ),
                                            fcnDecodeUri, u"Map Layers"_s )
        << new QgsStaticExpressionFunction( u"mime_type"_s,
                                            QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( u"binary_data"_s ),
                                            fcnMimeType, u"General"_s )
        << new QgsStaticExpressionFunction( u"raster_statistic"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"layer"_s )
                                            << QgsExpressionFunction::Parameter( u"band"_s )
                                            << QgsExpressionFunction::Parameter( u"statistic"_s ), fcnGetRasterBandStat, u"Rasters"_s );

    // **var** function
    QgsStaticExpressionFunction *varFunction = new QgsStaticExpressionFunction( u"var"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"name"_s ), fcnGetVariable, u"General"_s );
    varFunction->setIsStaticFunction(
      []( const QgsExpressionNodeFunction * node, QgsExpression * parent, const QgsExpressionContext * context )
    {
      /* A variable node is static if it has a static name and the name can be found at prepare
       * time and is tagged with isStatic.
       * It is not static if a variable is set during iteration or not tagged isStatic.
       * (e.g. geom_part variable)
       */
      if ( node->args()->count() > 0 )
      {
        QgsExpressionNode *argNode = node->args()->at( 0 );

        if ( !argNode->isStatic( parent, context ) )
          return false;

        const QString varName = argNode->eval( parent, context ).toString();
        if ( varName == "feature"_L1 || varName == "id"_L1 || varName == "geometry"_L1 )
          return false;

        const QgsExpressionContextScope *scope = context->activeScopeForVariable( varName );
        return scope ? scope->isStatic( varName ) : false;
      }
      return false;
    }
    );
    varFunction->setUsesGeometryFunction(
      []( const QgsExpressionNodeFunction * node ) -> bool
    {
      if ( node && node->args()->count() > 0 )
      {
        QgsExpressionNode *argNode = node->args()->at( 0 );
        if ( QgsExpressionNodeLiteral *literal = dynamic_cast<QgsExpressionNodeLiteral *>( argNode ) )
        {
          if ( literal->value() == "geometry"_L1 || literal->value() == "feature"_L1 )
            return true;
        }
      }
      return false;
    }
    );

    functions
        << varFunction;

    QgsStaticExpressionFunction *evalTemplateFunction = new QgsStaticExpressionFunction( u"eval_template"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"template"_s ), fcnEvalTemplate, u"General"_s, QString(), true, QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES );
    evalTemplateFunction->setIsStaticFunction(
      []( const QgsExpressionNodeFunction * node, QgsExpression * parent, const QgsExpressionContext * context )
    {
      if ( node->args()->count() > 0 )
      {
        QgsExpressionNode *argNode = node->args()->at( 0 );

        if ( argNode->isStatic( parent, context ) )
        {
          QString expString = argNode->eval( parent, context ).toString();

          QgsExpression e( expString );

          if ( e.rootNode() && e.rootNode()->isStatic( parent, context ) )
            return true;
        }
      }

      return false;
    } );
    functions << evalTemplateFunction;

    QgsStaticExpressionFunction *evalFunc = new QgsStaticExpressionFunction( u"eval"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"expression"_s ), fcnEval, u"General"_s, QString(), true, QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES );
    evalFunc->setIsStaticFunction(
      []( const QgsExpressionNodeFunction * node, QgsExpression * parent, const QgsExpressionContext * context )
    {
      if ( node->args()->count() > 0 )
      {
        QgsExpressionNode *argNode = node->args()->at( 0 );

        if ( argNode->isStatic( parent, context ) )
        {
          QString expString = argNode->eval( parent, context ).toString();

          QgsExpression e( expString );

          if ( e.rootNode() && e.rootNode()->isStatic( parent, context ) )
            return true;
        }
      }

      return false;
    } );

    functions << evalFunc;

    QgsStaticExpressionFunction *attributeFunc = new QgsStaticExpressionFunction( u"attribute"_s, -1, fcnAttribute, u"Record and Attributes"_s, QString(), false, QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES );
    attributeFunc->setIsStaticFunction(
      []( const QgsExpressionNodeFunction * node, QgsExpression * parent, const QgsExpressionContext * context )
    {
      const QList< QgsExpressionNode *> argList = node->args()->list();
      for ( QgsExpressionNode *argNode : argList )
      {
        if ( !argNode->isStatic( parent, context ) )
          return false;
      }

      if ( node->args()->count() == 1 )
      {
        // not static -- this is the variant which uses the current feature taken direct from the expression context
        return false;
      }

      return true;
    } );
    functions << attributeFunc;

    functions
        << new QgsStaticExpressionFunction( u"env"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"name"_s ), fcnEnvVar, u"General"_s, QString() )
        << new QgsWithVariableExpressionFunction()
        << new QgsStaticExpressionFunction( u"raster_value"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"layer"_s ) << QgsExpressionFunction::Parameter( u"band"_s ) << QgsExpressionFunction::Parameter( u"point"_s ), fcnRasterValue, u"Rasters"_s )
        << new QgsStaticExpressionFunction( u"raster_attributes"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"layer"_s ) << QgsExpressionFunction::Parameter( u"band"_s ) << QgsExpressionFunction::Parameter( u"point"_s ), fcnRasterAttributes, u"Rasters"_s )

        // functions for arrays
        << new QgsArrayForeachExpressionFunction()
        << new QgsArrayFilterExpressionFunction()
        << new QgsStaticExpressionFunction( u"array"_s, -1, fcnArray, u"Arrays"_s, QString(), false, QSet<QString>(), false, QStringList(), true )
        << new QgsStaticExpressionFunction( u"array_sort"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ) << QgsExpressionFunction::Parameter( u"ascending"_s, true, true ), fcnArraySort, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_length"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ), fcnArrayLength, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_contains"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ) << QgsExpressionFunction::Parameter( u"value"_s ), fcnArrayContains, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_count"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ) << QgsExpressionFunction::Parameter( u"value"_s ), fcnArrayCount, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_all"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array_a"_s ) << QgsExpressionFunction::Parameter( u"array_b"_s ), fcnArrayAll, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_find"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ) << QgsExpressionFunction::Parameter( u"value"_s ), fcnArrayFind, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_get"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ) << QgsExpressionFunction::Parameter( u"pos"_s ), fcnArrayGet, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_first"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ), fcnArrayFirst, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_last"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ), fcnArrayLast, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_min"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ), fcnArrayMinimum, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_max"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ), fcnArrayMaximum, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_mean"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ), fcnArrayMean, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_median"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ), fcnArrayMedian, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_majority"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ) << QgsExpressionFunction::Parameter( u"option"_s, true, QVariant( "all" ) ), fcnArrayMajority, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_minority"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ) << QgsExpressionFunction::Parameter( u"option"_s, true, QVariant( "all" ) ), fcnArrayMinority, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_sum"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ), fcnArraySum, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_append"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ) << QgsExpressionFunction::Parameter( u"value"_s ), fcnArrayAppend, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_prepend"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ) << QgsExpressionFunction::Parameter( u"value"_s ), fcnArrayPrepend, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_insert"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ) << QgsExpressionFunction::Parameter( u"pos"_s ) << QgsExpressionFunction::Parameter( u"value"_s ), fcnArrayInsert, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_remove_at"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ) << QgsExpressionFunction::Parameter( u"pos"_s ), fcnArrayRemoveAt, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_remove_all"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ) << QgsExpressionFunction::Parameter( u"value"_s ), fcnArrayRemoveAll, u"Arrays"_s, QString(), false, QSet<QString>(), false, QStringList(), true )
        << new QgsStaticExpressionFunction( u"array_replace"_s, -1, fcnArrayReplace, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_prioritize"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ) << QgsExpressionFunction::Parameter( u"array_prioritize"_s ), fcnArrayPrioritize, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_cat"_s, -1, fcnArrayCat, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_slice"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ) << QgsExpressionFunction::Parameter( u"start_pos"_s ) << QgsExpressionFunction::Parameter( u"end_pos"_s ), fcnArraySlice, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_reverse"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ), fcnArrayReverse, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_intersect"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array1"_s ) << QgsExpressionFunction::Parameter( u"array2"_s ), fcnArrayIntersect, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_distinct"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ), fcnArrayDistinct, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"array_to_string"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"array"_s ) << QgsExpressionFunction::Parameter( u"delimiter"_s, true, "," ) << QgsExpressionFunction::Parameter( u"emptyvalue"_s, true, "" ), fcnArrayToString, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"string_to_array"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string"_s ) << QgsExpressionFunction::Parameter( u"delimiter"_s, true, "," ) << QgsExpressionFunction::Parameter( u"emptyvalue"_s, true, "" ), fcnStringToArray, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"generate_series"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"start"_s ) << QgsExpressionFunction::Parameter( u"stop"_s ) << QgsExpressionFunction::Parameter( u"step"_s, true, 1.0 ), fcnGenerateSeries, u"Arrays"_s )
        << new QgsStaticExpressionFunction( u"geometries_to_array"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"geometries"_s ), fcnGeometryCollectionAsArray, u"Arrays"_s )

        //functions for maps
        << new QgsStaticExpressionFunction( u"from_json"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"value"_s ), fcnLoadJson, u"Maps"_s, QString(), false, QSet<QString>(), false, QStringList() << u"json_to_map"_s )
        << new QgsStaticExpressionFunction( u"to_json"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"json_string"_s ), fcnWriteJson, u"Maps"_s, QString(), false, QSet<QString>(), false, QStringList() << u"map_to_json"_s )
        << new QgsStaticExpressionFunction( u"hstore_to_map"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"string"_s ), fcnHstoreToMap, u"Maps"_s )
        << new QgsStaticExpressionFunction( u"map_to_hstore"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"map"_s ), fcnMapToHstore, u"Maps"_s )
        << new QgsStaticExpressionFunction( u"map"_s, -1, fcnMap, u"Maps"_s )
        << new QgsStaticExpressionFunction( u"map_get"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"map"_s ) << QgsExpressionFunction::Parameter( u"key"_s ), fcnMapGet, u"Maps"_s )
        << new QgsStaticExpressionFunction( u"map_exist"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"map"_s ) << QgsExpressionFunction::Parameter( u"key"_s ), fcnMapExist, u"Maps"_s )
        << new QgsStaticExpressionFunction( u"map_delete"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"map"_s ) << QgsExpressionFunction::Parameter( u"key"_s ), fcnMapDelete, u"Maps"_s )
        << new QgsStaticExpressionFunction( u"map_insert"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"map"_s ) << QgsExpressionFunction::Parameter( u"key"_s ) << QgsExpressionFunction::Parameter( u"value"_s ), fcnMapInsert, u"Maps"_s )
        << new QgsStaticExpressionFunction( u"map_concat"_s, -1, fcnMapConcat, u"Maps"_s )
        << new QgsStaticExpressionFunction( u"map_akeys"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"map"_s ), fcnMapAKeys, u"Maps"_s )
        << new QgsStaticExpressionFunction( u"map_avals"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"map"_s ), fcnMapAVals, u"Maps"_s )
        << new QgsStaticExpressionFunction( u"map_prefix_keys"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"map"_s )
                                            << QgsExpressionFunction::Parameter( u"prefix"_s ),
                                            fcnMapPrefixKeys, u"Maps"_s )
        << new QgsStaticExpressionFunction( u"map_to_html_table"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"map"_s ),
                                            fcnMapToHtmlTable, u"Maps"_s )
        << new QgsStaticExpressionFunction( u"map_to_html_dl"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"map"_s ),
                                            fcnMapToHtmlDefinitionList, u"Maps"_s )
        << new QgsStaticExpressionFunction( u"url_encode"_s, QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( u"map"_s ),
                                            fcnToFormUrlEncode, u"Maps"_s )

        ;

    QgsExpressionContextUtils::registerContextFunctions();

    //QgsExpression has ownership of all built-in functions
    for ( QgsExpressionFunction *func : std::as_const( functions ) )
    {
      *sOwnedFunctions() << func;
      *sBuiltinFunctions() << func->name();
      sBuiltinFunctions()->append( func->aliases() );
    }
  }
  return functions;
}

bool QgsExpression::registerFunction( QgsExpressionFunction *function, bool transferOwnership )
{
  int fnIdx = functionIndex( function->name() );
  if ( fnIdx != -1 )
  {
    return false;
  }

  QMutexLocker locker( &sFunctionsMutex );
  sFunctions()->append( function );
  if ( transferOwnership )
    sOwnedFunctions()->append( function );

  return true;
}

bool QgsExpression::unregisterFunction( const QString &name )
{
  // You can never override the built in functions.
  if ( QgsExpression::BuiltinFunctions().contains( name ) )
  {
    return false;
  }
  int fnIdx = functionIndex( name );
  if ( fnIdx != -1 )
  {
    QMutexLocker locker( &sFunctionsMutex );
    sFunctions()->removeAt( fnIdx );
    sFunctionIndexMap.clear();
    return true;
  }
  return false;
}

void QgsExpression::cleanRegisteredFunctions()
{
  qDeleteAll( *sOwnedFunctions() );
  sOwnedFunctions()->clear();
}

const QStringList &QgsExpression::BuiltinFunctions()
{
  if ( sBuiltinFunctions()->isEmpty() )
  {
    Functions();  // this method builds the gmBuiltinFunctions as well
  }
  return *sBuiltinFunctions();
}

QgsArrayForeachExpressionFunction::QgsArrayForeachExpressionFunction()
  : QgsExpressionFunction( u"array_foreach"_s, QgsExpressionFunction::ParameterList()  // skip-keyword-check
                           << QgsExpressionFunction::Parameter( u"array"_s )
                           << QgsExpressionFunction::Parameter( u"expression"_s ),
                           u"Arrays"_s )
{

}

bool QgsArrayForeachExpressionFunction::isStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const
{
  bool isStatic = false;

  QgsExpressionNode::NodeList *args = node->args();

  if ( args->count() < 2 )
    return false;

  if ( args->at( 0 )->isStatic( parent, context ) && args->at( 1 )->isStatic( parent, context ) )
  {
    isStatic = true;
  }
  return isStatic;
}

QVariant QgsArrayForeachExpressionFunction::run( QgsExpressionNode::NodeList *args, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node )
{
  Q_UNUSED( node )
  QVariantList result;

  if ( args->count() < 2 )
    // error
    return result;

  QVariantList array = args->at( 0 )->eval( parent, context ).toList();

  QgsExpressionContext *subContext = const_cast<QgsExpressionContext *>( context );
  std::unique_ptr< QgsExpressionContext > tempContext;
  if ( !subContext )
  {
    tempContext = std::make_unique< QgsExpressionContext >();
    subContext = tempContext.get();
  }

  QgsExpressionContextScope *subScope = new QgsExpressionContextScope();
  subContext->appendScope( subScope );

  int i = 0;
  for ( QVariantList::const_iterator it = array.constBegin(); it != array.constEnd(); ++it, ++i )
  {
    subScope->addVariable( QgsExpressionContextScope::StaticVariable( u"element"_s, *it, true ) );
    subScope->addVariable( QgsExpressionContextScope::StaticVariable( u"counter"_s, i, true ) );
    result << args->at( 1 )->eval( parent, subContext );
  }

  if ( context )
    delete subContext->popScope();

  return result;
}

QVariant QgsArrayForeachExpressionFunction::func( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node )
{
  // This is a dummy function, all the real handling is in run
  Q_UNUSED( values )
  Q_UNUSED( context )
  Q_UNUSED( parent )
  Q_UNUSED( node )

  Q_ASSERT( false );
  return QVariant();
}

bool QgsArrayForeachExpressionFunction::prepare( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const
{
  QgsExpressionNode::NodeList *args = node->args();

  if ( args->count() < 2 )
    // error
    return false;

  args->at( 0 )->prepare( parent, context );

  QgsExpressionContext subContext;
  if ( context )
    subContext = *context;

  QgsExpressionContextScope *subScope = new QgsExpressionContextScope();
  subScope->addVariable( QgsExpressionContextScope::StaticVariable( u"element"_s, QVariant(), true ) );
  subScope->addVariable( QgsExpressionContextScope::StaticVariable( u"counter"_s, QVariant(), true ) );
  subContext.appendScope( subScope );

  args->at( 1 )->prepare( parent, &subContext );

  return true;
}

QgsArrayFilterExpressionFunction::QgsArrayFilterExpressionFunction()
  : QgsExpressionFunction( u"array_filter"_s, QgsExpressionFunction::ParameterList()
                           << QgsExpressionFunction::Parameter( u"array"_s )
                           << QgsExpressionFunction::Parameter( u"expression"_s )
                           << QgsExpressionFunction::Parameter( u"limit"_s, true, 0 ),
                           u"Arrays"_s )
{

}

bool QgsArrayFilterExpressionFunction::isStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const
{
  bool isStatic = false;

  QgsExpressionNode::NodeList *args = node->args();

  if ( args->count() < 2 )
    return false;

  if ( args->at( 0 )->isStatic( parent, context ) && args->at( 1 )->isStatic( parent, context ) )
  {
    isStatic = true;
  }
  return isStatic;
}

QVariant QgsArrayFilterExpressionFunction::run( QgsExpressionNode::NodeList *args, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node )
{
  Q_UNUSED( node )
  QVariantList result;

  if ( args->count() < 2 )
    // error
    return result;

  const QVariantList array = args->at( 0 )->eval( parent, context ).toList();

  QgsExpressionContext *subContext = const_cast<QgsExpressionContext *>( context );
  std::unique_ptr< QgsExpressionContext > tempContext;
  if ( !subContext )
  {
    tempContext = std::make_unique< QgsExpressionContext >();
    subContext = tempContext.get();
  }

  QgsExpressionContextScope *subScope = new QgsExpressionContextScope();
  subContext->appendScope( subScope );

  int limit = 0;
  if ( args->count() >= 3 )
  {
    const QVariant limitVar = args->at( 2 )->eval( parent, context );

    if ( QgsExpressionUtils::isIntSafe( limitVar ) )
    {
      limit = limitVar.toInt();
    }
    else
    {
      return result;
    }
  }

  for ( const QVariant &value : array )
  {
    subScope->addVariable( QgsExpressionContextScope::StaticVariable( u"element"_s, value, true ) );
    if ( args->at( 1 )->eval( parent, subContext ).toBool() )
    {
      result << value;

      if ( limit > 0 && limit == result.size() )
        break;
    }
  }

  if ( context )
    delete subContext->popScope();

  return result;
}

QVariant QgsArrayFilterExpressionFunction::func( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node )
{
  // This is a dummy function, all the real handling is in run
  Q_UNUSED( values )
  Q_UNUSED( context )
  Q_UNUSED( parent )
  Q_UNUSED( node )

  Q_ASSERT( false );
  return QVariant();
}

bool QgsArrayFilterExpressionFunction::prepare( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const
{
  QgsExpressionNode::NodeList *args = node->args();

  if ( args->count() < 2 )
    // error
    return false;

  args->at( 0 )->prepare( parent, context );

  QgsExpressionContext subContext;
  if ( context )
    subContext = *context;

  QgsExpressionContextScope *subScope = new QgsExpressionContextScope();
  subScope->addVariable( QgsExpressionContextScope::StaticVariable( u"element"_s, QVariant(), true ) );
  subContext.appendScope( subScope );

  args->at( 1 )->prepare( parent, &subContext );

  return true;
}
QgsWithVariableExpressionFunction::QgsWithVariableExpressionFunction()
  : QgsExpressionFunction( u"with_variable"_s, QgsExpressionFunction::ParameterList() <<
                           QgsExpressionFunction::Parameter( u"name"_s )
                           << QgsExpressionFunction::Parameter( u"value"_s )
                           << QgsExpressionFunction::Parameter( u"expression"_s ),
                           u"General"_s )
{

}

bool QgsWithVariableExpressionFunction::isStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const
{
  bool isStatic = false;

  QgsExpressionNode::NodeList *args = node->args();

  if ( args->count() < 3 )
    return false;

  // We only need to check if the node evaluation is static, if both - name and value - are static.
  if ( args->at( 0 )->isStatic( parent, context ) && args->at( 1 )->isStatic( parent, context ) )
  {
    QVariant name = args->at( 0 )->eval( parent, context );
    QVariant value = args->at( 1 )->eval( parent, context );

    // Temporarily append a new scope to provide the variable
    appendTemporaryVariable( context, name.toString(), value );
    if ( args->at( 2 )->isStatic( parent, context ) )
      isStatic = true;
    popTemporaryVariable( context );
  }

  return isStatic;
}

QVariant QgsWithVariableExpressionFunction::run( QgsExpressionNode::NodeList *args, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node )
{
  Q_UNUSED( node )
  QVariant result;

  if ( args->count() < 3 )
    // error
    return result;

  QVariant name = args->at( 0 )->eval( parent, context );
  QVariant value = args->at( 1 )->eval( parent, context );

  const QgsExpressionContext *updatedContext = context;
  std::unique_ptr< QgsExpressionContext > tempContext;
  if ( !updatedContext )
  {
    tempContext = std::make_unique< QgsExpressionContext >();
    updatedContext = tempContext.get();
  }

  appendTemporaryVariable( updatedContext, name.toString(), value );
  result = args->at( 2 )->eval( parent, updatedContext );

  if ( context )
    popTemporaryVariable( updatedContext );

  return result;
}

QVariant QgsWithVariableExpressionFunction::func( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node )
{
  // This is a dummy function, all the real handling is in run
  Q_UNUSED( values )
  Q_UNUSED( context )
  Q_UNUSED( parent )
  Q_UNUSED( node )

  Q_ASSERT( false );
  return QVariant();
}

bool QgsWithVariableExpressionFunction::prepare( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const
{
  QgsExpressionNode::NodeList *args = node->args();

  if ( args->count() < 3 )
    // error
    return false;

  QVariant name = args->at( 0 )->prepare( parent, context );
  QVariant value = args->at( 1 )->prepare( parent, context );

  const QgsExpressionContext *updatedContext = context;
  std::unique_ptr< QgsExpressionContext > tempContext;
  if ( !updatedContext )
  {
    tempContext = std::make_unique< QgsExpressionContext >();
    updatedContext = tempContext.get();
  }

  appendTemporaryVariable( updatedContext, name.toString(), value );
  args->at( 2 )->prepare( parent, updatedContext );

  if ( context )
    popTemporaryVariable( updatedContext );

  return true;
}

void QgsWithVariableExpressionFunction::popTemporaryVariable( const QgsExpressionContext *context ) const
{
  QgsExpressionContext *updatedContext = const_cast<QgsExpressionContext *>( context );
  delete updatedContext->popScope();
}

void QgsWithVariableExpressionFunction::appendTemporaryVariable( const QgsExpressionContext *context, const QString &name, const QVariant &value ) const
{
  QgsExpressionContextScope *scope = new QgsExpressionContextScope();
  scope->addVariable( QgsExpressionContextScope::StaticVariable( name, value, true ) );

  QgsExpressionContext *updatedContext = const_cast<QgsExpressionContext *>( context );
  updatedContext->appendScope( scope );
}

