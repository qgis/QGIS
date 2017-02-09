/***************************************************************************
      qgsafsprovider.cpp
      ------------------
    begin                : May 27, 2015
    copyright            : (C) 2015 Sandro Mani
    email                : smani@sourcepole.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsafsprovider.h"
#include "qgsarcgisrestutils.h"
#include "qgsafsfeatureiterator.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "geometry/qgsgeometry.h"
#include "qgsnetworkaccessmanager.h"

#include <QEventLoop>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QNetworkReply>


QgsAfsProvider::QgsAfsProvider( const QString& uri )
    : QgsVectorDataProvider( uri )
    , mValid( false )
    , mGeometryType( QgsWkbTypes::Unknown )
    , mObjectIdFieldIdx( -1 )
{
  mDataSource = QgsDataSourceUri( uri );

  // Set CRS
  mSourceCRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( mDataSource.param( QStringLiteral( "crs" ) ) );

  // Get layer info
  QString errorTitle, errorMessage;
  QVariantMap layerData = QgsArcGisRestUtils::getLayerInfo( mDataSource.param( QStringLiteral( "url" ) ), errorTitle, errorMessage );
  if ( layerData.isEmpty() )
  {
    pushError( errorTitle + ": " + errorMessage );
    appendError( QgsErrorMessage( tr( "getLayerInfo failed" ), QStringLiteral( "AFSProvider" ) ) );
    return;
  }
  mLayerName = layerData[QStringLiteral( "name" )].toString();
  mLayerDescription = layerData[QStringLiteral( "description" )].toString();

  // Set extent
  QStringList coords = mDataSource.param( QStringLiteral( "bbox" ) ).split( QStringLiteral( "," ) );
  if ( coords.size() == 4 )
  {
    bool xminOk = false, yminOk = false, xmaxOk = false, ymaxOk = false;
    mExtent.setXMinimum( coords[0].toDouble( &xminOk ) );
    mExtent.setYMinimum( coords[1].toDouble( &yminOk ) );
    mExtent.setXMaximum( coords[2].toDouble( &xmaxOk ) );
    mExtent.setYMaximum( coords[3].toDouble( &ymaxOk ) );
    if ( !xminOk || !yminOk || !xmaxOk || !ymaxOk )
      mExtent = QgsRectangle();
  }
  if ( mExtent.isEmpty() )
  {
    QVariantMap layerExtentMap = layerData[QStringLiteral( "extent" )].toMap();
    bool xminOk = false, yminOk = false, xmaxOk = false, ymaxOk = false;
    mExtent.setXMinimum( layerExtentMap[QStringLiteral( "xmin" )].toDouble( &xminOk ) );
    mExtent.setYMinimum( layerExtentMap[QStringLiteral( "ymin" )].toDouble( &yminOk ) );
    mExtent.setXMaximum( layerExtentMap[QStringLiteral( "xmax" )].toDouble( &xmaxOk ) );
    mExtent.setYMaximum( layerExtentMap[QStringLiteral( "ymax" )].toDouble( &ymaxOk ) );
    if ( !xminOk || !yminOk || !xmaxOk || !ymaxOk )
    {
      appendError( QgsErrorMessage( tr( "Could not retrieve layer extent" ), QStringLiteral( "AFSProvider" ) ) );
      return;
    }
    QgsCoordinateReferenceSystem extentCrs = QgsArcGisRestUtils::parseSpatialReference( layerExtentMap[QStringLiteral( "spatialReference" )].toMap() );
    if ( !extentCrs.isValid() )
    {
      appendError( QgsErrorMessage( tr( "Could not parse spatial reference" ), QStringLiteral( "AFSProvider" ) ) );
      return;
    }
    mExtent = QgsCoordinateTransform( extentCrs, mSourceCRS ).transformBoundingBox( mExtent );
  }

  // Read fields
  foreach ( const QVariant& fieldData, layerData["fields"].toList() )
  {
    QVariantMap fieldDataMap = fieldData.toMap();
    QString fieldName = fieldDataMap[QStringLiteral( "name" )].toString();
    QVariant::Type type = QgsArcGisRestUtils::mapEsriFieldType( fieldDataMap[QStringLiteral( "type" )].toString() );
    if ( fieldName == QLatin1String( "geometry" ) || type == QVariant::Invalid )
    {
      QgsDebugMsg( QString( "Skipping unsupported (or possibly geometry) field" ).arg( fieldName ) );
      continue;
    }
    QgsField field( fieldName, type, fieldDataMap[QStringLiteral( "type" )].toString(), fieldDataMap[QStringLiteral( "length" )].toInt() );
    mFields.append( field );
  }

  // Determine geometry type
  bool hasM = layerData[QStringLiteral( "hasM" )].toBool();
  bool hasZ = layerData[QStringLiteral( "hasZ" )].toBool();
  mGeometryType = QgsArcGisRestUtils::mapEsriGeometryType( layerData[QStringLiteral( "geometryType" )].toString() );
  if ( mGeometryType == QgsWkbTypes::Unknown )
  {
    appendError( QgsErrorMessage( tr( "Failed to determine geometry type" ), QStringLiteral( "AFSProvider" ) ) );
    return;
  }
  mGeometryType = QgsWkbTypes::zmType( mGeometryType, hasZ, hasM );

  // Read OBJECTIDs of all features: these may not be a continuous sequence,
  // and we need to store these to iterate through the features. This query
  // also returns the name of the ObjectID field.
  QVariantMap objectIdData = QgsArcGisRestUtils::getObjectIds( mDataSource.param( QStringLiteral( "url" ) ), errorTitle, errorMessage );
  if ( objectIdData.isEmpty() )
  {
    appendError( QgsErrorMessage( tr( "getObjectIds failed: %1 - %2" ).arg( errorTitle, errorMessage ), QStringLiteral( "AFSProvider" ) ) );
    return;
  }
  if ( !objectIdData[QStringLiteral( "objectIdFieldName" )].isValid() || !objectIdData[QStringLiteral( "objectIds" )].isValid() )
  {
    appendError( QgsErrorMessage( tr( "Failed to determine objectIdFieldName and/or objectIds" ), QStringLiteral( "AFSProvider" ) ) );
    return;
  }
  mObjectIdFieldName = objectIdData[QStringLiteral( "objectIdFieldName" )].toString();
  for ( int idx = 0, nIdx = mFields.count(); idx < nIdx; ++idx )
  {
    if ( mFields.at( idx ).name() == mObjectIdFieldName )
    {
      mObjectIdFieldIdx = idx;

      // primary key is not null, unique
      QgsFieldConstraints constraints = mFields.at( idx ).constraints();
      constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
      constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
      mFields[ idx ].setConstraints( constraints );

      break;
    }
  }
  foreach ( const QVariant& objectId, objectIdData["objectIds"].toList() )
  {
    mObjectIds.append( objectId.toInt() );
  }

  mValid = true;
}

QgsAbstractFeatureSource* QgsAfsProvider::featureSource() const
{
  return new QgsAfsFeatureSource( this );
}

QgsFeatureIterator QgsAfsProvider::getFeatures( const QgsFeatureRequest& request ) const
{
  return new QgsAfsFeatureIterator( new QgsAfsFeatureSource( this ), true, request );
}

bool QgsAfsProvider::getFeature( QgsFeatureId id, QgsFeature &f, bool fetchGeometry, const QList<int>& /*fetchAttributes*/, const QgsRectangle& filterRect )
{
  // If cached, return cached feature
  QMap<QgsFeatureId, QgsFeature>::const_iterator it = mCache.find( id );
  if ( it != mCache.end() )
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
  int stopId = qMin( startId + 100, mObjectIds.length() );
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
    const_cast<QgsAfsProvider*>( this )->pushError( errorTitle + ": " + errorMessage );
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
      QgsAbstractGeometry* geometry = QgsArcGisRestUtils::parseEsriGeoJSON( geometryData, queryData[QStringLiteral( "geometryType" )].toString(),
                                      QgsWkbTypes::hasM( mGeometryType ), QgsWkbTypes::hasZ( mGeometryType ) );
      // Above might return 0, which is ok since in theory empty geometries are allowed
      feature.setGeometry( QgsGeometry( geometry ) );
    }
    feature.setValid( true );
    mCache.insert( feature.id(), feature );
  }
  f = mCache[id];
  Q_ASSERT( f.isValid() );
  return filterRect.isNull() || ( f.hasGeometry() && f.geometry().intersects( filterRect ) );
}

void QgsAfsProvider::setDataSourceUri( const QString &uri )
{
  mDataSource = QgsDataSourceUri( uri );
  QgsDataProvider::setDataSourceUri( uri );
}
