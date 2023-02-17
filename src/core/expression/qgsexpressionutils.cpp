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
#include "qgsvectorlayerfeatureiterator.h"

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

QgsMapLayer *QgsExpressionUtils::getMapLayer( const QVariant &value, const QgsExpressionContext *context, QgsExpression *parent )
{
  return getMapLayerPrivate( value, context, parent );
}

QgsMapLayer *QgsExpressionUtils::getMapLayerPrivate( const QVariant &value, const QgsExpressionContext *context, QgsExpression * )
{
  // First check if we already received a layer pointer
  QPointer< QgsMapLayer > ml = value.value< QgsWeakMapLayerPointer >().data();

  // clang analyzer gets this function absolutely 100% wrong
#ifdef __clang_analyzer__
  ( void )context;
#else

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

  const QString identifier = value.toString();

  // check through layer stores from context
  if ( context )
  {
    const QList< QgsMapLayerStore * > stores = context->layerStores();
    for ( QgsMapLayerStore *store : stores )
    {

      QPointer< QgsMapLayerStore > storePointer( store );
      auto findLayerInStoreFunction = [ storePointer, &ml, identifier ]
      {
        if ( QgsMapLayerStore *store = storePointer.data() )
        {
          // look for matching layer by id
          ml = store->mapLayer( identifier );

          if ( ml )
            return;

          // Still nothing? Check for layer name
          ml = store->mapLayersByName( identifier ).value( 0 );
        }
      };

      // Make sure we only deal with the store on the thread where it lives.
      // Anything else risks a crash.
      if ( QThread::currentThread() == store->thread() )
        findLayerInStoreFunction();
      else
        QMetaObject::invokeMethod( store, findLayerInStoreFunction, Qt::BlockingQueuedConnection );
      if ( ml )
        return ml;
    }
  }

  // last resort - QgsProject instance. This is bad, we need to remove this!
  auto getMapLayerFromProjectInstance = [ &ml, identifier ]
  {
    QgsProject *project = QgsProject::instance();

    // No pointer yet, maybe it's a layer id?
    ml = project->mapLayer( identifier );
    if ( ml )
      return;

    // Still nothing? Check for layer name
    ml = project->mapLayersByName( identifier ).value( 0 );
  };

  if ( QThread::currentThread() == qApp->thread() )
    getMapLayerFromProjectInstance();
  else
    QMetaObject::invokeMethod( qApp, getMapLayerFromProjectInstance, Qt::BlockingQueuedConnection );
#endif

  return ml;
}

