/***************************************************************************
  qgsogrproviderconnection.cpp

 ---------------------
 begin                : 6.8.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsogrproviderconnection.h"
#include "qgsogrdbconnection.h"
#include "qgssettings.h"
#include "qgsogrprovider.h"
#include "qgsmessagelog.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsfeedback.h"
#include "qgsogrutils.h"
#include "qgsfielddomain.h"

#include <QTextCodec>
#include <QRegularExpression>

#include <chrono>

///@cond PRIVATE

QgsOgrProviderConnection::QgsOgrProviderConnection( const QString &name )
  : QgsAbstractDatabaseProviderConnection( name )
{
  mProviderKey = QStringLiteral( "ogr" );
}

QgsOgrProviderConnection::QgsOgrProviderConnection( const QString &uri, const QVariantMap &configuration )
  : QgsAbstractDatabaseProviderConnection( uri, configuration )
{
  mProviderKey = QStringLiteral( "ogr" );

  // Cleanup the URI in case it contains other information other than the file path
  const QVariantMap parts = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) )->decodeUri( uri );
  if ( !parts.value( QStringLiteral( "path" ) ).toString().isEmpty() && parts.value( QStringLiteral( "path" ) ).toString() != uri )
  {
    setUri( parts.value( QStringLiteral( "path" ) ).toString() );
  }
  setDefaultCapabilities();
}

void QgsOgrProviderConnection::store( const QString & ) const
{
}

void QgsOgrProviderConnection::remove( const QString & ) const
{
}

QString QgsOgrProviderConnection::tableUri( const QString &, const QString &name ) const
{
  QVariantMap parts = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) )->decodeUri( uri() );
  parts.insert( QStringLiteral( "layerName" ), name );
  return QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) )->encodeUri( parts );
}

void QgsOgrProviderConnection::createVectorTable( const QString &schema,
    const QString &name,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    const QMap<QString, QVariant> *options ) const
{
  checkCapability( Capability::CreateVectorTable );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by OGR, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }
  QMap<QString, QVariant> opts { *options };
  opts[ QStringLiteral( "layerName" ) ] = QVariant( name );
  opts[ QStringLiteral( "update" ) ] = true;
  QMap<int, int> map;
  QString errCause;
  Qgis::VectorExportResult errCode = QgsOgrProvider::createEmptyLayer(
                                       uri(),
                                       fields,
                                       wkbType,
                                       srs,
                                       overwrite,
                                       &map,
                                       &errCause,
                                       &opts
                                     );
  if ( errCode != Qgis::VectorExportResult::Success )
  {
    throw QgsProviderConnectionException( QObject::tr( "An error occurred while creating the vector layer: %1" ).arg( errCause ) );
  }
}

void QgsOgrProviderConnection::dropVectorTable( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::DropVectorTable );
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by OGR, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }
  QString errCause;
  const QString layerUri = tableUri( schema, name );
  if ( ! QgsOgrProviderUtils::deleteLayer( layerUri, errCause ) )
  {
    throw QgsProviderConnectionException( QObject::tr( "Error deleting vector/aspatial table %1: %2" ).arg( name, errCause ) );
  }
}

void QgsOgrProviderConnection::setDefaultCapabilities()
{
  GDALDriverH hDriver = GDALIdentifyDriverEx( uri().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr );
  if ( !hDriver )
    return;

  mCapabilities =
  {
    Capability::DeleteField, // No generic way in GDAL to test this per driver/dataset yet
    Capability::AddField, // No generic way in GDAL to test this per driver/dataset yet
    Capability::CreateVectorTable, // No generic way in GDAL to test this per driver yet, only by opening the dataset in advance in update mode
    Capability::DropVectorTable, // No generic way in GDAL to test this per driver yet, only by opening the dataset in advance in update mode
  };

  mGeometryColumnCapabilities =
  {
    GeometryColumnCapability::Z, // No generic way in GDAL to test these per driver/dataset yet
    GeometryColumnCapability::SinglePart
  };

  char **driverMetadata = GDALGetMetadata( hDriver, nullptr );

  if ( !CSLFetchBoolean( driverMetadata, GDAL_DCAP_NONSPATIAL, false ) && CSLFetchBoolean( driverMetadata, GDAL_DCAP_VECTOR, false ) )
    mCapabilities |= Capability::Spatial;

  gdal::ogr_datasource_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
  if ( hDS )
  {
    if ( OGR_DS_TestCapability( hDS.get(), ODsCCurveGeometries ) )
      mGeometryColumnCapabilities |= GeometryColumnCapability::Curves;

    if ( OGR_DS_TestCapability( hDS.get(), ODsCMeasuredGeometries ) )
      mGeometryColumnCapabilities |= GeometryColumnCapability::M;
  }

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,5,0)
  if ( CSLFetchBoolean( driverMetadata, GDAL_DCAP_FIELD_DOMAINS, false ) )
  {
    mCapabilities |= Capability::RetrieveFieldDomain;
    mCapabilities |= Capability::ListFieldDomains;
    mCapabilities |= Capability::SetFieldDomain;
    mCapabilities |= Capability::AddFieldDomain;
  }
#endif
}

QList<QgsVectorDataProvider::NativeType> QgsOgrProviderConnection::nativeTypes() const
{
  QgsVectorLayer::LayerOptions options { false, true };
  options.skipCrsValidation = true;
  const QgsVectorLayer vl { uri(), QStringLiteral( "temp_layer" ), QStringLiteral( "ogr" ), options };
  if ( ! vl.isValid() || ! vl.dataProvider() )
  {
    const QString errorCause = vl.dataProvider() && vl.dataProvider()->hasErrors() ?
                               vl.dataProvider()->errors().join( '\n' ) :
                               QObject::tr( "unknown error" );
    throw QgsProviderConnectionException( QObject::tr( "Error retrieving native types for %1: %2" ).arg( uri(), errorCause ) );
  }
  return vl.dataProvider()->nativeTypes();
}

QStringList QgsOgrProviderConnection::fieldDomainNames() const
{
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,5,0)
  gdal::ogr_datasource_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
  if ( hDS )
  {
    QStringList names;
    if ( char **domainNames = GDALDatasetGetFieldDomainNames( hDS.get(), nullptr ) )
    {
      names = QgsOgrUtils::cStringListToQStringList( domainNames );
      CSLDestroy( domainNames );
    }
    return names;
  }
  else
  {
    throw QgsProviderConnectionException( QObject::tr( "There was an error opening the dataset %1!" ).arg( uri() ) );
  }
#else
  throw QgsProviderConnectionException( QObject::tr( "Listing field domains for datasets requires GDAL 3.5 or later" ) );
#endif
}

QgsFieldDomain *QgsOgrProviderConnection::fieldDomain( const QString &name ) const
{
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,3,0)
  gdal::ogr_datasource_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr ) );
  if ( hDS )
  {
    if ( OGRFieldDomainH domain = GDALDatasetGetFieldDomain( hDS.get(), name.toUtf8().constData() ) )
    {
      std::unique_ptr< QgsFieldDomain > res = QgsOgrUtils::convertFieldDomain( domain );
      if ( res )
      {
        return res.release();
      }
    }
    throw QgsProviderConnectionException( QObject::tr( "Could not retrieve field domain %1!" ).arg( name ) );
  }
  else
  {
    throw QgsProviderConnectionException( QObject::tr( "There was an error opening the dataset %1!" ).arg( uri() ) );
  }
#else
  ( void )name;
  throw QgsProviderConnectionException( QObject::tr( "Retrieving field domains for datasets requires GDAL 3.3 or later" ) );
#endif
}

void QgsOgrProviderConnection::setFieldDomainName( const QString &fieldName, const QString &schema, const QString &tableName, const QString &domainName ) const
{
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,3,0)
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by OGR, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }

  QString errCause;
  QgsOgrLayerUniquePtr layer = QgsOgrProviderUtils::getLayer( uri(),
                               true,
                               QStringList(),
                               tableName, errCause, true );
  if ( !layer )
  {
    throw QgsProviderConnectionException( QObject::tr( "There was an error opening the dataset: %1" ).arg( errCause ) );
  }

  //type does not matter, it will not be used
  gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( fieldName.toUtf8().constData(), OFTReal ) );
  OGR_Fld_SetDomainName( fld.get(), domainName.toUtf8().constData() );

  const int fieldIndex = layer->GetLayerDefn().GetFieldIndex( fieldName.toUtf8().constData() );
  if ( fieldIndex < 0 )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not set field domain for %1 - field does not exist" ).arg( fieldName ) );
  }
  if ( layer->AlterFieldDefn( fieldIndex, fld.get(), ALTER_DOMAIN_FLAG ) != OGRERR_NONE )
  {
    throw QgsProviderConnectionException( QObject::tr( "Could not set field domain: %1" ).arg( CPLGetLastErrorMsg() ) );
  }
#else
  ( void )fieldName;
  ( void )schema;
  ( void )tableName;
  ( void )domainName;
  throw QgsProviderConnectionException( QObject::tr( "Setting field domains for datasets requires GDAL 3.3 or later" ) );
#endif
}

void QgsOgrProviderConnection::addFieldDomain( const QgsFieldDomain &domain, const QString &schema ) const
{
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,3,0)
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by OGR, ignoring" ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }

  gdal::ogr_datasource_unique_ptr hDS( GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr ) );
  if ( hDS )
  {
    if ( OGRFieldDomainH ogrDomain = QgsOgrUtils::convertFieldDomain( &domain ) )
    {
      char *failureReason = nullptr;
      if ( !GDALDatasetAddFieldDomain( hDS.get(), ogrDomain, &failureReason ) )
      {
        OGR_FldDomain_Destroy( ogrDomain );
        QString error( failureReason );
        CPLFree( failureReason );
        throw QgsProviderConnectionException( QObject::tr( "Could not create field domain: %1" ).arg( error ) );
      }
      CPLFree( failureReason );
      OGR_FldDomain_Destroy( ogrDomain );
    }
    else
    {
      throw QgsProviderConnectionException( QObject::tr( "Could not create field domain" ) );
    }
  }
  else
  {
    throw QgsProviderConnectionException( QObject::tr( "There was an error opening the dataset %1!" ).arg( uri() ) );
  }
#else
  ( void )domain;
  ( void )schema;
  throw QgsProviderConnectionException( QObject::tr( "Creating field domains for datasets requires GDAL 3.3 or later" ) );
#endif
}

///@endcond
