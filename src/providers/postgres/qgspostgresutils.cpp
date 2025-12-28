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

#include "qgspostgresutils.h"

#include "qgslogger.h"
#include "qgsstringutils.h"

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
    QgsDebugMsgLevel( u"feature count adjusted from %1 to %2"_s.arg( mFeaturesCounted ).arg( fetched ), 2 );
    mFeaturesCounted = fetched;
  }
}

std::shared_ptr<QgsPostgresSharedData> QgsPostgresSharedData::clone() const
{
  QMutexLocker locker( &mMutex );

  auto copy = std::make_shared<QgsPostgresSharedData>();
  copy->mFeaturesCounted = mFeaturesCounted;
  copy->mFidCounter = mFidCounter;
  copy->mKeyToFid = mKeyToFid;
  copy->mFidToKey = mFidToKey;
  copy->mFieldSupportsEnumValues = mFieldSupportsEnumValues;
  return copy;
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
      whereClause = u"ctid='(%1,%2)'"_s
                      .arg( FID_TO_NUMBER( featureId ) >> 16 )
                      .arg( FID_TO_NUMBER( featureId ) & 0xffff );
      break;

    case PktOid:
      whereClause = u"oid=%1"_s.arg( featureId );
      break;

    case PktInt:
      Q_ASSERT( pkAttrs.size() == 1 );
      whereClause = u"%1=%2"_s.arg( QgsPostgresConn::quotedIdentifier( fields.at( pkAttrs[0] ).name() ) ).arg( QgsPostgresUtils::fid_to_int32pk( featureId ) );
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
          whereClause += " IS NULL"_L1;
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
            whereClause += " IS NULL"_L1;
          else
            whereClause += '=' + QgsPostgresConn::quotedValue( pkVals[i] ); // remove toString as it must be handled by quotedValue function

          delim = u" AND "_s;
        }
      }
      else
      {
        QgsDebugError( u"FAILURE: Key values for feature %1 not found."_s.arg( featureId ) );
        whereClause = u"NULL"_s;
      }
    }
    break;

    case PktUnknown:
      Q_ASSERT( !"FAILURE: Primary key unknown" );
      whereClause = u"NULL"_s;
      break;
  }

  return whereClause;
}