void QgsExpressionUtils::executeLambdaForMapLayer( const QVariant &value, const QgsExpressionContext *context, QgsExpression *expression, const std::function<void ( QgsMapLayer * )> &function, bool &foundLayer )
{
  foundLayer = false;

  // clang analyzer gets this function absolutely 100% wrong
#ifndef __clang_analyzer__

  // First check if we already received a layer pointer
  QPointer< QgsMapLayer > ml = value.value< QgsWeakMapLayerPointer >().data();
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
  {
    QPointer< QgsMapLayer > layerPointer( ml );
    auto runFunction = [ layerPointer, &function, &foundLayer ]
    {
      if ( QgsMapLayer *layer = layerPointer.data() )
      {
        foundLayer = true;
        function( layer );
      }
    };

    // Make sure we only deal with the layer on the thread where it lives.
    // Anything else risks a crash.

    if ( QThread::currentThread() == ml->thread() )
      runFunction();
    else
      QMetaObject::invokeMethod( ml, runFunction, Qt::BlockingQueuedConnection );

    return;
  }

  if ( !context || context->layerStores().empty() )
  {
    // if no layer stores, then this is only for layers in project and therefore associated with the main thread
    auto runFunction = [ value, context, expression, &function, &foundLayer ]
    {
      if ( QgsMapLayer *layer = getMapLayerPrivate( value, context, expression ) )
      {
        foundLayer = true;
        function( layer );
      }
      else
      {
        foundLayer = false;
      }
    };

    // Make sure we only deal with the project on the thread where it lives.
    // Anything else risks a crash.
    if ( QThread::currentThread() == QgsProject::instance()->thread() )
      runFunction();
    else
      QMetaObject::invokeMethod( QgsProject::instance(), runFunction, Qt::BlockingQueuedConnection );
  }
  else
  {
    // if layer stores, then we can't be certain in advance of which thread the layer will have affinity with.
    // So we need to fetch the layer and then run the function on the layer's thread.

    const QString identifier = value.toString();

    // check through layer stores from context
    const QList< QgsMapLayerStore * > stores = context->layerStores();

    for ( QgsMapLayerStore *store : stores )
    {
      QPointer< QgsMapLayerStore > storePointer( store );
      auto findLayerInStoreFunction = [ storePointer, identifier, function, &foundLayer ]
      {
        QgsMapLayer *ml = nullptr;
        if ( QgsMapLayerStore *store = storePointer.data() )
        {
          // look for matching layer by id
          ml = store->mapLayer( identifier );
          if ( !ml )
          {
            // Still nothing? Check for layer name
            ml = store->mapLayersByName( identifier ).value( 0 );
          }

          if ( ml )
          {
            function( ml );
            foundLayer = true;
          }
        }
      };

      // Make sure we only deal with the store on the thread where it lives.
      // Anything else risks a crash.
      if ( QThread::currentThread() == store->thread() )
        findLayerInStoreFunction();
      else
        QMetaObject::invokeMethod( store, findLayerInStoreFunction, Qt::BlockingQueuedConnection );

      if ( foundLayer )
        return;
    }

    // last resort - QgsProject instance. This is bad, we need to remove this!
    auto getMapLayerFromProjectInstance = [ value, identifier, &function, &foundLayer ]
    {
      QgsProject *project = QgsProject::instance();

      // maybe it's a layer id?
      QgsMapLayer *ml = project->mapLayer( identifier );

      // Still nothing? Check for layer name
      if ( !ml )
      {
        ml = project->mapLayersByName( identifier ).value( 0 );
      }

      if ( ml )
      {
        foundLayer = true;
        function( ml );
      }
    };

    if ( QThread::currentThread() == QgsProject::instance()->thread() )
      getMapLayerFromProjectInstance();
    else
      QMetaObject::invokeMethod( QgsProject::instance(), getMapLayerFromProjectInstance, Qt::BlockingQueuedConnection );
  }
#endif
}

QVariant QgsExpressionUtils::runMapLayerFunctionThreadSafe( const QVariant &value, const QgsExpressionContext *context, QgsExpression *expression, const std::function<QVariant( QgsMapLayer * )> &function, bool &foundLayer )
{
  QVariant res;
  foundLayer = false;

  executeLambdaForMapLayer( value, context, expression, [&res, function]( QgsMapLayer * layer )
  {
    if ( layer )
      res = function( layer );
  }, foundLayer );

  return res;
}

std::unique_ptr<QgsVectorLayerFeatureSource> QgsExpressionUtils::getFeatureSource( const QVariant &value, const QgsExpressionContext *context, QgsExpression *e, bool &foundLayer )
{
  std::unique_ptr<QgsVectorLayerFeatureSource> featureSource;

  executeLambdaForMapLayer( value, context, e, [&featureSource]( QgsMapLayer * layer )
  {
    if ( QgsVectorLayer *vl = qobject_cast< QgsVectorLayer *>( layer ) )
    {
      featureSource.reset( new QgsVectorLayerFeatureSource( vl ) );
    }
  }, foundLayer );

  return featureSource;
}

QgsVectorLayer *QgsExpressionUtils::getVectorLayer( const QVariant &value, const QgsExpressionContext *context, QgsExpression *e )
{
  return qobject_cast<QgsVectorLayer *>( getMapLayerPrivate( value, context, e ) );
}

QString QgsExpressionUtils::getFilePathValue( const QVariant &value, const QgsExpressionContext *context, QgsExpression *parent )
{
  // if it's a map layer, return the file path of that layer...
  QString res;
  Q_NOWARN_DEPRECATED_PUSH
  if ( QgsMapLayer *layer = getMapLayer( value, context, parent ) )
  {
    const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( layer->providerType(), layer->source() );
    res = parts.value( QStringLiteral( "path" ) ).toString();
  }
  Q_NOWARN_DEPRECATED_POP

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
