/***************************************************************************
  qgsdb2geometrycolumns.cpp - Access DB2 geometry columns table
  --------------------------------------
  Date      : 2016-01-27
  Copyright : (C) 2016 by David Adler
                          Shirley Xiao, David Nguyen
  Email     : dadler at adtechgeospatial.com
              xshirley2012 at yahoo.com, davidng0123 at gmail.com
****************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsdb2geometrycolumns.h"
#include "qgsdb2tablemodel.h" // needed for QgsDB2LayerProperty
#include "qgsdb2newconnection.h" // needed for ENV_ZOS, ENV_LUW
#include <QtSql>
#include <qgslogger.h>


QgsDb2GeometryColumns::QgsDb2GeometryColumns( const QSqlDatabase db )
    : mDatabase( db )
    , mEnvironment( ENV_LUW )
{
  QgsDebugMsg( "constructing" );
}

QgsDb2GeometryColumns::~QgsDb2GeometryColumns( )
{
  mQuery.clear();
}

int QgsDb2GeometryColumns::open()
{
  return open( QString(), QString() );
}

int QgsDb2GeometryColumns::open( const QString &schemaName, const QString &tableName )
{
  QString queryExtents( "SELECT TABLE_SCHEMA, TABLE_NAME, COLUMN_NAME, TYPE_NAME, "
                        "SRS_ID, SRS_NAME, MIN_X, MIN_Y, MAX_X, MAX_Y "
                        "FROM DB2GSE.ST_GEOMETRY_COLUMNS" );
  QString queryNoExtents( "SELECT TABLE_SCHEMA, TABLE_NAME, COLUMN_NAME, TYPE_NAME, "
                          "SRS_ID, SRS_NAME "
                          "FROM DB2GSE.ST_GEOMETRY_COLUMNS" );
  mQuery = QSqlQuery( mDatabase );
  int sqlcode = 0;
  mEnvironment = ENV_LUW;

  if ( !schemaName.isEmpty() && !tableName.isEmpty() )
  {
    QString whereClause = QString( " WHERE TABLE_SCHEMA = '%1' AND TABLE_NAME = '%2'" )
                          .arg( schemaName, tableName );
    queryExtents += whereClause;
    queryNoExtents += whereClause;
  }
  QgsDebugMsg( queryExtents );
  // issue the sql query
  if ( !mQuery.exec( queryExtents ) )
  {
    QgsDebugMsg( "ST_Geometry_Columns query failed: " + mDatabase.lastError().text() );
    sqlcode =  mQuery.lastError().number();
    QgsDebugMsg( QString( "SQLCODE: %1" ).arg( sqlcode ) );
    /* The MIN_X, MIN_Y, MAX_X, and MAX_Y columns are not available on z/OS (and LUW 9.5)
       so SQLCODE -206 is returned when specifying non-existent columns. */
    if ( mQuery.lastError().number() == -206 )
    {
      QgsDebugMsg( "Try query with no extents" );
      mQuery.clear();

      if ( !mQuery.exec( queryNoExtents ) )
      {
        QgsDebugMsg( QString( "SQLCODE: %1" ).arg( mQuery.lastError().number() ) );
      }
      else
      {
        QgsDebugMsg( "success; must be z/OS" );
        mEnvironment = ENV_ZOS;
        sqlcode = 0;
      }
    }
  }
//  QgsDebugMsg( QString( "sqlcode: %1" ).arg( sqlcode ) );

  return sqlcode;
}

bool QgsDb2GeometryColumns::isActive()
{
  return mQuery.isActive();
}

int QgsDb2GeometryColumns::db2Environment()
{
  return mEnvironment;
}

bool QgsDb2GeometryColumns::populateLayerProperty( QgsDb2LayerProperty &layer )
{
  if ( !mQuery.isActive() || !mQuery.next() )
  {
    return false;
  }

  layer.schemaName = mQuery.value( 0 ).toString().trimmed();
  layer.tableName = mQuery.value( 1 ).toString().trimmed();
  layer.geometryColName = mQuery.value( 2 ).toString().trimmed();
  layer.type = mQuery.value( 3 ).toString();
  if ( mQuery.value( 4 ).isNull() )
  {
    layer.srid = QString( "" );
    layer.srsName = QString( "" );
  }
  else
  {
    layer.srid = mQuery.value( 4 ).toString();
    layer.srsName = mQuery.value( 5 ).toString();
  }
  layer.extents = QString( "0 0 0 0" ); // no extents
  if ( ENV_LUW == mEnvironment )
  {
    if ( !mQuery.value( 6 ).isNull() ) // Don't get values if null
    {
      layer.extents = QString(
                        mQuery.value( 6 ).toString() + ' ' +
                        mQuery.value( 7 ).toString() + ' ' +
                        mQuery.value( 8 ).toString() + ' ' +
                        mQuery.value( 9 ).toString() ).trimmed();
    }
  }
  QgsDebugMsg( QString( "layer: %1.%2(%3) type='%4' srid='%5' srsName='%6'" )
               .arg( layer.schemaName, layer.tableName, layer.geometryColName,
                     layer.type, layer.srid, layer.srsName ) );
  QgsDebugMsg( "Extents: '" + layer.extents + "'" );

  layer.pkCols = QStringList();

  // Use the Qt functionality to get the primary key information
  // to set the FID column.
  // We can only use the primary key if it only has one column and
  // the type is Integer or BigInt.
  QString table = QString( "%1.%2" ).arg( layer.schemaName, layer.tableName );
  QSqlIndex pk = mDatabase.primaryIndex( table );
  if ( pk.count() == 1 )
  {
    QSqlField pkFld = pk.field( 0 );
    QVariant::Type pkType = pkFld.type();
    if (( pkType == QVariant::Int ||  pkType == QVariant::LongLong ) )
    {
      QString fidColName = pk.fieldName( 0 );
      layer.pkCols.append( fidColName );
      QgsDebugMsg( "pk is: " + fidColName );
    }
  }
  else
  {
    QgsDebugMsg( "Warning: table primary key count is " + QString::number( pk.count() ) );
  }
  layer.pkColumnName = layer.pkCols.size() > 0 ? layer.pkCols.at( 0 ) : QString::null;
  return true;
}