QString QgsPostgresUtils::whereClause( const QgsFeatureIds &featureIds, const QgsFields &fields, QgsPostgresConn *conn, QgsPostgresPrimaryKeyType pkType, const QList<int> &pkAttrs, const std::shared_ptr<QgsPostgresSharedData> &sharedData )
{
  auto lookupKeyWhereClause = [featureIds, fields, sharedData, pkAttrs] {
    if ( featureIds.isEmpty() )
      return QString();

    //simple primary key, so prefer to use an "IN (...)" query. These are much faster then multiple chained ...OR... clauses
    QString delim;
    QString expr = u"%1 IN ("_s.arg( QgsPostgresConn::quotedIdentifier( fields.at( pkAttrs[0] ).name() ) );

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
        expr = u"%1 IN ("_s.arg( ( pkType == PktOid ? u"oid"_s : QgsPostgresConn::quotedIdentifier( fields.at( pkAttrs[0] ).name() ) ) );

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
      return whereClauses.isEmpty() ? QString() : whereClauses.join( " OR "_L1 ).prepend( '(' ).append( ')' );
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

  return u"(%1) AND (%2)"_s.arg( c1, c2 );
}

void QgsPostgresUtils::replaceInvalidXmlChars( QString &xml )
{
  static const QRegularExpression replaceRe { u"([\\x00-\\x08\\x0B-\\x1F\\x7F])"_s };
  QRegularExpressionMatchIterator it { replaceRe.globalMatch( xml ) };
  while ( it.hasNext() )
  {
    const QRegularExpressionMatch match { it.next() };
    const QChar c { match.captured( 1 ).at( 0 ) };
    xml.replace( c, u"UTF-8[%1]"_s.arg( c.unicode() ) );
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
      xml.replace( u"UTF-8[%1]"_s.arg( code ), QChar( code ) );
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
  QString sqlViewCheck = u"SELECT relkind FROM pg_class WHERE oid=regclass(%1)::oid"_s
                           .arg( QgsPostgresConn::quotedValue( schemaTableName ) );
  QgsPostgresResult resViewCheck( conn->LoggedPQexec( "QgsPostgresUtils", sqlViewCheck ) );
  const QString type = resViewCheck.PQgetvalue( 0, 0 );
  const Qgis::PostgresRelKind relKind = QgsPostgresConn::relKindFromValue( type );

  switch ( relKind )
  {
    case Qgis::PostgresRelKind::View:
    case Qgis::PostgresRelKind::MaterializedView:
    {
      QString sql = u"DROP %1VIEW %2"_s.arg( type == "m"_L1 ? u"MATERIALIZED "_s : QString(), schemaTableName );
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
        sql = u"SELECT DropGeometryColumn(%1,%2,%3)"_s
                .arg( QgsPostgresConn::quotedValue( schemaName ), QgsPostgresConn::quotedValue( tableName ), QgsPostgresConn::quotedValue( geometryCol ) );
      }
      else
      {
        // drop the table
        sql = u"SELECT DropGeometryTable(%1,%2)"_s
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
  QString sql = u"DROP SCHEMA %1 %2"_s
                  .arg( schemaName, cascade ? u"CASCADE"_s : QString() );

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

bool QgsPostgresUtils::tableExists( QgsPostgresConn *conn, const QString &schema, const QString &table )
{
  QString sql;

  if ( schema.isEmpty() )
  {
    sql = u"SELECT EXISTS ( SELECT oid FROM pg_catalog.pg_class WHERE relname= %1)"_s.arg( QgsPostgresConn::quotedValue( table ) );
  }
  else
  {
    sql = u"SELECT EXISTS ( SELECT 1 FROM information_schema.tables WHERE table_name = %1 AND table_schema = %2)"_s
            .arg( QgsPostgresConn::quotedValue( table ), QgsPostgresConn::quotedValue( schema ) );
  }

  QgsPostgresResult res( conn->LoggedPQexec( u"tableExists"_s, sql ) );
  return res.PQgetvalue( 0, 0 ).startsWith( QLatin1Char( 't' ) );
}

bool QgsPostgresUtils::columnExists( QgsPostgresConn *conn, const QString &schema, const QString &table, const QString &column )
{
  QString sqlWhereClause = u"table_name = %1 AND column_name = %3"_s.arg( QgsPostgresConn::quotedValue( table ), QgsPostgresConn::quotedValue( column ) );

  if ( !schema.isEmpty() )
  {
    sqlWhereClause.append( u" AND table_schema = %1"_s.arg( QgsPostgresConn::quotedValue( schema ) ) );
  }

  const QString sql = QStringLiteral( "SELECT EXISTS( SELECT 1 FROM information_schema.columns "
                                      "WHERE %1)" )
                        .arg( sqlWhereClause );

  QgsPostgresResult res( conn->LoggedPQexec( u"columnExists"_s, sql ) );
  return res.PQgetvalue( 0, 0 ).startsWith( QLatin1Char( 't' ) );
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

bool QgsPostgresUtils::createProjectsTable( QgsPostgresConn *conn, const QString &schemaName )
{
  // try to create projects table
  const QString sql = u"CREATE TABLE IF NOT EXISTS %1.qgis_projects(name TEXT PRIMARY KEY, metadata JSONB, content BYTEA, comment TEXT DEFAULT '')"_s
                        .arg( QgsPostgresConn::quotedIdentifier( schemaName ) );

  QgsPostgresResult res( conn->PQexec( sql ) );
  if ( res.PQresultStatus() != PGRES_COMMAND_OK )
  {
    return false;
  }

  return true;
}

bool QgsPostgresUtils::deleteProjectFromSchema( QgsPostgresConn *conn, const QString &projectName, const QString &schemaName )
{
  //delete the project from db
  const QString sql = u"DELETE FROM %1.qgis_projects WHERE name=%2"_s
                        .arg( QgsPostgresConn::quotedIdentifier( schemaName ) )
                        .arg( QgsPostgresConn::quotedValue( projectName ) );

  QgsPostgresResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    return false;
  }

  return true;
}

bool QgsPostgresUtils::projectsTableExists( QgsPostgresConn *conn, const QString &schemaName )
{
  const QString tableName( "qgis_projects" );
  const QString sql( u"SELECT COUNT(*) FROM information_schema.tables WHERE table_name=%1 and table_schema=%2"_s
                       .arg( QgsPostgresConn::quotedValue( tableName ), QgsPostgresConn::quotedValue( schemaName ) )
  );
  QgsPostgresResult res( conn->PQexec( sql ) );

  if ( !res.result() )
  {
    return false;
  }

  return res.PQgetvalue( 0, 0 ).toInt() > 0;
}

bool QgsPostgresUtils::copyProjectToSchema( QgsPostgresConn *conn, const QString &originalSchema, const QString &projectName, const QString &targetSchema )
{
  //copy from one schema to another
  const QString sql = u"INSERT INTO %1.qgis_projects SELECT * FROM %2.qgis_projects WHERE name=%3;"_s
                        .arg( QgsPostgresConn::quotedIdentifier( targetSchema ) )
                        .arg( QgsPostgresConn::quotedIdentifier( originalSchema ) )
                        .arg( QgsPostgresConn::quotedValue( projectName ) );

  QgsPostgresResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    return false;
  }

  return true;
}

bool QgsPostgresUtils::moveProjectToSchema( QgsPostgresConn *conn, const QString &originalSchema, const QString &projectName, const QString &targetSchema )
{
  conn->begin();

  if ( !QgsPostgresUtils::copyProjectToSchema( conn, originalSchema, projectName, targetSchema ) )
  {
    return false;
  }

  if ( !QgsPostgresUtils::deleteProjectFromSchema( conn, projectName, originalSchema ) )
  {
    return false;
  }

  conn->commit();
  return true;
}

QString QgsPostgresUtils::variantMapToHtml( const QVariantMap &variantMap, const QString &title )
{
  QString result;
  if ( !title.isEmpty() )
  {
    result += u"<tr><td class=\"highlight\">%1</td><td></td></tr>"_s.arg( title );
  }
  for ( auto it = variantMap.constBegin(); it != variantMap.constEnd(); ++it )
  {
    const QVariantMap childMap = it.value().toMap();
    const QVariantList childList = it.value().toList();
    if ( !childList.isEmpty() )
    {
      result += u"<tr><td class=\"highlight\">%1</td><td><ul>"_s.arg( it.key() );
      for ( const QVariant &v : childList )
      {
        const QVariantMap grandChildMap = v.toMap();
        if ( !grandChildMap.isEmpty() )
        {
          result += u"<li><table>%1</table></li>"_s.arg( variantMapToHtml( grandChildMap ) );
        }
        else
        {
          result += u"<li>%1</li>"_s.arg( QgsStringUtils::insertLinks( v.toString() ) );
        }
      }
      result += "</ul></td></tr>"_L1;
    }
    else if ( !childMap.isEmpty() )
    {
      result += u"<tr><td class=\"highlight\">%1</td><td><table>%2</table></td></tr>"_s.arg( it.key(), variantMapToHtml( childMap ) );
    }
    else
    {
      result += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>"_s.arg( it.key(), QgsStringUtils::insertLinks( it.value().toString() ) );
    }
  }
  return result;
}

bool QgsPostgresUtils::setProjectComment( QgsPostgresConn *conn, const QString &projectName, const QString &schemaName, const QString &comment )
{
  const QString sql = QStringLiteral( "ALTER TABLE %1.qgis_projects ADD COLUMN IF NOT EXISTS comment TEXT DEFAULT '';"
                                      "UPDATE %1.qgis_projects SET comment = %3 WHERE name = %2" )
                        .arg( QgsPostgresConn::quotedIdentifier( schemaName ), QgsPostgresConn::quotedValue( projectName ), QgsPostgresConn::quotedValue( comment ) );

  QgsPostgresResult res( conn->PQexec( sql ) );
  return res.PQresultStatus() == PGRES_COMMAND_OK;
}

QString QgsPostgresUtils::projectComment( QgsPostgresConn *conn, const QString &schemaName, const QString &projectName )
{
  const QString sql = u"SELECT comment FROM %1.qgis_projects WHERE name = %2"_s
                        .arg( QgsPostgresConn::quotedIdentifier( schemaName ), QgsPostgresConn::quotedValue( projectName ) );

  QgsPostgresResult res( conn->PQexec( sql ) );
  if ( res.PQresultStatus() != PGRES_TUPLES_OK )
  {
    return QString();
  }

  return res.PQgetvalue( 0, 0 );
}

bool QgsPostgresUtils::addCommentColumnToProjectsTable( QgsPostgresConn *conn, const QString &schemaName )
{
  const QString sqlAddColumn = u"ALTER TABLE %1.qgis_projects ADD COLUMN IF NOT EXISTS comment TEXT DEFAULT ''"_s
                                 .arg( QgsPostgresConn::quotedIdentifier( schemaName ) );

  QgsPostgresResult resAddColumn( conn->PQexec( sqlAddColumn ) );
  return resAddColumn.PQresultStatus() == PGRES_COMMAND_OK;
}
