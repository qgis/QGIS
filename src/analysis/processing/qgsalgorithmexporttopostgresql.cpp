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

#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsvectorfilewriter.h"

///@cond PRIVATE

QString QgsExportToPostgresqlAlgorithm::name() const
{
  return u"importintopostgis"_s;
}

QString QgsExportToPostgresqlAlgorithm::displayName() const
{
  return u"Export to PostgreSQL"_s;
}

QStringList QgsExportToPostgresqlAlgorithm::tags() const
{
  return QObject::tr( "export,import,postgis,table,layer,into,copy" ).split( ',' );
}

QString QgsExportToPostgresqlAlgorithm::group() const
{
  return u"Database"_s;
}

QString QgsExportToPostgresqlAlgorithm::groupId() const
{
  return u"database"_s;
}

void QgsExportToPostgresqlAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Layer to export" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterProviderConnection( u"DATABASE"_s, QObject::tr( "Database (connection name)" ), u"postgres"_s ) );
  addParameter( new QgsProcessingParameterDatabaseSchema( u"SCHEMA"_s, QObject::tr( "Schema (schema name)" ), u"DATABASE"_s, u"public"_s, true ) );
  addParameter( new QgsProcessingParameterDatabaseTable( u"TABLENAME"_s, QObject::tr( "Table to export to (leave blank to use layer name)" ), u"DATABASE"_s, u"SCHEMA"_s, QVariant(), true, true ) );
  addParameter( new QgsProcessingParameterField( u"PRIMARY_KEY"_s, QObject::tr( "Primary key field" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any, false, true ) );
  addParameter( new QgsProcessingParameterString( u"GEOMETRY_COLUMN"_s, QObject::tr( "Geometry column" ), u"geom"_s ) );
  addParameter( new QgsProcessingParameterString( u"ENCODING"_s, QObject::tr( "Encoding" ), u"UTF-8"_s, false, true ) );
  addParameter( new QgsProcessingParameterBoolean( u"OVERWRITE"_s, QObject::tr( "Overwrite" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( u"CREATEINDEX"_s, QObject::tr( "Create spatial index" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( u"LOWERCASE_NAMES"_s, QObject::tr( "Convert field names to lowercase" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( u"DROP_STRING_LENGTH"_s, QObject::tr( "Drop length constraints on character fields" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( u"FORCE_SINGLEPART"_s, QObject::tr( "Create single-part geometries instead of multipart" ), false ) );
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
  return QObject::tr( "Exports a vector layer to a PostgreSQL database." );
}

QgsExportToPostgresqlAlgorithm *QgsExportToPostgresqlAlgorithm::createInstance() const
{
  return new QgsExportToPostgresqlAlgorithm();
}

bool QgsExportToPostgresqlAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !mSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const QString connectionName = parameterAsConnectionName( parameters, u"DATABASE"_s, context );

  try
  {
    QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( u"postgres"_s );
    mConn.reset( static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionName ) ) );
  }
  catch ( QgsProviderConnectionException & )
  {
    QgsProcessingException( QObject::tr( "Could not retrieve connection details for %1" ).arg( connectionName ) );
  }

  mSchema = parameterAsSchema( parameters, u"SCHEMA"_s, context );
  mPrimaryKeyField = parameterAsString( parameters, u"PRIMARY_KEY"_s, context );
  mEncoding = parameterAsString( parameters, u"ENCODING"_s, context );
  mOverwrite = parameterAsBoolean( parameters, u"OVERWRITE"_s, context );

  mTable = parameterAsDatabaseTableName( parameters, u"TABLENAME"_s, context ).trimmed();
  if ( mTable.isEmpty() )
  {
    mTable = mSource->sourceName();
    mTable = mTable.replace( '.', '_' );
  }
  mTable = mTable.replace( ' ', QString() ).right( 63 );

  mGeomColumn = parameterAsString( parameters, u"GEOMETRY_COLUMN"_s, context );
  if ( mGeomColumn.isEmpty() )
    mGeomColumn = u"geom"_s;
  if ( mSource->wkbType() == Qgis::WkbType::NoGeometry )
    mGeomColumn.clear();

  mCreateIndex = parameterAsBoolean( parameters, u"CREATEINDEX"_s, context );

  if ( mOverwrite )
    mOptions[u"overwrite"_s] = true;
  if ( parameterAsBoolean( parameters, u"LOWERCASE_NAMES"_s, context ) )
  {
    mOptions[u"lowercaseFieldNames"_s] = true;
    mGeomColumn = mGeomColumn.toLower();
  }
  if ( parameterAsBoolean( parameters, u"DROP_STRING_LENGTH"_s, context ) )
    mOptions[u"dropStringConstraints"_s] = true;
  if ( parameterAsBoolean( parameters, u"FORCE_SINGLEPART"_s, context ) )
    mOptions[u"forceSinglePartGeometryType"_s] = true;
  if ( !mEncoding.isEmpty() )
    mOptions[u"fileEncoding"_s] = mEncoding;

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

  auto exporter = std::make_unique<QgsVectorLayerExporter>( uri.uri(), mProviderName, mSource->fields(), mSource->wkbType(), mSource->sourceCrs(), mOverwrite, mOptions );

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
