/***************************************************************************
      qgspostgresutils.cpp  -  Utils for PostgreSQL/PostGIS 
                             -------------------
    begin                : Jan 2, 2004
    copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslogger.h"
#include "qgspostgresutils.h"

// ----------

void QgsPostgresSharedData::addFeaturesCounted( long long diff )
{
  QMutexLocker locker( &mMutex );

  if ( mFeaturesCounted >= 0 )
    mFeaturesCounted += diff;
}

void QgsPostgresSharedData::ensureFeaturesCountedAtLeast( long long fetched )
{
  QMutexLocker locker( &mMutex );

  /* only updates the feature count if it was already once.
   * Otherwise, this would lead to false feature count if
   * an existing project is open at a restrictive extent.
   */
  if ( mFeaturesCounted > 0 && mFeaturesCounted < fetched )
  {
    QgsDebugMsgLevel( QStringLiteral( "feature count adjusted from %1 to %2" ).arg( mFeaturesCounted ).arg( fetched ), 2 );
    mFeaturesCounted = fetched;
  }
}

long long QgsPostgresSharedData::featuresCounted()
{
  QMutexLocker locker( &mMutex );
  return mFeaturesCounted;
}

void QgsPostgresSharedData::setFeaturesCounted( long long count )
{
  QMutexLocker locker( &mMutex );
  mFeaturesCounted = count;
}


QgsFeatureId QgsPostgresSharedData::lookupFid( const QVariantList &v )
{
  QMutexLocker locker( &mMutex );

  QMap<QVariantList, QgsFeatureId>::const_iterator it = mKeyToFid.constFind( v );

  if ( it != mKeyToFid.constEnd() )
  {
    return it.value();
  }

  mFidToKey.insert( ++mFidCounter, v );
  mKeyToFid.insert( v, mFidCounter );

  return mFidCounter;
}


QVariantList QgsPostgresSharedData::removeFid( QgsFeatureId fid )
{
  QMutexLocker locker( &mMutex );

  QVariantList v = mFidToKey[fid];
  mFidToKey.remove( fid );
  mKeyToFid.remove( v );
  return v;
}

void QgsPostgresSharedData::insertFid( QgsFeatureId fid, const QVariantList &k )
{
  QMutexLocker locker( &mMutex );

  mFidToKey.insert( fid, k );
  mKeyToFid.insert( k, fid );
}

QVariantList QgsPostgresSharedData::lookupKey( QgsFeatureId featureId )
{
  QMutexLocker locker( &mMutex );

  QMap<QgsFeatureId, QVariantList>::const_iterator it = mFidToKey.constFind( featureId );
  if ( it != mFidToKey.constEnd() )
    return it.value();
  return QVariantList();
}

void QgsPostgresSharedData::clear()
{
  QMutexLocker locker( &mMutex );
  mFidToKey.clear();
  mKeyToFid.clear();
  mFeaturesCounted = -1;
  mFidCounter = 0;
}

void QgsPostgresSharedData::clearSupportsEnumValuesCache()
{
  QMutexLocker locker( &mMutex );
  mFieldSupportsEnumValues.clear();
}

bool QgsPostgresSharedData::fieldSupportsEnumValuesIsSet( int index )
{
  QMutexLocker locker( &mMutex );
  return mFieldSupportsEnumValues.contains( index );
}

bool QgsPostgresSharedData::fieldSupportsEnumValues( int index )
{
  QMutexLocker locker( &mMutex );
  return mFieldSupportsEnumValues.contains( index ) && mFieldSupportsEnumValues[index];
}

void QgsPostgresSharedData::setFieldSupportsEnumValues( int index, bool isSupported )
{
  QMutexLocker locker( &mMutex );
  mFieldSupportsEnumValues[index] = isSupported;
}

// ----------

