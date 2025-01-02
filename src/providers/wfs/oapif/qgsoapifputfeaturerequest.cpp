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
using namespace nlohmann;

#include "qgslogger.h"
#include "qgsjsonutils.h"
#include "qgsoapifputfeaturerequest.h"
#include "moc_qgsoapifputfeaturerequest.cpp"
#include "qgsoapifprovider.h"

QgsOapifPutFeatureRequest::QgsOapifPutFeatureRequest( const QgsDataSourceUri &uri )
  : QgsBaseNetworkRequest( QgsAuthorizationSettings( uri.username(), uri.password(), uri.authConfigId() ), "OAPIF" )
{
}

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
  const QString jsonFeature = QString::fromStdString( j.dump() );
  QgsDebugMsgLevel( jsonFeature, 5 );
  mEmptyResponseIsValid = true;
  mFakeResponseHasHeaders = true;
  QList<QNetworkReply::RawHeaderPair> extraHeaders;
  if ( !contentCrs.isEmpty() )
    extraHeaders.append( QNetworkReply::RawHeaderPair( QByteArray( "Content-Crs" ), contentCrs.toUtf8() ) );
  QUrl url( sharedData->mItemsUrl + QString( QStringLiteral( "/" ) + jsonId ) );
  return sendPUT( url, "application/geo+json", jsonFeature.toUtf8(), extraHeaders );
}

QString QgsOapifPutFeatureRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Put Feature request failed: %1" ).arg( reason );
}
