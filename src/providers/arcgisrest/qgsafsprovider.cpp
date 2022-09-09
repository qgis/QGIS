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
#include "qgsarcgisrestquery.h"
#include "qgsafsfeatureiterator.h"
#include "qgsdatasourceuri.h"
#include "qgsarcgisrestdataitems.h"
#include "qgslogger.h"
#include "qgsdataitemprovider.h"
#include "qgsapplication.h"
#include "qgsruntimeprofiler.h"
#include "qgsfeedback.h"
#include "qgsreadwritelocker.h"
#include "qgsvariantutils.h"

const QString QgsAfsProvider::AFS_PROVIDER_KEY = QStringLiteral( "arcgisfeatureserver" );
const QString QgsAfsProvider::AFS_PROVIDER_DESCRIPTION = QStringLiteral( "ArcGIS Feature Service data provider" );


QgsAfsProvider::QgsAfsProvider( const QString &uri, const ProviderOptions &options, QgsDataProvider::ReadFlags flags )
  : QgsVectorDataProvider( uri, options, flags )
{
  mSharedData.reset( new QgsAfsSharedData( QgsDataSourceUri( uri ) ) );
  mSharedData->mGeometryType = QgsWkbTypes::Unknown;

  const QString authcfg = mSharedData->mDataSource.authConfigId();

  // Set CRS
  if ( !mSharedData->mDataSource.param( QStringLiteral( "crs" ) ).isEmpty() )
    mSharedData->mSourceCRS.createFromString( mSharedData->mDataSource.param( QStringLiteral( "crs" ) ) );

  // Get layer info
  QString errorTitle, errorMessage;

  mRequestHeaders = mSharedData->mDataSource.httpHeaders();

  std::unique_ptr< QgsScopedRuntimeProfile > profile;
  if ( QgsApplication::profiler()->groupIsActive( QStringLiteral( "projectload" ) ) )
    profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Retrieve service definition" ), QStringLiteral( "projectload" ) );

  const QVariantMap layerData = QgsArcGisRestQueryUtils::getLayerInfo( mSharedData->mDataSource.param( QStringLiteral( "url" ) ),
                                authcfg, errorTitle, errorMessage, mRequestHeaders );
  if ( layerData.isEmpty() )
  {
    pushError( errorTitle + ": " + errorMessage );
    appendError( QgsErrorMessage( tr( "getLayerInfo failed" ), QStringLiteral( "AFSProvider" ) ) );
    return;
  }
  mLayerName = layerData[QStringLiteral( "name" )].toString();
  mLayerDescription = layerData[QStringLiteral( "description" )].toString();
  mCapabilityStrings = layerData[QStringLiteral( "capabilities" )].toString().split( ',' );

  if ( mCapabilityStrings.contains( QLatin1String( "update" ), Qt::CaseInsensitive ) )
  {
    // if the user has update capability, see if this extends to field definition modification
    QString adminUrl = mSharedData->mDataSource.param( QStringLiteral( "url" ) );

    // note that for hosted ArcGIS Server the admin url may not match this format (which is how it works for AGOL).
    // we may want to consider exposing the admin url as an option for users to set
    if ( adminUrl.contains( QStringLiteral( "/rest/services/" ) ) )
    {
      adminUrl.replace( QLatin1String( "/rest/services/" ), QLatin1String( "/rest/admin/services/" ) );
      const QVariantMap adminData = QgsArcGisRestQueryUtils::getLayerInfo( adminUrl,
                                    authcfg, errorTitle, errorMessage, mRequestHeaders );
      if ( !adminData.isEmpty() )
      {
        mAdminUrl = adminUrl;
        mAdminData = adminData;
        mAdminCapabilityStrings = mAdminData.value( QStringLiteral( "capabilities" ) ).toString().split( ',' );
      }
    }
  }

  mServerSupportsCurves = layerData.value( QStringLiteral( "allowTrueCurvesUpdates" ), false ).toBool();

  // Set extent
  QStringList coords = mSharedData->mDataSource.param( QStringLiteral( "bbox" ) ).split( ',' );
  if ( coords.size() == 4 )
  {
    bool xminOk = false, yminOk = false, xmaxOk = false, ymaxOk = false;
    mSharedData->mExtent.setXMinimum( coords[0].toDouble( &xminOk ) );
    mSharedData->mExtent.setYMinimum( coords[1].toDouble( &yminOk ) );
    mSharedData->mExtent.setXMaximum( coords[2].toDouble( &xmaxOk ) );
    mSharedData->mExtent.setYMaximum( coords[3].toDouble( &ymaxOk ) );
    if ( !xminOk || !yminOk || !xmaxOk || !ymaxOk )
      mSharedData->mExtent = QgsRectangle();
    else
      mSharedData->mLimitBBox = true;
  }

  const QVariantMap layerExtentMap = layerData[QStringLiteral( "extent" )].toMap();
  bool xminOk = false, yminOk = false, xmaxOk = false, ymaxOk = false;
  QgsRectangle originalExtent;
  originalExtent.setXMinimum( layerExtentMap[QStringLiteral( "xmin" )].toDouble( &xminOk ) );
  originalExtent.setYMinimum( layerExtentMap[QStringLiteral( "ymin" )].toDouble( &yminOk ) );
  originalExtent.setXMaximum( layerExtentMap[QStringLiteral( "xmax" )].toDouble( &xmaxOk ) );
  originalExtent.setYMaximum( layerExtentMap[QStringLiteral( "ymax" )].toDouble( &ymaxOk ) );
  if ( mSharedData->mExtent.isEmpty() && ( !xminOk || !yminOk || !xmaxOk || !ymaxOk ) )
  {
    appendError( QgsErrorMessage( tr( "Could not retrieve layer extent" ), QStringLiteral( "AFSProvider" ) ) );
    return;
  }
  const QgsCoordinateReferenceSystem extentCrs = QgsArcGisRestUtils::convertSpatialReference( layerExtentMap[QStringLiteral( "spatialReference" )].toMap() );
  if ( mSharedData->mExtent.isEmpty() && !extentCrs.isValid() )
  {
    appendError( QgsErrorMessage( tr( "Could not parse spatial reference" ), QStringLiteral( "AFSProvider" ) ) );
    return;
  }

  if ( !mSharedData->mSourceCRS.isValid() )
    mSharedData->mSourceCRS = extentCrs;

  if ( xminOk && yminOk && xmaxOk && ymaxOk )
  {
    QgsLayerMetadata::SpatialExtent spatialExtent;
    spatialExtent.bounds = QgsBox3d( originalExtent );
    spatialExtent.extentCrs = extentCrs;
    QgsLayerMetadata::Extent metadataExtent;
    metadataExtent.setSpatialExtents( QList<  QgsLayerMetadata::SpatialExtent >() << spatialExtent );
    mLayerMetadata.setExtent( metadataExtent );
  }
  if ( extentCrs.isValid() )
  {
    mLayerMetadata.setCrs( extentCrs );
  }

  if ( mSharedData->mExtent.isEmpty() )
  {
    mSharedData->mExtent = originalExtent;
    QgsCoordinateTransform ct( extentCrs, mSharedData->mSourceCRS, options.transformContext );
    ct.setBallparkTransformsAreAppropriate( true );
    try
    {
      mSharedData->mExtent = ct.transformBoundingBox( mSharedData->mExtent );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Exception raised while transforming layer extent" ) );
    }
  }

  QString objectIdFieldName;

  // Read fields
  const QVariantList fields = layerData.value( QStringLiteral( "fields" ) ).toList();
  for ( const QVariant &fieldData : fields )
  {
    const QVariantMap fieldDataMap = fieldData.toMap();
    const QString fieldName = fieldDataMap[QStringLiteral( "name" )].toString();
    const QString fieldAlias = fieldDataMap[QStringLiteral( "alias" )].toString();
    const QString fieldTypeString = fieldDataMap[QStringLiteral( "type" )].toString();
    const QVariant::Type type = QgsArcGisRestUtils::convertFieldType( fieldTypeString );
    if ( fieldName == QLatin1String( "geometry" ) || fieldTypeString == QLatin1String( "esriFieldTypeGeometry" ) )
    {
      // skip geometry field
      continue;
    }
    if ( fieldTypeString == QLatin1String( "esriFieldTypeOID" ) )
    {
      objectIdFieldName = fieldName;
    }
    if ( type == QVariant::Invalid )
    {
      QgsDebugMsg( QStringLiteral( "Skipping unsupported field %1 of type %2" ).arg( fieldName, fieldTypeString ) );
      continue;
    }
    QgsField field( fieldName, type, fieldDataMap[QStringLiteral( "type" )].toString(), fieldDataMap[QStringLiteral( "length" )].toInt() );
    if ( !fieldAlias.isEmpty() && fieldAlias != fieldName )
      field.setAlias( fieldAlias );

    if ( fieldDataMap.contains( QStringLiteral( "domain" ) ) && fieldDataMap.value( QStringLiteral( "domain" ) ).toMap().value( QStringLiteral( "type" ) ).toString() == QLatin1String( "codedValue" ) )
    {
      const QVariantList values = fieldDataMap.value( QStringLiteral( "domain" ) ).toMap().value( QStringLiteral( "codedValues" ) ).toList();
      QVariantList valueConfig;
      valueConfig.reserve( values.count() );
      for ( const QVariant &v : values )
      {
        const QVariantMap value = v.toMap();
        QVariantMap config;
        config[ value.value( QStringLiteral( "name" ) ).toString() ] = value.value( QStringLiteral( "code" ) );
        valueConfig.append( config );
      }
      QVariantMap editorConfig;
      editorConfig.insert( QStringLiteral( "map" ), valueConfig );
      field.setEditorWidgetSetup( QgsEditorWidgetSetup( QStringLiteral( "ValueMap" ), editorConfig ) );
    }

    if ( fieldDataMap.contains( QStringLiteral( "editable" ) ) && !fieldDataMap.value( QStringLiteral( "editable" ) ).toBool() )
    {
      field.setReadOnly( true );
    }
    if ( !field.isReadOnly() && fieldDataMap.contains( QStringLiteral( "nullable" ) ) && !fieldDataMap.value( QStringLiteral( "nullable" ) ).toBool() )
    {
      QgsFieldConstraints constraints;
      constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
      field.setConstraints( constraints );
    }

    mSharedData->mFields.append( field );
  }
  if ( objectIdFieldName.isEmpty() )
    objectIdFieldName = QStringLiteral( "objectid" );

  // Determine geometry type
  const bool hasM = layerData[QStringLiteral( "hasM" )].toBool();
  const bool hasZ = layerData[QStringLiteral( "hasZ" )].toBool();
  mSharedData->mGeometryType = QgsArcGisRestUtils::convertGeometryType( layerData[QStringLiteral( "geometryType" )].toString() );
  if ( mSharedData->mGeometryType == QgsWkbTypes::Unknown )
  {
    if ( layerData.value( QStringLiteral( "serviceDataType" ) ).toString().startsWith( QLatin1String( "esriImageService" ) ) )
    {
      // it's possible to connect to ImageServers as a feature service, to view tile boundaries
      mSharedData->mGeometryType = QgsWkbTypes::Polygon;
    }
    else
    {
      appendError( QgsErrorMessage( tr( "Failed to determine geometry type" ), QStringLiteral( "AFSProvider" ) ) );
      return;
    }
  }
  mSharedData->mGeometryType = QgsWkbTypes::zmType( mSharedData->mGeometryType, hasZ, hasM );

  // read temporal properties
  if ( layerData.contains( QStringLiteral( "timeInfo" ) ) )
  {
    const QVariantMap timeInfo = layerData.value( QStringLiteral( "timeInfo" ) ).toMap();

    temporalCapabilities()->setHasTemporalCapabilities( true );
    temporalCapabilities()->setStartField( timeInfo.value( QStringLiteral( "startTimeField" ) ).toString() );
    temporalCapabilities()->setEndField( timeInfo.value( QStringLiteral( "endTimeField" ) ).toString() );
    if ( !temporalCapabilities()->endField().isEmpty() )
      temporalCapabilities()->setMode( Qgis::VectorDataProviderTemporalMode::StoresFeatureDateTimeStartAndEndInSeparateFields );
    else if ( !temporalCapabilities()->startField().isEmpty() )
      temporalCapabilities()->setMode( Qgis::VectorDataProviderTemporalMode::StoresFeatureDateTimeInstantInField );
    else
      temporalCapabilities()->setMode( Qgis::VectorDataProviderTemporalMode::HasFixedTemporalRange );

    const QVariantList extent = timeInfo.value( QStringLiteral( "timeExtent" ) ).toList();
    if ( extent.size() == 2 )
    {
      temporalCapabilities()->setAvailableTemporalRange( QgsDateTimeRange( QgsArcGisRestUtils::convertDateTime( extent.at( 0 ) ),
          QgsArcGisRestUtils::convertDateTime( extent.at( 1 ) ) ) );
    }
  }

  QList<QgsVectorDataProvider::NativeType> types
  {
    QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::Int ), QStringLiteral( "esriFieldTypeSmallInteger" ), QVariant::Int, -1, -1, 0, 0 ),
    QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::LongLong ), QStringLiteral( "esriFieldTypeInteger" ), QVariant::LongLong, -1, -1, 0, 0 ),
    QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::Double ), QStringLiteral( "esriFieldTypeDouble" ), QVariant::Double, 1, 20, 0, 20 ),
    QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::String ), QStringLiteral( "esriFieldTypeString" ), QVariant::String, -1, -1, -1, -1 ),
    QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::DateTime ), QStringLiteral( "esriFieldTypeDate" ), QVariant::DateTime, -1, -1, -1, -1 ),
    QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QVariant::ByteArray ), QStringLiteral( "esriFieldTypeBlob" ), QVariant::ByteArray, -1, -1, -1, -1 )
  };
  setNativeTypes( types );

  if ( profile )
    profile->switchTask( tr( "Retrieve object IDs" ) );

  // Read OBJECTIDs of all features: these may not be a continuous sequence,
  // and we need to store these to iterate through the features. This query
  // also returns the name of the ObjectID field.
  if ( ! mSharedData->getObjectIds( errorMessage ) )
  {
    appendError( QgsErrorMessage( errorMessage, QStringLiteral( "AFSProvider" ) ) );
    return;
  }

  // layer metadata

  mLayerMetadata.setIdentifier( mSharedData->mDataSource.param( QStringLiteral( "url" ) ) );
  const QString parentIdentifier = layerData[QStringLiteral( "parentLayer" )].toMap().value( QStringLiteral( "id" ) ).toString();
  if ( !parentIdentifier.isEmpty() )
  {
    const QString childUrl = mSharedData->mDataSource.param( QStringLiteral( "url" ) );
    const QString parentUrl = childUrl.left( childUrl.lastIndexOf( '/' ) ) + '/' + parentIdentifier;
    mLayerMetadata.setParentIdentifier( parentUrl );
  }
  mLayerMetadata.setType( QStringLiteral( "dataset" ) );
  mLayerMetadata.setAbstract( mLayerDescription );
  mLayerMetadata.setTitle( mLayerName );
  const QString copyright = layerData[QStringLiteral( "copyrightText" )].toString();
  if ( !copyright.isEmpty() )
    mLayerMetadata.setRights( QStringList() << copyright );
  mLayerMetadata.addLink( QgsAbstractMetadataBase::Link( tr( "Source" ), QStringLiteral( "WWW:LINK" ), mSharedData->mDataSource.param( QStringLiteral( "url" ) ) ) );

  // renderer
  mRendererDataMap = layerData.value( QStringLiteral( "drawingInfo" ) ).toMap().value( QStringLiteral( "renderer" ) ).toMap();
  mLabelingDataList = layerData.value( QStringLiteral( "drawingInfo" ) ).toMap().value( QStringLiteral( "labelingInfo" ) ).toList();

  mValid = true;
}

