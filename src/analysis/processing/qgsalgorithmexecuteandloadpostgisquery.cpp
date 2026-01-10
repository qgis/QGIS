/***************************************************************************
                         qgsalgorithmexecuteandloadpostgisquery.cpp
                         ---------------------
    begin                : December 2025
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

#include "qgsalgorithmexecuteandloadpostgisquery.h"

#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsExecuteAndLoadPostgisQueryAlgorithm::name() const
{
  return u"postgisexecuteandloadsql"_s;
}

QString QgsExecuteAndLoadPostgisQueryAlgorithm::displayName() const
{
  return QObject::tr( "PostgreSQL execute and load SQL" );
}

QStringList QgsExecuteAndLoadPostgisQueryAlgorithm::tags() const
{
  return QObject::tr( "database,sql,postgresql,postgis,execute,load,layer,table" ).split( ',' );
}

QString QgsExecuteAndLoadPostgisQueryAlgorithm::group() const
{
  return QObject::tr( "Database" );
}

QString QgsExecuteAndLoadPostgisQueryAlgorithm::groupId() const
{
  return u"database"_s;
}

QString QgsExecuteAndLoadPostgisQueryAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm performs a SQL database query on a PostgreSQL database connected to QGIS and loads the query results as a new layer." );
}

QString QgsExecuteAndLoadPostgisQueryAlgorithm::shortDescription() const
{
  return QObject::tr( "Executes a SQL command on a PostgreSQL database and loads the result as a layer." );
}

Qgis::ProcessingAlgorithmFlags QgsExecuteAndLoadPostgisQueryAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NotAvailableInStandaloneTool | Qgis::ProcessingAlgorithmFlag::RequiresProject;
}

QgsExecuteAndLoadPostgisQueryAlgorithm *QgsExecuteAndLoadPostgisQueryAlgorithm::createInstance() const
{
  return new QgsExecuteAndLoadPostgisQueryAlgorithm();
}

void QgsExecuteAndLoadPostgisQueryAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterProviderConnection( u"DATABASE"_s, QObject::tr( "Database (connection name)" ), u"postgres"_s ) );
  addParameter( new QgsProcessingParameterString( u"SQL"_s, QObject::tr( "SQL query" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterString( u"ID_FIELD"_s, QObject::tr( "Unique ID field name" ), u"id"_s ) );
  addParameter( new QgsProcessingParameterString( u"GEOMETRY_FIELD"_s, QObject::tr( "Geometry field name" ), u"geom"_s, false, true ) );
  addOutput( new QgsProcessingOutputVectorLayer( u"OUTPUT"_s, QObject::tr( "Output layer" ) ) );
}

QVariantMap QgsExecuteAndLoadPostgisQueryAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  const QString connName = parameterAsConnectionName( parameters, u"DATABASE"_s, context );

  std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn;
  try
  {
    QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( u"postgres"_s );
    conn.reset( static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connName ) ) );
  }
  catch ( QgsProviderConnectionException & )
  {
    throw QgsProcessingException( QObject::tr( "Could not retrieve connection details for %1" ).arg( connName ) );
  }

  QString sql = parameterAsString( parameters, u"SQL"_s, context ).replace( '\n', ' ' );
  if ( sql.endsWith( ';' ) )
  {
    sql.chop( 1 );
  }
  const QString idField = parameterAsString( parameters, u"ID_FIELD"_s, context );
  const QString geomField = parameterAsString( parameters, u"GEOMETRY_FIELD"_s, context );

  QgsDataSourceUri uri( conn->uri() );
  uri.setDataSource( QString(), u"(%1)"_s.arg( sql ), geomField, QString(), idField );

  auto layer = std::make_unique<QgsVectorLayer>( uri.uri(), u"layername"_s, u"postgres"_s );
  if ( !layer->isValid() )
  {
    throw QgsProcessingException( QObject::tr( "This layer is invalid! Please check the PostGIS log for error messages." ) );
  }

  const QString layerId = layer->id();
  const QgsProcessingContext::LayerDetails details( u"SQL layer"_s, context.project(), u"OUTPUT"_s, QgsProcessingUtils::LayerHint::Vector );
  context.addLayerToLoadOnCompletion( layerId, details );
  context.temporaryLayerStore()->addMapLayer( layer.release() );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, layerId );
  return outputs;
}

///@endcond
