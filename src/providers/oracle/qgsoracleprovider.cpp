/***************************************************************************
  qgsoracleprovider.cpp  -  QGIS data provider for Oracle layers
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

#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgscurve.h"
#include "qgscompoundcurve.h"
#include "qgspolygon.h"
#include "qgsmultipolygon.h"
#include "qgsmultisurface.h"
#include "qgsmessageoutput.h"
#include "qgsmessagelog.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectorlayerexporter.h"
#include "qgslogger.h"
#include "qgsdbquerylog.h"

#include "qgsoracleprovider.h"
#include "qgsoracletablemodel.h"
#include "qgsoracledataitems.h"
#include "qgsoraclefeatureiterator.h"
#include "qgsoracleconnpool.h"
#include "qgsoracletransaction.h"
#include "qgsoracleproviderconnection.h"

#ifdef HAVE_GUI
#include "qgsoraclesourceselect.h"
#include "qgssourceselectprovider.h"
#endif

#include <QSqlRecord>
#include <QSqlField>
#include <QUuid>

#include "ocispatial/wkbptr.h"


#define LoggedExecStatic(query, sql, args, uri ) execLoggedStatic( query, sql, args, uri, QStringLiteral( "QgsOracleProvider" ), QString(QString( __FILE__ ).mid( sOracleConQueryLogFilePrefixLength ) + ':' + QString::number( __LINE__ ) + " (" + __FUNCTION__ + ")") )


const QString ORACLE_KEY = "oracle";
const QString ORACLE_DESCRIPTION = "Oracle data provider";

QgsOracleProvider::QgsOracleProvider( QString const &uri, const ProviderOptions &options,
                                      QgsDataProvider::ReadFlags flags )
  : QgsVectorDataProvider( uri, options, flags )
  , mValid( false )
  , mIsQuery( false )
  , mPrimaryKeyType( PktUnknown )
  , mFeaturesCounted( -1 )
  , mDetectedGeomType( QgsWkbTypes::Unknown )
  , mRequestedGeomType( QgsWkbTypes::Unknown )
  , mHasSpatialIndex( false )
  , mSpatialIndexName( QString() )
  , mOracleVersion( -1 )
  , mShared( new QgsOracleSharedData )
{
  static int geomMetaType = -1;
  if ( geomMetaType < 0 )
    geomMetaType = qRegisterMetaType<QOCISpatialGeometry>();

  QgsDebugMsgLevel( QStringLiteral( "URI: %1 " ).arg( uri ), 2 );

  mUri = QgsDataSourceUri( uri );

  // populate members from the uri structure
  mOwnerName = mUri.schema();
  mTableName = mUri.table();
  mGeometryColumn = mUri.geometryColumn();
  mSqlWhereClause = mUri.sql();
  mSrid = mUri.srid().toInt();
  mRequestedGeomType = mUri.wkbType();
  mUseEstimatedMetadata = mUri.useEstimatedMetadata();
  if ( mReadFlags & QgsDataProvider::FlagTrustDataSource )
  {
    mUseEstimatedMetadata = true;
  }
  mIncludeGeoAttributes = mUri.hasParam( "includegeoattributes" ) ? mUri.param( "includegeoattributes" ) == "true" : false;

  QgsOracleConn *conn = connectionRO();
  if ( !conn )
  {
    return;
  }

  if ( mOwnerName.isEmpty() && mTableName.startsWith( "(" ) && mTableName.endsWith( ")" ) )
  {
    mIsQuery = true;
    mQuery = mTableName;
    mTableName = "";
  }
  else
  {
    mIsQuery = false;

    if ( mOwnerName.isEmpty() )
    {
      mOwnerName = conn->currentUser();
    }

    if ( !mOwnerName.isEmpty() )
    {
      mQuery += quotedIdentifier( mOwnerName ) + ".";
    }

    if ( !mTableName.isEmpty() )
    {
      mQuery += quotedIdentifier( mTableName );
    }
  }

  QgsDebugMsgLevel( QStringLiteral( "Connection info is %1" ).arg( mUri.connectionInfo( false ) ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Geometry column is: %1" ).arg( mGeometryColumn ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Owner is: %1" ).arg( mOwnerName ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Table name is: %1" ).arg( mTableName ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Query is: %1" ).arg( mQuery ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Where clause is: %1" ).arg( mSqlWhereClause ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "SRID is: %1" ).arg( mSrid ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Using estimated metadata: %1" ).arg( mUseEstimatedMetadata ? "yes" : "no" ), 2 );

  // no table/query passed, the provider could be used to get tables
  if ( mQuery.isEmpty() )
  {
    return;
  }

  if ( !hasSufficientPermsAndCapabilities() )
  {
    return;
  }

  // get geometry details
  if ( !getGeometryDetails() ) // gets srid, geometry and data type
  {
    // the table is not a geometry table
    mValid = false;
    disconnectDb();
    return;
  }

  mLayerExtent.setMinimal();

  mOracleVersion = connectionRO()->version();
  if ( mOracleVersion < 0 )
  {
    mValid = false;
    disconnectDb();
    return;
  }

  // set the primary key
  if ( !determinePrimaryKey() )
  {
    mValid = false;
    disconnectDb();
    return;
  }

  //fill type names into sets
  setNativeTypes( connectionRO()->nativeTypes() );

  QString key;
  switch ( mPrimaryKeyType )
  {
    case PktRowId:
      key = "ROWID";
      break;

    case PktInt:
    case PktFidMap:
    {
      Q_ASSERT( mPrimaryKeyType != PktInt || mPrimaryKeyAttrs.size() == 1 );

      QString delim;
      const auto constMPrimaryKeyAttrs = mPrimaryKeyAttrs;
      for ( int idx : constMPrimaryKeyAttrs )
      {
        Q_ASSERT( idx >= 0 && idx < mAttributeFields.size() );
        key += delim + mAttributeFields.at( idx ).name();
        delim = ",";
      }
    }
    break;
    case PktUnknown:
      mValid = false;
      break;
  }

  if ( mValid )
  {
    mUri.setKeyColumn( key );
    setDataSourceUri( mUri.uri( false ) );
  }
  else
  {
    disconnectDb();
  }
}

QgsOracleProvider::~QgsOracleProvider()
{
  disconnectDb();
}

QString QgsOracleProvider::getWorkspace() const
{
  return mUri.param( "dbworkspace" );
}

Qgis::VectorLayerTypeFlags QgsOracleProvider::vectorLayerTypeFlags() const
{
  Qgis::VectorLayerTypeFlags flags;
  if ( mValid && mIsQuery )
  {
    flags.setFlag( Qgis::VectorLayerTypeFlag::SqlQuery );
  }
  return flags;
}

void QgsOracleProvider::handlePostCloneOperations( QgsVectorDataProvider *source )
{
  mShared = qobject_cast<QgsOracleProvider *>( source )->mShared;
}

void QgsOracleProvider::setWorkspace( const QString &workspace )
{
  QgsDataSourceUri prevUri( mUri );

  disconnectDb();

  if ( workspace.isEmpty() )
    mUri.removeParam( "dbworkspace" );
  else
    mUri.setParam( "dbworkspace", workspace );

  QgsOracleConn *conn = connectionRO();
  if ( !conn )
  {
    mUri = prevUri;
    QgsDebugMsgLevel( QStringLiteral( "restoring previous uri:%1" ).arg( mUri.uri( false ) ), 2 );
    conn = connectionRO();
  }
  else
  {
    setDataSourceUri( mUri.uri( false ) );
  }
}

QgsAbstractFeatureSource *QgsOracleProvider::featureSource() const
{
  return new QgsOracleFeatureSource( this );
}

void QgsOracleProvider::disconnectDb()
{
  QgsOracleConn *conn = QgsOracleConn::connectDb( mUri, false );
  if ( conn )
    conn->disconnect();
}

QgsOracleConn *QgsOracleProvider::connectionRW()
{
  return mTransaction ? mTransaction->connection() : QgsOracleConn::connectDb( mUri, false );
}

QgsOracleConn *QgsOracleProvider::connectionRO() const
{
  return mTransaction ? mTransaction->connection() : QgsOracleConn::connectDb( mUri, false );
}

bool QgsOracleProvider::execLoggedStatic( QSqlQuery &qry, const QString &sql, const QVariantList &args, const QString &uri, const QString &originatorClass, const QString &queryOrigin )
{
  QgsDatabaseQueryLogWrapper logWrapper { sql, uri, QStringLiteral( "oracle" ), originatorClass, queryOrigin };

  qry.setForwardOnly( true );

  bool res = qry.prepare( sql );

  if ( res )
  {
    for ( const auto &arg : args )
    {
      QgsDebugMsgLevel( QStringLiteral( " ARG: %1 [%2]" ).arg( arg.toString() ).arg( arg.typeName() ), 4 );
      qry.addBindValue( arg );
    }
    res = qry.exec();
  }

  if ( !res )
  {
    QgsDebugMsg( QStringLiteral( "SQL: %1\nERROR: %2" )
                 .arg( qry.lastQuery() )
                 .arg( qry.lastError().text() ) );
  }

  logWrapper.setQuery( QgsOracleConn::getLastExecutedQuery( qry ) );
  logWrapper.setError( qry.lastError().text() );
  // ORACLE does not support size so this will always be -1
  // we leave it here in case this changes in the future
  if ( qry.isSelect() )
  {
    logWrapper.setFetchedRows( qry.size() );
  }
  else
  {
    logWrapper.setFetchedRows( qry.numRowsAffected() );
  }
  return res;
}

void QgsOracleProvider::setTransaction( QgsTransaction *transaction )
{
  // static_cast since layers cannot be added to a transaction of a non-matching provider
  mTransaction = static_cast<QgsOracleTransaction *>( transaction );
}

QgsTransaction *QgsOracleProvider::transaction() const
{
  return mTransaction;
}

QString QgsOracleProvider::storageType() const
{
  return "Oracle database with locator/spatial extension";
}

QString QgsOracleProvider::pkParamWhereClause() const
{
  QgsOracleConn *conn = connectionRO();
  QString whereClause;

  switch ( mPrimaryKeyType )
  {
    case PktInt:
    case PktFidMap:
    {
      Q_ASSERT( mPrimaryKeyAttrs.size() >= 1 );

      QString delim = "";
      for ( int i = 0; i < mPrimaryKeyAttrs.size(); i++ )
      {
        int idx = mPrimaryKeyAttrs[i];
        QgsField fld = field( idx );

        whereClause += delim + QString( "%1=?" ).arg( conn->fieldExpression( fld ) );
        delim = " AND ";
      }
    }
    break;

    case PktRowId:
      return "ROWID=?";
      break;

    case PktUnknown:
      Q_ASSERT( !"FAILURE: Primary key unknown" );
      whereClause = "NULL IS NOT NULL";
      break;
  }

  if ( !mSqlWhereClause.isEmpty() )
  {
    if ( !whereClause.isEmpty() )
      whereClause += " AND ";

    whereClause += "(" + mSqlWhereClause + ")";
  }

  return whereClause;
}

void QgsOracleProvider::appendPkParams( QgsFeatureId fid, QSqlQuery &qry ) const
{
  switch ( mPrimaryKeyType )
  {
    case PktInt:
      QgsDebugMsgLevel( QStringLiteral( "addBindValue pk %1" ).arg( FID_TO_STRING( fid ) ), 4 );
      qry.addBindValue( FID_TO_STRING( fid ) );
      break;

    case PktRowId:
    case PktFidMap:
    {
      QVariant pkValsVariant = mShared->lookupKey( fid );
      if ( !pkValsVariant.isNull() )
      {
        const auto constToList = pkValsVariant.toList();
        for ( const QVariant &v : constToList )
        {
          QgsDebugMsgLevel( QStringLiteral( "addBindValue pk %1" ).arg( FID_TO_STRING( fid ) ), 4 );
          qry.addBindValue( v );
        }
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "key values for fid %1 not found." ).arg( fid ), 2 );
        for ( int i = 0; i < mPrimaryKeyAttrs.size(); i++ )
        {
          QgsDebugMsgLevel( QStringLiteral( "addBindValue pk NULL" ).arg( fid ), 4 );
          qry.addBindValue( QVariant() );
        }
      }
      break;
    }
    break;

    case PktUnknown:
      QgsDebugMsg( QStringLiteral( "Unknown key type" ) );
      break;
  }
}


QString QgsOracleUtils::whereClause( QgsFeatureId featureId, const QgsFields &fields, QgsOraclePrimaryKeyType primaryKeyType, const QList<int> &primaryKeyAttrs, std::shared_ptr<QgsOracleSharedData> sharedData, QVariantList &args )
{
  QString whereClause;

  switch ( primaryKeyType )
  {
    case PktInt:
      Q_ASSERT( primaryKeyAttrs.size() == 1 );
      whereClause = QString( "%1=?" ).arg( QgsOracleConn::quotedIdentifier( fields.at( primaryKeyAttrs[0] ).name() ) );
      args << featureId;
      break;

    case PktRowId:
    case PktFidMap:
    {
      QVariantList pkVals = sharedData->lookupKey( featureId );
      if ( !pkVals.isEmpty() )
      {
        if ( primaryKeyType == PktFidMap )
        {
          Q_ASSERT( pkVals.size() == primaryKeyAttrs.size() );

          QString delim = "";
          for ( int i = 0; i < primaryKeyAttrs.size(); i++ )
          {
            int idx = primaryKeyAttrs[i];
            QgsField fld = fields.at( idx );

            whereClause += delim + QString( "%1=?" ).arg( QgsOracleConn::fieldExpression( fld ) );
            args << pkVals[i];
            delim = " AND ";
          }
        }
        else
        {
          whereClause += QString( "ROWID=?" );
          args << pkVals[0];
        }
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "FAILURE: Key values for feature %1 not found." ).arg( featureId ) );
        whereClause = "NULL IS NOT NULL";
      }
    }
    break;

    case PktUnknown:
      Q_ASSERT( !"FAILURE: Primary key unknown" );
      whereClause = "NULL IS NOT NULL";
      break;
  }

  return whereClause;
}

QString QgsOracleUtils::whereClause( QgsFeatureIds featureIds, const QgsFields &fields, QgsOraclePrimaryKeyType primaryKeyType, const QList<int> &primaryKeyAttrs, std::shared_ptr<QgsOracleSharedData> sharedData, QVariantList &args )
{
  QStringList whereClauses;
  const auto constFeatureIds = featureIds;
  for ( const QgsFeatureId featureId : constFeatureIds )
  {
    whereClauses << whereClause( featureId, fields, primaryKeyType, primaryKeyAttrs, sharedData, args );
  }
  return whereClauses.isEmpty() ? "" : whereClauses.join( " OR " ).prepend( "(" ).append( ")" );
}

QString QgsOracleUtils::andWhereClauses( const QString &c1, const QString &c2 )
{
  if ( c1.isEmpty() )
    return c2;
  if ( c2.isEmpty() )
    return c1;

  return QString( "(%1) AND (%2)" ).arg( c1 ).arg( c2 );
}

QString QgsOracleProvider::whereClause( QgsFeatureId featureId, QVariantList &args ) const
{
  return QgsOracleUtils::whereClause( featureId, mAttributeFields, mPrimaryKeyType, mPrimaryKeyAttrs, mShared, args );
}

void QgsOracleProvider::setExtent( QgsRectangle &newExtent )
{
  mLayerExtent.setXMaximum( newExtent.xMaximum() );
  mLayerExtent.setXMinimum( newExtent.xMinimum() );
  mLayerExtent.setYMaximum( newExtent.yMaximum() );
  mLayerExtent.setYMinimum( newExtent.yMinimum() );
}

/**
 * Returns the feature type
 */
QgsWkbTypes::Type QgsOracleProvider::wkbType() const
{
  return mRequestedGeomType != QgsWkbTypes::Unknown ? mRequestedGeomType : mDetectedGeomType;
}

QgsField QgsOracleProvider::field( int index ) const
{
  if ( index < 0 || index >= mAttributeFields.size() )
  {
    QgsMessageLog::logMessage( tr( "FAILURE: Field %1 not found." ).arg( index ), tr( "Oracle" ) );
    throw OracleFieldNotFound();
  }

  return mAttributeFields.at( index );
}

QgsFeatureIterator QgsOracleProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  if ( !mValid )
  {
    QgsMessageLog::logMessage( tr( "Read attempt on an invalid oracle data source" ), tr( "Oracle" ) );
    return QgsFeatureIterator();
  }

  return QgsFeatureIterator( new QgsOracleFeatureIterator( new QgsOracleFeatureSource( this ), true, request ) );
}

/**
 * Returns the number of fields
 */
uint QgsOracleProvider::fieldCount() const
{
  return mAttributeFields.size();
}

QgsFields QgsOracleProvider::fields() const
{
  return mAttributeFields;
}

QString QgsOracleProvider::dataComment() const
{
  return mDataComment;
}