QgsAbstractFeatureSource *QgsAfsProvider::featureSource() const
{
  return new QgsAfsFeatureSource( mSharedData );
}

QgsFeatureIterator QgsAfsProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  return new QgsAfsFeatureIterator( new QgsAfsFeatureSource( mSharedData ), true, request );
}

QgsWkbTypes::Type QgsAfsProvider::wkbType() const
{
  return mSharedData->mGeometryType;
}

long long QgsAfsProvider::featureCount() const
{
  return mSharedData->featureCount();
}

QgsFields QgsAfsProvider::fields() const
{
  return mSharedData->mFields;
}

QgsLayerMetadata QgsAfsProvider::layerMetadata() const
{
  return mLayerMetadata;
}

bool QgsAfsProvider::deleteFeatures( const QgsFeatureIds &ids )
{
  if ( !mCapabilityStrings.contains( QLatin1String( "delete" ), Qt::CaseInsensitive ) )
    return false;

  QString error;
  QgsFeedback feedback;
  const bool result = mSharedData->deleteFeatures( ids, error, &feedback );
  if ( !result )
    pushError( tr( "Error while deleting features: %1" ).arg( error ) );
  else
    clearMinMaxCache();

  return result;
}

bool QgsAfsProvider::addFeatures( QgsFeatureList &flist, Flags )
{
  if ( !mCapabilityStrings.contains( QLatin1String( "create" ), Qt::CaseInsensitive ) )
    return false;

  if ( flist.isEmpty() )
    return true; // for consistency!

  QString error;
  QgsFeedback feedback;
  const bool res = mSharedData->addFeatures( flist, error, &feedback );
  if ( !res )
    pushError( tr( "Error while adding features: %1" ).arg( error ) );
  else
    clearMinMaxCache();

  return res;
}

