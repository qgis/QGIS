/***************************************************************************
                         qgsprocessingcontext.cpp
                         ----------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingcontext.h"
#include "qgsprocessingutils.h"

QgsProcessingContext::~QgsProcessingContext()
{
  for ( auto it = mLayersToLoadOnCompletion.constBegin(); it != mLayersToLoadOnCompletion.constEnd(); ++it )
  {
    delete it.value().postProcessor();
  }
}

void QgsProcessingContext::setLayersToLoadOnCompletion( const QMap<QString, QgsProcessingContext::LayerDetails> &layers )
{
  for ( auto it = mLayersToLoadOnCompletion.constBegin(); it != mLayersToLoadOnCompletion.constEnd(); ++it )
  {
    if ( !layers.contains( it.key() ) || layers.value( it.key() ).postProcessor() != it.value().postProcessor() )
      delete it.value().postProcessor();
  }
  mLayersToLoadOnCompletion = layers;
}

void QgsProcessingContext::addLayerToLoadOnCompletion( const QString &layer, const QgsProcessingContext::LayerDetails &details )
{
  if ( mLayersToLoadOnCompletion.contains( layer ) && mLayersToLoadOnCompletion.value( layer ).postProcessor() != details.postProcessor() )
    delete mLayersToLoadOnCompletion.value( layer ).postProcessor();

  mLayersToLoadOnCompletion.insert( layer, details );
}

void QgsProcessingContext::setInvalidGeometryCheck( QgsFeatureRequest::InvalidGeometryCheck check )
{
  mInvalidGeometryCheck = check;

  switch ( mInvalidGeometryCheck )
  {
    case  QgsFeatureRequest::GeometryAbortOnInvalid:
    {
      auto callback = []( const QgsFeature & feature )
      {
        throw QgsProcessingException( QObject::tr( "Feature (%1) has invalid geometry. Please fix the geometry or change the Processing setting to the \"Ignore invalid input features\" option." ).arg( feature.id() ) );
      };
      mInvalidGeometryCallback = callback;
      break;
    }

    case QgsFeatureRequest::GeometrySkipInvalid:
    {
      auto callback = [ = ]( const QgsFeature & feature )
      {
        if ( mFeedback )
          mFeedback->reportError( QObject::tr( "Feature (%1) has invalid geometry and has been skipped. Please fix the geometry or change the Processing setting to the \"Ignore invalid input features\" option." ).arg( feature.id() ) );
      };
      mInvalidGeometryCallback = callback;
      break;
    }

    default:
      break;
  }
}

void QgsProcessingContext::takeResultsFrom( QgsProcessingContext &context )
{
  setLayersToLoadOnCompletion( context.mLayersToLoadOnCompletion );
  context.mLayersToLoadOnCompletion.clear();
  tempLayerStore.transferLayersFromStore( context.temporaryLayerStore() );
}

QgsMapLayer *QgsProcessingContext::getMapLayer( const QString &identifier )
{
  return QgsProcessingUtils::mapLayerFromString( identifier, *this, false );
}

QgsMapLayer *QgsProcessingContext::takeResultLayer( const QString &id )
{
  return tempLayerStore.takeMapLayer( tempLayerStore.mapLayer( id ) );
}



QgsProcessingLayerPostProcessorInterface *QgsProcessingContext::LayerDetails::postProcessor() const
{
  return mPostProcessor;
}

void QgsProcessingContext::LayerDetails::setPostProcessor( QgsProcessingLayerPostProcessorInterface *processor )
{
  if ( mPostProcessor && mPostProcessor != processor )
    delete mPostProcessor;

  mPostProcessor = processor;
}
