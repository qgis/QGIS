/***************************************************************************
                         qgsalgorithmexporttospatialiteregistered.cpp
                         ---------------------
    begin                : April 2025
    copyright            : (C) 2025 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmexporttospatialiteregistered.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsvectorlayerexporter.h"

///@cond PRIVATE

QString QgsExportToRegisteredSpatialiteAlgorithm::name() const
{
  return QStringLiteral( "importintospatialiteregistered" );
}

QString QgsExportToRegisteredSpatialiteAlgorithm::displayName() const
{
  return QObject::tr( "Export to SpatiaLite (registered)" );
}

QStringList QgsExportToRegisteredSpatialiteAlgorithm::tags() const
{
  return QObject::tr( "export,import,spatialite,table,layer,into,copy" ).split( ',' );
}

QString QgsExportToRegisteredSpatialiteAlgorithm::group() const
{
  return QObject::tr( "Database" );
}

QString QgsExportToRegisteredSpatialiteAlgorithm::groupId() const
{
  return QStringLiteral( "database" );
}

QString QgsExportToRegisteredSpatialiteAlgorithm::shortHelpString() const
{
  return QObject::tr( "Exports a vector layer to a SpatiaLite database, creating "
                      "a new table.\n\n"
                      "Prior to this a connection between QGIS and the SpatiaLite "
                      "database has to be created (for example through the QGIS "
                      "Browser panel)." );
}

QString QgsExportToRegisteredSpatialiteAlgorithm::shortDescription() const
{
  return QObject::tr( "Exports a vector layer to a registered SpatiaLite database, creating a new table." );
}

Qgis::ProcessingAlgorithmFlags QgsExportToRegisteredSpatialiteAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading;
}

QgsExportToRegisteredSpatialiteAlgorithm *QgsExportToRegisteredSpatialiteAlgorithm::createInstance() const
{
  return new QgsExportToRegisteredSpatialiteAlgorithm();
}

void QgsExportToRegisteredSpatialiteAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Layer to export" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterProviderConnection( QStringLiteral( "DATABASE" ), QObject::tr( "Database (connection name)" ), QStringLiteral( "spatialite" ) ) );
  addParameter( new QgsProcessingParameterDatabaseTable( QStringLiteral( "TABLENAME" ), QObject::tr( "Table to export to (leave blank to use layer name)" ), QStringLiteral( "DATABASE" ), QString(), QVariant(), true, true ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "PRIMARY_KEY" ), QObject::tr( "Primary key field" ), QVariant(), QStringLiteral( "INPUT" ), Qgis::ProcessingFieldParameterDataType::Any, false, true ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "GEOMETRY_COLUMN" ), QObject::tr( "Geometry column" ), QStringLiteral( "geom" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "ENCODING" ), QObject::tr( "Encoding" ), QStringLiteral( "UTF-8" ), false, true ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "OVERWRITE" ), QObject::tr( "Overwrite" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "CREATEINDEX" ), QObject::tr( "Create spatial index" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "LOWERCASE_NAMES" ), QObject::tr( "Convert field names to lowercase" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "DROP_STRING_LENGTH" ), QObject::tr( "Drop length constraints on character fields" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "FORCE_SINGLEPART" ), QObject::tr( "Create single-part geometries instead of multipart" ), false ) );
}

QVariantMap QgsExportToRegisteredSpatialiteAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString connectionName = parameterAsConnectionName( parameters, QStringLiteral( "DATABASE" ), context );

  std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn;
  try
  {
    QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "spatialite" ) );
    conn.reset( static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionName ) ) );
  }
  catch ( QgsProviderConnectionException & )
  {
    QgsProcessingException( QObject::tr( "Could not retrieve connection details for %1" ).arg( connectionName ) );
  }

  const QString primaryKeyField = parameterAsString( parameters, QStringLiteral( "PRIMARY_KEY" ), context );
  const QString encoding = parameterAsString( parameters, QStringLiteral( "ENCODING" ), context );
  const bool overwrite = parameterAsBoolean( parameters, QStringLiteral( "OVERWRITE" ), context );

  QString tableName = parameterAsDatabaseTableName( parameters, QStringLiteral( "TABLENAME" ), context ).trimmed();
  if ( tableName.isEmpty() )
  {
    tableName = source->sourceName();
    tableName = tableName.replace( '.', '_' );
  }
  tableName = tableName.replace( ' ', QString() ).right( 63 );

  QString geometryColumn = parameterAsString( parameters, QStringLiteral( "GEOMETRY_COLUMN" ), context );
  if ( geometryColumn.isEmpty() )
  {
    geometryColumn = QStringLiteral( "geom" );
  }
  if ( source->wkbType() == Qgis::WkbType::NoGeometry )
  {
    geometryColumn.clear();
  }

  const bool createSpatialIndex = parameterAsBoolean( parameters, QStringLiteral( "CREATEINDEX" ), context );

  QMap<QString, QVariant> options;
  if ( overwrite )
  {
    options[QStringLiteral( "overwrite" )] = true;
  }
  if ( parameterAsBoolean( parameters, QStringLiteral( "LOWERCASE_NAMES" ), context ) )
  {
    options[QStringLiteral( "lowercaseFieldNames" )] = true;
    geometryColumn = geometryColumn.toLower();
  }
  if ( parameterAsBoolean( parameters, QStringLiteral( "DROP_STRING_LENGTH" ), context ) )
  {
    options[QStringLiteral( "dropStringConstraints" )] = true;
  }
  if ( parameterAsBoolean( parameters, QStringLiteral( "FORCE_SINGLEPART" ), context ) )
  {
    options[QStringLiteral( "forceSinglePartGeometryType" )] = true;
  }
  if ( !encoding.isEmpty() )
  {
    options[QStringLiteral( "fileEncoding" )] = encoding;
  }

  QgsDataSourceUri uri = QgsDataSourceUri( conn->uri() );
  uri.setTable( tableName );
  uri.setKeyColumn( primaryKeyField );
  uri.setGeometryColumn( geometryColumn );

  auto exporter = std::make_unique<QgsVectorLayerExporter>( uri.uri(), QStringLiteral( "spatialite" ), source->fields(), source->wkbType(), source->sourceCrs(), overwrite, options );

  if ( exporter->errorCode() != Qgis::VectorExportResult::Success )
    throw QgsProcessingException( QObject::tr( "Error exporting to SpatiaLite\n%1" ).arg( exporter->errorMessage() ) );

  QgsFeatureIterator featureIterator = source->getFeatures();

  const double step = source->featureCount() > 0 ? 100.0 / source->featureCount() : 0.0;

  long long i = 0;
  QgsFeature f;
  while ( featureIterator.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( !exporter->addFeature( f, QgsFeatureSink::FastInsert ) )
    {
      feedback->reportError( exporter->errorMessage() );
    }

    feedback->setProgress( i * step );
    i++;
  }
  exporter->flushBuffer();

  if ( exporter->errorCode() != Qgis::VectorExportResult::Success )
    throw QgsProcessingException( QObject::tr( "Error exporting to SpatiaLite\n%1" ).arg( exporter->errorMessage() ) );

  exporter.reset();

  if ( !geometryColumn.isEmpty() && createSpatialIndex )
  {
    try
    {
      QgsAbstractDatabaseProviderConnection::SpatialIndexOptions opt = QgsAbstractDatabaseProviderConnection::SpatialIndexOptions();
      opt.geometryColumnName = geometryColumn;
      conn->createSpatialIndex( "", tableName, opt );
    }
    catch ( QgsProviderConnectionException &e )
    {
      throw QgsProcessingException( QObject::tr( "Error creating spatial index:\n%1" ).arg( e.what() ) );
    }
  }

  try
  {
    conn->vacuum( "", tableName );
  }
  catch ( QgsProviderConnectionException &e )
  {
    feedback->reportError( QObject::tr( "Error vacuuming table:\n%1" ).arg( e.what() ) );
  }

  QVariantMap outputs;
  return outputs;
}

///@endcond