bool QgsAfsProvider::changeAttributeValues( const QgsChangedAttributesMap &attrMap )
{
  if ( !mCapabilityStrings.contains( QLatin1String( "update" ), Qt::CaseInsensitive ) )
    return false;

  QgsFeatureIds ids;
  ids.reserve( attrMap.size() );
  for ( auto it = attrMap.constBegin(); it != attrMap.constEnd(); ++it )
  {
    const QgsFeatureId id = it.key();
    ids << id;
  }

  // REST API requires a full definition of features, so we have to read their initial values first
  QgsFeatureIterator it = getFeatures( QgsFeatureRequest().setFilterFids( ids ).setFlags( QgsFeatureRequest::NoGeometry ) );
  QgsFeature feature;

  QgsFeatureList updatedFeatures;
  updatedFeatures.reserve( attrMap.size() );

  const int objectIdFieldIndex = mSharedData->mObjectIdFieldIdx;

  while ( it.nextFeature( feature ) )
  {
    QgsFeature modifiedFeature = feature;

    const QgsAttributeMap modifiedAttributes = attrMap.value( feature.id() );
    for ( auto aIt = modifiedAttributes.constBegin(); aIt != modifiedAttributes.constEnd(); ++aIt )
    {
      if ( aIt.key() == objectIdFieldIndex )
        continue; // can't modify the objectId field!

      modifiedFeature.setAttribute( aIt.key(), aIt.value() );
    }

    updatedFeatures.append( modifiedFeature );
  }

  QString error;
  QgsFeedback feedback;
  const bool res = mSharedData->updateFeatures( updatedFeatures, false, true, error, &feedback );
  if ( !res )
    pushError( tr( "Error while updating features: %1" ).arg( error ) );
  else
    clearMinMaxCache();

  return res;
}

