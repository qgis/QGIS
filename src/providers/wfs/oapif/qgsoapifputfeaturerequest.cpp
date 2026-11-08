/***************************************************************************
    qgsoapifputfeaturerequest.cpp
    -----------------------------
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

#include <QString>

using namespace Qt::StringLiterals;

using namespace nlohmann;

#include "qgslogger.h"
#include "qgsjsonutils.h"
#include "qgsoapifputfeaturerequest.h"
#include "moc_qgsoapifputfeaturerequest.cpp"
#include "qgsoapifshareddata.h"
#include "qgsoapifutils.h"
#include "qgswfsconstants.h"

QgsOapifPutFeatureRequest::QgsOapifPutFeatureRequest( const QgsDataSourceUri &uri )
  : QgsBaseNetworkRequest( QgsAuthorizationSettings( uri.username(), uri.password(), QgsHttpHeaders(), uri.authConfigId() ), tr( "OAPIF" ), uri.hasParam( QgsWFSConstants::URI_PARAM_DISABLE_DECOMPRESSED_SAFETY_CHECK_THRESHOLD ) )
{}

bool QgsOapifPutFeatureRequest::putFeature( const QgsOapifSharedData *sharedData, const QString &jsonId, const QgsFeature &f, const QString &contentCrs, bool hasAxisInverted )
{
  QgsJsonExporter exporter;

  QgsFeature fModified( f );
  if ( hasAxisInverted && f.hasGeometry() )
  {
    QgsGeometry g = f.geometry();
    g.get()->swapXy();
    fModified.setGeometry( g );
  }

  json j = exporter.exportFeatureToJsonObject( fModified, QVariantMap(), jsonId );
  auto iterBbox = j.find( "bbox" );
  if ( iterBbox != j.end() )
    j.erase( iterBbox );
  if ( !sharedData->mFoundIdInProperties )
  {
    auto jPropertiesIter = j.find( "properties" );
    if ( jPropertiesIter != j.end() )
    {
      auto &jProperties = *jPropertiesIter;
      auto iterId = jProperties.find( "id" );
      if ( iterId != jProperties.end() )
        jProperties.erase( iterId );
    }
  }
  QgsOAPIFJson::removeReadOnlyProperties( j, sharedData->mFields );
  const bool jsonFg = sharedData->shouldWriteJsonFg();
  if ( jsonFg )
  {
    QgsOAPIFJson::writeJsonFgPlace( j, fModified.geometry(), sharedData->mWKBType, contentCrs, true );
  }
  const QString jsonFeature = QString::fromStdString( j.dump() );
  QgsDebugMsgLevel( jsonFeature, 5 );
  mEmptyResponseIsValid = true;
  mFakeResponseHasHeaders = true;
  QList<QNetworkReply::RawHeaderPair> extraHeaders;
  if ( !contentCrs.isEmpty() )
  {
    extraHeaders.append( QNetworkReply::RawHeaderPair( QByteArray( "Content-Crs" ), contentCrs.toUtf8() ) );
  }
  if ( jsonFg )
  {
    extraHeaders.append( QNetworkReply::RawHeaderPair( QByteArray( "Link" ), u"<%1>; rel=profile"_s.arg( QgsOapifUtils::PROFILE_JSONFG ).toUtf8() ) );
  }
  QUrl url( sharedData->mItemsUrl + QString( u"/"_s + jsonId ) );
  return sendPUT( url, QgsOapifUtils::MEDIA_TYPE_GEOJSON, jsonFeature.toUtf8(), extraHeaders );
}

QString QgsOapifPutFeatureRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Put Feature request failed: %1" ).arg( reason );
}
