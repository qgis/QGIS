/***************************************************************************
    qgsmssqlgeomcolumntypethread.cpp
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmssqlgeomcolumntypethread.h"

#include "qgslogger.h"
#include "qgsmssqldatabase.h"
#include "qgsmssqlprovider.h"

#include <QSqlDatabase>
#include <QSqlQuery>

#include "moc_qgsmssqlgeomcolumntypethread.cpp"

QgsMssqlGeomColumnTypeThread::QgsMssqlGeomColumnTypeThread( const QString &service, const QString &host, const QString &database, const QString &username, const QString &password, bool useEstimatedMetadata, bool disableInvalidGeometryHandling )
  : mService( service )
  , mHost( host )
  , mDatabase( database )
  , mUsername( username )
  , mPassword( password )
  , mUseEstimatedMetadata( useEstimatedMetadata )
  , mDisableInvalidGeometryHandling( disableInvalidGeometryHandling )
{
  qRegisterMetaType<QgsMssqlLayerProperty>( "QgsMssqlLayerProperty" );
}

void QgsMssqlGeomColumnTypeThread::addGeometryColumn( const QgsMssqlLayerProperty &layerProperty )
{
  layerProperties << layerProperty;
}

void QgsMssqlGeomColumnTypeThread::stop()
{
  mStopped = true;
}

void QgsMssqlGeomColumnTypeThread::run()
{
  mStopped = false;

  QgsDataSourceUri uri;
  uri.setService( mService );
  uri.setHost( mHost );
  uri.setDatabase( mDatabase );
  uri.setUsername( mUsername );
  uri.setPassword( mPassword );

  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( uri );
  if ( !db->isValid() )
  {
    QgsDebugError( db->errorText() );
    return;
  }

  for ( QList<QgsMssqlLayerProperty>::iterator it = layerProperties.begin(),
                                               end = layerProperties.end();
        it != end; ++it )
  {
    QgsMssqlLayerProperty &layerProperty = *it;

    if ( !mStopped )
    {
      const QString table = u"%1[%2]"_s
                              .arg( layerProperty.schemaName.isEmpty() ? QString() : u"[%1]."_s.arg( layerProperty.schemaName ), layerProperty.tableName );

      QString query;
      if ( mDisableInvalidGeometryHandling )
      {
        query = QStringLiteral( "SELECT %3"
                                " UPPER([%1].STGeometryType()),"
                                " [%1].STSrid,"
                                " [%1].HasZ,"
                                " [%1].HasM"
                                " FROM %2"
                                " WHERE [%1] IS NOT NULL %4"
                                " GROUP BY [%1].STGeometryType(), [%1].STSrid, [%1].HasZ, [%1].HasM" )
                  .arg( layerProperty.geometryColName, table, mUseEstimatedMetadata ? "TOP 1" : "", layerProperty.sql.isEmpty() ? QString() : u" AND %1"_s.arg( layerProperty.sql ) );
      }
      else
      {
        query = QStringLiteral(
                  R"raw(
                        SELECT type, srid, hasz, hasm FROM
                            (SELECT %3 UPPER((CASE WHEN [%1].STIsValid() = 0 THEN [%1].MakeValid() ELSE [%1] END).STGeometryType()) as type,
                             [%1].STSrid as srid, [%1].HasZ as hasz, [%1].HasM as hasm FROM %2 WHERE [%1] IS NOT NULL %4) AS a
                        GROUP BY type, srid, hasz, hasm
                        )raw"
        )
                  .arg( layerProperty.geometryColName, table, mUseEstimatedMetadata ? "TOP 1" : "", layerProperty.sql.isEmpty() ? QString() : u" AND %1"_s.arg( layerProperty.sql ) );
      }

      // issue the sql query
      QSqlQuery q = QSqlQuery( db->db() );
      q.setForwardOnly( true );
      if ( !q.exec( query ) )
      {
        QgsDebugError( q.lastError().text() );
      }

      QString type;
      QString srid;

      if ( q.isActive() )
      {
        QStringList types;
        QStringList srids;

        while ( q.next() )
        {
          const bool hasZ { q.value( 2 ).toString() == '1' };
          const bool hasM { q.value( 3 ).toString() == '1' };
          const int dimensions { 2 + ( ( hasZ && hasM ) ? 2 : ( ( hasZ || hasM ) ? 1 : 0 ) ) };
          QString typeName { q.value( 0 ).toString().toUpper() };
          if ( typeName.isEmpty() )
            continue;

          if ( hasM && !typeName.endsWith( 'M' ) )
          {
            typeName.append( 'M' );
          }
          const QString type { QgsMssqlProvider::typeFromMetadata( typeName, dimensions ) };
          const QString srid = q.value( 1 ).toString();

          if ( type.isEmpty() )
            continue;

          types << type;
          srids << srid;
        }

        type = types.join( ','_L1 );
        srid = srids.join( ','_L1 );
      }

      layerProperty.type = type;
      layerProperty.srid = srid;
    }
    else
    {
      layerProperty.type.clear();
      layerProperty.srid.clear();
    }

    // Now tell the layer list dialog box...
    emit setLayerType( layerProperty );
  }
}