bool QgsAfsProvider::changeGeometryValues( const QgsGeometryMap &geometryMap )
{
  if ( !mCapabilityStrings.contains( QLatin1String( "update" ), Qt::CaseInsensitive ) )
    return false;

  const QgsFields fields = mSharedData->mFields;
  const int objectIdFieldIndex = mSharedData->mObjectIdFieldIdx;

  QgsFeatureList updatedFeatures;
  updatedFeatures.reserve( geometryMap.size() );

  // grab a lock upfront so we aren't trying to acquire for each feature
  QgsReadWriteLocker locker( mSharedData->mReadWriteLock, QgsReadWriteLocker::Read );
  for ( auto it = geometryMap.constBegin(); it != geometryMap.constEnd(); ++it )
  {
    const QgsFeatureId id = it.key();
    QgsFeature feature( fields );
    feature.setId( id );
    // we ONLY require the objectId field set here
    feature.setAttribute( objectIdFieldIndex, mSharedData->featureIdToObjectId( id ) );
    feature.setGeometry( it.value() );

    updatedFeatures.append( feature );
  }
  locker.unlock();

  QString error;
  QgsFeedback feedback;
  const bool res = mSharedData->updateFeatures( updatedFeatures, true, false, error, &feedback );
  if ( !res )
    pushError( tr( "Error while updating features: %1" ).arg( error ) );

  return res;
}

