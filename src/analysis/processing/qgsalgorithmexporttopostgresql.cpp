/***************************************************************************
                         qgsalgorithmexporttopostgresql.cpp
                         ---------------------
    begin                : November 2021
    copyright            : (C) 2021 by Clemens Raffler
    email                : clemens dot raffler at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmexporttopostgresql.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsvectorfilewriter.h"

///@cond PRIVATE

QString QgsExportToPostgresqlAlgorithm::name() const
{
  return QStringLiteral( "importintopostgis" );
}

QString QgsExportToPostgresqlAlgorithm::displayName() const
{
  return QStringLiteral( "Export to PostgreSQL" );
}

QStringList QgsExportToPostgresqlAlgorithm::tags() const
{
  return QObject::tr( "export,import,postgis,table,layer,into,copy" ).split( ',' );
}

QString QgsExportToPostgresqlAlgorithm::group() const
{
  return QStringLiteral( "Database" );
}

QString QgsExportToPostgresqlAlgorithm::groupId() const
{
  return QStringLiteral( "database" );
}

void QgsExportToPostgresqlAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Layer to export" ), QList<int>() << QgsProcessing::TypeVector ) );
  addParameter( new QgsProcessingParameterProviderConnection( QStringLiteral( "DATABASE" ), QObject::tr( "Database (connection name)" ), QStringLiteral( "postgres" ) ) );
  addParameter( new QgsProcessingParameterDatabaseSchema( QStringLiteral( "SCHEMA" ), QObject::tr( "Schema (schema name)" ), QStringLiteral( "DATABASE" ), QStringLiteral( "public" ), true ) );
  addParameter( new QgsProcessingParameterDatabaseTable( QStringLiteral( "TABLENAME" ), QObject::tr( "Table to export to (leave blank to use layer name)" ), QStringLiteral( "DATABASE" ), QStringLiteral( "SCHEMA" ), QVariant(), true, true ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "PRIMARY_KEY" ), QObject::tr( "Primary key field" ), QVariant(), QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, false, true ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "GEOMETRY_COLUMN" ), QObject::tr( "Geometry column" ), QStringLiteral( "geom" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "ENCODING" ), QObject::tr( "Encoding" ), QStringLiteral( "UTF-8" ), false, true ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "OVERWRITE" ), QObject::tr( "Overwrite" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "CREATEINDEX" ), QObject::tr( "Create spatial index" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "LOWERCASE_NAMES" ), QObject::tr( "Convert field names to lowercase" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "DROP_STRING_LENGTH" ), QObject::tr( "Drop length constraints on character fields" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "FORCE_SINGLEPART" ), QObject::tr( "Create single-part geometries instead of multipart" ), false ) );
}

QString QgsExportToPostgresqlAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm exports a vector layer to a PostgreSQL "
                      "database, creating a new table.\n\n"
                      "Prior to this a connection between QGIS and the PostgreSQL "
                      "database has to be created (for example through the QGIS Browser panel)."
                    );
}

QString QgsExportToPostgresqlAlgorithm::shortDescription() const
{
  return QObject::tr( "Exports a vector layer to a PostgreSQL database" );
}

QgsExportToPostgresqlAlgorithm *QgsExportToPostgresqlAlgorithm::createInstance() const
{
  return new QgsExportToPostgresqlAlgorithm();
}

bool QgsExportToPostgresqlAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !mSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString connectionName = parameterAsConnectionName( parameters, QStringLiteral( "DATABASE" ), context );

  try
  {
    QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "postgres" ) );
    mConn.reset( static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionName ) ) );
  }
  catch ( QgsProviderConnectionException & )
  {
    QgsProcessingException( QObject::tr( "Could not retrieve connection details for %1" ).arg( connectionName ) );
  }

  mSchema = parameterAsSchema( parameters, QStringLiteral( "SCHEMA" ), context );
  mPrimaryKeyField = parameterAsString( parameters, QStringLiteral( "PRIMARY_KEY" ), context );
  mEncoding = parameterAsString( parameters, QStringLiteral( "ENCODING" ), context );
  mOverwrite = parameterAsBoolean( parameters, QStringLiteral( "OVERWRITE" ), context );

  mTable = parameterAsDatabaseTableName( parameters, QStringLiteral( "TABLENAME" ), context ).trimmed();
  if ( mTable.isEmpty() )
  {
    mTable = mSource->sourceName();
    mTable = mTable.replace( '.', '_' );
  }
  mTable = mTable.replace( ' ', QString() ).right( 63 );

  mGeomColumn = parameterAsString( parameters, QStringLiteral( "GEOMETRY_COLUMN" ), context );
  if ( mGeomColumn.isEmpty() )
    mGeomColumn = QStringLiteral( "geom" );
  if ( mSource->wkbType() == QgsWkbTypes::NoGeometry )
    mGeomColumn.clear();

  mCreateIndex = parameterAsBoolean( parameters, QStringLiteral( "CREATEINDEX" ), context );

  if ( mOverwrite )
    mOptions[QStringLiteral( "overwrite" )] = true;
  if ( parameterAsBoolean( parameters, QStringLiteral( "LOWERCASE_NAMES" ), context ) )
  {
    mOptions[QStringLiteral( "lowercaseFieldNames" )] = true;
    mGeomColumn = mGeomColumn.toLower();
  }
  if ( parameterAsBoolean( parameters, QStringLiteral( "DROP_STRING_LENGTH" ), context ) )
    mOptions[QStringLiteral( "dropStringConstraints" )] = true;
  if ( parameterAsBoolean( parameters, QStringLiteral( "FORCE_SINGLEPART" ), context ) )
    mOptions[QStringLiteral( "forceSinglePartGeometryType" )] = true;
  if ( !mEncoding.isEmpty() )
    mOptions[QStringLiteral( "fileEncoding" )] = mEncoding;

  return true;
}

QVariantMap QgsExportToPostgresqlAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( parameters );
  Q_UNUSED( context );

  QgsDataSourceUri uri = QgsDataSourceUri( mConn->uri() );
  uri.setSchema( mSchema );
  uri.setTable( mTable );
  uri.setKeyColumn( mPrimaryKeyField );
  uri.setGeometryColumn( mGeomColumn );

  std::unique_ptr< QgsVectorLayerExporter > exporter = std::make_unique< QgsVectorLayerExporter >( uri.uri(), mProviderName, mSource->fields(), mSource->wkbType(), mSource->sourceCrs(), mOverwrite, mOptions );

  if ( exporter->errorCode() != Qgis::VectorExportResult::Success )
    throw QgsProcessingException( QObject::tr( "Error exporting to PostGIS\n%1" ).arg( exporter->errorMessage() ) );

  QgsFeatureIterator featureIterator = mSource->getFeatures();

  const double progressStep = ( mSource->featureCount() ) ? 100.0 / mSource->featureCount() : 0.0;

  qgssize i = 0;
  QgsFeature f;
  while ( featureIterator.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      break;

    if ( !exporter->addFeature( f, QgsFeatureSink::FastInsert ) )
      feedback->reportError( exporter->errorMessage() );

    feedback->setProgress( i * progressStep );
    i++;
  }
  exporter->flushBuffer();

  if ( exporter->errorCode() != Qgis::VectorExportResult::Success )
    throw QgsProcessingException( QObject::tr( "Error exporting to PostGIS\n%1" ).arg( exporter->errorMessage() ) );

  exporter.reset();

  if ( !mGeomColumn.isEmpty() && mCreateIndex )
  {
    try
    {
      QgsAbstractDatabaseProviderConnection::SpatialIndexOptions opt = QgsAbstractDatabaseProviderConnection::SpatialIndexOptions();
      opt.geometryColumnName = mGeomColumn;
      mConn->createSpatialIndex( mSchema, mTable, opt );
    }
    catch ( QgsProviderConnectionException &ex )
    {
      throw QgsProcessingException( QObject::tr( "Error creating spatial index:\n%1" ).arg( ex.what() ) );
    }
  }

  try
  {
    mConn->vacuum( mSchema, mTable );
  }
  catch ( QgsProviderConnectionException &ex )
  {
    feedback->reportError( QObject::tr( "Error vacuuming table:\n{0}" ).arg( ex.what() ) );
  }

  QVariantMap outputs;
  return outputs;
}
///@endcond
