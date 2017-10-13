/***************************************************************************
    qgsafsshareddata.cpp
    ---------------------
    begin                : June 2017
    copyright            : (C) 2017 by Sandro Mani
    email                : manisandro at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsafsshareddata.h"
#include "qgsarcgisrestutils.h"
#include "qgslogger.h"

bool QgsAfsSharedData::getFeature( QgsFeatureId id, QgsFeature &f, bool fetchGeometry, const QList<int> & /*fetchAttributes*/, const QgsRectangle &filterRect )
{
  // If cached, return cached feature
  QMap<QgsFeatureId, QgsFeature>::const_iterator it = mCache.constFind( id );
  if ( it != mCache.constEnd() )
  {
    f = it.value();
    return filterRect.isNull() || f.geometry().intersects( filterRect );
  }

  // Determine attributes to fetch
  /*QStringList fetchAttribNames;
  foreach ( int idx, fetchAttributes )
    fetchAttribNames.append( mFields.at( idx ).name() );
  */

  // When fetching from server, fetch all attributes and geometry by default so that we can cache them
  QStringList fetchAttribNames;
  QList<int> fetchAttribIdx;
  fetchAttribIdx.reserve( mFields.size() );
  for ( int idx = 0, n = mFields.size(); idx < n; ++idx )
  {
    fetchAttribNames.append( mFields.at( idx ).name() );
    fetchAttribIdx.append( idx );
  }
  fetchGeometry = true;

  // Fetch 100 features at the time
  int startId = ( id / 100 ) * 100;
  int stopId = std::min( startId + 100, mObjectIds.length() );
  QList<quint32> objectIds;
  objectIds.reserve( stopId );
  for ( int i = startId; i < stopId; ++i )
  {
    objectIds.append( mObjectIds[i] );
  }


  // Query
  QString errorTitle, errorMessage;
  QVariantMap queryData = QgsArcGisRestUtils::getObjects(
                            mDataSource.param( QStringLiteral( "url" ) ), objectIds, mDataSource.param( QStringLiteral( "crs" ) ), fetchGeometry,
                            fetchAttribNames, QgsWkbTypes::hasM( mGeometryType ), QgsWkbTypes::hasZ( mGeometryType ),
                            filterRect, errorTitle, errorMessage );
  if ( queryData.isEmpty() )
  {
//    const_cast<QgsAfsProvider *>( this )->pushError( errorTitle + ": " + errorMessage );
    QgsDebugMsg( "Query returned empty result" );
    return false;
  }

  QVariantList featuresData = queryData[QStringLiteral( "features" )].toList();
  if ( featuresData.isEmpty() )
  {
    QgsDebugMsg( "Query returned no features" );
    return false;
  }
  for ( int i = 0, n = featuresData.size(); i < n; ++i )
  {
    QVariantMap featureData = featuresData[i].toMap();
    QgsFeature feature;

    // Set FID
    feature.setId( startId + i );

    // Set attributes
    if ( !fetchAttribIdx.isEmpty() )
    {
      QVariantMap attributesData = featureData[QStringLiteral( "attributes" )].toMap();
      feature.setFields( mFields );
      QgsAttributes attributes( mFields.size() );
      foreach ( int idx, fetchAttribIdx )
      {
        attributes[idx] = attributesData[mFields.at( idx ).name()];
      }
      feature.setAttributes( attributes );
    }

    // Set geometry
    if ( fetchGeometry )
    {
      QVariantMap geometryData = featureData[QStringLiteral( "geometry" )].toMap();
      QgsAbstractGeometry *geometry = QgsArcGisRestUtils::parseEsriGeoJSON( geometryData, queryData[QStringLiteral( "geometryType" )].toString(),
                                      QgsWkbTypes::hasM( mGeometryType ), QgsWkbTypes::hasZ( mGeometryType ) );
      // Above might return 0, which is OK since in theory empty geometries are allowed
      feature.setGeometry( QgsGeometry( geometry ) );
    }
    feature.setValid( true );
    mCache.insert( feature.id(), feature );
  }
  f = mCache[id];
  Q_ASSERT( f.isValid() );
  return filterRect.isNull() || ( f.hasGeometry() && f.geometry().intersects( filterRect ) );
}