bool QgsAfsProvider::changeFeatures( const QgsChangedAttributesMap &attrMap, const QgsGeometryMap &geometryMap )
{
  if ( !mCapabilityStrings.contains( QLatin1String( "update" ), Qt::CaseInsensitive ) )
    return false;

  QgsFeatureIds ids;
  ids.reserve( attrMap.size() + geometryMap.size() );
  for ( auto it = attrMap.constBegin(); it != attrMap.constEnd(); ++it )
  {
    const QgsFeatureId id = it.key();
    ids << id;
  }
  for ( auto it = geometryMap.constBegin(); it != geometryMap.constEnd(); ++it )
  {
    const QgsFeatureId id = it.key();
    ids << id;
  }

  // REST API requires a full definition of features, so we have to read their initial values first
  QgsFeatureIterator it = getFeatures( QgsFeatureRequest().setFilterFids( ids ) );
  QgsFeature feature;

  QgsFeatureList updatedFeatures;
  updatedFeatures.reserve( attrMap.size() );
  const int objectIdFieldIndex = mSharedData->mObjectIdFieldIdx;

  while ( it.nextFeature( feature ) )
  {
    QgsFeature modifiedFeature = feature;

    const QgsAttributeMap modifiedAttributes = attrMap.value( feature.id() );
    for ( auto aIt = modifiedAttributes.constBegin(); aIt != modifiedAttributes.constEnd(); ++aIt )
    {
      if ( aIt.key() == objectIdFieldIndex )
        continue; // can't modify the objectId field!

      modifiedFeature.setAttribute( aIt.key(), aIt.value() );
    }

    const auto geomIt = geometryMap.constFind( feature.id() );
    if ( geomIt != geometryMap.constEnd() )
    {
      modifiedFeature.setGeometry( geomIt.value() );
    }

    updatedFeatures.append( modifiedFeature );
  }

  QString error;
  QgsFeedback feedback;
  const bool res = mSharedData->updateFeatures( updatedFeatures, true, true, error, &feedback );
  if ( !res )
    pushError( tr( "Error while updating features: %1" ).arg( error ) );
  else
    clearMinMaxCache();

  return res;
}

