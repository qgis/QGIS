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
#include "qgsfileutils.h"

#include <QLocale>

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

QString QgsMapLayerUtils::generalHtmlMetadata( const QgsMapLayer *layer )
{
  QString metadata;

  // name
  metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Name" ) + QStringLiteral( "</td><td>" ) + layer->name() + QStringLiteral( "</td></tr>\n" );

  // local path
  QVariantMap uriComponents = QgsProviderRegistry::instance()->decodeUri( layer->dataProvider()->name(), layer->publicSource() );
  QString path;
  bool isLocalPath = false;
  if ( uriComponents.contains( QStringLiteral( "path" ) ) )
  {
    path = uriComponents[QStringLiteral( "path" )].toString();
    QFileInfo fi( path );
    if ( fi.exists() )
    {
      isLocalPath = true;
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Path" ) + QStringLiteral( "</td><td>%1" ).arg( QStringLiteral( "<a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( path ).toString(), QDir::toNativeSeparators( path ) ) ) + QStringLiteral( "</td></tr>\n" );

      qint64 fileSize = fi.size();
      QDateTime lastModified = fi.lastModified();
      QString lastModifiedFileName;
      QSet<QString> sidecarFiles = QgsFileUtils::sidecarFilesForPath( path );
      if ( fi.isFile() )
      {
        if ( !sidecarFiles.isEmpty() )
        {
          lastModifiedFileName = fi.fileName();
          QStringList sidecarFileNames;
          for ( const QString &sidecarFile : sidecarFiles )
          {
            QFileInfo sidecarFi( sidecarFile );
            fileSize += sidecarFi.size();
            if ( sidecarFi.lastModified() > lastModified )
            {
              lastModified = sidecarFi.lastModified();
              lastModifiedFileName = sidecarFi.fileName();
            }
            sidecarFileNames << sidecarFi.fileName();
          }
          metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + ( sidecarFiles.size() > 1 ? QObject::tr( "Sidecar files" ) : QObject::tr( "Sidecar file" ) ) + QStringLiteral( "</td><td>%1" ).arg( sidecarFileNames.join( QLatin1String( ", " ) ) ) + QStringLiteral( "</td></tr>\n" );
        }
        metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + ( !sidecarFiles.isEmpty() ? QObject::tr( "Total size" ) : QObject::tr( "Size" ) ) + QStringLiteral( "</td><td>%1" ).arg( QgsFileUtils::representFileSize( fileSize ) ) + QStringLiteral( "</td></tr>\n" );
      }
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Last modified" ) + QStringLiteral( "</td><td>%1" ).arg( QLocale().toString( fi.lastModified() ) ) + ( !lastModifiedFileName.isEmpty() ? QStringLiteral( " (%1)" ).arg( lastModifiedFileName ) : "" ) + QStringLiteral( "</td></tr>\n" );
    }
  }
  if ( uriComponents.contains( QStringLiteral( "url" ) ) )
  {
    const QString url = uriComponents[QStringLiteral( "url" )].toString();
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "URL" ) + QStringLiteral( "</td><td>%1" ).arg( QStringLiteral( "<a href=\"%1\">%2</a>" ).arg( QUrl( url ).toString(), url ) ) + QStringLiteral( "</td></tr>\n" );
  }

  // data source
  if ( layer->publicSource() != path || !isLocalPath )
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Source" ) + QStringLiteral( "</td><td>%1" ).arg( layer->publicSource() != path ? layer->publicSource() : path ) + QStringLiteral( "</td></tr>\n" );

  // provider
  metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + QObject::tr( "Provider" ) + QStringLiteral( "</td><td>%1" ).arg( layer->dataProvider()->name() ) + QStringLiteral( "</td></tr>\n" );

  return metadata;
}