bool QgsOracleProvider::loadFields()
{
  mAttributeFields.clear();
  mDefaultValues.clear();
  mAlwaysGenerated.clear();

  QgsOracleConn *conn = connectionRO();
  QSqlQuery qry( *conn );

  QMap<QString, QString> comments;
  QMap<QString, QString> types;
  QMap<QString, QVariant> defvalues;
  QMap<QString, bool> alwaysGenerated;

  if ( !mIsQuery )
  {
    QgsDebugMsgLevel( QStringLiteral( "Loading fields for table %1" ).arg( mTableName ), 2 );

    if ( LoggedExecStatic( qry, QStringLiteral( "SELECT comments FROM all_tab_comments WHERE owner=? AND table_name=?" ),
                           QVariantList() << mOwnerName << mTableName, mUri.uri() ) )
    {
      if ( qry.next() )
        mDataComment = qry.value( 0 ).toString();
      else if ( LoggedExecStatic( qry, QStringLiteral( "SELECT comments FROM all_mview_comments WHERE owner=? AND mview_name=?" ),
                                  QVariantList() << mOwnerName << mTableName, mUri.uri() ) )
      {
        if ( qry.next() )
          mDataComment = qry.value( 0 ).toString();
      }
    }
    else
    {
      const QString error { tr( "Loading comment for table %1.%2 failed [%3]" )
                            .arg( mOwnerName )
                            .arg( mTableName )
                            .arg( qry.lastError().text() ) };
      QgsMessageLog::logMessage( error,
                                 tr( "Oracle" ) );
    }

    qry.finish();

    if ( LoggedExecStatic( qry, QStringLiteral( "SELECT column_name,comments FROM all_col_comments t WHERE t.owner=? AND t.table_name=?" ),
                           QVariantList() << mOwnerName << mTableName, mUri.uri() ) )
    {
      while ( qry.next() )
      {
        if ( qry.value( 0 ).toString() == mGeometryColumn )
          continue;
        comments.insert( qry.value( 0 ).toString(), qry.value( 1 ).toString() );

      }
    }
    else
    {
      const QString error { tr( "Loading comment for columns of table %1.%2 failed [%3]" ).arg( mOwnerName ).arg( mTableName ).arg( qry.lastError().text() ) };
      QgsMessageLog::logMessage( error, tr( "Oracle" ) );
    }

    qry.finish();

    QVariantList args;
    QString sql( "SELECT"
                 " t.column_name"
                 ",CASE WHEN t.data_type_owner IS NULL THEN t.data_type ELSE t.data_type_owner||'.'||t.data_type END"
                 ",t.data_precision"
                 ",t.data_scale"
                 ",t.char_length"
                 ",t.char_used"
                 ",t.data_default" +
                 QString( mOracleVersion >= 12 ?
                          ",CASE WHEN t.virtual_column = 'YES' OR a.generation_type = 'ALWAYS' THEN 'YES' ELSE 'NO' END" :
                          ",t.virtual_column" ) +
                 " FROM all_tab_cols t" +
                 QString( mOracleVersion >= 12 ?
                          " LEFT JOIN all_tab_identity_cols a ON a.column_name = t.column_name AND a.owner = t.owner AND a.table_name = t.table_name" :
                          "" ) +
                 " WHERE t.owner=? AND t.table_name=?"
                 " AND t.hidden_column='NO'" ) ;
    args << mOwnerName << mTableName;

    if ( !mGeometryColumn.isEmpty() )
    {
      sql += " AND t.column_name<>?";
      args << mGeometryColumn;
    }

    if ( !mIncludeGeoAttributes )
    {
      sql += " AND (t.data_type_owner<>'MDSYS' OR t.data_type<>'SDO_GEOMETRY')";
    }

    sql += " ORDER BY t.column_id";

    if ( LoggedExecStatic( qry, sql, args, mUri.uri() ) )
    {
      long long fetchedRows { 0 };
      while ( qry.next() )
      {
        fetchedRows++;
        QString name      = qry.value( 0 ).toString();
        QString type      = qry.value( 1 ).toString();
        int prec          = qry.value( 2 ).toInt();
        int scale         = qry.value( 3 ).toInt();
        int clength       = qry.value( 4 ).toInt();
        bool cused        = qry.value( 5 ).toString() == "C";
        QVariant defValue = qry.value( 6 );
        bool alwaysGen    = qry.value( 7 ).toString() == "YES";

        if ( type == "CHAR" || type == "VARCHAR2" || type == "VARCHAR" )
        {
          types.insert( name, QString( "%1(%2 %3)" ).arg( type ).arg( clength ).arg( cused ? "CHAR" : "BYTE" ) );
        }
        else if ( type == "NCHAR" || type == "NVARCHAR2" || type == "NVARCHAR" )
        {
          types.insert( name, QString( "%1(%2)" ).arg( type ).arg( clength ) );
        }
        else if ( type == "NUMBER" )
        {
          if ( scale == 0 )
          {
            types.insert( name, QString( "%1(%2)" ).arg( type ).arg( prec ) );
          }
          else
          {
            types.insert( name, QString( "%1(%2,%3)" ).arg( type ).arg( prec ).arg( scale ) );
          }
        }
        else
        {
          types.insert( name, type );
        }

        defvalues.insert( name, defValue );
        alwaysGenerated.insert( name, alwaysGen );
      }

    }
    else
    {
      const QString error { tr( "Loading field types for table %1.%2 failed [%3]" )
                            .arg( mOwnerName )
                            .arg( mTableName )
                            .arg( qry.lastError().text() ) };

      QgsMessageLog::logMessage( error,
                                 tr( "Oracle" ) );
    }

    if ( !mGeometryColumn.isEmpty() )
      mSpatialIndexName = conn->getSpatialIndexName( mOwnerName, mTableName, mGeometryColumn, mHasSpatialIndex );

    mEnabledCapabilities |= QgsVectorDataProvider::CreateSpatialIndex;
  }

  if ( !mGeometryColumn.isEmpty() )
  {
    if ( !mHasSpatialIndex )
    {
      const QString sql{ QStringLiteral( "SELECT %2 FROM %1 WHERE sdo_filter(%2,mdsys.sdo_geometry(2003,%3,NULL,mdsys.sdo_elem_info_array(1,1003,3),mdsys.sdo_ordinate_array(-1,-1,1,1)))='TRUE'" )
                         .arg( mQuery )
                         .arg( quotedIdentifier( mGeometryColumn ) )
                         .arg( mSrid < 1 ? "NULL" : QString::number( mSrid ) ) };

      mHasSpatialIndex = LoggedExecStatic( qry, sql, QVariantList(), mUri.uri() );
    }

    if ( !mHasSpatialIndex )
    {
      QgsMessageLog::logMessage( tr( "No spatial index on column %1.%2.%3 found - expect poor performance." )
                                 .arg( mOwnerName )
                                 .arg( mTableName )
                                 .arg( mGeometryColumn ),
                                 tr( "Oracle" ) );
    }
  }

  qry.finish();

  const QString sql { QStringLiteral( "SELECT * FROM %1 WHERE 1=0" ).arg( mQuery ) };

  if ( !LoggedExecStatic( qry, sql, QVariantList(), mUri.uri() ) )
  {
    const QString error { tr( "Retrieving fields from '%1' failed [%2]" ).arg( mQuery ).arg( qry.lastError().text() ) };
    QgsMessageLog::logMessage( error, tr( "Oracle" ) );
    return false;
  }

  QSqlRecord record = qry.record();

  for ( int i = 0; i < record.count(); i++ )
  {
    QSqlField field = record.field( i );

    if ( field.name() == mGeometryColumn )
      continue;

    if ( !mIsQuery && !types.contains( field.name() ) )
      continue;

    QVariant::Type type = field.type();
    QgsField newField( field.name(), type, types.value( field.name() ), field.length(), field.precision(), comments.value( field.name() ) );
    newField.setReadOnly( alwaysGenerated.value( field.name(), false ) );

    QgsFieldConstraints constraints;
    if ( mPrimaryKeyAttrs.contains( i ) )
      constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
    if ( mPrimaryKeyAttrs.contains( i ) )
      constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
    newField.setConstraints( constraints );

    mAttributeFields.append( newField );
    mDefaultValues.append( defvalues.value( field.name(), QVariant() ) );
    mAlwaysGenerated.append( alwaysGenerated.value( field.name(), false ) );
  }

  return true;
}

