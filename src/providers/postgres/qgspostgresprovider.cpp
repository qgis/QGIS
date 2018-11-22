/***************************************************************************
  qgspostgresprovider.cpp  -  QGIS data provider for PostgreSQL/PostGIS layers
                             -------------------
    begin                : 2004/01/07
    copyright            : (C) 2004 by Gary E.Sherman
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

#include "qgsapplication.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsmessageoutput.h"
#include "qgsmessagelog.h"
#include "qgsprojectstorageregistry.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsxmlutils.h"
#include "qgsvectorlayer.h"

#include <QMessageBox>

#include "qgsvectorlayerexporter.h"
#include "qgspostgresprovider.h"
#include "qgspostgresconn.h"
#include "qgspostgresconnpool.h"
#include "qgspostgresdataitems.h"
#include "qgspostgresfeatureiterator.h"
#include "qgspostgrestransaction.h"
#include "qgspostgreslistener.h"
#include "qgspostgresprojectstorage.h"
#include "qgslogger.h"
#include "qgsfeedback.h"
#include "qgssettings.h"

#ifdef HAVE_GUI
#include "qgspgsourceselect.h"
#include "qgssourceselectprovider.h"
#endif

const QString POSTGRES_KEY = QStringLiteral( "postgres" );
const QString POSTGRES_DESCRIPTION = QStringLiteral( "PostgreSQL/PostGIS data provider" );
static const QString EDITOR_WIDGET_STYLES_TABLE = QStringLiteral( "qgis_editor_widget_styles" );

inline qint64 PKINT2FID( qint32 x )
{
  return QgsPostgresUtils::int32pk_to_fid( x );
}

inline qint32 FID2PKINT( qint64 x )
{
  return QgsPostgresUtils::fid_to_int32pk( x );
}

static bool tableExists( QgsPostgresConn &conn, const QString &name )
{
  QgsPostgresResult res( conn.PQexec( "SELECT COUNT(*) FROM information_schema.tables WHERE table_name=" + QgsPostgresConn::quotedValue( name ) ) );
  return res.PQgetvalue( 0, 0 ).toInt() > 0;
}

QgsPostgresPrimaryKeyType
QgsPostgresProvider::pkType( const QgsField &f ) const
{
  switch ( f.type() )
  {
    case QVariant::LongLong:
      // unless we can guarantee all values are unsigned
      // (in which case we could use pktUint64)
      // we'll have to use a Map type.
      // See https://issues.qgis.org/issues/14262
      return PktFidMap; // pktUint64

    case QVariant::Int:
      return PktInt;

    default:
      return PktFidMap;
  }
}



QgsPostgresProvider::QgsPostgresProvider( QString const &uri, const ProviderOptions &options )
  : QgsVectorDataProvider( uri, options )
  , mShared( new QgsPostgresSharedData )
{

  QgsDebugMsg( QStringLiteral( "URI: %1 " ).arg( uri ) );

  mUri = QgsDataSourceUri( uri );

  // populate members from the uri structure
  mSchemaName = mUri.schema();
  mTableName = mUri.table();
  mGeometryColumn = mUri.geometryColumn();
  mSqlWhereClause = mUri.sql();
  mRequestedSrid = mUri.srid();
  mRequestedGeomType = mUri.wkbType();

  if ( mUri.hasParam( QStringLiteral( "checkPrimaryKeyUnicity" ) ) )
  {
    if ( mUri.param( QStringLiteral( "checkPrimaryKeyUnicity" ) ).compare( QLatin1String( "0" ) )  == 0 )
    {
      mCheckPrimaryKeyUnicity = false;
    }
    else
    {
      mCheckPrimaryKeyUnicity = true;
    }
  }

  if ( mSchemaName.isEmpty() && mTableName.startsWith( '(' ) && mTableName.endsWith( ')' ) )
  {
    mIsQuery = true;
    mQuery = mTableName;
    mTableName.clear();
  }
  else
  {
    mIsQuery = false;

    if ( !mSchemaName.isEmpty() )
    {
      mQuery += quotedIdentifier( mSchemaName ) + '.';
    }

    if ( !mTableName.isEmpty() )
    {
      mQuery += quotedIdentifier( mTableName );
    }
  }

  mUseEstimatedMetadata = mUri.useEstimatedMetadata();
  mSelectAtIdDisabled = mUri.selectAtIdDisabled();

  QgsDebugMsg( QStringLiteral( "Connection info is %1" ).arg( mUri.connectionInfo( false ) ) );
  QgsDebugMsg( QStringLiteral( "Geometry column is: %1" ).arg( mGeometryColumn ) );
  QgsDebugMsg( QStringLiteral( "Schema is: %1" ).arg( mSchemaName ) );
  QgsDebugMsg( QStringLiteral( "Table name is: %1" ).arg( mTableName ) );
  QgsDebugMsg( QStringLiteral( "Query is: %1" ).arg( mQuery ) );
  QgsDebugMsg( QStringLiteral( "Where clause is: %1" ).arg( mSqlWhereClause ) );

  // no table/query passed, the provider could be used to get tables
  if ( mQuery.isEmpty() )
  {
    return;
  }

  mConnectionRO = QgsPostgresConn::connectDb( mUri.connectionInfo( false ), true );
  if ( !mConnectionRO )
  {
    return;
  }

  if ( !hasSufficientPermsAndCapabilities() ) // check permissions and set capabilities
  {
    disconnectDb();
    return;
  }

  if ( !getGeometryDetails() ) // gets srid, geometry and data type
  {
    // the table is not a geometry table
    QgsMessageLog::logMessage( tr( "invalid PostgreSQL layer" ), tr( "PostGIS" ) );
    disconnectDb();
    return;
  }

  // NOTE: mValid would be true after true return from
  // getGeometryDetails, see https://issues.qgis.org/issues/13781

  if ( mSpatialColType == SctTopoGeometry )
  {
    if ( !getTopoLayerInfo() ) // gets topology name and layer id
    {
      QgsMessageLog::logMessage( tr( "invalid PostgreSQL topology layer" ), tr( "PostGIS" ) );
      mValid = false;
      disconnectDb();
      return;
    }
  }

  mLayerExtent.setMinimal();

  // set the primary key
  if ( !determinePrimaryKey() )
  {
    QgsMessageLog::logMessage( tr( "PostgreSQL layer has no primary key." ), tr( "PostGIS" ) );
    mValid = false;
    disconnectDb();
    return;
  }

  // Set the PostgreSQL message level so that we don't get the
  // 'there is no transaction in progress' warning.
#ifndef QGISDEBUG
  mConnectionRO->PQexecNR( QStringLiteral( "set client_min_messages to error" ) );
#endif

  //fill type names into sets
  QList<NativeType> nativeTypes;

  nativeTypes     // integer types
      << QgsVectorDataProvider::NativeType( tr( "Whole number (smallint - 16bit)" ), QStringLiteral( "int2" ), QVariant::Int, -1, -1, 0, 0 )
      << QgsVectorDataProvider::NativeType( tr( "Whole number (integer - 32bit)" ), QStringLiteral( "int4" ), QVariant::Int, -1, -1, 0, 0 )
      << QgsVectorDataProvider::NativeType( tr( "Whole number (integer - 64bit)" ), QStringLiteral( "int8" ), QVariant::LongLong, -1, -1, 0, 0 )
      << QgsVectorDataProvider::NativeType( tr( "Decimal number (numeric)" ), QStringLiteral( "numeric" ), QVariant::Double, 1, 20, 0, 20 )
      << QgsVectorDataProvider::NativeType( tr( "Decimal number (decimal)" ), QStringLiteral( "decimal" ), QVariant::Double, 1, 20, 0, 20 )

      // floating point
      << QgsVectorDataProvider::NativeType( tr( "Decimal number (real)" ), QStringLiteral( "real" ), QVariant::Double, -1, -1, -1, -1 )
      << QgsVectorDataProvider::NativeType( tr( "Decimal number (double)" ), QStringLiteral( "double precision" ), QVariant::Double, -1, -1, -1, -1 )

      // string types
      << QgsVectorDataProvider::NativeType( tr( "Text, fixed length (char)" ), QStringLiteral( "char" ), QVariant::String, 1, 255, -1, -1 )
      << QgsVectorDataProvider::NativeType( tr( "Text, limited variable length (varchar)" ), QStringLiteral( "varchar" ), QVariant::String, 1, 255, -1, -1 )
      << QgsVectorDataProvider::NativeType( tr( "Text, unlimited length (text)" ), QStringLiteral( "text" ), QVariant::String, -1, -1, -1, -1 )

      // date type
      << QgsVectorDataProvider::NativeType( tr( "Date" ), QStringLiteral( "date" ), QVariant::Date, -1, -1, -1, -1 )
      << QgsVectorDataProvider::NativeType( tr( "Time" ), QStringLiteral( "time" ), QVariant::Time, -1, -1, -1, -1 )
      << QgsVectorDataProvider::NativeType( tr( "Date & Time" ), QStringLiteral( "timestamp without time zone" ), QVariant::DateTime, -1, -1, -1, -1 )

      // complex types
      << QgsVectorDataProvider::NativeType( tr( "Map (hstore)" ), QStringLiteral( "hstore" ), QVariant::Map, -1, -1, -1, -1, QVariant::String )
      << QgsVectorDataProvider::NativeType( tr( "Array of number (integer - 32bit)" ), QStringLiteral( "int4[]" ), QVariant::List, -1, -1, -1, -1, QVariant::Int )
      << QgsVectorDataProvider::NativeType( tr( "Array of number (integer - 64bit)" ), QStringLiteral( "int8[]" ), QVariant::List, -1, -1, -1, -1, QVariant::LongLong )
      << QgsVectorDataProvider::NativeType( tr( "Array of number (double)" ), QStringLiteral( "double precision[]" ), QVariant::List, -1, -1, -1, -1, QVariant::Double )
      << QgsVectorDataProvider::NativeType( tr( "Array of text" ), QStringLiteral( "text[]" ), QVariant::StringList, -1, -1, -1, -1, QVariant::String )

      // boolean
      << QgsVectorDataProvider::NativeType( tr( "Boolean" ), QStringLiteral( "bool" ), QVariant::Bool, -1, -1, -1, -1 )
      ;

  if ( connectionRO()->pgVersion() >= 90200 )
  {
    nativeTypes << QgsVectorDataProvider::NativeType( tr( "Map (json)" ), QStringLiteral( "json" ), QVariant::Map, -1, -1, -1, -1, QVariant::String );

    if ( connectionRO()->pgVersion() >= 90400 )
    {
      nativeTypes << QgsVectorDataProvider::NativeType( tr( "Map (jsonb)" ), QStringLiteral( "jsonb" ), QVariant::Map, -1, -1, -1, -1, QVariant::String );
    }
  }
  setNativeTypes( nativeTypes );

  QString key;
  switch ( mPrimaryKeyType )
  {
    case PktOid:
      key = QStringLiteral( "oid" );
      break;
    case PktTid:
      key = QStringLiteral( "tid" );
      break;
    case PktInt:
    case PktUint64:
      Q_ASSERT( mPrimaryKeyAttrs.size() == 1 );
      Q_ASSERT( mPrimaryKeyAttrs[0] >= 0 && mPrimaryKeyAttrs[0] < mAttributeFields.count() );
      key = mAttributeFields.at( mPrimaryKeyAttrs.at( 0 ) ).name();
      break;
    case PktFidMap:
    {
      QString delim;
      Q_FOREACH ( int idx, mPrimaryKeyAttrs )
      {
        key += delim + mAttributeFields.at( idx ).name();
        delim = ',';
      }
    }
    break;
    case PktUnknown:
      QgsMessageLog::logMessage( tr( "PostgreSQL layer has unknown primary key type." ), tr( "PostGIS" ) );
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

  mLayerMetadata.setType( QStringLiteral( "dataset" ) );
  mLayerMetadata.setCrs( crs() );
}

QgsPostgresProvider::~QgsPostgresProvider()
{
  disconnectDb();

  QgsDebugMsg( QStringLiteral( "deconstructing." ) );
}


QgsAbstractFeatureSource *QgsPostgresProvider::featureSource() const
{
  return new QgsPostgresFeatureSource( this );
}

QgsPostgresConn *QgsPostgresProvider::connectionRO() const
{
  return mTransaction ? mTransaction->connection() : mConnectionRO;
}

void QgsPostgresProvider::setListening( bool isListening )
{
  if ( isListening && !mListener )
  {
    mListener.reset( QgsPostgresListener::create( mUri.connectionInfo( false ) ).release() );
    connect( mListener.get(), &QgsPostgresListener::notify, this, &QgsPostgresProvider::notify );
  }
  else if ( !isListening && mListener )
  {
    disconnect( mListener.get(), &QgsPostgresListener::notify, this, &QgsPostgresProvider::notify );
    mListener.reset();
  }
}

QgsPostgresConn *QgsPostgresProvider::connectionRW()
{
  if ( mTransaction )
  {
    return mTransaction->connection();
  }
  else if ( !mConnectionRW )
  {
    mConnectionRW = QgsPostgresConn::connectDb( mUri.connectionInfo( false ), false );
  }
  return mConnectionRW;
}

QgsTransaction *QgsPostgresProvider::transaction() const
{
  return static_cast<QgsTransaction *>( mTransaction );
}

void QgsPostgresProvider::setTransaction( QgsTransaction *transaction )
{
  // static_cast since layers cannot be added to a transaction of a non-matching provider
  mTransaction = static_cast<QgsPostgresTransaction *>( transaction );
}

void QgsPostgresProvider::disconnectDb()
{
  if ( mConnectionRO )
  {
    mConnectionRO->unref();
    mConnectionRO = nullptr;
  }

  if ( mConnectionRW )
  {
    mConnectionRW->unref();
    mConnectionRW = nullptr;
  }
}

QString QgsPostgresProvider::storageType() const
{
  return QStringLiteral( "PostgreSQL database with PostGIS extension" );
}

QgsFeatureIterator QgsPostgresProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  if ( !mValid )
  {
    QgsMessageLog::logMessage( tr( "Read attempt on an invalid PostgreSQL data source" ), tr( "PostGIS" ) );
    return QgsFeatureIterator();
  }

  QgsPostgresFeatureSource *featureSrc = static_cast<QgsPostgresFeatureSource *>( featureSource() );
  return QgsFeatureIterator( new QgsPostgresFeatureIterator( featureSrc, true, request ) );
}



QString QgsPostgresProvider::pkParamWhereClause( int offset, const char *alias ) const
{
  QString whereClause;

  QString aliased;
  if ( alias ) aliased = QStringLiteral( "%1." ).arg( alias );

  switch ( mPrimaryKeyType )
  {
    case PktTid:
      whereClause = QStringLiteral( "%2ctid=$%1" ).arg( offset ).arg( aliased );
      break;

    case PktOid:
      whereClause = QStringLiteral( "%2oid=$%1" ).arg( offset ).arg( aliased );
      break;

    case PktInt:
    case PktUint64:
      Q_ASSERT( mPrimaryKeyAttrs.size() == 1 );
      whereClause = QStringLiteral( "%3%1=$%2" ).arg( quotedIdentifier( field( mPrimaryKeyAttrs[0] ).name() ) ).arg( offset ).arg( aliased );
      break;

    case PktFidMap:
    {
      QString delim;
      for ( int i = 0; i < mPrimaryKeyAttrs.size(); i++ )
      {
        int idx = mPrimaryKeyAttrs[i];
        QgsField fld = field( idx );

        whereClause += delim + QStringLiteral( "%3%1=$%2" ).arg( connectionRO()->fieldExpression( fld ) ).arg( offset++ ).arg( aliased );
        delim = QStringLiteral( " AND " );
      }
    }
    break;

    case PktUnknown:
      Q_ASSERT( !"FAILURE: Primary key unknown" );
      whereClause = QStringLiteral( "NULL" );
      break;
  }

  if ( !mSqlWhereClause.isEmpty() )
  {
    if ( !whereClause.isEmpty() )
      whereClause += QLatin1String( " AND " );

    whereClause += '(' + mSqlWhereClause + ')';
  }

  return whereClause;
}

void QgsPostgresProvider::appendPkParams( QgsFeatureId featureId, QStringList &params ) const
{
  switch ( mPrimaryKeyType )
  {
    case PktOid:
    case PktUint64:
      params << QString::number( featureId );
      break;

    case PktInt:
      params << QString::number( FID2PKINT( featureId ) );
      break;

    case PktTid:
      params << QStringLiteral( "'(%1,%2)'" ).arg( FID_TO_NUMBER( featureId ) >> 16 ).arg( FID_TO_NUMBER( featureId ) & 0xffff );
      break;

    case PktFidMap:
    {
      QVariantList pkVals = mShared->lookupKey( featureId );
      if ( !pkVals.isEmpty() )
      {
        Q_ASSERT( pkVals.size() == mPrimaryKeyAttrs.size() );
      }

      for ( int i = 0; i < mPrimaryKeyAttrs.size(); i++ )
      {
        if ( i < pkVals.size() )
        {
          params << pkVals[i].toString();
        }
        else
        {
          QgsDebugMsg( QStringLiteral( "FAILURE: Key value %1 for feature %2 not found." ).arg( mPrimaryKeyAttrs[i] ).arg( featureId ) );
          params << QStringLiteral( "NULL" );
        }
      }

      QgsDebugMsg( QStringLiteral( "keys params: %1" ).arg( params.join( "; " ) ) );
    }
    break;

    case PktUnknown:
      Q_ASSERT( !"FAILURE: Primary key unknown" );
      break;
  }
}


QString QgsPostgresProvider::whereClause( QgsFeatureId featureId ) const
{
  return QgsPostgresUtils::whereClause( featureId, mAttributeFields, connectionRO(), mPrimaryKeyType, mPrimaryKeyAttrs, mShared );
}


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
      whereClause = QStringLiteral( "%1=%2" ).arg( QgsPostgresConn::quotedIdentifier( fields.at( pkAttrs[0] ).name() ) ).arg( FID2PKINT( featureId ) );
      break;

    case PktUint64:
      Q_ASSERT( pkAttrs.size() == 1 );
      whereClause = QStringLiteral( "%1=%2" ).arg( QgsPostgresConn::quotedIdentifier( fields.at( pkAttrs[0] ).name() ) ).arg( featureId );
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

          whereClause += delim + conn->fieldExpression( fld );
          if ( pkVals[i].isNull() )
            whereClause += QLatin1String( " IS NULL" );
          else
            whereClause += '=' + QgsPostgresConn::quotedValue( pkVals[i].toString() );

          delim = QStringLiteral( " AND " );
        }
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "FAILURE: Key values for feature %1 not found." ).arg( featureId ) );
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
  switch ( pkType )
  {
    case PktOid:
    case PktInt:
    case PktUint64:
    {
      QString expr;

      //simple primary key, so prefer to use an "IN (...)" query. These are much faster then multiple chained ...OR... clauses
      if ( !featureIds.isEmpty() )
      {
        QString delim;
        expr = QStringLiteral( "%1 IN (" ).arg( ( pkType == PktOid ? QStringLiteral( "oid" ) : QgsPostgresConn::quotedIdentifier( fields.at( pkAttrs[0] ).name() ) ) );

        Q_FOREACH ( const QgsFeatureId featureId, featureIds )
        {
          expr += delim + FID_TO_STRING( ( pkType == PktOid ? featureId : pkType == PktUint64 ? featureId : FID2PKINT( featureId ) ) );
          delim = ',';
        }
        expr += ')';
      }

      return expr;
    }
    case PktFidMap:
    case PktTid:
    case PktUnknown:
    {
      //complex primary key, need to build up where string
      QStringList whereClauses;
      Q_FOREACH ( const QgsFeatureId featureId, featureIds )
      {
        whereClauses << whereClause( featureId, fields, conn, pkType, pkAttrs, sharedData );
      }
      return whereClauses.isEmpty() ? QString() : whereClauses.join( QStringLiteral( " OR " ) ).prepend( '(' ).append( ')' );
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

QString QgsPostgresProvider::filterWhereClause() const
{
  QString where;
  QString delim = QStringLiteral( " WHERE " );

  if ( !mSqlWhereClause.isEmpty() )
  {
    where += delim + '(' + mSqlWhereClause + ')';
    delim = QStringLiteral( " AND " );
  }

  if ( !mRequestedSrid.isEmpty() && ( mRequestedSrid != mDetectedSrid || mRequestedSrid.toInt() == 0 ) )
  {
    where += delim + QStringLiteral( "%1(%2%3)=%4" )
             .arg( connectionRO()->majorVersion() < 2 ? "srid" : "st_srid",
                   quotedIdentifier( mGeometryColumn ),
                   mSpatialColType == SctGeography ? "::geography" : "",
                   mRequestedSrid );
    delim = QStringLiteral( " AND " );
  }

  if ( mRequestedGeomType != QgsWkbTypes::Unknown && mRequestedGeomType != mDetectedGeomType )
  {
    where += delim + QgsPostgresConn::postgisTypeFilter( mGeometryColumn, ( QgsWkbTypes::Type )mRequestedGeomType, mSpatialColType == SctGeography );
    delim = QStringLiteral( " AND " );
  }

  return where;
}

void QgsPostgresProvider::setExtent( QgsRectangle &newExtent )
{
  mLayerExtent.setXMaximum( newExtent.xMaximum() );
  mLayerExtent.setXMinimum( newExtent.xMinimum() );
  mLayerExtent.setYMaximum( newExtent.yMaximum() );
  mLayerExtent.setYMinimum( newExtent.yMinimum() );
}

/**
 * Returns the feature type
 */
