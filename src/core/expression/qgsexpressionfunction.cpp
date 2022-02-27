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


#include <random>

#include "qgscoordinateformatter.h"
#include "qgscoordinateutils.h"
#include "qgsexpressionfunction.h"
#include "qgsexpressionutils.h"
#include "qgsexpressionnodeimpl.h"
#include "qgsexiftools.h"
#include "qgsfeaturerequest.h"
#include "qgsgeos.h"
#include "qgsstringutils.h"
#include "qgsmultipoint.h"
#include "qgsgeometryutils.h"
#include "qgshstoreutils.h"
#include "qgsmultilinestring.h"
#include "qgslinestring.h"
#include "qgscurvepolygon.h"
#include "qgsmaptopixelgeometrysimplifier.h"
#include "qgspolygon.h"
#include "qgstriangle.h"
#include "qgscurve.h"
#include "qgsregularpolygon.h"
#include "qgsquadrilateral.h"
#include "qgsmultipolygon.h"
#include "qgsogcutils.h"
#include "qgsdistancearea.h"
#include "qgsgeometryengine.h"
#include "qgsexpressionsorter_p.h"
#include "qgssymbollayerutils.h"
#include "qgsstyle.h"
#include "qgsexception.h"
#include "qgsmessagelog.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsrasterbandstats.h"
#include "qgscolorramp.h"
#include "qgsfieldformatterregistry.h"
#include "qgsfieldformatter.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsproviderregistry.h"
#include "sqlite3.h"
#include "qgstransaction.h"
#include "qgsthreadingutils.h"
#include "qgsapplication.h"
#include "qgis.h"
#include "qgsexpressioncontextutils.h"
#include "qgsunittypes.h"
#include "qgsspatialindex.h"
#include "qgscolorrampimpl.h"

#include <QMimeDatabase>
#include <QProcessEnvironment>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QUuid>
#include <QUrlQuery>

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
  return mGroups.isEmpty() ? false : mGroups.contains( QStringLiteral( "deprecated" ) );
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

  QString name = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  return context->variable( name );
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

static QVariant fcnExpScale( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
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
  return QVariant( ( ( rangeMax - rangeMin ) / std::pow( domainMax - domainMin, exponent ) ) * std::pow( val - domainMin, exponent ) + rangeMin );
}