bool QgsAfsProvider::addAttributes( const QList<QgsField> &attributes )
{
  if ( mAdminUrl.isEmpty() )
    return false;

  if ( !mAdminCapabilityStrings.contains( QLatin1String( "update" ), Qt::CaseInsensitive ) )
    return false;

  QString error;
  QgsFeedback feedback;
  const bool res = mSharedData->addFields( mAdminUrl, attributes, error, &feedback );
  if ( !res )
    pushError( tr( "Error while adding fields: %1" ).arg( error ) );

  return true;
}

bool QgsAfsProvider::deleteAttributes( const QgsAttributeIds &attributes )
{
  if ( mAdminUrl.isEmpty() )
    return false;

  if ( !mAdminCapabilityStrings.contains( QLatin1String( "delete" ), Qt::CaseInsensitive ) )
    return false;

  QString error;
  QgsFeedback feedback;
  const bool res = mSharedData->deleteFields( mAdminUrl, attributes, error, &feedback );
  if ( !res )
    pushError( tr( "Error while deleting fields: %1" ).arg( error ) );

  return res;
}

bool QgsAfsProvider::createAttributeIndex( int field )
{
  if ( mAdminUrl.isEmpty() )
    return false;

  if ( !mAdminCapabilityStrings.contains( QLatin1String( "update" ), Qt::CaseInsensitive ) )
    return false;

  if ( field < 0 || field >= mSharedData->mFields.count() )
  {
    return false;
  }

  QString error;
  QgsFeedback feedback;
  const bool res = mSharedData->addAttributeIndex( mAdminUrl, field, error, &feedback );
  if ( !res )
    pushError( tr( "Error while creating attribute index: %1" ).arg( error ) );

  return true;
}

