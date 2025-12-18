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
  return QStringLiteral( "postgisexecuteandloadsql" );
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
  return QStringLiteral( "database" );
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
  addParameter( new QgsProcessingParameterProviderConnection( QStringLiteral( "DATABASE" ), QObject::tr( "Database (connection name)" ), QStringLiteral( "postgres" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "SQL" ), QObject::tr( "SQL query" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "ID_FIELD" ), QObject::tr( "Unique ID field name" ), QStringLiteral( "id" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "GEOMETRY_FIELD" ), QObject::tr( "Geometry field name" ), QStringLiteral( "geom" ), false, true ) );
  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Output layer" ) ) );
}

QVariantMap QgsExecuteAndLoadPostgisQueryAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  const QString connName = parameterAsConnectionName( parameters, QStringLiteral( "DATABASE" ), context );

  std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn;
  try
  {
    QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "postgres" ) );
    conn.reset( static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connName ) ) );
  }
  catch ( QgsProviderConnectionException & )
  {
    throw QgsProcessingException( QObject::tr( "Could not retrieve connection details for %1" ).arg( connName ) );
  }

  QString sql = parameterAsString( parameters, QStringLiteral( "SQL" ), context ).replace( '\n', ' ' );
  if ( sql.endsWith( ';' ) )
  {
    sql.chop( 1 );
  }
  const QString idField = parameterAsString( parameters, QStringLiteral( "ID_FIELD" ), context );
  const QString geomField = parameterAsString( parameters, QStringLiteral( "GEOMETRY_FIELD" ), context );

  QgsDataSourceUri uri( conn->uri() );
  uri.setDataSource( "", QStringLiteral( "(%1)" ).arg( sql ), geomField, "", idField );

  auto layer = std::make_unique<QgsVectorLayer>( uri.uri(), QStringLiteral( "layername" ), QStringLiteral( "postgres" ) );
  if ( !layer->isValid() )
  {
    throw QgsProcessingException( QObject::tr( "This layer is invalid! Please check the PostGIS log for error messages." ) );
  }

  const QString layerId = layer->id();
  const QgsProcessingContext::LayerDetails details( QStringLiteral( "SQL layer" ), context.project(), QStringLiteral( "OUTPUT" ), QgsProcessingUtils::LayerHint::Vector );
  context.addLayerToLoadOnCompletion( layerId, details );
  context.temporaryLayerStore()->addMapLayer( layer.release() );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), layerId );
  return outputs;
}

///@endcond
