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
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsvectorlayerexporter.h"

///@cond PRIVATE

QString QgsExportToRegisteredSpatialiteAlgorithm::name() const
{
  return u"importintospatialiteregistered"_s;
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
  return u"database"_s;
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
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Layer to export" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterProviderConnection( u"DATABASE"_s, QObject::tr( "Database (connection name)" ), u"spatialite"_s ) );
  addParameter( new QgsProcessingParameterDatabaseTable( u"TABLENAME"_s, QObject::tr( "Table to export to (leave blank to use layer name)" ), u"DATABASE"_s, QString(), QVariant(), true, true ) );
  addParameter( new QgsProcessingParameterField( u"PRIMARY_KEY"_s, QObject::tr( "Primary key field" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any, false, true ) );
  addParameter( new QgsProcessingParameterString( u"GEOMETRY_COLUMN"_s, QObject::tr( "Geometry column" ), u"geom"_s ) );
  addParameter( new QgsProcessingParameterString( u"ENCODING"_s, QObject::tr( "Encoding" ), u"UTF-8"_s, false, true ) );
  addParameter( new QgsProcessingParameterBoolean( u"OVERWRITE"_s, QObject::tr( "Overwrite" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( u"CREATEINDEX"_s, QObject::tr( "Create spatial index" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( u"LOWERCASE_NAMES"_s, QObject::tr( "Convert field names to lowercase" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( u"DROP_STRING_LENGTH"_s, QObject::tr( "Drop length constraints on character fields" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( u"FORCE_SINGLEPART"_s, QObject::tr( "Create single-part geometries instead of multipart" ), false ) );
}

QVariantMap QgsExportToRegisteredSpatialiteAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const QString connectionName = parameterAsConnectionName( parameters, u"DATABASE"_s, context );

  std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn;
  try
  {
    QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( u"spatialite"_s );
    conn.reset( static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionName ) ) );
  }
  catch ( QgsProviderConnectionException & )
  {
    QgsProcessingException( QObject::tr( "Could not retrieve connection details for %1" ).arg( connectionName ) );
  }

  const QString primaryKeyField = parameterAsString( parameters, u"PRIMARY_KEY"_s, context );
  const QString encoding = parameterAsString( parameters, u"ENCODING"_s, context );
  const bool overwrite = parameterAsBoolean( parameters, u"OVERWRITE"_s, context );

  QString tableName = parameterAsDatabaseTableName( parameters, u"TABLENAME"_s, context ).trimmed();
  if ( tableName.isEmpty() )
  {
    tableName = source->sourceName();
    tableName = tableName.replace( '.', '_' );
  }
  tableName = tableName.replace( ' ', QString() ).right( 63 );

  QString geometryColumn = parameterAsString( parameters, u"GEOMETRY_COLUMN"_s, context );
  if ( geometryColumn.isEmpty() )
  {
    geometryColumn = u"geom"_s;
  }
  if ( source->wkbType() == Qgis::WkbType::NoGeometry )
  {
    geometryColumn.clear();
  }

  const bool createSpatialIndex = parameterAsBoolean( parameters, u"CREATEINDEX"_s, context );

  QMap<QString, QVariant> options;
  if ( overwrite )
  {
    options[u"overwrite"_s] = true;
  }
  if ( parameterAsBoolean( parameters, u"LOWERCASE_NAMES"_s, context ) )
  {
    options[u"lowercaseFieldNames"_s] = true;
    geometryColumn = geometryColumn.toLower();
  }
  if ( parameterAsBoolean( parameters, u"DROP_STRING_LENGTH"_s, context ) )
  {
    options[u"dropStringConstraints"_s] = true;
  }
  if ( parameterAsBoolean( parameters, u"FORCE_SINGLEPART"_s, context ) )
  {
    options[u"forceSinglePartGeometryType"_s] = true;
  }
  if ( !encoding.isEmpty() )
  {
    options[u"fileEncoding"_s] = encoding;
  }

  QgsDataSourceUri uri = QgsDataSourceUri( conn->uri() );
  uri.setTable( tableName );
  uri.setKeyColumn( primaryKeyField );
  uri.setGeometryColumn( geometryColumn );

  auto exporter = std::make_unique<QgsVectorLayerExporter>( uri.uri(), u"spatialite"_s, source->fields(), source->wkbType(), source->sourceCrs(), overwrite, options );

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