QgsVectorDataProvider::Capabilities QgsAfsProvider::capabilities() const
{
  QgsVectorDataProvider::Capabilities c = QgsVectorDataProvider::SelectAtId
                                          | QgsVectorDataProvider::ReadLayerMetadata
                                          | QgsVectorDataProvider::Capability::ReloadData;
  if ( !mRendererDataMap.empty() )
  {
    c = c | QgsVectorDataProvider::CreateRenderer;
  }
  if ( !mLabelingDataList.empty() )
  {
    c = c | QgsVectorDataProvider::CreateLabeling;
  }

  if ( mServerSupportsCurves )
    c |= QgsVectorDataProvider::CircularGeometries;

  if ( mCapabilityStrings.contains( QLatin1String( "delete" ), Qt::CaseInsensitive ) )
  {
    c |= QgsVectorDataProvider::DeleteFeatures;
  }
  if ( mCapabilityStrings.contains( QLatin1String( "create" ), Qt::CaseInsensitive ) )
  {
    c |= QgsVectorDataProvider::AddFeatures;
  }
  if ( mCapabilityStrings.contains( QLatin1String( "update" ), Qt::CaseInsensitive ) )
  {
    c |= QgsVectorDataProvider::ChangeAttributeValues;
    c |= QgsVectorDataProvider::ChangeFeatures;
    c |= QgsVectorDataProvider::ChangeGeometries;
  }

  if ( mAdminCapabilityStrings.contains( QLatin1String( "update" ), Qt::CaseInsensitive ) )
  {
    c |= QgsVectorDataProvider::AddAttributes;
    c |= QgsVectorDataProvider::CreateAttributeIndex;
  }
  if ( mAdminCapabilityStrings.contains( QLatin1String( "delete" ), Qt::CaseInsensitive ) )
  {
    c |= QgsVectorDataProvider::DeleteAttributes;
  }

  return c;
}

QgsAttributeList QgsAfsProvider::pkAttributeIndexes() const
{
  return QgsAttributeList() << mSharedData->mObjectIdFieldIdx;
}

QString QgsAfsProvider::defaultValueClause( int fieldId ) const
{
  if ( fieldId == mSharedData->mObjectIdFieldIdx )
    return QStringLiteral( "Autogenerate" );
  return QString();
}

bool QgsAfsProvider::skipConstraintCheck( int fieldIndex, QgsFieldConstraints::Constraint, const QVariant &value ) const
{
  return fieldIndex == mSharedData->mObjectIdFieldIdx && value.toString() == QLatin1String( "Autogenerate" );
}

QString QgsAfsProvider::subsetString() const
{
  return mSharedData->subsetString();
}

bool QgsAfsProvider::setSubsetString( const QString &subset, bool )
{
  const QString trimmedSubset = subset.trimmed();
  if ( trimmedSubset == mSharedData->subsetString() )
    return true;

  mSharedData->setSubsetString( trimmedSubset );

  // Update datasource uri too
  QgsDataSourceUri uri = dataSourceUri();
  uri.setSql( trimmedSubset );
  setDataSourceUri( uri.uri( false ) );

  clearMinMaxCache();

  emit dataChanged();

  return true;
}

void QgsAfsProvider::setDataSourceUri( const QString &uri )
{
  mSharedData->mDataSource = QgsDataSourceUri( uri );
  QgsDataProvider::setDataSourceUri( uri );
}

QgsCoordinateReferenceSystem QgsAfsProvider::crs() const
{
  return mSharedData->crs();
}

QgsRectangle QgsAfsProvider::extent() const
{
  return mSharedData->extent();
}

QString QgsAfsProvider::name() const
{
  return AFS_PROVIDER_KEY;
}

QString QgsAfsProvider::providerKey()
{
  return AFS_PROVIDER_KEY;
}

void QgsAfsProvider::handlePostCloneOperations( QgsVectorDataProvider *source )
{
  mSharedData = qobject_cast<QgsAfsProvider *>( source )->mSharedData;
}

QString QgsAfsProvider::description() const
{
  return AFS_PROVIDER_DESCRIPTION;
}

QString QgsAfsProvider::dataComment() const
{
  return mLayerDescription;
}

void QgsAfsProvider::reloadProviderData()
{
  mSharedData->clearCache();
}

QgsFeatureRenderer *QgsAfsProvider::createRenderer( const QVariantMap & ) const
{
  return QgsArcGisRestUtils::convertRenderer( mRendererDataMap );
}

