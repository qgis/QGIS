/***************************************************************************
                             qgsmaplayerutils.cpp
                             -------------------
    begin                : May 2021
    copyright            : (C) 2021 Nyall Dawson
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

#include "qgsmaplayerutils.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsreferencedgeometry.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgscoordinatetransform.h"
#include <QRegularExpression>

QgsRectangle QgsMapLayerUtils::combinedExtent( const QList<QgsMapLayer *> &layers, const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &transformContext )
{
  // We can't use a constructor since QgsRectangle normalizes the rectangle upon construction
  QgsRectangle fullExtent;
  fullExtent.setMinimal();

  // iterate through the map layers and test each layers extent
  // against the current min and max values
  QgsDebugMsgLevel( QStringLiteral( "Layer count: %1" ).arg( layers.count() ), 5 );
  for ( const QgsMapLayer *layer : layers )
  {
    QgsDebugMsgLevel( "Updating extent using " + layer->name(), 5 );
    QgsDebugMsgLevel( "Input extent: " + layer->extent().toString(), 5 );

    if ( layer->extent().isNull() )
      continue;

    // Layer extents are stored in the coordinate system (CS) of the
    // layer. The extent must be projected to the canvas CS
    QgsCoordinateTransform ct( layer->crs(), crs, transformContext );
    ct.setBallparkTransformsAreAppropriate( true );
    try
    {
      const QgsRectangle extent = ct.transformBoundingBox( layer->extent() );

      QgsDebugMsgLevel( "Output extent: " + extent.toString(), 5 );
      fullExtent.combineExtentWith( extent );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Could not reproject layer extent" ) );
    }
  }

  if ( fullExtent.width() == 0.0 || fullExtent.height() == 0.0 )
  {
    // If all of the features are at the one point, buffer the
    // rectangle a bit. If they are all at zero, do something a bit
    // more crude.

    if ( fullExtent.xMinimum() == 0.0 && fullExtent.xMaximum() == 0.0 &&
         fullExtent.yMinimum() == 0.0 && fullExtent.yMaximum() == 0.0 )
    {
      fullExtent.set( -1.0, -1.0, 1.0, 1.0 );
    }
    else
    {
      const double padFactor = 1e-8;
      const double widthPad = fullExtent.xMinimum() * padFactor;
      const double heightPad = fullExtent.yMinimum() * padFactor;
      const double xmin = fullExtent.xMinimum() - widthPad;
      const double xmax = fullExtent.xMaximum() + widthPad;
      const double ymin = fullExtent.yMinimum() - heightPad;
      const double ymax = fullExtent.yMaximum() + heightPad;
      fullExtent.set( xmin, ymin, xmax, ymax );
    }
  }

  QgsDebugMsgLevel( "Full extent: " + fullExtent.toString(), 5 );
  return fullExtent;
}

QgsAbstractDatabaseProviderConnection *QgsMapLayerUtils::databaseConnection( const QgsMapLayer *layer )
{
  if ( ! layer || ! layer->dataProvider() )
  {
    return nullptr;
  }

  try
  {
    QgsProviderMetadata *providerMetadata = QgsProviderRegistry::instance()->providerMetadata( layer->dataProvider()->name() );
    if ( ! providerMetadata )
    {
      return nullptr;
    }

    std::unique_ptr< QgsAbstractDatabaseProviderConnection > conn { static_cast<QgsAbstractDatabaseProviderConnection *>( providerMetadata->createConnection( layer->source(), {} ) ) };
    return conn.release();
  }
  catch ( const QgsProviderConnectionException &ex )
  {
    QgsDebugMsg( QStringLiteral( "Error retrieving database connection for layer %1: %2" ).arg( layer->name(), ex.what() ) );
    return nullptr;
  }
}

bool QgsMapLayerUtils::layerSourceMatchesPath( const QgsMapLayer *layer, const QString &path )
{
  if ( !layer || path.isEmpty() )
    return false;

  const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( layer->providerType(), layer->source() );
  return parts.value( QStringLiteral( "path" ) ).toString() == path;
}

bool QgsMapLayerUtils::updateLayerSourcePath( QgsMapLayer *layer, const QString &newPath )
{
  if ( !layer || newPath.isEmpty() )
    return false;

  QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( layer->providerType(), layer->source() );
  if ( !parts.contains( QStringLiteral( "path" ) ) )
    return false;

  parts.insert( QStringLiteral( "path" ), newPath );
  const QString newUri = QgsProviderRegistry::instance()->encodeUri( layer->providerType(), parts );
  layer->setDataSource( newUri, layer->name(), layer->providerType() );
  return true;
}

QList<QgsMapLayer *> QgsMapLayerUtils::sortLayersByType( const QList<QgsMapLayer *> &layers, const QList<QgsMapLayerType> &order )
{
  QList< QgsMapLayer * > res = layers;
  std::sort( res.begin(), res.end(), [&order]( const QgsMapLayer * a, const QgsMapLayer * b ) -> bool
  {
    for ( QgsMapLayerType type : order )
    {
      if ( a->type() == type && b->type() != type )
        return true;
      else if ( b->type() == type )
        return false;
    }
    return false;
  } );
  return res;
}

QString QgsMapLayerUtils::launderLayerName( const QString &name )
{
  QString laundered = name.toLower();
  const thread_local QRegularExpression sRxSwapChars( QStringLiteral( "\\s" ) );
  laundered.replace( sRxSwapChars, QStringLiteral( "_" ) );

  const thread_local QRegularExpression sRxRemoveChars( QStringLiteral( "[^a-zA-Z0-9_]" ) );
  laundered.replace( sRxRemoveChars, QString() );

  return laundered;
}