QString QgsPostgresUtils::whereClause( QgsFeatureId featureId, const QgsFields &fields, QgsPostgresConn *conn, QgsPostgresPrimaryKeyType pkType, const QList<int> &pkAttrs, const std::shared_ptr<QgsPostgresSharedData> &sharedData )
{
  QString whereClause;

  switch ( pkType )
  {
    case PktTid:
      whereClause = QStringLiteral( "ctid='(%1,%2)'" )
                      .arg( FID_TO_NUMBER( featureId ) >> 16 )
                      .arg( FID_TO_NUMBER( featureId ) & 0xffff );
      break;

    case PktOid:
      whereClause = QStringLiteral( "oid=%1" ).arg( featureId );
      break;

    case PktInt:
      Q_ASSERT( pkAttrs.size() == 1 );
      whereClause = QStringLiteral( "%1=%2" ).arg( QgsPostgresConn::quotedIdentifier( fields.at( pkAttrs[0] ).name() ) ).arg( QgsPostgresUtils::fid_to_int32pk( featureId ) );
      break;

    case PktInt64:
    case PktUint64:
    {
      Q_ASSERT( pkAttrs.size() == 1 );
      QVariantList pkVals = sharedData->lookupKey( featureId );
      if ( !pkVals.isEmpty() )
      {
        QgsField fld = fields.at( pkAttrs[0] );
        whereClause = conn->fieldExpression( fld );
        if ( !QgsVariantUtils::isNull( pkVals[0] ) )
          whereClause += '=' + pkVals[0].toString();
        else
          whereClause += QLatin1String( " IS NULL" );
      }
    }
    break;

    case PktFidMap:
    {
      QVariantList pkVals = sharedData->lookupKey( featureId );
      if ( !pkVals.isEmpty() )
      {
        Q_ASSERT( pkVals.size() == pkAttrs.size() );

        QString delim;
        for ( int i = 0; i < pkAttrs.size(); i++ )
        {
          int idx = pkAttrs[i];
          QgsField fld = fields.at( idx );

          whereClause += delim + conn->fieldExpressionForWhereClause( fld, static_cast<QMetaType::Type>( pkVals[i].userType() ) );
          if ( QgsVariantUtils::isNull( pkVals[i] ) )
            whereClause += QLatin1String( " IS NULL" );
          else
            whereClause += '=' + QgsPostgresConn::quotedValue( pkVals[i] ); // remove toString as it must be handled by quotedValue function

          delim = QStringLiteral( " AND " );
        }
      }
      else
      {
        QgsDebugError( QStringLiteral( "FAILURE: Key values for feature %1 not found." ).arg( featureId ) );
        whereClause = QStringLiteral( "NULL" );
      }
    }
    break;

    case PktUnknown:
      Q_ASSERT( !"FAILURE: Primary key unknown" );
      whereClause = QStringLiteral( "NULL" );
      break;
  }

  return whereClause;
}

QString QgsPostgresUtils::whereClause( const QgsFeatureIds &featureIds, const QgsFields &fields, QgsPostgresConn *conn, QgsPostgresPrimaryKeyType pkType, const QList<int> &pkAttrs, const std::shared_ptr<QgsPostgresSharedData> &sharedData )
{
  auto lookupKeyWhereClause = [=] {
    if ( featureIds.isEmpty() )
      return QString();

    //simple primary key, so prefer to use an "IN (...)" query. These are much faster then multiple chained ...OR... clauses
    QString delim;
    QString expr = QStringLiteral( "%1 IN (" ).arg( QgsPostgresConn::quotedIdentifier( fields.at( pkAttrs[0] ).name() ) );

    for ( const QgsFeatureId featureId : std::as_const( featureIds ) )
    {
      const QVariantList pkVals = sharedData->lookupKey( featureId );
      if ( !pkVals.isEmpty() )
      {
        expr += delim + QgsPostgresConn::quotedValue( pkVals.at( 0 ) );
        delim = ',';
      }
    }
    expr += ')';

    return expr;
  };

  switch ( pkType )
  {
    case PktOid:
    case PktInt:
    {
      QString expr;

      //simple primary key, so prefer to use an "IN (...)" query. These are much faster then multiple chained ...OR... clauses
      if ( !featureIds.isEmpty() )
      {
        QString delim;
        expr = QStringLiteral( "%1 IN (" ).arg( ( pkType == PktOid ? QStringLiteral( "oid" ) : QgsPostgresConn::quotedIdentifier( fields.at( pkAttrs[0] ).name() ) ) );

        for ( const QgsFeatureId featureId : std::as_const( featureIds ) )
        {
          expr += delim + FID_TO_STRING( ( pkType == PktOid ? featureId : QgsPostgresUtils::fid_to_int32pk( featureId ) ) );
          delim = ',';
        }
        expr += ')';
      }

      return expr;
    }
    case PktInt64:
    case PktUint64:
      return lookupKeyWhereClause();

    case PktFidMap:
    case PktTid:
    case PktUnknown:
    {
      // on simple string primary key we can use IN
      if ( pkType == PktFidMap && pkAttrs.count() == 1 && fields.at( pkAttrs[0] ).type() == QMetaType::Type::QString )
        return lookupKeyWhereClause();

      //complex primary key, need to build up where string
      QStringList whereClauses;
      for ( const QgsFeatureId featureId : std::as_const( featureIds ) )
      {
        whereClauses << whereClause( featureId, fields, conn, pkType, pkAttrs, sharedData );
      }
      return whereClauses.isEmpty() ? QString() : whereClauses.join( QLatin1String( " OR " ) ).prepend( '(' ).append( ')' );
    }
  }
  return QString(); //avoid warning
}

