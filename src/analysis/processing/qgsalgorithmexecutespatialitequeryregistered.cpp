/***************************************************************************
                         qgsalgorithmexecutespatialitequeryregistered.cpp
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

#include "qgsalgorithmexecutespatialitequeryregistered.h"

#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"

///@cond PRIVATE

QString QgsExecuteRegisteredSpatialiteQueryAlgorithm::name() const
{
  return u"spatialiteexecutesqlregistered"_s;
}

QString QgsExecuteRegisteredSpatialiteQueryAlgorithm::displayName() const
{
  return QObject::tr( "SpatiaLite execute SQL (registered DB)" );
}

QStringList QgsExecuteRegisteredSpatialiteQueryAlgorithm::tags() const
{
  return QObject::tr( "database,sql,spatialite,execute" ).split( ',' );
}

QString QgsExecuteRegisteredSpatialiteQueryAlgorithm::group() const
{
  return QObject::tr( "Database" );
}

QString QgsExecuteRegisteredSpatialiteQueryAlgorithm::groupId() const
{
  return u"database"_s;
}

QString QgsExecuteRegisteredSpatialiteQueryAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm executes a SQL command on a SpatiaLite database." );
}

QString QgsExecuteRegisteredSpatialiteQueryAlgorithm::shortDescription() const
{
  return QObject::tr( "Executes a SQL command on a SpatiaLite database." );
}

QgsExecuteRegisteredSpatialiteQueryAlgorithm *QgsExecuteRegisteredSpatialiteQueryAlgorithm::createInstance() const
{
  return new QgsExecuteRegisteredSpatialiteQueryAlgorithm();
}

void QgsExecuteRegisteredSpatialiteQueryAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterProviderConnection( u"DATABASE"_s, QObject::tr( "Database (connection name)" ), u"spatialite"_s ) );
  addParameter( new QgsProcessingParameterString( u"SQL"_s, QObject::tr( "SQL query" ), QVariant(), true ) );
}

QVariantMap QgsExecuteRegisteredSpatialiteQueryAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  const QString connName = parameterAsConnectionName( parameters, u"DATABASE"_s, context );

  std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn;
  try
  {
    QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( u"spatialite"_s );
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
