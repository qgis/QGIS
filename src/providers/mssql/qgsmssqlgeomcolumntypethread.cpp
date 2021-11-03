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

#include <QSqlDatabase>
#include <QSqlQuery>

#include "qgslogger.h"
#include "qgsmssqlprovider.h"
#include "qgsmssqldatabase.h"

QgsMssqlGeomColumnTypeThread::QgsMssqlGeomColumnTypeThread( const QString &service, const QString &host, const QString &database, const QString &username, const QString &password, bool useEstimatedMetadata )
  : mService( service )
  , mHost( host )
  , mDatabase( database )
  , mUsername( username )
  , mPassword( password )
  , mUseEstimatedMetadata( useEstimatedMetadata )
  , mStopped( false )
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

  for ( QList<QgsMssqlLayerProperty>::iterator it = layerProperties.begin(),
        end = layerProperties.end();
        it != end; ++it )
  {
    QgsMssqlLayerProperty &layerProperty = *it;

    if ( !mStopped )
    {
      const QString table = QStringLiteral( "%1[%2]" )
                            .arg( layerProperty.schemaName.isEmpty() ? QString() : QStringLiteral( "[%1]." ).arg( layerProperty.schemaName ),
                                  layerProperty.tableName );

      const QString query = QStringLiteral( "SELECT %3"
                                            " UPPER([%1].STGeometryType()),"
                                            " [%1].STSrid"
                                            " FROM %2"
                                            " WHERE [%1] IS NOT NULL %4"
                                            " GROUP BY [%1].STGeometryType(), [%1].STSrid" )
                            .arg( layerProperty.geometryColName,
                                  table,
                                  mUseEstimatedMetadata ? "TOP 1" : "",
                                  layerProperty.sql.isEmpty() ? QString() : QStringLiteral( " AND %1" ).arg( layerProperty.sql ) );

      // issue the sql query
      std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( mService, mHost, mDatabase, mUsername, mPassword );
      if ( !db->isValid() )
      {
        QgsDebugMsg( db->errorText() );
        continue;
      }

      QSqlQuery q = QSqlQuery( db->db() );
      q.setForwardOnly( true );
      if ( !q.exec( query ) )
      {
        QgsDebugMsg( q.lastError().text() );
      }

      QString type;
      QString srid;

      if ( q.isActive() )
      {
        QStringList types;
        QStringList srids;

        while ( q.next() )
        {
          const QString type = q.value( 0 ).toString().toUpper();
          const QString srid = q.value( 1 ).toString();

          if ( type.isEmpty() )
            continue;

          types << type;
          srids << srid;
        }

        type = types.join( QLatin1Char( ',' ) );
        srid = srids.join( QLatin1Char( ',' ) );
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