QgsWkbTypes::Type QgsPostgresProvider::wkbType() const
{
  return mRequestedGeomType != QgsWkbTypes::Unknown ? mRequestedGeomType : mDetectedGeomType;
}

QgsLayerMetadata QgsPostgresProvider::layerMetadata() const
{
  return mLayerMetadata;
}

QgsField QgsPostgresProvider::field( int index ) const
{
  if ( index < 0 || index >= mAttributeFields.count() )
  {
    QgsMessageLog::logMessage( tr( "FAILURE: Field %1 not found." ).arg( index ), tr( "PostGIS" ) );
    throw PGFieldNotFound();
  }

  return mAttributeFields.at( index );
}

QgsFields QgsPostgresProvider::fields() const
{
  return mAttributeFields;
}

QString QgsPostgresProvider::dataComment() const
{
  return mDataComment;
}


//! \todo XXX Perhaps this should be promoted to QgsDataProvider?
QString QgsPostgresProvider::endianString()
{
  switch ( QgsApplication::endian() )
  {
    case QgsApplication::NDR:
      return QStringLiteral( "NDR" );
    case QgsApplication::XDR:
      return QStringLiteral( "XDR" );
    default :
      return QStringLiteral( "Unknown" );
  }
}


struct PGTypeInfo
{
  QString typeName;
  QString typeType;
  QString typeElem;
  int typeLen;
};

bool QgsPostgresProvider::loadFields()
{
  if ( !mIsQuery )
  {
    QgsDebugMsg( QStringLiteral( "Loading fields for table %1" ).arg( mTableName ) );

    // Get the relation oid for use in later queries
    QString sql = QStringLiteral( "SELECT regclass(%1)::oid" ).arg( quotedValue( mQuery ) );
    QgsPostgresResult tresult( connectionRO()->PQexec( sql ) );
    QString tableoid = tresult.PQgetvalue( 0, 0 );

    // Get the table description
    sql = QStringLiteral( "SELECT description FROM pg_description WHERE objoid=%1 AND objsubid=0" ).arg( tableoid );
    tresult = connectionRO()->PQexec( sql );
    if ( tresult.PQntuples() > 0 )
    {
      mDataComment = tresult.PQgetvalue( 0, 0 );
      mLayerMetadata.setAbstract( mDataComment );
    }
  }

  // Populate the field vector for this layer. The field vector contains
  // field name, type, length, and precision (if numeric)
  QString sql = QStringLiteral( "SELECT * FROM %1 LIMIT 0" ).arg( mQuery );

  QgsPostgresResult result( connectionRO()->PQexec( sql ) );

  // Collect type info
  sql = QStringLiteral( "SELECT oid,typname,typtype,typelem,typlen FROM pg_type" );
  QgsPostgresResult typeResult( connectionRO()->PQexec( sql ) );

  QMap<int, PGTypeInfo> typeMap;
  for ( int i = 0; i < typeResult.PQntuples(); ++i )
  {
    PGTypeInfo typeInfo =
    {
      /* typeName = */ typeResult.PQgetvalue( i, 1 ),
      /* typeType = */ typeResult.PQgetvalue( i, 2 ),
      /* typeElem = */ typeResult.PQgetvalue( i, 3 ),
      /* typeLen = */ typeResult.PQgetvalue( i, 4 ).toInt()
    };
    typeMap.insert( typeResult.PQgetvalue( i, 0 ).toInt(), typeInfo );
  }


  QMap<int, QMap<int, QString> > fmtFieldTypeMap, descrMap, defValMap;
  QMap<int, QMap<int, int> > attTypeIdMap;
  QMap<int, QMap<int, bool> > notNullMap, uniqueMap;
  if ( result.PQnfields() > 0 )
  {
    // Collect table oids
    QSet<int> tableoids;
    for ( int i = 0; i < result.PQnfields(); i++ )
    {
      int tableoid = result.PQftable( i );
      if ( tableoid > 0 )
      {
        tableoids.insert( tableoid );
      }
    }

    if ( !tableoids.isEmpty() )
    {
      QStringList tableoidsList;
      Q_FOREACH ( int tableoid, tableoids )
      {
        tableoidsList.append( QString::number( tableoid ) );
      }

      QString tableoidsFilter = '(' + tableoidsList.join( QStringLiteral( "," ) ) + ')';

      // Collect formatted field types
      sql = "SELECT attrelid, attnum, pg_catalog.format_type(atttypid,atttypmod), pg_catalog.col_description(attrelid,attnum), pg_catalog.pg_get_expr(adbin,adrelid), atttypid, attnotnull::int, indisunique::int"
            " FROM pg_attribute"
            " LEFT OUTER JOIN pg_attrdef ON attrelid=adrelid AND attnum=adnum"

            // find unique constraints if present. Text cast required to handle int2vector comparison. Distinct required as multiple unique constraints may exist
            " LEFT OUTER JOIN ( SELECT DISTINCT indrelid, indkey, indisunique FROM pg_index WHERE indisunique ) uniq ON attrelid=indrelid AND attnum::text=indkey::text "

            " WHERE attrelid IN " + tableoidsFilter;
      QgsPostgresResult fmtFieldTypeResult( connectionRO()->PQexec( sql ) );
      for ( int i = 0; i < fmtFieldTypeResult.PQntuples(); ++i )
      {
        int attrelid = fmtFieldTypeResult.PQgetvalue( i, 0 ).toInt();
        int attnum = fmtFieldTypeResult.PQgetvalue( i, 1 ).toInt();
        QString formatType = fmtFieldTypeResult.PQgetvalue( i, 2 );
        QString descr = fmtFieldTypeResult.PQgetvalue( i, 3 );
        QString defVal = fmtFieldTypeResult.PQgetvalue( i, 4 );
        int attType = fmtFieldTypeResult.PQgetvalue( i, 5 ).toInt();
        bool attNotNull = fmtFieldTypeResult.PQgetvalue( i, 6 ).toInt();
        bool uniqueConstraint = fmtFieldTypeResult.PQgetvalue( i, 7 ).toInt();
        fmtFieldTypeMap[attrelid][attnum] = formatType;
        descrMap[attrelid][attnum] = descr;
        defValMap[attrelid][attnum] = defVal;
        attTypeIdMap[attrelid][attnum] = attType;
        notNullMap[attrelid][attnum] = attNotNull;
        uniqueMap[attrelid][attnum] = uniqueConstraint;
      }
    }
  }

  QSet<QString> fields;
  mAttributeFields.clear();
  for ( int i = 0; i < result.PQnfields(); i++ )
  {
    QString fieldName = result.PQfname( i );
    if ( fieldName == mGeometryColumn )
      continue;

    int fldtyp = result.PQftype( i );
    int fldMod = result.PQfmod( i );
    int fieldPrec = -1;
    int tableoid = result.PQftable( i );
    int attnum = result.PQftablecol( i );
    int atttypid = attTypeIdMap[tableoid][attnum];

    const PGTypeInfo &typeInfo = typeMap.value( fldtyp );
    QString fieldTypeName = typeInfo.typeName;
    QString fieldTType = typeInfo.typeType;
    int fieldSize = typeInfo.typeLen;

    bool isDomain = ( typeMap.value( atttypid ).typeType == QLatin1String( "d" ) );

    QString formattedFieldType = fmtFieldTypeMap[tableoid][attnum];
    QString originalFormattedFieldType = formattedFieldType;
    if ( isDomain )
    {
      // get correct formatted field type for domain
      sql = QStringLiteral( "SELECT format_type(%1, %2)" ).arg( fldtyp ).arg( fldMod );
      QgsPostgresResult fmtFieldModResult( connectionRO()->PQexec( sql ) );
      if ( fmtFieldModResult.PQntuples() > 0 )
      {
        formattedFieldType = fmtFieldModResult.PQgetvalue( 0, 0 );
      }
    }

    QString fieldComment = descrMap[tableoid][attnum];

    QVariant::Type fieldType;
    QVariant::Type fieldSubType = QVariant::Invalid;

    if ( fieldTType == QLatin1String( "b" ) )
    {
      bool isArray = fieldTypeName.startsWith( '_' );

      if ( isArray )
        fieldTypeName = fieldTypeName.mid( 1 );

      if ( fieldTypeName == QLatin1String( "int8" ) || fieldTypeName == QLatin1String( "serial8" ) )
      {
        fieldType = QVariant::LongLong;
        fieldSize = -1;
        fieldPrec = 0;
      }
      else if ( fieldTypeName == QLatin1String( "int2" ) || fieldTypeName == QLatin1String( "int4" ) ||
                fieldTypeName == QLatin1String( "oid" ) || fieldTypeName == QLatin1String( "serial" ) )
      {
        fieldType = QVariant::Int;
        fieldSize = -1;
        fieldPrec = 0;
      }
      else if ( fieldTypeName == QLatin1String( "real" ) || fieldTypeName == QLatin1String( "double precision" ) ||
                fieldTypeName == QLatin1String( "float4" ) || fieldTypeName == QLatin1String( "float8" ) )
      {
        fieldType = QVariant::Double;
        fieldSize = -1;
        fieldPrec = -1;
      }
      else if ( fieldTypeName == QLatin1String( "numeric" ) )
      {
        fieldType = QVariant::Double;

        if ( formattedFieldType == QLatin1String( "numeric" ) || formattedFieldType.isEmpty() )
        {
          fieldSize = -1;
          fieldPrec = -1;
        }
        else
        {
          QRegExp re( "numeric\\((\\d+),(\\d+)\\)" );
          if ( re.exactMatch( formattedFieldType ) )
          {
            fieldSize = re.cap( 1 ).toInt();
            fieldPrec = re.cap( 2 ).toInt();
          }
          else if ( formattedFieldType != QLatin1String( "numeric" ) )
          {
            QgsMessageLog::logMessage( tr( "unexpected formatted field type '%1' for field %2" )
                                       .arg( formattedFieldType,
                                             fieldName ),
                                       tr( "PostGIS" ) );
            fieldSize = -1;
            fieldPrec = -1;
          }
        }
      }
      else if ( fieldTypeName == QLatin1String( "varchar" ) )
      {
        fieldType = QVariant::String;

        QRegExp re( "character varying\\((\\d+)\\)" );
        if ( re.exactMatch( formattedFieldType ) )
        {
          fieldSize = re.cap( 1 ).toInt();
        }
        else
        {
          fieldSize = -1;
        }
      }
      else if ( fieldTypeName == QLatin1String( "date" ) )
      {
        fieldType = QVariant::Date;
        fieldSize = -1;
      }
      else if ( fieldTypeName == QLatin1String( "time" ) )
      {
        fieldType = QVariant::Time;
        fieldSize = -1;
      }
      else if ( fieldTypeName == QLatin1String( "timestamp" ) )
      {
        fieldType = QVariant::DateTime;
        fieldSize = -1;
      }
      else if ( fieldTypeName == QLatin1String( "text" ) ||
                fieldTypeName == QLatin1String( "geometry" ) ||
                fieldTypeName == QLatin1String( "inet" ) ||
                fieldTypeName == QLatin1String( "money" ) ||
                fieldTypeName == QLatin1String( "ltree" ) ||
                fieldTypeName == QLatin1String( "uuid" ) ||
                fieldTypeName == QLatin1String( "xml" ) ||
                fieldTypeName.startsWith( QLatin1String( "time" ) ) ||
                fieldTypeName.startsWith( QLatin1String( "date" ) ) )
      {
        fieldType = QVariant::String;
        fieldSize = -1;
      }
      else if ( fieldTypeName == QLatin1String( "bpchar" ) )
      {
        // although postgres internally uses "bpchar", this is exposed to users as character in postgres
        fieldTypeName = QStringLiteral( "character" );

        fieldType = QVariant::String;

        QRegExp re( "character\\((\\d+)\\)" );
        if ( re.exactMatch( formattedFieldType ) )
        {
          fieldSize = re.cap( 1 ).toInt();
        }
        else
        {
          QgsDebugMsg( QStringLiteral( "unexpected formatted field type '%1' for field %2" )
                       .arg( formattedFieldType,
                             fieldName ) );
          fieldSize = -1;
          fieldPrec = -1;
        }
      }
      else if ( fieldTypeName == QLatin1String( "char" ) )
      {
        fieldType = QVariant::String;

        QRegExp re( "char\\((\\d+)\\)" );
        if ( re.exactMatch( formattedFieldType ) )
        {
          fieldSize = re.cap( 1 ).toInt();
        }
        else
        {
          QgsMessageLog::logMessage( tr( "unexpected formatted field type '%1' for field %2" )
                                     .arg( formattedFieldType,
                                           fieldName ) );
          fieldSize = -1;
          fieldPrec = -1;
        }
      }
      else if ( fieldTypeName == QLatin1String( "hstore" ) ||  fieldTypeName == QLatin1String( "json" ) || fieldTypeName == QLatin1String( "jsonb" ) )
      {
        fieldType = QVariant::Map;
        fieldSubType = QVariant::String;
        fieldSize = -1;
      }
      else if ( fieldTypeName == QLatin1String( "bool" ) )
      {
        // enum
        fieldType = QVariant::Bool;
        fieldSize = -1;
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Field %1 ignored, because of unsupported type %2" ).arg( fieldName, fieldTypeName ), tr( "PostGIS" ) );
        continue;
      }

      if ( isArray )
      {
        fieldTypeName = '_' + fieldTypeName;
        fieldSubType = fieldType;
        fieldType = ( fieldType == QVariant::String ? QVariant::StringList : QVariant::List );
        fieldSize = -1;
      }
    }
    else if ( fieldTType == QLatin1String( "e" ) )
    {
      // enum
      fieldType = QVariant::String;
      fieldSize = -1;
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Field %1 ignored, because of unsupported type %2" ).arg( fieldName, fieldTType ), tr( "PostGIS" ) );
      continue;
    }

    if ( fields.contains( fieldName ) )
    {
      QgsMessageLog::logMessage( tr( "Duplicate field %1 found\n" ).arg( fieldName ), tr( "PostGIS" ) );
      return false;
    }

    fields << fieldName;

    if ( isDomain )
    {
      //field was defined using domain, so use domain type name for fieldTypeName
      fieldTypeName = originalFormattedFieldType;
    }

    mAttrPalIndexName.insert( i, fieldName );
    mDefaultValues.insert( mAttributeFields.size(), defValMap[tableoid][attnum] );

    QgsField newField = QgsField( fieldName, fieldType, fieldTypeName, fieldSize, fieldPrec, fieldComment, fieldSubType );

    QgsFieldConstraints constraints;
    if ( notNullMap[tableoid][attnum] || mPrimaryKeyAttrs.contains( i ) )
      constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
    if ( uniqueMap[tableoid][attnum] || mPrimaryKeyAttrs.contains( i ) )
      constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
    newField.setConstraints( constraints );

    mAttributeFields.append( newField );
  }

  setEditorWidgets();

  return true;
}

void QgsPostgresProvider::setEditorWidgets()
{
  if ( tableExists( *connectionRO(), EDITOR_WIDGET_STYLES_TABLE ) )
  {
    for ( int i = 0; i < mAttributeFields.count(); ++i )
    {
      // CREATE TABLE qgis_editor_widget_styles (schema_name TEXT NOT NULL, table_name TEXT NOT NULL, field_name TEXT NOT NULL,
      //                                         type TEXT NOT NULL, config TEXT,
      //                                         PRIMARY KEY(schema_name, table_name, field_name));
      QgsField &field = mAttributeFields[i];
      const QString sql = QStringLiteral( "SELECT type, config FROM %1 WHERE schema_name = %2 and table_name = %3 and field_name = %4 LIMIT 1" ).
                          arg( EDITOR_WIDGET_STYLES_TABLE, quotedValue( mSchemaName ), quotedValue( mTableName ), quotedValue( field.name() ) );
      QgsPostgresResult result( connectionRO()->PQexec( sql ) );
      for ( int i = 0; i < result.PQntuples(); ++i )
      {
        const QString type = result.PQgetvalue( i, 0 );
        QVariantMap config;
        if ( !result.PQgetisnull( i, 1 ) ) // Can be null and it's OK
        {
          const QString configTxt = result.PQgetvalue( i, 1 );
          QDomDocument doc;
          if ( doc.setContent( configTxt ) )
          {
            config = QgsXmlUtils::readVariant( doc.documentElement() ).toMap();
          }
          else
          {
            QgsMessageLog::logMessage( tr( "Cannot parse widget configuration for field %1.%2.%3\n" ).arg( mSchemaName, mTableName, field.name() ), tr( "PostGIS" ) );
          }
        }

        field.setEditorWidgetSetup( QgsEditorWidgetSetup( type, config ) );
      }
    }
  }
}

bool QgsPostgresProvider::hasSufficientPermsAndCapabilities()
{
  QgsDebugMsg( QStringLiteral( "Checking for permissions on the relation" ) );

  QgsPostgresResult testAccess;
  if ( !mIsQuery )
  {
    // Check that we can read from the table (i.e., we have select permission).
    QString sql = QStringLiteral( "SELECT * FROM %1 LIMIT 1" ).arg( mQuery );
    QgsPostgresResult testAccess( connectionRO()->PQexec( sql ) );
    if ( testAccess.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsMessageLog::logMessage( tr( "Unable to access the %1 relation.\nThe error message from the database was:\n%2.\nSQL: %3" )
                                 .arg( mQuery,
                                       testAccess.PQresultErrorMessage(),
                                       sql ), tr( "PostGIS" ) );
      return false;
    }

    bool inRecovery = false;

    if ( connectionRO()->pgVersion() >= 90000 )
    {
      testAccess = connectionRO()->PQexec( QStringLiteral( "SELECT pg_is_in_recovery()" ) );
      if ( testAccess.PQresultStatus() != PGRES_TUPLES_OK || testAccess.PQgetvalue( 0, 0 ) == QLatin1String( "t" ) )
      {
        QgsMessageLog::logMessage( tr( "PostgreSQL is still in recovery after a database crash\n(or you are connected to a (read-only) slave).\nWrite accesses will be denied." ), tr( "PostGIS" ) );
        inRecovery = true;
      }
    }

    // postgres has fast access to features at id (thanks to primary key / unique index)
    // the latter flag is here just for compatibility
    if ( !mSelectAtIdDisabled )
    {
      mEnabledCapabilities = QgsVectorDataProvider::SelectAtId;
    }

    if ( !inRecovery )
    {
      if ( connectionRO()->pgVersion() >= 80400 )
      {
        sql = QString( "SELECT "
                       "has_table_privilege(%1,'DELETE'),"
                       "has_any_column_privilege(%1,'UPDATE'),"
                       "%2"
                       "has_table_privilege(%1,'INSERT'),"
                       "current_schema()" )
              .arg( quotedValue( mQuery ),
                    mGeometryColumn.isNull()
                    ? QStringLiteral( "'f'," )
                    : QStringLiteral( "has_column_privilege(%1,%2,'UPDATE')," )
                    .arg( quotedValue( mQuery ),
                          quotedValue( mGeometryColumn ) )
                  );
      }
      else
      {
        sql = QString( "SELECT "
                       "has_table_privilege(%1,'DELETE'),"
                       "has_table_privilege(%1,'UPDATE'),"
                       "has_table_privilege(%1,'UPDATE'),"
                       "has_table_privilege(%1,'INSERT'),"
                       "current_schema()" )
              .arg( quotedValue( mQuery ) );
      }

      testAccess = connectionRO()->PQexec( sql );
      if ( testAccess.PQresultStatus() != PGRES_TUPLES_OK )
      {
        QgsMessageLog::logMessage( tr( "Unable to determine table access privileges for the %1 relation.\nThe error message from the database was:\n%2.\nSQL: %3" )
                                   .arg( mQuery,
                                         testAccess.PQresultErrorMessage(),
                                         sql ),
                                   tr( "PostGIS" ) );
        return false;
      }


      if ( testAccess.PQgetvalue( 0, 0 ) == QLatin1String( "t" ) )
      {
        // DELETE
        mEnabledCapabilities |= QgsVectorDataProvider::DeleteFeatures | QgsVectorDataProvider::FastTruncate;
      }

      if ( testAccess.PQgetvalue( 0, 1 ) == QLatin1String( "t" ) )
      {
        // UPDATE
        mEnabledCapabilities |= QgsVectorDataProvider::ChangeAttributeValues;
      }

      if ( testAccess.PQgetvalue( 0, 2 ) == QLatin1String( "t" ) )
      {
        // UPDATE
        mEnabledCapabilities |= QgsVectorDataProvider::ChangeGeometries;
      }

      if ( testAccess.PQgetvalue( 0, 3 ) == QLatin1String( "t" ) )
      {
        // INSERT
        mEnabledCapabilities |= QgsVectorDataProvider::AddFeatures;
      }

      if ( mSchemaName.isEmpty() )
        mSchemaName = testAccess.PQgetvalue( 0, 4 );

      sql = QString( "SELECT 1 FROM pg_class,pg_namespace WHERE "
                     "pg_class.relnamespace=pg_namespace.oid AND "
                     "%3 AND "
                     "relname=%1 AND nspname=%2" )
            .arg( quotedValue( mTableName ),
                  quotedValue( mSchemaName ),
                  connectionRO()->pgVersion() < 80100 ? "pg_get_userbyid(relowner)=current_user" : "pg_has_role(relowner,'MEMBER')" );
      testAccess = connectionRO()->PQexec( sql );
      if ( testAccess.PQresultStatus() == PGRES_TUPLES_OK && testAccess.PQntuples() == 1 )
      {
        mEnabledCapabilities |= QgsVectorDataProvider::AddAttributes | QgsVectorDataProvider::DeleteAttributes | QgsVectorDataProvider::RenameAttributes;
      }
    }
  }
  else
  {
    // Check if the sql is a select query
    if ( !mQuery.startsWith( '(' ) && !mQuery.endsWith( ')' ) )
    {
      QgsMessageLog::logMessage( tr( "The custom query is not a select query." ), tr( "PostGIS" ) );
      return false;
    }

    // get a new alias for the subquery
    int index = 0;
    QString alias;
    QRegExp regex;
    do
    {
      alias = QStringLiteral( "subQuery_%1" ).arg( QString::number( index++ ) );
      QString pattern = QStringLiteral( "(\\\"?)%1\\1" ).arg( QRegExp::escape( alias ) );
      regex.setPattern( pattern );
      regex.setCaseSensitivity( Qt::CaseInsensitive );
    }
    while ( mQuery.contains( regex ) );

    // convert the custom query into a subquery
    mQuery = QStringLiteral( "%1 AS %2" )
             .arg( mQuery,
                   quotedIdentifier( alias ) );

    QString sql = QStringLiteral( "SELECT * FROM %1 LIMIT 1" ).arg( mQuery );

    testAccess = connectionRO()->PQexec( sql );
    if ( testAccess.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsMessageLog::logMessage( tr( "Unable to execute the query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                                 .arg( testAccess.PQresultErrorMessage(),
                                       sql ), tr( "PostGIS" ) );
      return false;
    }

    if ( !mSelectAtIdDisabled )
    {
      mEnabledCapabilities = QgsVectorDataProvider::SelectAtId;
    }
  }

  // supports geometry simplification on provider side
  mEnabledCapabilities |= ( QgsVectorDataProvider::SimplifyGeometries | QgsVectorDataProvider::SimplifyGeometriesWithTopologicalValidation );

  //supports transactions
  mEnabledCapabilities |= QgsVectorDataProvider::TransactionSupport;

  // supports circular geometries
  mEnabledCapabilities |= QgsVectorDataProvider::CircularGeometries;

  // supports layer metadata
  mEnabledCapabilities |= QgsVectorDataProvider::ReadLayerMetadata;

  if ( ( mEnabledCapabilities & QgsVectorDataProvider::ChangeGeometries ) &&
       ( mEnabledCapabilities & QgsVectorDataProvider::ChangeAttributeValues ) &&
       mSpatialColType != SctTopoGeometry )
  {
    mEnabledCapabilities |= QgsVectorDataProvider::ChangeFeatures;
  }

  return true;
}

