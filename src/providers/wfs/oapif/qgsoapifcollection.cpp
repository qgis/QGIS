/***************************************************************************
    qgsoapifcollection.cpp
    ---------------------
    begin                : October 2019
    copyright            : (C) 2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <nlohmann/json.hpp>
using namespace nlohmann;

#include "qgslogger.h"
#include "qgsoapifcollection.h"
#include "qgsoapifutils.h"
#include "qgsoapifprovider.h"

#include <set>

#include <QTextCodec>

bool QgsOapifCollection::deserialize( const json &j )
{
  if ( !j.is_object() )
    return false;
  const char *idPropertyName = "id";
  if ( !j.contains( "id" ) )
  {
#ifndef REMOVE_SUPPORT_DRAFT_VERSIONS
    if ( j.contains( "name" ) )
    {
      idPropertyName = "name";
    }
    else
#endif
    {
      QgsDebugMsg( QStringLiteral( "missing id in collection" ) );
      return false;
    }
  }
  const auto id = j[idPropertyName];
  if ( !id.is_string() )
    return false;
  mId = QString::fromStdString( id.get<std::string>() );

  mLayerMetadata.setType( QStringLiteral( "dataset" ) );

  const auto links = QgsOAPIFJson::parseLinks( j );
  const auto selfUrl = QgsOAPIFJson::findLink( links,
                       QStringLiteral( "self" ),
  { QStringLiteral( "application/json" ) } );
  if ( !selfUrl.isEmpty() )
  {
    mLayerMetadata.setIdentifier( selfUrl );
  }
  else
  {
    mLayerMetadata.setIdentifier( mId );
  }

  const auto parentUrl = QgsOAPIFJson::findLink( links,
                         QStringLiteral( "parent" ),
  { QStringLiteral( "application/json" ) } );
  if ( !parentUrl.isEmpty() )
  {
    mLayerMetadata.setParentIdentifier( parentUrl );
  }

  for ( const auto &link : links )
  {
    auto mdLink = QgsAbstractMetadataBase::Link( link.rel, QStringLiteral( "WWW:LINK" ), link.href );
    mdLink.mimeType = link.type;
    mdLink.description = link.title;
    if ( link.length > 0 )
      mdLink.size = QString::number( link.length );
    mLayerMetadata.addLink( mdLink );
  }

  if ( j.contains( "title" ) )
  {
    const auto title = j["title"];
    if ( title.is_string() )
    {
      mTitle = QString::fromStdString( title.get<std::string>() );
      mLayerMetadata.setTitle( mTitle );
    }
  }

  if ( j.contains( "description" ) )
  {
    const auto description = j["description"];
    if ( description.is_string() )
    {
      mDescription = QString::fromStdString( description.get<std::string>() );
      mLayerMetadata.setAbstract( mDescription );
    }
  }

  if ( j.contains( "extent" ) )
  {
    QgsLayerMetadata::Extent metadataExtent;
    const auto extent = j["extent"];
    if ( extent.is_object() && extent.contains( "spatial" ) )
    {
      const auto spatial = extent["spatial"];
      if ( spatial.is_object() && spatial.contains( "bbox" ) )
      {
        QgsCoordinateReferenceSystem crs( QgsCoordinateReferenceSystem::fromOgcWmsCrs(
                                            QgsOapifProvider::OAPIF_PROVIDER_DEFAULT_CRS ) );
        if ( spatial.contains( "crs" ) )
        {
          const auto jCrs = spatial["crs"];
          if ( jCrs.is_string() )
          {
            crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QString::fromStdString( jCrs.get<std::string>() ) );
          }
        }
        mLayerMetadata.setCrs( crs );

        const auto jBboxes = spatial["bbox"];
        if ( jBboxes.is_array() )
        {
          QList<  QgsLayerMetadata::SpatialExtent > spatialExtents;
          bool firstBbox = true;
          for ( const auto &jBbox : jBboxes )
          {
            if ( jBbox.is_array() && ( jBbox.size() == 4 || jBbox.size() == 6 ) )
            {
              std::vector<double> values;
              for ( size_t i = 0; i < jBbox.size(); i++ )
              {
                if ( !jBbox[i].is_number() )
                {
                  values.clear();
                  break;
                }
                values.push_back( jBbox[i].get<double>() );
              }
              QgsLayerMetadata::SpatialExtent spatialExtent;
              spatialExtent.extentCrs = crs;
              if ( values.size() == 4 )
              {
                if ( firstBbox )
                {
                  mBbox.set( values[0], values[1], values[2], values[3] );
                }
                spatialExtent.bounds = QgsBox3d( mBbox );
              }
              else if ( values.size() == 6 ) // with zmin at [2] and zmax at [5]
              {
                if ( firstBbox )
                {
                  mBbox.set( values[0], values[1], values[3], values[4] );
                }
                spatialExtent.bounds = QgsBox3d( values[0], values[1], values[2],
                                                 values[3], values[4], values[5] );
              }
              if ( values.size() == 4 || values.size() == 6 )
              {
                spatialExtents << spatialExtent;
                firstBbox = false;
              }
            }
          }
          metadataExtent.setSpatialExtents( spatialExtents );
        }
      }
    }
#ifndef REMOVE_SUPPORT_DRAFT_VERSIONS
    else if ( extent.is_object() && extent.contains( "bbox" ) )
    {
      const auto bbox = extent["bbox"];
      if ( bbox.is_array() && bbox.size() == 4 )
      {
        std::vector<double> values;
        for ( size_t i = 0; i < bbox.size(); i++ )
        {
          if ( !bbox[i].is_number() )
          {
            values.clear();
            break;
          }
          values.push_back( bbox[i].get<double>() );
        }
        if ( values.size() == 4 )
        {
          mBbox.set( values[0], values[1], values[2], values[3] );
          QgsLayerMetadata::SpatialExtent spatialExtent;
          spatialExtent.extentCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs(
                                      QgsOapifProvider::OAPIF_PROVIDER_DEFAULT_CRS );
          mLayerMetadata.setCrs( spatialExtent.extentCrs );
          metadataExtent.setSpatialExtents( QList<  QgsLayerMetadata::SpatialExtent >() << spatialExtent );
        }
      }
    }
#endif

    if ( extent.is_object() && extent.contains( "temporal" ) )
    {
      const auto temporal = extent["temporal"];
      if ( temporal.is_object() && temporal.contains( "interval" ) )
      {
        const auto jIntervals = temporal["interval"];
        if ( jIntervals.is_array() )
        {
          QList< QgsDateTimeRange > temporalExtents;
          for ( const auto &jInterval : jIntervals )
          {
            if ( jInterval.is_array() && jInterval.size() == 2 )
            {
              QDateTime dt[2];
              for ( int i = 0; i < 2; i++ )
              {
                if ( jInterval[i].is_string() )
                {
                  dt[i] = QDateTime::fromString( QString::fromStdString( jInterval[i].get<std::string>() ), Qt::ISODateWithMs );
                }
              }
              if ( !dt[0].isNull() || !dt[1].isNull() )
              {
                temporalExtents << QgsDateTimeRange( dt[0], dt[1] );
              }
            }
          }
          metadataExtent.setTemporalExtents( temporalExtents );
        }
      }
    }

    mLayerMetadata.setExtent( metadataExtent );
  }

  // From STAC specification ( https://stacspec.org/ )
  bool isProprietaryLicense = false;
  if ( j.contains( "license" ) )
  {
    const auto jLicense = j["license"];
    if ( jLicense.is_string() )
    {
      const auto license = QString::fromStdString( jLicense.get<std::string>() );
      if ( license == QLatin1String( "proprietary" ) )
      {
        isProprietaryLicense = true;
      }
      else if ( license != QLatin1String( "various" ) )
      {
        mLayerMetadata.setLicenses( { license } );
      }
    }
  }
  if ( mLayerMetadata.licenses().isEmpty() ) // standard OAPIF
  {
    QStringList licenses;
    std::set<QString> licenseSet;
    for ( const auto &link : links )
    {
      if ( link.rel == QLatin1String( "license" ) )
      {
        const auto license =  !link.title.isEmpty() ? link.title : link.href;
        if ( licenseSet.find( license ) == licenseSet.end() )
        {
          licenseSet.insert( license );
          licenses << license;
        }
      }
    }
    if ( licenses.isEmpty() && isProprietaryLicense )
    {
      licenses << QStringLiteral( "proprietary" );
    }
    mLayerMetadata.setLicenses( licenses );
  }

  // From STAC specification
  if ( j.contains( "keywords" ) )
  {
    const auto jKeywords = j["keywords"];
    if ( jKeywords.is_array() )
    {
      QStringList keywords;
      for ( const auto &jKeyword : jKeywords )
      {
        if ( jKeyword.is_string() )
        {
          keywords << QString::fromStdString( jKeyword.get<std::string>() );
        }
      }
      if ( !keywords.empty() )
      {
        mLayerMetadata.addKeywords( QStringLiteral( "keywords" ), keywords );
      }
    }
  }

  if ( j.contains( "crs" ) )
  {
    const auto crsUrls = j["crs"];
    if ( crsUrls.is_array() )
    {
      for ( const auto &crsUrl : crsUrls )
      {
        if ( crsUrl.is_string() )
        {
          QString crs = QString::fromStdString( crsUrl.get<std::string>() );
          mLayerMetadata.setCrs( QgsCoordinateReferenceSystem::fromOgcWmsCrs( crs ) );

          // Take the first CRS of the list
          break;
        }
      }
    }
  }

  return true;
}

// -----------------------------------------

QgsOapifCollectionsRequest::QgsOapifCollectionsRequest( const QgsDataSourceUri &baseUri, const QString &url ):
  QgsBaseNetworkRequest( QgsAuthorizationSettings( baseUri.username(), baseUri.password(), baseUri.authConfigId() ), tr( "OAPIF" ) ),
  mUrl( url )
{
  // Using Qt::DirectConnection since the download might be running on a different thread.
  // In this case, the request was sent from the main thread and is executed with the main
  // thread being blocked in future.waitForFinished() so we can run code on this object which
  // lives in the main thread without risking havoc.
  connect( this, &QgsBaseNetworkRequest::downloadFinished, this, &QgsOapifCollectionsRequest::processReply, Qt::DirectConnection );
}

bool QgsOapifCollectionsRequest::request( bool synchronous, bool forceRefresh )
{
  if ( !sendGET( QUrl( mUrl ), QStringLiteral( "application/json" ), synchronous, forceRefresh ) )
  {
    emit gotResponse();
    return false;
  }
  return true;
}

QString QgsOapifCollectionsRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of collections description failed: %1" ).arg( reason );
}

void QgsOapifCollectionsRequest::processReply()
{
  if ( mErrorCode != QgsBaseNetworkRequest::NoError )
  {
    emit gotResponse();
    return;
  }
  const QByteArray &buffer = mResponse;
  if ( buffer.isEmpty() )
  {
    mErrorMessage = tr( "empty response" );
    mErrorCode = QgsBaseNetworkRequest::ServerExceptionError;
    emit gotResponse();
    return;
  }

  QgsDebugMsgLevel( QStringLiteral( "parsing collections response: " ) + buffer, 4 );

  QTextCodec::ConverterState state;
  QTextCodec *codec = QTextCodec::codecForName( "UTF-8" );
  Q_ASSERT( codec );

  const QString utf8Text = codec->toUnicode( buffer.constData(), buffer.size(), &state );
  if ( state.invalidChars != 0 )
  {
    mErrorCode = QgsBaseNetworkRequest::ApplicationLevelError;
    mAppLevelError = ApplicationLevelError::JsonError;
    mErrorMessage = errorMessageWithReason( tr( "Invalid UTF-8 content" ) );
    emit gotResponse();
    return;
  }

  try
  {
    const json j = json::parse( utf8Text.toStdString() );

    const auto links = QgsOAPIFJson::parseLinks( j );
    QStringList licenses;
    std::set<QString> licenseSet;
    for ( const auto &link : links )
    {
      if ( link.rel == QLatin1String( "license" ) )
      {
        const auto license =  !link.title.isEmpty() ? link.title : link.href;
        if ( licenseSet.find( license ) == licenseSet.end() )
        {
          licenseSet.insert( license );
          licenses << license;
        }
      }
    }

    if ( j.is_object() && j.contains( "collections" ) )
    {
      const auto collections = j["collections"];
      if ( collections.is_array() )
      {
        for ( const auto &jCollection : collections )
        {
          QgsOapifCollection collection;
          if ( collection.deserialize( jCollection ) )
          {
            if ( collection.mLayerMetadata.licenses().isEmpty() )
            {
              // If there are not licenses from the collection description,
              // use the one from the collection set.
              collection.mLayerMetadata.setLicenses( licenses );
            }
            mCollections.emplace_back( collection );
          }
        }
      }
    }

    // Paging informal extension used by api.planet.com/
    mNextUrl = QgsOAPIFJson::findLink( links,
                                       QStringLiteral( "next" ),
    {  QStringLiteral( "application/json" ) } );
  }
  catch ( const json::parse_error &ex )
  {
    mErrorCode = QgsBaseNetworkRequest::ApplicationLevelError;
    mAppLevelError = ApplicationLevelError::JsonError;
    mErrorMessage = errorMessageWithReason( tr( "Cannot decode JSON document: %1" ).arg( QString::fromStdString( ex.what() ) ) );
    emit gotResponse();
    return;
  }

  emit gotResponse();
}

// -----------------------------------------

QgsOapifCollectionRequest::QgsOapifCollectionRequest( const QgsDataSourceUri &baseUri, const QString &url ):
  QgsBaseNetworkRequest( QgsAuthorizationSettings( baseUri.username(), baseUri.password(), baseUri.authConfigId() ), tr( "OAPIF" ) ),
  mUrl( url )
{
  // Using Qt::DirectConnection since the download might be running on a different thread.
  // In this case, the request was sent from the main thread and is executed with the main
  // thread being blocked in future.waitForFinished() so we can run code on this object which
  // lives in the main thread without risking havoc.
  connect( this, &QgsBaseNetworkRequest::downloadFinished, this, &QgsOapifCollectionRequest::processReply, Qt::DirectConnection );
}

bool QgsOapifCollectionRequest::request( bool synchronous, bool forceRefresh )
{
  if ( !sendGET( QUrl( mUrl ), QStringLiteral( "application/json" ), synchronous, forceRefresh ) )
  {
    emit gotResponse();
    return false;
  }
  return true;
}

QString QgsOapifCollectionRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of collection description failed: %1" ).arg( reason );
}

void QgsOapifCollectionRequest::processReply()
{
  if ( mErrorCode != QgsBaseNetworkRequest::NoError )
  {
    emit gotResponse();
    return;
  }
  const QByteArray &buffer = mResponse;
  if ( buffer.isEmpty() )
  {
    mErrorMessage = tr( "empty response" );
    mErrorCode = QgsBaseNetworkRequest::ServerExceptionError;
    emit gotResponse();
    return;
  }

  QgsDebugMsgLevel( QStringLiteral( "parsing collection response: " ) + buffer, 4 );

  QTextCodec::ConverterState state;
  QTextCodec *codec = QTextCodec::codecForName( "UTF-8" );
  Q_ASSERT( codec );

  const QString utf8Text = codec->toUnicode( buffer.constData(), buffer.size(), &state );
  if ( state.invalidChars != 0 )
  {
    mErrorCode = QgsBaseNetworkRequest::ApplicationLevelError;
    mAppLevelError = ApplicationLevelError::JsonError;
    mErrorMessage = errorMessageWithReason( tr( "Invalid UTF-8 content" ) );
    emit gotResponse();
    return;
  }

  try
  {
    const json j = json::parse( utf8Text.toStdString() );
    mCollection.deserialize( j );
  }
  catch ( const json::parse_error &ex )
  {
    mErrorCode = QgsBaseNetworkRequest::ApplicationLevelError;
    mAppLevelError = ApplicationLevelError::JsonError;
    mErrorMessage = errorMessageWithReason( tr( "Cannot decode JSON document: %1" ).arg( QString::fromStdString( ex.what() ) ) );
    emit gotResponse();
    return;
  }

  emit gotResponse();
}
