/***************************************************************************
                         qgsalgorithmexecutepostgisquery.cpp
                         ---------------------
    begin                : May 2020
    copyright            : (C) 2020 by Alexander Bruy
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

#include "qgsalgorithmexecutespatialitequery.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsabstractdatabaseproviderconnection.h"

///@cond PRIVATE

QString QgsExecuteSpatialiteQueryAlgorithm::name() const
{
  return QStringLiteral( "spatialiteexecutesql" );
}

QString QgsExecuteSpatialiteQueryAlgorithm::displayName() const
{
  return QObject::tr( "SpatiaLite execute SQL" );
}

QStringList QgsExecuteSpatialiteQueryAlgorithm::tags() const
{
  return QObject::tr( "database,sql,spatialite,execute" ).split( ',' );
}

QString QgsExecuteSpatialiteQueryAlgorithm::group() const
{
  return QObject::tr( "Database" );
}

QString QgsExecuteSpatialiteQueryAlgorithm::groupId() const
{
  return QStringLiteral( "database" );
}

QString QgsExecuteSpatialiteQueryAlgorithm::shortHelpString() const
{
  return QObject::tr( "Executes a SQL command on a SpatiaLite database." );
}

QgsExecuteSpatialiteQueryAlgorithm *QgsExecuteSpatialiteQueryAlgorithm::createInstance() const
{
  return new QgsExecuteSpatialiteQueryAlgorithm();
}

void QgsExecuteSpatialiteQueryAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterProviderConnection( QStringLiteral( "DATABASE" ), QObject::tr( "Database (connection name)" ), QStringLiteral( "spatialite" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "SQL" ), QObject::tr( "SQL query" ), QVariant(), true ) );
}

QVariantMap QgsExecuteSpatialiteQueryAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QString connName = parameterAsConnectionName( parameters, QStringLiteral( "DATABASE" ), context );

  std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn;
  try
  {
    std::unique_ptr<QgsProviderMetadata> md( QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "spatialite" ) ) );
    conn.reset( static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connName ) ) );
  }
  catch ( QgsProviderConnectionException & )
  {
    throw QgsProcessingException( QObject::tr( "Could not retrieve connection details for %1" ).arg( connName ) );
  }

  QString sql = parameterAsString( parameters, QStringLiteral( "SQL" ), context ).replace( '\n', ' ' );
  try
  {
    conn->executeSql( sql );
  }
  catch ( QgsProviderConnectionException &ex )
  {
    throw QgsProcessingException( QObject::tr( "Error executing SQL:\n%1" ).arg( ex.what() ) );
  }

  return QVariantMap();
}

///@endcond
