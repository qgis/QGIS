
/***************************************************************************
                         qgsalgorithmexporttospatialite.cpp
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

#include "qgsalgorithmexporttospatialite.h"

#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsvectorlayerexporter.h"

///@cond PRIVATE

QString QgsExportToSpatialiteAlgorithm::name() const
{
  return u"importintospatialite"_s;
}

QString QgsExportToSpatialiteAlgorithm::displayName() const
{
  return QObject::tr( "Export to SpatiaLite" );
}

QStringList QgsExportToSpatialiteAlgorithm::tags() const
{
  return QObject::tr( "export,import,spatialite,table,layer,into,copy" ).split( ',' );
}

QString QgsExportToSpatialiteAlgorithm::group() const
{
  return QObject::tr( "Database" );
}

QString QgsExportToSpatialiteAlgorithm::groupId() const
{
  return u"database"_s;
}

QString QgsExportToSpatialiteAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm exports a vector layer to a SpatiaLite database, creating a new table." );
}

QString QgsExportToSpatialiteAlgorithm::shortDescription() const
{
  return QObject::tr( "Exports a vector layer to a SpatiaLite database, creating a new table." );
}

QgsExportToSpatialiteAlgorithm *QgsExportToSpatialiteAlgorithm::createInstance() const
{
  return new QgsExportToSpatialiteAlgorithm();
}

void QgsExportToSpatialiteAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Layer to export" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterVectorLayer( u"DATABASE"_s, QObject::tr( "Database layer (or file)" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterString( u"TABLENAME"_s, QObject::tr( "Table to export to (leave blank to use layer name)" ), QVariant(), false, true ) );
  addParameter( new QgsProcessingParameterField( u"PRIMARY_KEY"_s, QObject::tr( "Primary key field" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any, false, true ) );
  addParameter( new QgsProcessingParameterString( u"GEOMETRY_COLUMN"_s, QObject::tr( "Geometry column" ), u"geom"_s ) );
  addParameter( new QgsProcessingParameterString( u"ENCODING"_s, QObject::tr( "Encoding" ), u"UTF-8"_s, false, true ) );
  addParameter( new QgsProcessingParameterBoolean( u"OVERWRITE"_s, QObject::tr( "Overwrite" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( u"CREATEINDEX"_s, QObject::tr( "Create spatial index" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( u"LOWERCASE_NAMES"_s, QObject::tr( "Convert field names to lowercase" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( u"DROP_STRING_LENGTH"_s, QObject::tr( "Drop length constraints on character fields" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( u"FORCE_SINGLEPART"_s, QObject::tr( "Create single-part geometries instead of multipart" ), false ) );
}

bool QgsExportToSpatialiteAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsVectorLayer *layer = parameterAsVectorLayer( parameters, u"DATABASE"_s, context );
  mProviderType = layer->providerType();
  mDatabaseUri = layer->dataProvider()->dataSourceUri();
  return true;
}

QVariantMap QgsExportToSpatialiteAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  QgsDataSourceUri uri( mDatabaseUri );
  if ( uri.database().isEmpty() )
  {
    QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( mProviderType );
    const QVariantMap parts = md->decodeUri( mDatabaseUri );
    mDatabaseUri = parts.value( u"path"_s ).toString();
    uri = QgsDataSourceUri( u"dbname='%1'"_s.arg( mDatabaseUri ) );
  }

  std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn;
  try
  {
    QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( u"spatialite"_s );
    conn.reset( static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( uri.uri(), QVariantMap() ) ) );
  }
  catch ( QgsProviderConnectionException & )
  {
    throw QgsProcessingException( QObject::tr( "Could not connect to %1" ).arg( uri.uri() ) );
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

  uri = QgsDataSourceUri( conn->uri() );
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