bool QgsOracleProvider::hasSufficientPermsAndCapabilities()
{
  QgsDebugMsgLevel( QStringLiteral( "Checking for permissions on the relation" ), 2 );

  mEnabledCapabilities = QgsVectorDataProvider::SelectAtId | QgsVectorDataProvider::TransactionSupport;

  // supports circular geometries
  mEnabledCapabilities |= QgsVectorDataProvider::CircularGeometries;

  QgsOracleConn *conn = connectionRO();

  QSqlQuery qry( *conn );
  if ( !mIsQuery )
  {
    if ( conn->currentUser() == mOwnerName )
    {
      // full set of privileges for the owner
      mEnabledCapabilities |= QgsVectorDataProvider::DeleteFeatures
                              |  QgsVectorDataProvider::ChangeAttributeValues
                              |  QgsVectorDataProvider::AddFeatures
                              |  QgsVectorDataProvider::AddAttributes
                              |  QgsVectorDataProvider::DeleteAttributes
                              |  QgsVectorDataProvider::ChangeGeometries
                              |  QgsVectorDataProvider::RenameAttributes
                              ;
    }
    else
    {
      if ( LoggedExecStatic( qry, QStringLiteral( "SELECT privilege FROM all_tab_privs WHERE table_schema=? AND table_name=? AND privilege IN ('DELETE','UPDATE','INSERT','ALTER TABLE')" ),
                             QVariantList() << mOwnerName << mTableName, mUri.uri() ) )
      {
        // check grants
        long long fetchedRows { 0 };
        while ( qry.next() )
        {
          fetchedRows++;
          QString priv = qry.value( 0 ).toString();

          if ( priv == "DELETE" )
          {
            mEnabledCapabilities |= QgsVectorDataProvider::DeleteFeatures;
          }
          else if ( priv == "UPDATE" )
          {
            mEnabledCapabilities |= QgsVectorDataProvider::ChangeAttributeValues;
          }
          else if ( priv == "INSERT" )
          {
            mEnabledCapabilities |= QgsVectorDataProvider::AddFeatures;
          }
          else if ( priv == "ALTER TABLE" )
          {
            mEnabledCapabilities |= QgsVectorDataProvider::AddAttributes | QgsVectorDataProvider::DeleteAttributes | QgsVectorDataProvider::RenameAttributes;
          }
        }

        if ( !mGeometryColumn.isNull() )
        {

          if ( LoggedExecStatic( qry, QStringLiteral( "SELECT 1 FROM all_col_privs WHERE table_schema=? AND table_name=? AND column_name=? AND privilege='UPDATE'" ),
                                 QVariantList() << mOwnerName << mTableName << mGeometryColumn, mUri.uri() ) )
          {
            if ( qry.next() )
              mEnabledCapabilities |= QgsVectorDataProvider::ChangeGeometries;
          }
          else
          {
            const QString error { tr( "Unable to determine geometry column access privileges for column %1.%2.\nThe error message from the database was:\n%3.\nSQL: %4" )
                                  .arg( mQuery )
                                  .arg( mGeometryColumn )
                                  .arg( qry.lastError().text() )
                                  .arg( qry.lastQuery() ) };
            QgsMessageLog::logMessage( error,
                                       tr( "Oracle" ) );
          }
        }
      }
      else
      {
        const QString error { tr( "Unable to determine table access privileges for the table %1.\nThe error message from the database was:\n%2.\nSQL: %3" )
                              .arg( mQuery )
                              .arg( qry.lastError().text() )
                              .arg( qry.lastQuery() ) };
        QgsMessageLog::logMessage( error,
                                   tr( "Oracle" ) );
      }
    }
  }
  else
  {
    // Check if the sql is a select query
    if ( !mQuery.startsWith( "(" ) && !mQuery.endsWith( ")" ) )
    {
      QgsMessageLog::logMessage( tr( "The custom query is not a select query." ), tr( "Oracle" ) );
      return false;
    }

    const QString sql { QStringLiteral( "SELECT * FROM %1 WHERE 1=0" ).arg( mQuery ) };
    if ( !LoggedExecStatic( qry, sql, QVariantList(), mUri.uri() ) )
    {
      const QString error { tr( "Unable to execute the query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                            .arg( qry.lastError().text() )
                            .arg( qry.lastQuery() ) };
      QgsMessageLog::logMessage( error, tr( "Oracle" ) );
      return false;
    }
  }

  qry.finish();

  return true;
}

bool QgsOracleProvider::determinePrimaryKey()
{
  if ( !loadFields() )
  {
    return false;
  }

  // check to see if there is an unique index on the relation, which
  // can be used as a key into the table. Primary keys are always
  // unique indices, so we catch them as well.
  QgsOracleConn *conn = connectionRO();
  QSqlQuery qry( *conn );
  if ( !mIsQuery )
  {
    const QStringList pkeys = conn->getPrimaryKeys( mOwnerName, mTableName );

    bool isInt = true;

    for ( QString name : pkeys )
    {
      int idx = mAttributeFields.indexFromName( name );
      if ( idx < 0 )
      {
        QgsMessageLog::logMessage( tr( "Primary key field %1 not found in %2" ).arg( name ).arg( mQuery ), tr( "Oracle" ) );
        return false;
      }

      QgsField fld = mAttributeFields.at( idx );

      if ( isInt &&
           fld.type() != QVariant::Int &&
           fld.type() != QVariant::LongLong &&
           !( fld.type() == QVariant::Double && fld.precision() == 0 ) )
        isInt = false;

      mPrimaryKeyAttrs << idx;
    }

    if ( mPrimaryKeyAttrs.size() > 0 )
    {
      mPrimaryKeyType = ( mPrimaryKeyAttrs.size() == 1 && isInt ) ? PktInt : PktFidMap;
    }
    else
    {
      if ( !LoggedExecStatic( qry, QStringLiteral( "SELECT 1 FROM all_tables WHERE owner=? AND table_name=?" ), QVariantList() << mOwnerName << mTableName, mUri.uri() ) )
      {
        const QString error { tr( "Unable to execute the query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                              .arg( qry.lastError().text() )
                              .arg( qry.lastQuery() ) };
        QgsMessageLog::logMessage( error, tr( "Oracle" ) );
      }
      else if ( qry.next() )
      {
        // is table
        QgsMessageLog::logMessage( tr( "No primary key found, using ROWID." ), tr( "Oracle" ) );
        mPrimaryKeyType = PktRowId;
      }
      else
      {
        determinePrimaryKeyFromUriKeyColumn();
      }
    }
  }
  else
  {
    determinePrimaryKeyFromUriKeyColumn();
  }

  qry.finish();

  mValid = mPrimaryKeyType != PktUnknown;

  const auto constMPrimaryKeyAttrs = mPrimaryKeyAttrs;
  for ( int fieldIdx : constMPrimaryKeyAttrs )
  {
    //primary keys are unique, not null
    QgsFieldConstraints constraints = mAttributeFields.at( fieldIdx ).constraints();
    constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
    constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
    mAttributeFields[ fieldIdx ].setConstraints( constraints );
  }

  return mValid;
}

void QgsOracleProvider::determinePrimaryKeyFromUriKeyColumn()
{
  QString primaryKey = mUri.keyColumn();
  mPrimaryKeyType = PktUnknown;

  if ( !primaryKey.isEmpty() )
  {
    int idx = fieldNameIndex( primaryKey );

    if ( idx >= 0 )
    {
      QgsField fld = mAttributeFields.at( idx );

      if ( mUseEstimatedMetadata || uniqueData( mQuery, primaryKey ) )
      {
        if ( fld.type() == QVariant::Int ||
             fld.type() == QVariant::LongLong ||
             ( fld.type() == QVariant::Double && fld.precision() == 0 ) )
        {
          mPrimaryKeyType = PktInt;
        }
        else
        {
          mPrimaryKeyType = PktFidMap;
        }
        mPrimaryKeyAttrs << idx;
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Primary key field '%1' for view/query not unique." ).arg( primaryKey ), tr( "Oracle" ) );
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Key field '%1' for view/query not found." ).arg( primaryKey ), tr( "Oracle" ) );
    }
  }
  else
  {
    QgsMessageLog::logMessage( tr( "No key field for view/query given." ), tr( "Oracle" ) );
  }
}

bool QgsOracleProvider::uniqueData( QString query, QString colName )
{
  Q_UNUSED( query )
  // Check to see if the given column contains unique data
  QgsOracleConn *conn = connectionRO();
  QSqlQuery qry( *conn );

  QString table = mQuery;
  if ( !mSqlWhereClause.isEmpty() )
  {
    table += " WHERE " + mSqlWhereClause;
  }

  // This is tricky: in case of SQL query layers we have a generated uid in the form "qgis_generated_uid_%1_" which cannot be quoted as identifier.

  QString sql = QString( "SELECT (SELECT count(distinct %1) FROM %2)-(SELECT count(%1) FROM %2) FROM dual" )
                .arg( colName.startsWith( QLatin1String( "qgis_generated_uid_" ) ) ? colName : quotedIdentifier( colName ), mQuery );

  if ( !LoggedExecStatic( qry, sql, QVariantList(), mUri.uri() ) || !qry.next() )
  {
    QgsMessageLog::logMessage( tr( "Unable to execute the query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                               .arg( qry.lastError().text(), qry.lastQuery() ), tr( "Oracle" ) );
    return false;
  }

  return qry.value( 0 ).toInt() == 0;
}

// Returns the minimum value of an attribute
QVariant QgsOracleProvider::minimumValue( int index ) const
{
  QgsOracleConn *conn = connectionRO();
  if ( !conn )
    return QVariant( QString() );

  try
  {
    // get the field name
    QgsField fld = field( index );
    QString sql = QString( "SELECT min(%1) FROM %2" )
                  .arg( quotedIdentifier( fld.name() ), mQuery );

    if ( !mSqlWhereClause.isEmpty() )
    {
      sql += QString( " WHERE %1" ).arg( mSqlWhereClause );
    }

    QSqlQuery qry( *conn );

    if ( !LoggedExecStatic( qry, sql, QVariantList(), mUri.uri() ) )
    {
      QgsMessageLog::logMessage( tr( "Unable to execute the query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                                 .arg( qry.lastError().text(), qry.lastQuery() ), tr( "Oracle" ) );
      return QVariant( QString() );
    }

    if ( qry.next() )
    {
      return qry.value( 0 );
    }
  }
  catch ( OracleFieldNotFound )
  {
    ;
  }
  return QVariant( QString() );
}

// Returns the list of unique values of an attribute
QSet<QVariant> QgsOracleProvider::uniqueValues( int index, int limit ) const
{
  QSet<QVariant> uniqueValues;

  QgsOracleConn *conn = connectionRO();
  if ( !conn )
    return uniqueValues;

  try
  {
    // get the field name
    QgsField fld = field( index );
    QString sql = QString( "SELECT DISTINCT %1 FROM %2" )
                  .arg( quotedIdentifier( fld.name() ), mQuery );

    if ( !mSqlWhereClause.isEmpty() )
    {
      sql += QString( " WHERE %1" ).arg( mSqlWhereClause );
    }

    sql +=  QString( " ORDER BY %1" )
            .arg( quotedIdentifier( fld.name() ) );

    if ( limit >= 0 )
    {
      sql = QString( "SELECT * FROM (%1) WHERE rownum<=%2" ).arg( sql ).arg( limit );
    }

    QSqlQuery qry( *conn );

    if ( !LoggedExecStatic( qry, sql, QVariantList(), mUri.uri() ) )
    {
      QgsMessageLog::logMessage( tr( "Unable to execute the query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                                 .arg( qry.lastError().text(), qry.lastQuery() ), tr( "Oracle" ) );
      return QSet<QVariant>();
    }

    while ( qry.next() )
    {
      uniqueValues << qry.value( 0 );
    }
  }
  catch ( OracleFieldNotFound )
  {
    return QSet<QVariant>();
  }

  return uniqueValues;
}

// Returns the maximum value of an attribute
QVariant QgsOracleProvider::maximumValue( int index ) const
{
  QgsOracleConn *conn = connectionRO();
  if ( !conn )
    return QVariant();

  try
  {
    // get the field name
    QgsField fld = field( index );
    QString sql = QString( "SELECT max(%1) FROM %2" )
                  .arg( quotedIdentifier( fld.name() ) )
                  .arg( mQuery );

    if ( !mSqlWhereClause.isEmpty() )
    {
      sql += QString( " WHERE %1" ).arg( mSqlWhereClause );
    }

    QSqlQuery qry( *conn );

    if ( !LoggedExecStatic( qry, sql, QVariantList(), mUri.uri() ) )
    {
      QgsMessageLog::logMessage( tr( "Unable to execute the query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                                 .arg( qry.lastError().text(), qry.lastQuery() ), tr( "Oracle" ) );
      return QVariant( QString() );
    }

    if ( qry.next() )
    {
      return qry.value( 0 );
    }
  }
  catch ( OracleFieldNotFound )
  {
    ;
  }

  return QVariant( QString() );
}


bool QgsOracleProvider::isValid() const
{
  return mValid;
}

QVariant QgsOracleProvider::defaultValue( int fieldId ) const
{
  QString defVal = mDefaultValues.value( fieldId, QString() ).toString();

  if ( providerProperty( EvaluateDefaultValues, false ).toBool() && !defVal.isEmpty() )
  {
    QgsField fld = field( fieldId );
    return evaluateDefaultExpression( defVal, fld.type() );
  }

  return defVal;
}

QString QgsOracleProvider::defaultValueClause( int fieldId ) const
{
  QString defVal = mDefaultValues.value( fieldId, QString() ).toString();
  const bool isGenerated = mAlwaysGenerated.value( fieldId, false );

  if ( isGenerated )
  {
    return defVal;
  }

  if ( !providerProperty( EvaluateDefaultValues, false ).toBool() && !defVal.isEmpty() )
  {
    return defVal;
  }

  return QString();
}

bool QgsOracleProvider::skipConstraintCheck( int fieldIndex, QgsFieldConstraints::Constraint constraint, const QVariant &value ) const
{
  Q_UNUSED( constraint );
  if ( providerProperty( EvaluateDefaultValues, false ).toBool() )
  {
    return !mDefaultValues.value( fieldIndex ).toString().isEmpty();
  }
  else
  {
    // stricter check - if we are evaluating default values only on commit then we can only bypass the check
    // if the attribute values matches the original default clause
    return mDefaultValues.value( fieldIndex ) == value.toString() && !value.isNull();
  }
}

QVariant QgsOracleProvider::evaluateDefaultExpression( const QString &value, const QVariant::Type &fieldType ) const
{
  if ( value.isEmpty() )
  {
    return QVariant( fieldType );
  }

  QgsOracleConn *conn = connectionRO();
  QSqlQuery qry( *conn );

  const QString sql { QStringLiteral( "SELECT %1 FROM dual" ).arg( value ) };

  if ( !LoggedExecStatic( qry, sql, QVariantList(), mUri.uri() ) || !qry.next() )
  {
    throw OracleException( tr( "Evaluation of default value failed" ), qry );
  }

  // return the evaluated value
  return convertValue( fieldType, qry.value( 0 ).toString() );
}


bool QgsOracleProvider::addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags )
{
  if ( flist.size() == 0 )
    return true;

  QgsOracleConn *conn = connectionRW();
  if ( mIsQuery || !conn )
    return false;

  bool returnvalue = true;

  if ( !( flags & QgsFeatureSink::FastInsert ) && !getWorkspace().isEmpty() && getWorkspace().compare( QStringLiteral( "LIVE" ), Qt::CaseInsensitive ) != 0 )
  {
    static bool warn = true;
    if ( warn )
    {
      QgsMessageLog::logMessage( tr( "Retrieval of updated primary keys from versioned tables not supported" ), tr( "Oracle" ) );
      warn = false;
    }
    flags |= QgsFeatureSink::FastInsert;
  }

  QSqlDatabase db( *conn );

  try
  {
    QSqlQuery ins( db ), getfid( db );

    if ( !conn->begin( db ) )
    {
      throw OracleException( tr( "Could not start transaction" ), db );
    }

    // Prepare the INSERT statement
    QString insert = QStringLiteral( "INSERT INTO %1(" ).arg( mQuery );
    QString values = QStringLiteral( ") VALUES (" );
    QString delim;

    QStringList defaultValues;
    QList<int> fieldId;

    if ( !mGeometryColumn.isNull() )
    {
      insert += quotedIdentifier( mGeometryColumn );
      values += '?';
      delim = ',';
    }

    if ( mPrimaryKeyType == PktInt || mPrimaryKeyType == PktFidMap )
    {
      QString keys, kdelim;

      const auto constMPrimaryKeyAttrs = mPrimaryKeyAttrs;
      for ( int idx : constMPrimaryKeyAttrs )
      {
        QgsField fld = field( idx );
        keys += kdelim + quotedIdentifier( fld.name() );
        if ( mAlwaysGenerated.at( idx ) )
          continue;
        insert += delim + quotedIdentifier( fld.name() );
        values += delim + '?';
        delim = ',';
        kdelim = ',';
        fieldId << idx;
        defaultValues << defaultValueClause( idx );
      }

      if ( !getfid.prepare( QStringLiteral( "SELECT %1 FROM %2 WHERE ROWID=?" ).arg( keys, mQuery ) ) )
      {
        throw OracleException( tr( "Could not prepare get feature id statement" ), getfid );
      }
    }

    QgsAttributes attributevec = flist[0].attributes();

    // look for unique attribute values to place in statement instead of passing as parameter
    // e.g. for defaults
    for ( int idx = 0; idx < std::min( attributevec.size(), mAttributeFields.size() ); ++idx )
    {
      if ( mAlwaysGenerated.at( idx ) )
        continue;

      if ( fieldId.contains( idx ) )
        continue;

      QgsField fld = mAttributeFields.at( idx );

      QgsDebugMsgLevel( "Checking field against: " + fld.name(), 4 );

      if ( fld.name().isEmpty() || fld.name() == mGeometryColumn )
        continue;

      insert += delim + quotedIdentifier( fld.name() );

      const QString defVal = defaultValueClause( idx );

      values += delim + '?';
      defaultValues.append( defVal );
      fieldId.append( idx );

      delim = ',';
    }

    insert += values + ")";

    QgsDebugMsgLevel( QStringLiteral( "SQL prepare: %1" ).arg( insert ), 4 );

    if ( !ins.prepare( insert ) )
    {
      throw OracleException( tr( "Could not prepare insert statement" ), ins );
    }

    for ( QgsFeatureList::iterator features = flist.begin(); features != flist.end(); ++features )
    {
      QgsAttributes attributevec = features->attributes();

      QgsDebugMsgLevel( QStringLiteral( "insert feature %1" ).arg( features->id() ), 4 );

      if ( !mGeometryColumn.isNull() )
      {
        appendGeomParam( features->geometry(), ins );
      }

      for ( int i = 0; i < fieldId.size(); i++ )
      {
        QVariant value = attributevec.value( fieldId[i], QVariant() );

        QgsField fld = field( fieldId[i] );
        if ( ( value.isNull() && mPrimaryKeyAttrs.contains( i ) && !defaultValues.at( i ).isEmpty() ) ||
             ( value.toString() == defaultValues[i] ) )
        {
          value = evaluateDefaultExpression( defaultValues[i], fld.type() );
        }
        features->setAttribute( fieldId[i], value );

        QgsDebugMsgLevel( QStringLiteral( "addBindValue: %1" ).arg( value.toString() ), 4 );
        ins.addBindValue( value );
      }

      if ( !ins.exec() )
      {
        throw OracleException( tr( "Could not insert feature %1" ).arg( features->id() ), ins );
      }

      if ( !( flags & QgsFeatureSink::FastInsert ) )
      {
        if ( mPrimaryKeyType == PktRowId )
        {
          features->setId( mShared->lookupFid( QList<QVariant>() << QVariant( ins.lastInsertId() ) ) );
          QgsDebugMsgLevel( QStringLiteral( "new fid=%1" ).arg( features->id() ), 4 );
        }
        else if ( mPrimaryKeyType == PktInt || mPrimaryKeyType == PktFidMap )
        {
          if ( ins.lastInsertId().isValid() )
          {
            getfid.addBindValue( QVariant( ins.lastInsertId() ) );

            const bool result { getfid.exec() };

            if ( !result || !getfid.next() )
              throw OracleException( tr( "Could not retrieve feature id %1" ).arg( features->id() ), getfid );

            int col = 0;
            const auto constMPrimaryKeyAttrs = mPrimaryKeyAttrs;
            for ( int idx : constMPrimaryKeyAttrs )
            {
              QgsField fld = field( idx );

              QVariant v = getfid.value( col++ );
              if ( v.type() != fld.type() )
                v = QgsVectorDataProvider::convertValue( fld.type(), v.toString() );
              features->setAttribute( idx, v );
            }
          }
        }
      }
    }

    ins.finish();

    if ( !conn->commit( db ) )
    {
      throw OracleException( tr( "Could not commit transaction" ), db );
    }

    if ( !( flags & QgsFeatureSink::FastInsert ) )
    {
      // update feature ids
      if ( mPrimaryKeyType == PktInt || mPrimaryKeyType == PktFidMap )
      {
        for ( QgsFeatureList::iterator features = flist.begin(); features != flist.end(); ++features )
        {
          QgsAttributes attributevec = features->attributes();

          if ( mPrimaryKeyType == PktInt )
          {
            features->setId( STRING_TO_FID( attributevec[ mPrimaryKeyAttrs[0] ] ) );
          }
          else
          {
            QVariantList primaryKeyVals;

            const auto constMPrimaryKeyAttrs = mPrimaryKeyAttrs;
            for ( int idx : constMPrimaryKeyAttrs )
            {
              primaryKeyVals << attributevec[ idx ];
            }

            features->setId( mShared->lookupFid( primaryKeyVals ) );
          }
          QgsDebugMsgLevel( QStringLiteral( "new fid=%1" ).arg( features->id() ), 4 );
        }
      }
    }

    if ( mFeaturesCounted >= 0 )
      mFeaturesCounted += flist.size();

    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();
  }
  catch ( OracleException &e )
  {
    QgsDebugMsg( QStringLiteral( "Oracle error: %1" ).arg( e.errorMessage() ) );
    pushError( tr( "Oracle error while adding features: %1" ).arg( e.errorMessage() ) );
    if ( !conn->rollback( db ) )
    {
      QgsMessageLog::logMessage( tr( "Could not rollback transaction" ), tr( "Oracle" ) );
    }
    returnvalue = false;
  }

  return returnvalue;
}

bool QgsOracleProvider::deleteFeatures( const QgsFeatureIds &id )
{
  bool returnvalue = true;

  QgsOracleConn *conn = connectionRW();
  if ( mIsQuery || !conn )
    return false;

  QSqlDatabase db( *conn );

  try
  {
    QSqlQuery qry( db );

    if ( !conn->begin( db ) )
    {
      throw OracleException( tr( "Could not start transaction" ), db );
    }

    for ( QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it )
    {
      QVariantList args;
      QString sql = QStringLiteral( "DELETE FROM %1 WHERE %2" )
                    .arg( mQuery, whereClause( *it, args ) );
      QgsDebugMsgLevel( "delete sql: " + sql, 2 );

      if ( !LoggedExecStatic( qry, sql, args, mUri.uri() ) )
      {
        throw OracleException( tr( "Deletion of feature %1 failed" ).arg( *it ), qry );
      }

      mShared->removeFid( *it );
    }

    qry.finish();

    if ( !conn->commit( db ) )
    {
      throw OracleException( tr( "Could not commit transaction" ), db );
    }

    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();

    mFeaturesCounted -= id.size();
  }
  catch ( OracleException &e )
  {
    pushError( tr( "Oracle error while deleting features: %1" ).arg( e.errorMessage() ) );
    if ( !conn->rollback( db ) )
      QgsMessageLog::logMessage( tr( "Could not rollback transaction" ), tr( "Oracle" ) );
    returnvalue = false;
  }

  return returnvalue;
}

bool QgsOracleProvider::addAttributes( const QList<QgsField> &attributes )
{
  bool returnvalue = true;

  QgsOracleConn *conn = connectionRW();
  if ( mIsQuery || !conn )
    return false;

  QSqlDatabase db( *conn );

  try
  {
    QSqlQuery qry( db );

    if ( !conn->begin( db ) )
    {
      throw OracleException( tr( "Could not start transaction" ), db );
    }

    for ( QList<QgsField>::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter )
    {
      QString type = iter->typeName().toLower();
      if ( type == "char" || type == "varchar2" )
      {
        type = QString( "%1(%2 char)" ).arg( type ).arg( iter->length() );
      }
      else if ( type == "number" )
      {
        if ( iter->precision() > 0 )
        {
          type = QString( "%1(%2,%3)" ).arg( type ).arg( iter->length() ).arg( iter->precision() );
        }
        else
        {
          type = QString( "%1(%2,%3)" ).arg( type ).arg( iter->length() );
        }
      }

      QString sql = QString( "ALTER TABLE %1 ADD %2 %3" )
                    .arg( mQuery, quotedIdentifier( iter->name() ), type );
      QgsDebugMsgLevel( sql, 2 );

      if ( !LoggedExecStatic( qry, sql, QVariantList(), mUri.uri() ) )
      {
        throw OracleException( tr( "Adding attribute %1 failed" ).arg( iter->name() ), qry );
      }

      if ( !iter->comment().isEmpty() )
      {
        sql = QString( "COMMENT ON COLUMN %1.%2 IS ?" )
              .arg( mQuery, quotedIdentifier( iter->name() ) );

        if ( !LoggedExecStatic( qry, sql, QVariantList() << iter->comment(), mUri.uri() ) )
        {
          throw OracleException( tr( "Setting comment on %1 failed" ).arg( iter->name() ), qry );
        }
      }

      qry.finish();

    }

    if ( !conn->commit( db ) )
    {
      throw OracleException( tr( "Could not commit transaction" ), db );
    }

    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();
  }
  catch ( OracleException &e )
  {
    pushError( tr( "Oracle error while adding attributes: %1" ).arg( e.errorMessage() ) );
    if ( !conn->rollback( db ) )
      QgsMessageLog::logMessage( tr( "Could not rollback transaction" ), tr( "Oracle" ) );
    returnvalue = false;
  }

  if ( !loadFields() )
  {
    QgsMessageLog::logMessage( tr( "Could not reload fields." ), tr( "Oracle" ) );
  }

  return returnvalue;
}

bool QgsOracleProvider::deleteAttributes( const QgsAttributeIds &ids )
{
  bool returnvalue = true;

  QgsOracleConn *conn = connectionRW();
  if ( mIsQuery || !conn )
    return false;

  QSqlDatabase db( *conn );

  try
  {
    QSqlQuery qry( db );

    if ( !conn->begin( db ) )
    {
      throw OracleException( tr( "Could not start transaction" ), db );
    }

    qry.finish();

    QList<int> idsList = ids.values();
    std::sort( idsList.begin(), idsList.end(), std::greater<int>() );

    const auto constIdsList = idsList;
    for ( int id : constIdsList )
    {
      QgsField fld = mAttributeFields.at( id );

      QString sql = QString( "ALTER TABLE %1 DROP COLUMN %2" )
                    .arg( mQuery, quotedIdentifier( fld.name() ) );

      //send sql statement and do error handling
      if ( !LoggedExecStatic( qry, sql, QVariantList(), mUri.uri() ) )
      {
        throw OracleException( tr( "Dropping column %1 failed" ).arg( fld.name() ), qry );
      }

      //delete the attribute from mAttributeFields
      mAttributeFields.remove( id );
      mDefaultValues.removeAt( id );
      mAlwaysGenerated.removeAt( id );
    }

    if ( !conn->commit( db ) )
    {
      throw OracleException( tr( "Could not commit transaction" ), db );
    }

    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();
  }
  catch ( OracleException &e )
  {
    pushError( tr( "Oracle error while deleting attributes: %1" ).arg( e.errorMessage() ) );
    if ( !conn->rollback( db ) )
    {
      QgsMessageLog::logMessage( tr( "Could not rollback transaction" ), tr( "Oracle" ) );
    }
    returnvalue = false;
  }

  if ( !loadFields() )
  {
    QgsMessageLog::logMessage( tr( "Could not reload fields." ), tr( "Oracle" ) );
  }

  return returnvalue;
}

bool QgsOracleProvider::renameAttributes( const QgsFieldNameMap &renamedAttributes )
{
  QgsOracleConn *conn = connectionRW();
  if ( mIsQuery || !conn )
    return false;

  QgsFieldNameMap::const_iterator renameIt = renamedAttributes.constBegin();
  for ( ; renameIt != renamedAttributes.constEnd(); ++renameIt )
  {
    int fieldIndex = renameIt.key();
    if ( fieldIndex < 0 || fieldIndex >= mAttributeFields.count() )
    {
      pushError( tr( "Invalid attribute index: %1" ).arg( fieldIndex ) );
      return false;
    }
    if ( mAttributeFields.indexFromName( renameIt.value() ) >= 0 )
    {
      //field name already in use
      pushError( tr( "Error renaming field %1: name '%2' already exists" ).arg( fieldIndex ).arg( renameIt.value() ) );
      return false;
    }
  }

  QSqlDatabase db( *conn );

  bool returnvalue = true;

  try
  {
    QSqlQuery qry( db );

    if ( !conn->begin( db ) )
    {
      throw OracleException( tr( "Could not start transaction" ), db );
    }

    qry.finish();

    for ( renameIt = renamedAttributes.constBegin(); renameIt != renamedAttributes.constEnd(); ++renameIt )
    {
      QString src( mAttributeFields.at( renameIt.key() ).name() );
      const QString sql { QString( "ALTER TABLE %1 RENAME COLUMN %2 TO %3" )
                          .arg( mQuery,
                                quotedIdentifier( src ),
                                quotedIdentifier( renameIt.value() ) ) };

      if ( !LoggedExecStatic( qry, sql, QVariantList(), mUri.uri() ) )
      {
        throw OracleException( tr( "Renaming column %1 to %2 failed" )
                               .arg( quotedIdentifier( src ),
                                     quotedIdentifier( renameIt.value() ) ),
                               qry );
      }
    }

    if ( !conn->commit( db ) )
    {
      throw OracleException( tr( "Could not commit transaction" ), db );
    }

    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();
  }
  catch ( OracleException &e )
  {
    pushError( tr( "Oracle error while renaming attributes: %1" ).arg( e.errorMessage() ) );
    if ( !conn->rollback( db ) )
    {
      QgsMessageLog::logMessage( tr( "Could not rollback transaction" ), tr( "Oracle" ) );
    }
    returnvalue = false;
  }

  if ( !loadFields() )
  {
    QgsMessageLog::logMessage( tr( "Could not reload fields." ), tr( "Oracle" ) );
    returnvalue = false;
  }

  return returnvalue;
}


bool QgsOracleProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  bool returnvalue = true;

  QgsOracleConn *conn = connectionRW();
  if ( mIsQuery || !conn )
    return false;

  if ( attr_map.isEmpty() )
    return true;

  QSqlDatabase db( *conn );

  try
  {
    QSqlQuery qry( db );
    if ( !conn->begin( db ) )
    {
      throw OracleException( tr( "Could not start transaction" ), db );
    }

    // cycle through the features
    for ( QgsChangedAttributesMap::const_iterator iter = attr_map.begin(); iter != attr_map.end(); ++iter )
    {
      QgsFeatureId fid = iter.key();

      // skip added features
      if ( FID_IS_NEW( fid ) )
        continue;

      const QgsAttributeMap &attrs = iter.value();
      if ( attrs.isEmpty() )
        continue;

      QString sql = QString( "UPDATE %1 SET " ).arg( mQuery );

      bool pkChanged = false;
      QList<int> params;

      // cycle through the changed attributes of the feature
      QString delim;
      for ( QgsAttributeMap::const_iterator siter = attrs.begin(); siter != attrs.end(); ++siter )
      {
        try
        {
          QgsField fld = field( siter.key() );

          if ( mAlwaysGenerated.at( siter.key() ) )
          {
            QgsLogger::warning( tr( "Changing the value of GENERATED field %1 is not allowed." ).arg( fld.name() ) );
            continue;
          }

          pkChanged = pkChanged || mPrimaryKeyAttrs.contains( siter.key() );

          sql += delim + QString( "%1=?" ).arg( quotedIdentifier( fld.name() ) );
          delim = ",";

          params << siter.key();
        }
        catch ( OracleFieldNotFound )
        {
          // Field was missing - shouldn't happen
        }
      }

      if ( params.isEmpty() )
        // Don't try to UPDATE an empty set of values (might happen if only GENERATED fields have changed
        continue;

      QVariantList args;
      sql += QString( " WHERE %1" ).arg( whereClause( fid, args ) );

      if ( !qry.prepare( sql ) )
      {
        throw OracleException( tr( "Could not prepare update statement." ), qry );
      }

      const auto constParams = params;
      for ( int idx : constParams )
      {
        const QgsField &fld = field( idx );

        if ( fld.typeName().endsWith( ".SDO_GEOMETRY" ) )
        {
          QgsGeometry g;
          if ( !attrs[idx].isNull() )
          {
            g = QgsGeometry::fromWkt( attrs[ idx ].toString() );
          }
          appendGeomParam( g, qry );
        }
        else
        {
          qry.addBindValue( attrs[ idx ] );
        }
      }

      for ( const auto &arg : std::as_const( args ) )
        qry.addBindValue( arg );

      QgsDatabaseQueryLogWrapper logWrapper { sql, mUri.uri(), QStringLiteral( "oracle" ), QStringLiteral( "QgsOracleProvider" ), QGS_QUERY_LOG_ORIGIN };

      if ( !qry.exec() )
      {
        logWrapper.setError( qry.lastError().text() );
        logWrapper.setQuery( QgsOracleConn::getLastExecutedQuery( qry ) );
        throw OracleException( tr( "Update of feature %1 failed" ).arg( iter.key() ), qry );
      }

      qry.finish();
      logWrapper.setQuery( QgsOracleConn::getLastExecutedQuery( qry ) );

      // update feature id map if key was changed
      if ( pkChanged && mPrimaryKeyType == PktFidMap )
      {
        QVariant v = mShared->removeFid( fid );

        QList<QVariant> k = v.toList();

        for ( int i = 0; i < mPrimaryKeyAttrs.size(); i++ )
        {
          int idx = mPrimaryKeyAttrs[i];
          if ( !attrs.contains( idx ) )
            continue;

          k[i] = attrs[ idx ];
        }

        mShared->insertFid( fid, k );
      }
    }

    if ( !conn->commit( db ) )
    {
      throw OracleException( tr( "Could not commit transaction" ), db );
    }

    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();
  }
  catch ( OracleException &e )
  {
    pushError( tr( "Oracle error while changing attributes: %1" ).arg( e.errorMessage() ) );
    if ( !conn->rollback( db ) )
    {
      QgsMessageLog::logMessage( tr( "Could not rollback transaction" ), tr( "Oracle" ) );
    }
    returnvalue = false;
  }

  return returnvalue;
}

void QgsOracleProvider::appendGeomParam( const QgsGeometry &geom, QSqlQuery &qry ) const
{
  QOCISpatialGeometry g;

  QByteArray wkb = geom.asWkb();

  wkbPtr ptr;
  ptr.ucPtr = !geom.isEmpty() ? reinterpret_cast< unsigned char * >( const_cast<char *>( wkb.constData() ) ) : nullptr;
  g.isNull = !ptr.ucPtr;
  g.gtype = -1;
  g.srid  = mSrid < 1 ? -1 : mSrid;

  if ( !g.isNull )
  {
    ptr.ucPtr++;  // skip endianness

    g.eleminfo.clear();
    g.ordinates.clear();

    int iOrdinate = 1;
    QgsWkbTypes::Type type = ( QgsWkbTypes::Type ) * ptr.iPtr++;
    int dim = 2;

    switch ( type )
    {
      case QgsWkbTypes::Point25D:
      case QgsWkbTypes::PointZ:
        dim = 3;
        FALLTHROUGH

      case QgsWkbTypes::Point:
        g.srid  = mSrid;
        g.gtype = SDO_GTYPE( dim, GtPoint );
        g.x = *ptr.dPtr++;
        g.y = *ptr.dPtr++;
        g.z = dim == 3 ? *ptr.dPtr++ : 0.0;
        break;

      case QgsWkbTypes::LineString25D:
      case QgsWkbTypes::MultiLineString25D:
      case QgsWkbTypes::LineStringZ:
      case QgsWkbTypes::MultiLineStringZ:
        dim = 3;
        FALLTHROUGH

      case QgsWkbTypes::LineString:
      case QgsWkbTypes::MultiLineString:
      {
        g.gtype = SDO_GTYPE( dim, GtLine );
        int nLines = 1;
        if ( type == QgsWkbTypes::MultiLineString25D || type == QgsWkbTypes::MultiLineString || type == QgsWkbTypes::MultiLineStringZ )
        {
          g.gtype = SDO_GTYPE( dim, GtMultiLine );
          nLines = *ptr.iPtr++;
          ptr.ucPtr++; // Skip endianness of first linestring
          ptr.iPtr++;  // Skip type of first linestring
        }

        for ( int iLine = 0; iLine < nLines; iLine++ )
        {
          g.eleminfo << iOrdinate << 2 << 1;

          for ( int i = 0, n = *ptr.iPtr++; i < n; i++ )
          {
            g.ordinates << *ptr.dPtr++;
            g.ordinates << *ptr.dPtr++;
            if ( dim == 3 )
              g.ordinates << *ptr.dPtr++;
            iOrdinate  += dim;
          }

          ptr.ucPtr++; // Skip endianness of next linestring
          ptr.iPtr++;  // Skip type of next linestring
        }
      }
      break;

      case QgsWkbTypes::Polygon25D:
      case QgsWkbTypes::MultiPolygon25D:
      case QgsWkbTypes::PolygonZ:
      case QgsWkbTypes::MultiPolygonZ:
        dim = 3;
        FALLTHROUGH

      case QgsWkbTypes::Polygon:
      case QgsWkbTypes::MultiPolygon:
      {
        g.gtype = SDO_GTYPE( dim, GtPolygon );
        int nPolygons = 1;
        const QgsMultiPolygon *multipoly =
          ( QgsWkbTypes::flatType( type ) == QgsWkbTypes::MultiPolygon ) ?
          dynamic_cast<const QgsMultiPolygon *>( geom.constGet() ) : nullptr;
        if ( multipoly )
        {
          g.gtype = SDO_GTYPE( dim, GtMultiPolygon );
          nPolygons = *ptr.iPtr++;

          ptr.ucPtr++; // Skip endianness of first polygon
          ptr.iPtr++;  // Skip type of first polygon
        }

        for ( int iPolygon = 0; iPolygon < nPolygons; iPolygon++ )
        {
          const QgsPolygon *poly = multipoly ?
                                   dynamic_cast<const QgsPolygon *>( multipoly->geometryN( iPolygon ) ) :
                                   dynamic_cast<const QgsPolygon *>( geom.constGet() );
          for ( int iRing = 0, nRings = *ptr.iPtr++; iRing < nRings; iRing++ )
          {
            g.eleminfo << iOrdinate << ( iRing == 0 ? 1003 : 2003 ) << 1;

            // Oracle polygons must have their exterior ring in counterclockwise
            // order, and the interior ring(s) in clockwise order.
            const bool reverseRing =
              iRing == 0 ? poly->exteriorRing()->orientation() == Qgis::AngularDirection::Clockwise :
              poly->interiorRing( iRing - 1 )->orientation() == Qgis::AngularDirection::CounterClockwise;

            const int n = *ptr.iPtr++;
            if ( reverseRing )
            {
              const double *dPtr = ptr.dPtr + n * dim;
              for ( int i = 0; i < n; i++ )
              {
                dPtr -= dim;
                g.ordinates << dPtr[0];
                g.ordinates << dPtr[1];
                if ( dim == 3 )
                  g.ordinates << dPtr[2];
              }
              ptr.dPtr += n * dim;
            }
            else
            {
              for ( int i = 0; i < n; i++ )
              {
                g.ordinates << *ptr.dPtr++;
                g.ordinates << *ptr.dPtr++;
                if ( dim == 3 )
                  g.ordinates << *ptr.dPtr++;
              }
            }
            iOrdinate  += n * dim;
          }

          ptr.ucPtr++; // Skip endianness of next polygon
          ptr.iPtr++;  // Skip type of next polygon
        }
      }
      break;

      case QgsWkbTypes::MultiPoint25D:
      case QgsWkbTypes::MultiPointZ:
        dim = 3;
        FALLTHROUGH

      case QgsWkbTypes::MultiPoint:
      {
        g.gtype = SDO_GTYPE( dim, GtMultiPoint );
        int n = *ptr.iPtr++;

        g.eleminfo << 1 << 1 << n;

        for ( int i = 0; i < n; i++ )
        {
          ptr.ucPtr++; // Skip endianness of point
          ptr.iPtr++;  // Skip type of point

          g.ordinates << *ptr.dPtr++;
          g.ordinates << *ptr.dPtr++;
          if ( dim == 3 )
            g.ordinates << *ptr.dPtr++;
        }
      }
      break;

      case QgsWkbTypes::CircularStringZ:
      case QgsWkbTypes::CompoundCurveZ:
      case QgsWkbTypes::MultiCurveZ:
        dim = 3;
        FALLTHROUGH

      case QgsWkbTypes::CircularString:
      case QgsWkbTypes::CompoundCurve:
      case QgsWkbTypes::MultiCurve:
      {
        g.gtype = SDO_GTYPE( dim, GtLine );
        int nCurves = 1;
        if ( type == QgsWkbTypes::MultiCurve || type == QgsWkbTypes::MultiCurveZ )
        {
          g.gtype = SDO_GTYPE( dim, GtMultiLine );
          nCurves = *ptr.iPtr++;
        }

        for ( int iCurve = 0; iCurve < nCurves; iCurve++ )
        {
          QgsWkbTypes::Type curveType = type;
          if ( type == QgsWkbTypes::MultiCurve || type == QgsWkbTypes::MultiCurveZ )
          {
            ptr.ucPtr++; // Skip endianness of curve
            curveType = ( QgsWkbTypes::Type ) * ptr.iPtr++; // type of curve
          }

          int nLines = 1;
          QgsWkbTypes::Type lineType = curveType;
          if ( curveType == QgsWkbTypes::CompoundCurve || curveType == QgsWkbTypes::CompoundCurveZ )
          {
            nLines = *ptr.iPtr++;

            // Oracle don't store compound curve with only one line
            if ( nLines > 1 )
            {
              g.eleminfo << iOrdinate << 4 << nLines;
            }

            ptr.ucPtr++; // Skip endianness of first linestring
            lineType = ( QgsWkbTypes::Type ) * ptr.iPtr++; // type of first linestring
          }

          for ( int iLine = 0; iLine < nLines; iLine++ )
          {
            if ( iLine > 0 )
            {
              ptr.ucPtr++; // Skip endianness of linestring
              lineType = ( QgsWkbTypes::Type ) * ptr.iPtr++; // type of linestring
            }
            bool circularString = lineType == QgsWkbTypes::CircularString || lineType == QgsWkbTypes::CircularStringZ;

            g.eleminfo << iOrdinate << 2 << ( circularString ? 2 : 1 );

            for ( int i = 0, n = *ptr.iPtr++; i < n; i++ )
            {
              // Inside a compound curve, two consecutives lines share start/end points
              // We don't repeat this point in ordinates, so we skip the last point (except for last line)
              if ( ( curveType == QgsWkbTypes::CompoundCurve || curveType == QgsWkbTypes::CompoundCurveZ )
                   && i == n - 1 && iLine < nLines - 1 )
              {
                ptr.dPtr += dim;
                continue;
              }

              g.ordinates << *ptr.dPtr++;
              g.ordinates << *ptr.dPtr++;
              if ( dim == 3 )
                g.ordinates << *ptr.dPtr++;

              iOrdinate  += dim;
            }
          }
        }
      }
      break;


      case QgsWkbTypes::CurvePolygonZ:
      case QgsWkbTypes::MultiSurfaceZ:
        dim = 3;
        FALLTHROUGH

      case QgsWkbTypes::CurvePolygon:
      case QgsWkbTypes::MultiSurface:
      {
        g.gtype = SDO_GTYPE( dim, GtPolygon );
        int nSurfaces = 1;
        const QgsMultiSurface *multisurface =
          ( QgsWkbTypes::flatType( type ) == QgsWkbTypes::MultiSurface ) ?
          dynamic_cast<const QgsMultiSurface *>( geom.constGet() ) : nullptr;
        if ( multisurface )
        {
          g.gtype = SDO_GTYPE( dim, GtMultiPolygon );
          nSurfaces = multisurface->numGeometries();
        }

        for ( int iSurface = 0; iSurface < nSurfaces; iSurface++ )
        {
          const QgsCurvePolygon *curvepoly = multisurface ?
                                             dynamic_cast<const QgsCurvePolygon *>( multisurface->geometryN( iSurface ) ) :
                                             dynamic_cast<const QgsCurvePolygon *>( geom.constGet() );

          const int nRings = ( curvepoly->exteriorRing() ? 1 : 0 ) + curvepoly->numInteriorRings();

          for ( int iRing = 0; iRing < nRings; iRing++ )
          {
            const QgsCurve *ring = iRing == 0 ? curvepoly->exteriorRing() : curvepoly->interiorRing( iRing - 1 );
            const QgsWkbTypes::Type ringType = ring->wkbType();

            // Oracle polygons must have their exterior ring in counterclockwise
            // order, and the interior ring(s) in clockwise order.
            const bool reverseRing =
              iRing == 0 ? ring->orientation() == Qgis::AngularDirection::Clockwise :
              ring->orientation() == Qgis::AngularDirection::CounterClockwise;
            std::unique_ptr<QgsCurve> reversedRing( reverseRing ? ring->reversed() : nullptr );
            const QgsCurve *correctedRing = reversedRing ? reversedRing.get() : ring;
            const QgsCompoundCurve *compound = dynamic_cast<const QgsCompoundCurve *>( correctedRing );
            int nLines = 1;
            QgsWkbTypes::Type lineType = ringType;
            if ( compound )
            {
              nLines = compound->nCurves();
              if ( nLines )
                lineType = compound->curveAt( 0 )->wkbType();
            }

            // Oracle don't store compound curve with only one line
            g.eleminfo << iOrdinate
                       << ( iRing == 0 ? 1000 : 2000 ) + ( nLines > 1 ? 5 : 3 )
                       << ( nLines > 1 ? nLines : ( QgsWkbTypes::flatType( lineType ) == QgsWkbTypes::CircularString ? 2 : 1 ) );

            for ( int iLine = 0; iLine < nLines; iLine++ )
            {
              if ( iLine > 0 )
              {
                lineType = compound->curveAt( iLine )->wkbType();
              }
              if ( nLines > 1 )
              {
                g.eleminfo << iOrdinate << 2 << ( QgsWkbTypes::flatType( lineType ) == QgsWkbTypes::CircularString ? 2 : 1 );
              }
              const QgsCurve *lineCurve = compound ? compound->curveAt( iLine ) : correctedRing;

              for ( int i = 0, n = lineCurve->numPoints(); i < n; i++ )
              {
                // Inside a compound polygon, two consecutives lines share start/end points
                // We don't repeat this point in ordinates, so we skip the last point (except for last line)
                if ( compound
                     && i == n - 1 && iLine < nLines - 1 )
                {
                  break;
                }

                QgsPoint p;
                Qgis::VertexType ignored;
                lineCurve->pointAt( i, p, ignored );
                g.ordinates << p.x();
                g.ordinates << p.y();
                if ( dim == 3 )
                  g.ordinates << p.z();

                iOrdinate  += dim;
              }
            }
          }
        }
      }
      break;

      // unsupported M values
      case QgsWkbTypes::PointM:
      case QgsWkbTypes::PointZM:
      case QgsWkbTypes::LineStringM:
      case QgsWkbTypes::LineStringZM:
      case QgsWkbTypes::PolygonM:
      case QgsWkbTypes::PolygonZM:
      case QgsWkbTypes::MultiPointM:
      case QgsWkbTypes::MultiPointZM:
      case QgsWkbTypes::MultiLineStringM:
      case QgsWkbTypes::MultiLineStringZM:
      case QgsWkbTypes::MultiPolygonM:
      case QgsWkbTypes::MultiPolygonZM:
      case QgsWkbTypes::CircularStringM:
      case QgsWkbTypes::CircularStringZM:
      case QgsWkbTypes::CompoundCurveM:
      case QgsWkbTypes::CompoundCurveZM:
      case QgsWkbTypes::MultiCurveM:
      case QgsWkbTypes::MultiCurveZM:
      case QgsWkbTypes::CurvePolygonM:
      case QgsWkbTypes::CurvePolygonZM:
      case QgsWkbTypes::MultiSurfaceM:
      case QgsWkbTypes::MultiSurfaceZM:

      // other unsupported or missing geometry types
      case QgsWkbTypes::GeometryCollection:
      case QgsWkbTypes::GeometryCollectionZ:
      case QgsWkbTypes::GeometryCollectionM:
      case QgsWkbTypes::GeometryCollectionZM:
      case QgsWkbTypes::Triangle:
      case QgsWkbTypes::TriangleZ:
      case QgsWkbTypes::TriangleM:
      case QgsWkbTypes::TriangleZM:
      case QgsWkbTypes::Unknown:
      case QgsWkbTypes::NoGeometry:

        g.isNull = true;
        break;
    }
  }

  QgsDebugMsgLevel( QStringLiteral( "addBindValue geometry: isNull=%1 gtype=%2 srid=%3 p=%4,%5,%6 eleminfo=%7 ordinates=%8" )
                    .arg( g.isNull )
                    .arg( g.gtype )
                    .arg( g.srid )
                    .arg( g.x ).arg( g.y ).arg( g.z )
                    .arg( g.eleminfo.size() )
                    .arg( g.ordinates.size() )
                    , 4 );
  qry.addBindValue( QVariant::fromValue( g ) );
}

bool QgsOracleProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  QgsOracleConn *conn = connectionRW();
  if ( mIsQuery || mGeometryColumn.isNull() || !conn )
    return false;

  QSqlDatabase db( *conn );

  bool returnvalue = true;

  try
  {
    QSqlQuery qry( db );
    if ( !conn->begin( db ) )
    {
      throw OracleException( tr( "Could not start transaction" ), db );
    }

    QString update = QString( "UPDATE %1 SET %2=? WHERE %3" )
                     .arg( mQuery )
                     .arg( quotedIdentifier( mGeometryColumn ) )
                     .arg( pkParamWhereClause() );
    QgsDebugMsgLevel( QStringLiteral( "SQL prepare: %1" ).arg( update ), 4 );
    if ( !qry.prepare( update ) )
    {
      throw OracleException( tr( "Could not prepare update statement." ), qry );
    }

    for ( QgsGeometryMap::const_iterator iter = geometry_map.constBegin();
          iter != geometry_map.constEnd();
          ++iter )
    {
      appendGeomParam( iter.value(), qry );
      appendPkParams( iter.key(), qry );

      QgsDatabaseQueryLogWrapper logWrapper { update, mUri.uri(), QStringLiteral( "oracle" ), QStringLiteral( "QgsOracleProvider" ), QGS_QUERY_LOG_ORIGIN };

      if ( !qry.exec() )
      {
        logWrapper.setQuery( QgsOracleConn::getLastExecutedQuery( qry ) );
        logWrapper.setError( qry.lastError().text() );
        throw OracleException( tr( "Update of feature %1 failed" ).arg( iter.key() ), qry );
      }
      logWrapper.setQuery( QgsOracleConn::getLastExecutedQuery( qry ) );
    }

    qry.finish();

    if ( !conn->commit( db ) )
    {
      throw OracleException( tr( "Could not commit transaction" ), db );
    }

    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();
  }
  catch ( OracleException &e )
  {
    pushError( tr( "Oracle error while changing geometry values: %1" ).arg( e.errorMessage() ) );
    if ( !conn->rollback( db ) )
    {
      QgsMessageLog::logMessage( tr( "Could not rollback transaction" ), tr( "Oracle" ) );
    }
    returnvalue = false;
  }

  return returnvalue;
}

QgsVectorDataProvider::Capabilities QgsOracleProvider::capabilities() const
{
  return mEnabledCapabilities;
}

bool QgsOracleProvider::setSubsetString( const QString &theSQL, bool updateFeatureCount )
{
  QgsOracleConn *conn = connectionRO();
  if ( !conn )
    return false;

  if ( theSQL.trimmed() == mSqlWhereClause )
    return true;

  QString prevWhere = mSqlWhereClause;

  mSqlWhereClause = theSQL.trimmed();

  QString sql = QString( "SELECT * FROM %1 WHERE " ).arg( mQuery );

  if ( !mSqlWhereClause.isEmpty() )
  {
    sql += "(" + mSqlWhereClause + ") AND ";
  }

  sql += "1=0";

  QSqlQuery qry( *conn );


  if ( !LoggedExecStatic( qry, sql, QVariantList(), mUri.uri() ) )
  {
    pushError( qry.lastError().text() );
    mSqlWhereClause = prevWhere;
    qry.finish();
    return false;
  }
  qry.finish();

  if ( mPrimaryKeyType == PktInt && !mUseEstimatedMetadata && !uniqueData( mQuery, mAttributeFields.at( mPrimaryKeyAttrs[0] ).name() ) )
  {
    mSqlWhereClause = prevWhere;
    return false;
  }

  // Update datasource uri too
  mUri.setSql( theSQL );
  // Update yet another copy of the uri. Why are there 3 copies of the
  // uri? Perhaps this needs some rationalisation.....
  setDataSourceUri( mUri.uri( false ) );

  if ( updateFeatureCount )
  {
    mFeaturesCounted = -1;
  }
  mLayerExtent.setMinimal();

  emit dataChanged();

  return true;
}

/**
 * Returns the feature count
 */
long long QgsOracleProvider::featureCount() const
{
  QgsOracleConn *conn = connectionRO();
  if ( mFeaturesCounted >= 0 || !conn )
    return mFeaturesCounted;

  // get total number of features
  QString sql;

  // use estimated metadata even when there is a where clause,
  // although we get an incorrect feature count for the subset
  // - but make huge dataset usable.
  QVariantList args;
  if ( !mIsQuery && mUseEstimatedMetadata )
  {
    sql = QString( "SELECT num_rows FROM all_tables WHERE owner=? AND table_name=?" );
    args << mOwnerName << mTableName;
  }
  else
  {
    sql = QString( "SELECT count(*) FROM %1" ).arg( mQuery );

    if ( !mSqlWhereClause.isEmpty() )
    {
      sql += " WHERE " + mSqlWhereClause;
    }
  }

  QSqlQuery qry( *conn );

  if ( LoggedExecStatic( qry, sql, QVariantList( ), mUri.uri() ) && qry.next() )
  {
    mFeaturesCounted = qry.value( 0 ).toLongLong();
  }

  qry.finish();

  QgsDebugMsgLevel( "number of features: " + QString::number( mFeaturesCounted ), 2 );

  return mFeaturesCounted;
}

QgsRectangle QgsOracleProvider::extent() const
{
  QgsOracleConn *conn = connectionRO();
  if ( mGeometryColumn.isNull() || !conn )
    return QgsRectangle();

  if ( mLayerExtent.isEmpty() )
  {
    QString sql;
    QSqlQuery qry( *conn );
    bool ok = false;

    if ( !mIsQuery )
    {
      if ( mUseEstimatedMetadata )
      {
        // TODO: make SDO_DIMNAME values configurable (#16252)

        const QString sql { QStringLiteral( "SELECT sdo_lb,sdo_ub FROM mdsys.all_sdo_geom_metadata m, table(m.diminfo) WHERE owner=? AND table_name=? AND column_name=? AND sdo_dimname='X'" ) };

        if ( LoggedExecStatic( qry, sql,
                               QVariantList() << mOwnerName << mTableName << mGeometryColumn, mUri.uri() ) &&
             qry.next() )
        {
          mLayerExtent.setXMinimum( qry.value( 0 ).toDouble() );
          mLayerExtent.setXMaximum( qry.value( 1 ).toDouble() );

          if ( LoggedExecStatic( qry, QStringLiteral( "SELECT sdo_lb,sdo_ub FROM mdsys.all_sdo_geom_metadata m, table(m.diminfo) WHERE owner=? AND table_name=? AND column_name=? AND sdo_dimname='Y'" ),
                                 QVariantList() << mOwnerName << mTableName << mGeometryColumn, mUri.uri() ) &&
               qry.next() )
          {
            mLayerExtent.setYMinimum( qry.value( 0 ).toDouble() );
            mLayerExtent.setYMaximum( qry.value( 1 ).toDouble() );
            return mLayerExtent;
          }
        }
      }

      if ( mHasSpatialIndex && mUseEstimatedMetadata )
      {
        const QString sql { QStringLiteral( "SELECT SDO_TUNE.EXTENT_OF(?,?) FROM dual" ) };
        ok = LoggedExecStatic( qry,
                               sql,
                               QVariantList() << QString( "%1.%2" ).arg( mOwnerName ).arg( mTableName ) << mGeometryColumn, mUri.uri() );
      }
    }

    if ( !ok )
    {
      sql = QString( "SELECT SDO_AGGR_MBR(%1) FROM %2" ).arg( quotedIdentifier( mGeometryColumn ) ).arg( mQuery );

      if ( !mSqlWhereClause.isEmpty() )
        sql += QString( " WHERE %1" ).arg( mSqlWhereClause );

      ok = LoggedExecStatic( qry, sql, QVariantList(), mUri.uri() );
    }

    if ( ok && qry.next() )
    {
      QByteArray ba( qry.value( 0 ).toByteArray() );
      QgsGeometry g;
      g.fromWkb( ba );
      mLayerExtent = g.boundingBox();
      QgsDebugMsgLevel( "extent: " + mLayerExtent.toString(), 2 );
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Could not retrieve extents: %1\nSQL: %2" ).arg( qry.lastError().text() ).arg( qry.lastQuery() ), tr( "Oracle" ) );
    }
  }

  return mLayerExtent;
}

void QgsOracleProvider::updateExtents()
{
  mLayerExtent.setMinimal();
}

bool QgsOracleProvider::getGeometryDetails()
{
  if ( mGeometryColumn.isNull() )
  {
    mDetectedGeomType = QgsWkbTypes::NoGeometry;
    mValid = true;
    return true;
  }

  // Trust the datasource config means that we used requested geometry type and srid
  if ( mReadFlags & QgsDataProvider::FlagTrustDataSource )
  {
    mDetectedGeomType = mRequestedGeomType;
    return true;
  }

  QString ownerName = mOwnerName;
  QString tableName = mTableName;
  QString geomCol = mGeometryColumn;

  QgsOracleConn *conn = connectionRO();
  QSqlQuery qry( *conn );
  if ( mIsQuery )
  {
    const QString sql {  QStringLiteral( "SELECT %1 FROM %2 WHERE 1=0" ).arg( quotedIdentifier( mGeometryColumn ) ).arg( mQuery ) };

    if ( !LoggedExecStatic( qry, sql, QVariantList(), mUri.uri() ) )
    {
      QgsMessageLog::logMessage( tr( "Could not execute query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                                 .arg( qry.lastError().text() )
                                 .arg( qry.lastQuery() ), tr( "Oracle" ) );
      mValid = false;
      return false;
    }

    ownerName = "";
    tableName = mQuery;
  }

  int detectedSrid = -1;
  QgsWkbTypes::Type detectedType = QgsWkbTypes::Unknown;
  mHasSpatialIndex = false;

  if ( mIsQuery )
  {
    detectedSrid = mSrid;
    detectedType = mRequestedGeomType;
  }

  if ( !ownerName.isEmpty() )
  {

    {
      const QString sql {QStringLiteral( "SELECT srid FROM mdsys.all_sdo_geom_metadata WHERE owner=? AND table_name=? AND column_name=?" ) };

      if ( LoggedExecStatic( qry, sql,
                             QVariantList() << ownerName << tableName << geomCol, mUri.uri() ) )
      {
        if ( qry.next() )
        {
          detectedSrid = qry.value( 0 ).toInt();
        }
        else
        {
          QgsMessageLog::logMessage( tr( "Could not retrieve SRID of %1.\nThe error message from the database was:\n%2.\nSQL: %3" )
                                     .arg( mQuery )
                                     .arg( qry.lastError().text() )
                                     .arg( qry.lastQuery() ), tr( "Oracle" ) );
        }
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Could not determine SRID of %1.\nThe error message from the database was:\n%2.\nSQL: %3" )
                                   .arg( mQuery )
                                   .arg( qry.lastError().text() )
                                   .arg( qry.lastQuery() ), tr( "Oracle" ) );
      }
    }

    QString sql { mUseEstimatedMetadata ?
                  QStringLiteral( "SELECT DISTINCT gtype FROM (SELECT t.%1.sdo_gtype AS gtype FROM %2 t WHERE t.%1 IS NOT NULL AND rownum<100) WHERE rownum<=2" ) :
                  QStringLiteral( "SELECT DISTINCT t.%1.sdo_gtype FROM %2 t WHERE t.%1 IS NOT NULL AND rownum<=2" ) };

    sql = sql.arg( quotedIdentifier( geomCol ), mQuery );

    if ( LoggedExecStatic( qry, sql, QVariantList(), mUri.uri() ) )
    {
      if ( qry.next() )
      {
        detectedType = QgsOracleConn::wkbTypeFromDatabase( qry.value( 0 ).toInt() );
        if ( qry.next() )
        {
          detectedType = QgsWkbTypes::Unknown;
        }
      }
      else
      {
        detectedType = QgsWkbTypes::Unknown;
        QgsMessageLog::logMessage( tr( "%1 has no valid geometry types.\nSQL: %2" )
                                   .arg( mQuery )
                                   .arg( qry.lastQuery() ), tr( "Oracle" ) );
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Could not determine geometry type of %1.\nThe error message from the database was:\n%2.\nSQL: %3" )
                                 .arg( mQuery )
                                 .arg( qry.lastError().text() )
                                 .arg( qry.lastQuery() ), tr( "Oracle" ) );
    }
  }

  if ( detectedType == QgsWkbTypes::Unknown || detectedSrid <= 0 )
  {
    QgsOracleLayerProperty layerProperty;

    if ( !mIsQuery )
    {
      layerProperty.ownerName = ownerName;
      layerProperty.tableName = tableName;
      layerProperty.geometryColName = mGeometryColumn;
      layerProperty.types << detectedType;
      layerProperty.srids << detectedSrid;

      QString delim = "";

      if ( !mSqlWhereClause.isEmpty() )
      {
        layerProperty.sql += delim + "(" + mSqlWhereClause + ")";
        delim = " AND ";
      }

      conn->retrieveLayerTypes( layerProperty, mUseEstimatedMetadata, false );

      Q_ASSERT( layerProperty.types.size() == layerProperty.srids.size() );
    }

    if ( layerProperty.types.isEmpty() )
    {
      // no data - so take what's requested
      if ( mRequestedGeomType == QgsWkbTypes::Unknown )
      {
        QgsMessageLog::logMessage( tr( "Geometry type and srid for empty column %1 of %2 undefined." ).arg( mGeometryColumn, mQuery ) );
      }

      detectedType = QgsWkbTypes::Unknown;
    }
    else
    {
      // requested type && srid is available
      if ( mRequestedGeomType == QgsWkbTypes::Unknown || layerProperty.types.contains( mRequestedGeomType ) )
      {
        if ( layerProperty.size() == 1 )
        {
          // only what we requested is available
          detectedType = layerProperty.types.at( 0 );
          detectedSrid = layerProperty.srids.at( 0 );
        }
        else
        {
          // we need to filter
          detectedType = QgsWkbTypes::Unknown;
        }
      }
      else
      {
        // geometry type undetermined or not unrequested
        QgsMessageLog::logMessage( tr( "Feature type or srid for %1 of %2 could not be determined or was not requested." ).arg( mGeometryColumn ).arg( mQuery ) );
        detectedType = QgsWkbTypes::Unknown;
      }
    }
  }

  mDetectedGeomType = detectedType;
  if ( detectedSrid != -1 )
    mSrid = detectedSrid;

  QgsDebugMsgLevel( QStringLiteral( "Detected Oracle SRID is %1" ).arg( mSrid ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Detected type is %1" ).arg( mDetectedGeomType ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Requested type is %1" ).arg( mRequestedGeomType ), 2 );

  mValid = ( mDetectedGeomType != QgsWkbTypes::Unknown || mRequestedGeomType != QgsWkbTypes::Unknown );

  if ( !mValid )
    return false;

  QgsDebugMsgLevel( QStringLiteral( "Feature type name is %1" ).arg( QgsWkbTypes::displayString( wkbType() ) ), 2 );

  return mValid;
}

bool QgsOracleProvider::createSpatialIndex()
{
  if ( mGeometryColumn.isEmpty() )
    return true;

  QgsOracleConn *conn = connectionRW();
  if ( !conn )
    return false;

  QSqlQuery qry( *conn );

  if ( !crs().isGeographic() )
  {
    // TODO: make precision configurable
    // TODO: make SDO_DIMNAME values configurable (#16252)
    QgsRectangle r( extent() );

    const QString sql { QStringLiteral( "UPDATE mdsys.user_sdo_geom_metadata SET diminfo=mdsys.sdo_dim_array("
                                        "mdsys.sdo_dim_element('X', ?, ?, 0.001),"
                                        "mdsys.sdo_dim_element('Y', ?, ?, 0.001)"
                                        ") WHERE table_name=? AND column_name=?" ) };

    {

      if ( !LoggedExecStatic( qry, sql,
                              QVariantList() << r.xMinimum() << r.xMaximum() << r.yMinimum() << r.yMaximum() << mTableName << mGeometryColumn, mUri.uri() )
         )
      {
        QgsMessageLog::logMessage( tr( "Could not update metadata for %1.%2.\nSQL: %3\nError: %4" )
                                   .arg( mTableName )
                                   .arg( mGeometryColumn )
                                   .arg( qry.lastQuery() )
                                   .arg( qry.lastError().text() ),
                                   tr( "Oracle" ) );
        return false;
      }

    }

    if ( qry.numRowsAffected() == 0 )
    {

      const QString sql { QStringLiteral( "INSERT INTO mdsys.user_sdo_geom_metadata(table_name,column_name,srid,diminfo) VALUES (?,?,?,mdsys.sdo_dim_array("
                                          "mdsys.sdo_dim_element('X', ?, ?, 0.001),"
                                          "mdsys.sdo_dim_element('Y', ?, ?, 0.001)"
                                          "))" ) };


      if ( !LoggedExecStatic( qry, sql,
                              QVariantList() << mTableName << mGeometryColumn << ( mSrid < 1 ? QVariant( QVariant::Int ) : mSrid )
                              << r.xMinimum() << r.xMaximum() << r.yMinimum() << r.yMaximum(), mUri.uri() )
         )
      {
        QgsMessageLog::logMessage( tr( "Could not insert metadata for %1.%2.\nSQL: %3\nError: %4" )
                                   .arg( quotedValue( mTableName ) )
                                   .arg( quotedValue( mGeometryColumn ) )
                                   .arg( qry.lastQuery() )
                                   .arg( qry.lastError().text() ),
                                   tr( "Oracle" ) );
        return false;
      }
    }
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "geographic CRS" ), 2 );
  }

  if ( !mHasSpatialIndex )
  {
    const QString spatialIndexName = conn->createSpatialIndex( mOwnerName, mTableName, mGeometryColumn );
    if ( spatialIndexName.isEmpty() )
      return false;
    else
      mSpatialIndexName = spatialIndexName;
  }
  else
  {

    const QString sql { QStringLiteral( "ALTER INDEX %1 REBUILD" ).arg( mSpatialIndexName ) };

    if ( !LoggedExecStatic( qry, sql, QVariantList(), mUri.uri() ) )
    {
      QgsMessageLog::logMessage( tr( "Rebuild of spatial index failed.\nSQL: %1\nError: %2" )
                                 .arg( qry.lastQuery() )
                                 .arg( qry.lastError().text() ),
                                 tr( "Oracle" ) );
      return false;
    }
  }

  return true;
}

bool QgsOracleProvider::convertField( QgsField &field )
{
  QString fieldType = "VARCHAR2(2047)"; //default to string
  int fieldSize = field.length();
  int fieldPrec = field.precision();
  switch ( field.type() )
  {
    case QVariant::LongLong:
      fieldType = "NUMBER(20,0)";
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QVariant::DateTime:
      fieldType = "TIMESTAMP";
      fieldPrec = 0;
      break;


    case QVariant::Time:
    case QVariant::String:
      fieldType = "VARCHAR2(2047)";
      fieldPrec = 0;
      break;

    case QVariant::Date:
      fieldType = "DATE";
      fieldPrec = 0;
      break;

    case QVariant::Int:
      fieldType = "NUMBER(10,0)";
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QVariant::Double:
      if ( fieldSize <= 0 || fieldPrec <= 0 )
      {
        fieldType = "BINARY_DOUBLE";
        fieldSize = -1;
        fieldPrec = 0;
      }
      else
      {
        fieldType = QString( "NUMBER(%1,%2)" ).arg( fieldSize ).arg( fieldPrec );
      }
      break;

    default:
      return false;
  }

  field.setTypeName( fieldType );
  field.setLength( fieldSize );
  field.setPrecision( fieldPrec );
  return true;
}


Qgis::VectorExportResult QgsOracleProvider::createEmptyLayer(
  const QString &uri,
  const QgsFields &fields,
  QgsWkbTypes::Type wkbType,
  const QgsCoordinateReferenceSystem &srs,
  bool overwrite,
  QMap<int, int> &oldToNewAttrIdxMap,
  QString &errorMessage,
  const QMap<QString, QVariant> *options )
{
  Q_UNUSED( wkbType )
  Q_UNUSED( options )

  // populate members from the uri structure
  QgsDataSourceUri dsUri( uri );
  QString ownerName = dsUri.schema();

  QgsDebugMsgLevel( QStringLiteral( "Connection info is: %1" ).arg( dsUri.connectionInfo( false ) ), 2 );

  // create the table
  QgsOracleConn *conn = QgsOracleConn::connectDb( dsUri, false );
  if ( !conn )
  {
    errorMessage = QObject::tr( "Connection to database failed" );
    return Qgis::VectorExportResult::ErrorConnectionFailed;
  }

  if ( ownerName.isEmpty() )
  {
    ownerName = conn->currentUser();
  }

  if ( ownerName.isEmpty() )
  {
    errorMessage = QObject::tr( "No owner name found" );
    return Qgis::VectorExportResult::ErrorInvalidLayer;
  }

  QString tableName = dsUri.table();
  QString geometryColumn = dsUri.geometryColumn();

  QString primaryKey = dsUri.keyColumn();
  QString primaryKeyType;

  QString ownerTableName = quotedIdentifier( ownerName ) + "." + quotedIdentifier( tableName );

  QgsDebugMsgLevel( QStringLiteral( "Geometry column is: %1" ).arg( geometryColumn ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Owner is: %1" ).arg( ownerName ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Table name is: %1" ).arg( tableName ), 2 );

  if ( geometryColumn.isEmpty() && fields.isEmpty() )
  {
    errorMessage = QObject::tr( "Cannot create a table with no columns" );
    return Qgis::VectorExportResult::ErrorCreatingDataSource;
  }

  // get the pk's name and type

  // if no pk name was passed, define the new pk field name
  if ( primaryKey.isEmpty() )
  {
    int index = 0;
    QString pk = primaryKey = "id";
    while ( fields.indexFromName( primaryKey ) >= 0 )
    {
      primaryKey = QString( "%1_%2" ).arg( pk ).arg( index++ );
    }
  }
  else
  {
    int idx = fields.indexFromName( primaryKey );
    if ( idx >= 0 )
    {
      QgsField fld = fields.at( idx );
      if ( ( options && options->value( QStringLiteral( "skipConvertFields" ), false ).toBool() ) || convertField( fld ) )
      {
        primaryKeyType = fld.typeName();
      }
    }
  }

  QSqlDatabase db( *conn );
  QSqlQuery qry( db );
  bool created = false;

  const bool hasPrimaryKey = !primaryKey.isEmpty() && !primaryKeyType.isEmpty();

  // It's not possible to create a table without any column, so we add a fake one
  // if needed, and will remove it later
  const QString fakeColumn = geometryColumn.isEmpty() && !hasPrimaryKey ?
                             QString( "fake_column_%1" ).arg( QUuid::createUuid().toString() ) : QString();

  try
  {
    if ( !conn->begin( db ) )
    {
      throw OracleException( tr( "Could not start transaction" ), db );
    }

    {
      const QString sql { QStringLiteral( "SELECT 1 FROM all_tables WHERE owner=? AND table_name=?" ) };

      if ( !LoggedExecStatic( qry, sql,
                              QVariantList() << ownerName << tableName, uri ) )
      {
        throw OracleException( tr( "Could not determine table existence." ), qry );
      }

    }

    bool exists = qry.next();

    if ( exists )
    {
      if ( overwrite )
      {
        // delete the table if exists, then re-create it

        const QString sql { QStringLiteral( "DROP TABLE %1" ).arg( ownerTableName ) };


        if ( !LoggedExecStatic( qry, sql, QVariantList(), uri ) )
        {
          throw OracleException( tr( "Table %1 could not be dropped." ).arg( ownerTableName ), qry );
        }
      }
      else
      {
        throw OracleException( tr( "Table %1 already exists." ).arg( ownerTableName ), qry );
      }
    }

    QString sql = QStringLiteral( "CREATE TABLE %1(" ).arg( ownerTableName );
    QString delim;

    if ( hasPrimaryKey )
    {
      sql += QString( "%1 %2 PRIMARY KEY" ).arg( quotedIdentifier( primaryKey ) ).arg( primaryKeyType );
      delim = ",";
    }

    // create geometry column
    if ( !geometryColumn.isEmpty() )
      sql += QString( "%1%2 MDSYS.SDO_GEOMETRY" ).arg( delim ).arg( quotedIdentifier( geometryColumn ) );

    if ( !fakeColumn.isEmpty() )
      sql += QString( "\"%1\" INTEGER" ).arg( fakeColumn );

    sql += ")";

    if ( !LoggedExecStatic( qry, sql, QVariantList(), uri ) )
    {
      throw OracleException( tr( "Table creation failed." ), qry );
    }

    created = true;

    if ( !geometryColumn.isEmpty() )
      insertGeomMetadata( conn, tableName, geometryColumn, srs );

    if ( !conn->commit( db ) )
    {
      QgsMessageLog::logMessage( tr( "Could not commit transaction" ), tr( "Oracle" ) );
    }
  }
  catch ( OracleException &e )
  {
    errorMessage = QObject::tr( "Creation of data source %1 failed: \n%2" )
                   .arg( ownerTableName )
                   .arg( e.errorMessage() );

    if ( !conn->rollback( db ) )
    {
      QgsMessageLog::logMessage( tr( "Could not rollback transaction" ), tr( "Oracle" ) );
    }

    if ( created )
    {

      const QString sql { QStringLiteral( "DROP TABLE %1" ).arg( ownerTableName ) };

      if ( !LoggedExecStatic( qry, sql, QVariantList(), uri ) )
      {
        QgsMessageLog::logMessage( tr( "Drop created table %1 failed.\nSQL: %2\nError: %3" )
                                   .arg( qry.lastQuery() )
                                   .arg( qry.lastError().text() ), tr( "Oracle" ) );
      }
    }

    conn->disconnect();

    return Qgis::VectorExportResult::ErrorCreatingLayer;
  }

  conn->disconnect();

  QgsDebugMsgLevel( QStringLiteral( "layer %1 created" ).arg( ownerTableName ), 2 );

  // use the provider to edit the table1
  dsUri.setDataSource( ownerName, tableName, geometryColumn, QString(), primaryKey );

  // provider cannot guess the type when there is no feature, so force the wkbtype in the uri
  dsUri.setWkbType( wkbType );

  QgsDataProvider::ProviderOptions providerOptions;
  std::unique_ptr<QgsOracleProvider> provider( new QgsOracleProvider( dsUri.uri( false ), providerOptions ) );
  if ( !provider->isValid() )
  {
    errorMessage = QObject::tr( "Loading of the layer %1 failed" ).arg( ownerTableName );
    return Qgis::VectorExportResult::ErrorInvalidLayer;
  }

  QgsDebugMsgLevel( QStringLiteral( "layer loaded" ), 2 );

  // add fields to the layer
  oldToNewAttrIdxMap.clear();

  if ( fields.size() > 0 )
  {
    int offset = 0;

    QSet<QString> names;
    QList<QgsField> launderedFields;
    for ( int i = 0; i < fields.size(); i++ )
    {
      QgsField fld = fields.at( i );

      QString name = fld.name();

      if ( names.contains( name ) )
      {
        int j;
        int n;
        for ( j = 1, n = 10; j < 3; j++, n *= 10 )
        {
          int k;
          for ( k = 0; k < n && names.contains( name ); k++ )
          {
            name = QString( "%1%2" ).arg( name.left( 30 - j ) ).arg( k, j, 10, QChar( '0' ) );
          }

          if ( k < n )
            break;
        }

        if ( j == 3 )
        {
          errorMessage = QObject::tr( "Field name clash found (%1 not remappable)" ).arg( fld.name() );
          return Qgis::VectorExportResult::ErrorAttributeTypeUnsupported;
        }
      }

      if ( fld.name() == geometryColumn )
        geometryColumn = name;

      fld.setName( name );
      launderedFields << fld;
      names << name;
    }

    // get the list of fields
    QList<QgsField> flist;
    for ( int i = 0; i < launderedFields.size(); i++ )
    {
      QgsField fld = launderedFields[i];

      if ( fld.name() == primaryKey )
      {
        oldToNewAttrIdxMap.insert( i, 0 );
        continue;
      }

      if ( fld.name() == geometryColumn )
      {
        QgsDebugMsgLevel( QStringLiteral( "Found a field with the same name of the geometry column. Skip it!" ), 2 );
        continue;
      }

      if ( !( options && options->value( QStringLiteral( "skipConvertFields" ), false ).toBool() ) && ! convertField( fld ) )
      {
        errorMessage = QObject::tr( "Unsupported type for field %1" ).arg( fld.name() );
        return Qgis::VectorExportResult::ErrorAttributeTypeUnsupported;
      }

      QgsDebugMsgLevel( QStringLiteral( "Field #%1 name %2 type %3 typename %4 width %5 precision %6" )
                        .arg( i )
                        .arg( fld.name() ).arg( QVariant::typeToName( fld.type() ) ).arg( fld.typeName() )
                        .arg( fld.length() ).arg( fld.precision() ), 2 );

      flist.append( fld );
      oldToNewAttrIdxMap.insert( i, offset++ );
    }

    if ( !provider->addAttributes( flist ) )
    {
      errorMessage = QObject::tr( "Creation of fields failed" );
      return Qgis::VectorExportResult::ErrorAttributeCreationFailed;
    }

    QgsDebugMsgLevel( QStringLiteral( "Done creating fields" ), 2 );
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "No fields created." ), 2 );
  }

  if ( !fakeColumn.isEmpty()
       && !provider->deleteAttributes( QgsAttributeIds { provider->fields().indexOf( fakeColumn ) } ) )
  {
    errorMessage = QObject::tr( "Remove of temporary column '%1' failed" ).arg( fakeColumn );
    return Qgis::VectorExportResult::ErrorAttributeCreationFailed;
  }

  return Qgis::VectorExportResult::Success;
}

void QgsOracleProvider::insertGeomMetadata( QgsOracleConn *conn, const QString &tableName, const QString &geometryColumn, const QgsCoordinateReferenceSystem &srs )
{
  QSqlDatabase db( *conn );
  QSqlQuery qry( db );

  // TODO: make precision configurable
  QString diminfo;
  if ( srs.isGeographic() )
  {
    diminfo = "mdsys.sdo_dim_array("
              "mdsys.sdo_dim_element('Longitude', -180, 180, 0.001),"
              "mdsys.sdo_dim_element('Latitude', -90, 90, 0.001)"
              ")";
  }
  else
  {
    diminfo = "mdsys.sdo_dim_array("
              "mdsys.sdo_dim_element('X', NULL, NULL, 0.001),"
              "mdsys.sdo_dim_element('Y', NULL, NULL, 0.001)"
              ")";
  }

  int srid = 0;
  QStringList parts = srs.authid().split( ":" );
  if ( parts.size() == 2 )
  {
    const int id = parts[1].toInt();
    QgsCoordinateReferenceSystem crs = lookupCrs( conn, id );
    if ( crs == srs )
      srid = id;
  }

  if ( srid == 0 )
  {
    QgsDebugMsgLevel( QStringLiteral( "%1:%2 not found in mdsys.cs_srs - trying WKT" ).arg( parts[0] ).arg( parts[1] ), 2 );

    QString wkt = srs.toWkt();

    const QString sql { QStringLiteral( "SELECT srid FROM mdsys.cs_srs WHERE wktext=?" ) };

    {

      if ( !LoggedExecStatic( qry, sql, QVariantList() << wkt, conn->connInfo() ) )
      {
        throw OracleException( tr( "Could not lookup WKT." ), qry );
      }

    }

    if ( qry.next() )
    {
      srid = qry.value( 0 ).toInt();
    }
    else
    {

      {
        const QString sql { QStringLiteral( "SELECT max(srid)+1 FROM sdo_coord_ref_system" ) };

        if ( !LoggedExecStatic( qry, sql, QVariantList(), conn->connInfo() ) || !qry.next() )
        {
          throw OracleException( tr( "Could not determine new srid." ), qry );
        }
      }

      srid = qry.value( 0 ).toInt();

      const QString sql { QStringLiteral( "INSERT"
                                          " INTO sdo_coord_ref_system(srid,coord_ref_sys_name,coord_ref_sys_kind,legacy_wktext,is_valid,is_legacy,information_source)"
                                          " VALUES (?,?,?,?,'TRUE','TRUE','GDAL/OGR via QGIS')" ) };


      if ( !LoggedExecStatic( qry, sql,
                              QVariantList() << srid << srs.description() << ( srs.isGeographic() ? "GEOGRAPHIC2D" : "PROJECTED" ) << wkt, conn->connInfo() ) )
      {
        throw OracleException( tr( "CRS not found and could not be created." ), qry );
      }
    }
  }

  if ( tableName.toUpper() != tableName || geometryColumn.toUpper() != geometryColumn )
    throw OracleException( tr( "Cannot insert geometry metadata for table '%1' and geometry column '%2'. Both needs to be uppercase" ).arg(
                             tableName, geometryColumn ), qry );

  const QString sql { QStringLiteral( "INSERT INTO mdsys.user_sdo_geom_metadata(table_name,column_name,srid,diminfo) VALUES (?,?,?,%1)" ).arg( diminfo ) };

  if ( !LoggedExecStatic( qry, sql,
                          QVariantList() << tableName.toUpper() << geometryColumn.toUpper() << srid, conn->connInfo() ) )
  {
    throw OracleException( tr( "Could not insert metadata." ), qry );
  }
}


QgsCoordinateReferenceSystem QgsOracleProvider::lookupCrs( QgsOracleConn *conn, int srsid )
{
  QgsCoordinateReferenceSystem srs;
  QSqlQuery qry( *conn );

  // apparently some EPSG codes don't have the auth_name setup in cs_srs
  if ( LoggedExecStatic( qry, QString( "SELECT coalesce(auth_name,'EPSG'),auth_srid,wktext FROM mdsys.cs_srs WHERE srid=?" ), QVariantList() << srsid, conn->connInfo() ) )
  {
    if ( qry.next() )
    {
      if ( qry.value( 0 ).toString() == "EPSG" )
      {
        srs.createFromOgcWmsCrs( QString( "EPSG:%1" ).arg( qry.value( 1 ).toString() ) );
      }
      else
      {
        srs.createFromWkt( qry.value( 2 ).toString() );
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Oracle SRID %1 not found." ).arg( srsid ), tr( "Oracle" ) );
    }
  }
  else
  {
    QgsMessageLog::logMessage( tr( "Lookup of Oracle SRID %1 failed.\nSQL: %2\nError: %3" )
                               .arg( srsid )
                               .arg( qry.lastQuery() )
                               .arg( qry.lastError().text() ),
                               tr( "Oracle" ) );
  }

  return srs;
}

QgsCoordinateReferenceSystem QgsOracleProvider::crs() const
{
  QgsOracleConn *conn = connectionRO();
  if ( !conn )
    return QgsCoordinateReferenceSystem();

  return lookupCrs( conn, mSrid );
}

QString QgsOracleProvider::subsetString() const
{
  return mSqlWhereClause;
}

QString QgsOracleProvider::getTableName()
{
  return mTableName;
}

size_t QgsOracleProvider::layerCount() const
{
  return 1;                   // XXX need to return actual number of layers
} // QgsOracleProvider::layerCount()


QString  QgsOracleProvider::name() const
{
  return ORACLE_KEY;
} //  QgsOracleProvider::name()

QString  QgsOracleProvider::description() const
{
  return ORACLE_DESCRIPTION;
} //  QgsOracleProvider::description()


QgsOracleProvider *QgsOracleProviderMetadata::createProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  QgsDataProvider::ReadFlags flags )
{
  return new QgsOracleProvider( uri, options, flags );
}

QList< QgsDataItemProvider * > QgsOracleProviderMetadata::dataItemProviders() const
{
  return QList< QgsDataItemProvider * >() << new QgsOracleDataItemProvider;
}

QgsTransaction *QgsOracleProviderMetadata::createTransaction( const QString &connString )
{
  return new QgsOracleTransaction( connString );
}

// ---------------------------------------------------------------------------

Qgis::VectorExportResult QgsOracleProviderMetadata::createEmptyLayer( const QString &uri,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    QMap<int, int> &oldToNewAttrIdxMap,
    QString &errorMessage,
    const QMap<QString, QVariant> *options )
{
  return QgsOracleProvider::createEmptyLayer(
           uri, fields, wkbType, srs, overwrite,
           oldToNewAttrIdxMap, errorMessage, options
         );
}

void QgsOracleProviderMetadata::cleanupProvider()
{
  QgsOracleConnPool::cleanupInstance();
}

// ----------

QgsFeatureId QgsOracleSharedData::lookupFid( const QVariantList &v )
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

QVariant QgsOracleSharedData::removeFid( QgsFeatureId fid )
{
  QMutexLocker locker( &mMutex );

  QVariantList v = mFidToKey[ fid ];
  mFidToKey.remove( fid );
  mKeyToFid.remove( v );
  return v;
}

void QgsOracleSharedData::insertFid( QgsFeatureId fid, const QVariantList &k )
{
  QMutexLocker locker( &mMutex );

  mFidToKey.insert( fid, k );
  mKeyToFid.insert( k, fid );
}

QVariantList QgsOracleSharedData::lookupKey( QgsFeatureId featureId )
{
  QMutexLocker locker( &mMutex );

  QMap<QgsFeatureId, QVariantList>::const_iterator it = mFidToKey.find( featureId );
  if ( it != mFidToKey.constEnd() )
    return it.value();
  return QVariantList();
}

bool QgsOracleProviderMetadata::styleExists( const QString &uri, const QString &styleId, QString &errorCause )
{
  errorCause.clear();

  QgsDataSourceUri dsUri( uri );

  QgsOracleConn *conn = QgsOracleConn::connectDb( dsUri, false );
  if ( !conn )
  {
    errorCause = QObject::tr( "Could not connect to database" );
    return false;
  }

  QSqlQuery qry = QSqlQuery( *conn );
  if ( !qry.exec( "SELECT COUNT(*) FROM user_tables WHERE table_name='LAYER_STYLES'" ) || !qry.next() )
  {
    errorCause = QObject::tr( "Unable to check layer style existence [%1]" ).arg( qry.lastError().text() );
    conn->disconnect();
    return false;
  }
  else if ( qry.value( 0 ).toInt() == 0 )
  {
    // layer styles table does not exist
    conn->disconnect();
    return false;
  }

  if ( !qry.prepare( "SELECT id,stylename FROM layer_styles"
                     " WHERE f_table_catalog=?"
                     " AND f_table_schema=?"
                     " AND f_table_name=?"
                     " AND f_geometry_column" +
                     QString( dsUri.geometryColumn().isEmpty() ? " IS NULL" : "=?" ) +
                     " AND styleName=?" ) )
  {
    errorCause = QObject::tr( "Could not prepare select [%1]" ).arg( qry.lastError().text() );
    conn->disconnect();
    return false;
  }
  qry.addBindValue( dsUri.database() );
  qry.addBindValue( dsUri.schema() );
  qry.addBindValue( dsUri.table() );
  if ( !dsUri.geometryColumn().isEmpty() )
    qry.addBindValue( dsUri.geometryColumn() );
  qry.addBindValue( styleId.isEmpty() ? dsUri.table() : styleId );
  if ( !qry.exec() )
  {
    errorCause = QObject::tr( "Unable to check style existence [%1]" ).arg( qry.lastError().text() );
    conn->disconnect();
    return false;
  }
  else if ( qry.next() )
  {
    conn->disconnect();
    return true;
  }
  else
  {
    conn->disconnect();
    return false;
  }
}

bool QgsOracleProviderMetadata::saveStyle( const QString &uri,
    const QString &qmlStyle,
    const QString &sldStyle,
    const QString &styleName,
    const QString &styleDescription,
    const QString &uiFileContent,
    bool useAsDefault,
    QString &errCause )
{
  QgsDataSourceUri dsUri( uri );

  QgsOracleConn *conn = QgsOracleConn::connectDb( dsUri, false );
  if ( !conn )
  {
    errCause = QObject::tr( "Could not connect to database" );
    return false;
  }

  QSqlQuery qry = QSqlQuery( *conn );
  if ( !qry.exec( "SELECT COUNT(*) FROM user_tables WHERE table_name='LAYER_STYLES'" ) || !qry.next() )
  {
    errCause = QObject::tr( "Unable to check layer style existence [%1]" ).arg( qry.lastError().text() );
    conn->disconnect();
    return false;
  }
  else if ( qry.value( 0 ).toInt() == 0 )
  {
    QgsDebugMsgLevel( QStringLiteral( "Creating layer style table." ), 2 );

    if ( !qry.exec( "CREATE TABLE layer_styles("
                    "id INTEGER PRIMARY KEY,"
                    "f_table_catalog VARCHAR2(30) NOT NULL,"
                    "f_table_schema VARCHAR2(30) NOT NULL,"
                    "f_table_name VARCHAR2(30) NOT NULL,"
                    "f_geometry_column VARCHAR2(30),"
                    "stylename VARCHAR2(2047),"
                    "styleqml CLOB,"
                    "stylesld CLOB,"
                    "useasdefault INTEGER,"
                    "description VARCHAR2(2047),"
                    "owner VARCHAR2(30),"
                    "ui CLOB,"
                    "update_time timestamp"
                    ")" ) )
    {
      errCause = QObject::tr( "Unable to create layer style table [%1]" ).arg( qry.lastError().text() );
      conn->disconnect();
      return false;
    }
  }

  int id;
  QString sql;
  if ( !qry.prepare( "SELECT id,stylename FROM layer_styles"
                     " WHERE f_table_catalog=?"
                     " AND f_table_schema=?"
                     " AND f_table_name=?"
                     " AND f_geometry_column" +
                     QString( dsUri.geometryColumn().isEmpty() ? " IS NULL" : "=?" ) +
                     " AND styleName=?" ) )
  {
    errCause = QObject::tr( "Could not prepare select [%1]" ).arg( qry.lastError().text() );
    conn->disconnect();
    return false;
  }
  qry.addBindValue( dsUri.database() );
  qry.addBindValue( dsUri.schema() );
  qry.addBindValue( dsUri.table() );
  if ( !dsUri.geometryColumn().isEmpty() )
    qry.addBindValue( dsUri.geometryColumn() );
  qry.addBindValue( styleName.isEmpty() ? dsUri.table() : styleName );
  if ( !qry.exec() )
  {
    errCause = QObject::tr( "Unable to check style existence [%1]" ).arg( qry.lastError().text() );
    conn->disconnect();
    return false;
  }
  else if ( qry.next() )
  {
    id = qry.value( 0 ).toInt();

    sql = QString( "UPDATE layer_styles"
                   " SET update_time=(select current_timestamp from dual),"
                   "f_table_catalog=?,"
                   "f_table_schema=?,"
                   "f_table_name=?,"
                   "f_geometry_column=?,"
                   "styleName=?,"
                   "styleQML=?,"
                   "styleSLD=?,"
                   "useAsDefault=?,"
                   "description=?,"
                   "owner=?"
                   "%1"
                   " WHERE id=%2" )
          .arg( uiFileContent.isEmpty() ? "" : ",ui=?" )
          .arg( id );
  }
  else if ( qry.exec( "select coalesce(max(id)+1,0) FROM layer_styles" ) && qry.next() )
  {
    id = qry.value( 0 ).toInt();

    sql = QString( "INSERT INTO layer_styles("
                   "id,update_time,f_table_catalog,f_table_schema,f_table_name,f_geometry_column,styleName,styleQML,styleSLD,useAsDefault,description,owner%1"
                   ") VALUES ("
                   "%2,"
                   "(select current_timestamp from dual)"
                   "%3"
                   ")" )
          .arg( uiFileContent.isEmpty() ? "" : ",ui" )
          .arg( id )
          .arg( QString( ",?" ).repeated( uiFileContent.isEmpty() ? 10 : 11 ) );
  }
  else
  {
    errCause = QObject::tr( "Cannot fetch new layer style id." );
    conn->disconnect();
    return false;
  }

  if ( !qry.prepare( sql ) )
  {
    errCause = QObject::tr( "Could not prepare insert/update [%1]" ).arg( qry.lastError().text() );
    QgsDebugMsg( QStringLiteral( "prepare insert/update failed" ) );
    conn->disconnect();
    return false;
  }

  qry.addBindValue( dsUri.database() );
  qry.addBindValue( dsUri.schema() );
  qry.addBindValue( dsUri.table() );
  qry.addBindValue( dsUri.geometryColumn() );
  qry.addBindValue( styleName.isEmpty() ? dsUri.table() : styleName );
  qry.addBindValue( qmlStyle );
  qry.addBindValue( sldStyle );
  qry.addBindValue( useAsDefault ? 1 : 0 );
  qry.addBindValue( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription );
  qry.addBindValue( dsUri.username() );
  if ( !uiFileContent.isEmpty() )
    qry.addBindValue( uiFileContent );

  if ( !qry.exec() )
  {
    errCause = QObject::tr( "Could not execute insert/update [%1]" ).arg( qry.lastError().text() );
    QgsDebugMsg( QStringLiteral( "execute insert/update failed" ) );
    conn->disconnect();
    return false;
  }

  if ( useAsDefault )
  {
    if ( !qry.prepare( QStringLiteral( "UPDATE layer_styles"
                                       " SET useasdefault=0,update_time=(select current_timestamp from dual)"
                                       " WHERE f_table_catalog=?"
                                       " AND f_table_schema=?"
                                       " AND f_table_name=?"
                                       " AND f_geometry_column=?"
                                       " AND id<>?" ) ) ||
         !(
           qry.addBindValue( dsUri.database() ),
           qry.addBindValue( dsUri.schema() ),
           qry.addBindValue( dsUri.table() ),
           qry.addBindValue( dsUri.geometryColumn() ),
           qry.addBindValue( id ),
           qry.exec()
         ) )
    {
      errCause = QObject::tr( "Could not reset default status [%1]" ).arg( qry.lastError().text() );
      QgsDebugMsg( QStringLiteral( "execute update failed" ) );
      conn->disconnect();
      return false;
    }
  }

  conn->disconnect();

  return true;
}

QString QgsOracleProviderMetadata::loadStyle( const QString &uri, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );

  QgsOracleConn *conn = QgsOracleConn::connectDb( dsUri, false );
  if ( !conn )
  {
    errCause = QObject::tr( "Could not connect to database" );
    return QString();
  }

  QSqlQuery qry( *conn );

  QString style;
  if ( !qry.exec( "SELECT COUNT(*) FROM user_tables WHERE table_name='LAYER_STYLES'" ) || !qry.next() || qry.value( 0 ).toInt() == 0 )
  {
    errCause = QObject::tr( "Unable to find layer style table [%1]" ).arg( qry.lastError().text() );
    conn->disconnect();
    return QString();
  }

  if ( !qry.prepare( "SELECT styleQML FROM ("
                     "SELECT styleQML"
                     " FROM layer_styles"
                     " WHERE f_table_catalog=?"
                     " AND f_table_schema=?"
                     " AND f_table_name=?"
                     " AND f_geometry_column" +
                     QString( dsUri.geometryColumn().isEmpty() ? " IS NULL" : "=?" ) +
                     " ORDER BY useAsDefault DESC"
                     ") WHERE rownum=1" ) )
  {
    errCause = QObject::tr( "Could not prepare select [%1]" ).arg( qry.lastError().text() );
    conn->disconnect();
    return QString();
  }
  qry.addBindValue( dsUri.database() );
  qry.addBindValue( dsUri.schema() );
  qry.addBindValue( dsUri.table() );
  if ( !dsUri.geometryColumn().isEmpty() )
    qry.addBindValue( dsUri.geometryColumn() );

  if ( !qry.exec() )
  {
    errCause = QObject::tr( "Could not retrieve style [%1]" ).arg( qry.lastError().text() );
  }
  else if ( !qry.next() )
  {
    errCause = QObject::tr( "Style not found" );
  }
  else
  {
    style = qry.value( 0 ).toString();
  }

  conn->disconnect();

  return style;
}

int QgsOracleProviderMetadata::listStyles( const QString &uri,
    QStringList &ids,
    QStringList &names,
    QStringList &descriptions,
    QString &errCause )
{
  QgsDataSourceUri dsUri( uri );

  QgsOracleConn *conn = QgsOracleConn::connectDb( dsUri, false );
  if ( !conn )
  {
    errCause = QObject::tr( "Could not connect to database" );
    return -1;
  }

  QSqlQuery qry( *conn );

  if ( !qry.exec( "SELECT count(*) FROM user_tables WHERE table_name='LAYER_STYLES'" ) || !qry.next() )
  {
    errCause = QObject::tr( "Could not verify existence of layer style table [%1]" ).arg( qry.lastError().text() );
    conn->disconnect();
    return -1;
  }
  if ( qry.value( 0 ).toInt() == 0 )
  {
    errCause = QObject::tr( "Layer style table does not exist [%1]" ).arg( qry.value( 0 ).toString() );
    conn->disconnect();
    return -1;
  }

  if ( !qry.prepare( "SELECT id,styleName,description FROM layer_styles WHERE f_table_catalog=? AND f_table_schema=? AND f_table_name=? AND f_geometry_column"
                     + QString( dsUri.geometryColumn().isEmpty() ? " IS NULL" : "=?" ) ) )
  {
    errCause = QObject::tr( "Could not prepare select [%1]" ).arg( qry.lastError().text() );
    conn->disconnect();
    return -1;
  }
  qry.addBindValue( dsUri.database() );
  qry.addBindValue( dsUri.schema() );
  qry.addBindValue( dsUri.table() );
  if ( !dsUri.geometryColumn().isEmpty() )
    qry.addBindValue( dsUri.geometryColumn() );
  if ( !qry.exec() )
  {
    conn->disconnect();
    errCause = QObject::tr( "No style for layer found" );
    return -1;
  }

  int res = 0;
  while ( qry.next() )
  {
    ids << qry.value( 0 ).toString();
    names << qry.value( 1 ).toString();
    descriptions << qry.value( 2 ).toString();
    res++;
  }

  qry.finish();

  if ( !qry.prepare( "SELECT id,styleName,description FROM layer_styles WHERE NOT (f_table_catalog=? AND f_table_schema=? AND f_table_name=? AND f_geometry_column"
                     + QString( dsUri.geometryColumn().isEmpty() ? " IS NULL" : "=?" )
                     + ") ORDER BY update_time DESC" ) )
  {
    errCause = QObject::tr( "Could not prepare select [%1]" ).arg( qry.lastError().text() );
    conn->disconnect();
    return -1;
  }
  qry.addBindValue( dsUri.database() );
  qry.addBindValue( dsUri.schema() );
  qry.addBindValue( dsUri.table() );
  if ( !dsUri.geometryColumn().isEmpty() )
    qry.addBindValue( dsUri.geometryColumn() );
  if ( qry.exec() )
  {
    while ( qry.next() )
    {
      ids << qry.value( 0 ).toString();
      names << qry.value( 1 ).toString();
      descriptions << qry.value( 2 ).toString();
    }
  }

  conn->disconnect();
  return res;
}

QString QgsOracleProviderMetadata::getStyleById( const QString &uri, const QString &styleId, QString &errCause )
{
  QString style;
  QgsDataSourceUri dsUri( uri );

  QgsOracleConn *conn = QgsOracleConn::connectDb( dsUri, false );
  if ( !conn )
  {
    errCause = QObject::tr( "Could not connect to database" );
    return style;
  }

  QSqlQuery qry( *conn );

  if ( !qry.prepare( QStringLiteral( "SELECT styleQml FROM layer_styles WHERE id=?" ) ) ||
       !(
         qry.addBindValue( styleId ),
         qry.exec() ) )
  {
    errCause = QObject::tr( "Could not load layer style table [%1]" ).arg( qry.lastError().text() );
  }
  else if ( !qry.next() )
  {
    errCause = QObject::tr( "No styles found in layer table [%1]" ).arg( qry.lastError().text() );
  }
  else
  {
    style = qry.value( 0 ).toString();
  }

  conn->disconnect();

  return style;
}


#ifdef HAVE_GUI

//! Provider for Oracle source select
class QgsOracleSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    QString providerKey() const override { return QStringLiteral( "oracle" ); }
    QString text() const override { return QObject::tr( "Oracle" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderDatabaseProvider + 40; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddOracleLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr,
        Qt::WindowFlags fl = Qt::Widget,
        QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsOracleSourceSelect( parent, fl, widgetMode );
    }
};

QgsOracleProviderGuiMetadata::QgsOracleProviderGuiMetadata()
  : QgsProviderGuiMetadata( ORACLE_KEY )
{

}

QList<QgsSourceSelectProvider *> QgsOracleProviderGuiMetadata::sourceSelectProviders()
{
  return QList<QgsSourceSelectProvider *>() << new QgsOracleSourceSelectProvider;
}

void QgsOracleProviderGuiMetadata::registerGui( QMainWindow *mainWindow )
{
  QgsOracleRootItem::sMainWindow = mainWindow;
}

#endif

QgsOracleProviderMetadata::QgsOracleProviderMetadata():
  QgsProviderMetadata( ORACLE_KEY, ORACLE_DESCRIPTION )
{
}

QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return QSqlDatabase::isDriverAvailable( "QOCISPATIAL" ) ? new QgsOracleProviderMetadata() : nullptr;
}

#ifdef HAVE_GUI
QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return QSqlDatabase::isDriverAvailable( "QOCISPATIAL" ) ? new QgsOracleProviderGuiMetadata() : nullptr;
}
#endif

QVariantMap QgsOracleProviderMetadata::decodeUri( const QString &uri ) const
{
  const QgsDataSourceUri dsUri { uri };
  QVariantMap uriParts;

  if ( ! dsUri.database().isEmpty() )
    uriParts[ QStringLiteral( "dbname" ) ] = dsUri.database();
  if ( ! dsUri.host().isEmpty() )
    uriParts[ QStringLiteral( "host" ) ] = dsUri.host();
  if ( ! dsUri.port().isEmpty() )
    uriParts[ QStringLiteral( "port" ) ] = dsUri.port();
  if ( ! dsUri.service().isEmpty() )
    uriParts[ QStringLiteral( "service" ) ] = dsUri.service();
  if ( ! dsUri.param( "dbworkspace" ).isEmpty() )
    uriParts[ QStringLiteral( "dbworkspace" ) ] = dsUri.param( "dbworkspace" );
  if ( ! dsUri.username().isEmpty() )
    uriParts[ QStringLiteral( "username" ) ] = dsUri.username();
  if ( ! dsUri.password().isEmpty() )
    uriParts[ QStringLiteral( "password" ) ] = dsUri.password();
  if ( ! dsUri.authConfigId().isEmpty() )
    uriParts[ QStringLiteral( "authcfg" ) ] = dsUri.authConfigId();
  if ( dsUri.wkbType() != QgsWkbTypes::Type::Unknown )
    uriParts[ QStringLiteral( "type" ) ] = dsUri.wkbType();
  if ( ! dsUri.table().isEmpty() )
    uriParts[ QStringLiteral( "table" ) ] = dsUri.table();
  if ( ! dsUri.schema().isEmpty() )
    uriParts[ QStringLiteral( "schema" ) ] = dsUri.schema();
  if ( ! dsUri.keyColumn().isEmpty() )
    uriParts[ QStringLiteral( "key" ) ] = dsUri.keyColumn();
  if ( ! dsUri.srid().isEmpty() )
    uriParts[ QStringLiteral( "srid" ) ] = dsUri.srid();
  if ( uri.contains( QStringLiteral( "estimatedmetadata=" ), Qt::CaseSensitivity::CaseInsensitive ) )
    uriParts[ QStringLiteral( "estimatedmetadata" ) ] = dsUri.useEstimatedMetadata();
  if ( ! dsUri.param( "includegeoattributes" ).isEmpty() )
    uriParts[ QStringLiteral( "includegeoattributes" ) ] = dsUri.param( "includegeoattributes" );
  if ( ! dsUri.sql().isEmpty() )
    uriParts[ QStringLiteral( "sql" ) ] = dsUri.sql();
  if ( ! dsUri.param( "checkPrimaryKeyUnicity" ).isEmpty() )
    uriParts[ QStringLiteral( "checkPrimaryKeyUnicity" ) ] = dsUri.param( "checkPrimaryKeyUnicity" );
  if ( ! dsUri.geometryColumn().isEmpty() )
    uriParts[ QStringLiteral( "geometrycolumn" ) ] = dsUri.geometryColumn();
  if ( ! dsUri.param( "dboptions" ).isEmpty() )
    uriParts[ QStringLiteral( "dboptions" ) ] = dsUri.param( "dboptions" );

  return uriParts;
}

QString QgsOracleProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  if ( parts.contains( QStringLiteral( "dbname" ) ) )
    dsUri.setDatabase( parts.value( QStringLiteral( "dbname" ) ).toString() );
  if ( parts.contains( QStringLiteral( "host" ) ) )
    dsUri.setParam( QStringLiteral( "host" ), parts.value( QStringLiteral( "host" ) ).toString() );
  if ( parts.contains( QStringLiteral( "port" ) ) )
    dsUri.setParam( QStringLiteral( "port" ), parts.value( QStringLiteral( "port" ) ).toString() );
  if ( parts.contains( QStringLiteral( "service" ) ) )
    dsUri.setParam( QStringLiteral( "service" ), parts.value( QStringLiteral( "service" ) ).toString() );
  if ( parts.contains( QStringLiteral( "dbworkspace" ) ) )
    dsUri.setParam( QStringLiteral( "dbworkspace" ), parts.value( QStringLiteral( "dbworkspace" ) ).toString() );
  if ( parts.contains( QStringLiteral( "username" ) ) )
    dsUri.setUsername( parts.value( QStringLiteral( "username" ) ).toString() );
  if ( parts.contains( QStringLiteral( "password" ) ) )
    dsUri.setPassword( parts.value( QStringLiteral( "password" ) ).toString() );
  if ( parts.contains( QStringLiteral( "authcfg" ) ) )
    dsUri.setAuthConfigId( parts.value( QStringLiteral( "authcfg" ) ).toString() );
  if ( parts.contains( QStringLiteral( "type" ) ) )
    dsUri.setParam( QStringLiteral( "type" ), QgsWkbTypes::displayString( static_cast<QgsWkbTypes::Type>( parts.value( QStringLiteral( "type" ) ).toInt() ) ) );
  if ( parts.contains( QStringLiteral( "table" ) ) )
    dsUri.setTable( parts.value( QStringLiteral( "table" ) ).toString() );
  if ( parts.contains( QStringLiteral( "schema" ) ) )
    dsUri.setSchema( parts.value( QStringLiteral( "schema" ) ).toString() );
  if ( parts.contains( QStringLiteral( "key" ) ) )
    dsUri.setParam( QStringLiteral( "key" ), parts.value( QStringLiteral( "key" ) ).toString() );
  if ( parts.contains( QStringLiteral( "srid" ) ) )
    dsUri.setSrid( parts.value( QStringLiteral( "srid" ) ).toString() );
  if ( parts.contains( QStringLiteral( "estimatedmetadata" ) ) )
    dsUri.setParam( QStringLiteral( "estimatedmetadata" ), parts.value( QStringLiteral( "estimatedmetadata" ) ).toString() );
  if ( parts.contains( QStringLiteral( "includegeoattributes" ) ) )
    dsUri.setParam( QStringLiteral( "includegeoattributes" ), parts.value( QStringLiteral( "includegeoattributes" ) ).toString() );
  if ( parts.contains( QStringLiteral( "sql" ) ) )
    dsUri.setSql( parts.value( QStringLiteral( "sql" ) ).toString() );
  if ( parts.contains( QStringLiteral( "checkPrimaryKeyUnicity" ) ) )
    dsUri.setParam( QStringLiteral( "checkPrimaryKeyUnicity" ), parts.value( QStringLiteral( "checkPrimaryKeyUnicity" ) ).toString() );
  if ( parts.contains( QStringLiteral( "geometrycolumn" ) ) )
    dsUri.setGeometryColumn( parts.value( QStringLiteral( "geometrycolumn" ) ).toString() );
  return dsUri.uri( false );
}

QMap<QString, QgsAbstractProviderConnection *> QgsOracleProviderMetadata::connections( bool cached )
{
  return connectionsProtected<QgsOracleProviderConnection, QgsOracleConn>( cached );
}

QgsAbstractProviderConnection *QgsOracleProviderMetadata::createConnection( const QString &uri, const QVariantMap &configuration )
{
  return new QgsOracleProviderConnection( uri, configuration );
}

QgsAbstractProviderConnection *QgsOracleProviderMetadata::createConnection( const QString &name )
{
  return new QgsOracleProviderConnection( name );
}

void QgsOracleProviderMetadata::deleteConnection( const QString &name )
{
  deleteConnectionProtected<QgsOracleProviderConnection>( name );
}

void QgsOracleProviderMetadata::saveConnection( const QgsAbstractProviderConnection *conn,  const QString &name )
{
  saveConnectionProtected( conn, name );
}

// vim: set sw=2
