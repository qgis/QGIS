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
#include "qgsafsdataitems.h"
#include "qgslogger.h"
#include "qgsdataitemprovider.h"
#include "qgsapplication.h"

const QString QgsAfsProvider::AFS_PROVIDER_KEY = QStringLiteral( "arcgisfeatureserver" );
const QString QgsAfsProvider::AFS_PROVIDER_DESCRIPTION = QStringLiteral( "ArcGIS Feature Server data provider" );


QgsAfsProvider::QgsAfsProvider( const QString &uri, const ProviderOptions &options )
  : QgsVectorDataProvider( uri, options )
{
  mSharedData.reset( new QgsAfsSharedData() );
  mSharedData->mGeometryType = QgsWkbTypes::Unknown;
  mSharedData->mDataSource = QgsDataSourceUri( uri );

  const QString authcfg = mSharedData->mDataSource.authConfigId();

  // Set CRS
  mSharedData->mSourceCRS.createFromString( mSharedData->mDataSource.param( QStringLiteral( "crs" ) ) );

  // Get layer info
  QString errorTitle, errorMessage;

  const QString referer = mSharedData->mDataSource.param( QStringLiteral( "referer" ) );
  if ( !referer.isEmpty() )
    mRequestHeaders[ QStringLiteral( "Referer" )] = referer;

  const QVariantMap layerData = QgsArcGisRestUtils::getLayerInfo( mSharedData->mDataSource.param( QStringLiteral( "url" ) ),
                                authcfg, errorTitle, errorMessage, mRequestHeaders );
  if ( layerData.isEmpty() )
  {
    pushError( errorTitle + ": " + errorMessage );
    appendError( QgsErrorMessage( tr( "getLayerInfo failed" ), QStringLiteral( "AFSProvider" ) ) );
    return;
  }
  mLayerName = layerData[QStringLiteral( "name" )].toString();
  mLayerDescription = layerData[QStringLiteral( "description" )].toString();

  // Set extent
  QStringList coords = mSharedData->mDataSource.param( QStringLiteral( "bbox" ) ).split( ',' );
  bool limitBbox = false;
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
    {
      // user has set a bounding box limit on the layer - so we only EVER fetch features from this extent
      limitBbox = true;
    }
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
  QgsCoordinateReferenceSystem extentCrs = QgsArcGisRestUtils::parseSpatialReference( layerExtentMap[QStringLiteral( "spatialReference" )].toMap() );
  if ( mSharedData->mExtent.isEmpty() && !extentCrs.isValid() )
  {
    appendError( QgsErrorMessage( tr( "Could not parse spatial reference" ), QStringLiteral( "AFSProvider" ) ) );
    return;
  }

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
    const QString fieldTypeString = fieldDataMap[QStringLiteral( "type" )].toString();
    QVariant::Type type = QgsArcGisRestUtils::mapEsriFieldType( fieldTypeString );
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

    if ( fieldDataMap.contains( QStringLiteral( "domain" ) ) && fieldDataMap.value( QStringLiteral( "domain" ) ).toMap().value( QStringLiteral( "type" ) ).toString() == QStringLiteral( "codedValue" ) )
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

    mSharedData->mFields.append( field );
  }
  if ( objectIdFieldName.isEmpty() )
    objectIdFieldName = QStringLiteral( "objectid" );

  // Determine geometry type
  bool hasM = layerData[QStringLiteral( "hasM" )].toBool();
  bool hasZ = layerData[QStringLiteral( "hasZ" )].toBool();
  mSharedData->mGeometryType = QgsArcGisRestUtils::mapEsriGeometryType( layerData[QStringLiteral( "geometryType" )].toString() );
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

  // Read OBJECTIDs of all features: these may not be a continuous sequence,
  // and we need to store these to iterate through the features. This query
  // also returns the name of the ObjectID field.
  QVariantMap objectIdData = QgsArcGisRestUtils::getObjectIds( mSharedData->mDataSource.param( QStringLiteral( "url" ) ), authcfg,
                             errorTitle,  errorMessage, mRequestHeaders, limitBbox ? mSharedData->mExtent : QgsRectangle() );
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
  mSharedData->mObjectIdFieldName = objectIdData[QStringLiteral( "objectIdFieldName" )].toString();
  for ( int idx = 0, nIdx = mSharedData->mFields.count(); idx < nIdx; ++idx )
  {
    if ( mSharedData->mFields.at( idx ).name() == mSharedData->mObjectIdFieldName )
    {
      mObjectIdFieldIdx = idx;

      // primary key is not null, unique
      QgsFieldConstraints constraints = mSharedData->mFields.at( idx ).constraints();
      constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
      constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
      mSharedData->mFields[ idx ].setConstraints( constraints );

      break;
    }
  }
  const QVariantList objectIds = objectIdData.value( QStringLiteral( "objectIds" ) ).toList();
  for ( const QVariant &objectId : objectIds )
  {
    mSharedData->mObjectIds.append( objectId.toInt() );
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
  QString copyright = layerData[QStringLiteral( "copyrightText" )].toString();
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

long QgsAfsProvider::featureCount() const
{
  return mSharedData->mObjectIds.size();
}

QgsFields QgsAfsProvider::fields() const
{
  return mSharedData->mFields;
}

QgsLayerMetadata QgsAfsProvider::layerMetadata() const
{
  return mLayerMetadata;
}

QgsVectorDataProvider::Capabilities QgsAfsProvider::capabilities() const
{
  QgsVectorDataProvider::Capabilities c = QgsVectorDataProvider::SelectAtId | QgsVectorDataProvider::ReadLayerMetadata;
  if ( !mRendererDataMap.empty() )
  {
    c = c | QgsVectorDataProvider::CreateRenderer;
  }
  if ( !mLabelingDataList.empty() )
  {
    c = c | QgsVectorDataProvider::CreateLabeling;
  }
  return c;
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

QString QgsAfsProvider::description() const
{
  return AFS_PROVIDER_DESCRIPTION;
}

QString QgsAfsProvider::dataComment() const
{
  return mLayerDescription;
}

void QgsAfsProvider::reloadData()
{
  mSharedData->clearCache();
}

QgsFeatureRenderer *QgsAfsProvider::createRenderer( const QVariantMap & ) const
{
  return QgsArcGisRestUtils::parseEsriRenderer( mRendererDataMap );
}

QgsAbstractVectorLayerLabeling *QgsAfsProvider::createLabeling( const QVariantMap & ) const
{
  return QgsArcGisRestUtils::parseEsriLabeling( mLabelingDataList );
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

QList<QgsDataItemProvider *> QgsAfsProviderMetadata::dataItemProviders() const
{
  QList<QgsDataItemProvider *> providers;

  providers
      << new QgsAfsDataItemProvider;

  return providers;
}

QVariantMap QgsAfsProviderMetadata::decodeUri( const QString &uri )
{
  QgsDataSourceUri dsUri = QgsDataSourceUri( uri );

  QVariantMap components;
  components.insert( QStringLiteral( "url" ), dsUri.param( QStringLiteral( "url" ) ) );
  return components;
}

QgsAfsProvider *QgsAfsProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options )
{
  return new QgsAfsProvider( uri, options );
}


QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsAfsProviderMetadata();
}