QString QgsPostgresUtils::andWhereClauses( const QString &c1, const QString &c2 )
{
  if ( c1.isEmpty() )
    return c2;
  if ( c2.isEmpty() )
    return c1;

  return QStringLiteral( "(%1) AND (%2)" ).arg( c1, c2 );
}

void QgsPostgresUtils::replaceInvalidXmlChars( QString &xml )
{
  static const QRegularExpression replaceRe { QStringLiteral( "([\\x00-\\x08\\x0B-\\x1F\\x7F])" ) };
  QRegularExpressionMatchIterator it { replaceRe.globalMatch( xml ) };
  while ( it.hasNext() )
  {
    const QRegularExpressionMatch match { it.next() };
    const QChar c { match.captured( 1 ).at( 0 ) };
    xml.replace( c, QStringLiteral( "UTF-8[%1]" ).arg( c.unicode() ) );
  }
}

void QgsPostgresUtils::restoreInvalidXmlChars( QString &xml )
{
  static const QRegularExpression replaceRe { QStringLiteral( R"raw(UTF-8\[(\d+)\])raw" ) };
  QRegularExpressionMatchIterator it { replaceRe.globalMatch( xml ) };
  while ( it.hasNext() )
  {
    const QRegularExpressionMatch match { it.next() };
    bool ok;
    const ushort code { match.captured( 1 ).toUShort( &ok ) };
    if ( ok )
    {
      xml.replace( QStringLiteral( "UTF-8[%1]" ).arg( code ), QChar( code ) );
    }
  }
}

bool QgsPostgresUtils::deleteLayer( const QString &uri, QString &errCause )
{
  QgsDebugMsgLevel( "deleting layer " + uri, 2 );

  QgsDataSourceUri dsUri( uri );
  QString schemaName = dsUri.schema();
  QString tableName = dsUri.table();
  QString geometryCol = dsUri.geometryColumn();

  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsPostgresConn::quotedIdentifier( schemaName ) + '.';
  }
  schemaTableName += QgsPostgresConn::quotedIdentifier( tableName );

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri, false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  // handle deletion of views
  QString sqlViewCheck = QStringLiteral( "SELECT relkind FROM pg_class WHERE oid=regclass(%1)::oid" )
                           .arg( QgsPostgresConn::quotedValue( schemaTableName ) );
  QgsPostgresResult resViewCheck( conn->LoggedPQexec( "QgsPostgresUtils", sqlViewCheck ) );
  const QString type = resViewCheck.PQgetvalue( 0, 0 );
  const Qgis::PostgresRelKind relKind = QgsPostgresConn::relKindFromValue( type );

  switch ( relKind )
  {
    case Qgis::PostgresRelKind::View:
    case Qgis::PostgresRelKind::MaterializedView:
    {
      QString sql = QStringLiteral( "DROP %1VIEW %2" ).arg( type == QLatin1String( "m" ) ? QStringLiteral( "MATERIALIZED " ) : QString(), schemaTableName );
      QgsPostgresResult result( conn->LoggedPQexec( "QgsPostgresUtils", sql ) );
      if ( result.PQresultStatus() != PGRES_COMMAND_OK )
      {
        errCause = QObject::tr( "Unable to delete view %1: \n%2" )
                     .arg( schemaTableName, result.PQresultErrorMessage() );
        conn->unref();
        return false;
      }
      conn->unref();
      return true;
    }

    case Qgis::PostgresRelKind::NotSet:
    case Qgis::PostgresRelKind::Unknown:
    case Qgis::PostgresRelKind::OrdinaryTable:
    case Qgis::PostgresRelKind::Index:
    case Qgis::PostgresRelKind::Sequence:
    case Qgis::PostgresRelKind::CompositeType:
    case Qgis::PostgresRelKind::ToastTable:
    case Qgis::PostgresRelKind::ForeignTable:
    case Qgis::PostgresRelKind::PartitionedTable:
    {
      // TODO -- this logic is being applied to a whole bunch
      // of potentially non-table items, eg indexes and sequences.
      // These should have special handling!

      // check the geometry column count
      QString sql = QString( "SELECT count(*) "
                             "FROM geometry_columns, pg_class, pg_namespace "
                             "WHERE f_table_name=relname AND f_table_schema=nspname "
                             "AND pg_class.relnamespace=pg_namespace.oid "
                             "AND f_table_schema=%1 AND f_table_name=%2" )
                      .arg( QgsPostgresConn::quotedValue( schemaName ), QgsPostgresConn::quotedValue( tableName ) );
      QgsPostgresResult result( conn->LoggedPQexec( "QgsPostgresUtils", sql ) );
      if ( result.PQresultStatus() != PGRES_TUPLES_OK )
      {
        errCause = QObject::tr( "Unable to delete layer %1: \n%2" )
                     .arg( schemaTableName, result.PQresultErrorMessage() );
        conn->unref();
        return false;
      }

      int count = result.PQgetvalue( 0, 0 ).toInt();

      if ( !geometryCol.isEmpty() && count > 1 )
      {
        // the table has more geometry columns, drop just the geometry column
        sql = QStringLiteral( "SELECT DropGeometryColumn(%1,%2,%3)" )
                .arg( QgsPostgresConn::quotedValue( schemaName ), QgsPostgresConn::quotedValue( tableName ), QgsPostgresConn::quotedValue( geometryCol ) );
      }
      else
      {
        // drop the table
        sql = QStringLiteral( "SELECT DropGeometryTable(%1,%2)" )
                .arg( QgsPostgresConn::quotedValue( schemaName ), QgsPostgresConn::quotedValue( tableName ) );
      }

      result = conn->LoggedPQexec( "QgsPostgresUtils", sql );
      if ( result.PQresultStatus() != PGRES_TUPLES_OK )
      {
        errCause = QObject::tr( "Unable to delete layer %1: \n%2" )
                     .arg( schemaTableName, result.PQresultErrorMessage() );
        conn->unref();
        return false;
      }

      conn->unref();
      return true;
    }
  }
  BUILTIN_UNREACHABLE
}