static QVariant fcnMax( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariant result( QVariant::Double );
  double maxVal = std::numeric_limits<double>::quiet_NaN();
  for ( const QVariant &val : values )
  {
    double testVal = val.isNull() ? std::numeric_limits<double>::quiet_NaN() : QgsExpressionUtils::getDoubleValue( val, parent );
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
  QVariant result( QVariant::Double );
  double minVal = std::numeric_limits<double>::quiet_NaN();
  for ( const QVariant &val : values )
  {
    double testVal = val.isNull() ? std::numeric_limits<double>::quiet_NaN() : QgsExpressionUtils::getDoubleValue( val, parent );
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
  QgsVectorLayer *vl = QgsExpressionUtils::getVectorLayer( value, parent );
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
  QgsAggregateCalculator::Aggregate aggregate = QgsAggregateCalculator::stringToAggregate( QgsExpressionUtils::getStringValue( value, parent ), &ok );
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

    bool isStatic = true;
    if ( filterExp.referencedVariables().contains( QStringLiteral( "parent" ) )
         || filterExp.referencedVariables().contains( QString() )
         || subExp.referencedVariables().contains( QStringLiteral( "parent" ) )
         || subExp.referencedVariables().contains( QString() ) )
    {
      isStatic = false;
    }
    else
    {
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
    }

    if ( !isStatic )
    {
      cacheKey = QStringLiteral( "aggfcn:%1:%2:%3:%4:%5%6:%7" ).arg( vl->id(), QString::number( aggregate ), subExpression, parameters.filter,
                 QString::number( context->feature().id() ), QString::number( qHash( context->feature() ) ), orderBy );
    }
    else
    {
      cacheKey = QStringLiteral( "aggfcn:%1:%2:%3:%4:%5" ).arg( vl->id(), QString::number( aggregate ), subExpression, parameters.filter, orderBy );
    }

    if ( context->hasCachedValue( cacheKey ) )
    {
      return context->cachedValue( cacheKey );
    }

    QgsExpressionContext subContext( *context );
    QgsExpressionContextScope *subScope = new QgsExpressionContextScope();
    subScope->setVariable( QStringLiteral( "parent" ), context->feature() );
    subContext.appendScope( subScope );
    result = vl->aggregate( aggregate, subExpression, parameters, &subContext, &ok, nullptr, context->feedback(), &aggregateError );

    if ( ok )
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
  QgsVectorLayer *vl = QgsExpressionUtils::getVectorLayer( context->variable( QStringLiteral( "layer" ) ), parent );
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

  QgsVectorLayer *childLayer = relation.referencingLayer();

  // second node is aggregate type
  node = QgsExpressionUtils::getNode( values.at( 1 ), parent );
  ENSURE_NO_EVAL_ERROR
  value = node->eval( parent, context );
  ENSURE_NO_EVAL_ERROR
  bool ok = false;
  QgsAggregateCalculator::Aggregate aggregate = QgsAggregateCalculator::stringToAggregate( QgsExpressionUtils::getStringValue( value, parent ), &ok );
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

  QString cacheKey = QStringLiteral( "relagg:%1:%2:%3:%4:%5" ).arg( vl->id(),
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


static QVariant fcnAggregateGeneric( QgsAggregateCalculator::Aggregate aggregate, const QVariantList &values, QgsAggregateCalculator::AggregateParameters parameters, const QgsExpressionContext *context, QgsExpression *parent, int orderByPos = -1 )
{
  if ( !context )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot use aggregate function in this context" ) );
    return QVariant();
  }

  // first step - find current layer
  QgsVectorLayer *vl = QgsExpressionUtils::getVectorLayer( context->variable( QStringLiteral( "layer" ) ), parent );
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
    QString groupByClause = QStringLiteral( "%1 %2 %3" ).arg( groupBy,
                            groupByValue.isNull() ? QStringLiteral( "is" ) : QStringLiteral( "=" ),
                            QgsExpression::quotedValue( groupByValue ) );
    if ( !parameters.filter.isEmpty() )
      parameters.filter = QStringLiteral( "(%1) AND (%2)" ).arg( parameters.filter, groupByClause );
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
    cacheKey = QStringLiteral( "agg:%1:%2:%3:%4:%5%6:%7" ).arg( vl->id(), QString::number( aggregate ), subExpression, parameters.filter,
               QString::number( context->feature().id() ), QString::number( qHash( context->feature() ) ), orderBy );
  }
  else
  {
    cacheKey = QStringLiteral( "agg:%1:%2:%3:%4:%5" ).arg( vl->id(), QString::number( aggregate ), subExpression, parameters.filter, orderBy );
  }

  if ( context->hasCachedValue( cacheKey ) )
    return context->cachedValue( cacheKey );

  QVariant result;
  bool ok = false;

  QgsExpressionContext subContext( *context );
  QgsExpressionContextScope *subScope = new QgsExpressionContextScope();
  subScope->setVariable( QStringLiteral( "parent" ), context->feature() );
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
  return fcnAggregateGeneric( QgsAggregateCalculator::Count, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateCountDistinct( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::CountDistinct, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateCountMissing( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::CountMissing, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMin( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::Min, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMax( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::Max, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateSum( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::Sum, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMean( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::Mean, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMedian( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::Median, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateStdev( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::StDevSample, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateRange( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::Range, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMinority( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::Minority, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMajority( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::Majority, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateQ1( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::FirstQuartile, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateQ3( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::ThirdQuartile, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateIQR( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::InterQuartileRange, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMinLength( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::StringMinimumLength, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateMaxLength( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::StringMaximumLength, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
}

static QVariant fcnAggregateCollectGeometry( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::GeometryCollect, values, QgsAggregateCalculator::AggregateParameters(), context, parent );
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

  return fcnAggregateGeneric( QgsAggregateCalculator::StringConcatenate, values, parameters, context, parent, 4 );
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

  return fcnAggregateGeneric( QgsAggregateCalculator::StringConcatenateUnique, values, parameters, context, parent, 4 );
}

static QVariant fcnAggregateArray( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  return fcnAggregateGeneric( QgsAggregateCalculator::ArrayAggregate, values, QgsAggregateCalculator::AggregateParameters(), context, parent, 3 );
}

static QVariant fcnMapScale( const QVariantList &, const QgsExpressionContext *context, QgsExpression *, const QgsExpressionNodeFunction * )
{
  if ( !context )
    return QVariant();

  QVariant scale = context->variable( QStringLiteral( "map_scale" ) );
  bool ok = false;
  if ( !scale.isValid() || scale.isNull() )
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
    if ( value.isNull() )
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
  return QVariant( elems.join( QLatin1Char( ' ' ) ) );
}

static QVariant fcnTrim( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  return QVariant( str.trimmed() );
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
  if ( values.at( 0 ).canConvert<QgsGeometry>() )
  {
    //geometry variant
    QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
    if ( geom.type() != QgsWkbTypes::LineGeometry )
      return QVariant();

    return QVariant( geom.length() );
  }

  //otherwise fall back to string variant
  QString str = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  return QVariant( str.length() );
}

static QVariant fcnLength3D( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.type() != QgsWkbTypes::LineGeometry )
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

static QVariant fcnReplace( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( values.count() == 2 && values.at( 1 ).type() == QVariant::Map )
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

    if ( !QgsExpressionUtils::isList( values.at( 1 ) ) && values.at( 2 ).type() != QVariant::StringList )
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
  if ( values.at( 0 ).toString().compare( QStringLiteral( "WithoutBraces" ), Qt::CaseInsensitive ) == 0 )
    uuid = QUuid::createUuid().toString( QUuid::StringFormat::WithoutBraces );
  else if ( values.at( 0 ).toString().compare( QStringLiteral( "Id128" ), Qt::CaseInsensitive ) == 0 )
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
  // TODO: handling of 64-bit feature ids?
  return QVariant( static_cast< int >( f.id() ) );
}

static QVariant fcnRasterValue( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsRasterLayer *layer = QgsExpressionUtils::getRasterLayer( values.at( 0 ), parent );
  if ( !layer || !layer->dataProvider() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `raster_value` requires a valid raster layer." ) );
    return QVariant();
  }

  int bandNb = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  if ( bandNb < 1 || bandNb > layer->bandCount() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `raster_value` requires a valid raster band number." ) );
    return QVariant();
  }

  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 2 ), parent );
  if ( geom.isNull() || geom.type() != QgsWkbTypes::PointGeometry )
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

static QVariant fcnAttributes( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsFeature feature;
  if ( values.size() == 0 || values.at( 0 ).isNull() )
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

  if ( values.isEmpty() )
  {
    feature = context->feature();
    layer = QgsExpressionUtils::getVectorLayer( context->variable( QStringLiteral( "layer" ) ), parent );
  }
  else if ( values.size() == 1 )
  {
    layer = QgsExpressionUtils::getVectorLayer( context->variable( QStringLiteral( "layer" ) ), parent );
    feature = QgsExpressionUtils::getFeature( values.at( 0 ), parent );
  }
  else if ( values.size() == 2 )
  {
    layer = QgsExpressionUtils::getVectorLayer( values.at( 0 ), parent );
    feature = QgsExpressionUtils::getFeature( values.at( 1 ), parent );
  }
  else
  {
    parent->setEvalErrorString( QObject::tr( "Function `represent_attributes` requires no more than two parameters. %n given.", nullptr, values.length() ) );
    return QVariant();
  }

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
    const QString cacheValueKey = QStringLiteral( "repvalfcnval:%1:%2:%3" ).arg( layer->id(), fieldName, attributeVal.toString() );
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
        const QString cacheKey = QStringLiteral( "repvalfcn:%1:%2" ).arg( layer->id(), fieldName );

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

  if ( values.isEmpty() )
  {
    feature = context->feature();
    layer = QgsExpressionUtils::getVectorLayer( context->variable( QStringLiteral( "layer" ) ), parent );
  }
  else if ( values.size() == 1 )
  {
    layer = QgsExpressionUtils::getVectorLayer( context->variable( QStringLiteral( "layer" ) ), parent );
    feature = QgsExpressionUtils::getFeature( values.at( 0 ), parent );
  }
  else if ( values.size() == 2 )
  {
    layer = QgsExpressionUtils::getVectorLayer( values.at( 0 ), parent );
    feature = QgsExpressionUtils::getFeature( values.at( 1 ), parent );
  }
  else if ( values.size() == 3 )
  {
    layer = QgsExpressionUtils::getVectorLayer( values.at( 0 ), parent );
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
  QgsVectorLayer *layer = nullptr;
  QgsFeature feature;

  if ( values.isEmpty() )
  {
    feature = context->feature();
    layer = QgsExpressionUtils::getVectorLayer( context->variable( QStringLiteral( "layer" ) ), parent );
  }
  else if ( values.size() == 1 )
  {
    layer = QgsExpressionUtils::getVectorLayer( context->variable( QStringLiteral( "layer" ) ), parent );
    feature = QgsExpressionUtils::getFeature( values.at( 0 ), parent );
  }
  else if ( values.size() == 2 )
  {
    layer = QgsExpressionUtils::getVectorLayer( values.at( 0 ), parent );
    feature = QgsExpressionUtils::getFeature( values.at( 1 ), parent );
  }
  else
  {
    parent->setEvalErrorString( QObject::tr( "Function `is_selected` requires no more than two parameters. %n given.", nullptr, values.length() ) );
    return QVariant();
  }

  if ( !layer || !feature.isValid() )
  {
    return QVariant( QVariant::Bool );
  }

  return layer->selectedFeatureIds().contains( feature.id() );
}

static QVariant fcnNumSelected( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsVectorLayer *layer = nullptr;

  if ( values.isEmpty() )
    layer = QgsExpressionUtils::getVectorLayer( context->variable( QStringLiteral( "layer" ) ), parent );
  else if ( values.count() == 1 )
    layer = QgsExpressionUtils::getVectorLayer( values.at( 0 ), parent );
  else
  {
    parent->setEvalErrorString( QObject::tr( "Function `num_selected` requires no more than one parameter. %n given.", nullptr, values.length() ) );
    return QVariant();
  }

  if ( !layer )
  {
    return QVariant( QVariant::LongLong );
  }

  return layer->selectedFeatureCount();
}

static QVariant fcnSqliteFetchAndIncrement( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  static QMap<QString, qlonglong> counterCache;
  QVariant functionResult;

  std::function<void()> fetchAndIncrementFunc = [ =, &functionResult ]()
  {
    QString database;
    const QgsVectorLayer *layer = QgsExpressionUtils::getVectorLayer( values.at( 0 ), parent );

    if ( layer )
    {
      const QVariantMap decodedUri = QgsProviderRegistry::instance()->decodeUri( layer->providerType(), layer->dataProvider()->dataSourceUri() );
      database = decodedUri.value( QStringLiteral( "path" ) ).toString();
      if ( database.isEmpty() )
      {
        parent->setEvalErrorString( QObject::tr( "Could not extract file path from layer `%1`." ).arg( layer->name() ) );
      }
    }
    else
    {
      database = values.at( 0 ).toString();
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

    QString cacheString = QStringLiteral( "%1:%2:%3:%4:%5" ).arg( database, table, idColumn, filterAttribute, filterValue.toString() );

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

      currentValSql = QStringLiteral( "SELECT %1 FROM %2" ).arg( QgsSqliteUtils::quotedIdentifier( idColumn ), QgsSqliteUtils::quotedIdentifier( table ) );
      if ( !filterAttribute.isNull() )
      {
        currentValSql += QStringLiteral( " WHERE %1 = %2" ).arg( QgsSqliteUtils::quotedIdentifier( filterAttribute ), QgsSqliteUtils::quotedValue( filterValue ) );
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
      upsertSql = QStringLiteral( "INSERT OR REPLACE INTO %1" ).arg( QgsSqliteUtils::quotedIdentifier( table ) );
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

      upsertSql += QLatin1String( " (" ) + cols.join( ',' ) + ')';
      upsertSql += QLatin1String( " VALUES " );
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
        parent->setEvalErrorString( QStringLiteral( "Could not increment value: SQLite error: \"%1\" (%2)." ).arg( errorMessage, QString::number( result ) ) );
        functionResult = QVariant();
        return;
      }
    }

    functionResult = QVariant();
  };

  QgsThreadingUtils::runOnMainThread( fetchAndIncrementFunc );

  return functionResult;
}

static QVariant fcnConcat( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString concat;
  for ( const QVariant &value : values )
  {
    if ( !value.isNull() )
      concat += QgsExpressionUtils::getStringValue( value, parent );
  }
  return concat;
}

static QVariant fcnStrpos( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString string = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  return string.indexOf( QgsExpressionUtils::getStringValue( values.at( 1 ), parent ) ) + 1;
}

static QVariant fcnRight( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString string = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );
  int pos = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  return string.right( pos );
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
  if ( formatString.compare( QLatin1String( "suffix" ), Qt::CaseInsensitive ) == 0 )
  {
    flags = QgsCoordinateFormatter::FlagDegreesUseStringSuffix;
  }
  else if ( formatString.compare( QLatin1String( "aligned" ), Qt::CaseInsensitive ) == 0 )
  {
    flags = QgsCoordinateFormatter::FlagDegreesUseStringSuffix | QgsCoordinateFormatter::FlagDegreesPadMinutesSeconds;
  }
  else if ( ! formatString.isEmpty() )
  {
    parent->setEvalErrorString( QObject::tr( "Invalid formatting parameter: '%1'. It must be empty, or 'suffix' or 'aligned'." ).arg( formatString ) );
    return QVariant();
  }

  if ( axis.compare( QLatin1String( "x" ), Qt::CaseInsensitive ) == 0 )
  {
    return QVariant::fromValue( QgsCoordinateFormatter::formatX( value, format, precision, flags ) );
  }
  else if ( axis.compare( QLatin1String( "y" ), Qt::CaseInsensitive ) == 0 )
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

static QVariant fcnExif( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString filepath = QgsExpressionUtils::getFilePathValue( values.at( 0 ), parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( QStringLiteral( "exif" ) ) );
    return QVariant();
  }
  QString tag = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  return !tag.isNull() ? QgsExifTools::readTag( filepath, tag ) : QVariant( QgsExifTools::readTags( filepath ) );
}

static QVariant fcnExifGeoTag( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString filepath = QgsExpressionUtils::getFilePathValue( values.at( 0 ), parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( QStringLiteral( "exif_geotag" ) ) );
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
  ENSURE_GEOM_TYPE( f, g, QgsWkbTypes::PointGeometry )
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
  ENSURE_GEOM_TYPE( f, g, QgsWkbTypes::PointGeometry )
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
  ENSURE_GEOM_TYPE( f, g, QgsWkbTypes::PointGeometry )

  if ( g.isEmpty() )
    return QVariant();

  const QgsAbstractGeometry *abGeom = g.constGet();

  if ( g.isEmpty() || !abGeom->is3D() )
    return QVariant();

  if ( g.type() == QgsWkbTypes::PointGeometry && !g.isMultipart() )
  {
    const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( g.constGet() );
    if ( point )
      return point->z();
  }
  else if ( g.type() == QgsWkbTypes::PointGeometry && g.isMultipart() )
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

static QVariant fcnGeomX( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
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

static QVariant fcnGeomY( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
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

static QVariant fcnGeomZ( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( geom.isNull() )
    return QVariant(); //or 0?

  if ( !geom.constGet()->is3D() )
    return QVariant();

  //if single point, return the point's z coordinate
  if ( geom.type() == QgsWkbTypes::PointGeometry && !geom.isMultipart() )
  {
    const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( geom.constGet() );
    if ( point )
      return point->z();
  }
  else if ( geom.type() == QgsWkbTypes::PointGeometry && geom.isMultipart() )
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
  if ( geom.type() == QgsWkbTypes::PointGeometry && !geom.isMultipart() )
  {
    const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( geom.constGet() );
    if ( point )
      return point->m();
  }
  else if ( geom.type() == QgsWkbTypes::PointGeometry && geom.isMultipart() )
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

  QgsMapToPixelSimplifier simplifier( QgsMapToPixelSimplifier::SimplifyGeometry, tolerance, QgsMapToPixelSimplifier::Visvalingam );

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
      parent->setEvalErrorString( QStringLiteral( "Dash pattern must be an array of numbers" ) );
      return QgsGeometry();
    }
  }

  if ( dashPattern.size() % 2 != 0 )
  {
    parent->setEvalErrorString( QStringLiteral( "Dash pattern must contain an even number of elements" ) );
    return QgsGeometry();
  }

  const QString startRuleString = QgsExpressionUtils::getStringValue( values.at( 2 ), parent ).trimmed();
  Qgis::DashPatternLineEndingRule startRule = Qgis::DashPatternLineEndingRule::NoRule;
  if ( startRuleString.compare( QLatin1String( "no_rule" ), Qt::CaseInsensitive ) == 0 )
    startRule = Qgis::DashPatternLineEndingRule::NoRule;
  else if ( startRuleString.compare( QLatin1String( "full_dash" ), Qt::CaseInsensitive ) == 0 )
    startRule = Qgis::DashPatternLineEndingRule::FullDash;
  else if ( startRuleString.compare( QLatin1String( "half_dash" ), Qt::CaseInsensitive ) == 0 )
    startRule = Qgis::DashPatternLineEndingRule::HalfDash;
  else if ( startRuleString.compare( QLatin1String( "full_gap" ), Qt::CaseInsensitive ) == 0 )
    startRule = Qgis::DashPatternLineEndingRule::FullGap;
  else if ( startRuleString.compare( QLatin1String( "half_gap" ), Qt::CaseInsensitive ) == 0 )
    startRule = Qgis::DashPatternLineEndingRule::HalfGap;
  else
  {
    parent->setEvalErrorString( QStringLiteral( "'%1' is not a valid dash pattern rule" ).arg( startRuleString ) );
    return QgsGeometry();
  }

  const QString endRuleString = QgsExpressionUtils::getStringValue( values.at( 3 ), parent ).trimmed();
  Qgis::DashPatternLineEndingRule endRule = Qgis::DashPatternLineEndingRule::NoRule;
  if ( endRuleString.compare( QLatin1String( "no_rule" ), Qt::CaseInsensitive ) == 0 )
    endRule = Qgis::DashPatternLineEndingRule::NoRule;
  else if ( endRuleString.compare( QLatin1String( "full_dash" ), Qt::CaseInsensitive ) == 0 )
    endRule = Qgis::DashPatternLineEndingRule::FullDash;
  else if ( endRuleString.compare( QLatin1String( "half_dash" ), Qt::CaseInsensitive ) == 0 )
    endRule = Qgis::DashPatternLineEndingRule::HalfDash;
  else if ( endRuleString.compare( QLatin1String( "full_gap" ), Qt::CaseInsensitive ) == 0 )
    endRule = Qgis::DashPatternLineEndingRule::FullGap;
  else if ( endRuleString.compare( QLatin1String( "half_gap" ), Qt::CaseInsensitive ) == 0 )
    endRule = Qgis::DashPatternLineEndingRule::HalfGap;
  else
  {
    parent->setEvalErrorString( QStringLiteral( "'%1' is not a valid dash pattern rule" ).arg( endRuleString ) );
    return QgsGeometry();
  }

  const QString adjustString = QgsExpressionUtils::getStringValue( values.at( 4 ), parent ).trimmed();
  Qgis::DashPatternSizeAdjustment adjustment = Qgis::DashPatternSizeAdjustment::ScaleBothDashAndGap;
  if ( adjustString.compare( QLatin1String( "both" ), Qt::CaseInsensitive ) == 0 )
    adjustment = Qgis::DashPatternSizeAdjustment::ScaleBothDashAndGap;
  else if ( adjustString.compare( QLatin1String( "dash" ), Qt::CaseInsensitive ) == 0 )
    adjustment = Qgis::DashPatternSizeAdjustment::ScaleDashOnly;
  else if ( adjustString.compare( QLatin1String( "gap" ), Qt::CaseInsensitive ) == 0 )
    adjustment = Qgis::DashPatternSizeAdjustment::ScaleGapOnly;
  else
  {
    parent->setEvalErrorString( QStringLiteral( "'%1' is not a valid dash pattern size adjustment" ).arg( adjustString ) );
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
    if ( value.canConvert<QgsGeometry>() )
    {
      parts << value.value<QgsGeometry>();
    }
    else
    {
      parent->setEvalErrorString( QStringLiteral( "Cannot convert to geometry" ) );
      return QgsGeometry();
    }
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
      return QVariant::fromValue( QgsGeometry( new QgsPoint( QgsWkbTypes::PointZ, x, y, z ) ) );
    case 4:
      return QVariant::fromValue( QgsGeometry( new QgsPoint( QgsWkbTypes::PointZM, x, y, z, m ) ) );
  }
  return QVariant(); //avoid warning
}

static QVariant fcnMakePointM( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  double x = QgsExpressionUtils::getDoubleValue( values.at( 0 ), parent );
  double y = QgsExpressionUtils::getDoubleValue( values.at( 1 ), parent );
  double m = QgsExpressionUtils::getDoubleValue( values.at( 2 ), parent );
  return QVariant::fromValue( QgsGeometry( new QgsPoint( QgsWkbTypes::PointM, x, y, 0.0, m ) ) );
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

    if ( geom.type() != QgsWkbTypes::PointGeometry || geom.isMultipart() )
      return;

    const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( geom.constGet() );
    if ( !point )
      return;

    points << *point;
  };

  for ( const QVariant &value : values )
  {
    if ( value.type() == QVariant::List )
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

  if ( outerRing.type() == QgsWkbTypes::PolygonGeometry )
    return outerRing; // if it's already a polygon we have nothing to do

  if ( outerRing.type() != QgsWkbTypes::LineGeometry || outerRing.isNull() )
    return QVariant();

  std::unique_ptr< QgsPolygon > polygon = std::make_unique< QgsPolygon >();

  const QgsCurve *exteriorRing = qgsgeometry_cast< QgsCurve * >( outerRing.constGet() );
  if ( !exteriorRing && outerRing.isMultipart() )
  {
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( outerRing.constGet() ) )
    {
      if ( collection->numGeometries() == 1 )
      {
        exteriorRing = qgsgeometry_cast< QgsCurve * >( collection->geometryN( 0 ) );
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

    if ( ringGeom.type() != QgsWkbTypes::LineGeometry || ringGeom.isNull() )
      continue;

    const QgsCurve *ring = qgsgeometry_cast< QgsCurve * >( ringGeom.constGet() );
    if ( !ring && ringGeom.isMultipart() )
    {
      if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( ringGeom.constGet() ) )
      {
        if ( collection->numGeometries() == 1 )
        {
          ring = qgsgeometry_cast< QgsCurve * >( collection->geometryN( 0 ) );
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
  std::unique_ptr<QgsTriangle> tr( new QgsTriangle() );
  std::unique_ptr<QgsLineString> lineString( new QgsLineString() );
  lineString->clear();

  for ( const QVariant &value : values )
  {
    QgsGeometry geom = QgsExpressionUtils::getGeometry( value, parent );
    if ( geom.isNull() )
      return QVariant();

    if ( geom.type() != QgsWkbTypes::PointGeometry || geom.isMultipart() )
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

  if ( geom.type() != QgsWkbTypes::PointGeometry || geom.isMultipart() )
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

  if ( geom.type() != QgsWkbTypes::PointGeometry || geom.isMultipart() )
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

  if ( pt1.type() != QgsWkbTypes::PointGeometry || pt1.isMultipart() )
    return QVariant();

  QgsGeometry pt2 = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  if ( pt2.isNull() )
    return QVariant();

  if ( pt2.type() != QgsWkbTypes::PointGeometry || pt2.isMultipart() )
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
  if ( pt1.type() != QgsWkbTypes::PointGeometry || pt1.isMultipart() )
    return QVariant();

  QgsGeometry pt2 = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  if ( pt2.isNull() )
    return QVariant();
  if ( pt2.type() != QgsWkbTypes::PointGeometry || pt2.isMultipart() )
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
  if ( pt1.type() != QgsWkbTypes::PointGeometry || pt1.isMultipart() )
    return QVariant();

  QgsGeometry pt2 = QgsExpressionUtils::getGeometry( values.at( 1 ), parent );
  if ( pt2.isNull() )
    return QVariant();
  if ( pt2.type() != QgsWkbTypes::PointGeometry || pt2.isMultipart() )
    return QVariant();

  QgsGeometry pt3 = QgsExpressionUtils::getGeometry( values.at( 2 ), parent );
  if ( pt3.isNull() )
    return QVariant();
  if ( pt3.type() != QgsWkbTypes::PointGeometry || pt3.isMultipart() )
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

static QVariant pointAt( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent ) // helper function
{
  FEAT_FROM_CONTEXT( context, f )
  int idx = QgsExpressionUtils::getNativeIntValue( values.at( 0 ), parent );
  QgsGeometry g = f.geometry();
  if ( g.isNull() )
    return QVariant();

  if ( idx < 0 )
  {
    idx += g.constGet()->nCoordinates();
  }
  if ( idx < 0 || idx >= g.constGet()->nCoordinates() )
  {
    parent->setEvalErrorString( QObject::tr( "Index is out of range" ) );
    return QVariant();
  }

  QgsPointXY p = g.vertexAt( idx );
  return QVariant( QPointF( p.x(), p.y() ) );
}

static QVariant fcnXat( const QVariantList &values, const QgsExpressionContext *f, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariant v = pointAt( values, f, parent );
  if ( v.type() == QVariant::PointF )
    return QVariant( v.toPointF().x() );
  else
    return QVariant();
}
static QVariant fcnYat( const QVariantList &values, const QgsExpressionContext *f, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariant v = pointAt( values, f, parent );
  if ( v.type() == QVariant::PointF )
    return QVariant( v.toPointF().y() );
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
    QgsWeakMapLayerPointer mapLayerPtr {context->variable( QStringLiteral( "layer" ) ).value<QgsWeakMapLayerPointer>() };
    if ( mapLayerPtr )
    {
      ogcContext.layer = mapLayerPtr.data();
      ogcContext.transformContext = context->variable( QStringLiteral( "_project_transform_context" ) ).value<QgsCoordinateTransformContext>();
    }
  }
  QgsGeometry geom = QgsOgcUtils::geometryFromGML( gml, ogcContext );
  QVariant result = !geom.isNull() ? QVariant::fromValue( geom ) : QVariant();
  return result;
}

static QVariant fcnGeomArea( const QVariantList &, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  FEAT_FROM_CONTEXT( context, f )
  ENSURE_GEOM_TYPE( f, g, QgsWkbTypes::PolygonGeometry )
  QgsDistanceArea *calc = parent->geomCalculator();
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

static QVariant fcnArea( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.type() != QgsWkbTypes::PolygonGeometry )
    return QVariant();

  return QVariant( geom.area() );
}

static QVariant fcnGeomLength( const QVariantList &, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  FEAT_FROM_CONTEXT( context, f )
  ENSURE_GEOM_TYPE( f, g, QgsWkbTypes::LineGeometry )
  QgsDistanceArea *calc = parent->geomCalculator();
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

static QVariant fcnGeomPerimeter( const QVariantList &, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  FEAT_FROM_CONTEXT( context, f )
  ENSURE_GEOM_TYPE( f, g, QgsWkbTypes::PolygonGeometry )
  QgsDistanceArea *calc = parent->geomCalculator();
  if ( calc )
  {
    double len = calc->measurePerimeter( f.geometry() );
    len = calc->convertLengthMeasurement( len, parent->distanceUnits() );
    return QVariant( len );
  }
  else
  {
    return f.geometry().isNull() ? QVariant( 0 ) : QVariant( f.geometry().constGet()->perimeter() );
  }
}

static QVariant fcnPerimeter( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( geom.type() != QgsWkbTypes::PolygonGeometry )
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
      curvePolygon = qgsgeometry_cast< QgsCurvePolygon *>( collection->geometryN( i ) );
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
    return QVariant( QVariant::Double );

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
    return QVariant( QVariant::Double );

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
    return QVariant( QVariant::Double );

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
    return QVariant( QVariant::Double );

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
  if ( values.at( 0 ).isNull() )
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
  if ( endCapString.compare( QLatin1String( "flat" ), Qt::CaseInsensitive ) == 0 )
    capStyle = Qgis::EndCapStyle::Flat;
  else if ( endCapString.compare( QLatin1String( "square" ), Qt::CaseInsensitive ) == 0 )
    capStyle = Qgis::EndCapStyle::Square;

  Qgis::JoinStyle joinStyle = Qgis::JoinStyle::Round;
  if ( joinString.compare( QLatin1String( "miter" ), Qt::CaseInsensitive ) == 0 )
    joinStyle = Qgis::JoinStyle::Miter;
  else if ( joinString.compare( QLatin1String( "bevel" ), Qt::CaseInsensitive ) == 0 )
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
  if ( fGeom.type() != QgsWkbTypes::LineGeometry )
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
  if ( fGeom.type() != QgsWkbTypes::LineGeometry )
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
    else if ( QgsWkbTypes::flatType( center.constGet()->simplifiedTypeRef()->wkbType() ) != QgsWkbTypes::Point )
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
  else if ( QgsWkbTypes::flatType( center.constGet()->simplifiedTypeRef()->wkbType() ) != QgsWkbTypes::Point )
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
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( fGeom.isNull() )
    return QVariant();

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

static QVariant fcnProject( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry geom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );

  if ( ! geom.constGet() || QgsWkbTypes::flatType( geom.constGet()->simplifiedTypeRef( )->wkbType() ) != QgsWkbTypes::Type::Point )
  {
    parent->setEvalErrorString( QStringLiteral( "'project' requires a point geometry" ) );
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

  if ( ( fGeom1.type() != QgsWkbTypes::PointGeometry ) || ( fGeom2.type() != QgsWkbTypes::PointGeometry ) ||
       !pt1 || !pt2 )
  {
    parent->setEvalErrorString( QStringLiteral( "Function 'inclination' requires two points as arguments." ) );
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

static QVariant fcnLineSubset( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry lineGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  if ( lineGeom.type() != QgsWkbTypes::LineGeometry )
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

  QLocale locale = !language.isEmpty() ? QLocale( language ) : QLocale();
  locale.setNumberOptions( locale.numberOptions() &= ~QLocale::NumberOption::OmitGroupSeparator );
  return locale.toString( value, 'f', places );
}

static QVariant fcnFormatDate( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QDateTime datetime = QgsExpressionUtils::getDateTimeValue( values.at( 0 ), parent );
  const QString format = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  const QString language = QgsExpressionUtils::getStringValue( values.at( 2 ), parent );

  QLocale locale = !language.isEmpty() ? QLocale( language ) : QLocale();
  return locale.toString( datetime, format );
}

static QVariant fcnColorGrayscaleAverage( const QVariantList &values, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * )
{
  QColor color = QgsSymbolLayerUtils::decodeColor( values.at( 0 ).toString() );
  int avg = ( color.red() + color.green() + color.blue() ) / 3;
  int alpha = color.alpha();

  color.setRgb( avg, avg, avg, alpha );

  return QgsSymbolLayerUtils::encodeColor( color );
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

  return QStringLiteral( "%1,%2,%3" ).arg( color.red() ).arg( color.green() ).arg( color.blue() );
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

QVariant fcnRampColor( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGradientColorRamp expRamp;
  const QgsColorRamp *ramp = nullptr;
  if ( values.at( 0 ).canConvert<QgsGradientColorRamp>() )
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
  return QgsSymbolLayerUtils::encodeColor( color );
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

  return QStringLiteral( "%1,%2,%3" ).arg( color.red() ).arg( color.green() ).arg( color.blue() );
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

  return QStringLiteral( "%1,%2,%3" ).arg( color.red() ).arg( color.green() ).arg( color.blue() );
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

  return QStringLiteral( "%1,%2,%3" ).arg( color.red() ).arg( color.green() ).arg( color.blue() );
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
  QColor color = QgsSymbolLayerUtils::decodeColor( values.at( 0 ).toString() );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to color" ).arg( values.at( 0 ).toString() ) );
    return QVariant();
  }

  QString part = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
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

static QVariant fcnCreateRamp( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QVariantMap map = QgsExpressionUtils::getMapValue( values.at( 0 ), parent );
  if ( map.count() < 1 )
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

  return QVariant::fromValue( QgsGradientColorRamp( colors.first(), colors.last(), discrete, stops ) );
}

static QVariant fncSetColorPart( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QColor color = QgsSymbolLayerUtils::decodeColor( values.at( 0 ).toString() );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to color" ).arg( values.at( 0 ).toString() ) );
    return QVariant();
  }

  QString part = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  int value = QgsExpressionUtils::getNativeIntValue( values.at( 2 ), parent );
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

static QVariant fncDarker( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QColor color = QgsSymbolLayerUtils::decodeColor( values.at( 0 ).toString() );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to color" ).arg( values.at( 0 ).toString() ) );
    return QVariant();
  }

  color = color.darker( QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent ) );

  return QgsSymbolLayerUtils::encodeColor( color );
}

static QVariant fncLighter( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QColor color = QgsSymbolLayerUtils::decodeColor( values.at( 0 ).toString() );
  if ( ! color.isValid() )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to color" ).arg( values.at( 0 ).toString() ) );
    return QVariant();
  }

  color = color.lighter( QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent ) );

  return QgsSymbolLayerUtils::encodeColor( color );
}

static QVariant fcnGetGeometry( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsFeature feat = QgsExpressionUtils::getFeature( values.at( 0 ), parent );
  QgsGeometry geom = feat.geometry();
  if ( !geom.isNull() )
    return QVariant::fromValue( geom );
  return QVariant();
}

static QVariant fcnTransformGeometry( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsGeometry fGeom = QgsExpressionUtils::getGeometry( values.at( 0 ), parent );
  QString sAuthId = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  QString dAuthId = QgsExpressionUtils::getStringValue( values.at( 2 ), parent );

  QgsCoordinateReferenceSystem s = QgsCoordinateReferenceSystem::fromOgcWmsCrs( sAuthId );
  if ( ! s.isValid() )
    return QVariant::fromValue( fGeom );
  QgsCoordinateReferenceSystem d = QgsCoordinateReferenceSystem::fromOgcWmsCrs( dAuthId );
  if ( ! d.isValid() )
    return QVariant::fromValue( fGeom );

  QgsCoordinateTransformContext tContext;
  if ( context )
    tContext = context->variable( QStringLiteral( "_project_transform_context" ) ).value<QgsCoordinateTransformContext>();
  QgsCoordinateTransform t( s, d, tContext );
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
  QVariant result;
  QgsVectorLayer *vl = QgsExpressionUtils::getVectorLayer( values.at( 0 ), parent );
  if ( vl )
  {
    QgsFeatureId fid = QgsExpressionUtils::getIntValue( values.at( 1 ), parent );

    QgsFeatureRequest req;
    req.setFilterFid( fid );
    req.setTimeout( 10000 );
    req.setRequestMayBeNested( true );
    if ( context )
      req.setFeedback( context->feedback() );
    QgsFeatureIterator fIt = vl->getFeatures( req );

    QgsFeature fet;
    if ( fIt.nextFeature( fet ) )
      result = QVariant::fromValue( fet );
  }

  return result;
}

static QVariant fcnGetFeature( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  //arguments: 1. layer id / name, 2. key attribute, 3. eq value

  std::unique_ptr<QgsVectorLayerFeatureSource> featureSource = QgsExpressionUtils::getFeatureSource( values.at( 0 ), parent );

  //no layer found
  if ( !featureSource )
  {
    return QVariant();
  }
  QgsFeatureRequest req;
  QString cacheValueKey;
  if ( values.at( 1 ).type() == QVariant::Map )
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
    cacheValueKey = QStringLiteral( "getfeature:%1:%2" ).arg( featureSource->id(), filterString );
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

    cacheValueKey = QStringLiteral( "getfeature:%1:%2:%3" ).arg( featureSource->id(), QString::number( attributeId ), attVal.toString() );
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
    req.setFlags( QgsFeatureRequest::NoGeometry );
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
      parent->setEvalErrorString( QCoreApplication::translate( "expression", "%1: Field not found %2" ).arg( QStringLiteral( "represent_value" ), fieldName ) );
    }
    else
    {
      QgsVectorLayer *layer = QgsExpressionUtils::getVectorLayer( context->variable( QStringLiteral( "layer" ) ), parent );

      const QString cacheValueKey = QStringLiteral( "repvalfcnval:%1:%2:%3" ).arg( layer ? layer->id() : QStringLiteral( "[None]" ), fieldName, value.toString() );
      if ( context->hasCachedValue( cacheValueKey ) )
      {
        return context->cachedValue( cacheValueKey );
      }

      const QgsEditorWidgetSetup setup = fields.at( fieldIndex ).editorWidgetSetup();
      const QgsFieldFormatter *formatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );

      const QString cacheKey = QStringLiteral( "repvalfcn:%1:%2" ).arg( layer ? layer->id() : QStringLiteral( "[None]" ), fieldName );

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
    parent->setEvalErrorString( QCoreApplication::translate( "expression", "%1: function cannot be evaluated without a context." ).arg( QStringLiteral( "represent_value" ), fieldName ) );
  }

  return result;
}

static QVariant fcnMimeType( const QVariantList &values, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * )
{
  const QVariant data = values.at( 0 );
  const QMimeDatabase db;
  return db.mimeTypeForData( data.toByteArray() ).name();
}

static QVariant fcnGetLayerProperty( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsMapLayer *layer = QgsExpressionUtils::getMapLayer( values.at( 0 ), parent );

  if ( !layer )
    return QVariant();

  // here, we always prefer the layer metadata values over the older server-specific published values
  QString layerProperty = QgsExpressionUtils::getStringValue( values.at( 1 ), parent );
  if ( QString::compare( layerProperty, QStringLiteral( "name" ), Qt::CaseInsensitive ) == 0 )
    return layer->name();
  else if ( QString::compare( layerProperty, QStringLiteral( "id" ), Qt::CaseInsensitive ) == 0 )
    return layer->id();
  else if ( QString::compare( layerProperty, QStringLiteral( "title" ), Qt::CaseInsensitive ) == 0 )
    return !layer->metadata().title().isEmpty() ? layer->metadata().title() : layer->title();
  else if ( QString::compare( layerProperty, QStringLiteral( "abstract" ), Qt::CaseInsensitive ) == 0 )
    return !layer->metadata().abstract().isEmpty() ? layer->metadata().abstract() : layer->abstract();
  else if ( QString::compare( layerProperty, QStringLiteral( "keywords" ), Qt::CaseInsensitive ) == 0 )
  {
    QStringList keywords;
    const QgsAbstractMetadataBase::KeywordMap keywordMap = layer->metadata().keywords();
    for ( auto it = keywordMap.constBegin(); it != keywordMap.constEnd(); ++it )
    {
      keywords.append( it.value() );
    }
    if ( !keywords.isEmpty() )
      return keywords;
    return layer->keywordList();
  }
  else if ( QString::compare( layerProperty, QStringLiteral( "data_url" ), Qt::CaseInsensitive ) == 0 )
    return layer->dataUrl();
  else if ( QString::compare( layerProperty, QStringLiteral( "attribution" ), Qt::CaseInsensitive ) == 0 )
  {
    return !layer->metadata().rights().isEmpty() ? QVariant( layer->metadata().rights() ) : QVariant( layer->attribution() );
  }
  else if ( QString::compare( layerProperty, QStringLiteral( "attribution_url" ), Qt::CaseInsensitive ) == 0 )
    return layer->attributionUrl();
  else if ( QString::compare( layerProperty, QStringLiteral( "source" ), Qt::CaseInsensitive ) == 0 )
    return layer->publicSource();
  else if ( QString::compare( layerProperty, QStringLiteral( "min_scale" ), Qt::CaseInsensitive ) == 0 )
    return layer->minimumScale();
  else if ( QString::compare( layerProperty, QStringLiteral( "max_scale" ), Qt::CaseInsensitive ) == 0 )
    return layer->maximumScale();
  else if ( QString::compare( layerProperty, QStringLiteral( "is_editable" ), Qt::CaseInsensitive ) == 0 )
    return layer->isEditable();
  else if ( QString::compare( layerProperty, QStringLiteral( "crs" ), Qt::CaseInsensitive ) == 0 )
    return layer->crs().authid();
  else if ( QString::compare( layerProperty, QStringLiteral( "crs_definition" ), Qt::CaseInsensitive ) == 0 )
    return layer->crs().toProj();
  else if ( QString::compare( layerProperty, QStringLiteral( "crs_description" ), Qt::CaseInsensitive ) == 0 )
    return layer->crs().description();
  else if ( QString::compare( layerProperty, QStringLiteral( "extent" ), Qt::CaseInsensitive ) == 0 )
  {
    QgsGeometry extentGeom = QgsGeometry::fromRect( layer->extent() );
    QVariant result = QVariant::fromValue( extentGeom );
    return result;
  }
  else if ( QString::compare( layerProperty, QStringLiteral( "distance_units" ), Qt::CaseInsensitive ) == 0 )
    return QgsUnitTypes::encodeUnit( layer->crs().mapUnits() );
  else if ( QString::compare( layerProperty, QStringLiteral( "type" ), Qt::CaseInsensitive ) == 0 )
  {
    switch ( layer->type() )
    {
      case QgsMapLayerType::VectorLayer:
        return QCoreApplication::translate( "expressions", "Vector" );
      case QgsMapLayerType::RasterLayer:
        return QCoreApplication::translate( "expressions", "Raster" );
      case QgsMapLayerType::MeshLayer:
        return QCoreApplication::translate( "expressions", "Mesh" );
      case QgsMapLayerType::VectorTileLayer:
        return QCoreApplication::translate( "expressions", "Vector Tile" );
      case QgsMapLayerType::PluginLayer:
        return QCoreApplication::translate( "expressions", "Plugin" );
      case QgsMapLayerType::AnnotationLayer:
        return QCoreApplication::translate( "expressions", "Annotation" );
      case QgsMapLayerType::PointCloudLayer:
        return QCoreApplication::translate( "expressions", "Point Cloud" );
      case QgsMapLayerType::GroupLayer:
        return QCoreApplication::translate( "expressions", "Group" );
    }
  }
  else
  {
    //vector layer methods
    QgsVectorLayer *vLayer = qobject_cast< QgsVectorLayer * >( layer );
    if ( vLayer )
    {
      if ( QString::compare( layerProperty, QStringLiteral( "storage_type" ), Qt::CaseInsensitive ) == 0 )
        return vLayer->storageType();
      else if ( QString::compare( layerProperty, QStringLiteral( "geometry_type" ), Qt::CaseInsensitive ) == 0 )
        return QgsWkbTypes::geometryDisplayString( vLayer->geometryType() );
      else if ( QString::compare( layerProperty, QStringLiteral( "feature_count" ), Qt::CaseInsensitive ) == 0 )
        return QVariant::fromValue( vLayer->featureCount() );
      else if ( QString::compare( layerProperty, QStringLiteral( "path" ), Qt::CaseInsensitive ) == 0 )
      {
        if ( vLayer->dataProvider() )
        {
          const QVariantMap decodedUri = QgsProviderRegistry::instance()->decodeUri( layer->providerType(), layer->dataProvider()->dataSourceUri() );
          return decodedUri.value( QStringLiteral( "path" ) );
        }
      }
    }
  }

  return QVariant();
}

static QVariant fcnDecodeUri( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QgsMapLayer *layer = QgsExpressionUtils::getMapLayer( values.at( 0 ), parent );
  if ( !layer )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot find layer %1" ).arg( values.at( 0 ).toString() ) );
    return QVariant();
  }

  if ( !layer->dataProvider() )
  {
    parent->setEvalErrorString( QObject::tr( "Layer %1 has invalid data provider" ).arg( layer->name() ) );
    return QVariant();
  }

  const QString uriPart = values.at( 1 ).toString();

  const QVariantMap decodedUri = QgsProviderRegistry::instance()->decodeUri( layer->providerType(), layer->dataProvider()->dataSourceUri() );

  if ( !uriPart.isNull() )
  {
    return decodedUri.value( values.at( 1 ).toString() );
  }
  else
  {
    return decodedUri;
  }
}

static QVariant fcnGetRasterBandStat( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QString layerIdOrName = QgsExpressionUtils::getStringValue( values.at( 0 ), parent );

  //try to find a matching layer by name
  QgsMapLayer *layer = QgsProject::instance()->mapLayer( layerIdOrName ); //search by id first
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

  QgsRasterLayer *rl = qobject_cast< QgsRasterLayer * >( layer );
  if ( !rl )
    return QVariant();

  int band = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  if ( band < 1 || band > rl->bandCount() )
  {
    parent->setEvalErrorString( QObject::tr( "Invalid band number %1 for layer %2" ).arg( band ).arg( layerIdOrName ) );
    return QVariant();
  }

  QString layerProperty = QgsExpressionUtils::getStringValue( values.at( 2 ), parent );
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

static QVariant convertToSameType( const QVariant &value, QVariant::Type type )
{
  QVariant result = value;
  result.convert( static_cast<int>( type ) );
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
  if ( option.compare( QLatin1String( "all" ), Qt::CaseInsensitive ) == 0 )
  {
    return convertToSameType( hash.keys( maxValue ), values.at( 0 ).type() );
  }
  else if ( option.compare( QLatin1String( "any" ), Qt::CaseInsensitive ) == 0 )
  {
    if ( hash.isEmpty() )
      return QVariant();

    return QVariant( hash.keys( maxValue ).first() );
  }
  else if ( option.compare( QLatin1String( "median" ), Qt::CaseInsensitive ) == 0 )
  {
    return fcnArrayMedian( QVariantList() << convertToSameType( hash.keys( maxValue ), values.at( 0 ).type() ), context, parent, node );
  }
  else if ( option.compare( QLatin1String( "real_majority" ), Qt::CaseInsensitive ) == 0 )
  {
    if ( maxValue * 2 <= list.size() )
      return QVariant();

    return QVariant( hash.keys( maxValue ).first() );
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
  if ( option.compare( QLatin1String( "all" ), Qt::CaseInsensitive ) == 0 )
  {
    return convertToSameType( hash.keys( minValue ), values.at( 0 ).type() );
  }
  else if ( option.compare( QLatin1String( "any" ), Qt::CaseInsensitive ) == 0 )
  {
    if ( hash.isEmpty() )
      return QVariant();

    return QVariant( hash.keys( minValue ).first() );
  }
  else if ( option.compare( QLatin1String( "median" ), Qt::CaseInsensitive ) == 0 )
  {
    return fcnArrayMedian( QVariantList() << convertToSameType( hash.keys( minValue ), values.at( 0 ).type() ), context, parent, node );
  }
  else if ( option.compare( QLatin1String( "real_minority" ), Qt::CaseInsensitive ) == 0 )
  {
    if ( hash.keys().isEmpty() )
      return QVariant();

    // Remove the majority, all others are minority
    const int maxValue = *std::max_element( occurrences.constBegin(), occurrences.constEnd() );
    if ( maxValue * 2 > list.size() )
      hash.remove( hash.key( maxValue ) );

    return convertToSameType( hash.keys(), values.at( 0 ).type() );
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
  return convertToSameType( list, values.at( 0 ).type() );
}

static QVariant fcnArrayPrepend( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  list.prepend( values.at( 1 ) );
  return convertToSameType( list, values.at( 0 ).type() );
}

static QVariant fcnArrayInsert( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  list.insert( QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent ), values.at( 2 ) );
  return convertToSameType( list, values.at( 0 ).type() );
}

static QVariant fcnArrayRemoveAt( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  int position = QgsExpressionUtils::getNativeIntValue( values.at( 1 ), parent );
  if ( position < 0 )
    position = position + list.length();
  if ( position >= 0 && position < list.length() )
    list.removeAt( position );
  return convertToSameType( list, values.at( 0 ).type() );
}

static QVariant fcnArrayRemoveAll( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantList list = QgsExpressionUtils::getListValue( values.at( 0 ), parent );
  list.removeAll( values.at( 1 ) );
  return convertToSameType( list, values.at( 0 ).type() );
}

static QVariant fcnArrayReplace( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( values.count() == 2 && values.at( 1 ).type() == QVariant::Map )
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

    return convertToSameType( list, values.at( 0 ).type() );
  }
  else if ( values.count() == 3 )
  {
    QVariantList before;
    QVariantList after;
    bool isSingleReplacement = false;

    if ( !QgsExpressionUtils::isList( values.at( 1 ) ) && values.at( 2 ).type() != QVariant::StringList )
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

    return convertToSameType( list, values.at( 0 ).type() );
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

  return convertToSameType( list_new, values.at( 0 ).type() );
}

static QVariant fcnArrayCat( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  QVariantList list;
  for ( const QVariant &cur : values )
  {
    list += QgsExpressionUtils::getListValue( cur, parent );
  }
  return convertToSameType( list, values.at( 0 ).type() );
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
  return document.toJson( QJsonDocument::Compact );
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
  QString envVarName = values.at( 0 ).toString();
  return QProcessEnvironment::systemEnvironment().value( envVarName );
}

static QVariant fcnBaseFileName( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString file = QgsExpressionUtils::getFilePathValue( values.at( 0 ), parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( QStringLiteral( "base_file_name" ) ) );
    return QVariant();
  }
  return QFileInfo( file ).completeBaseName();
}

static QVariant fcnFileSuffix( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString file = QgsExpressionUtils::getFilePathValue( values.at( 0 ), parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( QStringLiteral( "file_suffix" ) ) );
    return QVariant();
  }
  return QFileInfo( file ).completeSuffix();
}

static QVariant fcnFileExists( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString file = QgsExpressionUtils::getFilePathValue( values.at( 0 ), parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( QStringLiteral( "file_exists" ) ) );
    return QVariant();
  }
  return QFileInfo::exists( file );
}

static QVariant fcnFileName( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString file = QgsExpressionUtils::getFilePathValue( values.at( 0 ), parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( QStringLiteral( "file_name" ) ) );
    return QVariant();
  }
  return QFileInfo( file ).fileName();
}

static QVariant fcnPathIsFile( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString file = QgsExpressionUtils::getFilePathValue( values.at( 0 ), parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( QStringLiteral( "is_file" ) ) );
    return QVariant();
  }
  return QFileInfo( file ).isFile();
}

static QVariant fcnPathIsDir( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString file = QgsExpressionUtils::getFilePathValue( values.at( 0 ), parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( QStringLiteral( "is_directory" ) ) );
    return QVariant();
  }
  return QFileInfo( file ).isDir();
}

static QVariant fcnFilePath( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString file = QgsExpressionUtils::getFilePathValue( values.at( 0 ), parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( QStringLiteral( "file_path" ) ) );
    return QVariant();
  }
  return QDir::toNativeSeparators( QFileInfo( file ).path() );
}

static QVariant fcnFileSize( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  const QString file = QgsExpressionUtils::getFilePathValue( values.at( 0 ), parent );
  if ( parent->hasEvalError() )
  {
    parent->setEvalErrorString( QObject::tr( "Function `%1` requires a value which represents a possible file path" ).arg( QStringLiteral( "file_size" ) ) );
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

  if ( method == QLatin1String( "md4" ) )
  {
    hash = fcnHash( str, QCryptographicHash::Md4 );
  }
  else if ( method == QLatin1String( "md5" ) )
  {
    hash = fcnHash( str, QCryptographicHash::Md5 );
  }
  else if ( method == QLatin1String( "sha1" ) )
  {
    hash = fcnHash( str, QCryptographicHash::Sha1 );
  }
  else if ( method == QLatin1String( "sha224" ) )
  {
    hash = fcnHash( str, QCryptographicHash::Sha224 );
  }
  else if ( method == QLatin1String( "sha256" ) )
  {
    hash = fcnHash( str, QCryptographicHash::Sha256 );
  }
  else if ( method == QLatin1String( "sha384" ) )
  {
    hash = fcnHash( str, QCryptographicHash::Sha384 );
  }
  else if ( method == QLatin1String( "sha512" ) )
  {
    hash = fcnHash( str, QCryptographicHash::Sha512 );
  }
  else if ( method == QLatin1String( "sha3_224" ) )
  {
    hash = fcnHash( str, QCryptographicHash::Sha3_224 );
  }
  else if ( method == QLatin1String( "sha3_256" ) )
  {
    hash = fcnHash( str, QCryptographicHash::Sha3_256 );
  }
  else if ( method == QLatin1String( "sha3_384" ) )
  {
    hash = fcnHash( str, QCryptographicHash::Sha3_384 );
  }
  else if ( method == QLatin1String( "sha3_512" ) )
  {
    hash = fcnHash( str, QCryptographicHash::Sha3_512 );
  }
  else if ( method == QLatin1String( "keccak_224" ) )
  {
    hash = fcnHash( str, QCryptographicHash::Keccak_224 );
  }
  else if ( method == QLatin1String( "keccak_256" ) )
  {
    hash = fcnHash( str, QCryptographicHash::Keccak_256 );
  }
  else if ( method == QLatin1String( "keccak_384" ) )
  {
    hash = fcnHash( str, QCryptographicHash::Keccak_384 );
  }
  else if ( method == QLatin1String( "keccak_512" ) )
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

  const QVariant sourceLayerRef = context->variable( QStringLiteral( "layer" ) ); //used to detect if sourceLayer and targetLayer are the same
  QgsVectorLayer *sourceLayer = QgsExpressionUtils::getVectorLayer( sourceLayerRef, parent );

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
  QgsVectorLayer *targetLayer = QgsExpressionUtils::getVectorLayer( targetLayerValue, parent );
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
#if GEOS_VERSION_MAJOR==3 && GEOS_VERSION_MINOR<9
    if ( minInscribedCircleRadiusValue != -1 )
    {
      parent->setEvalErrorString( QObject::tr( "'min_inscribed_circle_radius' is only available when QGIS is built with GEOS >= 3.9." ) );
      return QVariant();
    }
#endif
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
    QgsCoordinateTransformContext TransformContext = context->variable( QStringLiteral( "_project_transform_context" ) ).value<QgsCoordinateTransformContext>();
    request.setDestinationCrs( sourceLayer->crs(), TransformContext ); //if crs are not the same, cached target will be reprojected to source crs
  }

  bool sameLayers = ( sourceLayer && sourceLayer->id() == targetLayer->id() );

  QgsRectangle intDomain = geometry.boundingBox();
  if ( bboxGrow != 0 )
  {
    intDomain.grow( bboxGrow ); //optional parameter to enlarge boundary context for touches and equals methods
  }

  const QString cacheBase { QStringLiteral( "%1:%2:%3" ).arg( targetLayer->id(), subExpString, filterString ) };

  // Cache (a local spatial index) is always enabled for nearest function (as we need QgsSpatialIndex::nearestNeighbor)
  // Otherwise, it can be toggled by the user
  QgsSpatialIndex spatialIndex;
  QgsVectorLayer *cachedTarget;
  QList<QgsFeature> features;
  if ( isNearestFunc || ( layerCanBeCached && cacheEnabled ) )
  {
    // If the cache (local spatial index) is enabled, we materialize the whole
    // layer, then do the request on that layer instead.
    const QString cacheLayer { QStringLiteral( "ovrlaylyr:%1" ).arg( cacheBase ) };
    const QString cacheIndex { QStringLiteral( "ovrlayidx:%1" ).arg( cacheBase ) };

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
    const QString expCacheKey { QStringLiteral( "exp:%1" ).arg( cacheBase ) };
    const QString ctxCacheKey { QStringLiteral( "ctx:%1" ).arg( cacheBase ) };

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
        const QgsGeometry intersection { geometry.intersection( feat2.geometry() ) };

        // Depending on the intersection geometry type and on the geometry type of
        // the tested geometry we can run different tests and collect different measures
        // that can be used for sorting (if required).
        switch ( intersection.type() )
        {

          case QgsWkbTypes::GeometryType::PolygonGeometry:
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

#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=9 )
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
#endif
            }

            // Get the max values
            if ( !radiusValues.isEmpty() )
            {
              radiusValue = *std::max_element( radiusValues.cbegin(), radiusValues.cend() );
            }

            if ( ! overlapValues.isEmpty() )
            {
              overlapValue = *std::max_element( overlapValues.cbegin(), overlapValues.cend() );
            }

            if ( ! testResult && overlapOrRadiusFilter )
            {
              continue;
            }

            break;
          }
          case QgsWkbTypes::GeometryType::LineGeometry:
          {
            bool testResult { false };
            // For return measures:
            QVector<double> overlapValues;
            for ( auto it = intersection.const_parts_begin(); ! testResult && it != intersection.const_parts_end(); ++it )
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

            if ( ! testResult && overlapOrRadiusFilter )
            {
              continue;
            }

            break;
          }
          case QgsWkbTypes::GeometryType::PointGeometry:
          {
            if ( minOverlap != -1  || requireMeasures )
            {
              // Initially set this to 0 because it's a point intersection...
              overlapValue = 0;
              // ... but if the target geometry is not a point and the source
              // geometry is a point, we must record the length or the area
              // of the intersected geometry and use that as a measure for
              // sorting or reporting.
              if ( geometry.type() == QgsWkbTypes::GeometryType::PointGeometry )
              {
                switch ( feat2.geometry().type() )
                {
                  case QgsWkbTypes::GeometryType::UnknownGeometry:
                  case QgsWkbTypes::GeometryType::NullGeometry:
                  case QgsWkbTypes::GeometryType::PointGeometry:
                  {
                    break;
                  }
                  case QgsWkbTypes::GeometryType::LineGeometry:
                  {
                    overlapValue = feat2.geometry().length();
                    break;
                  }
                  case QgsWkbTypes::GeometryType::PolygonGeometry:
                  {
                    overlapValue = feat2.geometry().area();
                    break;
                  }
                }
              }

              if ( minOverlap != -1 && overlapValue < minOverlap )
              {
                continue;
              }

            }
            break;
          }
          case QgsWkbTypes::GeometryType::NullGeometry:
          case QgsWkbTypes::GeometryType::UnknownGeometry:
          {
            break;
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
          resultRecord.insert( QStringLiteral( "id" ), feat2.id() );
          resultRecord.insert( QStringLiteral( "result" ), expResult );
          // Overlap is always added because return measures was set
          resultRecord.insert( QStringLiteral( "overlap" ), overlapValue );
          // Radius is only added when is different than -1 (because for linestrings is not set)
          if ( radiusValue != -1 )
          {
            resultRecord.insert( QStringLiteral( "radius" ), radiusValue );
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
          recordB.toMap().value( QStringLiteral( "overlap" ) ).toDouble() > recordA.toMap().value( QStringLiteral( "overlap" ) ).toDouble()
          : recordA.toMap().value( QStringLiteral( "overlap" ) ).toDouble() > recordB.toMap().value( QStringLiteral( "overlap" ) ).toDouble();
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
          expResults.append( it->toMap().value( QStringLiteral( "result" ) ) );
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
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  static QMutex sFunctionsMutex( QMutex::Recursive );
  QMutexLocker locker( &sFunctionsMutex );
#else
  static QRecursiveMutex sFunctionsMutex;
  QMutexLocker locker( &sFunctionsMutex );
#endif

  QList<QgsExpressionFunction *> &functions = *sFunctions();

  if ( functions.isEmpty() )
  {
    QgsExpressionFunction::ParameterList aggParams = QgsExpressionFunction::ParameterList()
        << QgsExpressionFunction::Parameter( QStringLiteral( "expression" ) )
        << QgsExpressionFunction::Parameter( QStringLiteral( "group_by" ), true )
        << QgsExpressionFunction::Parameter( QStringLiteral( "filter" ), true );

    QgsExpressionFunction::ParameterList aggParamsConcat = aggParams;
    aggParamsConcat <<  QgsExpressionFunction::Parameter( QStringLiteral( "concatenator" ), true )
                    << QgsExpressionFunction::Parameter( QStringLiteral( "order_by" ), true, QVariant(), true );

    QgsExpressionFunction::ParameterList aggParamsArray = aggParams;
    aggParamsArray << QgsExpressionFunction::Parameter( QStringLiteral( "order_by" ), true, QVariant(), true );

    functions
        << new QgsStaticExpressionFunction( QStringLiteral( "sqrt" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnSqrt, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "radians" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "degrees" ) ), fcnRadians, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "degrees" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "radians" ) ), fcnDegrees, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "azimuth" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "point_a" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "point_b" ) ), fcnAzimuth, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "inclination" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "point_a" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "point_b" ) ), fcnInclination, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "project" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "point" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "distance" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "azimuth" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "elevation" ), true, M_PI_2 ), fcnProject, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "abs" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnAbs, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "cos" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "angle" ) ), fcnCos, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "sin" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "angle" ) ), fcnSin, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "tan" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "angle" ) ), fcnTan, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "asin" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnAsin, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "acos" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnAcos, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "atan" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnAtan, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "atan2" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "dx" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "dy" ) ), fcnAtan2, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "exp" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnExp, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "ln" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnLn, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "log10" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnLog10, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "log" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "base" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnLog, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "round" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "places" ), true, 0 ), fcnRound, QStringLiteral( "Math" ) );

    QgsStaticExpressionFunction *randFunc = new QgsStaticExpressionFunction( QStringLiteral( "rand" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "min" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "max" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "seed" ), true ), fcnRnd, QStringLiteral( "Math" ) );
    randFunc->setIsStatic( false );
    functions << randFunc;

    QgsStaticExpressionFunction *randfFunc = new QgsStaticExpressionFunction( QStringLiteral( "randf" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "min" ), true, 0.0 ) << QgsExpressionFunction::Parameter( QStringLiteral( "max" ), true, 1.0 ) << QgsExpressionFunction::Parameter( QStringLiteral( "seed" ), true ), fcnRndF, QStringLiteral( "Math" ) );
    randfFunc->setIsStatic( false );
    functions << randfFunc;

    functions
        << new QgsStaticExpressionFunction( QStringLiteral( "max" ), -1, fcnMax, QStringLiteral( "Math" ), QString(), false, QSet<QString>(), false, QStringList(), /* handlesNull = */ true )
        << new QgsStaticExpressionFunction( QStringLiteral( "min" ), -1, fcnMin, QStringLiteral( "Math" ), QString(), false, QSet<QString>(), false, QStringList(), /* handlesNull = */ true )
        << new QgsStaticExpressionFunction( QStringLiteral( "clamp" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "min" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "max" ) ), fcnClamp, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "scale_linear" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) )  << QgsExpressionFunction::Parameter( QStringLiteral( "domain_min" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "domain_max" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "range_min" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "range_max" ) ), fcnLinearScale, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "scale_exp" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) )  << QgsExpressionFunction::Parameter( QStringLiteral( "domain_min" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "domain_max" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "range_min" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "range_max" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "exponent" ) ), fcnExpScale, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "floor" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnFloor, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "ceil" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnCeil, QStringLiteral( "Math" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "pi" ), 0, fcnPi, QStringLiteral( "Math" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "$pi" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "to_int" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnToInt, QStringLiteral( "Conversions" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "toint" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "to_real" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnToReal, QStringLiteral( "Conversions" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "toreal" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "to_string" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnToString, QStringList() << QStringLiteral( "Conversions" ) << QStringLiteral( "String" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "tostring" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "to_datetime" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "format" ), true, QVariant() ) << QgsExpressionFunction::Parameter( QStringLiteral( "language" ), true, QVariant() ), fcnToDateTime, QStringList() << QStringLiteral( "Conversions" ) << QStringLiteral( "Date and Time" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "todatetime" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "to_date" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "format" ), true, QVariant() ) << QgsExpressionFunction::Parameter( QStringLiteral( "language" ), true, QVariant() ), fcnToDate, QStringList() << QStringLiteral( "Conversions" ) << QStringLiteral( "Date and Time" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "todate" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "to_time" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "format" ), true, QVariant() ) << QgsExpressionFunction::Parameter( QStringLiteral( "language" ), true, QVariant() ), fcnToTime, QStringList() << QStringLiteral( "Conversions" ) << QStringLiteral( "Date and Time" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "totime" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "to_interval" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnToInterval, QStringList() << QStringLiteral( "Conversions" ) << QStringLiteral( "Date and Time" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "tointerval" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "to_dm" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "axis" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "precision" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "formatting" ), true ), fcnToDegreeMinute, QStringLiteral( "Conversions" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "todm" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "to_dms" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "axis" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "precision" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "formatting" ), true ), fcnToDegreeMinuteSecond, QStringLiteral( "Conversions" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "todms" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "to_decimal" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnToDecimal, QStringLiteral( "Conversions" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "todecimal" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "coalesce" ), -1, fcnCoalesce, QStringLiteral( "Conditionals" ), QString(), false, QSet<QString>(), false, QStringList(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "nullif" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value1" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "value2" ) ), fcnNullIf, QStringLiteral( "Conditionals" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "if" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "condition" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "result_when_true" ) )  << QgsExpressionFunction::Parameter( QStringLiteral( "result_when_false" ) ), fcnIf, QStringLiteral( "Conditionals" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "try" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "expression" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "alternative" ), true, QVariant() ), fcnTry, QStringLiteral( "Conditionals" ), QString(), false, QSet<QString>(), true )

        << new QgsStaticExpressionFunction( QStringLiteral( "aggregate" ),
                                            QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "layer" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "aggregate" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "expression" ), false, QVariant(), true )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "filter" ), true, QVariant(), true )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "concatenator" ), true )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "order_by" ), true, QVariant(), true ),
                                            fcnAggregate,
                                            QStringLiteral( "Aggregates" ),
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
      return referencedVars.contains( QStringLiteral( "parent" ) ) || referencedVars.contains( QString() );
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

      if ( referencedVars.contains( QStringLiteral( "parent" ) ) || referencedVars.contains( QString() ) )
        return QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES;
      else
        return referencedCols;
    },
    true
                                          )

        << new QgsStaticExpressionFunction( QStringLiteral( "relation_aggregate" ), QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "relation" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "aggregate" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "expression" ), false, QVariant(), true )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "concatenator" ), true )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "order_by" ), true, QVariant(), true ),
                                            fcnAggregateRelation, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES, true )

        << new QgsStaticExpressionFunction( QStringLiteral( "count" ), aggParams, fcnAggregateCount, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "count_distinct" ), aggParams, fcnAggregateCountDistinct, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "count_missing" ), aggParams, fcnAggregateCountMissing, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "minimum" ), aggParams, fcnAggregateMin, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "maximum" ), aggParams, fcnAggregateMax, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "sum" ), aggParams, fcnAggregateSum, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "mean" ), aggParams, fcnAggregateMean, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "median" ), aggParams, fcnAggregateMedian, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "stdev" ), aggParams, fcnAggregateStdev, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "range" ), aggParams, fcnAggregateRange, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "minority" ), aggParams, fcnAggregateMinority, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "majority" ), aggParams, fcnAggregateMajority, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "q1" ), aggParams, fcnAggregateQ1, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "q3" ), aggParams, fcnAggregateQ3, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "iqr" ), aggParams, fcnAggregateIQR, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "min_length" ), aggParams, fcnAggregateMinLength, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "max_length" ), aggParams, fcnAggregateMaxLength, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "collect" ), aggParams, fcnAggregateCollectGeometry, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "concatenate" ), aggParamsConcat, fcnAggregateStringConcat, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "concatenate_unique" ), aggParamsConcat, fcnAggregateStringConcatUnique, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_agg" ), aggParamsArray, fcnAggregateArray, QStringLiteral( "Aggregates" ), QString(), false, QSet<QString>(), true )

        << new QgsStaticExpressionFunction( QStringLiteral( "regexp_match" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "regex" ) ), fcnRegexpMatch, QStringList() << QStringLiteral( "Conditionals" ) << QStringLiteral( "String" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "regexp_matches" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "regex" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "emptyvalue" ), true, "" ), fcnRegexpMatches, QStringLiteral( "Arrays" ) )

        << new QgsStaticExpressionFunction( QStringLiteral( "now" ), 0, fcnNow, QStringLiteral( "Date and Time" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "$now" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "age" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "datetime1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "datetime2" ) ),
                                            fcnAge, QStringLiteral( "Date and Time" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "year" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "date" ) ), fcnYear, QStringLiteral( "Date and Time" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "month" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "date" ) ), fcnMonth, QStringLiteral( "Date and Time" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "week" ),  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "date" ) ), fcnWeek, QStringLiteral( "Date and Time" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "day" ),  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "date" ) ), fcnDay, QStringLiteral( "Date and Time" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "hour" ),  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "datetime" ) ), fcnHour, QStringLiteral( "Date and Time" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "minute" ),  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "datetime" ) ), fcnMinute, QStringLiteral( "Date and Time" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "second" ),  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "datetime" ) ), fcnSeconds, QStringLiteral( "Date and Time" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "epoch" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "date" ) ), fcnEpoch, QStringLiteral( "Date and Time" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "datetime_from_epoch" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "long" ) ), fcnDateTimeFromEpoch, QStringLiteral( "Date and Time" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "day_of_week" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "date" ) ), fcnDayOfWeek, QStringLiteral( "Date and Time" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "make_date" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "year" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "month" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "day" ) ),
                                            fcnMakeDate, QStringLiteral( "Date and Time" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "make_time" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "hour" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "minute" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "second" ) ),
                                            fcnMakeTime, QStringLiteral( "Date and Time" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "make_datetime" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "year" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "month" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "day" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "hour" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "minute" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "second" ) ),
                                            fcnMakeDateTime, QStringLiteral( "Date and Time" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "make_interval" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "years" ), true, 0 )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "months" ), true, 0 )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "weeks" ), true, 0 )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "days" ), true, 0 )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "hours" ), true, 0 )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "minutes" ), true, 0 )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "seconds" ), true, 0 ),
                                            fcnMakeInterval, QStringLiteral( "Date and Time" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "lower" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string" ) ), fcnLower, QStringLiteral( "String" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "upper" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string" ) ), fcnUpper, QStringLiteral( "String" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "title" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string" ) ), fcnTitle, QStringLiteral( "String" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "trim" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string" ) ), fcnTrim, QStringLiteral( "String" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "levenshtein" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string1" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "string2" ) ), fcnLevenshtein, QStringLiteral( "Fuzzy Matching" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "longest_common_substring" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string1" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "string2" ) ), fcnLCS, QStringLiteral( "Fuzzy Matching" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "hamming_distance" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string1" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "string2" ) ), fcnHamming, QStringLiteral( "Fuzzy Matching" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "soundex" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string" ) ), fcnSoundex, QStringLiteral( "Fuzzy Matching" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "char" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "code" ) ), fcnChar, QStringLiteral( "String" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "ascii" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string" ) ), fcnAscii, QStringLiteral( "String" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "wordwrap" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "text" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "length" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "delimiter" ), true, "" ), fcnWordwrap, QStringLiteral( "String" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "length" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "text" ), true, "" ), fcnLength, QStringList() << QStringLiteral( "String" ) << QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "length3D" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnLength3D, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "replace" ), -1, fcnReplace, QStringLiteral( "String" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "regexp_replace" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "input_string" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "regex" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "replacement" ) ), fcnRegexpReplace, QStringLiteral( "String" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "regexp_substr" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "input_string" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "regex" ) ), fcnRegexpSubstr, QStringLiteral( "String" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "substr" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "start" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "length" ), true ), fcnSubstr, QStringLiteral( "String" ), QString(),
                                            false, QSet< QString >(), false, QStringList(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "concat" ), -1, fcnConcat, QStringLiteral( "String" ), QString(), false, QSet<QString>(), false, QStringList(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "strpos" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "haystack" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "needle" ) ), fcnStrpos, QStringLiteral( "String" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "left" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "length" ) ), fcnLeft, QStringLiteral( "String" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "right" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "length" ) ), fcnRight, QStringLiteral( "String" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "rpad" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "width" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "fill" ) ), fcnRPad, QStringLiteral( "String" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "lpad" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "width" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "fill" ) ), fcnLPad, QStringLiteral( "String" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "format" ), -1, fcnFormatString, QStringLiteral( "String" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "format_number" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "number" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "places" ), true, 0 ) << QgsExpressionFunction::Parameter( QStringLiteral( "language" ), true, QVariant() ), fcnFormatNumber, QStringLiteral( "String" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "format_date" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "datetime" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "format" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "language" ), true, QVariant() ), fcnFormatDate, QStringList() << QStringLiteral( "String" ) << QStringLiteral( "Date and Time" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "color_grayscale_average" ),  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "color" ) ), fcnColorGrayscaleAverage, QStringLiteral( "Color" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "color_mix_rgb" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "color1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "color2" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "ratio" ) ),
                                            fcnColorMixRgb, QStringLiteral( "Color" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "color_rgb" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "red" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "green" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "blue" ) ),
                                            fcnColorRgb, QStringLiteral( "Color" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "color_rgba" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "red" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "green" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "blue" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "alpha" ) ),
                                            fncColorRgba, QStringLiteral( "Color" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "ramp_color" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "ramp_name" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ),
                                            fcnRampColor, QStringLiteral( "Color" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "create_ramp" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "map" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "discrete" ), true, false ),
                                            fcnCreateRamp, QStringLiteral( "Color" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "color_hsl" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "hue" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "saturation" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "lightness" ) ),
                                            fcnColorHsl, QStringLiteral( "Color" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "color_hsla" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "hue" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "saturation" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "lightness" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "alpha" ) ),
                                            fncColorHsla, QStringLiteral( "Color" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "color_hsv" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "hue" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "saturation" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ),
                                            fcnColorHsv, QStringLiteral( "Color" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "color_hsva" ),  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "hue" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "saturation" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "alpha" ) ),
                                            fncColorHsva, QStringLiteral( "Color" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "color_cmyk" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "cyan" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "magenta" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "yellow" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "black" ) ),
                                            fcnColorCmyk, QStringLiteral( "Color" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "color_cmyka" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "cyan" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "magenta" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "yellow" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "black" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "alpha" ) ),
                                            fncColorCmyka, QStringLiteral( "Color" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "color_part" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "color" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "component" ) ),
                                            fncColorPart, QStringLiteral( "Color" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "darker" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "color" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "factor" ) ),
                                            fncDarker, QStringLiteral( "Color" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "lighter" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "color" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "factor" ) ),
                                            fncLighter, QStringLiteral( "Color" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "set_color_part" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "color" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "component" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fncSetColorPart, QStringLiteral( "Color" ) )

        // file info
        << new QgsStaticExpressionFunction( QStringLiteral( "base_file_name" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "path" ) ),
                                            fcnBaseFileName, QStringLiteral( "Files and Paths" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "file_suffix" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "path" ) ),
                                            fcnFileSuffix, QStringLiteral( "Files and Paths" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "file_exists" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "path" ) ),
                                            fcnFileExists, QStringLiteral( "Files and Paths" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "file_name" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "path" ) ),
                                            fcnFileName, QStringLiteral( "Files and Paths" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "is_file" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "path" ) ),
                                            fcnPathIsFile, QStringLiteral( "Files and Paths" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "is_directory" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "path" ) ),
                                            fcnPathIsDir, QStringLiteral( "Files and Paths" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "file_path" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "path" ) ),
                                            fcnFilePath, QStringLiteral( "Files and Paths" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "file_size" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "path" ) ),
                                            fcnFileSize, QStringLiteral( "Files and Paths" ) )

        << new QgsStaticExpressionFunction( QStringLiteral( "exif" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "path" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "tag" ), true ),
                                            fcnExif, QStringLiteral( "Files and Paths" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "exif_geotag" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "path" ) ),
                                            fcnExifGeoTag, QStringLiteral( "GeometryGroup" ) )

        // hash
        << new QgsStaticExpressionFunction( QStringLiteral( "hash" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "method" ) ),
                                            fcnGenericHash, QStringLiteral( "Conversions" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "md5" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string" ) ),
                                            fcnHashMd5, QStringLiteral( "Conversions" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "sha256" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string" ) ),
                                            fcnHashSha256, QStringLiteral( "Conversions" ) )

        //base64
        << new QgsStaticExpressionFunction( QStringLiteral( "to_base64" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ),
                                            fcnToBase64, QStringLiteral( "Conversions" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "from_base64" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string" ) ),
                                            fcnFromBase64, QStringLiteral( "Conversions" ) )

        // deprecated stuff - hidden from users
        << new QgsStaticExpressionFunction( QStringLiteral( "$scale" ), QgsExpressionFunction::ParameterList(), fcnMapScale, QStringLiteral( "deprecated" ) );

    QgsStaticExpressionFunction *geomFunc = new QgsStaticExpressionFunction( QStringLiteral( "$geometry" ), 0, fcnGeometry, QStringLiteral( "GeometryGroup" ), QString(), true );
    geomFunc->setIsStatic( false );
    functions << geomFunc;

    QgsStaticExpressionFunction *areaFunc = new QgsStaticExpressionFunction( QStringLiteral( "$area" ), 0, fcnGeomArea, QStringLiteral( "GeometryGroup" ), QString(), true );
    areaFunc->setIsStatic( false );
    functions << areaFunc;

    functions << new QgsStaticExpressionFunction( QStringLiteral( "area" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnArea, QStringLiteral( "GeometryGroup" ) );

    QgsStaticExpressionFunction *lengthFunc = new QgsStaticExpressionFunction( QStringLiteral( "$length" ), 0, fcnGeomLength, QStringLiteral( "GeometryGroup" ), QString(), true );
    lengthFunc->setIsStatic( false );
    functions << lengthFunc;

    QgsStaticExpressionFunction *perimeterFunc = new QgsStaticExpressionFunction( QStringLiteral( "$perimeter" ), 0, fcnGeomPerimeter, QStringLiteral( "GeometryGroup" ), QString(), true );
    perimeterFunc->setIsStatic( false );
    functions << perimeterFunc;

    functions << new QgsStaticExpressionFunction( QStringLiteral( "perimeter" ),  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnPerimeter, QStringLiteral( "GeometryGroup" ) );

    functions << new QgsStaticExpressionFunction( QStringLiteral( "roundness" ),
              QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
              fcnRoundness, QStringLiteral( "GeometryGroup" ) );

    QgsStaticExpressionFunction *xFunc = new QgsStaticExpressionFunction( QStringLiteral( "$x" ), 0, fcnX, QStringLiteral( "GeometryGroup" ), QString(), true );
    xFunc->setIsStatic( false );
    functions << xFunc;

    QgsStaticExpressionFunction *yFunc = new QgsStaticExpressionFunction( QStringLiteral( "$y" ), 0, fcnY, QStringLiteral( "GeometryGroup" ), QString(), true );
    yFunc->setIsStatic( false );
    functions << yFunc;

    QgsStaticExpressionFunction *zFunc = new QgsStaticExpressionFunction( QStringLiteral( "$z" ), 0, fcnZ, QStringLiteral( "GeometryGroup" ), QString(), true );
    zFunc->setIsStatic( false );
    functions << zFunc;

    QMap< QString, QgsExpressionFunction::FcnEval > geometry_overlay_definitions
    {
      { QStringLiteral( "overlay_intersects" ), fcnGeomOverlayIntersects },
      { QStringLiteral( "overlay_contains" ), fcnGeomOverlayContains },
      { QStringLiteral( "overlay_crosses" ), fcnGeomOverlayCrosses },
      { QStringLiteral( "overlay_equals" ), fcnGeomOverlayEquals },
      { QStringLiteral( "overlay_touches" ), fcnGeomOverlayTouches },
      { QStringLiteral( "overlay_disjoint" ), fcnGeomOverlayDisjoint },
      { QStringLiteral( "overlay_within" ), fcnGeomOverlayWithin },
    };
    QMapIterator< QString, QgsExpressionFunction::FcnEval > i( geometry_overlay_definitions );
    while ( i.hasNext() )
    {
      i.next();
      QgsStaticExpressionFunction *fcnGeomOverlayFunc = new QgsStaticExpressionFunction( i.key(), QgsExpressionFunction::ParameterList()
          << QgsExpressionFunction::Parameter( QStringLiteral( "layer" ) )
          << QgsExpressionFunction::Parameter( QStringLiteral( "expression" ), true, QVariant(), true )
          << QgsExpressionFunction::Parameter( QStringLiteral( "filter" ), true, QVariant(), true )
          << QgsExpressionFunction::Parameter( QStringLiteral( "limit" ), true, QVariant( -1 ), true )
          << QgsExpressionFunction::Parameter( QStringLiteral( "cache" ), true, QVariant( false ), false )
          << QgsExpressionFunction::Parameter( QStringLiteral( "min_overlap" ), true, QVariant( -1 ), false )
          << QgsExpressionFunction::Parameter( QStringLiteral( "min_inscribed_circle_radius" ), true, QVariant( -1 ), false )
          << QgsExpressionFunction::Parameter( QStringLiteral( "return_details" ), true, false, false )
          << QgsExpressionFunction::Parameter( QStringLiteral( "sort_by_intersection_size" ), true, QString(), false ),
          i.value(), QStringLiteral( "GeometryGroup" ), QString(), true, QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES, true );

      // The current feature is accessed for the geometry, so this should not be cached
      fcnGeomOverlayFunc->setIsStatic( false );
      functions << fcnGeomOverlayFunc;
    }

    QgsStaticExpressionFunction *fcnGeomOverlayNearestFunc   = new QgsStaticExpressionFunction( QStringLiteral( "overlay_nearest" ), QgsExpressionFunction::ParameterList()
        << QgsExpressionFunction::Parameter( QStringLiteral( "layer" ) )
        << QgsExpressionFunction::Parameter( QStringLiteral( "expression" ), true, QVariant(), true )
        << QgsExpressionFunction::Parameter( QStringLiteral( "filter" ), true, QVariant(), true )
        << QgsExpressionFunction::Parameter( QStringLiteral( "limit" ), true, QVariant( 1 ), true )
        << QgsExpressionFunction::Parameter( QStringLiteral( "max_distance" ), true, 0 )
        << QgsExpressionFunction::Parameter( QStringLiteral( "cache" ), true, QVariant( false ), false ),
        fcnGeomOverlayNearest, QStringLiteral( "GeometryGroup" ), QString(), true, QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES, true );
    // The current feature is accessed for the geometry, so this should not be cached
    fcnGeomOverlayNearestFunc->setIsStatic( false );
    functions << fcnGeomOverlayNearestFunc;

    functions
        << new QgsStaticExpressionFunction( QStringLiteral( "is_valid" ),  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnGeomIsValid, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "x" ),  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnGeomX, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "y" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnGeomY, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "z" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnGeomZ, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "m" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnGeomM, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "point_n" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "index" ) ), fcnPointN, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "start_point" ),  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnStartPoint, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "end_point" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnEndPoint, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "nodes_to_points" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "ignore_closing_nodes" ), true, false ),
                                            fcnNodesToPoints, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "segments_to_lines" ),  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnSegmentsToLines, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "collect_geometries" ), -1, fcnCollectGeometries, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "make_point" ), -1, fcnMakePoint, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "make_point_m" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "x" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "y" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "m" ) ),
                                            fcnMakePointM, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "make_line" ), -1, fcnMakeLine, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "make_polygon" ), -1, fcnMakePolygon, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "make_triangle" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "point1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "point2" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "point3" ) ),
                                            fcnMakeTriangle, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "make_circle" ), QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "center" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "radius" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "segments" ), true, 36 ),
                                            fcnMakeCircle, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "make_ellipse" ), QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "center" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "semi_major_axis" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "semi_minor_axis" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "azimuth" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "segments" ), true, 36 ),
                                            fcnMakeEllipse, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "make_regular_polygon" ), QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "center" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "radius" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "number_sides" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "circle" ), true, 0 ),
                                            fcnMakeRegularPolygon, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "make_square" ), QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "point1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "point2" ) ),
                                            fcnMakeSquare, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "make_rectangle_3points" ), QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "point1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "point2" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "point3" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "option" ), true, 0 ),
                                            fcnMakeRectangleFrom3Points, QStringLiteral( "GeometryGroup" ) );
    QgsStaticExpressionFunction *xAtFunc = new QgsStaticExpressionFunction( QStringLiteral( "$x_at" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "i" ) ), fcnXat, QStringLiteral( "GeometryGroup" ), QString(), true, QSet<QString>(), false, QStringList() << QStringLiteral( "xat" ) << QStringLiteral( "x_at" ) );
    xAtFunc->setIsStatic( false );
    functions << xAtFunc;

    QgsStaticExpressionFunction *yAtFunc = new QgsStaticExpressionFunction( QStringLiteral( "$y_at" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "i" ) ), fcnYat, QStringLiteral( "GeometryGroup" ), QString(), true, QSet<QString>(), false, QStringList() << QStringLiteral( "yat" ) << QStringLiteral( "y_at" ) );
    yAtFunc->setIsStatic( false );
    functions << yAtFunc;

    functions
        << new QgsStaticExpressionFunction( QStringLiteral( "geometry_type" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnGeometryType, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "x_min" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnXMin, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "xmin" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "x_max" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnXMax, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "xmax" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "y_min" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnYMin, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "ymin" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "y_max" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnYMax, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "ymax" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "geom_from_wkt" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "text" ) ), fcnGeomFromWKT, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "geomFromWKT" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "geom_from_wkb" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "binary" ) ), fcnGeomFromWKB, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false )
        << new QgsStaticExpressionFunction( QStringLiteral( "geom_from_gml" ),  QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "gml" ) ), fcnGeomFromGML, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "geomFromGML" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "flip_coordinates" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnFlipCoordinates, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "relate" ), -1, fcnRelate, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "intersects_bbox" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry1" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "geometry2" ) ), fcnBbox, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "bbox" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "disjoint" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "geometry2" ) ),
                                            fcnDisjoint, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "intersects" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "geometry2" ) ),
                                            fcnIntersects, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "touches" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "geometry2" ) ),
                                            fcnTouches, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "crosses" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "geometry2" ) ),
                                            fcnCrosses, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "contains" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "geometry2" ) ),
                                            fcnContains, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "overlaps" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "geometry2" ) ),
                                            fcnOverlaps, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "within" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "geometry2" ) ),
                                            fcnWithin, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "translate" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "dx" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "dy" ) ),
                                            fcnTranslate, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "rotate" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "rotation" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "center" ), true )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "per_part" ), true, false ),
                                            fcnRotate, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "scale" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "x_scale" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "y_scale" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "center" ), true ),
                                            fcnScale, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "affine_transform" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "delta_x" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "delta_y" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "rotation_z" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "scale_x" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "scale_y" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "delta_z" ), true, 0 )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "delta_m" ), true, 0 )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "scale_z" ), true, 1 )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "scale_m" ), true, 1 ),
                                            fcnAffineTransform, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "buffer" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "distance" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "segments" ), true, 8 )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "cap" ), true, QStringLiteral( "round" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "join" ), true, QStringLiteral( "round" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "miter_limit" ), true, 2 ),
                                            fcnBuffer, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "force_rhr" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
                                            fcnForceRHR, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "force_polygon_cw" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
                                            fcnForcePolygonCW, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "force_polygon_ccw" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
                                            fcnForcePolygonCCW, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "wedge_buffer" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "center" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "azimuth" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "width" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "outer_radius" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "inner_radius" ), true, 0.0 ), fcnWedgeBuffer, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "tapered_buffer" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "start_width" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "end_width" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "segments" ), true, 8.0 )
                                            , fcnTaperedBuffer, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "buffer_by_m" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "segments" ), true, 8.0 )
                                            , fcnBufferByM, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "offset_curve" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "distance" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "segments" ), true, 8.0 )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "join" ), true, static_cast< int >( Qgis::JoinStyle::Round ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "miter_limit" ), true, 2.0 ),
                                            fcnOffsetCurve, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "single_sided_buffer" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "distance" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "segments" ), true, 8.0 )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "join" ), true, static_cast< int >( Qgis::JoinStyle::Round ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "miter_limit" ), true, 2.0 ),
                                            fcnSingleSidedBuffer, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "extend" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "start_distance" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "end_distance" ) ),
                                            fcnExtend, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "centroid" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnCentroid, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "point_on_surface" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnPointOnSurface, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "pole_of_inaccessibility" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "tolerance" ) ), fcnPoleOfInaccessibility, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "reverse" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnReverse, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "exterior_ring" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnExteriorRing, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "interior_ring_n" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "index" ) ),
                                            fcnInteriorRingN, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "geometry_n" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "index" ) ),
                                            fcnGeometryN, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "boundary" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnBoundary, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "line_merge" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnLineMerge, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "bounds" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnBounds, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "simplify" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "tolerance" ) ), fcnSimplify, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "simplify_vw" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "tolerance" ) ), fcnSimplifyVW, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "smooth" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "iterations" ), true, 1 )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "offset" ), true, 0.25 )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "min_length" ), true, -1 )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "max_angle" ), true, 180 ), fcnSmooth, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "triangular_wave" ),
    {
      QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "wavelength" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "amplitude" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "strict" ), true, false )
    }, fcnTriangularWave, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "triangular_wave_randomized" ),
    {
      QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "min_wavelength" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "max_wavelength" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "min_amplitude" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "max_amplitude" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "seed" ), true, 0 )
    }, fcnTriangularWaveRandomized, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "square_wave" ),
    {
      QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "wavelength" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "amplitude" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "strict" ), true, false )
    }, fcnSquareWave, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "square_wave_randomized" ),
    {
      QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "min_wavelength" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "max_wavelength" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "min_amplitude" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "max_amplitude" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "seed" ), true, 0 )
    }, fcnSquareWaveRandomized, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "wave" ),
    {
      QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "wavelength" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "amplitude" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "strict" ), true, false )
    }, fcnRoundWave, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "wave_randomized" ),
    {
      QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "min_wavelength" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "max_wavelength" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "min_amplitude" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "max_amplitude" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "seed" ), true, 0 )
    }, fcnRoundWaveRandomized, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "apply_dash_pattern" ),
    {
      QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "pattern" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "start_rule" ), true, QStringLiteral( "no_rule" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "end_rule" ), true, QStringLiteral( "no_rule" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "adjustment" ), true, QStringLiteral( "both" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "pattern_offset" ), true, 0 ),
    }, fcnApplyDashPattern, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "densify_by_count" ),
    {
      QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "vertices" ) )
    }, fcnDensifyByCount, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "densify_by_distance" ),
    {
      QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
      QgsExpressionFunction::Parameter( QStringLiteral( "distance" ) )
    }, fcnDensifyByDistance, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "num_points" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnGeomNumPoints, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "num_interior_rings" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnGeomNumInteriorRings, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "num_rings" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnGeomNumRings, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "num_geometries" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnGeomNumGeometries, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "bounds_width" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnBoundsWidth, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "bounds_height" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnBoundsHeight, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "is_closed" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnIsClosed, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "close_line" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnCloseLine, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "is_empty" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnIsEmpty, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "is_empty_or_null" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnIsEmptyOrNull, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "convex_hull" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ), fcnConvexHull, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "convexHull" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "oriented_bbox" ), QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
                                            fcnOrientedBBox, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "main_angle" ), QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
                                            fcnMainAngle, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "minimal_circle" ), QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "segments" ), true, 36 ),
                                            fcnMinimalCircle, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "difference" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "geometry2" ) ),
                                            fcnDifference, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "distance" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "geometry2" ) ),
                                            fcnDistance, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "hausdorff_distance" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry1" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "geometry2" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "densify_fraction" ), true ),
                                            fcnHausdorffDistance, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "intersection" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "geometry2" ) ),
                                            fcnIntersection, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "sym_difference" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "geometry2" ) ),
                                            fcnSymDifference, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "symDifference" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "combine" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "geometry2" ) ),
                                            fcnCombine, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "union" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "geometry2" ) ),
                                            fcnCombine, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "geom_to_wkt" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "precision" ), true, 8.0 ),
                                            fcnGeomToWKT, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "geomToWKT" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "geom_to_wkb" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
                                            fcnGeomToWKB, QStringLiteral( "GeometryGroup" ), QString(), false, QSet<QString>(), false )
        << new QgsStaticExpressionFunction( QStringLiteral( "geometry" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "feature" ) ), fcnGetGeometry, QStringLiteral( "GeometryGroup" ), QString(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "transform" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "source_auth_id" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "dest_auth_id" ) ),
                                            fcnTransformGeometry, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "extrude" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "x" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "y" ) ),
                                            fcnExtrude, QStringLiteral( "GeometryGroup" ), QString() )
        << new QgsStaticExpressionFunction( QStringLiteral( "is_multipart" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
                                            fcnGeomIsMultipart, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "z_max" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
                                            fcnZMax, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "z_min" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
                                            fcnZMin, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "m_max" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
                                            fcnMMax, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "m_min" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
                                            fcnMMin, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "sinuosity" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
                                            fcnSinuosity, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "straight_distance_2d" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) ),
                                            fcnStraightDistance2d, QStringLiteral( "GeometryGroup" ) );


    QgsStaticExpressionFunction *orderPartsFunc = new QgsStaticExpressionFunction( QStringLiteral( "order_parts" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
        << QgsExpressionFunction::Parameter( QStringLiteral( "orderby" ) )
        << QgsExpressionFunction::Parameter( QStringLiteral( "ascending" ), true, true ),
        fcnOrderParts, QStringLiteral( "GeometryGroup" ), QString() );

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
        << new QgsStaticExpressionFunction( QStringLiteral( "closest_point" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "geometry2" ) ),
                                            fcnClosestPoint, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "shortest_line" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry1" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "geometry2" ) ),
                                            fcnShortestLine, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "line_interpolate_point" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "distance" ) ), fcnLineInterpolatePoint, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "line_interpolate_angle" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "distance" ) ), fcnLineInterpolateAngle, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "line_locate_point" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "point" ) ), fcnLineLocatePoint, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "angle_at_vertex" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "vertex" ) ), fcnAngleAtVertex, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "distance_to_vertex" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "vertex" ) ), fcnDistanceToVertex, QStringLiteral( "GeometryGroup" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "line_substring" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "geometry" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "start_distance" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "end_distance" ) ), fcnLineSubset, QStringLiteral( "GeometryGroup" ) );


    // **Record** functions

    QgsStaticExpressionFunction *idFunc = new QgsStaticExpressionFunction( QStringLiteral( "$id" ), 0, fcnFeatureId, QStringLiteral( "Record and Attributes" ) );
    idFunc->setIsStatic( false );
    functions << idFunc;

    QgsStaticExpressionFunction *currentFeatureFunc = new QgsStaticExpressionFunction( QStringLiteral( "$currentfeature" ), 0, fcnFeature, QStringLiteral( "Record and Attributes" ) );
    currentFeatureFunc->setIsStatic( false );
    functions << currentFeatureFunc;

    QgsStaticExpressionFunction *uuidFunc = new QgsStaticExpressionFunction( QStringLiteral( "uuid" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "format" ), true, QStringLiteral( "WithBraces" ) ), fcnUuid, QStringLiteral( "Record and Attributes" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "$uuid" ) );
    uuidFunc->setIsStatic( false );
    functions << uuidFunc;

    functions
        << new QgsStaticExpressionFunction( QStringLiteral( "get_feature" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "layer" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "attribute" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "value" ), true ),
                                            fcnGetFeature, QStringLiteral( "Record and Attributes" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "QgsExpressionUtils::getFeature" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "get_feature_by_id" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "layer" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "feature_id" ) ),
                                            fcnGetFeatureById, QStringLiteral( "Record and Attributes" ), QString(), false, QSet<QString>(), false );

    QgsStaticExpressionFunction *attributesFunc = new QgsStaticExpressionFunction( QStringLiteral( "attributes" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "feature" ), true ),
        fcnAttributes, QStringLiteral( "Record and Attributes" ), QString(), false, QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES );
    attributesFunc->setIsStatic( false );
    functions << attributesFunc;
    QgsStaticExpressionFunction *representAttributesFunc = new QgsStaticExpressionFunction( QStringLiteral( "represent_attributes" ), -1,
        fcnRepresentAttributes, QStringLiteral( "Record and Attributes" ), QString(), false, QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES );
    representAttributesFunc->setIsStatic( false );
    functions << representAttributesFunc;

    QgsStaticExpressionFunction *maptipFunc = new QgsStaticExpressionFunction(
      QStringLiteral( "maptip" ),
      -1,
      fcnFeatureMaptip,
      QStringLiteral( "Record and Attributes" ),
      QString(),
      false,
      QSet<QString>()
    );
    maptipFunc->setIsStatic( false );
    functions << maptipFunc;

    QgsStaticExpressionFunction *displayFunc = new QgsStaticExpressionFunction(
      QStringLiteral( "display_expression" ),
      -1,
      fcnFeatureDisplayExpression,
      QStringLiteral( "Record and Attributes" ),
      QString(),
      false,
      QSet<QString>()
    );
    displayFunc->setIsStatic( false );
    functions << displayFunc;

    QgsStaticExpressionFunction *isSelectedFunc = new QgsStaticExpressionFunction(
      QStringLiteral( "is_selected" ),
      -1,
      fcnIsSelected,
      QStringLiteral( "Record and Attributes" ),
      QString(),
      false,
      QSet<QString>()
    );
    isSelectedFunc->setIsStatic( false );
    functions << isSelectedFunc;

    functions
        << new QgsStaticExpressionFunction(
          QStringLiteral( "num_selected" ),
          -1,
          fcnNumSelected,
          QStringLiteral( "Record and Attributes" ),
          QString(),
          false,
          QSet<QString>()
        );

    functions
        << new QgsStaticExpressionFunction(
          QStringLiteral( "sqlite_fetch_and_increment" ),
          QgsExpressionFunction::ParameterList()
          << QgsExpressionFunction::Parameter( QStringLiteral( "database" ) )
          << QgsExpressionFunction::Parameter( QStringLiteral( "table" ) )
          << QgsExpressionFunction::Parameter( QStringLiteral( "id_field" ) )
          << QgsExpressionFunction::Parameter( QStringLiteral( "filter_attribute" ) )
          << QgsExpressionFunction::Parameter( QStringLiteral( "filter_value" ) )
          << QgsExpressionFunction::Parameter( QStringLiteral( "default_values" ), true ),
          fcnSqliteFetchAndIncrement,
          QStringLiteral( "Record and Attributes" )
        );

    // **Fields and Values** functions
    QgsStaticExpressionFunction *representValueFunc = new QgsStaticExpressionFunction( QStringLiteral( "represent_value" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "attribute" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "field_name" ), true ), fcnRepresentValue, QStringLiteral( "Record and Attributes" ) );

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
        << new QgsStaticExpressionFunction( QStringLiteral( "layer_property" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "layer" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "property" ) ),
                                            fcnGetLayerProperty, QStringLiteral( "Map Layers" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "decode_uri" ),
                                            QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "layer" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "part" ), true ),
                                            fcnDecodeUri, QStringLiteral( "Map Layers" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "mime_type" ),
                                            QgsExpressionFunction::ParameterList()
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "binary_data" ) ),
                                            fcnMimeType, QStringLiteral( "General" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "raster_statistic" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "layer" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "band" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "statistic" ) ), fcnGetRasterBandStat, QStringLiteral( "Rasters" ) );

    // **var** function
    QgsStaticExpressionFunction *varFunction = new QgsStaticExpressionFunction( QStringLiteral( "var" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "name" ) ), fcnGetVariable, QStringLiteral( "General" ) );
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

        QString varName = argNode->eval( parent, context ).toString();

        const QgsExpressionContextScope *scope = context->activeScopeForVariable( varName );
        return scope ? scope->isStatic( varName ) : false;
      }
      return false;
    }
    );

    functions
        << varFunction;

    functions << new QgsStaticExpressionFunction( QStringLiteral( "eval_template" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "template" ) ), fcnEvalTemplate, QStringLiteral( "General" ), QString(), true );

    QgsStaticExpressionFunction *evalFunc = new QgsStaticExpressionFunction( QStringLiteral( "eval" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "expression" ) ), fcnEval, QStringLiteral( "General" ), QString(), true, QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES );
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

    QgsStaticExpressionFunction *attributeFunc = new QgsStaticExpressionFunction( QStringLiteral( "attribute" ), -1, fcnAttribute, QStringLiteral( "Record and Attributes" ), QString(), false, QSet<QString>() << QgsFeatureRequest::ALL_ATTRIBUTES );
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
        << new QgsStaticExpressionFunction( QStringLiteral( "env" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "name" ) ), fcnEnvVar, QStringLiteral( "General" ), QString() )
        << new QgsWithVariableExpressionFunction()
        << new QgsStaticExpressionFunction( QStringLiteral( "raster_value" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "layer" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "band" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "point" ) ), fcnRasterValue, QStringLiteral( "Rasters" ) )

        // functions for arrays
        << new QgsArrayForeachExpressionFunction()
        << new QgsArrayFilterExpressionFunction()
        << new QgsStaticExpressionFunction( QStringLiteral( "array" ), -1, fcnArray, QStringLiteral( "Arrays" ), QString(), false, QSet<QString>(), false, QStringList(), true )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_sort" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "ascending" ), true, true ), fcnArraySort, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_length" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ), fcnArrayLength, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_contains" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnArrayContains, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_count" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnArrayCount, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_all" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array_a" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "array_b" ) ), fcnArrayAll, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_find" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnArrayFind, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_get" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "pos" ) ), fcnArrayGet, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_first" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ), fcnArrayFirst, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_last" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ), fcnArrayLast, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_min" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ), fcnArrayMinimum, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_max" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ), fcnArrayMaximum, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_mean" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ), fcnArrayMean, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_median" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ), fcnArrayMedian, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_majority" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "option" ), true, QVariant( "all" ) ), fcnArrayMajority, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_minority" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "option" ), true, QVariant( "all" ) ), fcnArrayMinority, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_sum" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ), fcnArraySum, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_append" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnArrayAppend, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_prepend" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnArrayPrepend, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_insert" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "pos" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnArrayInsert, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_remove_at" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "pos" ) ), fcnArrayRemoveAt, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_remove_all" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnArrayRemoveAll, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_replace" ), -1, fcnArrayReplace, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_prioritize" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "array_prioritize" ) ), fcnArrayPrioritize, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_cat" ), -1, fcnArrayCat, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_slice" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "start_pos" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "end_pos" ) ), fcnArraySlice, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_reverse" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ), fcnArrayReverse, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_intersect" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array1" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "array2" ) ), fcnArrayIntersect, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_distinct" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ), fcnArrayDistinct, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "array_to_string" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "delimiter" ), true, "," ) << QgsExpressionFunction::Parameter( QStringLiteral( "emptyvalue" ), true, "" ), fcnArrayToString, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "string_to_array" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "delimiter" ), true, "," ) << QgsExpressionFunction::Parameter( QStringLiteral( "emptyvalue" ), true, "" ), fcnStringToArray, QStringLiteral( "Arrays" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "generate_series" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "start" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "stop" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "step" ), true, 1.0 ), fcnGenerateSeries, QStringLiteral( "Arrays" ) )

        //functions for maps
        << new QgsStaticExpressionFunction( QStringLiteral( "from_json" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnLoadJson, QStringLiteral( "Maps" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "json_to_map" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "to_json" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "json_string" ) ), fcnWriteJson, QStringLiteral( "Maps" ), QString(), false, QSet<QString>(), false, QStringList() << QStringLiteral( "map_to_json" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "hstore_to_map" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "string" ) ), fcnHstoreToMap, QStringLiteral( "Maps" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "map_to_hstore" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "map" ) ), fcnMapToHstore, QStringLiteral( "Maps" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "map" ), -1, fcnMap, QStringLiteral( "Maps" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "map_get" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "map" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "key" ) ), fcnMapGet, QStringLiteral( "Maps" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "map_exist" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "map" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "key" ) ), fcnMapExist, QStringLiteral( "Maps" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "map_delete" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "map" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "key" ) ), fcnMapDelete, QStringLiteral( "Maps" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "map_insert" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "map" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "key" ) ) << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) ), fcnMapInsert, QStringLiteral( "Maps" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "map_concat" ), -1, fcnMapConcat, QStringLiteral( "Maps" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "map_akeys" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "map" ) ), fcnMapAKeys, QStringLiteral( "Maps" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "map_avals" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "map" ) ), fcnMapAVals, QStringLiteral( "Maps" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "map_prefix_keys" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "map" ) )
                                            << QgsExpressionFunction::Parameter( QStringLiteral( "prefix" ) ),
                                            fcnMapPrefixKeys, QStringLiteral( "Maps" ) )
        << new QgsStaticExpressionFunction( QStringLiteral( "url_encode" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "map" ) ),
                                            fcnToFormUrlEncode, QStringLiteral( "Maps" ) )

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
    sFunctions()->removeAt( fnIdx );
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
  : QgsExpressionFunction( QStringLiteral( "array_foreach" ), QgsExpressionFunction::ParameterList()  // skip-keyword-check
                           << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) )
                           << QgsExpressionFunction::Parameter( QStringLiteral( "expression" ) ),
                           QStringLiteral( "Arrays" ) )
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

  for ( QVariantList::const_iterator it = array.constBegin(); it != array.constEnd(); ++it )
  {
    subScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "element" ), *it, true ) );
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
  subScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "element" ), QVariant(), true ) );
  subContext.appendScope( subScope );

  args->at( 1 )->prepare( parent, &subContext );

  return true;
}

QgsArrayFilterExpressionFunction::QgsArrayFilterExpressionFunction()
  : QgsExpressionFunction( QStringLiteral( "array_filter" ), QgsExpressionFunction::ParameterList()
                           << QgsExpressionFunction::Parameter( QStringLiteral( "array" ) )
                           << QgsExpressionFunction::Parameter( QStringLiteral( "expression" ) )
                           << QgsExpressionFunction::Parameter( QStringLiteral( "limit" ), true, 0 ),
                           QStringLiteral( "Arrays" ) )
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
    subScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "element" ), value, true ) );
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
  subScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "element" ), QVariant(), true ) );
  subContext.appendScope( subScope );

  args->at( 1 )->prepare( parent, &subContext );

  return true;
}
QgsWithVariableExpressionFunction::QgsWithVariableExpressionFunction()
  : QgsExpressionFunction( QStringLiteral( "with_variable" ), QgsExpressionFunction::ParameterList() <<
                           QgsExpressionFunction::Parameter( QStringLiteral( "name" ) )
                           << QgsExpressionFunction::Parameter( QStringLiteral( "value" ) )
                           << QgsExpressionFunction::Parameter( QStringLiteral( "expression" ) ),
                           QStringLiteral( "General" ) )
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
