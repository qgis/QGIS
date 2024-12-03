/***************************************************************************
    qgsoapifcreatefeaturerequest.cpp
    --------------------------------
    begin                : March 2023
    copyright            : (C) 2023 by Even Rouault
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
#include "qgsjsonutils.h"
#include "qgsoapifcreatefeaturerequest.h"
#include "moc_qgsoapifcreatefeaturerequest.cpp"
#include "qgsoapifprovider.h"

QgsOapifCreateFeatureRequest::QgsOapifCreateFeatureRequest( const QgsDataSourceUri &uri )
  : QgsBaseNetworkRequest( QgsAuthorizationSettings( uri.username(), uri.password(), uri.authConfigId() ), "OAPIF" )
{
}

QString QgsOapifCreateFeatureRequest::createFeature( const QgsOapifSharedData *sharedData, const QgsFeature &f, const QString &contentCrs, bool hasAxisInverted )
{
  QgsJsonExporter exporter;

  QgsFeature fModified( f );
  if ( hasAxisInverted && f.hasGeometry() )
  {
    QgsGeometry g = f.geometry();
    g.get()->swapXy();
    fModified.setGeometry( g );
  }

  json j = exporter.exportFeatureToJsonObject( fModified );
  auto iterId = j.find( "id" );
  if ( iterId != j.end() )
    j.erase( iterId );
  auto iterBbox = j.find( "bbox" );
  if ( iterBbox != j.end() )
    j.erase( iterBbox );
  if ( !sharedData->mFoundIdInProperties && j["properties"].contains( "id" ) )
    j["properties"].erase( "id" );
  const QString jsonFeature = QString::fromStdString( j.dump() );
  QgsDebugMsgLevel( jsonFeature, 5 );
  mEmptyResponseIsValid = true;
  mFakeResponseHasHeaders = true;
  QList<QNetworkReply::RawHeaderPair> extraHeaders;
  if ( !contentCrs.isEmpty() )
    extraHeaders.append( QNetworkReply::RawHeaderPair( QByteArray( "Content-Crs" ), contentCrs.toUtf8() ) );
  if ( !sendPOST( sharedData->mItemsUrl, "application/geo+json", jsonFeature.toUtf8(), extraHeaders ) )
    return QString();

  QString location;
  for ( const auto &headerKeyValue : mResponseHeaders )
  {
    if ( headerKeyValue.first == QByteArray( "Location" ) )
    {
      location = QString::fromUtf8( headerKeyValue.second );
      break;
    }
  }

  const int posItems = location.lastIndexOf( QLatin1String( "/items/" ) );
  if ( posItems < 0 )
    return QString();

  const QString createdId = location.mid( posItems + static_cast<int>( strlen( "/items/" ) ) );
  QgsDebugMsgLevel( "createdId = " + createdId, 5 );
  return createdId;
}

QString QgsOapifCreateFeatureRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Create Feature request failed: %1" ).arg( reason );
}