QgsAbstractVectorLayerLabeling *QgsAfsProvider::createLabeling( const QVariantMap & ) const
{
  return QgsArcGisRestUtils::convertLabeling( mLabelingDataList );
}

bool QgsAfsProvider::renderInPreview( const QgsDataProvider::PreviewContext & )
{
  // these servers can be sloooooooow, and unpredictable. The previous preview job may have been fast to render,
  // but the next may take minutes or worse to download...
  return false;
}


QgsAfsProviderMetadata::QgsAfsProviderMetadata():
  QgsProviderMetadata( QgsAfsProvider::AFS_PROVIDER_KEY, QgsAfsProvider::AFS_PROVIDER_DESCRIPTION )
{
}

QIcon QgsAfsProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconAfs.svg" ) );
}

QList<QgsDataItemProvider *> QgsAfsProviderMetadata::dataItemProviders() const
{
  QList<QgsDataItemProvider *> providers;

  providers
      << new QgsArcGisRestDataItemProvider;

  return providers;
}

QVariantMap QgsAfsProviderMetadata::decodeUri( const QString &uri ) const
{
  const QgsDataSourceUri dsUri = QgsDataSourceUri( uri );

  QVariantMap components;
  components.insert( QStringLiteral( "url" ), dsUri.param( QStringLiteral( "url" ) ) );
  const QStringList bbox = dsUri.param( QStringLiteral( "bbox" ) ).split( ',' );
  if ( bbox.size() == 4 )
  {
    QgsRectangle r;
    bool xminOk = false;
    bool yminOk = false;
    bool xmaxOk = false;
    bool ymaxOk = false;
    r.setXMinimum( bbox[0].toDouble( &xminOk ) );
    r.setYMinimum( bbox[1].toDouble( &yminOk ) );
    r.setXMaximum( bbox[2].toDouble( &xmaxOk ) );
    r.setYMaximum( bbox[3].toDouble( &ymaxOk ) );
    if ( xminOk && yminOk && xmaxOk && ymaxOk )
      components.insert( QStringLiteral( "bounds" ), r );
  }

  dsUri.httpHeaders().updateMap( components );

  if ( !dsUri.param( QStringLiteral( "crs" ) ).isEmpty() )
  {
    components.insert( QStringLiteral( "crs" ), dsUri.param( QStringLiteral( "crs" ) ) );
  }
  if ( !dsUri.authConfigId().isEmpty() )
  {
    components.insert( QStringLiteral( "authcfg" ), dsUri.authConfigId() );
  }
  return components;
}

QString QgsAfsProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setParam( QStringLiteral( "url" ), parts.value( QStringLiteral( "url" ) ).toString() );

  if ( parts.contains( QStringLiteral( "bounds" ) ) && parts.value( QStringLiteral( "bounds" ) ).userType() == QMetaType::type( "QgsRectangle" ) )
  {
    const QgsRectangle bBox = parts.value( QStringLiteral( "bounds" ) ).value< QgsRectangle >();
    dsUri.setParam( QStringLiteral( "bbox" ), QStringLiteral( "%1,%2,%3,%4" ).arg( bBox.xMinimum() ).arg( bBox.yMinimum() ).arg( bBox.xMaximum() ).arg( bBox.yMaximum() ) );
  }

  if ( !parts.value( QStringLiteral( "crs" ) ).toString().isEmpty() )
  {
    dsUri.setParam( QStringLiteral( "crs" ), parts.value( QStringLiteral( "crs" ) ).toString() );
  }

  dsUri.httpHeaders().setFromMap( parts );

  if ( !parts.value( QStringLiteral( "authcfg" ) ).toString().isEmpty() )
  {
    dsUri.setAuthConfigId( parts.value( QStringLiteral( "authcfg" ) ).toString() );
  }
  return dsUri.uri( false );
}

QgsAfsProvider *QgsAfsProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsAfsProvider( uri, options, flags );
}

QList<QgsMapLayerType> QgsAfsProviderMetadata::supportedLayerTypes() const
{
  return { QgsMapLayerType::VectorLayer };
}


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsAfsProviderMetadata();
}
#endif