bool QgsPostgresProvider::determinePrimaryKey()
{
  if ( !loadFields() )
  {
    return false;
  }

  // check to see if there is an unique index on the relation, which
  // can be used as a key into the table. Primary keys are always
  // unique indices, so we catch them as well.

  QString sql;
  if ( !mIsQuery )
  {
    sql = QStringLiteral( "SELECT count(*) FROM pg_inherits WHERE inhparent=%1::regclass" ).arg( quotedValue( mQuery ) );
    QgsDebugMsg( QStringLiteral( "Checking whether %1 is a parent table" ).arg( sql ) );
    QgsPostgresResult res( connectionRO()->PQexec( sql ) );
    bool isParentTable( res.PQntuples() == 0 || res.PQgetvalue( 0, 0 ).toInt() > 0 );

    sql = QStringLiteral( "SELECT indexrelid FROM pg_index WHERE indrelid=%1::regclass AND (indisprimary OR indisunique) ORDER BY CASE WHEN indisprimary THEN 1 ELSE 2 END LIMIT 1" ).arg( quotedValue( mQuery ) );
    QgsDebugMsg( QStringLiteral( "Retrieving first primary or unique index: %1" ).arg( sql ) );

    res = connectionRO()->PQexec( sql );
    QgsDebugMsg( QStringLiteral( "Got %1 rows." ).arg( res.PQntuples() ) );

    QStringList log;

    // no primary or unique indizes found
    if ( res.PQntuples() == 0 )
    {
      QgsDebugMsg( QStringLiteral( "Relation has no primary key -- investigating alternatives" ) );

      // Two options here. If the relation is a table, see if there is
      // an oid column that can be used instead.
      // If the relation is a view try to find a suitable column to use as
      // the primary key.

      QgsPostgresProvider::Relkind type = relkind();

      if ( type == Relkind::OrdinaryTable || type == Relkind::PartitionedTable )
      {
        QgsDebugMsg( QStringLiteral( "Relation is a table. Checking to see if it has an oid column." ) );

        mPrimaryKeyAttrs.clear();

        // If there is an oid on the table, use that instead,
        sql = QStringLiteral( "SELECT attname FROM pg_attribute WHERE attname='oid' AND attrelid=regclass(%1)" ).arg( quotedValue( mQuery ) );

        res = connectionRO()->PQexec( sql );
        if ( res.PQntuples() == 1 )
        {
          // Could warn the user here that performance will suffer if
          // oid isn't indexed (and that they may want to add a
          // primary key to the table)
          mPrimaryKeyType = PktOid;
        }
        else
        {
          sql = QStringLiteral( "SELECT attname FROM pg_attribute WHERE attname='ctid' AND attrelid=regclass(%1)" ).arg( quotedValue( mQuery ) );

          res = connectionRO()->PQexec( sql );
          if ( res.PQntuples() == 1 )
          {
            mPrimaryKeyType = PktTid;

            QgsMessageLog::logMessage( tr( "Primary key is ctid - changing of existing features disabled (%1; %2)" ).arg( mGeometryColumn, mQuery ) );
            mEnabledCapabilities &= ~( QgsVectorDataProvider::DeleteFeatures | QgsVectorDataProvider::ChangeAttributeValues | QgsVectorDataProvider::ChangeGeometries | QgsVectorDataProvider::ChangeFeatures );
          }
          else
          {
            QgsMessageLog::logMessage( tr( "The table has no column suitable for use as a key. QGIS requires a primary key, a PostgreSQL oid column or a ctid for tables." ), tr( "PostGIS" ) );
          }
        }
      }
      else if ( type == Relkind::View || type == Relkind::MaterializedView )
      {
        determinePrimaryKeyFromUriKeyColumn();
      }
      else
      {
        const QMetaEnum metaEnum( QMetaEnum::fromType<Relkind>() );
        QString typeName = metaEnum.valueToKey( type );
        QgsMessageLog::logMessage( tr( "Unexpected relation type '%1'." ).arg( typeName ), tr( "PostGIS" ) );
      }
    }
    else
    {
      // have a primary key or unique index
      QString indrelid = res.PQgetvalue( 0, 0 );
      sql = QStringLiteral( "SELECT attname,attnotnull FROM pg_index,pg_attribute WHERE indexrelid=%1 AND indrelid=attrelid AND pg_attribute.attnum=any(pg_index.indkey)" ).arg( indrelid );

      QgsDebugMsg( "Retrieving key columns: " + sql );
      res = connectionRO()->PQexec( sql );
      QgsDebugMsg( QStringLiteral( "Got %1 rows." ).arg( res.PQntuples() ) );

      bool mightBeNull = false;
      QString primaryKey;
      QString delim;

      mPrimaryKeyType = PktFidMap; // map by default, will downgrade if needed
      for ( int i = 0; i < res.PQntuples(); i++ )
      {
        QString name = res.PQgetvalue( i, 0 );
        if ( res.PQgetvalue( i, 1 ).startsWith( 'f' ) )
        {
          QgsMessageLog::logMessage( tr( "Unique column '%1' doesn't have a NOT NULL constraint." ).arg( name ), tr( "PostGIS" ) );
          mightBeNull = true;
        }

        primaryKey += delim + quotedIdentifier( name );
        delim = ',';

        int idx = fieldNameIndex( name );
        if ( idx == -1 )
        {
          QgsDebugMsg( "Skipping " + name );
          continue;
        }
        QgsField fld = mAttributeFields.at( idx );

        // Always use PktFidMap for multi-field keys
        mPrimaryKeyType = i ? PktFidMap : pkType( fld );

        mPrimaryKeyAttrs << idx;
      }

      if ( ( mightBeNull || isParentTable ) && !mUseEstimatedMetadata && !uniqueData( primaryKey ) )
      {
        QgsMessageLog::logMessage( tr( "Ignoring key candidate because of NULL values or inheritance" ), tr( "PostGIS" ) );
        mPrimaryKeyType = PktUnknown;
        mPrimaryKeyAttrs.clear();
      }
    }
  }
  else
  {
    determinePrimaryKeyFromUriKeyColumn();
  }

  Q_FOREACH ( int fieldIdx, mPrimaryKeyAttrs )
  {
    //primary keys are unique, not null
    QgsFieldConstraints constraints = mAttributeFields.at( fieldIdx ).constraints();
    constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
    constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
    mAttributeFields[ fieldIdx ].setConstraints( constraints );
  }

  mValid = mPrimaryKeyType != PktUnknown;

  return mValid;
}

/* static */
QStringList QgsPostgresProvider::parseUriKey( const QString &key )
{
  if ( key.isEmpty() ) return QStringList();

  QStringList cols;

  // remove quotes from key list
  if ( key.startsWith( '"' ) && key.endsWith( '"' ) )
  {
    int i = 1;
    QString col;
    while ( i < key.size() )
    {
      if ( key[i] == '"' )
      {
        if ( i + 1 < key.size() && key[i + 1] == '"' )
        {
          i++;
        }
        else
        {
          cols << col;
          col.clear();

          if ( ++i == key.size() )
            break;

          Q_ASSERT( key[i] == ',' );
          i++;
          Q_ASSERT( key[i] == '"' );
          i++;
          col.clear();
          continue;
        }
      }

      col += key[i++];
    }
  }
  else if ( key.contains( ',' ) )
  {
    cols = key.split( ',' );
  }
  else
  {
    cols << key;
  }

  return cols;
}

