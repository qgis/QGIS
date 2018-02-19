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
  QMap< QString, LayerDetails > loadOnCompletion = context.layersToLoadOnCompletion();
  QMap< QString, LayerDetails >::const_iterator llIt = loadOnCompletion.constBegin();
  for ( ; llIt != loadOnCompletion.constEnd(); ++llIt )
  {
    mLayersToLoadOnCompletion.insert( llIt.key(), llIt.value() );
  }
  context.setLayersToLoadOnCompletion( QMap< QString, LayerDetails >() );
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
