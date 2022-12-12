/***************************************************************************
                               qgsexpressionutils.cpp
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

#include "qgsexpressionutils.h"
#include "qgsvectorlayer.h"
#include "qgscolorrampimpl.h"
#include "qgsproviderregistry.h"
#include "qgsvariantutils.h"
#include "qgsproject.h"

///@cond PRIVATE

QgsExpressionUtils::TVL QgsExpressionUtils::AND[3][3] =
{
  // false  true    unknown
  { False, False,   False },   // false
  { False, True,    Unknown }, // true
  { False, Unknown, Unknown }  // unknown
};
QgsExpressionUtils::TVL QgsExpressionUtils::OR[3][3] =
{
  { False,   True, Unknown },  // false
  { True,    True, True },     // true
  { Unknown, True, Unknown }   // unknown
};

QgsExpressionUtils::TVL QgsExpressionUtils::NOT[3] = { True, False, Unknown };


QgsGradientColorRamp QgsExpressionUtils::getRamp( const QVariant &value, QgsExpression *parent, bool report_error )
{
  if ( value.userType() == QMetaType::type( "QgsGradientColorRamp" ) )
    return value.value<QgsGradientColorRamp>();

  // If we get here then we can't convert so we just error and return invalid.
  if ( report_error )
    parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to gradient ramp" ).arg( value.toString() ) );

  return QgsGradientColorRamp();
}

QgsMapLayer *QgsExpressionUtils::getMapLayer( const QVariant &value, const QgsExpressionContext *, QgsExpression * )
{
  // First check if we already received a layer pointer
  QgsMapLayer *ml = value.value< QgsWeakMapLayerPointer >().data();
  if ( !ml )
  {
    ml = value.value< QgsMapLayer * >();
#ifdef QGISDEBUG
    if ( ml )
    {
      qWarning( "Raw map layer pointer stored in expression evaluation, switch to QgsWeakMapLayerPointer instead" );
    }
#endif
  }
  if ( ml )
    return ml;

  // last resort - QgsProject instance. This is bad, we need to remove this!
  auto getMapLayerFromProjectInstance = [ value, &ml ]
  {
    QgsProject *project = QgsProject::instance();

    // No pointer yet, maybe it's a layer id?
    ml = project->mapLayer( value.toString() );
    if ( ml )
      return;

    // Still nothing? Check for layer name
    ml = project->mapLayersByName( value.toString() ).value( 0 );
  };

#ifndef __clang_analyzer__
  if ( QThread::currentThread() == qApp->thread() )
    getMapLayerFromProjectInstance();
  else
    QMetaObject::invokeMethod( qApp, getMapLayerFromProjectInstance, Qt::BlockingQueuedConnection );
#endif

  return ml;
}

QVariant QgsExpressionUtils::runMapLayerFunctionThreadSafe( const QVariant &value, const QgsExpressionContext *context, QgsExpression *expression, const std::function<QVariant( QgsMapLayer * )> &function )
{
  QVariant res;
  auto runFunction = [ value, context, expression, &function, &res ]
  {
    if ( QgsMapLayer *layer = getMapLayer( value, context, expression ) )
    {
      res = function( layer );
    }
  };

  // Make sure we only deal with the layer on the thread where it lives.
  // Anything else risks a crash.

  // Note that this is not completely correct -- a layer may have been created on a non-main thread!
#ifndef __clang_analyzer__
  if ( QThread::currentThread() == qApp->thread() )
    runFunction();
  else
    QMetaObject::invokeMethod( qApp, runFunction, Qt::BlockingQueuedConnection );
#endif

  return res;
}

std::unique_ptr<QgsVectorLayerFeatureSource> QgsExpressionUtils::getFeatureSource( const QVariant &value, const QgsExpressionContext *context, QgsExpression *e )
{
  std::unique_ptr<QgsVectorLayerFeatureSource> featureSource;

#ifndef __clang_analyzer__
  auto getFeatureSource = [ &value, context, e, &featureSource ]
  {
    QgsVectorLayer *layer = getVectorLayer( value, context, e );

    if ( layer )
    {
      featureSource.reset( new QgsVectorLayerFeatureSource( layer ) );
    }
  };

  // Make sure we only deal with the vector layer on the main thread where it lives.
  // Anything else risks a crash.

  if ( QThread::currentThread() == qApp->thread() )
    getFeatureSource();
  else
    QMetaObject::invokeMethod( qApp, getFeatureSource, Qt::BlockingQueuedConnection );
#endif

  return featureSource;
}

QString QgsExpressionUtils::getFilePathValue( const QVariant &value, const QgsExpressionContext *context, QgsExpression *parent )
{
  // if it's a map layer, return the file path of that layer...
  QString res;
  if ( QgsMapLayer *layer = getMapLayer( value, context, parent ) )
  {
    const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( layer->providerType(), layer->source() );
    res = parts.value( QStringLiteral( "path" ) ).toString();
  }

  if ( res.isEmpty() )
    res = value.toString();

  if ( res.isEmpty() && !QgsVariantUtils::isNull( value ) )
  {
    parent->setEvalErrorString( QObject::tr( "Cannot convert value to a file path" ) );
  }
  return res;
}

///@endcond

std::tuple<QVariant::Type, int> QgsExpressionUtils::determineResultType( const QString &expression, const QgsVectorLayer *layer, QgsFeatureRequest request, QgsExpressionContext context, bool *foundFeatures )
{
  QgsExpression exp( expression );
  request.setFlags( ( exp.needsGeometry() ) ?
                    QgsFeatureRequest::NoFlags :
                    QgsFeatureRequest::NoGeometry );
  request.setLimit( 10 );
  request.setExpressionContext( context );

  // avoid endless recursion by removing virtual fields while going through features
  // to determine result type
  QgsAttributeList attributes;
  const QgsFields fields = layer->fields();
  for ( int i = 0; i < fields.count(); i++ )
  {
    if ( fields.fieldOrigin( i ) != QgsFields::OriginExpression )
      attributes << i;
  }
  request.setSubsetOfAttributes( attributes );

  QVariant value;
  QgsFeature f;
  QgsFeatureIterator it = layer->getFeatures( request );
  bool hasFeature = it.nextFeature( f );
  if ( foundFeatures )
    *foundFeatures = hasFeature;
  while ( hasFeature )
  {
    context.setFeature( f );
    const QVariant value = exp.evaluate( &context );
    if ( !QgsVariantUtils::isNull( value ) )
    {
      return std::make_tuple( value.type(), value.userType() );
    }
    hasFeature = it.nextFeature( f );
  }
  value = QVariant();
  return std::make_tuple( value.type(), value.userType() );
}
