/***************************************************************************
  qgsoracleconn.cpp  -  connection class to Oracle
                             -------------------
    begin                : August 2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoracleconn.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgsmessagelog.h"
#include "qgscredentials.h"
#include "qgsfields.h"
#include "qgsoracletablemodel.h"
#include "qgssettings.h"

#include <QSqlError>

QMap<QString, QgsOracleConn *> QgsOracleConn::sConnections;
int QgsOracleConn::snConnections = 0;
const int QgsOracleConn::sGeomTypeSelectLimit = 100;
QMap<QString, QDateTime> QgsOracleConn::sBrokenConnections;

QgsOracleConn *QgsOracleConn::connectDb( const QgsDataSourceUri &uri )
{
  QString conninfo = toPoolName( uri );
  if ( sConnections.contains( conninfo ) )
  {
    QgsDebugMsg( QString( "Using cached connection for %1" ).arg( conninfo ) );
    sConnections[conninfo]->mRef++;
    return sConnections[conninfo];
  }

  QgsOracleConn *conn = new QgsOracleConn( uri );

  if ( conn->mRef == 0 )
  {
    delete conn;
    return 0;
  }

  sConnections.insert( conninfo, conn );

  return conn;
}

QgsOracleConn::QgsOracleConn( QgsDataSourceUri uri )
  : mRef( 1 )
  , mCurrentUser( QString() )
  , mHasSpatial( -1 )
{
  QgsDebugMsg( QString( "New Oracle connection for " ) + uri.connectionInfo() );

  QString database = databaseName( uri.database(), uri.host(), uri.port() );
  QgsDebugMsg( QString( "New Oracle database " ) + database );

  mDatabase = QSqlDatabase::addDatabase( "QOCISPATIAL", QString( "oracle%1" ).arg( snConnections++ ) );
  mDatabase.setDatabaseName( database );
  QString options = uri.hasParam( "dboptions" ) ? uri.param( "dboptions" ) : "OCI_ATTR_PREFETCH_ROWS=1000";
  QString workspace = uri.hasParam( "dbworkspace" ) ? uri.param( "dbworkspace" ) : QString();
  mDatabase.setConnectOptions( options );
  mDatabase.setUserName( uri.username() );
  mDatabase.setPassword( uri.password() );

  QString username = uri.username();
  QString password = uri.password();

  QString realm( database );
  if ( !username.isEmpty() )
    realm.prepend( username + "@" );

  if ( sBrokenConnections.contains( realm ) )
  {
    QDateTime now( QDateTime::currentDateTime() );
    QDateTime since( sBrokenConnections[ realm ] );
    QgsDebugMsg( QString( "Broken since %1 [%2s ago]" ).arg( since.toString( Qt::ISODate ) ).arg( since.secsTo( now ) ) );

    if ( since.secsTo( now ) < 30 )
    {
      QgsMessageLog::logMessage( tr( "Connection failed %1s ago - skipping retry" ).arg( since.secsTo( now ) ), tr( "Oracle" ) );
      mRef = 0;
      return;
    }
  }

  QgsDebugMsg( QString( "Connecting with options: " ) + options );
  if ( !mDatabase.open() )
  {
    QgsCredentials::instance()->lock();

    while ( !mDatabase.open() )
    {
      bool ok = QgsCredentials::instance()->get( realm, username, password, mDatabase.lastError().text() );
      if ( !ok )
      {
        QDateTime now( QDateTime::currentDateTime() );
        QgsDebugMsg( QString( "get failed: %1 <= %2" ).arg( realm ).arg( now.toString( Qt::ISODate ) ) );
        sBrokenConnections.insert( realm, now );
        break;
      }

      sBrokenConnections.remove( realm );

      if ( !username.isEmpty() )
      {
        uri.setUsername( username );
        realm = username + "@" + database;
      }

      if ( !password.isEmpty() )
        uri.setPassword( password );

      QgsDebugMsg( "Connecting to " + database );
      mDatabase.setUserName( username );
      mDatabase.setPassword( password );
    }

    if ( mDatabase.isOpen() )
      QgsCredentials::instance()->put( realm, username, password );

    QgsCredentials::instance()->unlock();
  }

  if ( !mDatabase.isOpen() )
  {
    mDatabase.close();
    QgsMessageLog::logMessage( tr( "Connection to database failed" ), tr( "Oracle" ) );
    mRef = 0;
    return;
  }

  if ( !workspace.isNull() )
  {
    QSqlQuery qry( mDatabase );

    if ( !qry.prepare( QString( "BEGIN\nDBMS_WM.GotoWorkspace(?);\nEND;" ) ) || !( qry.addBindValue( workspace ), qry.exec() ) )
    {
      mDatabase.close();
      QgsMessageLog::logMessage( tr( "Could not switch to workspace %1 [%2]" ).arg( workspace, qry.lastError().databaseText() ), tr( "Oracle" ) );
      mRef = 0;
      return;
    }
  }
}

QgsOracleConn::~QgsOracleConn()
{
  Q_ASSERT( mRef == 0 );
  if ( mDatabase.isOpen() )
    mDatabase.close();
}

QString QgsOracleConn::toPoolName( const QgsDataSourceUri &uri )
{
  QString conninfo = uri.connectionInfo();
  if ( uri.hasParam( "dbworkspace" ) )
    conninfo += " dbworkspace=" + uri.param( "dbworkspace" );
  return conninfo;
}

QString QgsOracleConn::connInfo()
{
  return sConnections.key( this, QString() );
}

void QgsOracleConn::disconnect()
{
  if ( --mRef > 0 )
    return;

  QString key = sConnections.key( this, QString() );

  if ( !key.isNull() )
  {
    sConnections.remove( key );
  }
  else
  {
    QgsDebugMsg( "Connection not found" );
  }

  deleteLater();
}

bool QgsOracleConn::exec( QSqlQuery &qry, QString sql, const QVariantList &params )
{
  QgsDebugMsgLevel( QString( "SQL: %1" ).arg( sql ), 4 );

  bool res = qry.prepare( sql );
  if ( res )
  {
    for ( const auto &param : params )
    {
      QgsDebugMsgLevel( QString( " ARG: %1 [%2]" ).arg( param.toString() ).arg( param.typeName() ), 4 );
      qry.addBindValue( param );
    }

    res = qry.exec();
  }

  if ( !res )
  {
    QgsDebugMsg( QString( "SQL: %1\nERROR: %2" )
                 .arg( qry.lastQuery() )
                 .arg( qry.lastError().text() ) );
  }

  return res;
}

QStringList QgsOracleConn::pkCandidates( QString ownerName, QString viewName )
{
  QStringList cols;

  QSqlQuery qry( mDatabase );
  if ( !exec( qry, QString( "SELECT column_name FROM all_tab_columns WHERE owner=? AND table_name=? ORDER BY column_id" ),
              QVariantList() << ownerName << viewName ) )
  {
    QgsMessageLog::logMessage( tr( "SQL: %1 [owner: %2 table_name: %3]\nerror: %4\n" ).arg( qry.lastQuery(), qry.lastError().text(), ownerName, viewName ), tr( "Oracle" ) );
    return cols;
  }

  while ( qry.next() )
  {
    cols << qry.value( 0 ).toString();
  }

  qry.finish();

  return cols;
}

bool QgsOracleConn::tableInfo( bool geometryColumnsOnly, bool userTablesOnly, bool allowGeometrylessTables )
{
  QgsDebugMsg( "Entering." );

  mLayersSupported.clear();

  QString sql, delim;

  QString
  prefix( userTablesOnly ? "user" : "all" ),
          owner( userTablesOnly ? "user AS owner" : "c.owner" );

  sql = QString( "SELECT %1,c.table_name,c.column_name,%2,o.object_type AS type"
                 " FROM %3_%4 c"
                 " JOIN %3_objects o ON c.table_name=o.object_name AND o.object_type IN ('TABLE','VIEW','SYNONYM')%5%6" )
        .arg( owner )
        .arg( geometryColumnsOnly ? "c.srid" : "NULL AS srid" )
        .arg( prefix )
        .arg( geometryColumnsOnly ? "sdo_geom_metadata" : "tab_columns" )
        .arg( userTablesOnly ? "" : " AND c.owner=o.owner" )
        .arg( geometryColumnsOnly ? "" : " WHERE c.data_type='SDO_GEOMETRY'" );

  if ( allowGeometrylessTables )
  {
    sql += QString( " UNION SELECT %1,object_name,NULL AS column_name,NULL AS srid,object_type AS type"
                    " FROM %2_objects c WHERE c.object_type IN ('TABLE','VIEW','SYNONYM')" )
           .arg( owner ).arg( prefix );
  }

  // sql = "SELECT * FROM (" + sql + ")";
  // sql += " ORDER BY owner,isview,table_name,column_name";

  QSqlQuery qry( mDatabase );
  if ( !exec( qry, sql, QVariantList() ) )
  {
    QgsMessageLog::logMessage( tr( "Querying available tables failed.\nSQL: %1\nerror: %2\n" ).arg( qry.lastQuery() ).arg( qry.lastError().text() ), tr( "Oracle" ) );
    return false;
  }

  while ( qry.next() )
  {
    QgsOracleLayerProperty layerProperty;
    layerProperty.ownerName       = qry.value( 0 ).toString();
    layerProperty.tableName       = qry.value( 1 ).toString();
    layerProperty.geometryColName = qry.value( 2 ).toString();
    layerProperty.types           = QList<QgsWkbTypes::Type>() << ( qry.value( 2 ).isNull() ? QgsWkbTypes::NoGeometry : QgsWkbTypes::Unknown );
    layerProperty.srids           = QList<int>() << qry.value( 3 ).toInt();
    layerProperty.isView          = qry.value( 4 ) != "TABLE";
    layerProperty.pkCols.clear();

    mLayersSupported << layerProperty;
  }

  if ( mLayersSupported.size() == 0 )
  {
    QgsMessageLog::logMessage( tr( "Database connection was successful, but the accessible tables could not be determined." ), tr( "Oracle" ) );
  }

  return true;
}

bool QgsOracleConn::supportedLayers( QVector<QgsOracleLayerProperty> &layers, bool geometryTablesOnly, bool userTablesOnly, bool allowGeometrylessTables )
{
  // Get the list of supported tables
  if ( !tableInfo( geometryTablesOnly, userTablesOnly, allowGeometrylessTables ) )
  {
    QgsMessageLog::logMessage( tr( "Unable to get list of spatially enabled tables from the database" ), tr( "Oracle" ) );
    return false;
  }

  layers = mLayersSupported;

  QgsDebugMsg( "Exiting." );

  return true;
}

QString QgsOracleConn::quotedIdentifier( QString ident )
{
  ident.replace( '"', "\"\"" );
  ident = ident.prepend( "\"" ).append( "\"" );
  return ident;
}

QString QgsOracleConn::quotedValue( const QVariant &value, QVariant::Type type )
{
  if ( value.isNull() )
    return "NULL";

  if ( type == QVariant::Invalid )
    type = value.type();

  if ( value.canConvert( type ) )
  {
    switch ( type )
    {
      case QVariant::Int:
      case QVariant::LongLong:
      case QVariant::Double:
        return value.toString();

      case QVariant::DateTime:
      {
        QDateTime datetime( value.toDateTime() );
        if ( datetime.isValid() )
          return QString( "TO_DATE('%1','YYYY-MM-DD HH24:MI:SS')" ).arg( datetime.toString( "yyyy-MM-dd hh:mm:ss" ) );
        break;
      }

      case QVariant::Date:
      {
        QDate date( value.toDate() );
        if ( date.isValid() )
          return QString( "TO_DATE('%1','YYYY-MM-DD')" ).arg( date.toString( "yyyy-MM-dd" ) );
        break;
      }

      case QVariant::Time:
      {
        QDateTime datetime( value.toDateTime() );
        if ( datetime.isValid() )
          return QString( "TO_DATE('%1','HH24:MI:SS')" ).arg( datetime.toString( "hh:mm:ss" ) );
        break;
      }

      default:
        break;
    }
  }

  QString v = value.toString();
  v.replace( "'", "''" );
  v.replace( "\\\"", "\\\\\"" );
  return v.prepend( "'" ).append( "'" );
}

QString QgsOracleConn::fieldExpression( const QgsField &fld )
{
#if 0
  const QString &type = fld.typeName();
  if ( type == "money" )
  {
    return QString( "cash_out(%1)" ).arg( quotedIdentifier( fld.name() ) );
  }
  else if ( type.startsWith( "_" ) )
  {
    return QString( "array_out(%1)" ).arg( quotedIdentifier( fld.name() ) );
  }
  else if ( type == "bool" )
  {
    return QString( "boolout(%1)" ).arg( quotedIdentifier( fld.name() ) );
  }
  else if ( type == "geometry" )
  {
    return QString( "%1(%2)" )
           .arg( majorVersion() < 2 ? "asewkt" : "st_asewkt" )
           .arg( quotedIdentifier( fld.name() ) );
  }
  else if ( type == "geography" )
  {
    return QString( "st_astext(%1)" ).arg( quotedIdentifier( fld.name() ) );
  }
  else
  {
    return quotedIdentifier( fld.name() ) + "::text";
  }
#else
  return quotedIdentifier( fld.name() );
#endif
}

void QgsOracleConn::retrieveLayerTypes( QgsOracleLayerProperty &layerProperty, bool useEstimatedMetadata, bool onlyExistingTypes )
{
  QgsDebugMsgLevel( "entering: " + layerProperty.toString(), 3 );

  if ( layerProperty.isView )
  {
    layerProperty.pkCols = pkCandidates( layerProperty.ownerName, layerProperty.tableName );
    if ( layerProperty.pkCols.isEmpty() )
    {
      QgsMessageLog::logMessage( tr( "View %1.%2 doesn't have integer columns for use as keys." )
                                 .arg( layerProperty.ownerName ).arg( layerProperty.tableName ),
                                 tr( "Oracle" ) );
    }
  }

  if ( layerProperty.geometryColName.isEmpty() )
    return;

  QString table;
  QString where;

  if ( useEstimatedMetadata )
  {
    table = QString( "(SELECT %1 FROM %2.%3 WHERE %1 IS NOT NULL%4 AND rownum<=%5)" )
            .arg( quotedIdentifier( layerProperty.geometryColName ) )
            .arg( quotedIdentifier( layerProperty.ownerName ) )
            .arg( quotedIdentifier( layerProperty.tableName ) )
            .arg( layerProperty.sql.isEmpty() ? "" : QString( " AND (%1)" ).arg( layerProperty.sql ) )
            .arg( sGeomTypeSelectLimit );
  }
  else if ( !layerProperty.ownerName.isEmpty() )
  {
    table = QString( "%1.%2" )
            .arg( quotedIdentifier( layerProperty.ownerName ) )
            .arg( quotedIdentifier( layerProperty.tableName ) );
    where = layerProperty.sql;
  }
  else
  {
    table = quotedIdentifier( layerProperty.tableName );
    where = layerProperty.sql;
  }

  QgsWkbTypes::Type detectedType = layerProperty.types.value( 0, QgsWkbTypes::Unknown );
  int detectedSrid = layerProperty.srids.value( 0, -1 );

  Q_ASSERT( detectedType == QgsWkbTypes::Unknown || detectedSrid <= 0 );

  QSqlQuery qry( mDatabase );
  int idx = 0;
  QString sql = "SELECT DISTINCT ";
  if ( detectedType == QgsWkbTypes::Unknown )
  {
    sql += "t.%1.SDO_GTYPE";
    if ( detectedSrid <= 0 )
    {
      sql += ",";
      idx = 1;
    }
  }

  if ( detectedSrid <= 0 )
  {
    sql += "t.%1.SDO_SRID";
  }

  sql += " FROM %2 t WHERE NOT t.%1 IS NULL%3";

  if ( !exec( qry, sql
              .arg( quotedIdentifier( layerProperty.geometryColName ) )
              .arg( table )
              .arg( where.isEmpty() ? "" : QString( " AND (%1)" ).arg( where ) ), QVariantList() ) )
  {
    QgsMessageLog::logMessage( tr( "SQL: %1\nerror: %2\n" )
                               .arg( qry.lastQuery() )
                               .arg( qry.lastError().text() ),
                               tr( "Oracle" ) );
    return;
  }

  layerProperty.types.clear();
  layerProperty.srids.clear();

  QSet<int> srids;
  while ( qry.next() )
  {
    if ( detectedType == QgsWkbTypes::Unknown )
    {
      QgsWkbTypes::Type type = wkbTypeFromDatabase( qry.value( 0 ).toInt() );
      if ( type == QgsWkbTypes::Unknown )
      {
        QgsMessageLog::logMessage( tr( "Unsupported geometry type %1 in %2.%3.%4 ignored" )
                                   .arg( qry.value( 0 ).toInt() )
                                   .arg( layerProperty.ownerName ).arg( layerProperty.tableName ).arg( layerProperty.geometryColName ),
                                   tr( "Oracle" ) );
        continue;
      }
      QgsDebugMsg( QString( "add type %1" ).arg( type ) );
      layerProperty.types << type;
    }
    else
    {
      layerProperty.types << detectedType;
    }

    int srid = detectedSrid > 0 ? detectedSrid : ( qry.value( idx ).isNull() ? -1 : qry.value( idx ).toInt() );
    layerProperty.srids << srid;
    srids << srid;
  }

  qry.finish();

  if ( !onlyExistingTypes )
  {
    layerProperty.types << QgsWkbTypes::Unknown;
    layerProperty.srids << ( srids.size() == 1 ? *srids.constBegin() : 0 );
  }

  QgsDebugMsg( "leaving." );
}

QString QgsOracleConn::databaseTypeFilter( QString alias, QString geomCol, QgsWkbTypes::Type geomType )
{
  geomCol = quotedIdentifier( alias ) + "." + quotedIdentifier( geomCol );

  switch ( geomType )
  {
    case QgsWkbTypes::Point:
    case QgsWkbTypes::Point25D:
    case QgsWkbTypes::MultiPoint:
    case QgsWkbTypes::MultiPoint25D:
      return QString( "mod(%1.sdo_gtype,100) IN (1,5)" ).arg( geomCol );
    case QgsWkbTypes::LineString:
    case QgsWkbTypes::LineString25D:
    case QgsWkbTypes::MultiLineString:
    case QgsWkbTypes::MultiLineString25D:
      return QString( "mod(%1.sdo_gtype,100) IN (2,6)" ).arg( geomCol );
    case QgsWkbTypes::Polygon:
    case QgsWkbTypes::Polygon25D:
    case QgsWkbTypes::MultiPolygon:
    case QgsWkbTypes::MultiPolygon25D:
      return QString( "mod(%1.sdo_gtype,100) IN (3,7)" ).arg( geomCol );
    case QgsWkbTypes::NoGeometry:
      return QString( "%1 IS NULL" ).arg( geomCol );
    case QgsWkbTypes::Unknown:
      Q_ASSERT( !"unknown geometry unexpected" );
      return QString();
    default:
      break;
  }

  Q_ASSERT( !"unexpected geomType" );
  return QString();
}

QgsWkbTypes::Type QgsOracleConn::wkbTypeFromDatabase( int gtype )
{
  QgsDebugMsg( QString( "entering %1" ).arg( gtype ) );
  int t = gtype % 100;

  if ( t == 0 )
    return QgsWkbTypes::Unknown;

  int d = gtype / 1000;
  if ( d == 2 )
  {
    switch ( t )
    {
      case 1:
        return QgsWkbTypes::Point;
      case 2:
        return QgsWkbTypes::LineString;
      case 3:
        return QgsWkbTypes::Polygon;
      case 4:
        QgsDebugMsg( QString( "geometry collection type %1 unsupported" ).arg( gtype ) );
        return QgsWkbTypes::Unknown;
      case 5:
        return QgsWkbTypes::MultiPoint;
      case 6:
        return QgsWkbTypes::MultiLineString;
      case 7:
        return QgsWkbTypes::MultiPolygon;
      default:
        QgsDebugMsg( QString( "gtype %1 unsupported" ).arg( gtype ) );
        return QgsWkbTypes::Unknown;
    }
  }
  else if ( d == 3 )
  {
    switch ( t )
    {
      case 1:
        return QgsWkbTypes::Point25D;
      case 2:
        return QgsWkbTypes::LineString25D;
      case 3:
        return QgsWkbTypes::Polygon25D;
      case 4:
        QgsDebugMsg( QString( "geometry collection type %1 unsupported" ).arg( gtype ) );
        return QgsWkbTypes::Unknown;
      case 5:
        return QgsWkbTypes::MultiPoint25D;
      case 6:
        return QgsWkbTypes::MultiLineString25D;
      case 7:
        return QgsWkbTypes::MultiPolygon25D;
      default:
        QgsDebugMsg( QString( "gtype %1 unsupported" ).arg( gtype ) );
        return QgsWkbTypes::Unknown;
    }
  }
  else
  {
    QgsDebugMsg( QString( "dimension of gtype %1 unsupported" ).arg( gtype ) );
    return QgsWkbTypes::Unknown;
  }
}

QString QgsOracleConn::displayStringForWkbType( QgsWkbTypes::Type type )
{
  switch ( type )
  {
    case QgsWkbTypes::Point:
    case QgsWkbTypes::Point25D:
      return tr( "Point" );

    case QgsWkbTypes::MultiPoint:
    case QgsWkbTypes::MultiPoint25D:
      return tr( "Multipoint" );

    case QgsWkbTypes::LineString:
    case QgsWkbTypes::LineString25D:
      return tr( "Line" );

    case QgsWkbTypes::MultiLineString:
    case QgsWkbTypes::MultiLineString25D:
      return tr( "Multiline" );

    case QgsWkbTypes::Polygon:
    case QgsWkbTypes::Polygon25D:
      return tr( "Polygon" );

    case QgsWkbTypes::MultiPolygon:
    case QgsWkbTypes::MultiPolygon25D:
      return tr( "Multipolygon" );

    case QgsWkbTypes::NoGeometry:
      return tr( "No Geometry" );

    case QgsWkbTypes::Unknown:
      return tr( "Unknown Geometry" );

    default:
      break;
  }

  Q_ASSERT( !"unexpected wkbType" );
  return QString();
}

QgsWkbTypes::Type QgsOracleConn::wkbTypeFromGeomType( QgsWkbTypes::GeometryType geomType )
{
  switch ( geomType )
  {
    case QgsWkbTypes::PointGeometry:
      return QgsWkbTypes::Point;
    case QgsWkbTypes::LineGeometry:
      return QgsWkbTypes::LineString;
    case QgsWkbTypes::PolygonGeometry:
      return QgsWkbTypes::Polygon;
    case QgsWkbTypes::NullGeometry:
      return QgsWkbTypes::NoGeometry;
    case QgsWkbTypes::UnknownGeometry:
      return QgsWkbTypes::Unknown;
  }

  Q_ASSERT( !"unexpected geomType" );
  return QgsWkbTypes::Unknown;
}

QStringList QgsOracleConn::connectionList()
{
  QgsSettings settings;
  settings.beginGroup( "/Oracle/connections" );
  return settings.childGroups();
}

void QgsOracleConn::deleteConnection( QString connName )
{
  QgsSettings settings;

  QString key = "/Oracle/connections/" + connName;
  settings.remove( key + "/host" );
  settings.remove( key + "/port" );
  settings.remove( key + "/database" );
  settings.remove( key + "/username" );
  settings.remove( key + "/password" );
  settings.remove( key + "/userTablesOnly" );
  settings.remove( key + "/geometryColumnsOnly" );
  settings.remove( key + "/allowGeometrylessTables" );
  settings.remove( key + "/estimatedMetadata" );
  settings.remove( key + "/onlyExistingTypes" );
  settings.remove( key + "/includeGeoAttributes" );
  settings.remove( key + "/saveUsername" );
  settings.remove( key + "/savePassword" );
  settings.remove( key + "/save" );
  settings.remove( key );
}

QString QgsOracleConn::selectedConnection()
{
  QgsSettings settings;
  return settings.value( "/Oracle/connections/selected" ).toString();
}

void QgsOracleConn::setSelectedConnection( QString name )
{
  QgsSettings settings;
  return settings.setValue( "/Oracle/connections/selected", name );
}

QgsDataSourceUri QgsOracleConn::connUri( QString connName )
{
  QgsDebugMsgLevel( "theConnName = " + connName, 3 );

  QgsSettings settings;

  QString key = "/Oracle/connections/" + connName;

  QString database = settings.value( key + "/database" ).toString();

  QString host = settings.value( key + "/host" ).toString();
  QString port = settings.value( key + "/port" ).toString();
  if ( port.length() == 0 )
  {
    port = "1521";
  }

  bool useEstimatedMetadata = settings.value( key + "/estimatedMetadata", false ).toBool();

  QString username;
  QString password;
  if ( settings.value( key + "/saveUsername" ).toString() == "true" )
  {
    username = settings.value( key + "/username" ).toString();
  }

  if ( settings.value( key + "/savePassword" ).toString() == "true" )
  {
    password = settings.value( key + "/password" ).toString();
  }

  QgsDataSourceUri uri;
  uri.setConnection( host, port, database, username, password );
  uri.setUseEstimatedMetadata( useEstimatedMetadata );
  if ( !settings.value( key + "/dboptions" ).toString().isEmpty() )
  {
    uri.setParam( "dboptions", settings.value( key + "/dboptions" ).toString() );
  }
  if ( !settings.value( key + "/dbworkspace" ).toString().isEmpty() )
  {
    uri.setParam( "dbworkspace", settings.value( key + "/dbworkspace" ).toString() );
  }

  return uri;
}

bool QgsOracleConn::userTablesOnly( QString connName )
{
  QgsSettings settings;
  return settings.value( "/Oracle/connections/" + connName + "/userTablesOnly", false ).toBool();
}

bool QgsOracleConn::geometryColumnsOnly( QString connName )
{
  QgsSettings settings;
  return settings.value( "/Oracle/connections/" + connName + "/geometryColumnsOnly", true ).toBool();
}

bool QgsOracleConn::allowGeometrylessTables( QString connName )
{
  QgsSettings settings;
  return settings.value( "/Oracle/connections/" + connName + "/allowGeometrylessTables", false ).toBool();
}

bool QgsOracleConn::estimatedMetadata( QString connName )
{
  QgsSettings settings;
  return settings.value( "/Oracle/connections/" + connName + "/estimatedMetadata", false ).toBool();
}

bool QgsOracleConn::onlyExistingTypes( QString connName )
{
  QgsSettings settings;
  return settings.value( "/Oracle/connections/" + connName + "/onlyExistingTypes", false ).toBool();
}

QString QgsOracleConn::databaseName( QString database, QString host, QString port )
{
  QString db;

  if ( !host.isEmpty() )
  {
    db += host;

    if ( !port.isEmpty() && port != "1521" )
    {
      db += QString( ":%1" ).arg( port );
    }

    if ( !database.isEmpty() )
    {
      db += "/" + database;
    }
  }
  else if ( !database.isEmpty() )
  {
    db = database;
  }

  return db;
}

bool QgsOracleConn::hasSpatial()
{
  if ( mHasSpatial == -1 )
  {
    QSqlQuery qry( mDatabase );
    mHasSpatial = exec( qry, "SELECT 1 FROM v$option WHERE parameter='Spatial' AND value='TRUE'", QVariantList() ) && qry.next();
  }

  return mHasSpatial;
}

QString QgsOracleConn::currentUser()
{
  if ( mCurrentUser.isNull() )
  {
    QSqlQuery qry( mDatabase );
    if ( exec( qry, "SELECT user FROM dual", QVariantList() ) && qry.next() )
    {
      mCurrentUser = qry.value( 0 ).toString();
    }
  }

  return mCurrentUser;
}

// vim: sw=2 :