void QgsPostgresProvider::determinePrimaryKeyFromUriKeyColumn()
{
  QString primaryKey = mUri.keyColumn();
  mPrimaryKeyType = PktUnknown;

  if ( !primaryKey.isEmpty() )
  {
    QStringList cols = parseUriKey( primaryKey );

    primaryKey.clear();
    QString del;
    Q_FOREACH ( const QString &col, cols )
    {
      primaryKey += del + quotedIdentifier( col );
      del = QStringLiteral( "," );
    }

    Q_FOREACH ( const QString &col, cols )
    {
      int idx = fieldNameIndex( col );
      if ( idx < 0 )
      {
        QgsMessageLog::logMessage( tr( "Key field '%1' for view/query not found." ).arg( col ), tr( "PostGIS" ) );
        mPrimaryKeyAttrs.clear();
        break;
      }

      mPrimaryKeyAttrs << idx;
    }

    if ( !mPrimaryKeyAttrs.isEmpty() )
    {
      bool unique = true;
      if ( mCheckPrimaryKeyUnicity )
      {
        unique = uniqueData( primaryKey );
      }

      if ( mUseEstimatedMetadata || unique )
      {
        mPrimaryKeyType = PktFidMap; // Map by default
        if ( mPrimaryKeyAttrs.size() == 1 )
        {
          QgsField fld = mAttributeFields.at( 0 );
          mPrimaryKeyType = pkType( fld );
        }
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Primary key field '%1' for view/query not unique." ).arg( primaryKey ), tr( "PostGIS" ) );
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Keys for view/query undefined." ), tr( "PostGIS" ) );
    }
  }
  else
  {
    QgsMessageLog::logMessage( tr( "No key field for view/query given." ), tr( "PostGIS" ) );
  }
}

bool QgsPostgresProvider::uniqueData( const QString &quotedColNames )
{
  // Check to see if the given columns contain unique data
  QString sql = QStringLiteral( "SELECT count(distinct (%1))=count((%1)) FROM %2%3" )
                .arg( quotedColNames,
                      mQuery,
                      filterWhereClause() );

  QgsPostgresResult unique( connectionRO()->PQexec( sql ) );

  if ( unique.PQresultStatus() != PGRES_TUPLES_OK )
  {
    pushError( unique.PQresultErrorMessage() );
    return false;
  }

  return unique.PQntuples() == 1 && unique.PQgetvalue( 0, 0 ).startsWith( 't' );
}

// Returns the minimum value of an attribute
QVariant QgsPostgresProvider::minimumValue( int index ) const
{
  try
  {
    // get the field name
    QgsField fld = field( index );
    QString sql = QStringLiteral( "SELECT min(%1) AS %1 FROM %2" )
                  .arg( quotedIdentifier( fld.name() ),
                        mQuery );

    if ( !mSqlWhereClause.isEmpty() )
    {
      sql += QStringLiteral( " WHERE %1" ).arg( mSqlWhereClause );
    }

    sql = QStringLiteral( "SELECT %1 FROM (%2) foo" ).arg( connectionRO()->fieldExpression( fld ), sql );

    QgsPostgresResult rmin( connectionRO()->PQexec( sql ) );
    return convertValue( fld.type(), fld.subType(), rmin.PQgetvalue( 0, 0 ), fld.typeName() );
  }
  catch ( PGFieldNotFound )
  {
    return QVariant( QString() );
  }
}

// Returns the list of unique values of an attribute
QSet<QVariant> QgsPostgresProvider::uniqueValues( int index, int limit ) const
{
  QSet<QVariant> uniqueValues;

  try
  {
    // get the field name
    QgsField fld = field( index );
    QString sql = QStringLiteral( "SELECT DISTINCT %1 FROM %2" )
                  .arg( quotedIdentifier( fld.name() ),
                        mQuery );

    if ( !mSqlWhereClause.isEmpty() )
    {
      sql += QStringLiteral( " WHERE %1" ).arg( mSqlWhereClause );
    }

    sql += QStringLiteral( " ORDER BY %1" ).arg( quotedIdentifier( fld.name() ) );

    if ( limit >= 0 )
    {
      sql += QStringLiteral( " LIMIT %1" ).arg( limit );
    }

    sql = QStringLiteral( "SELECT %1 FROM (%2) foo" ).arg( connectionRO()->fieldExpression( fld ), sql );

    QgsPostgresResult res( connectionRO()->PQexec( sql ) );
    if ( res.PQresultStatus() == PGRES_TUPLES_OK )
    {
      for ( int i = 0; i < res.PQntuples(); i++ )
        uniqueValues.insert( convertValue( fld.type(), fld.subType(), res.PQgetvalue( i, 0 ), fld.typeName() ) );
    }
  }
  catch ( PGFieldNotFound )
  {
  }
  return uniqueValues;
}

QStringList QgsPostgresProvider::uniqueStringsMatching( int index, const QString &substring, int limit, QgsFeedback *feedback ) const
{
  QStringList results;

  try
  {
    // get the field name
    QgsField fld = field( index );
    QString sql = QStringLiteral( "SELECT DISTINCT %1 FROM %2 WHERE" )
                  .arg( quotedIdentifier( fld.name() ),
                        mQuery );

    if ( !mSqlWhereClause.isEmpty() )
    {
      sql += QStringLiteral( " ( %1 ) AND " ).arg( mSqlWhereClause );
    }

    sql += QStringLiteral( " %1::text ILIKE '%%2%'" ).arg( quotedIdentifier( fld.name() ), substring );


    sql += QStringLiteral( " ORDER BY %1" ).arg( quotedIdentifier( fld.name() ) );

    if ( limit >= 0 )
    {
      sql += QStringLiteral( " LIMIT %1" ).arg( limit );
    }

    sql = QStringLiteral( "SELECT %1 FROM (%2) foo" ).arg( connectionRO()->fieldExpression( fld ), sql );

    QgsPostgresResult res( connectionRO()->PQexec( sql ) );
    if ( res.PQresultStatus() == PGRES_TUPLES_OK )
    {
      for ( int i = 0; i < res.PQntuples(); i++ )
      {
        results << ( convertValue( fld.type(), fld.subType(), res.PQgetvalue( i, 0 ), fld.typeName() ) ).toString();
        if ( feedback && feedback->isCanceled() )
          break;
      }
    }
  }
  catch ( PGFieldNotFound )
  {
  }
  return results;
}

void QgsPostgresProvider::enumValues( int index, QStringList &enumList ) const
{
  enumList.clear();

  if ( index < 0 || index >= mAttributeFields.count() )
    return;

  //find out type of index
  QString fieldName = mAttributeFields.at( index ).name();
  QString typeName = mAttributeFields.at( index ).typeName();

  // Remove schema extension from typeName
  typeName.remove( QRegularExpression( "^([^.]+\\.)+" ) );

  //is type an enum?
  QString typeSql = QStringLiteral( "SELECT typtype FROM pg_type WHERE typname=%1" ).arg( quotedValue( typeName ) );
  QgsPostgresResult typeRes( connectionRO()->PQexec( typeSql ) );
  if ( typeRes.PQresultStatus() != PGRES_TUPLES_OK || typeRes.PQntuples() < 1 )
  {
    return;
  }


  QString typtype = typeRes.PQgetvalue( 0, 0 );
  if ( typtype.compare( QLatin1String( "e" ), Qt::CaseInsensitive ) == 0 )
  {
    //try to read enum_range of attribute
    if ( !parseEnumRange( enumList, fieldName ) )
    {
      enumList.clear();
    }
  }
  else
  {
    //is there a domain check constraint for the attribute?
    if ( !parseDomainCheckConstraint( enumList, fieldName ) )
    {
      enumList.clear();
    }
  }
}

bool QgsPostgresProvider::parseEnumRange( QStringList &enumValues, const QString &attributeName ) const
{
  enumValues.clear();

  QString enumRangeSql = QStringLiteral( "SELECT enumlabel FROM pg_catalog.pg_enum WHERE enumtypid=(SELECT atttypid::regclass FROM pg_attribute WHERE attrelid=%1::regclass AND attname=%2)" )
                         .arg( quotedValue( mQuery ),
                               quotedValue( attributeName ) );
  QgsPostgresResult enumRangeRes( connectionRO()->PQexec( enumRangeSql ) );
  if ( enumRangeRes.PQresultStatus() != PGRES_TUPLES_OK )
    return false;

  for ( int i = 0; i < enumRangeRes.PQntuples(); i++ )
  {
    enumValues << enumRangeRes.PQgetvalue( i, 0 );
  }

  return true;
}

bool QgsPostgresProvider::parseDomainCheckConstraint( QStringList &enumValues, const QString &attributeName ) const
{
  enumValues.clear();

  //is it a domain type with a check constraint?
  QString domainSql = QStringLiteral( "SELECT domain_name, domain_schema FROM information_schema.columns WHERE table_name=%1 AND column_name=%2" ).arg( quotedValue( mTableName ), quotedValue( attributeName ) );
  QgsPostgresResult domainResult( connectionRO()->PQexec( domainSql ) );
  if ( domainResult.PQresultStatus() == PGRES_TUPLES_OK && domainResult.PQntuples() > 0 && !domainResult.PQgetvalue( 0, 0 ).isNull() )
  {
    //a domain type
    QString domainCheckDefinitionSql = QStringLiteral( ""
                                       "SELECT consrc FROM pg_constraint "
                                       "  WHERE contypid =("
                                       "    SELECT oid FROM pg_type "
                                       "      WHERE typname = %1 "
                                       "      AND typnamespace =("
                                       "        SELECT oid FROM pg_namespace WHERE nspname = %2"
                                       "      )"
                                       "    )" )
                                       .arg( quotedValue( domainResult.PQgetvalue( 0, 0 ) ) )
                                       .arg( quotedValue( domainResult.PQgetvalue( 0, 1 ) ) );
    QgsPostgresResult domainCheckRes( connectionRO()->PQexec( domainCheckDefinitionSql ) );
    if ( domainCheckRes.PQresultStatus() == PGRES_TUPLES_OK && domainCheckRes.PQntuples() > 0 )
    {
      QString checkDefinition = domainCheckRes.PQgetvalue( 0, 0 );

      //we assume that the constraint is of the following form:
      //(VALUE = ANY (ARRAY['a'::text, 'b'::text, 'c'::text, 'd'::text]))
      //normally, PostgreSQL creates that if the contstraint has been specified as 'VALUE in ('a', 'b', 'c', 'd')

      int anyPos = checkDefinition.indexOf( QRegExp( "VALUE\\s*=\\s*ANY\\s*\\(\\s*ARRAY\\s*\\[" ) );
      int arrayPosition = checkDefinition.lastIndexOf( QLatin1String( "ARRAY[" ) );
      int closingBracketPos = checkDefinition.indexOf( ']', arrayPosition + 6 );

      if ( anyPos == -1 || anyPos >= arrayPosition )
      {
        return false; //constraint has not the required format
      }

      if ( arrayPosition != -1 )
      {
        QString valueList = checkDefinition.mid( arrayPosition + 6, closingBracketPos );
        QStringList commaSeparation = valueList.split( ',', QString::SkipEmptyParts );
        QStringList::const_iterator cIt = commaSeparation.constBegin();
        for ( ; cIt != commaSeparation.constEnd(); ++cIt )
        {
          //get string between ''
          int beginQuotePos = cIt->indexOf( '\'' );
          int endQuotePos = cIt->lastIndexOf( '\'' );
          if ( beginQuotePos != -1 && ( endQuotePos - beginQuotePos ) > 1 )
          {
            enumValues << cIt->mid( beginQuotePos + 1, endQuotePos - beginQuotePos - 1 );
          }
        }
      }
      return true;
    }
  }
  return false;
}

// Returns the maximum value of an attribute
QVariant QgsPostgresProvider::maximumValue( int index ) const
{
  try
  {
    // get the field name
    QgsField fld = field( index );
    QString sql = QStringLiteral( "SELECT max(%1) AS %1 FROM %2" )
                  .arg( quotedIdentifier( fld.name() ),
                        mQuery );

    if ( !mSqlWhereClause.isEmpty() )
    {
      sql += QStringLiteral( " WHERE %1" ).arg( mSqlWhereClause );
    }

    sql = QStringLiteral( "SELECT %1 FROM (%2) foo" ).arg( connectionRO()->fieldExpression( fld ), sql );

    QgsPostgresResult rmax( connectionRO()->PQexec( sql ) );

    return convertValue( fld.type(), fld.subType(), rmax.PQgetvalue( 0, 0 ), fld.typeName() );
  }
  catch ( PGFieldNotFound )
  {
    return QVariant( QString() );
  }
}


bool QgsPostgresProvider::isValid() const
{
  return mValid;
}

QString QgsPostgresProvider::defaultValueClause( int fieldId ) const
{
  QString defVal = mDefaultValues.value( fieldId, QString() );

  if ( !providerProperty( EvaluateDefaultValues, false ).toBool() && !defVal.isEmpty() )
  {
    return defVal;
  }

  return QString();
}

QVariant QgsPostgresProvider::defaultValue( int fieldId ) const
{
  QString defVal = mDefaultValues.value( fieldId, QString() );

  if ( providerProperty( EvaluateDefaultValues, false ).toBool() && !defVal.isEmpty() )
  {
    QgsField fld = field( fieldId );

    QgsPostgresResult res( connectionRO()->PQexec( QStringLiteral( "SELECT %1" ).arg( defVal ) ) );

    if ( res.result() )
      return convertValue( fld.type(), fld.subType(), res.PQgetvalue( 0, 0 ), fld.typeName() );
    else
    {
      pushError( tr( "Could not execute query" ) );
      return QVariant();
    }
  }

  return QVariant();
}

bool QgsPostgresProvider::skipConstraintCheck( int fieldIndex, QgsFieldConstraints::Constraint, const QVariant &value ) const
{
  if ( providerProperty( EvaluateDefaultValues, false ).toBool() )
  {
    return !mDefaultValues.value( fieldIndex ).isEmpty();
  }
  else
  {
    // stricter check - if we are evaluating default values only on commit then we can only bypass the check
    // if the attribute values matches the original default clause
    return mDefaultValues.contains( fieldIndex ) && mDefaultValues.value( fieldIndex ) == value.toString() && !value.isNull();
  }
}

QString QgsPostgresProvider::paramValue( const QString &fieldValue, const QString &defaultValue ) const
{
  if ( fieldValue.isNull() )
    return QString();

  if ( fieldValue == defaultValue && !defaultValue.isNull() )
  {
    QgsPostgresResult result( connectionRO()->PQexec( QStringLiteral( "SELECT %1" ).arg( defaultValue ) ) );
    if ( result.PQresultStatus() != PGRES_TUPLES_OK )
      throw PGException( result );

    return result.PQgetvalue( 0, 0 );
  }

  return fieldValue;
}


/* private */
bool QgsPostgresProvider::getTopoLayerInfo()
{
  QString sql = QString( "SELECT t.name, l.layer_id "
                         "FROM topology.layer l, topology.topology t "
                         "WHERE l.topology_id = t.id AND l.schema_name=%1 "
                         "AND l.table_name=%2 AND l.feature_column=%3" )
                .arg( quotedValue( mSchemaName ),
                      quotedValue( mTableName ),
                      quotedValue( mGeometryColumn ) );
  QgsPostgresResult result( connectionRO()->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    throw PGException( result ); // we should probably not do this
  }
  if ( result.PQntuples() < 1 )
  {
    QgsMessageLog::logMessage( tr( "Could not find topology of layer %1.%2.%3" )
                               .arg( quotedValue( mSchemaName ),
                                     quotedValue( mTableName ),
                                     quotedValue( mGeometryColumn ) ),
                               tr( "PostGIS" ) );
    return false;
  }
  mTopoLayerInfo.topologyName = result.PQgetvalue( 0, 0 );
  mTopoLayerInfo.layerId = result.PQgetvalue( 0, 1 ).toLong();
  return true;
}

/* private */
void QgsPostgresProvider::dropOrphanedTopoGeoms()
{
  QString sql = QString( "DELETE FROM %1.relation WHERE layer_id = %2 AND "
                         "topogeo_id NOT IN ( SELECT id(%3) FROM %4.%5 )" )
                .arg( quotedIdentifier( mTopoLayerInfo.topologyName ) )
                .arg( mTopoLayerInfo.layerId )
                .arg( quotedIdentifier( mGeometryColumn ),
                      quotedIdentifier( mSchemaName ),
                      quotedIdentifier( mTableName ) )
                ;

  QgsDebugMsg( "TopoGeom orphans cleanup query: " + sql );

  connectionRW()->PQexecNR( sql );
}

QString QgsPostgresProvider::geomParam( int offset ) const
{
  QString geometry;

  bool forceMulti = false;

  if ( mSpatialColType != SctTopoGeometry )
  {
    forceMulti = QgsWkbTypes::isMultiType( wkbType() );
  }

  if ( mSpatialColType == SctTopoGeometry )
  {
    geometry += QStringLiteral( "toTopoGeom(" );
  }

  if ( forceMulti )
  {
    geometry += connectionRO()->majorVersion() < 2 ? "multi(" : "st_multi(";
  }

  geometry += QStringLiteral( "%1($%2%3,%4)" )
              .arg( connectionRO()->majorVersion() < 2 ? "geomfromwkb" : "st_geomfromwkb" )
              .arg( offset )
              .arg( connectionRO()->useWkbHex() ? "" : "::bytea",
                    mRequestedSrid.isEmpty() ? mDetectedSrid : mRequestedSrid );

  if ( forceMulti )
  {
    geometry += ')';
  }

  if ( mSpatialColType == SctTopoGeometry )
  {
    geometry += QStringLiteral( ",%1,%2)" )
                .arg( quotedValue( mTopoLayerInfo.topologyName ) )
                .arg( mTopoLayerInfo.layerId );
  }

  return geometry;
}

bool QgsPostgresProvider::addFeatures( QgsFeatureList &flist, Flags flags )
{
  if ( flist.isEmpty() )
    return true;

  if ( mIsQuery )
    return false;

  QgsPostgresConn *conn = connectionRW();
  if ( !conn )
  {
    return false;
  }
  conn->lock();

  bool returnvalue = true;

  try
  {
    conn->begin();

    // Prepare the INSERT statement
    QString insert = QStringLiteral( "INSERT INTO %1(" ).arg( mQuery );
    QString values = QStringLiteral( ") VALUES (" );
    QString delim;
    int offset = 1;

    QStringList defaultValues;
    QList<int> fieldId;

    if ( !mGeometryColumn.isNull() )
    {
      insert += quotedIdentifier( mGeometryColumn );

      values += geomParam( offset++ );

      delim = ',';
    }

    // Optimization: if we have a single primary key column whose default value
    // is a sequence, and that none of the features have a value set for that
    // column, then we can completely omit inserting it.
    bool skipSinglePKField = false;

    if ( ( mPrimaryKeyType == PktInt || mPrimaryKeyType == PktFidMap || mPrimaryKeyType == PktUint64 ) )
    {
      if ( mPrimaryKeyAttrs.size() == 1 &&
           defaultValueClause( mPrimaryKeyAttrs[0] ).startsWith( "nextval(" ) )
      {
        bool foundNonNullPK = false;
        int idx = mPrimaryKeyAttrs[0];
        for ( int i = 0; i < flist.size(); i++ )
        {
          QgsAttributes attrs2 = flist[i].attributes();
          QVariant v2 = attrs2.value( idx, QVariant( QVariant::Int ) );
          if ( !v2.isNull() )
          {
            foundNonNullPK = true;
            break;
          }
        }
        skipSinglePKField = !foundNonNullPK;
      }

      if ( !skipSinglePKField )
      {
        for ( int idx : mPrimaryKeyAttrs )
        {
          insert += delim + quotedIdentifier( field( idx ).name() );
          values += delim + QStringLiteral( "$%1" ).arg( defaultValues.size() + offset );
          delim = ',';
          fieldId << idx;
          defaultValues << defaultValueClause( idx );
        }
      }
    }

    QgsAttributes attributevec = flist[0].attributes();

    // look for unique attribute values to place in statement instead of passing as parameter
    // e.g. for defaults
    for ( int idx = 0; idx < attributevec.count(); ++idx )
    {
      QVariant v = attributevec.value( idx, QVariant( QVariant::Int ) ); // default to NULL for missing attributes
      if ( skipSinglePKField && idx == mPrimaryKeyAttrs[0] )
        continue;
      if ( fieldId.contains( idx ) )
        continue;

      if ( idx >= mAttributeFields.count() )
        continue;

      QString fieldname = mAttributeFields.at( idx ).name();
      QString fieldTypeName = mAttributeFields.at( idx ).typeName();

      QgsDebugMsg( "Checking field against: " + fieldname );

      if ( fieldname.isEmpty() || fieldname == mGeometryColumn )
        continue;

      int i;
      for ( i = 1; i < flist.size(); i++ )
      {
        QgsAttributes attrs2 = flist[i].attributes();
        QVariant v2 = attrs2.value( idx, QVariant( QVariant::Int ) ); // default to NULL for missing attributes

        if ( v2 != v )
          break;
      }

      insert += delim + quotedIdentifier( fieldname );

      QString defVal = defaultValueClause( idx );

      if ( i == flist.size() )
      {
        if ( qgsVariantEqual( v, defVal ) )
        {
          if ( defVal.isNull() )
          {
            values += delim + "NULL";
          }
          else
          {
            values += delim + defVal;
          }
        }
        else if ( fieldTypeName == QLatin1String( "geometry" ) )
        {
          values += QStringLiteral( "%1%2(%3)" )
                    .arg( delim,
                          connectionRO()->majorVersion() < 2 ? "geomfromewkt" : "st_geomfromewkt",
                          quotedValue( v.toString() ) );
        }
        else if ( fieldTypeName == QLatin1String( "geography" ) )
        {
          values += QStringLiteral( "%1st_geographyfromewkt(%2)" )
                    .arg( delim,
                          quotedValue( v.toString() ) );
        }
        //TODO: convert arrays and hstore to native types
        else
        {
          //this should be for json/jsonb in future
          values += delim + quotedValue( v );
        }
      }
      else
      {
        // value is not unique => add parameter
        if ( fieldTypeName == QLatin1String( "geometry" ) )
        {
          values += QStringLiteral( "%1%2($%3)" )
                    .arg( delim,
                          connectionRO()->majorVersion() < 2 ? "geomfromewkt" : "st_geomfromewkt" )
                    .arg( defaultValues.size() + offset );
        }
        else if ( fieldTypeName == QLatin1String( "geography" ) )
        {
          values += QStringLiteral( "%1st_geographyfromewkt($%2)" )
                    .arg( delim )
                    .arg( defaultValues.size() + offset );
        }
        else
        {
          values += QStringLiteral( "%1$%2" )
                    .arg( delim )
                    .arg( defaultValues.size() + offset );
        }
        defaultValues.append( defVal );
        fieldId.append( idx );
      }

      delim = ',';
    }

    insert += values + ')';

    if ( !( flags & QgsFeatureSink::FastInsert ) )
    {
      if ( mPrimaryKeyType == PktFidMap || mPrimaryKeyType == PktInt || mPrimaryKeyType == PktUint64 )
      {
        insert += QLatin1String( " RETURNING " );

        QString delim;
        Q_FOREACH ( int idx, mPrimaryKeyAttrs )
        {
          insert += delim + quotedIdentifier( mAttributeFields.at( idx ).name() );
          delim = ',';
        }
      }
    }

    QgsDebugMsg( QStringLiteral( "prepare addfeatures: %1" ).arg( insert ) );
    QgsPostgresResult stmt( conn->PQprepare( QStringLiteral( "addfeatures" ), insert, fieldId.size() + offset - 1, nullptr ) );

    if ( stmt.PQresultStatus() != PGRES_COMMAND_OK )
      throw PGException( stmt );

    for ( QgsFeatureList::iterator features = flist.begin(); features != flist.end(); ++features )
    {
      QgsAttributes attrs = features->attributes();

      QStringList params;
      if ( !mGeometryColumn.isNull() )
      {
        appendGeomParam( features->geometry(), params );
      }

      params.reserve( fieldId.size() );
      for ( int i = 0; i < fieldId.size(); i++ )
      {
        int attrIdx = fieldId[i];
        QVariant value = attrIdx < attrs.length() ? attrs.at( attrIdx ) : QVariant( QVariant::Int );

        QString v;
        if ( value.isNull() )
        {
          QgsField fld = field( attrIdx );
          v = paramValue( defaultValues[ i ], defaultValues[ i ] );
          features->setAttribute( attrIdx, convertValue( fld.type(), fld.subType(), v, fld.typeName() ) );
        }
        else
        {
          v = paramValue( value.toString(), defaultValues[ i ] );

          if ( v != value.toString() )
          {
            QgsField fld = field( attrIdx );
            features->setAttribute( attrIdx, convertValue( fld.type(), fld.subType(), v, fld.typeName() ) );
          }
        }

        params << v;
      }

      QgsPostgresResult result( conn->PQexecPrepared( QStringLiteral( "addfeatures" ), params ) );

      if ( !( flags & QgsFeatureSink::FastInsert ) && result.PQresultStatus() == PGRES_TUPLES_OK )
      {
        for ( int i = 0; i < mPrimaryKeyAttrs.size(); ++i )
        {
          const int idx = mPrimaryKeyAttrs.at( i );
          const QgsField fld = mAttributeFields.at( idx );
          features->setAttribute( idx, convertValue( fld.type(), fld.subType(), result.PQgetvalue( 0, i ), fld.typeName() ) );
        }
      }
      else if ( result.PQresultStatus() != PGRES_COMMAND_OK )
        throw PGException( result );

      if ( !( flags & QgsFeatureSink::FastInsert ) && mPrimaryKeyType == PktOid )
      {
        features->setId( result.PQoidValue() );
        QgsDebugMsgLevel( QStringLiteral( "new fid=%1" ).arg( features->id() ), 4 );
      }
    }

    if ( !( flags & QgsFeatureSink::FastInsert ) )
    {
      // update feature ids
      if ( mPrimaryKeyType == PktInt || mPrimaryKeyType == PktFidMap || mPrimaryKeyType == PktUint64 )
      {
        for ( QgsFeatureList::iterator features = flist.begin(); features != flist.end(); ++features )
        {
          QgsAttributes attrs = features->attributes();

          if ( mPrimaryKeyType == PktUint64 )
          {
            features->setId( STRING_TO_FID( attrs.at( mPrimaryKeyAttrs.at( 0 ) ) ) );
          }
          else if ( mPrimaryKeyType == PktInt )
          {
            features->setId( PKINT2FID( STRING_TO_FID( attrs.at( mPrimaryKeyAttrs.at( 0 ) ) ) ) );
          }
          else
          {
            QVariantList primaryKeyVals;

            Q_FOREACH ( int idx, mPrimaryKeyAttrs )
            {
              primaryKeyVals << attrs.at( idx );
            }

            features->setId( mShared->lookupFid( primaryKeyVals ) );
          }
          QgsDebugMsgLevel( QStringLiteral( "new fid=%1" ).arg( features->id() ), 4 );
        }
      }
    }

    conn->PQexecNR( QStringLiteral( "DEALLOCATE addfeatures" ) );

    returnvalue &= conn->commit();
    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();

    mShared->addFeaturesCounted( flist.size() );
  }
  catch ( PGException &e )
  {
    pushError( tr( "PostGIS error while adding features: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    conn->PQexecNR( QStringLiteral( "DEALLOCATE addfeatures" ) );
    returnvalue = false;
  }

  conn->unlock();
  return returnvalue;
}

bool QgsPostgresProvider::deleteFeatures( const QgsFeatureIds &id )
{
  bool returnvalue = true;

  if ( mIsQuery )
  {
    QgsDebugMsg( QStringLiteral( "Cannot delete features (is a query)" ) );
    return false;
  }

  QgsPostgresConn *conn = connectionRW();
  if ( !conn )
  {
    return false;
  }
  conn->lock();

  try
  {
    conn->begin();

    for ( QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it )
    {
      QString sql = QStringLiteral( "DELETE FROM %1 WHERE %2" )
                    .arg( mQuery, whereClause( *it ) );
      QgsDebugMsg( "delete sql: " + sql );

      //send DELETE statement and do error handling
      QgsPostgresResult result( conn->PQexec( sql ) );
      if ( result.PQresultStatus() != PGRES_COMMAND_OK && result.PQresultStatus() != PGRES_TUPLES_OK )
        throw PGException( result );

      mShared->removeFid( *it );
    }

    returnvalue &= conn->commit();
    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();

    if ( mSpatialColType == SctTopoGeometry )
    {
      // NOTE: in presence of multiple TopoGeometry objects
      //       for the same table or when deleting a Geometry
      //       layer _also_ having a TopoGeometry component,
      //       orphans would still be left.
      // TODO: decouple layer from table and signal table when
      //       records are added or removed
      dropOrphanedTopoGeoms();
    }

    mShared->addFeaturesCounted( -id.size() );
  }
  catch ( PGException &e )
  {
    pushError( tr( "PostGIS error while deleting features: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  conn->unlock();
  return returnvalue;
}

bool QgsPostgresProvider::truncate()
{
  bool returnvalue = true;

  if ( mIsQuery )
  {
    QgsDebugMsg( QStringLiteral( "Cannot truncate (is a query)" ) );
    return false;
  }

  QgsPostgresConn *conn = connectionRW();
  if ( !conn )
  {
    return false;
  }
  conn->lock();

  try
  {
    conn->begin();

    QString sql = QStringLiteral( "TRUNCATE %1" ).arg( mQuery );
    QgsDebugMsg( "truncate sql: " + sql );

    //send truncate statement and do error handling
    QgsPostgresResult result( conn->PQexec( sql ) );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK && result.PQresultStatus() != PGRES_TUPLES_OK )
      throw PGException( result );

    returnvalue &= conn->commit();
    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();

    if ( returnvalue )
    {
      if ( mSpatialColType == SctTopoGeometry )
      {
        // NOTE: in presence of multiple TopoGeometry objects
        //       for the same table or when deleting a Geometry
        //       layer _also_ having a TopoGeometry component,
        //       orphans would still be left.
        // TODO: decouple layer from table and signal table when
        //       records are added or removed
        dropOrphanedTopoGeoms();
      }
      mShared->clear();
    }
  }
  catch ( PGException &e )
  {
    pushError( tr( "PostGIS error while truncating: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  conn->unlock();
  return returnvalue;
}

bool QgsPostgresProvider::addAttributes( const QList<QgsField> &attributes )
{
  bool returnvalue = true;

  if ( mIsQuery )
    return false;

  if ( attributes.isEmpty() )
    return true;

  QgsPostgresConn *conn = connectionRW();
  if ( !conn )
  {
    return false;
  }
  conn->lock();

  try
  {
    conn->begin();

    QString delim;
    QString sql = QStringLiteral( "ALTER TABLE %1 " ).arg( mQuery );
    for ( QList<QgsField>::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter )
    {
      QString type = iter->typeName();
      if ( type == QLatin1String( "char" ) || type == QLatin1String( "varchar" ) )
      {
        if ( iter->length() > 0 )
          type = QStringLiteral( "%1(%2)" ).arg( type ).arg( iter->length() );
      }
      else if ( type == QLatin1String( "numeric" ) || type == QLatin1String( "decimal" ) )
      {
        if ( iter->length() > 0 && iter->precision() >= 0 )
          type = QStringLiteral( "%1(%2,%3)" ).arg( type ).arg( iter->length() ).arg( iter->precision() );
      }
      sql.append( QStringLiteral( "%1ADD COLUMN %2 %3" ).arg( delim, quotedIdentifier( iter->name() ), type ) );
      delim = ',';
    }

    //send sql statement and do error handling
    QgsPostgresResult result( conn->PQexec( sql ) );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK )
      throw PGException( result );

    for ( QList<QgsField>::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter )
    {
      if ( !iter->comment().isEmpty() )
      {
        sql = QStringLiteral( "COMMENT ON COLUMN %1.%2 IS %3" )
              .arg( mQuery,
                    quotedIdentifier( iter->name() ),
                    quotedValue( iter->comment() ) );
        result = conn->PQexec( sql );
        if ( result.PQresultStatus() != PGRES_COMMAND_OK )
          throw PGException( result );
      }
    }

    returnvalue &= conn->commit();
    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();
  }
  catch ( PGException &e )
  {
    pushError( tr( "PostGIS error while adding attributes: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  loadFields();
  conn->unlock();
  return returnvalue;
}

bool QgsPostgresProvider::deleteAttributes( const QgsAttributeIds &ids )
{
  bool returnvalue = true;

  if ( mIsQuery )
    return false;

  QgsPostgresConn *conn = connectionRW();
  if ( !conn )
  {
    return false;
  }
  conn->lock();

  try
  {
    conn->begin();

    QList<int> idsList = ids.values();
    std::sort( idsList.begin(), idsList.end(), std::greater<int>() );

    for ( auto iter = idsList.constBegin(); iter != idsList.constEnd(); ++iter )
    {
      int index = *iter;
      if ( index < 0 || index >= mAttributeFields.count() )
        continue;

      QString column = mAttributeFields.at( index ).name();
      QString sql = QStringLiteral( "ALTER TABLE %1 DROP COLUMN %2" )
                    .arg( mQuery,
                          quotedIdentifier( column ) );

      //send sql statement and do error handling
      QgsPostgresResult result( conn->PQexec( sql ) );
      if ( result.PQresultStatus() != PGRES_COMMAND_OK )
        throw PGException( result );

      //delete the attribute from mAttributeFields
      mAttributeFields.remove( index );
    }

    returnvalue &= conn->commit();
    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();
  }
  catch ( PGException &e )
  {
    pushError( tr( "PostGIS error while deleting attributes: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  loadFields();
  conn->unlock();
  return returnvalue;
}

bool QgsPostgresProvider::renameAttributes( const QgsFieldNameMap &renamedAttributes )
{
  if ( mIsQuery )
    return false;


  QString sql = QStringLiteral( "BEGIN;" );

  QgsFieldNameMap::const_iterator renameIt = renamedAttributes.constBegin();
  bool returnvalue = true;
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

    sql += QStringLiteral( "ALTER TABLE %1 RENAME COLUMN %2 TO %3;" )
           .arg( mQuery,
                 quotedIdentifier( mAttributeFields.at( fieldIndex ).name() ),
                 quotedIdentifier( renameIt.value() ) );
  }
  sql += QLatin1String( "COMMIT;" );

  QgsPostgresConn *conn = connectionRW();
  if ( !conn )
  {
    return false;
  }
  conn->lock();

  try
  {
    conn->begin();
    //send sql statement and do error handling
    QgsPostgresResult result( conn->PQexec( sql ) );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK )
      throw PGException( result );
    returnvalue = conn->commit();
    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();
  }
  catch ( PGException &e )
  {
    pushError( tr( "PostGIS error while renaming attributes: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  loadFields();
  conn->unlock();
  return returnvalue;
}

bool QgsPostgresProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  bool returnvalue = true;

  if ( mIsQuery )
    return false;

  if ( attr_map.isEmpty() )
    return true;

  QgsPostgresConn *conn = connectionRW();
  if ( !conn )
    return false;

  conn->lock();

  try
  {
    conn->begin();

    // cycle through the features
    for ( QgsChangedAttributesMap::const_iterator iter = attr_map.constBegin(); iter != attr_map.constEnd(); ++iter )
    {
      QgsFeatureId fid = iter.key();

      // skip added features
      if ( FID_IS_NEW( fid ) )
        continue;

      const QgsAttributeMap &attrs = iter.value();
      if ( attrs.isEmpty() )
        continue;

      QString sql = QStringLiteral( "UPDATE %1 SET " ).arg( mQuery );

      bool pkChanged = false;

      // cycle through the changed attributes of the feature
      QString delim;
      for ( QgsAttributeMap::const_iterator siter = attrs.constBegin(); siter != attrs.constEnd(); ++siter )
      {
        try
        {
          QgsField fld = field( siter.key() );

          pkChanged = pkChanged || mPrimaryKeyAttrs.contains( siter.key() );

          sql += delim + QStringLiteral( "%1=" ).arg( quotedIdentifier( fld.name() ) );
          delim = ',';

          if ( fld.typeName() == QLatin1String( "geometry" ) )
          {
            sql += QStringLiteral( "%1(%2)" )
                   .arg( connectionRO()->majorVersion() < 2 ? "geomfromewkt" : "st_geomfromewkt",
                         quotedValue( siter->toString() ) );
          }
          else if ( fld.typeName() == QLatin1String( "geography" ) )
          {
            sql += QStringLiteral( "st_geographyfromewkt(%1)" )
                   .arg( quotedValue( siter->toString() ) );
          }
          else
          {
            sql += quotedValue( *siter );
          }
        }
        catch ( PGFieldNotFound )
        {
          // Field was missing - shouldn't happen
        }
      }

      sql += QStringLiteral( " WHERE %1" ).arg( whereClause( fid ) );

      QgsPostgresResult result( conn->PQexec( sql ) );
      if ( result.PQresultStatus() != PGRES_COMMAND_OK && result.PQresultStatus() != PGRES_TUPLES_OK )
        throw PGException( result );

      // update feature id map if key was changed
      if ( pkChanged && mPrimaryKeyType == PktFidMap )
      {
        QVariantList k = mShared->removeFid( fid );

        for ( int i = 0; i < mPrimaryKeyAttrs.size(); i++ )
        {
          int idx = mPrimaryKeyAttrs.at( i );
          if ( !attrs.contains( idx ) )
            continue;

          k[i] = attrs[ idx ];
        }

        mShared->insertFid( fid, k );
      }
    }

    returnvalue &= conn->commit();
    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();
  }
  catch ( PGException &e )
  {
    pushError( tr( "PostGIS error while changing attributes: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  conn->unlock();
  return returnvalue;
}

void QgsPostgresProvider::appendGeomParam( const QgsGeometry &geom, QStringList &params ) const
{
  if ( geom.isNull() )
  {
    params << QString();
    return;
  }

  QString param;

  QgsGeometry convertedGeom( convertToProviderType( geom ) );
  QByteArray wkb( !convertedGeom.isNull() ? convertedGeom.asWkb() : geom.asWkb() );
  const unsigned char *buf = reinterpret_cast< const unsigned char * >( wkb.constData() );
  int wkbSize = wkb.length();

  for ( int i = 0; i < wkbSize; ++i )
  {
    if ( connectionRO()->useWkbHex() )
      param += QStringLiteral( "%1" ).arg( ( int ) buf[i], 2, 16, QChar( '0' ) );
    else
      param += QStringLiteral( "\\%1" ).arg( ( int ) buf[i], 3, 8, QChar( '0' ) );
  }
  params << param;
}

bool QgsPostgresProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{

  if ( mIsQuery || mGeometryColumn.isNull() )
    return false;

  QgsPostgresConn *conn = connectionRW();
  if ( !conn )
  {
    return false;
  }
  conn->lock();

  bool returnvalue = true;

  try
  {
    // Start the PostGIS transaction
    conn->begin();

    QString update;
    QgsPostgresResult result;

    if ( mSpatialColType == SctTopoGeometry )
    {
      // We will create a new TopoGeometry object with the new shape.
      // Later, we'll replace the old TopoGeometry with the new one,
      // to avoid orphans and retain higher level in an eventual
      // hierarchical definition
      update = QStringLiteral( "SELECT id(%1) FROM %2 o WHERE %3" )
               .arg( geomParam( 1 ),
                     mQuery,
                     pkParamWhereClause( 2 ) );

      QString getid = QStringLiteral( "SELECT id(%1) FROM %2 WHERE %3" )
                      .arg( quotedIdentifier( mGeometryColumn ),
                            mQuery,
                            pkParamWhereClause( 1 ) );

      QgsDebugMsg( "getting old topogeometry id: " + getid );

      result = connectionRO()->PQprepare( QStringLiteral( "getid" ), getid, 1, nullptr );
      if ( result.PQresultStatus() != PGRES_COMMAND_OK )
      {
        QgsDebugMsg( QStringLiteral( "Exception thrown due to PQprepare of this query returning != PGRES_COMMAND_OK (%1 != expected %2): %3" )
                     .arg( result.PQresultStatus() ).arg( PGRES_COMMAND_OK ).arg( getid ) );
        throw PGException( result );
      }

      QString replace = QString( "UPDATE %1 SET %2="
                                 "( topology_id(%2),layer_id(%2),$1,type(%2) )"
                                 "WHERE %3" )
                        .arg( mQuery,
                              quotedIdentifier( mGeometryColumn ),
                              pkParamWhereClause( 2 ) );
      QgsDebugMsg( "TopoGeom swap: " + replace );
      result = conn->PQprepare( QStringLiteral( "replacetopogeom" ), replace, 2, nullptr );
      if ( result.PQresultStatus() != PGRES_COMMAND_OK )
      {
        QgsDebugMsg( QStringLiteral( "Exception thrown due to PQprepare of this query returning != PGRES_COMMAND_OK (%1 != expected %2): %3" )
                     .arg( result.PQresultStatus() ).arg( PGRES_COMMAND_OK ).arg( replace ) );
        throw PGException( result );
      }

    }
    else
    {
      update = QStringLiteral( "UPDATE %1 SET %2=%3 WHERE %4" )
               .arg( mQuery,
                     quotedIdentifier( mGeometryColumn ),
                     geomParam( 1 ),
                     pkParamWhereClause( 2 ) );
    }

    QgsDebugMsg( "updating: " + update );

    result = conn->PQprepare( QStringLiteral( "updatefeatures" ), update, 2, nullptr );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK && result.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsDebugMsg( QStringLiteral( "Exception thrown due to PQprepare of this query returning != PGRES_COMMAND_OK (%1 != expected %2): %3" )
                   .arg( result.PQresultStatus() ).arg( PGRES_COMMAND_OK ).arg( update ) );
      throw PGException( result );
    }

    QgsDebugMsg( QStringLiteral( "iterating over the map of changed geometries..." ) );

    for ( QgsGeometryMap::const_iterator iter = geometry_map.constBegin();
          iter != geometry_map.constEnd();
          ++iter )
    {
      QgsDebugMsg( "iterating over feature id " + FID_TO_STRING( iter.key() ) );

      // Save the id of the current topogeometry
      long old_tg_id = -1;
      if ( mSpatialColType == SctTopoGeometry )
      {
        QStringList params;
        appendPkParams( iter.key(), params );
        result = connectionRO()->PQexecPrepared( QStringLiteral( "getid" ), params );
        if ( result.PQresultStatus() != PGRES_TUPLES_OK )
        {
          QgsDebugMsg( QStringLiteral( "Exception thrown due to PQexecPrepared of 'getid' returning != PGRES_TUPLES_OK (%1 != expected %2)" )
                       .arg( result.PQresultStatus() ).arg( PGRES_TUPLES_OK ) );
          throw PGException( result );
        }
        // TODO: watch out for NULL, handle somehow
        old_tg_id = result.PQgetvalue( 0, 0 ).toLong();
        QgsDebugMsg( QStringLiteral( "Old TG id is %1" ).arg( old_tg_id ) );
      }

      QStringList params;
      appendGeomParam( *iter, params );
      appendPkParams( iter.key(), params );

      result = conn->PQexecPrepared( QStringLiteral( "updatefeatures" ), params );
      if ( result.PQresultStatus() != PGRES_COMMAND_OK && result.PQresultStatus() != PGRES_TUPLES_OK )
        throw PGException( result );

      if ( mSpatialColType == SctTopoGeometry )
      {
        long new_tg_id = result.PQgetvalue( 0, 0 ).toLong(); // new topogeo_id

        // Replace old TopoGeom with new TopoGeom, so that
        // any hierarchically defined TopoGeom will still have its
        // definition and we'll leave no orphans
        QString replace = QString( "DELETE FROM %1.relation WHERE "
                                   "layer_id = %2 AND topogeo_id = %3" )
                          .arg( quotedIdentifier( mTopoLayerInfo.topologyName ) )
                          .arg( mTopoLayerInfo.layerId )
                          .arg( old_tg_id );
        result = conn->PQexec( replace );
        if ( result.PQresultStatus() != PGRES_COMMAND_OK )
        {
          QgsDebugMsg( QStringLiteral( "Exception thrown due to PQexec of this query returning != PGRES_COMMAND_OK (%1 != expected %2): %3" )
                       .arg( result.PQresultStatus() ).arg( PGRES_COMMAND_OK ).arg( replace ) );
          throw PGException( result );
        }
        // TODO: use prepared query here
        replace = QString( "UPDATE %1.relation SET topogeo_id = %2 "
                           "WHERE layer_id = %3 AND topogeo_id = %4" )
                  .arg( quotedIdentifier( mTopoLayerInfo.topologyName ) )
                  .arg( old_tg_id )
                  .arg( mTopoLayerInfo.layerId )
                  .arg( new_tg_id );
        QgsDebugMsg( "relation swap: " + replace );
        result = conn->PQexec( replace );
        if ( result.PQresultStatus() != PGRES_COMMAND_OK )
        {
          QgsDebugMsg( QStringLiteral( "Exception thrown due to PQexec of this query returning != PGRES_COMMAND_OK (%1 != expected %2): %3" )
                       .arg( result.PQresultStatus() ).arg( PGRES_COMMAND_OK ).arg( replace ) );
          throw PGException( result );
        }
      } // if TopoGeometry

    } // for each feature

    conn->PQexecNR( QStringLiteral( "DEALLOCATE updatefeatures" ) );
    if ( mSpatialColType == SctTopoGeometry )
    {
      connectionRO()->PQexecNR( QStringLiteral( "DEALLOCATE getid" ) );
      conn->PQexecNR( QStringLiteral( "DEALLOCATE replacetopogeom" ) );
    }

    returnvalue &= conn->commit();
    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();
  }
  catch ( PGException &e )
  {
    pushError( tr( "PostGIS error while changing geometry values: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    conn->PQexecNR( QStringLiteral( "DEALLOCATE updatefeatures" ) );
    if ( mSpatialColType == SctTopoGeometry )
    {
      connectionRO()->PQexecNR( QStringLiteral( "DEALLOCATE getid" ) );
      conn->PQexecNR( QStringLiteral( "DEALLOCATE replacetopogeom" ) );
    }
    returnvalue = false;
  }

  conn->unlock();

  QgsDebugMsg( QStringLiteral( "leaving." ) );

  return returnvalue;
}

bool QgsPostgresProvider::changeFeatures( const QgsChangedAttributesMap &attr_map,
    const QgsGeometryMap &geometry_map )
{
  Q_ASSERT( mSpatialColType != SctTopoGeometry );

  bool returnvalue = true;

  if ( mIsQuery )
    return false;

  if ( attr_map.isEmpty() )
    return true;

  QgsPostgresConn *conn = connectionRW();
  if ( !conn )
    return false;

  conn->lock();

  try
  {
    conn->begin();

    QgsFeatureIds ids( attr_map.keys().toSet() );
    ids |= geometry_map.keys().toSet();

    // cycle through the features
    Q_FOREACH ( QgsFeatureId fid, ids )
    {
      // skip added features
      if ( FID_IS_NEW( fid ) )
        continue;

      const QgsAttributeMap &attrs = attr_map.value( fid );
      if ( attrs.isEmpty() && !geometry_map.contains( fid ) )
        continue;

      QString sql = QStringLiteral( "UPDATE %1 SET " ).arg( mQuery );

      bool pkChanged = false;

      // cycle through the changed attributes of the feature
      QString delim;
      for ( QgsAttributeMap::const_iterator siter = attrs.constBegin(); siter != attrs.constEnd(); ++siter )
      {
        try
        {
          QgsField fld = field( siter.key() );

          pkChanged = pkChanged || mPrimaryKeyAttrs.contains( siter.key() );

          sql += delim + QStringLiteral( "%1=" ).arg( quotedIdentifier( fld.name() ) );
          delim = ',';

          if ( fld.typeName() == QLatin1String( "geometry" ) )
          {
            sql += QStringLiteral( "%1(%2)" )
                   .arg( connectionRO()->majorVersion() < 2 ? "geomfromewkt" : "st_geomfromewkt",
                         quotedValue( siter->toString() ) );
          }
          else if ( fld.typeName() == QLatin1String( "geography" ) )
          {
            sql += QStringLiteral( "st_geographyfromewkt(%1)" )
                   .arg( quotedValue( siter->toString() ) );
          }
          else
          {
            sql += quotedValue( *siter );
          }
        }
        catch ( PGFieldNotFound )
        {
          // Field was missing - shouldn't happen
        }
      }

      if ( !geometry_map.contains( fid ) )
      {
        sql += QStringLiteral( " WHERE %1" ).arg( whereClause( fid ) );

        QgsPostgresResult result( conn->PQexec( sql ) );
        if ( result.PQresultStatus() != PGRES_COMMAND_OK && result.PQresultStatus() != PGRES_TUPLES_OK )
          throw PGException( result );
      }
      else
      {
        sql += QStringLiteral( "%1%2=%3" ).arg( delim, quotedIdentifier( mGeometryColumn ), geomParam( 1 ) );
        sql += QStringLiteral( " WHERE %1" ).arg( whereClause( fid ) );

        QgsPostgresResult result( conn->PQprepare( QStringLiteral( "updatefeature" ), sql, 1, nullptr ) );
        if ( result.PQresultStatus() != PGRES_COMMAND_OK && result.PQresultStatus() != PGRES_TUPLES_OK )
        {
          QgsDebugMsg( QStringLiteral( "Exception thrown due to PQprepare of this query returning != PGRES_COMMAND_OK (%1 != expected %2): %3" )
                       .arg( result.PQresultStatus() ).arg( PGRES_COMMAND_OK ).arg( sql ) );
          throw PGException( result );
        }

        QStringList params;
        const QgsGeometry &geom = geometry_map[ fid ];
        appendGeomParam( geom, params );

        result = conn->PQexecPrepared( QStringLiteral( "updatefeature" ), params );
        if ( result.PQresultStatus() != PGRES_COMMAND_OK && result.PQresultStatus() != PGRES_TUPLES_OK )
          throw PGException( result );

        conn->PQexecNR( QStringLiteral( "DEALLOCATE updatefeature" ) );
      }

      // update feature id map if key was changed
      if ( pkChanged && mPrimaryKeyType == PktFidMap )
      {
        QVariantList k = mShared->removeFid( fid );

        for ( int i = 0; i < mPrimaryKeyAttrs.size(); i++ )
        {
          int idx = mPrimaryKeyAttrs.at( i );
          if ( !attrs.contains( idx ) )
            continue;

          k[i] = attrs[ idx ];
        }

        mShared->insertFid( fid, k );
      }
    }

    returnvalue &= conn->commit();
    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();
  }
  catch ( PGException &e )
  {
    pushError( tr( "PostGIS error while changing attributes: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  conn->unlock();

  QgsDebugMsg( QStringLiteral( "leaving." ) );

  return returnvalue;
}

QgsAttributeList QgsPostgresProvider::attributeIndexes() const
{
  QgsAttributeList lst;
  lst.reserve( mAttributeFields.count() );
  for ( int i = 0; i < mAttributeFields.count(); ++i )
    lst.append( i );
  return lst;
}

QgsVectorDataProvider::Capabilities QgsPostgresProvider::capabilities() const
{
  return mEnabledCapabilities;
}

bool QgsPostgresProvider::setSubsetString( const QString &theSQL, bool updateFeatureCount )
{
  if ( theSQL.trimmed() == mSqlWhereClause )
    return true;

  QString prevWhere = mSqlWhereClause;

  mSqlWhereClause = theSQL.trimmed();

  QString sql = QStringLiteral( "SELECT * FROM %1" ).arg( mQuery );

  if ( !mSqlWhereClause.isEmpty() )
  {
    sql += QStringLiteral( " WHERE %1" ).arg( mSqlWhereClause );
  }

  sql += QLatin1String( " LIMIT 0" );

  QgsPostgresResult res( connectionRO()->PQexec( sql ) );
  if ( res.PQresultStatus() != PGRES_TUPLES_OK )
  {
    pushError( res.PQresultErrorMessage() );
    mSqlWhereClause = prevWhere;
    return false;
  }

#if 0
  // FIXME
  if ( mPrimaryKeyType == PktInt && !uniqueData( primaryKeyAttr ) )
  {
    sqlWhereClause = prevWhere;
    return false;
  }
#endif

  // Update datasource uri too
  mUri.setSql( theSQL );
  // Update yet another copy of the uri. Why are there 3 copies of the
  // uri? Perhaps this needs some rationalisation.....
  setDataSourceUri( mUri.uri( false ) );

  if ( updateFeatureCount )
  {
    mShared->setFeaturesCounted( -1 );
  }
  mLayerExtent.setMinimal();

  emit dataChanged();

  return true;
}

/**
 * Returns the feature count
 */
long QgsPostgresProvider::featureCount() const
{
  int featuresCounted = mShared->featuresCounted();
  if ( featuresCounted >= 0 )
    return featuresCounted;

  // See: https://issues.qgis.org/issues/17388 - QGIS crashes on featureCount())
  if ( ! connectionRO() )
  {
    return 0;
  }

  // get total number of features
  QString sql;

  // use estimated metadata even when there is a where clause,
  // although we get an incorrect feature count for the subset
  // - but make huge dataset usable.
  if ( !mIsQuery && mUseEstimatedMetadata )
  {
    sql = QStringLiteral( "SELECT reltuples::int FROM pg_catalog.pg_class WHERE oid=regclass(%1)::oid" ).arg( quotedValue( mQuery ) );
  }
  else
  {
    sql = QStringLiteral( "SELECT count(*) FROM %1%2" ).arg( mQuery, filterWhereClause() );
  }

  QgsPostgresResult result( connectionRO()->PQexec( sql ) );

  QgsDebugMsg( "number of features as text: " + result.PQgetvalue( 0, 0 ) );

  long num = result.PQgetvalue( 0, 0 ).toLong();
  mShared->setFeaturesCounted( num );

  QgsDebugMsg( "number of features: " + QString::number( num ) );

  return num;
}

bool QgsPostgresProvider::empty() const
{
  QString sql = QStringLiteral( "SELECT EXISTS (SELECT * FROM %1%2 LIMIT 1)" ).arg( mQuery, filterWhereClause() );
  QgsPostgresResult res( connectionRO()->PQexec( sql ) );
  if ( res.PQresultStatus() != PGRES_TUPLES_OK )
  {
    pushError( res.PQresultErrorMessage() );
    return false;
  }

  return res.PQgetvalue( 0, 0 ) != QLatin1String( "t" );
}

QgsRectangle QgsPostgresProvider::extent() const
{
  if ( mGeometryColumn.isNull() )
    return QgsRectangle();

  if ( mSpatialColType == SctGeography )
    return QgsRectangle( -180.0, -90.0, 180.0, 90.0 );

  if ( mLayerExtent.isEmpty() )
  {
    QString sql;
    QgsPostgresResult result;
    QString ext;

    // get the extents
    if ( !mIsQuery && ( mUseEstimatedMetadata || mSqlWhereClause.isEmpty() ) )
    {
      // do stats exists?
      sql = QStringLiteral( "SELECT count(*) FROM pg_stats WHERE schemaname=%1 AND tablename=%2 AND attname=%3" )
            .arg( quotedValue( mSchemaName ),
                  quotedValue( mTableName ),
                  quotedValue( mGeometryColumn ) );
      result = connectionRO()->PQexec( sql );
      if ( result.PQresultStatus() == PGRES_TUPLES_OK && result.PQntuples() == 1 )
      {
        if ( result.PQgetvalue( 0, 0 ).toInt() > 0 )
        {
          sql = QStringLiteral( "SELECT reltuples::int FROM pg_catalog.pg_class WHERE oid=regclass(%1)::oid" ).arg( quotedValue( mQuery ) );
          result = connectionRO()->PQexec( sql );
          if ( result.PQresultStatus() == PGRES_TUPLES_OK
               && result.PQntuples() == 1
               && result.PQgetvalue( 0, 0 ).toLong() > 0 )
          {
            sql = QStringLiteral( "SELECT %1(%2,%3,%4)" )
                  .arg( connectionRO()->majorVersion() < 2 ? "estimated_extent" :
                        ( connectionRO()->majorVersion() == 2 && connectionRO()->minorVersion() < 1 ? "st_estimated_extent" : "st_estimatedextent" ),
                        quotedValue( mSchemaName ),
                        quotedValue( mTableName ),
                        quotedValue( mGeometryColumn ) );
            result = mConnectionRO->PQexec( sql );
            if ( result.PQresultStatus() == PGRES_TUPLES_OK && result.PQntuples() == 1 && !result.PQgetisnull( 0, 0 ) )
            {
              ext = result.PQgetvalue( 0, 0 );

              // fix for what might be a PostGIS bug: when the extent crosses the
              // dateline extent() returns -180 to 180 (which appears right), but
              // estimated_extent() returns eastern bound of data (>-180) and
              // 180 degrees.
              if ( !ext.startsWith( QLatin1String( "-180 " ) ) && ext.contains( QLatin1String( ",180 " ) ) )
              {
                ext.clear();
              }
            }
          }
          else
          {
            // no features => ignore estimated extent
            ext.clear();
          }
        }
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "no column statistics for %1.%2.%3" ).arg( mSchemaName, mTableName, mGeometryColumn ) );
      }
    }

    if ( ext.isEmpty() )
    {
      sql = QStringLiteral( "SELECT %1(%2%3) FROM %4%5" )
            .arg( connectionRO()->majorVersion() < 2 ? "extent" : "st_extent",
                  quotedIdentifier( mGeometryColumn ),
                  mSpatialColType == SctPcPatch ? "::geometry" : "",
                  mQuery,
                  filterWhereClause() );

      result = connectionRO()->PQexec( sql );
      if ( result.PQresultStatus() != PGRES_TUPLES_OK )
        connectionRO()->PQexecNR( QStringLiteral( "ROLLBACK" ) );
      else if ( result.PQntuples() == 1 && !result.PQgetisnull( 0, 0 ) )
        ext = result.PQgetvalue( 0, 0 );
    }

    if ( !ext.isEmpty() )
    {
      QgsDebugMsg( "Got extents using: " + sql );

      QRegExp rx( "\\((.+) (.+),(.+) (.+)\\)" );
      if ( ext.contains( rx ) )
      {
        QStringList ex = rx.capturedTexts();

        mLayerExtent.setXMinimum( ex[1].toDouble() );
        mLayerExtent.setYMinimum( ex[2].toDouble() );
        mLayerExtent.setXMaximum( ex[3].toDouble() );
        mLayerExtent.setYMaximum( ex[4].toDouble() );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "result of extents query invalid: %1" ).arg( ext ), tr( "PostGIS" ) );
      }
    }

    QgsDebugMsg( "Set extents to: " + mLayerExtent.toString() );
  }

  return mLayerExtent;
}

void QgsPostgresProvider::updateExtents()
{
  mLayerExtent.setMinimal();
}

bool QgsPostgresProvider::getGeometryDetails()
{
  if ( mGeometryColumn.isNull() )
  {
    mDetectedGeomType = QgsWkbTypes::NoGeometry;
    mValid = true;
    return true;
  }

  QgsPostgresResult result;
  QString sql;

  QString schemaName = mSchemaName;
  QString tableName = mTableName;
  QString geomCol = mGeometryColumn;
  QString geomColType;

  if ( mIsQuery )
  {
    sql = QStringLiteral( "SELECT %1 FROM %2 LIMIT 0" ).arg( quotedIdentifier( mGeometryColumn ), mQuery );

    QgsDebugMsg( QStringLiteral( "Getting geometry column: %1" ).arg( sql ) );

    QgsPostgresResult result( connectionRO()->PQexec( sql ) );
    if ( PGRES_TUPLES_OK == result.PQresultStatus() )
    {
      Oid tableoid = result.PQftable( 0 );
      int column = result.PQftablecol( 0 );

      result = connectionRO()->PQexec( sql );
      if ( tableoid > 0 && PGRES_TUPLES_OK == result.PQresultStatus() )
      {
        sql = QStringLiteral( "SELECT pg_namespace.nspname,pg_class.relname FROM pg_class,pg_namespace WHERE pg_class.relnamespace=pg_namespace.oid AND pg_class.oid=%1" ).arg( tableoid );
        result = connectionRO()->PQexec( sql );

        if ( PGRES_TUPLES_OK == result.PQresultStatus() && 1 == result.PQntuples() )
        {
          schemaName = result.PQgetvalue( 0, 0 );
          tableName = result.PQgetvalue( 0, 1 );

          sql = QStringLiteral( "SELECT a.attname, t.typname FROM pg_attribute a, pg_type t WHERE a.attrelid=%1 AND a.attnum=%2 AND a.atttypid = t.oid" ).arg( tableoid ).arg( column );
          result = connectionRO()->PQexec( sql );
          if ( PGRES_TUPLES_OK == result.PQresultStatus() && 1 == result.PQntuples() )
          {
            geomCol = result.PQgetvalue( 0, 0 );
            geomColType = result.PQgetvalue( 0, 1 );
            if ( geomColType == QLatin1String( "geometry" ) )
              mSpatialColType = SctGeometry;
            else if ( geomColType == QLatin1String( "geography" ) )
              mSpatialColType = SctGeography;
            else if ( geomColType == QLatin1String( "topogeometry" ) )
              mSpatialColType = SctTopoGeometry;
            else if ( geomColType == QLatin1String( "pcpatch" ) )
              mSpatialColType = SctPcPatch;
            else
              mSpatialColType = SctNone;
          }
          else
          {
            schemaName = mSchemaName;
            tableName = mTableName;
          }
        }
      }
      else
      {
        schemaName.clear();
        tableName = mQuery;
      }
    }
    else
    {
      mValid = false;
      return false;
    }
  }

  QString detectedType;
  QString detectedSrid = mRequestedSrid;
  if ( !schemaName.isEmpty() )
  {
    // check geometry columns
    sql = QStringLiteral( "SELECT upper(type),srid,coord_dimension FROM geometry_columns WHERE f_table_name=%1 AND f_geometry_column=%2 AND f_table_schema=%3" )
          .arg( quotedValue( tableName ),
                quotedValue( geomCol ),
                quotedValue( schemaName ) );

    QgsDebugMsg( QStringLiteral( "Getting geometry column: %1" ).arg( sql ) );
    result = connectionRO()->PQexec( sql );
    QgsDebugMsg( QStringLiteral( "Geometry column query returned %1 rows" ).arg( result.PQntuples() ) );

    if ( result.PQntuples() == 1 )
    {
      detectedType = result.PQgetvalue( 0, 0 );
      QString dim = result.PQgetvalue( 0, 2 );
      if ( dim == QLatin1String( "3" ) && !detectedType.endsWith( 'M' ) )
        detectedType += QLatin1String( "Z" );
      else if ( dim == QLatin1String( "4" ) )
        detectedType += QLatin1String( "ZM" );

      detectedSrid = result.PQgetvalue( 0, 1 );
      mSpatialColType = SctGeometry;
    }
    else
    {
      connectionRO()->PQexecNR( QStringLiteral( "COMMIT" ) );
    }

    if ( detectedType.isEmpty() )
    {
      // check geography columns
      sql = QStringLiteral( "SELECT upper(type),srid FROM geography_columns WHERE f_table_name=%1 AND f_geography_column=%2 AND f_table_schema=%3" )
            .arg( quotedValue( tableName ),
                  quotedValue( geomCol ),
                  quotedValue( schemaName ) );

      QgsDebugMsg( QStringLiteral( "Getting geography column: %1" ).arg( sql ) );
      result = connectionRO()->PQexec( sql, false );
      QgsDebugMsg( QStringLiteral( "Geography column query returned %1" ).arg( result.PQntuples() ) );

      if ( result.PQntuples() == 1 )
      {
        detectedType = result.PQgetvalue( 0, 0 );
        detectedSrid = result.PQgetvalue( 0, 1 );
        mSpatialColType = SctGeography;
      }
      else
      {
        connectionRO()->PQexecNR( QStringLiteral( "COMMIT" ) );
      }
    }

    if ( detectedType.isEmpty() && connectionRO()->hasTopology() )
    {
      // check topology.layer
      sql = QString( "SELECT CASE "
                     "WHEN l.feature_type = 1 THEN 'MULTIPOINT' "
                     "WHEN l.feature_type = 2 THEN 'MULTILINESTRING' "
                     "WHEN l.feature_type = 3 THEN 'MULTIPOLYGON' "
                     "WHEN l.feature_type = 4 THEN 'GEOMETRYCOLLECTION' "
                     "END AS type, t.srid FROM topology.layer l, topology.topology t "
                     "WHERE l.topology_id = t.id AND l.schema_name=%3 "
                     "AND l.table_name=%1 AND l.feature_column=%2" )
            .arg( quotedValue( tableName ),
                  quotedValue( geomCol ),
                  quotedValue( schemaName ) );

      QgsDebugMsg( QStringLiteral( "Getting TopoGeometry column: %1" ).arg( sql ) );
      result = connectionRO()->PQexec( sql, false );
      QgsDebugMsg( QStringLiteral( "TopoGeometry column query returned %1" ).arg( result.PQntuples() ) );

      if ( result.PQntuples() == 1 )
      {
        detectedType = result.PQgetvalue( 0, 0 );
        detectedSrid = result.PQgetvalue( 0, 1 );
        mSpatialColType = SctTopoGeometry;
      }
      else
      {
        connectionRO()->PQexecNR( QStringLiteral( "COMMIT" ) );
      }
    }

    if ( detectedType.isEmpty() && connectionRO()->hasPointcloud() )
    {
      // check pointcloud columns
      sql = QStringLiteral( "SELECT 'POLYGON',srid FROM pointcloud_columns WHERE \"table\"=%1 AND \"column\"=%2 AND \"schema\"=%3" )
            .arg( quotedValue( tableName ),
                  quotedValue( geomCol ),
                  quotedValue( schemaName ) );

      QgsDebugMsg( QStringLiteral( "Getting pointcloud column: %1" ).arg( sql ) );
      result = connectionRO()->PQexec( sql, false );
      QgsDebugMsg( QStringLiteral( "Pointcloud column query returned %1" ).arg( result.PQntuples() ) );

      if ( result.PQntuples() == 1 )
      {
        detectedType = result.PQgetvalue( 0, 0 );
        detectedSrid = result.PQgetvalue( 0, 1 );
        mSpatialColType = SctPcPatch;
      }
      else
      {
        connectionRO()->PQexecNR( QStringLiteral( "COMMIT" ) );
      }
    }

    if ( mSpatialColType == SctNone )
    {
      sql = QString( "SELECT t.typname FROM "
                     "pg_attribute a, pg_class c, pg_namespace n, pg_type t "
                     "WHERE a.attrelid=c.oid AND c.relnamespace=n.oid "
                     "AND a.atttypid=t.oid "
                     "AND n.nspname=%3 AND c.relname=%1 AND a.attname=%2" )
            .arg( quotedValue( tableName ),
                  quotedValue( geomCol ),
                  quotedValue( schemaName ) );
      QgsDebugMsg( QStringLiteral( "Getting column datatype: %1" ).arg( sql ) );
      result = connectionRO()->PQexec( sql, false );
      QgsDebugMsg( QStringLiteral( "Column datatype query returned %1" ).arg( result.PQntuples() ) );
      if ( result.PQntuples() == 1 )
      {
        geomColType = result.PQgetvalue( 0, 0 );
        if ( geomColType == QLatin1String( "geometry" ) )
          mSpatialColType = SctGeometry;
        else if ( geomColType == QLatin1String( "geography" ) )
          mSpatialColType = SctGeography;
        else if ( geomColType == QLatin1String( "topogeometry" ) )
          mSpatialColType = SctTopoGeometry;
        else if ( geomColType == QLatin1String( "pcpatch" ) )
          mSpatialColType = SctPcPatch;
      }
      else
      {
        connectionRO()->PQexecNR( QStringLiteral( "COMMIT" ) );
      }
    }
  }
  else
  {
    sql = QStringLiteral( "SELECT %1 FROM %2 LIMIT 0" ).arg( quotedIdentifier( mGeometryColumn ), mQuery );
    result = connectionRO()->PQexec( sql );
    if ( PGRES_TUPLES_OK == result.PQresultStatus() )
    {
      sql = QStringLiteral( "SELECT (SELECT t.typname FROM pg_type t WHERE oid = %1), upper(postgis_typmod_type(%2)), postgis_typmod_srid(%2)" )
            .arg( QString::number( result.PQftype( 0 ) ), QString::number( result.PQfmod( 0 ) ) );
      result = connectionRO()->PQexec( sql, false );
      if ( result.PQntuples() == 1 )
      {
        geomColType  = result.PQgetvalue( 0, 0 );
        detectedType = result.PQgetvalue( 0, 1 );
        detectedSrid = result.PQgetvalue( 0, 2 );
        if ( geomColType == QLatin1String( "geometry" ) )
          mSpatialColType = SctGeometry;
        else if ( geomColType == QLatin1String( "geography" ) )
          mSpatialColType = SctGeography;
        else if ( geomColType == QLatin1String( "topogeometry" ) )
          mSpatialColType = SctTopoGeometry;
        else if ( geomColType == QLatin1String( "pcpatch" ) )
          mSpatialColType = SctPcPatch;
        else
        {
          detectedType = mRequestedGeomType == QgsWkbTypes::Unknown ? QString() : QgsPostgresConn::postgisWkbTypeName( mRequestedGeomType );
          detectedSrid = mRequestedSrid;
        }
      }
      else
      {
        connectionRO()->PQexecNR( QStringLiteral( "COMMIT" ) );
        detectedType = mRequestedGeomType == QgsWkbTypes::Unknown ? QString() : QgsPostgresConn::postgisWkbTypeName( mRequestedGeomType );
      }
    }
    else
    {
      mValid = false;
      return false;
    }
  }

  mDetectedGeomType = QgsPostgresConn::wkbTypeFromPostgis( detectedType );
  mDetectedSrid     = detectedSrid;

  if ( mDetectedGeomType == QgsWkbTypes::Unknown )
  {
    mDetectedSrid.clear();

    QgsPostgresLayerProperty layerProperty;
    if ( !mIsQuery )
    {
      layerProperty.schemaName = schemaName;
      layerProperty.tableName  = tableName;
    }
    else
    {
      layerProperty.schemaName.clear();
      layerProperty.tableName  = mQuery;
    }
    layerProperty.geometryColName = mGeometryColumn;
    layerProperty.geometryColType = mSpatialColType;

    QString delim;

    if ( !mSqlWhereClause.isEmpty() )
    {
      layerProperty.sql += delim + '(' + mSqlWhereClause + ')';
      delim = QStringLiteral( " AND " );
    }

    connectionRO()->retrieveLayerTypes( layerProperty, mUseEstimatedMetadata );

    mSpatialColType = layerProperty.geometryColType;

    if ( layerProperty.size() == 0 )
    {
      // no data - so take what's requested
      if ( mRequestedGeomType == QgsWkbTypes::Unknown || mRequestedSrid.isEmpty() )
      {
        QgsMessageLog::logMessage( tr( "Geometry type and srid for empty column %1 of %2 undefined." ).arg( mGeometryColumn, mQuery ) );
      }
    }
    else
    {
      int i;
      for ( i = 0; i < layerProperty.size(); i++ )
      {
        QgsWkbTypes::Type wkbType = layerProperty.types.at( i );

        if ( ( wkbType != QgsWkbTypes::Unknown && ( mRequestedGeomType == QgsWkbTypes::Unknown || mRequestedGeomType == wkbType ) ) &&
             ( mRequestedSrid.isEmpty() || layerProperty.srids.at( i ) == mRequestedSrid.toInt() ) )
          break;
      }

      // requested type && srid is available
      if ( i < layerProperty.size() )
      {
        if ( layerProperty.size() == 1 )
        {
          // only what we requested is available
          mDetectedGeomType = layerProperty.types.at( 0 );
          mDetectedSrid     = QString::number( layerProperty.srids.at( 0 ) );
        }
      }
      else
      {
        // geometry type undetermined or not unrequested
        QgsMessageLog::logMessage( tr( "Feature type or srid for %1 of %2 could not be determined or was not requested." ).arg( mGeometryColumn, mQuery ) );
      }
    }
  }

  QgsDebugMsg( QStringLiteral( "Detected SRID is %1" ).arg( mDetectedSrid ) );
  QgsDebugMsg( QStringLiteral( "Requested SRID is %1" ).arg( mRequestedSrid ) );
  QgsDebugMsg( QStringLiteral( "Detected type is %1" ).arg( mDetectedGeomType ) );
  QgsDebugMsg( QStringLiteral( "Requested type is %1" ).arg( mRequestedGeomType ) );

  mValid = ( mDetectedGeomType != QgsWkbTypes::Unknown || mRequestedGeomType != QgsWkbTypes::Unknown )
           && ( !mDetectedSrid.isEmpty() || !mRequestedSrid.isEmpty() );

  if ( !mValid )
    return false;

  QgsDebugMsg( QStringLiteral( "Spatial column type is %1" ).arg( QgsPostgresConn::displayStringForGeomType( mSpatialColType ) ) );

  return mValid;
}

bool QgsPostgresProvider::convertField( QgsField &field, const QMap<QString, QVariant> *options )
{
  //determine field type to use for strings
  QString stringFieldType = QStringLiteral( "varchar" );
  if ( options && options->value( QStringLiteral( "dropStringConstraints" ), false ).toBool() )
  {
    //drop string length constraints by using PostgreSQL text type for strings
    stringFieldType = QStringLiteral( "text" );
  }

  QString fieldType = stringFieldType; //default to string
  int fieldSize = field.length();
  int fieldPrec = field.precision();
  switch ( field.type() )
  {
    case QVariant::LongLong:
      fieldType = QStringLiteral( "int8" );
      fieldPrec = 0;
      break;

    case QVariant::DateTime:
      fieldType = QStringLiteral( "timestamp without time zone" );
      break;

    case QVariant::Time:
      fieldType = QStringLiteral( "time" );
      break;

    case QVariant::String:
      fieldType = stringFieldType;
      fieldPrec = -1;
      break;

    case QVariant::Int:
      fieldType = QStringLiteral( "int4" );
      fieldPrec = 0;
      break;

    case QVariant::Date:
      fieldType = QStringLiteral( "date" );
      fieldPrec = 0;
      break;

    case QVariant::Map:
      fieldType = field.typeName();
      if ( fieldType.isEmpty() )
        fieldType = QStringLiteral( "hstore" );
      fieldPrec = -1;
      break;

    case QVariant::StringList:
      fieldType = QStringLiteral( "_text" );
      fieldPrec = -1;
      break;

    case QVariant::List:
    {
      QgsField sub( QString(), field.subType(), QString(), fieldSize, fieldPrec );
      if ( !convertField( sub, nullptr ) ) return false;
      fieldType = "_" + sub.typeName();
      fieldPrec = -1;
      break;
    }

    case QVariant::Double:
      if ( fieldSize > 18 )
      {
        fieldType = QStringLiteral( "numeric" );
        fieldSize = -1;
      }
      else
      {
        fieldType = QStringLiteral( "float8" );
      }
      fieldPrec = -1;
      break;

    case QVariant::Bool:
      fieldType = QStringLiteral( "bool" );
      fieldPrec = -1;
      fieldSize = -1;
      break;

    default:
      return false;
  }

  field.setTypeName( fieldType );
  field.setLength( fieldSize );
  field.setPrecision( fieldPrec );
  return true;
}

QgsVectorLayerExporter::ExportError QgsPostgresProvider::createEmptyLayer( const QString &uri,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    QMap<int, int> *oldToNewAttrIdxMap,
    QString *errorMessage,
    const QMap<QString, QVariant> *options )
{
  // populate members from the uri structure
  QgsDataSourceUri dsUri( uri );

  QString schemaName = dsUri.schema();
  QString tableName = dsUri.table();

  QString geometryColumn = dsUri.geometryColumn();
  QString geometryType;

  QString primaryKey = dsUri.keyColumn();
  QString primaryKeyType;

  QStringList pkList;
  QStringList pkType;

  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName += quotedIdentifier( schemaName ) + '.';
  }
  schemaTableName += quotedIdentifier( tableName );

  QgsDebugMsg( QStringLiteral( "Connection info is: %1" ).arg( dsUri.connectionInfo( false ) ) );
  QgsDebugMsg( QStringLiteral( "Geometry column is: %1" ).arg( geometryColumn ) );
  QgsDebugMsg( QStringLiteral( "Schema is: %1" ).arg( schemaName ) );
  QgsDebugMsg( QStringLiteral( "Table name is: %1" ).arg( tableName ) );

  // create the table
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Connection to database failed" );
    return QgsVectorLayerExporter::ErrConnectionFailed;
  }

  // get the pk's name and type

  // if no pk name was passed, define the new pk field name
  if ( primaryKey.isEmpty() )
  {
    int index = 0;
    QString pk = primaryKey = QStringLiteral( "id" );
    for ( int fldIdx = 0; fldIdx < fields.count(); ++fldIdx )
    {
      if ( fields.at( fldIdx ).name() == primaryKey )
      {
        // it already exists, try again with a new name
        primaryKey = QStringLiteral( "%1_%2" ).arg( pk ).arg( index++ );
        fldIdx = -1; // it is incremented in the for loop, i.e. restarts at 0
      }
    }

    pkList = QStringList( primaryKey );
    pkType = QStringList( QStringLiteral( "serial" ) );
  }
  else
  {
    pkList = parseUriKey( primaryKey );
    Q_FOREACH ( const QString &col, pkList )
    {
      // search for the passed field
      QString type;
      for ( int fldIdx = 0; fldIdx < fields.count(); ++fldIdx )
      {
        if ( fields[fldIdx].name() == col )
        {
          // found, get the field type
          QgsField fld = fields[fldIdx];
          if ( convertField( fld, options ) )
          {
            type = fld.typeName();
            break;
          }
        }
      }
      if ( type.isEmpty() ) type = QStringLiteral( "serial" );
      else
      {
        // if the pk field's type is one of the postgres integer types,
        // use the equivalent autoincremental type (serialN)
        if ( primaryKeyType == QLatin1String( "int2" ) || primaryKeyType == QLatin1String( "int4" ) )
        {
          primaryKeyType = QStringLiteral( "serial" );
        }
        else if ( primaryKeyType == QLatin1String( "int8" ) )
        {
          primaryKeyType = QStringLiteral( "serial8" );
        }
      }
      pkType << type;
    }
  }

  try
  {
    conn->PQexecNR( QStringLiteral( "BEGIN" ) );

    // We want a valid schema name ...
    if ( schemaName.isEmpty() )
    {
      QString sql = QString( "SELECT current_schema" );
      QgsPostgresResult result( conn->PQexec( sql ) );
      if ( result.PQresultStatus() != PGRES_TUPLES_OK )
        throw PGException( result );
      schemaName = result.PQgetvalue( 0, 0 );
      if ( schemaName.isEmpty() )
      {
        schemaName = QStringLiteral( "public" );
      }
    }

    QString sql = QString( "SELECT 1"
                           " FROM pg_class AS cls JOIN pg_namespace AS nsp"
                           " ON nsp.oid=cls.relnamespace "
                           " WHERE cls.relname=%1 AND nsp.nspname=%2" )
                  .arg( quotedValue( tableName ),
                        quotedValue( schemaName ) );

    QgsPostgresResult result( conn->PQexec( sql ) );
    if ( result.PQresultStatus() != PGRES_TUPLES_OK )
      throw PGException( result );

    bool exists = result.PQntuples() > 0;

    if ( exists && overwrite )
    {
      // delete the table if exists, then re-create it
      QString sql = QString( "SELECT DropGeometryTable(%1,%2)"
                             " FROM pg_class AS cls JOIN pg_namespace AS nsp"
                             " ON nsp.oid=cls.relnamespace "
                             " WHERE cls.relname=%2 AND nsp.nspname=%1" )
                    .arg( quotedValue( schemaName ),
                          quotedValue( tableName ) );

      result = conn->PQexec( sql );
      if ( result.PQresultStatus() != PGRES_TUPLES_OK )
        throw PGException( result );
    }

    sql = QStringLiteral( "CREATE TABLE %1(" ) .arg( schemaTableName );
    QString pk;
    for ( int i = 0; i < pkList.size(); ++i )
    {
      QString col = pkList[i];
      const QString &type = pkType[i];

      if ( options && options->value( QStringLiteral( "lowercaseFieldNames" ), false ).toBool() )
      {
        col = col.toLower();
      }
      else
      {
        col = quotedIdentifier( col ); // no need to quote lowercase field
      }

      if ( i )
      {
        pk  += QLatin1String( "," );
        sql += QLatin1String( "," );
      }

      pk += col;
      sql += col + " " + type;
    }
    sql += QStringLiteral( ", PRIMARY KEY (%1) )" ) .arg( pk );

    result = conn->PQexec( sql );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK )
      throw PGException( result );

    // get geometry type, dim and srid
    int dim = 2;
    long srid = srs.postgisSrid();

    QgsPostgresConn::postgisWkbType( wkbType, geometryType, dim );

    // create geometry column
    if ( !geometryType.isEmpty() )
    {
      sql = QStringLiteral( "SELECT AddGeometryColumn(%1,%2,%3,%4,%5,%6)" )
            .arg( quotedValue( schemaName ),
                  quotedValue( tableName ),
                  quotedValue( geometryColumn ) )
            .arg( srid )
            .arg( quotedValue( geometryType ) )
            .arg( dim );

      result = conn->PQexec( sql );
      if ( result.PQresultStatus() != PGRES_TUPLES_OK )
        throw PGException( result );
    }
    else
    {
      geometryColumn.clear();
    }

    conn->PQexecNR( QStringLiteral( "COMMIT" ) );
  }
  catch ( PGException &e )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Creation of data source %1 failed: \n%2" )
                      .arg( schemaTableName,
                            e.errorMessage() );

    conn->PQexecNR( QStringLiteral( "ROLLBACK" ) );
    conn->unref();
    return QgsVectorLayerExporter::ErrCreateLayer;
  }
  conn->unref();

  QgsDebugMsg( QStringLiteral( "layer %1 created" ).arg( schemaTableName ) );

  // use the provider to edit the table
  dsUri.setDataSource( schemaName, tableName, geometryColumn, QString(), primaryKey );

  QgsDataProvider::ProviderOptions providerOptions;
  std::unique_ptr< QgsPostgresProvider > provider = qgis::make_unique< QgsPostgresProvider >( dsUri.uri( false ), providerOptions );
  if ( !provider->isValid() )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Loading of the layer %1 failed" ).arg( schemaTableName );

    return QgsVectorLayerExporter::ErrInvalidLayer;
  }

  QgsDebugMsg( QStringLiteral( "layer loaded" ) );

  // add fields to the layer
  if ( oldToNewAttrIdxMap )
    oldToNewAttrIdxMap->clear();

  if ( fields.size() > 0 )
  {
    int offset = 1;

    // get the list of fields
    QList<QgsField> flist;
    for ( int fldIdx = 0; fldIdx < fields.count(); ++fldIdx )
    {
      QgsField fld = fields.at( fldIdx );

      if ( fld.name() == geometryColumn )
      {
        //the "lowercaseFieldNames" option does not affect the name of the geometry column, so we perform
        //this test before converting the field name to lowercase
        QgsDebugMsg( QStringLiteral( "Found a field with the same name of the geometry column. Skip it!" ) );
        continue;
      }

      if ( options && options->value( QStringLiteral( "lowercaseFieldNames" ), false ).toBool() )
      {
        //convert field name to lowercase
        fld.setName( fld.name().toLower() );
      }

      int pkIdx = -1;
      for ( int i = 0; i < pkList.size(); ++i )
      {
        QString col = pkList[i];
        if ( options && options->value( QStringLiteral( "lowercaseFieldNames" ), false ).toBool() )
        {
          //convert field name to lowercase (TODO: avoid doing this
          //over and over)
          col = col.toLower();
        }
        if ( fld.name() == col )
        {
          pkIdx = i;
          break;
        }
      }
      if ( pkIdx >= 0 )
      {
        oldToNewAttrIdxMap->insert( fldIdx, pkIdx );
        continue;
      }

      if ( !convertField( fld, options ) )
      {
        if ( errorMessage )
          *errorMessage = QObject::tr( "Unsupported type for field %1" ).arg( fld.name() );

        return QgsVectorLayerExporter::ErrAttributeTypeUnsupported;
      }

      QgsDebugMsg( QStringLiteral( "creating field #%1 -> #%2 name %3 type %4 typename %5 width %6 precision %7" )
                   .arg( fldIdx ).arg( offset )
                   .arg( fld.name(), QVariant::typeToName( fld.type() ), fld.typeName() )
                   .arg( fld.length() ).arg( fld.precision() )
                 );

      flist.append( fld );
      if ( oldToNewAttrIdxMap )
        oldToNewAttrIdxMap->insert( fldIdx, offset++ );
    }

    if ( !provider->addAttributes( flist ) )
    {
      if ( errorMessage )
        *errorMessage = QObject::tr( "Creation of fields failed" );

      return QgsVectorLayerExporter::ErrAttributeCreationFailed;
    }

    QgsDebugMsg( QStringLiteral( "Done creating fields" ) );
  }
  return QgsVectorLayerExporter::NoError;
}

QgsCoordinateReferenceSystem QgsPostgresProvider::crs() const
{
  QgsCoordinateReferenceSystem srs;
  int srid = mRequestedSrid.isEmpty() ? mDetectedSrid.toInt() : mRequestedSrid.toInt();
  srs.createFromSrid( srid );
  if ( !srs.isValid() )
  {
    static QMutex sMutex;
    QMutexLocker locker( &sMutex );
    static QMap<int, QgsCoordinateReferenceSystem> sCrsCache;
    if ( sCrsCache.contains( srid ) )
      srs = sCrsCache.value( srid );
    else
    {
      QgsPostgresConn *conn = connectionRO();
      conn->lock();
      QgsPostgresResult result( conn->PQexec( QStringLiteral( "SELECT proj4text FROM spatial_ref_sys WHERE srid=%1" ).arg( srid ) ) );
      conn->unlock();
      if ( result.PQresultStatus() == PGRES_TUPLES_OK )
      {
        srs = QgsCoordinateReferenceSystem::fromProj4( result.PQgetvalue( 0, 0 ) );
        sCrsCache.insert( srid, srs );
      }
    }
  }
  return srs;
}

QString QgsPostgresProvider::subsetString() const
{
  return mSqlWhereClause;
}

QString QgsPostgresProvider::getTableName()
{
  return mTableName;
}

size_t QgsPostgresProvider::layerCount() const
{
  return 1;                   // XXX need to return actual number of layers
} // QgsPostgresProvider::layerCount()


QString  QgsPostgresProvider::name() const
{
  return POSTGRES_KEY;
} //  QgsPostgresProvider::name()

QString  QgsPostgresProvider::description() const
{
  QString pgVersion( tr( "PostgreSQL version: unknown" ) );
  QString postgisVersion( tr( "unknown" ) );

  if ( connectionRO() )
  {
    QgsPostgresResult result;

    result = connectionRO()->PQexec( QStringLiteral( "SELECT version()" ) );
    if ( result.PQresultStatus() == PGRES_TUPLES_OK )
    {
      pgVersion = result.PQgetvalue( 0, 0 );
    }

    result = connectionRO()->PQexec( QStringLiteral( "SELECT postgis_version()" ) );
    if ( result.PQresultStatus() == PGRES_TUPLES_OK )
    {
      postgisVersion = result.PQgetvalue( 0, 0 );
    }
  }
  else
  {
    pgVersion = tr( "PostgreSQL not connected" );
  }

  return tr( "PostgreSQL/PostGIS provider\n%1\nPostGIS %2" ).arg( pgVersion, postgisVersion );
} //  QgsPostgresProvider::description()

static void jumpSpace( const QString &txt, int &i )
{
  while ( i < txt.length() && txt.at( i ).isSpace() )
    ++i;
}

static QString getNextString( const QString &txt, int &i, const QString &sep )
{
  jumpSpace( txt, i );
  QString cur = txt.mid( i );
  if ( cur.startsWith( '"' ) )
  {
    QRegExp stringRe( "^\"((?:\\\\.|[^\"\\\\])*)\".*" );
    if ( !stringRe.exactMatch( cur ) )
    {
      QgsLogger::warning( "Cannot find end of double quoted string: " + txt );
      return QString();
    }
    i += stringRe.cap( 1 ).length() + 2;
    jumpSpace( txt, i );
    if ( !txt.midRef( i ).startsWith( sep ) && i < txt.length() )
    {
      QgsLogger::warning( "Cannot find separator: " + txt.mid( i ) );
      return QString();
    }
    i += sep.length();
    return stringRe.cap( 1 ).replace( QLatin1String( "\\\"" ), QLatin1String( "\"" ) ).replace( QLatin1String( "\\\\" ), QLatin1String( "\\" ) );
  }
  else
  {
    int sepPos = cur.indexOf( sep );
    if ( sepPos < 0 )
    {
      i += cur.length();
      return cur.trimmed();
    }
    i += sepPos + sep.length();
    return cur.left( sepPos ).trimmed();
  }
}

static QVariant parseHstore( const QString &txt )
{
  QVariantMap result;
  int i = 0;
  while ( i < txt.length() )
  {
    QString key = getNextString( txt, i, QStringLiteral( "=>" ) );
    QString value = getNextString( txt, i, QStringLiteral( "," ) );
    if ( key.isNull() || value.isNull() )
    {
      QgsLogger::warning( "Error parsing hstore: " + txt );
      break;
    }
    result.insert( key, value );
  }

  return result;
}

static QVariant parseJson( const QString &txt )
{
  QVariant result;
  QJsonDocument jsonResponse = QJsonDocument::fromJson( txt.toUtf8() );
  //it's null if no json format
  result = jsonResponse.toVariant();
  return result;
}

static QVariant parseOtherArray( const QString &txt, QVariant::Type subType, const QString &typeName )
{
  int i = 0;
  QVariantList result;
  while ( i < txt.length() )
  {
    const QString value = getNextString( txt, i, QStringLiteral( "," ) );
    if ( value.isNull() )
    {
      QgsLogger::warning( "Error parsing array: " + txt );
      break;
    }
    result.append( QgsPostgresProvider::convertValue( subType, QVariant::Invalid, value, typeName ) );
  }
  return result;
}

static QVariant parseStringArray( const QString &txt )
{
  int i = 0;
  QStringList result;
  while ( i < txt.length() )
  {
    const QString value = getNextString( txt, i, QStringLiteral( "," ) );
    if ( value.isNull() )
    {
      QgsLogger::warning( "Error parsing array: " + txt );
      break;
    }
    result.append( value );
  }
  return result;
}

static QVariant parseArray( const QString &txt, QVariant::Type type, QVariant::Type subType, const QString &typeName )
{
  if ( !txt.startsWith( '{' ) || !txt.endsWith( '}' ) )
  {
    if ( !txt.isEmpty() )
      QgsLogger::warning( "Error parsing array, missing curly braces: " + txt );
    return QVariant( type );
  }
  QString inner = txt.mid( 1, txt.length() - 2 );
  if ( type == QVariant::StringList )
    return parseStringArray( inner );
  else
    return parseOtherArray( inner, subType, typeName );
}

QVariant QgsPostgresProvider::convertValue( QVariant::Type type, QVariant::Type subType, const QString &value, const QString &typeName )
{
  QVariant result;
  switch ( type )
  {
    case QVariant::Map:
      if ( typeName == QLatin1String( "json" ) || typeName == QLatin1String( "jsonb" ) )
        result = parseJson( value );
      else
        result = parseHstore( value );
      break;
    case QVariant::StringList:
    case QVariant::List:
      result = parseArray( value, type, subType, typeName );
      break;
    case QVariant::Bool:
      if ( value == QChar( 't' ) )
        result = true;
      else if ( value == QChar( 'f' ) )
        result = false;
      else
        result = QVariant( type );
      break;
    default:
      result = value;
      if ( !result.convert( type ) || value.isNull() )
        result = QVariant( type );
      break;
  }

  return result;
}

QList<QgsVectorLayer *> QgsPostgresProvider::searchLayers( const QList<QgsVectorLayer *> &layers, const QString &connectionInfo, const QString &schema, const QString &tableName )
{
  QList<QgsVectorLayer *> result;
  Q_FOREACH ( QgsVectorLayer *layer, layers )
  {
    const QgsPostgresProvider *pgProvider = qobject_cast<QgsPostgresProvider *>( layer->dataProvider() );
    if ( pgProvider &&
         pgProvider->mUri.connectionInfo( false ) == connectionInfo && pgProvider->mSchemaName == schema && pgProvider->mTableName == tableName )
    {
      result.append( layer );
    }
  }
  return result;
}

QList<QgsRelation> QgsPostgresProvider::discoverRelations( const QgsVectorLayer *self, const QList<QgsVectorLayer *> &layers ) const
{
  QList<QgsRelation> result;
  QString sql(
    "SELECT RC.CONSTRAINT_NAME, KCU1.COLUMN_NAME, KCU2.CONSTRAINT_SCHEMA, KCU2.TABLE_NAME, KCU2.COLUMN_NAME, KCU1.ORDINAL_POSITION "
    "FROM INFORMATION_SCHEMA.REFERENTIAL_CONSTRAINTS AS RC "
    "INNER JOIN INFORMATION_SCHEMA.KEY_COLUMN_USAGE AS KCU1 "
    "ON KCU1.CONSTRAINT_CATALOG = RC.CONSTRAINT_CATALOG AND KCU1.CONSTRAINT_SCHEMA = RC.CONSTRAINT_SCHEMA AND KCU1.CONSTRAINT_NAME = RC.CONSTRAINT_NAME "
    "INNER JOIN INFORMATION_SCHEMA.KEY_COLUMN_USAGE AS KCU2 "
    "ON KCU2.CONSTRAINT_CATALOG = RC.UNIQUE_CONSTRAINT_CATALOG AND KCU2.CONSTRAINT_SCHEMA = RC.UNIQUE_CONSTRAINT_SCHEMA AND KCU2.CONSTRAINT_NAME = RC.UNIQUE_CONSTRAINT_NAME "
    "AND KCU2.ORDINAL_POSITION = KCU1.ORDINAL_POSITION "
    "WHERE KCU1.CONSTRAINT_SCHEMA=" + QgsPostgresConn::quotedValue( mSchemaName ) + " AND KCU1.TABLE_NAME=" + QgsPostgresConn::quotedValue( mTableName ) +
    "GROUP BY RC.CONSTRAINT_NAME, KCU1.COLUMN_NAME, KCU2.CONSTRAINT_SCHEMA, KCU2.TABLE_NAME, KCU2.COLUMN_NAME, KCU1.ORDINAL_POSITION " +
    "ORDER BY KCU1.ORDINAL_POSITION"
  );
  QgsPostgresResult sqlResult( connectionRO()->PQexec( sql ) );
  if ( sqlResult.PQresultStatus() != PGRES_TUPLES_OK )
  {
    QgsLogger::warning( "Error getting the foreign keys of " + mTableName );
    return result;
  }

  int nbFound = 0;
  for ( int row = 0; row < sqlResult.PQntuples(); ++row )
  {
    const QString name = sqlResult.PQgetvalue( row, 0 );
    const QString fkColumn = sqlResult.PQgetvalue( row, 1 );
    const QString refSchema = sqlResult.PQgetvalue( row, 2 );
    const QString refTable = sqlResult.PQgetvalue( row, 3 );
    const QString refColumn = sqlResult.PQgetvalue( row, 4 );
    const QString position = sqlResult.PQgetvalue( row, 5 );
    if ( position == QLatin1String( "1" ) )
    {
      // first reference field => try to find if we have layers for the referenced table
      const QList<QgsVectorLayer *> foundLayers = searchLayers( layers, mUri.connectionInfo( false ), refSchema, refTable );
      Q_FOREACH ( const QgsVectorLayer *foundLayer, foundLayers )
      {
        QgsRelation relation;
        relation.setName( name );
        relation.setReferencingLayer( self->id() );
        relation.setReferencedLayer( foundLayer->id() );
        relation.addFieldPair( fkColumn, refColumn );
        relation.generateId();
        if ( relation.isValid() )
        {
          result.append( relation );
          ++nbFound;
        }
        else
        {
          QgsLogger::warning( "Invalid relation for " + name );
        }
      }
    }
    else
    {
      // multi reference field => add the field pair to all the referenced layers found
      for ( int i = 0; i < nbFound; ++i )
      {
        result[result.size() - 1 - i].addFieldPair( fkColumn, refColumn );
      }
    }
  }
  return result;
}

QgsAttrPalIndexNameHash QgsPostgresProvider::palAttributeIndexNames() const
{
  return mAttrPalIndexName;
}

QgsPostgresProvider::Relkind QgsPostgresProvider::relkind() const
{
  if ( mIsQuery )
    return Relkind::Unknown;

  QString sql = QStringLiteral( "SELECT relkind FROM pg_class WHERE oid=regclass(%1)::oid" ).arg( quotedValue( mQuery ) );
  QgsPostgresResult res( connectionRO()->PQexec( sql ) );
  QString type = res.PQgetvalue( 0, 0 );

  QgsPostgresProvider::Relkind kind = Relkind::Unknown;

  if ( type == QLatin1String( "r" ) )
  {
    kind = Relkind::OrdinaryTable;
  }
  else if ( type == QLatin1String( "i" ) )
  {
    kind = Relkind::Index;
  }
  else if ( type == QLatin1String( "s" ) )
  {
    kind = Relkind::Sequence;
  }
  else if ( type == QLatin1String( "v" ) )
  {
    kind = Relkind::View;
  }
  else if ( type == QLatin1String( "m" ) )
  {
    kind = Relkind::MaterializedView;
  }
  else if ( type == QLatin1String( "c" ) )
  {
    kind = Relkind::CompositeType;
  }
  else if ( type == QLatin1String( "t" ) )
  {
    kind = Relkind::ToastTable;
  }
  else if ( type == QLatin1String( "f" ) )
  {
    kind = Relkind::ForeignTable;
  }
  else if ( type == QLatin1String( "p" ) )
  {
    kind = Relkind::PartitionedTable;
  }

  return kind;
}

bool QgsPostgresProvider::hasMetadata() const
{
  bool hasMetadata = true;
  QgsPostgresProvider::Relkind kind = relkind();

  if ( kind == Relkind::View || kind == Relkind::MaterializedView )
  {
    hasMetadata = false;
  }

  return hasMetadata;
}

/**
 * Class factory to return a pointer to a newly created
 * QgsPostgresProvider object
 */
QGISEXTERN QgsPostgresProvider *classFactory( const QString *uri, const QgsDataProvider::ProviderOptions &options )
{
  return new QgsPostgresProvider( *uri, options );
}

/**
 * Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return  POSTGRES_KEY;
}

/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return POSTGRES_DESCRIPTION;
}

/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}

#ifdef HAVE_GUI
QGISEXTERN QgsPgSourceSelect *selectWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  return new QgsPgSourceSelect( parent, fl, widgetMode );
}
#endif

QGISEXTERN int dataCapabilities()
{
  return QgsDataProvider::Database;
}

QGISEXTERN QgsDataItem *dataItem( QString path, QgsDataItem *parentItem )
{
  Q_UNUSED( path );
  return new QgsPGRootItem( parentItem, QStringLiteral( "PostGIS" ), QStringLiteral( "pg:" ) );
}

// ---------------------------------------------------------------------------

QGISEXTERN QgsVectorLayerExporter::ExportError createEmptyLayer(
  const QString &uri,
  const QgsFields &fields,
  QgsWkbTypes::Type wkbType,
  const QgsCoordinateReferenceSystem &srs,
  bool overwrite,
  QMap<int, int> *oldToNewAttrIdxMap,
  QString *errorMessage,
  const QMap<QString, QVariant> *options )
{
  return QgsPostgresProvider::createEmptyLayer(
           uri, fields, wkbType, srs, overwrite,
           oldToNewAttrIdxMap, errorMessage, options
         );
}

QGISEXTERN bool deleteLayer( const QString &uri, QString &errCause )
{
  QgsDebugMsg( "deleting layer " + uri );

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

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  // check the geometry column count
  QString sql = QString( "SELECT count(*) "
                         "FROM geometry_columns, pg_class, pg_namespace "
                         "WHERE f_table_name=relname AND f_table_schema=nspname "
                         "AND pg_class.relnamespace=pg_namespace.oid "
                         "AND f_table_schema=%1 AND f_table_name=%2" )
                .arg( QgsPostgresConn::quotedValue( schemaName ),
                      QgsPostgresConn::quotedValue( tableName ) );
  QgsPostgresResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    errCause = QObject::tr( "Unable to delete layer %1: \n%2" )
               .arg( schemaTableName,
                     result.PQresultErrorMessage() );
    conn->unref();
    return false;
  }

  int count = result.PQgetvalue( 0, 0 ).toInt();

  if ( !geometryCol.isEmpty() && count > 1 )
  {
    // the table has more geometry columns, drop just the geometry column
    sql = QStringLiteral( "SELECT DropGeometryColumn(%1,%2,%3)" )
          .arg( QgsPostgresConn::quotedValue( schemaName ),
                QgsPostgresConn::quotedValue( tableName ),
                QgsPostgresConn::quotedValue( geometryCol ) );
  }
  else
  {
    // drop the table
    sql = QStringLiteral( "SELECT DropGeometryTable(%1,%2)" )
          .arg( QgsPostgresConn::quotedValue( schemaName ),
                QgsPostgresConn::quotedValue( tableName ) );
  }

  result = conn->PQexec( sql );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    errCause = QObject::tr( "Unable to delete layer %1: \n%2" )
               .arg( schemaTableName,
                     result.PQresultErrorMessage() );
    conn->unref();
    return false;
  }

  conn->unref();
  return true;
}

QGISEXTERN bool deleteSchema( const QString &schema, const QgsDataSourceUri &uri, QString &errCause, bool cascade = false )
{
  QgsDebugMsg( "deleting schema " + schema );

  if ( schema.isEmpty() )
    return false;

  QString schemaName = QgsPostgresConn::quotedIdentifier( schema );

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  // drop the schema
  QString sql = QStringLiteral( "DROP SCHEMA %1 %2" )
                .arg( schemaName, cascade ? QStringLiteral( "CASCADE" ) : QString() );

  QgsPostgresResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    errCause = QObject::tr( "Unable to delete schema %1: \n%2" )
               .arg( schemaName,
                     result.PQresultErrorMessage() );
    conn->unref();
    return false;
  }

  conn->unref();
  return true;
}

QGISEXTERN bool saveStyle( const QString &uri, const QString &qmlStyle, const QString &sldStyle,
                           const QString &styleName, const QString &styleDescription,
                           const QString &uiFileContent, bool useAsDefault, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  if ( !tableExists( *conn, QStringLiteral( "layer_styles" ) ) )
  {
    QgsPostgresResult res( conn->PQexec( "CREATE TABLE layer_styles("
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
                                         ",owner varchar(63)"
                                         ",ui xml"
                                         ",update_time timestamp DEFAULT CURRENT_TIMESTAMP"
                                         ")" ) );
    if ( res.PQresultStatus() != PGRES_COMMAND_OK )
    {
      errCause = QObject::tr( "Unable to save layer style. It's not possible to create the destination table on the database. Maybe this is due to table permissions (user=%1). Please contact your database admin" ).arg( dsUri.username() );
      conn->unref();
      return false;
    }
  }

  if ( dsUri.database().isEmpty() ) // typically when a service file is used
  {
    dsUri.setDatabase( conn->currentDatabase() );
  }

  QString uiFileColumn;
  QString uiFileValue;
  if ( !uiFileContent.isEmpty() )
  {
    uiFileColumn = QStringLiteral( ",ui" );
    uiFileValue = QStringLiteral( ",XMLPARSE(DOCUMENT %1)" ).arg( QgsPostgresConn::quotedValue( uiFileContent ) );
  }

  // Note: in the construction of the INSERT and UPDATE strings the qmlStyle and sldStyle values
  // can contain user entered strings, which may themselves include %## values that would be
  // replaced by the QString.arg function.  To ensure that the final SQL string is not corrupt these
  // two values are both replaced in the final .arg call of the string construction.

  QString sql = QString( "INSERT INTO layer_styles("
                         "f_table_catalog,f_table_schema,f_table_name,f_geometry_column,styleName,styleQML,styleSLD,useAsDefault,description,owner%11"
                         ") VALUES ("
                         "%1,%2,%3,%4,%5,XMLPARSE(DOCUMENT %16),XMLPARSE(DOCUMENT %17),%8,%9,%10%12"
                         ")" )
                .arg( QgsPostgresConn::quotedValue( dsUri.database() ) )
                .arg( QgsPostgresConn::quotedValue( dsUri.schema() ) )
                .arg( QgsPostgresConn::quotedValue( dsUri.table() ) )
                .arg( QgsPostgresConn::quotedValue( dsUri.geometryColumn() ) )
                .arg( QgsPostgresConn::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) )
                .arg( useAsDefault ? "true" : "false" )
                .arg( QgsPostgresConn::quotedValue( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription ) )
                .arg( QgsPostgresConn::quotedValue( dsUri.username() ) )
                .arg( uiFileColumn )
                .arg( uiFileValue )
                // Must be the final .arg replacement - see above
                .arg( QgsPostgresConn::quotedValue( qmlStyle ),
                      QgsPostgresConn::quotedValue( sldStyle ) );

  QString checkQuery = QString( "SELECT styleName"
                                " FROM layer_styles"
                                " WHERE f_table_catalog=%1"
                                " AND f_table_schema=%2"
                                " AND f_table_name=%3"
                                " AND f_geometry_column=%4"
                                " AND styleName=%5" )
                       .arg( QgsPostgresConn::quotedValue( dsUri.database() ) )
                       .arg( QgsPostgresConn::quotedValue( dsUri.schema() ) )
                       .arg( QgsPostgresConn::quotedValue( dsUri.table() ) )
                       .arg( QgsPostgresConn::quotedValue( dsUri.geometryColumn() ) )
                       .arg( QgsPostgresConn::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) );

  QgsPostgresResult res( conn->PQexec( checkQuery ) );
  if ( res.PQntuples() > 0 )
  {
    if ( QMessageBox::question( nullptr, QObject::tr( "Save style in database" ),
                                QObject::tr( "A style named \"%1\" already exists in the database for this layer. Do you want to overwrite it?" )
                                .arg( styleName.isEmpty() ? dsUri.table() : styleName ),
                                QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
    {
      errCause = QObject::tr( "Operation aborted. No changes were made in the database" );
      conn->unref();
      return false;
    }

    sql = QString( "UPDATE layer_styles"
                   " SET useAsDefault=%1"
                   ",styleQML=XMLPARSE(DOCUMENT %12)"
                   ",styleSLD=XMLPARSE(DOCUMENT %13)"
                   ",description=%4"
                   ",owner=%5"
                   " WHERE f_table_catalog=%6"
                   " AND f_table_schema=%7"
                   " AND f_table_name=%8"
                   " AND f_geometry_column=%9"
                   " AND styleName=%10" )
          .arg( useAsDefault ? "true" : "false" )
          .arg( QgsPostgresConn::quotedValue( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription ) )
          .arg( QgsPostgresConn::quotedValue( dsUri.username() ) )
          .arg( QgsPostgresConn::quotedValue( dsUri.database() ) )
          .arg( QgsPostgresConn::quotedValue( dsUri.schema() ) )
          .arg( QgsPostgresConn::quotedValue( dsUri.table() ) )
          .arg( QgsPostgresConn::quotedValue( dsUri.geometryColumn() ) )
          .arg( QgsPostgresConn::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) )
          // Must be the final .arg replacement - see above
          .arg( QgsPostgresConn::quotedValue( qmlStyle ),
                QgsPostgresConn::quotedValue( sldStyle ) );
  }

  if ( useAsDefault )
  {
    QString removeDefaultSql = QString( "UPDATE layer_styles"
                                        " SET useAsDefault=false"
                                        " WHERE f_table_catalog=%1"
                                        " AND f_table_schema=%2"
                                        " AND f_table_name=%3"
                                        " AND f_geometry_column=%4" )
                               .arg( QgsPostgresConn::quotedValue( dsUri.database() ) )
                               .arg( QgsPostgresConn::quotedValue( dsUri.schema() ) )
                               .arg( QgsPostgresConn::quotedValue( dsUri.table() ) )
                               .arg( QgsPostgresConn::quotedValue( dsUri.geometryColumn() ) );
    sql = QStringLiteral( "BEGIN; %1; %2; COMMIT;" ).arg( removeDefaultSql, sql );
  }

  res = conn->PQexec( sql );

  bool saved = res.PQresultStatus() == PGRES_COMMAND_OK;
  if ( !saved )
    errCause = QObject::tr( "Unable to save layer style. It's not possible to insert a new record into the style table. Maybe this is due to table permissions (user=%1). Please contact your database administrator." ).arg( dsUri.username() );

  conn->unref();

  return saved;
}


QGISEXTERN QString loadStyle( const QString &uri, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return QString();
  }

  if ( dsUri.database().isEmpty() ) // typically when a service file is used
  {
    dsUri.setDatabase( conn->currentDatabase() );
  }

  if ( !tableExists( *conn, QStringLiteral( "layer_styles" ) ) )
  {
    conn->unref();
    return QString();
  }

  QString geomColumnExpr;
  if ( dsUri.geometryColumn().isEmpty() )
  {
    geomColumnExpr = QStringLiteral( "IS NULL" );
  }
  else
  {
    geomColumnExpr = QStringLiteral( "=" ) + QgsPostgresConn::quotedValue( dsUri.geometryColumn() );
  }

  QString selectQmlQuery = QString( "SELECT styleQML"
                                    " FROM layer_styles"
                                    " WHERE f_table_catalog=%1"
                                    " AND f_table_schema=%2"
                                    " AND f_table_name=%3"
                                    " AND f_geometry_column %4"
                                    " ORDER BY CASE WHEN useAsDefault THEN 1 ELSE 2 END"
                                    ",update_time DESC LIMIT 1" )
                           .arg( QgsPostgresConn::quotedValue( dsUri.database() ) )
                           .arg( QgsPostgresConn::quotedValue( dsUri.schema() ) )
                           .arg( QgsPostgresConn::quotedValue( dsUri.table() ) )
                           .arg( geomColumnExpr );

  QgsPostgresResult result( conn->PQexec( selectQmlQuery ) );

  QString style = result.PQntuples() == 1 ? result.PQgetvalue( 0, 0 ) : QString();
  conn->unref();

  return style;
}

QGISEXTERN int listStyles( const QString &uri, QStringList &ids, QStringList &names,
                           QStringList &descriptions, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed using username: %1" ).arg( dsUri.username() );
    return -1;
  }

  if ( dsUri.database().isEmpty() ) // typically when a service file is used
  {
    dsUri.setDatabase( conn->currentDatabase() );
  }

  QString selectRelatedQuery = QString( "SELECT id,styleName,description"
                                        " FROM layer_styles"
                                        " WHERE f_table_catalog=%1"
                                        " AND f_table_schema=%2"
                                        " AND f_table_name=%3"
                                        " AND f_geometry_column=%4"
                                        " ORDER BY useasdefault DESC, update_time DESC" )
                               .arg( QgsPostgresConn::quotedValue( dsUri.database() ) )
                               .arg( QgsPostgresConn::quotedValue( dsUri.schema() ) )
                               .arg( QgsPostgresConn::quotedValue( dsUri.table() ) )
                               .arg( QgsPostgresConn::quotedValue( dsUri.geometryColumn() ) );

  QgsPostgresResult result( conn->PQexec( selectRelatedQuery ) );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectRelatedQuery ) );
    errCause = QObject::tr( "Error executing the select query for related styles. The query was logged" );
    conn->unref();
    return -1;
  }

  int numberOfRelatedStyles = result.PQntuples();
  for ( int i = 0; i < numberOfRelatedStyles; i++ )
  {
    ids.append( result.PQgetvalue( i, 0 ) );
    names.append( result.PQgetvalue( i, 1 ) );
    descriptions.append( result.PQgetvalue( i, 2 ) );
  }

  QString selectOthersQuery = QString( "SELECT id,styleName,description"
                                       " FROM layer_styles"
                                       " WHERE NOT (f_table_catalog=%1 AND f_table_schema=%2 AND f_table_name=%3 AND f_geometry_column=%4)"
                                       " ORDER BY update_time DESC" )
                              .arg( QgsPostgresConn::quotedValue( dsUri.database() ) )
                              .arg( QgsPostgresConn::quotedValue( dsUri.schema() ) )
                              .arg( QgsPostgresConn::quotedValue( dsUri.table() ) )
                              .arg( QgsPostgresConn::quotedValue( dsUri.geometryColumn() ) );

  result = conn->PQexec( selectOthersQuery );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectOthersQuery ) );
    errCause = QObject::tr( "Error executing the select query for unrelated styles. The query was logged" );
    conn->unref();
    return -1;
  }

  for ( int i = 0; i < result.PQntuples(); i++ )
  {
    ids.append( result.PQgetvalue( i, 0 ) );
    names.append( result.PQgetvalue( i, 1 ) );
    descriptions.append( result.PQgetvalue( i, 2 ) );
  }

  conn->unref();

  return numberOfRelatedStyles;
}

QGISEXTERN bool deleteStyleById( const QString &uri, QString styleId, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  bool deleted;

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed using username: %1" ).arg( dsUri.username() );
    deleted = false;
  }
  else
  {
    QString deleteStyleQuery = QStringLiteral( "DELETE FROM layer_styles WHERE id=%1" ).arg(
                                 QgsPostgresConn::quotedValue( styleId ) );
    QgsPostgresResult result( conn->PQexec( deleteStyleQuery ) );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK )
    {
      QgsDebugMsg(
        QString( "PQexec of this query returning != PGRES_COMMAND_OK (%1 != expected %2): %3" )
        .arg( result.PQresultStatus() ).arg( PGRES_COMMAND_OK ).arg( deleteStyleQuery ) );
      QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( deleteStyleQuery ) );
      errCause = QObject::tr( "Error executing the delete query. The query was logged" );
      deleted = false;
    }
    else
    {
      deleted = true;
    }
    conn->unref();
  }
  return deleted;
}

QGISEXTERN QString getStyleById( const QString &uri, QString styleId, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed using username: %1" ).arg( dsUri.username() );
    return QString();
  }

  QString style;
  QString selectQmlQuery = QStringLiteral( "SELECT styleQml FROM layer_styles WHERE id=%1" ).arg( QgsPostgresConn::quotedValue( styleId ) );
  QgsPostgresResult result( conn->PQexec( selectQmlQuery ) );
  if ( result.PQresultStatus() == PGRES_TUPLES_OK )
  {
    if ( result.PQntuples() == 1 )
      style = result.PQgetvalue( 0, 0 );
    else
      errCause = QObject::tr( "Consistency error in table '%1'. Style id should be unique" ).arg( QStringLiteral( "layer_styles" ) );
  }
  else
  {
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectQmlQuery ) );
    errCause = QObject::tr( "Error executing the select query. The query was logged" );
  }

  conn->unref();

  return style;
}

QGISEXTERN QgsTransaction *createTransaction( const QString &connString )
{
  return new QgsPostgresTransaction( connString );
}


QgsPostgresProjectStorage *gProjectStorage = nullptr;   // when not null it is owned by QgsApplication::projectStorageRegistry()

QGISEXTERN void initProvider()
{
  Q_ASSERT( !gProjectStorage );
  gProjectStorage = new QgsPostgresProjectStorage;
  QgsApplication::projectStorageRegistry()->registerProjectStorage( gProjectStorage );  // takes ownership
}

QGISEXTERN void cleanupProvider()
{
  QgsApplication::projectStorageRegistry()->unregisterProjectStorage( gProjectStorage );  // destroys the object
  gProjectStorage = nullptr;

  QgsPostgresConnPool::cleanupInstance();
}


#ifdef HAVE_GUI

//! Provider for postgres source select
class QgsPostgresSourceSelectProvider : public QgsSourceSelectProvider  //#spellok
{
  public:

    QString providerKey() const override { return QStringLiteral( "postgres" ); }
    QString text() const override { return QObject::tr( "PostgreSQL" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderDatabaseProvider + 20; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddPostgisLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsPgSourceSelect( parent, fl, widgetMode );
    }
};


QGISEXTERN QList<QgsSourceSelectProvider *> *sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> *providers = new QList<QgsSourceSelectProvider *>();

  *providers
      << new QgsPostgresSourceSelectProvider;  //#spellok

  return providers;
}
#endif

// ----------

void QgsPostgresSharedData::addFeaturesCounted( long diff )
{
  QMutexLocker locker( &mMutex );

  if ( mFeaturesCounted >= 0 )
    mFeaturesCounted += diff;
}

void QgsPostgresSharedData::ensureFeaturesCountedAtLeast( long fetched )
{
  QMutexLocker locker( &mMutex );

  /* only updates the feature count if it was already once.
   * Otherwise, this would lead to false feature count if
   * an existing project is open at a restrictive extent.
   */
  if ( mFeaturesCounted > 0 && mFeaturesCounted < fetched )
  {
    QgsDebugMsg( QStringLiteral( "feature count adjusted from %1 to %2" ).arg( mFeaturesCounted ).arg( fetched ) );
    mFeaturesCounted = fetched;
  }
}

long QgsPostgresSharedData::featuresCounted()
{
  QMutexLocker locker( &mMutex );
  return mFeaturesCounted;
}

void QgsPostgresSharedData::setFeaturesCounted( long count )
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

  QVariantList v = mFidToKey[ fid ];
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
