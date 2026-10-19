/***************************************************************************
    qgsoapifpatchfeaturerequest.cpp
    -------------------------------
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

#include "qgsjsonutils.h"

#include "qgsoapifpatchfeaturerequest.h"
#include "moc_qgsoapifpatchfeaturerequest.cpp"
#include "qgsoapifshareddata.h"
#include "qgsoapifutils.h"

QgsOapifPatchFeatureRequest::QgsOapifPatchFeatureRequest( const QgsDataSourceUri &uri )
  : QgsBaseNetworkRequest( QgsAuthorizationSettings( uri.username(), uri.password(), QgsHttpHeaders(), uri.authConfigId() ), "OAPIF" )
{}

bool QgsOapifPatchFeatureRequest::patchFeature( const QgsOapifSharedData *sharedData, const QString &jsonId, const QgsGeometry &geom, const QString &contentCrs, bool hasAxisInverted )
{
  QgsGeometry geomModified( geom );
  if ( hasAxisInverted && !geomModified.isNull() )
  {
    geomModified.get()->swapXy();
  }

  json j;
  const bool jsonFg = sharedData->shouldWriteJsonFg();
  if ( jsonFg )
    QgsOAPIFJson::writeJsonFgPlace( j, geomModified, sharedData->mWKBType, contentCrs, false );
  else
    j["geometry"] = geomModified.asJsonObject();
  QList<QNetworkReply::RawHeaderPair> extraHeaders;
  if ( !contentCrs.isEmpty() )
    extraHeaders.append( QNetworkReply::RawHeaderPair( QByteArray( "Content-Crs" ), contentCrs.toUtf8() ) );
  if ( jsonFg )
    extraHeaders.append( QNetworkReply::RawHeaderPair( QByteArray( "Link" ), u"<%1>; rel=profile"_s.arg( QgsOapifUtils::PROFILE_JSONFG ).toUtf8() ) );
  mEmptyResponseIsValid = true;
  mFakeURLIncludesContentType = true;
  QUrl url( sharedData->mItemsUrl + QString( u"/"_s + jsonId ) );
  return sendPATCH( url, QgsOapifUtils::MEDIA_TYPE_MERGE_PATCH_JSON, QString::fromStdString( j.dump() ).toUtf8(), extraHeaders );
}

bool QgsOapifPatchFeatureRequest::patchFeature( const QgsOapifSharedData *sharedData, const QString &jsonId, const QgsAttributeMap &attrMap )
{
  json properties;
  QgsAttributeMap::const_iterator attMapIt = attrMap.constBegin();
  for ( ; attMapIt != attrMap.constEnd(); ++attMapIt )
  {
    QString fieldName = sharedData->mFields.at( attMapIt.key() ).name();
    properties[fieldName.toStdString()] = QgsJsonUtils::jsonFromVariant( attMapIt.value() );
  }
  json j;
  j["properties"] = properties;
  mEmptyResponseIsValid = true;
  mFakeURLIncludesContentType = true;
  QUrl url( sharedData->mItemsUrl + QString( u"/"_s + jsonId ) );
  return sendPATCH( url, QgsOapifUtils::MEDIA_TYPE_MERGE_PATCH_JSON, QString::fromStdString( j.dump() ).toUtf8() );
}

QString QgsOapifPatchFeatureRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Patch Feature request failed: %1" ).arg( reason );
}
