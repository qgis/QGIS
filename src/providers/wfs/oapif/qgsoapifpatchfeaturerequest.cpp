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
using namespace nlohmann;

#include "qgsjsonutils.h"

#include "qgsoapifpatchfeaturerequest.h"
#include "moc_qgsoapifpatchfeaturerequest.cpp"
#include "qgsoapifprovider.h"

QgsOapifPatchFeatureRequest::QgsOapifPatchFeatureRequest( const QgsDataSourceUri &uri )
  : QgsBaseNetworkRequest( QgsAuthorizationSettings( uri.username(), uri.password(), uri.authConfigId() ), "OAPIF" )
{
}

bool QgsOapifPatchFeatureRequest::patchFeature( const QgsOapifSharedData *sharedData, const QString &jsonId, const QgsGeometry &geom, const QString &contentCrs, bool hasAxisInverted )
{
  QgsGeometry geomModified( geom );
  if ( hasAxisInverted )
  {
    geomModified.get()->swapXy();
  }

  json j;
  j["geometry"] = geomModified.asJsonObject();
  QList<QNetworkReply::RawHeaderPair> extraHeaders;
  if ( !contentCrs.isEmpty() )
    extraHeaders.append( QNetworkReply::RawHeaderPair( QByteArray( "Content-Crs" ), contentCrs.toUtf8() ) );
  mEmptyResponseIsValid = true;
  mFakeURLIncludesContentType = true;
  QUrl url( sharedData->mItemsUrl + QString( QStringLiteral( "/" ) + jsonId ) );
  return sendPATCH( url, "application/merge-patch+json", QString::fromStdString( j.dump() ).toUtf8(), extraHeaders );
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
  QUrl url( sharedData->mItemsUrl + QString( QStringLiteral( "/" ) + jsonId ) );
  return sendPATCH( url, "application/merge-patch+json", QString::fromStdString( j.dump() ).toUtf8() );
}

QString QgsOapifPatchFeatureRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Patch Feature request failed: %1" ).arg( reason );
}
