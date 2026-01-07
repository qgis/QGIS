/***************************************************************************
                         qgsalgorithmexecutespatialitequery.cpp
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

#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsExecuteSpatialiteQueryAlgorithm::name() const
{
  return u"spatialiteexecutesql"_s;
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
  return u"database"_s;
}

QString QgsExecuteSpatialiteQueryAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm executes a SQL command on a SpatiaLite database. The database is determined by an input layer or file." );
}

QString QgsExecuteSpatialiteQueryAlgorithm::shortDescription() const
{
  return QObject::tr( "Executes a SQL command on a SpatiaLite database determined by an input layer or file." );
}

QgsExecuteSpatialiteQueryAlgorithm *QgsExecuteSpatialiteQueryAlgorithm::createInstance() const
{
  return new QgsExecuteSpatialiteQueryAlgorithm();
}

void QgsExecuteSpatialiteQueryAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterVectorLayer( u"DATABASE"_s, QObject::tr( "Database layer (or file)" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterString( u"SQL"_s, QObject::tr( "SQL query" ), QVariant(), true ) );
}

QVariantMap QgsExecuteSpatialiteQueryAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );
  QgsVectorLayer *layer = parameterAsVectorLayer( parameters, u"DATABASE"_s, context );
  QString databaseUri = layer->dataProvider()->dataSourceUri();
  QgsDataSourceUri uri( databaseUri );
  if ( uri.database().isEmpty() )
  {
    if ( databaseUri.contains( u"|layername"_s, Qt::CaseInsensitive ) )
      databaseUri = databaseUri.left( databaseUri.indexOf( "|layername"_L1 ) );
    else if ( databaseUri.contains( u"|layerid"_s, Qt::CaseInsensitive ) )
      databaseUri = databaseUri.left( databaseUri.indexOf( "|layerid"_L1 ) );

    uri = QgsDataSourceUri( u"dbname='%1'"_s.arg( databaseUri ) );
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