bool QgsPostgresUtils::deleteSchema( const QString &schema, const QgsDataSourceUri &uri, QString &errCause, bool cascade )
{
  QgsDebugMsgLevel( "deleting schema " + schema, 2 );

  if ( schema.isEmpty() )
    return false;

  QString schemaName = QgsPostgresConn::quotedIdentifier( schema );

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri, false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  // drop the schema
  QString sql = QStringLiteral( "DROP SCHEMA %1 %2" )
                  .arg( schemaName, cascade ? QStringLiteral( "CASCADE" ) : QString() );

  QgsPostgresResult result( conn->LoggedPQexec( "QgsPostgresUtils", sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    errCause = QObject::tr( "Unable to delete schema %1: \n%2" )
                 .arg( schemaName, result.PQresultErrorMessage() );
    conn->unref();
    return false;
  }

  conn->unref();
  return true;
}

bool QgsPostgresUtils::tableExists( QgsPostgresConn *conn, const QString &name )
{
  QgsPostgresResult res( conn->LoggedPQexec( QStringLiteral( "tableExists" ), "SELECT EXISTS ( SELECT oid FROM pg_catalog.pg_class WHERE relname=" + QgsPostgresConn::quotedValue( name ) + ")" ) );
  return res.PQgetvalue( 0, 0 ).startsWith( 't' );
}

bool QgsPostgresUtils::columnExists( QgsPostgresConn *conn, const QString &table, const QString &column )
{
  QgsPostgresResult res( conn->LoggedPQexec( QStringLiteral( "columnExists" ), "SELECT COUNT(*) FROM information_schema.columns WHERE table_name=" + QgsPostgresConn::quotedValue( table ) + " and column_name=" + QgsPostgresConn::quotedValue( column ) ) );
  return res.PQgetvalue( 0, 0 ).toInt() > 0;
}

bool QgsPostgresUtils::createStylesTable( QgsPostgresConn *conn, QString loggedClass )
{
  QgsPostgresResult res( conn->LoggedPQexec( loggedClass, "CREATE TABLE layer_styles("
                                                          "id SERIAL PRIMARY KEY"
                                                          ",f_table_catalog varchar"
                                                          ",f_table_schema varchar"
                                                          ",f_table_name varchar"
                                                          ",f_geometry_column varchar"
                                                          ",styleName text"
                                                          ",styleQML xml"
                                                          ",styleSLD xml"
                                                          ",useAsDefault boolean"
                                                          ",description text"
                                                          ",owner varchar(63) DEFAULT CURRENT_USER"
                                                          ",ui xml"
                                                          ",update_time timestamp DEFAULT CURRENT_TIMESTAMP"
                                                          ",type varchar"
                                                          ",r_raster_column varchar"
                                                          ")" ) );

  return res.PQresultStatus() == PGRES_COMMAND_OK;
}
