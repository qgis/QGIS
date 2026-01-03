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

#include "qgsalgorithmexecutepostgisquery.h"

#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"

///@cond PRIVATE

QString QgsExecutePostgisQueryAlgorithm::name() const
{
  return u"postgisexecutesql"_s;
}

QString QgsExecutePostgisQueryAlgorithm::displayName() const
{
  return QObject::tr( "PostgreSQL execute SQL" );
}

QStringList QgsExecutePostgisQueryAlgorithm::tags() const
{
  return QObject::tr( "database,sql,postgresql,postgis,execute" ).split( ',' );
}

QString QgsExecutePostgisQueryAlgorithm::group() const
{
  return QObject::tr( "Database" );
}

QString QgsExecutePostgisQueryAlgorithm::groupId() const
{
  return u"database"_s;
}

QString QgsExecutePostgisQueryAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm executes a SQL command on a PostgreSQL database." );
}

QString QgsExecutePostgisQueryAlgorithm::shortDescription() const
{
  return QObject::tr( "Executes a SQL command on a PostgreSQL database." );
}

QgsExecutePostgisQueryAlgorithm *QgsExecutePostgisQueryAlgorithm::createInstance() const
{
  return new QgsExecutePostgisQueryAlgorithm();
}

void QgsExecutePostgisQueryAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterProviderConnection( u"DATABASE"_s, QObject::tr( "Database (connection name)" ), u"postgres"_s ) );
  addParameter( new QgsProcessingParameterString( u"SQL"_s, QObject::tr( "SQL query" ), QVariant(), true ) );
}

QVariantMap QgsExecutePostgisQueryAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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

  const QString sql = parameterAsString( parameters, u"SQL"_s, context ).replace( '\n', ' ' );
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
