/***************************************************************************
    qgsdamengprovider.cpp  -  QGIS data provider for Dameng/Dameng layers
                             -------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsmessagelog.h"
#include "qgsprojectstorageregistry.h"
#include "qgslayermetadataproviderregistry.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsxmlutils.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerexporter.h"
#include "qgsdamengprovider.h"
#include "qgsdamengconn.h"
#include "qgsdamengconnpool.h"
#include "qgsdamengdataitems.h"
#include "qgsdamengfeatureiterator.h"
#include "qgsdamengtransaction.h"
#include "qgsdamengprojectstorage.h"
#include "qgsdamengproviderconnection.h"
#include "qgslogger.h"
#include "qgsfeedback.h"
#include "qgssettings.h"
#include "qgsstringutils.h"
#include "qgsdbquerylog.h"
#include "qgsdbquerylog_p.h"
#include "qgsjsonutils.h"

#include "qgsdamengprovider.h"
#include "qgsprovidermetadata.h"
#include "qgsdamengproviderconnection.h"
#include <QRegularExpression>

const QString QgsDamengProvider::DAMENG_KEY = QStringLiteral( "dameng" );
const QString QgsDamengProvider::DAMENG_DESCRIPTION = QStringLiteral( "Dameng/Dameng data provider" );

static const QString EDITOR_WIDGET_STYLES_TABLE = QStringLiteral( "qgis_editor_widget_styles" );

inline qint64 PKINT2FID( qint32 x )
{
  return QgsDamengUtils::int32pk_to_fid( x );
}

inline qint32 FID2PKINT( qint64 x )
{
  return QgsDamengUtils::fid_to_int32pk( x );
}

static bool tableExists( QgsDamengConn &conn, const QString &name )
{
  QgsDMResult *res( conn.DMexec( QStringLiteral( "SELECT COUNT(*) from ( SELECT ID FROM SYSOBJECTS WHERE name = %1 );" ).arg( QgsDamengConn::quotedValue( name ) ) ) );
  res->fetchNext();
  return res->value( 0 ).toInt();
}

static bool columnExists( QgsDamengConn &conn, const QString &table, const QString &column )
{
  QgsDMResult *res( conn.DMexec( QStringLiteral( "SELECT COUNT(*) FROM ALL_TAB_COLUMNS WHERE table_name=%1 and column_name=%2;" ).arg( QgsDamengConn::quotedValue( table ) ).arg( QgsDamengConn::quotedValue( column ) ) ) );
  res->fetchNext();
  return res->value( 0 ).toInt() > 0;
}

QgsDamengPrimaryKeyType QgsDamengProvider::pkType( const QgsField &f ) const
{
  switch ( f.type() )
  {
    case QVariant::LongLong:
      return PktInt64;

    case QVariant::Int:
      return PktInt;

    default:
      return PktFidMap;
  }
}


QgsDamengProvider::QgsDamengProvider( QString const &uri, const ProviderOptions &options, Qgis::DataProviderReadFlags flags )
  : QgsVectorDataProvider( uri, options, flags )
  , mShared( new QgsDamengSharedData )
{
  QgsDebugMsgLevel( QStringLiteral( "URI: %1 " ).arg( uri ), 2 );

  mUri = QgsDataSourceUri( uri );

  // populate members from the uri structure
  mSchemaName = mUri.schema();
  mTableName = mUri.table();
  mGeometryColumn = mUri.geometryColumn();
  mBoundingBoxColumn = mUri.param( "bbox" );
  if ( mBoundingBoxColumn.isEmpty() )
  {
    mBoundingBoxColumn = mGeometryColumn;
  }
  mSqlWhereClause = mUri.sql();
  mRequestedSrid = mUri.srid();
  mRequestedGeomType = mUri.wkbType();

  const QString checkUnicityKey { QStringLiteral( "checkPrimaryKeyUnicity" ) };
  if ( mUri.hasParam( checkUnicityKey ) )
  {
    if ( mUri.param( checkUnicityKey ).compare( QLatin1String( "0" ) )  == 0 )
    {
      mCheckPrimaryKeyUnicity = false;
    }
    else
    {
      mCheckPrimaryKeyUnicity = true;
    }
    if ( mReadFlags & Qgis::DataProviderReadFlag::TrustDataSource )
    {
      mCheckPrimaryKeyUnicity = false;
    }
  }

  if ( mSchemaName.isEmpty() && mTableName.startsWith( '(' ) && mTableName.endsWith( ')' ) )
  {
    mIsQuery = true;
    setQuery( mTableName );
    mTableName.clear();
  }
  else
  {
    mIsQuery = false;

    setQuery( ( !mSchemaName.isEmpty() ? quotedIdentifier( mSchemaName ) + '.' : QString() ) + ( !mTableName.isEmpty() ? quotedIdentifier( mTableName ) : QString() ) );
  }

  mUseEstimatedMetadata = mUri.useEstimatedMetadata();
  if ( mReadFlags & Qgis::DataProviderReadFlag::TrustDataSource )
  {
    mUseEstimatedMetadata = true;
  }
  mSelectAtIdDisabled = mUri.selectAtIdDisabled();

  QgsDebugMsgLevel( QStringLiteral( "Connection info is %1" ).arg( mUri.connectionInfo( false ) ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Geometry column is: %1" ).arg( mGeometryColumn ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Schema is: %1" ).arg( mSchemaName ), 2 );
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

  mConnectionRO = QgsDamengConn::connectDb( mUri.connectionInfo( false ), true, true, false,  !mReadFlags.testFlag( Qgis::DataProviderReadFlag::SkipCredentialsRequest ) );
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
    QgsMessageLog::logMessage( tr( "Invalid Dameng layer" ), tr( "Dameng" ) );
    disconnectDb();
    return;
  }

  if ( mSpatialColType == SctTopoGeometry )
  {
    if ( !getTopoLayerInfo() ) // gets topology name and layer id
    {
      QgsMessageLog::logMessage( tr( "Invalid Dameng topology layer" ), tr( "Dameng" ) );
      mValid = false;
      disconnectDb();
      return;
    }
  }

  // set the primary key
  if ( !determinePrimaryKey() )
  {
    QgsMessageLog::logMessage( tr( "Dameng layer has no primary key." ), tr( "Dameng" ) );
    mValid = false;
    disconnectDb();
    return;
  }

  setNativeTypes( mConnectionRO->nativeTypes() );

  QString key;
  switch ( mPrimaryKeyType )
  {
    case PktRowId:
      key = QStringLiteral( "ROWID" );
      break;
    case PktInt:
      Q_ASSERT( mPrimaryKeyAttrs.size() == 1 );
      Q_ASSERT( mPrimaryKeyAttrs[0] >= 0 && mPrimaryKeyAttrs[0] < mAttributeFields.count() );
      key = mAttributeFields.at( mPrimaryKeyAttrs.at( 0 ) ).name();
      break;
    case PktInt64:
    case PktUint64:
    case PktFidMap:
    {
      QString delim;
      const auto constMPrimaryKeyAttrs = mPrimaryKeyAttrs;
      for ( int idx : constMPrimaryKeyAttrs )
      {
        key += delim + mAttributeFields.at( idx ).name();
        delim = ',';
      }
    }
    break;
    case PktUnknown:
      QgsMessageLog::logMessage( tr( "Dameng layer has unknown primary key type." ), tr( "Dameng" ) );
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

QgsDamengProvider::~QgsDamengProvider()
{
  disconnectDb();

  QgsDebugMsgLevel( QStringLiteral( "deconstructing." ), 3 );
}


QString QgsDamengProvider::providerKey()
{
  return DAMENG_KEY;
}

QgsAbstractFeatureSource *QgsDamengProvider::featureSource() const
{
  return new QgsDamengFeatureSource( this );
}

void QgsDamengProvider::disconnectDb()
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

QgsDamengConn *QgsDamengProvider::connectionRO() const
{
  if ( mTransaction )
    return mTransaction->connection();

  if ( !mConnectionRO )
    mConnectionRO = QgsDamengConn::connectDb( mUri.connectionInfo( false ), true, true, false, !mReadFlags.testFlag( Qgis::DataProviderReadFlag::SkipCredentialsRequest ) );

  return mConnectionRO;
}

QgsDamengConn *QgsDamengProvider::connectionRW()
{
  if ( mTransaction )
  {
    return mTransaction->connection();
  }
  else if ( !mConnectionRW )
  {
    mConnectionRW = QgsDamengConn::connectDb( mUri.connectionInfo( false ), false );
  }
  return mConnectionRW;
}

QgsTransaction *QgsDamengProvider::transaction() const
{
  return mTransaction;
}

void QgsDamengProvider::setTransaction( QgsTransaction *transaction )
{
  // static_cast since layers cannot be added to a transaction of a non-matching provider
  mTransaction = static_cast<QgsDamengTransaction *>( transaction );
}

Qgis::VectorLayerTypeFlags QgsDamengProvider::vectorLayerTypeFlags() const
{
  Qgis::VectorLayerTypeFlags flags;
  if ( mValid && mIsQuery )
  {
    flags.setFlag( Qgis::VectorLayerTypeFlag::SqlQuery );
  }
  return flags;
}

void QgsDamengProvider::handlePostCloneOperations( QgsVectorDataProvider *source )
{
  mShared = qobject_cast<QgsDamengProvider*>( source )->mShared;
}

void QgsDamengProvider::reloadProviderData()
{
  mShared->setFeaturesCounted(-1 );
  mLayerExtent.reset();
}

QString QgsDamengProvider::storageType() const
{
  return QStringLiteral( "Dameng database with DMGEO2 package" );
}

QgsFeatureIterator QgsDamengProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  if ( !mValid )
  {
    QgsMessageLog::logMessage( tr( "Read attempt on an invalid Dameng data source" ), tr( "Dameng" ) );
    return QgsFeatureIterator();
  }

  QgsDamengFeatureSource *featureSrc = static_cast<QgsDamengFeatureSource *>( featureSource() );
  return QgsFeatureIterator( new QgsDamengFeatureIterator( featureSrc, true, request ) );
}

QString QgsDamengProvider::quotedByteaValue( const QVariant &value )
{
  if ( value.isNull() )
    return QStringLiteral( "NULL" );

  return QStringLiteral( "\'0x%1\'" ).arg( value.toString() );
}


QString QgsDamengProvider::pkParamWhereClause( int offset, const char *alias ) const
{
  QString whereClause;

  QString aliased;
  if ( alias ) aliased = QStringLiteral( "%1." ).arg( alias );

  switch ( mPrimaryKeyType )
  {
    case PktRowId:
      whereClause = QStringLiteral( "%1ROWID=?" ).arg( aliased );
      break;

    case PktInt:
      Q_ASSERT( mPrimaryKeyAttrs.size() == 1 );
      whereClause = QStringLiteral( "%2%1=?" ).arg( quotedIdentifier( field( mPrimaryKeyAttrs[0] ).name() ) ).arg( aliased );
      break;

    case PktInt64:
    case PktUint64:
    case PktFidMap:
    {
      QString delim;
      for ( int i = 0; i < mPrimaryKeyAttrs.size(); i++ )
      {
        int idx = mPrimaryKeyAttrs[i];
        QgsField fld = field( idx );

        whereClause += delim + QStringLiteral( "%2%1=?" ).arg( connectionRO()->fieldExpressionForWhereClause( fld ) ).arg( aliased );
        offset++;
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

void QgsDamengProvider::appendPkParams( QgsFeatureId featureId, QStringList &params ) const
{
  switch ( mPrimaryKeyType )
  {
    case PktInt:
      params << QString::number( FID2PKINT( featureId ) );
      break;

    case PktRowId:
      params << QStringLiteral( "'(%1,%2)'" ).arg( FID_TO_NUMBER( featureId ) >> 16 ).arg( FID_TO_NUMBER( featureId ) & 0xffff );
      break;

    case PktInt64:
    case PktUint64:
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
          QgsDebugError( QStringLiteral( "FAILURE: Key value %1 for feature %2 not found." ).arg( mPrimaryKeyAttrs[i] ).arg( featureId ) );
          params << QStringLiteral( "NULL" );
        }
      }

      QgsDebugMsgLevel( QStringLiteral( "keys params: %1" ).arg( params.join( "; " ) ), 2 );
    }
    break;

    case PktUnknown:
      Q_ASSERT( !"FAILURE: Primary key unknown" );
      break;
  }
}


QString QgsDamengProvider::whereClause( QgsFeatureId featureId ) const
{
  return QgsDamengUtils::whereClause( featureId, mAttributeFields, connectionRO(), mPrimaryKeyType, mPrimaryKeyAttrs, mShared );
}

QString QgsDamengProvider::whereClause( QgsFeatureIds featureIds ) const
{
  return QgsDamengUtils::whereClause( featureIds, mAttributeFields, connectionRO(), mPrimaryKeyType, mPrimaryKeyAttrs, mShared );
}


QString QgsDamengUtils::whereClause( QgsFeatureId featureId, const QgsFields &fields, QgsDamengConn *conn, QgsDamengPrimaryKeyType pkType, const QList<int> &pkAttrs, const std::shared_ptr<QgsDamengSharedData> &sharedData )
{
  QString whereClause;

  switch ( pkType )
  {
    case PktInt:
      Q_ASSERT( pkAttrs.size() == 1 );
      whereClause = QStringLiteral( "%1=%2" ).arg( QgsDamengConn::quotedIdentifier( fields.at( pkAttrs[0] ).name() ) ).arg( FID2PKINT( featureId ) );
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
        if ( !pkVals[0].isNull() )
          whereClause += '=' + pkVals[0].toString();
        else
          whereClause += QLatin1String( " IS NULL" );
      }
    }
    break;

    case PktRowId:
      whereClause = QStringLiteral( "cast(ROWID as bigint) = %1" )
        .arg( FID_TO_NUMBER( featureId ) );
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
          if ( pkVals[i].isNull() )
            whereClause += QLatin1String( " IS NULL" );
          else
            whereClause += '=' + QgsDamengConn::quotedValue( pkVals[i] ); // remove toString as it must be handled by quotedValue function

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

QString QgsDamengUtils::whereClause( const QgsFeatureIds &featureIds, const QgsFields &fields, QgsDamengConn *conn, QgsDamengPrimaryKeyType pkType, const QList<int> &pkAttrs, const std::shared_ptr<QgsDamengSharedData> &sharedData )
{
  auto lookupKeyWhereClause = [=]
  {
    if ( featureIds.isEmpty() )
      return QString();

    QString delim;
    QString expr = QStringLiteral( "%1 IN (" ).arg( QgsDamengConn::quotedIdentifier( fields.at( pkAttrs[0] ).name() ) );

    for ( const QgsFeatureId featureId : std::as_const( featureIds ) )
    {
      const QVariantList pkVals = sharedData->lookupKey( featureId );
      if ( !pkVals.isEmpty() )
      {
        expr += delim + QgsDamengConn::quotedValue( pkVals.at( 0 ) );
        delim = ',';
      }
    }
    expr += ')';

    return expr;
  };

  switch ( pkType )
  {
    case PktInt:
    {
      QString expr;

      //simple primary key, so prefer to use an "IN (...)" query. These are much faster then multiple chained ...OR... clauses
      if ( !featureIds.isEmpty() )
      {
        QString delim;
        expr = QStringLiteral( "%1 IN (" ).arg( ( QgsDamengConn::quotedIdentifier( fields.at( pkAttrs[0] ).name() ) ) );

        for ( const QgsFeatureId featureId : std::as_const( featureIds ) )
        {
          expr += delim + FID_TO_STRING( FID2PKINT( featureId ) );
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
    case PktRowId:
    case PktUnknown:
    {
      // on simple string primary key we can use IN
      if ( pkType == PktFidMap && pkAttrs.count() == 1 && fields.at( pkAttrs[0] ).type() == QVariant::String )
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

QString QgsDamengUtils::andWhereClauses( const QString &c1, const QString &c2 )
{
  if ( c1.isEmpty() )
    return c2;
  if ( c2.isEmpty() )
    return c1;

  return QStringLiteral( "(%1) AND (%2)" ).arg( c1, c2 );
}

void QgsDamengUtils::replaceInvalidXmlChars( QString &xml )
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

void QgsDamengUtils::restoreInvalidXmlChars( QString &xml )
{
  static const QRegularExpression replaceRe { QStringLiteral( R"raw( UTF-8\[(\d+)\])raw" ) };
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

QString QgsDamengProvider::filterWhereClause() const
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
    QString geom;
    switch ( mSpatialColType )
    {
    case SctGeometry:
      geom = quotedIdentifier( mGeometryColumn );
      break;
    case SctGeography:
      geom = quotedIdentifier( mGeometryColumn ) + QLatin1String( "::SYSGEO2.st_geometry" );
      break;
    case SctTopoGeometry:
      geom = QStringLiteral( "SYSTOPOLOGY.DMTOPOLOGY.Geometry(%1)" )
        .arg( quotedIdentifier( mGeometryColumn ) );
      break;
    default:
      break;
    }

    where += delim + QStringLiteral( "%1(%2)=%3" )
             .arg( "DMGEO2.st_srid", geom, mRequestedSrid );
    delim = QStringLiteral( " AND " );
  }

  if ( mRequestedGeomType != Qgis::WkbType::Unknown && mRequestedGeomType != mDetectedGeomType )
  {
    where += delim + QgsDamengConn::dmSpatialTypeFilter( mGeometryColumn, ( Qgis::WkbType )mRequestedGeomType, mSpatialColType == SctGeography );
    delim = QStringLiteral( " AND " );
  }

  return where;
}

Qgis::WkbType QgsDamengProvider::wkbType() const
{
  return mRequestedGeomType != Qgis::WkbType::Unknown ? mRequestedGeomType : mDetectedGeomType;
}

QgsLayerMetadata QgsDamengProvider::layerMetadata() const
{
  return mLayerMetadata;
}

QString QgsDamengProvider::dataComment() const
{
  return mDataComment;
}

QgsField QgsDamengProvider::field( int index ) const
{
  if ( index < 0 || index >= mAttributeFields.count() )
  {
    QgsMessageLog::logMessage( tr( "FAILURE: Field %1 not found." ).arg( index ), tr( "Dameng" ) );
    throw DMFieldNotFound();
  }

  return mAttributeFields.at( index );
}

QgsFields QgsDamengProvider::fields() const
{
  return mAttributeFields;
}


//! \todo XXX Perhaps this should be promoted to QgsDataProvider?
QString QgsDamengProvider::endianString()
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

bool QgsDamengProvider::loadFields()
{
  // Clear cached information about enum values support
  mShared->clearSupportsEnumValuesCache();

  QString sql;
  QString attroidsFilter;

  if ( !mIsQuery )
  {
    QgsDebugMsgLevel( QStringLiteral( "Loading fields for table %1" ).arg( mTableName ), 2 );

    // Get the table description
    sql = QStringLiteral( "SELECT COMMENT$ FROM SYS.SYSTABLECOMMENTS WHERE SCHNAME = %1 and TVNAME = %2;"
                                  ).arg( quotedValue( mSchemaName ) ).arg( quotedValue( mTableName ) );
    QgsDMResult *tresult( connectionRO()->DMexec( sql ) );
    if ( tresult->fetchNext() )
    {
      mDataComment = tresult->value( 0 ).toString();
      mLayerMetadata.setAbstract( mDataComment );
    }
  }

  sql = QStringLiteral( "SELECT * FROM %1 LIMIT 0" ).arg( mQuery );

  QgsDamengResult result( connectionRO()->DMexec( sql ) );

  QMap< udint4, QMap<int, QString> > FieldTypeMap, fmtFieldTypeMap, descrMap, defValMap, identityMap, generatedMap;
  QMap< udint4, QMap<int, bool> > notNullMap, uniqueMap;
  if ( result.DMnfields() > 0 )
  {
    // Collect table ids
    QSet<sdint4> tableids;
    for ( int i = 0; i < result.DMnfields(); i++ )
    {
      sdint4 tableid = result.DMftable( i );
      if ( tableid >= 0 )
      {
        tableids.insert( tableid );
      }
    }

    if ( !tableids.isEmpty() )
    {
      QStringList tableoidsList;
      for ( sdint4 tableoid : tableids )
      {
        tableoidsList.append( QString::number( tableoid ) );
      }

      QString tableoidFilter = '(' + tableoidsList.join(QLatin1Char(',') ) + ')';

      // Collect formatted field types
      sql = QStringLiteral(
        "select C.ID,C.COLID,C.TYPE$,C.LENGTH$,C.SCALE,D.COMMENT$,C.DEFVAL,C.NULLABLE$,C.INFO2 & 1 IDENT, D.COLID - 1 "
        "from SYSCOLUMNS C "
        "LEFT OUTER JOIN( select A.COLUMN_ID COLID, COMMENT$, O.ID ID "
        "  from SYSOBJECTS O, SYS.ALL_TAB_COLUMNS A "
        "  LEFT OUTER JOIN( select TVNAME, SCHNAME, COLNAME, COMMENT$ from SYSCOLUMNCOMMENTS ) "
        "  ON TVNAME = A.TABLE_NAME and SCHNAME = A.OWNER and COLNAME = COLUMN_NAME"
        "  where A.OWNER in( select NAME from SYS.SYSOBJECTS "
        "    where ID in( select SCHID from SYSOBJECTS where ID in %1 ) "
        "  ) and O.ID in %1 and A.TABLE_NAME = O.NAME) D "
        "on D.ID = C.ID "
        "where C.ID in %1 and C.ID = D.ID and C.COLID = D.COLID - 1 "
      ).arg( tableoidFilter );

      QgsDMResult *fmtFieldTypeResult( connectionRO()->DMexec( sql ) );
      if ( !fmtFieldTypeResult || !fmtFieldTypeResult->execstatus() )
      {
        return false;
      }

      while ( fmtFieldTypeResult->fetchNext() )
      {
        udint4 tableid = fmtFieldTypeResult->value( 0 ).toUInt();
        int attnum = fmtFieldTypeResult->value( 1 ).toInt();
        QString formatType = fmtFieldTypeResult->value( 2 ).toString();
        QString TypeLength = fmtFieldTypeResult->value( 3 ).toString();
        QString TypeScale = fmtFieldTypeResult->value( 4 ).toString();

        QString descr = fmtFieldTypeResult->value( 5 ).toString();
        QString defVal = fmtFieldTypeResult->value( 6 ).toString();
        bool attNotNull = fmtFieldTypeResult->value( 7 ).toString() != "Y";
        QString attIdentity = fmtFieldTypeResult->value( 8 ).toString();

        FieldTypeMap[tableid][attnum] = formatType;
        fmtFieldTypeMap[tableid][attnum] = formatType + QStringLiteral( "(%1,%2)" ).arg( TypeLength ).arg( TypeScale );
        descrMap[tableid][attnum] = descr;
        defValMap[tableid][attnum] = defVal;
        notNullMap[tableid][attnum] = attNotNull;
        
        identityMap[tableid][attnum] = attIdentity == '0' ? QString() : attIdentity;
        generatedMap[tableid][attnum] = identityMap[tableid][attnum];
        uniqueMap[tableid][attnum] = false;
      }

      sql = QStringLiteral( "select A.ID, B.POSITION-1, C.type$ "
                        "  from( select ID, SCHID, NAME from SYS.SYSOBJECTS ) A, "
                        "     ( select TABLE_NAME, COLUMN_NAME, CONSTRAINT_NAME, POSITION from SYS.ALL_CONS_COLUMNS "
                        "         where TABLE_NAME in( select NAME from SYSOBJECTS where ID in %1 ) "
                        "         and OWNER in( select NAME from SYSOBJECTS where "
                        "            ID in( select SCHID from SYSOBJECTS where ID in %1) ) "
                        "         and substr(CONSTRAINT_NAME,1,4) = \'CONS\') B, "
                        "     ( select TABLEID, ID, TYPE$ from syscons where TYPE$ = \'P\' or TYPE$ = \'U\') C "
                        "  where A.ID in %1 and B.TABLE_name = A.NAME and C.TABLEID = A.ID "
                        "     and cast( substr(B.CONSTRAINT_NAME, 5) AS bigint ) = C.ID;"
            ).arg( tableoidFilter );
      fmtFieldTypeResult = connectionRO()->DMexec( sql );
      if ( fmtFieldTypeResult && fmtFieldTypeResult->execstatus() )
      {
        while ( fmtFieldTypeResult->fetchNext() )
        {
          uint tableid = fmtFieldTypeResult->value( 0 ).toUInt();
          int attnum = fmtFieldTypeResult->value( 1 ).toInt();

          QString Constraint_tmp = fmtFieldTypeResult->value( 2 ).toString();
          uniqueMap[tableid][attnum] = true;
        }
      }

    }
  }

  QSet<QString> fields;
  mAttributeFields.clear();
  mIdentityFields.clear();

  result = connectionRO()->DMexec( QStringLiteral( "SELECT * FROM %1 LIMIT 0" ).arg( mQuery ) );
  
  for ( int i = 0; i < result.DMnfields(); i++ )
  {
    sdint4 tableoid = result.DMftable( i );
    if ( tableoid == -1 )
    {
      continue;
    }

    QString fieldName = result.DMfname( i );
    if ( fieldName == mGeometryColumn )
      continue;

    int attnum = i;

    QString formattedFieldType = fmtFieldTypeMap[tableoid][attnum];
    QString originalFormattedFieldType = formattedFieldType;

    QString fieldComment = descrMap[tableoid][attnum];

    QString fieldTypeName = FieldTypeMap[tableoid][attnum].toLower();
    QVariant::Type fieldType = QVariant::Invalid;
    QVariant::Type fieldSubType = QVariant::Invalid;
    int fieldSize = -1;

    int fieldPrec = 0;

    if ( fieldTypeName == QLatin1String( "bigint" ) || fieldTypeName == QLatin1String( "rowid" ) )
    {
      fieldType = QVariant::LongLong;
    }
    else if ( fieldTypeName == QLatin1String( "smallint" ) ||
              fieldTypeName == QLatin1String( "int" ) || fieldTypeName == QLatin1String( "integer" ) ||
              fieldTypeName == QLatin1String( "tinyint" ) || fieldTypeName == QLatin1String( "byte" ) )
    {
      fieldType = QVariant::Int;
    }
    else if ( fieldTypeName == QLatin1String( "real" ) || fieldTypeName == QLatin1String( "double precision" ) ||
              fieldTypeName == QLatin1String( "float" ) || fieldTypeName == QLatin1String( "double" ) )
    {
      fieldType = QVariant::Double;

      if ( fieldTypeName != QLatin1String( "real" ) )
      {
        const QRegularExpression re( QRegularExpression::anchoredPattern( QStringLiteral( "%1\\((\\d+),(\\d+)\\)" ).arg( fieldTypeName.toLower() ) ) );
        const QRegularExpressionMatch match = re.match( formattedFieldType.toLower() );
        if ( match.hasMatch() )
        {
          fieldSize = match.captured( 1 ).toInt();
        }
      }
    }
    else if ( fieldTypeName == QLatin1String( "number" ) || fieldTypeName == QLatin1String( "numeric" ) ||
              fieldTypeName == QLatin1String( "dec" ) || fieldTypeName == QLatin1String( "decimal" ) )
    {
      fieldType = QVariant::Double;

      const QRegularExpression re( QRegularExpression::anchoredPattern( QStringLiteral( "%1\\((\\d+),(\\d+)\\)" ).arg( fieldTypeName.toLower()) ) );
      const QRegularExpressionMatch match = re.match( formattedFieldType.toLower() );
      if ( match.hasMatch() )
      {
        fieldSize = match.captured( 1 ).toInt();
        fieldPrec = match.captured( 2 ).toInt();
      }
      else if ( formattedFieldType != QLatin1String( "numeric" ) )
      {
        QgsMessageLog::logMessage( tr( "Unexpected formatted field type '%1' for field %2" )
                                    .arg( formattedFieldType, fieldName ),
                                    tr( "Dameng" ) );
      }
      
    }
    else if ( fieldTypeName == QLatin1String( "varchar" ) || fieldTypeName == QLatin1String( "varchar2" ) ||
              fieldTypeName == QLatin1String( "longvarchar" ) )
    {
      fieldType = QVariant::String;

      if ( fieldTypeName != QLatin1String( "longvarchar" ) )
      {
        const QRegularExpression re( QRegularExpression::anchoredPattern( QStringLiteral( "%1\\((\\d+),(\\d+)\\)" ).arg( fieldTypeName.toLower() ) ) );
        const QRegularExpressionMatch match = re.match( formattedFieldType.toLower() );
        if ( match.hasMatch() )
        {
          fieldSize = match.captured( 1 ).toInt();
        }
      }
    }
    else if ( fieldTypeName == QLatin1String( "bit" ) )
    {
      fieldType = QVariant::Bool;
    }
    else if ( fieldTypeName == QLatin1String( "date" ) )
    {
      fieldType = QVariant::Date;
    }
    else if ( fieldTypeName == QLatin1String( "time" ) )
    {
      fieldType = QVariant::Time;

      const QRegularExpression re( QRegularExpression::anchoredPattern( QStringLiteral( "%1\\((\\d+),(\\d+)\\)" ).arg( fieldTypeName.toLower() ) ) );
      const QRegularExpressionMatch match = re.match( formattedFieldType.toLower() );
      if ( match.hasMatch() )
      {
        fieldSize = match.captured( 1 ).toInt();
        fieldPrec = match.captured( 2 ).toInt();
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Unexpected formatted field type '%1' for field %2" )
          .arg( formattedFieldType, fieldName ) );
      }
    }
    else if ( fieldTypeName == QLatin1String( "timestamp" ) || fieldTypeName.startsWith( QStringLiteral( "datetime" ) ) )
    {
      fieldType = QVariant::DateTime;

      const QRegularExpression re( QRegularExpression::anchoredPattern( QStringLiteral( "%1\\((\\d+),(\\d+)\\)" ).arg( fieldTypeName.toLower() ) ) );
      const QRegularExpressionMatch match = re.match( formattedFieldType.toLower() );
      if ( match.hasMatch() )
      {
        fieldSize = match.captured( 1 ).toInt();
        fieldPrec = match.captured( 2 ).toInt();
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Unexpected formatted field type '%1' for field %2" )
          .arg( formattedFieldType, fieldName ) );
      }
    }
    else if ( fieldTypeName == QLatin1String( "binary" ) ||
              fieldTypeName == QLatin1String( "varbinary" ) ||
              fieldTypeName == QLatin1String( "longvarbinary" ) ||
              fieldTypeName == QLatin1String( "raw" ) ||
              fieldTypeName == QLatin1String( "blob" ) ||
              fieldTypeName == QLatin1String( "image" ) )
    {
      fieldType = QVariant::ByteArray;

      if ( fieldTypeName == QLatin1String( "binary" ) ||
           fieldTypeName == QLatin1String( "varbinary" ) ||
           fieldTypeName == QLatin1String( "raw" ) )
      {
        const QRegularExpression re( QRegularExpression::anchoredPattern( QStringLiteral( "%1\\((\\d+),(\\d+)\\)" ).arg( fieldTypeName.toLower() ) ) );
        const QRegularExpressionMatch match = re.match( formattedFieldType.toLower() );
        if ( match.hasMatch() )
        {
          fieldSize = match.captured( 1 ).toInt();
        }
        else
        {
          QgsMessageLog::logMessage( tr( "Unexpected formatted field type '%1' for field %2" )
            .arg( formattedFieldType, fieldName ) );
        }
      }
    }
    else if ( fieldTypeName == QLatin1String( "text" ) ||
              fieldTypeName == QLatin1String( "clob" ) ||
              fieldTypeName == QLatin1String( "bfile" ) ||
              fieldTypeName.startsWith( QLatin1String( "time" ) ) ||
              fieldTypeName.startsWith( QLatin1String( "interval" ) ) )
    {
      fieldType = QVariant::String;
    }
    else if ( fieldTypeName.startsWith( "class" ) &&
              fieldTypeName.mid( 5 ).toInt() >= NDCT_CLSID_GEO2_ST_GEOMETRY && 
              fieldTypeName.mid( 5 ).toInt() <= NDCT_CLSID_GEO2_ST_GEOGRAPHY )
    {
      fieldTypeName = QgsDMResult::getGeoName( fieldTypeName.mid( 5 ).toUInt() );
      if ( fieldTypeName != QLatin1String( "unknow" ) )
      {
        if ( fieldTypeName == QLatin1String( "ST_GEOGRAPHY" ) )
          fieldTypeName = QStringLiteral( "geography" );
        else 
          fieldTypeName = QStringLiteral( "geometry" );
      }
      
      fieldType = QVariant::String;
    }
    else if ( fieldTypeName == QLatin1String( "char" ) || fieldTypeName == QLatin1String( "character" ) )
    {
      fieldType = QVariant::String;

      const QRegularExpression re( QRegularExpression::anchoredPattern( QStringLiteral(  "%1\\((\\d+),(\\d+)\\)"  ).arg( fieldTypeName.toLower() ) ) );
      const QRegularExpressionMatch match = re.match( formattedFieldType.toLower() );
      if ( match.hasMatch() )
      {
        fieldSize = match.captured( 1 ).toInt();
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Unexpected formatted field type '%1' for field %2" )
                                    .arg( formattedFieldType, fieldName ) );
      }
    }
    else if ( fieldTypeName == QLatin1String( "json" ) || fieldTypeName == QLatin1String( "jsonb" ) )
    {
      fieldType = QVariant::Map;
      fieldSubType = QVariant::String;
    }
    else
    {
      const QgsDamengProvider::Relkind type = relkind();
      if ( ( type == Relkind::View || type == Relkind::MaterializedView ) && parseUriKey( mUri.keyColumn() ).contains( fieldName ) )
      {
        fieldType = QVariant::String;
      }
      else if ( fieldTypeName == QLatin1String( "unknown" ) )
      {
        fieldType = QVariant::String;
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Field %1 ignored, because of unsupported type %2" ).arg( fieldName, fieldTypeName ), tr( "Dameng" ) );
        continue;
      }
    }
    

    if ( fields.contains( fieldName ) )
    {
      QgsMessageLog::logMessage( tr( "Duplicate field %1 found\n" ).arg( fieldName ), tr( "Dameng" ) );
      // In case of read-only query layers we can safely ignore the issue and rename the duplicated field
      if ( ! mIsQuery )
      {
        return false;
      }
      else
      {
        unsigned short int i = 1;
        while ( i < std::numeric_limits< unsigned short int >::max() )
        {
          const QString newName { QStringLiteral( "%1 (%2)" ).arg( fieldName ).arg( ++i ) };
          if ( ! fields.contains( newName ) )
          {
            fieldName = newName;
            break;
          }
        }
      }
    }

    fields << fieldName;

    mAttrPalIndexName.insert( i, fieldName );

    // If this is an identity field with constraints 
    if ( !identityMap[tableoid][attnum].isEmpty() )
    { 
      defValMap[tableoid][attnum] = QStringLiteral( "ident_current(\'%1.%2\')+ident_incr(\'%1.%2\');" ).arg( mSchemaName ).arg( mTableName );
    }

    mDefaultValues.insert( mAttributeFields.size(), defValMap[tableoid][attnum] );

    const QString generatedValue = generatedMap[tableoid][attnum];
    if ( !generatedValue.isNull() )
      mGeneratedValues.insert( mAttributeFields.size(), generatedValue );

    QgsField newField = QgsField( fieldName, fieldType, fieldTypeName, fieldSize, fieldPrec, fieldComment, fieldSubType );
    newField.setReadOnly( !generatedValue.isNull() );

    QgsFieldConstraints constraints;
    if ( notNullMap[tableoid][attnum] || ( mPrimaryKeyAttrs.size() == 1 && mPrimaryKeyAttrs[0] == i ) || !identityMap[tableoid][attnum].isEmpty() )
      constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
    if ( uniqueMap[tableoid][attnum] || ( mPrimaryKeyAttrs.size() == 1 && mPrimaryKeyAttrs[0] == i ) || !identityMap[tableoid][attnum].isEmpty() )
      constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
    newField.setConstraints( constraints );

    mIdentityFields.insert( mAttributeFields.size(), identityMap[tableoid][attnum][0].toLatin1() );
    mAttributeFields.append( newField );
  }

  setEditorWidgets();

  return true;
}

void QgsDamengProvider::setEditorWidgets()
{
  if ( ! tableExists( *connectionRO(), EDITOR_WIDGET_STYLES_TABLE ) )
  {
    return;
  }

  QStringList quotedFnames;
  const QStringList fieldNames = mAttributeFields.names();
  for ( const QString &name : fieldNames )
  {
    quotedFnames << quotedValue( name );
  }

  // We expect the table to be created like this:
  //
  // CREATE TABLE qgis_editor_widget_styles ( schema_name TEXT NOT NULL, table_name TEXT NOT NULL, field_name TEXT NOT NULL,
  //                                         type TEXT NOT NULL, config TEXT,
  //                                         PRIMARY KEY( schema_name, table_name, field_name ) );
  const QString sql = QStringLiteral( "SELECT field_name, type, config "
                                      "FROM %1 WHERE schema_name = %2 "
                                      "AND table_name = %3 "
                                      "AND field_name IN ( %4 )" ) .
                      arg( EDITOR_WIDGET_STYLES_TABLE, quotedValue( mSchemaName ),
                           quotedValue( mTableName ), quotedFnames.join( "," ) );
  QgsDMResult *result( connectionRO()->DMexec( sql ) );
  while ( result->fetchNext() )
  {
    if ( result->value( 2 ).toString().isNull() ) continue; // config can be null and it's OK

    const QString &configTxt = result->value( 2 ).toString();
    const QString &type = result->value( 1 ).toString();
    const QString &fname = result->value( 0 ).toString();
    QVariantMap config;
    QDomDocument doc;
    if ( doc.setContent( configTxt ) )
    {
      config = QgsXmlUtils::readVariant( doc.documentElement() ).toMap();
    }
    else
    {
      QgsMessageLog::logMessage(
        tr( "Cannot parse widget configuration for field %1.%2.%3\n" )
        .arg( mSchemaName, mTableName, fname ), tr( "Dameng" )
      );
      continue;
    }

    // Set corresponding editor widget
    for ( auto &field : mAttributeFields )
    {
      if ( field.name() == fname )
      {
        field.setEditorWidgetSetup( QgsEditorWidgetSetup( type, config ) );
        break;
      }
    }
  }
}

bool QgsDamengProvider::hasSufficientPermsAndCapabilities()
{
  QgsDebugMsgLevel( QStringLiteral( "Checking for permissions on the relation" ), 2 );

  mEnabledCapabilities = Qgis::VectorProviderCapability::ReloadData;

  QgsDMResult *testAccess;

  bool forceReadOnly = ( mReadFlags & Qgis::DataProviderReadFlag::ForceReadOnly );

  if ( !mIsQuery )
  {
    // Check that we can read from the table ( i.e., we have select permission ).
    QString sql = QStringLiteral( "SELECT * FROM %1 LIMIT 1" ).arg( mQuery );
    QgsDMResult *testAccess( connectionRO()->DMexec( sql ) );
    if ( !testAccess || !testAccess->execstatus() )
    {
      QgsMessageLog::logMessage( tr( "Unable to access the %1 relation.\nThe error message from the database was:\n%2.\nSQL: %3" )
                                 .arg( mQuery,
                                       testAccess->getMsg(),
                                       sql ), tr( "Dameng" ) );
      return false;
    }

    bool inRecovery = false;

    {
      testAccess = connectionRO()->DMexec( QStringLiteral( "select ROLE$ from V$DATABASE;" ) );
      testAccess->fetchNext();
      if ( testAccess && testAccess->execstatus() && testAccess->value( 0 ).toInt() == 2 )
      {
        QgsMessageLog::logMessage( tr( "You are connected to a (read-only) standby server，Write accesses will be denied." ) );
        inRecovery = true;
      }
    }

    // dameng has fast access to features at id ( thanks to primary key / unique index )
    // the latter flag is here just for compatibility
    if ( !mSelectAtIdDisabled )
    {
      mEnabledCapabilities |= Qgis::VectorProviderCapability::SelectAtId;
    }

    // Do not set editable capabilities if the provider has been forced to be
    // in read-only mode or if the database is still in recovery
    if ( !forceReadOnly && !inRecovery )
    {
      if ( relkind() == Relkind::OrdinaryTable || ( relkind() == Relkind::Unknown && mSchemaName.isEmpty() ) )
      {
        if ( mSchemaName.isEmpty() )
        {
          sql = QString( "SELECT SYS_CONTEXT('userenv', 'current_schema');" );

          testAccess = connectionRO()->DMexec( sql );
          testAccess->fetchNext();
          mSchemaName = testAccess->value( 0 ).toString();
        }

        sql = QString( "SELECT "
          " SF_CHECK_USER_TABLE_PRIV(%1, %2, USER, 2 )," //delete
          " SF_CHECK_USER_TABLE_PRIV(%1, %2, USER, 3 )," //update
          " SF_CHECK_USER_TABLE_PRIV(%1, %2, USER, 3 )," //update
          " SF_CHECK_USER_TABLE_PRIV(%1, %2, USER, 1 )," //insert
          " SYS_CONTEXT('userenv', 'current_schema');" )
          .arg( quotedValue( mSchemaName ), quotedValue( mTableName ) );

        testAccess = connectionRO()->DMexec( sql );
        if ( !testAccess || !testAccess->execstatus() || !testAccess->fetchNext() )
        {
          QgsMessageLog::logMessage( tr( "Unable to determine table access privileges for the %1 relation.\nThe error message from the database was:\n%2.\nSQL: %3" )
            .arg( mQuery,
              testAccess->getMsg(),
              sql ),
            tr( "Dameng" ) );
          return false;
        }

        if ( testAccess->value( 0 ).toInt() == 1 )
        {
          // DELETE
          mEnabledCapabilities |= Qgis::VectorProviderCapability::DeleteFeatures | Qgis::VectorProviderCapability::FastTruncate;
        }

        if ( testAccess->value( 1 ).toInt() == 1 )
        {
          // UPDATE
          mEnabledCapabilities |= Qgis::VectorProviderCapability::ChangeAttributeValues;
        }

        if ( testAccess->value( 2 ).toInt() == 1 )
        {
          // UPDATE
          mEnabledCapabilities |= Qgis::VectorProviderCapability::ChangeGeometries;
        }

        if ( testAccess->value( 3 ).toInt() == 1 )
        {
          // INSERT
          mEnabledCapabilities |= Qgis::VectorProviderCapability::AddFeatures;
        }

        if ( mSchemaName.isEmpty() )
          mSchemaName = testAccess->value( 4 ).toString();
      }
      else
      {
        sql = QString( "SELECT SYS_CONTEXT('userenv', 'current_schema');" );

        testAccess = connectionRO()->DMexec( sql );
        if ( !testAccess || !testAccess->execstatus() || !testAccess->fetchNext() )
        {
          QgsMessageLog::logMessage( tr( "Unable to determine table access privileges for the %1 relation.\nThe error message from the database was:\n%2.\nSQL: %3" )
            .arg( mQuery, testAccess->getMsg(), sql ),
            tr( "Dameng" ) );
          return false;
        }
        if ( mSchemaName.isEmpty() )
          mSchemaName = testAccess->value( 0 ).toString();
      }

      mEnabledCapabilities |= Qgis::VectorProviderCapability::AddAttributes | Qgis::VectorProviderCapability::DeleteAttributes | Qgis::VectorProviderCapability::RenameAttributes;
    }
  }
  else
  {
    // Check if the sql is a select query
    if ( !mQuery.startsWith( '(' ) && !mQuery.endsWith( ')' ) )
    {
      QgsMessageLog::logMessage( tr( "The custom query is not a select query." ), tr( "Dameng" ) );
      return false;
    }

    QString sql = QStringLiteral( "SELECT * FROM %1 LIMIT 1" ).arg( mQuery );

    testAccess = connectionRO()->DMexec( sql );
    if ( !testAccess || !testAccess->execstatus() )
    {
      QgsMessageLog::logMessage( tr( "Unable to execute the query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                                 .arg( testAccess->getMsg(),
                                       sql ), tr( "Dameng" ) );
      return false;
    }

    if ( !mSelectAtIdDisabled )
    {
      mEnabledCapabilities = Qgis::VectorProviderCapability::SelectAtId;
    }
  }

  // supports geometry simplification on provider side
  mEnabledCapabilities |= ( Qgis::VectorProviderCapability::SimplifyGeometries | Qgis::VectorProviderCapability::SimplifyGeometriesWithTopologicalValidation );

  // supports transactions
  mEnabledCapabilities |= Qgis::VectorProviderCapability::TransactionSupport;

  // supports circular geometries
  mEnabledCapabilities |= Qgis::VectorProviderCapability::CircularGeometries;

  // supports layer metadata
  mEnabledCapabilities |= Qgis::VectorProviderCapability::ReadLayerMetadata;

  if ( ( mEnabledCapabilities & Qgis::VectorProviderCapability::ChangeGeometries ) &&
       ( mEnabledCapabilities & Qgis::VectorProviderCapability::ChangeAttributeValues ) &&
       mSpatialColType != SctTopoGeometry )
  {
    mEnabledCapabilities |= Qgis::VectorProviderCapability::ChangeFeatures;
  }

  return true;
}

bool QgsDamengProvider::determinePrimaryKey()
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
    sql = QStringLiteral( "select count(*) from ALL_CONSTRAINTS "
                          " where OWNER = %1 and TABLE_NAME = %2 and "
                          " CONSTRAINT_TYPE = \'F\'; " ).arg( quotedValue( mSchemaName ) ).arg( quotedValue( mTableName ) );
    QgsDebugMsgLevel( QStringLiteral( "Checking whether %1 is a parent table" ).arg( sql ), 2 );
    QgsDMResult *res( connectionRO()->DMexec( sql ) );
    bool isParentTable( !res->fetchNext() || res->value( 0 ).toInt() > 0 );

    sql = QStringLiteral( "select count(*) from ALL_CONSTRAINTS "
                          " where OWNER = %1 and TABLE_NAME = %2"
                          " and ( CONSTRAINT_TYPE = \'U\' or CONSTRAINT_TYPE = \'P\'); "
                        ).arg( quotedValue( mSchemaName ) ).arg( quotedValue( mTableName ) );
    QgsDebugMsgLevel( QStringLiteral( "Retrieving first primary or unique index: %1" ).arg( sql ), 2 );

    res = connectionRO()->DMexec( sql );

    // no primary or unique indices found
    res->fetchNext();
    if ( !res->value( 0 ).toInt() )
    {
      QgsDebugMsgLevel( QStringLiteral( "Relation has no primary key -- investigating alternatives" ), 2 );

      const QgsDamengProvider::Relkind type = relkind();

      if ( type == Relkind::OrdinaryTable || type == Relkind::MaterializedView )
      {
        QgsDebugMsgLevel( QStringLiteral( "Relation is a table. Checking to see if it has an udint4 column." ), 2 );

        mPrimaryKeyAttrs.clear();
        mPrimaryKeyType = PktUnknown;

        sql = QStringLiteral( "select COLID from SYSCOLUMNS where INFO2 & 1= 1 and "
                            " ID in ( select ID from SYSOBJECTS where name = %2 and "
                            " SCHID = ( select ID from SYSOBJECTS where name = %1 and TYPE$ = \'SCH\') ); "
                            ).arg( quotedValue( mSchemaName ) )
                            .arg( type == Relkind::OrdinaryTable
                              ? quotedValue( mTableName )
                              : quotedValue( QStringLiteral( "MTAB$_%1" ).arg( mTableName ) ) );
        res = connectionRO()->DMexec( sql );

        if ( res->fetchNext() )
        {
          // Could warn the user here that performance will suffer if
          // attribute isn't indexed ( and that they may want to add a
          // primary key to the table )
          int idx = res->value( 0 ).toInt();
          mPrimaryKeyType = pkType( mAttributeFields.at( idx ) );
          mPrimaryKeyAttrs << idx;
          
        }

        if ( mPrimaryKeyType == PktUnknown )
        {
            mPrimaryKeyType = PktRowId;

            QgsMessageLog::logMessage( tr( "Primary key is Rowid - changing of existing features disabled (%1; %2)" ).arg( mGeometryColumn, mQuery ) );
            mEnabledCapabilities &= ~( Qgis::VectorProviderCapability::DeleteFeatures | Qgis::VectorProviderCapability::ChangeAttributeValues | Qgis::VectorProviderCapability::ChangeGeometries | Qgis::VectorProviderCapability::ChangeFeatures );
        }

        if ( mPrimaryKeyType == PktUnknown )
        {
          QgsMessageLog::logMessage( tr( "The table has no column suitable for use as a key. QGIS requires a primary key, a Dameng Rowid column for tables." ), tr( "Dameng" ) );
        }
      }
      else if ( type == Relkind::View )
      {
        determinePrimaryKeyFromUriKeyColumn();
      }
      else
      {
        const QMetaEnum metaEnum( QMetaEnum::fromType<Relkind>() );
        QString typeName = metaEnum.valueToKey( type );
        QgsMessageLog::logMessage( tr( "Unexpected relation type '%1'." ).arg( typeName ), tr( "Dameng" ) );
      }
    }
    else
    {
      // have a primary key or unique index
      QString indrelid = res->value( 0 ).toString();
      sql = QStringLiteral( "select distinct( d.column_name ),NULLABLE "
                            " FROM SYSCONS c, SYSOBJECTS o, SYS.ALL_TAB_COLUMNS d, ALL_CONS_COLUMNS a"
                            " where d.OWNER = a.OWNER and d.TABLE_NAME = a.TABLE_NAME and o.id = c.id and"
                            " ( c.TYPE$ = \'U\' or c.TYPE$ = \'P\') and o.name = a.CONSTRAINT_NAME and "
                            " a.COLUMN_NAME = d.COLUMN_NAME and a.OWNER = %1 and a.TABLE_NAME = %2; "
                          ).arg( quotedValue( mSchemaName ) ).arg( quotedValue( mTableName ) );

      QgsDebugMsgLevel( "Retrieving key columns: " + sql, 2 );
      res = connectionRO()->DMexec( sql, true, true );
      QgsDebugMsgLevel( QStringLiteral( "Got %1 rows." ).arg( res.DMntuples() ), 2 );

      bool mightBeNull = false;
      QString primaryKey;
      QString delim;

      mPrimaryKeyType = PktFidMap; // map by default, will downgrade if needed
      int i = 0;
      while ( res->fetchNext() )
      {

        QString name = res->value( 0 ).toString();
        if ( res->value( 1 ).toString().startsWith( 'Y' ) )
        {
          QgsMessageLog::logMessage( tr( "Unique column '%1' doesn't have a NOT NULL constraint." ).arg( name ), tr( "Dameng" ) );
          mightBeNull = true;
        }

        primaryKey += delim + quotedIdentifier( name );
        delim = ',';

        int idx = fieldNameIndex( name );
        if ( idx == -1 )
        {
          QgsDebugMsgLevel( "Skipping " + name, 2 );
          continue;
        }
        QgsField fld = mAttributeFields.at( idx );

        // Always use PktFidMap for multi-field keys
        mPrimaryKeyType = i ? PktFidMap : pkType( fld );
        ++i;
        mPrimaryKeyAttrs << idx;
      }

      if ( ( mightBeNull || isParentTable ) && !mUseEstimatedMetadata && !uniqueData( primaryKey ) )
      {
        QgsMessageLog::logMessage( tr( "Ignoring key candidate because of NULL values or inheritance" ), tr( "Dameng" ) );
        mPrimaryKeyType = PktUnknown;
        mPrimaryKeyAttrs.clear();
      }
    }
  }
  else
  {
    determinePrimaryKeyFromUriKeyColumn();
  }

  if ( mPrimaryKeyAttrs.size() == 1 )
  {
    //primary keys are unique, not null
    QgsFieldConstraints constraints = mAttributeFields.at( mPrimaryKeyAttrs[0] ).constraints();
    constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
    constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
    mAttributeFields[ mPrimaryKeyAttrs[0] ].setConstraints( constraints );
  }

  mValid = mPrimaryKeyType != PktUnknown;

  return mValid;
}

/* static */
QStringList QgsDamengProvider::parseUriKey( const QString &key )
{
  if ( key.isEmpty() )
    return QStringList();

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

void QgsDamengProvider::determinePrimaryKeyFromUriKeyColumn()
{
  QString primaryKey = mUri.keyColumn();
  mPrimaryKeyType = PktUnknown;

  if ( !primaryKey.isEmpty() )
  {
    const QStringList cols = parseUriKey( primaryKey );

    primaryKey.clear();
    QString del;
    for ( const QString &col : cols )
    {
      primaryKey += del + quotedIdentifier( col );
      del = QStringLiteral( "," );
    }

    if ( cols[0].toUpper() == QStringLiteral( "ROWID" ) )
    {
      mPrimaryKeyType = PktRowId;
      return;
    }
      
    for ( const QString &col : cols )
    {
      int idx = fieldNameIndex( col );
      if ( idx < 0 )
      {
        QgsMessageLog::logMessage( tr( "Key field '%1' for view/query not found." ).arg( col ), tr( "Dameng" ) );
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
          QgsField fld = mAttributeFields.at( mPrimaryKeyAttrs.at( 0 ) );
          mPrimaryKeyType = pkType( fld );
        }
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Primary key field '%1' for view/query not unique." ).arg( primaryKey ), tr( "Dameng" ) );
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Keys for view/query undefined." ), tr( "Dameng" ) );
    }
  }
  else
  {
    QgsMessageLog::logMessage( tr( "No key field for view/query given." ), tr( "Dameng" ) );
  }
}

bool QgsDamengProvider::uniqueData( const QString &quotedColNames )
{
  // Check to see if the given columns contain unique data
  QString sql = QStringLiteral( "SELECT count( distinct (%1) ) ^ count( (%1) ) FROM %2%3" )
                .arg( quotedColNames, mQuery, filterWhereClause() );

  QgsDMResult *unique( connectionRO()->DMexec( sql ) );

  if ( !unique || !unique->execstatus() )
  {
    pushError( unique->getMsg() );
    return false;
  }
  return unique->fetchNext() && ( unique->value( 0 ).toInt() == 0 );
}

// Returns the minimum value of an attribute
QVariant QgsDamengProvider::minimumValue( int index ) const
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

    QgsDMResult *rmin( connectionRO()->DMexec( sql ) );
    rmin->fetchNext();
    return convertValue( fld.type(), fld.subType(), rmin->value( 0 ).toString(), fld.typeName() );
  }
  catch ( DMFieldNotFound )
  {
    return QVariant( QString() );
  }
}

// Returns the maximum value of an attribute
QVariant QgsDamengProvider::maximumValue( int index ) const
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

    QgsDMResult *rmax( connectionRO()->DMexec( sql ) );
    rmax->fetchNext();
    return convertValue( fld.type(), fld.subType(), rmax->value( 0 ).toString(), fld.typeName() );
  }
  catch ( DMFieldNotFound )
  {
    return QVariant( QString() );
  }
}

// Returns the list of unique values of an attribute
QSet<QVariant> QgsDamengProvider::uniqueValues( int index, int limit ) const
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

    QgsDMResult *res( connectionRO()->DMexec( sql ) );
    if ( res && res->execstatus() )
    {
      while ( res->fetchNext() )
        uniqueValues.insert( convertValue( fld.type(), fld.subType(), res->value( 0 ).toString(), fld.typeName() ) );
    }
  }
  catch ( DMFieldNotFound )
  {
  }
  return uniqueValues;
}

QStringList QgsDamengProvider::uniqueStringsMatching( int index, const QString &substring, int limit, QgsFeedback *feedback ) const
{
  QStringList results;

  try
  {
    // get the field name
    QgsField fld = field( index );
    QString sql = QStringLiteral( "SELECT DISTINCT %1 FROM %2 WHERE " )
                  .arg( quotedIdentifier( fld.name() ),
                        mQuery );

    if ( !mSqlWhereClause.isEmpty() )
    {
      sql += QStringLiteral( " ( %1 ) AND " ).arg( mSqlWhereClause );
    }

    sql += QStringLiteral( " UPPER(%1) LIKE UPPER('%%2%')" ).arg( quotedIdentifier( fld.name() ), substring );


    sql += QStringLiteral( " ORDER BY %1" ).arg( quotedIdentifier( fld.name() ) );

    if ( limit >= 0 )
    {
      sql += QStringLiteral( " LIMIT %1" ).arg( limit );
    }

    sql = QStringLiteral( "SELECT %1 FROM (%2) foo" ).arg( connectionRO()->fieldExpression( fld ), sql );

    QgsDMResult *res( connectionRO()->DMexec( sql ) );
    if ( res && res->execstatus() )
    {
      while ( res->fetchNext() )
      {
        results << ( convertValue( fld.type(), fld.subType(), res->value( 0 ).toString(), fld.typeName() ) ).toString();
        if ( feedback && feedback->isCanceled() )
          break;
      }
    }
  }
  catch ( DMFieldNotFound )
  {
  }
  return results;
}

QString QgsDamengProvider::defaultValueClause( int fieldId ) const
{
  QString defVal = mDefaultValues.value( fieldId, QString() );

  if ( mGeneratedValues.contains( fieldId ) )
  {
    return defVal;
  }

  if ( !providerProperty( EvaluateDefaultValues, false ).toBool() && !defVal.isEmpty() )
  {
    return defVal;
  }

  return QString();
}

QVariant QgsDamengProvider::defaultValue( int fieldId ) const
{
  QString defVal = mDefaultValues.value( fieldId, QString() );

  if ( providerProperty( EvaluateDefaultValues, false ).toBool() && !defVal.isEmpty() )
  {
    QgsField fld = field( fieldId );

    QgsDMResult *res( connectionRO()->DMexec( QStringLiteral( "SELECT %1" ).arg( defVal ) ) );
    
    if ( res->fetchNext() )
    {
      return convertValue( fld.type(), fld.subType(), res->value( 0 ).toString(), fld.typeName() );
    }
    else
    {
      pushError( tr( "Could not execute query" ) );
      return QVariant();
    }
  }

  return QVariant();
}

QString QgsDamengProvider::paramValue( const QString &fieldValue, const QString &defaultValue ) const
{
  if ( fieldValue.isNull() )
    return QString();

  if ( fieldValue == defaultValue && !defaultValue.isNull() )
  {
    QgsDMResult *result( connectionRO()->DMexec( QStringLiteral( "SELECT %1" ).arg( defaultValue ) ) );
    if ( !result || !result->execstatus() )
      throw DMException( result->getMsg() );
    result->fetchNext();
    return result->value( 0 ).toString();
  }

  return fieldValue;
}

bool QgsDamengProvider::skipConstraintCheck( int fieldIndex, QgsFieldConstraints::Constraint, const QVariant &value ) const
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


/* private */
bool QgsDamengProvider::getTopoLayerInfo()
{
  QString sql = QString( "SELECT t.name, l.layer_id, l.level, l.feature_type"
                         "FROM SYSTOPOLOGY.SYSLAYER l, SYSTOPOLOGY.SYSTOPOLOGY t "
                         "WHERE l.topology_id = t.id AND l.schema_name=%1 "
                         "AND l.table_name=%2 AND l.feature_column=%3" )
                .arg( quotedValue( mSchemaName ),
                      quotedValue( mTableName ),
                      quotedValue( mGeometryColumn ) );
  QgsDMResult *result( connectionRO()->DMexec( sql ) );

  if ( !result || !result->execstatus() )
  {
    throw DMException( result->getMsg() ); // we should probably not do this
  }
  if ( !result->fetchNext() )
  {
    QgsMessageLog::logMessage( tr( "Could not find topology of layer %1.%2.%3" )
                               .arg( quotedValue( mSchemaName ),
                                     quotedValue( mTableName ),
                                     quotedValue( mGeometryColumn ) ),
                               tr( "Dameng" ) );
    return false;
  }
  mTopoLayerInfo.topologyName = result->value( 0 ).toString();
  mTopoLayerInfo.layerId = result->value( 1 ).toLongLong();
  mTopoLayerInfo.layerLevel = result->value( 2 ).toInt();
  switch ( result->value( 3 ).toInt() )
  {
    case 1:
      mTopoLayerInfo.featureType = TopoLayerInfo::Puntal;
      break;
    case 2:
      mTopoLayerInfo.featureType = TopoLayerInfo::Lineal;
      break;
    case 3:
      mTopoLayerInfo.featureType = TopoLayerInfo::Polygonal;
      break;
    case 4:
    default:
      mTopoLayerInfo.featureType = TopoLayerInfo::Mixed;
      break;
  }
  return true;
}

/* private */
void QgsDamengProvider::dropOrphanedTopoGeoms()
{
  QString sql = QString( "DELETE FROM %1.relation WHERE layer_id = %2 AND "
                         "topogeo_id NOT IN ( SELECT id(%3) FROM %4.%5 )" )
                .arg( quotedIdentifier( mTopoLayerInfo.topologyName ) )
                .arg( mTopoLayerInfo.layerId )
                .arg( quotedIdentifier( mGeometryColumn ),
                      quotedIdentifier( mSchemaName ),
                      quotedIdentifier( mTableName ) )
                ;

  QgsDebugMsgLevel( "TopoGeom orphans cleanup query: " + sql, 2 );

  connectionRW()->DMexecNR( sql );
}


QString QgsDamengProvider::geomParam( int offset ) const
{
  Q_UNUSED( offset )
  QString geometry;

  bool forceMulti = false;

  if ( mSpatialColType != SctTopoGeometry )
  {
    forceMulti = QgsWkbTypes::isMultiType( wkbType() );
  }

  if ( mSpatialColType == SctTopoGeometry )
  {
    geometry += QLatin1String( "SYSTOPOLOGY.DMTOPOLOGY.toTopoGeom(" );
  }

  if ( forceMulti )
  {
    geometry += "DMGEO2.st_multi(";
  }
  
  geometry += mSpatialColType == SctGeography
              ? QStringLiteral( "DMGEO2.ST_GeomToGeog( DMGEO2.st_geomfromwkb(?,%1 ) )" )
                  .arg( mRequestedSrid.isEmpty() ? mDetectedSrid : mRequestedSrid )
              :QStringLiteral( "%1(?,%2 )" ).arg( "DMGEO2.st_geomfromwkb" )
                  .arg( mRequestedSrid.isEmpty() ? mDetectedSrid : mRequestedSrid );

  if ( forceMulti )
  {
    geometry += ')';
  }

  if ( mSpatialColType == SctTopoGeometry )
  {
    geometry += QStringLiteral( ",%1,%2 )" )
                .arg( quotedValue( mTopoLayerInfo.topologyName ) )
                .arg( mTopoLayerInfo.layerId );
  }

  return geometry;
}

void QgsDamengProvider::appendGeomParam( const QgsGeometry &geom, QByteArray &geom_wkb ) const
{
  if ( geom.isNull() )
  {
    geom_wkb = QByteArray();
    return;
  }

  QgsGeometry convertedGeom( convertToProviderType( geom ) );
  QByteArray wkb(!convertedGeom.isNull() ? convertedGeom.asWkb() : geom.asWkb() );

  geom_wkb = std::move( wkb );
}

bool QgsDamengProvider::addFeatures( QgsFeatureList &flist, Flags flags )
{
  if ( flist.isEmpty() )
    return true;

  if ( mIsQuery )
    return false;

  QgsDamengConn *conn = connectionRW();
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
    QString values;
    QString topogeom;
    QString delim;
    int offset = 1;

    QStringList defaultValues;
    QList<int> fieldId;

    if ( !mGeometryColumn.isNull() )
    {
      insert += quotedIdentifier( mGeometryColumn );
      
      values += geomParam( offset++ );
      if ( mSpatialColType == SctTopoGeometry )
      {
        topogeom = values;
        values = "";
      }
      delim = ',';
    }

    bool skipSinglePKField = false;

    if ( ( mPrimaryKeyType == PktInt || mPrimaryKeyType == PktInt64 || mPrimaryKeyType == PktUint64
        || mPrimaryKeyType == PktFidMap || mPrimaryKeyType == PktRowId ) )
    {
      if ( mPrimaryKeyAttrs.size() == 1 )
      {
        bool foundNonEmptyPK = false;
        int idx = mPrimaryKeyAttrs[0];
        QString defaultValue = defaultValueClause( idx );
        for ( int i = 0; i < flist.size(); i++ )
        {
          QgsAttributes attrs2 = flist[i].attributes();
          QVariant v2 = attrs2.value( idx, QVariant( QVariant::Int ) );

          if ( !v2.isNull() && ( v2.toString() != defaultValue || defaultValue == QStringLiteral( "identity_auto" ) ) )
          {
            foundNonEmptyPK = true;
            break;
          }
        }
        skipSinglePKField = !foundNonEmptyPK;
      }

      if ( !skipSinglePKField )
      {
        for ( int idx : mPrimaryKeyAttrs )
        {
          if ( !mGeneratedValues[idx].isEmpty() )
            continue;
          insert += delim + quotedIdentifier( field( idx ).name() );
          values += delim + QStringLiteral( "?" );
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

      if ( !mGeneratedValues.value( idx, QString() ).isEmpty() )
      {
        QgsDebugMsgLevel( QStringLiteral( "Skipping field %1 ( idx %2 ) which is GENERATED." ).arg( fieldname, QString::number( idx ) ), 2 );
        continue;
      }

      QString fieldTypeName = mAttributeFields.at( idx ).typeName();

      QgsDebugMsgLevel( "Checking field against: " + fieldname, 2 );

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
          values += delim + ( defVal.isNull() ? "NULL" : defVal );
        }
        else if ( fieldTypeName == QLatin1String( "geometry" ) )
        {
          values += QStringLiteral( "%1%2(%3)" )
                    .arg( delim,
                          "DMGEO2.st_geomfromewkt",
                          quotedValue( v.toString() ) );
        }
        else if ( fieldTypeName == QLatin1String( "geography" ) )
        {
          values += QStringLiteral( "%1DMGEO2.st_geographyfromewkt(%2)" )
                    .arg( delim,
                          quotedValue( v.toString() ) );
        }
        else if ( fieldTypeName == QLatin1String( "binary" ) ||
                  fieldTypeName == QLatin1String( "varbinary" ) ||
                  fieldTypeName == QLatin1String( "longvarbinary" ) ||
                  fieldTypeName == QLatin1String( "image" ) ||
                  fieldTypeName == QLatin1String( "blob" ) ||
                  fieldTypeName == QLatin1String( "raw" )
              )
        {
          values += delim + ( defVal.isNull() ? quotedByteaValue( v ) : defVal );
        }
        //TODO: convert arrays and hstore to native types
        else
        {
          values += delim + quotedValue( v );
        }
      }
      else
      {
        // value is not unique => add parameter
        if ( fieldTypeName == QLatin1String( "geometry" ) )
        {
          values += QStringLiteral( "%1%2(?)" )
                    .arg( delim,
                          "DMGEO2.st_geomfromewkt" );
        }
        else if ( fieldTypeName == QLatin1String( "geography" ) )
        {
          values += QStringLiteral( "%1DMGEO2.st_geographyfromewkt(?)" )
                    .arg( delim );
        }
        else
        {
          values += QStringLiteral( "%1?" )
                    .arg( delim );
        }
        defaultValues.append( defVal );
        fieldId.append( idx );
      }

      delim = ',';
    }

    insert = mSpatialColType == SctTopoGeometry
              ? QStringLiteral( "declare tg systopology.topogeometry; begin "
                "tg = %3; "
                "%1 ) VALUES( tg%2 ); end; " ).arg( insert ).arg( values ).arg( topogeom )
              : QStringLiteral( "%1 ) VALUES (%2)" ).arg( insert ).arg( values );

    QgsDebugMsgLevel( QStringLiteral( "prepare addfeatures: %1" ).arg( insert ), 2 );
    ExecStatusType ret = conn->DMprepare( insert, fieldId.size() + offset - 1, nullptr );

    if ( ret != DmResCommandOk )
      throw DMException( conn->DMgetResult()->getMsg() );

    for ( QgsFeatureList::iterator features = flist.begin(); features != flist.end(); ++features )
    {
      QgsAttributes attrs = features->attributes();

      QStringList params;
      QByteArray geom_wkb;
      if ( !mGeometryColumn.isNull() )
      {
        appendGeomParam( features->geometry(), geom_wkb );
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
          // the conversion functions expects the list as a string, so convert it
          if ( value.type() == QVariant::StringList )
          {
            QStringList list_vals = value.toStringList();
            
            list_vals.replaceInStrings( "\\", "\\\\" );
            list_vals.replaceInStrings( "\"", "\\\"" );
            v = QStringLiteral( "{\"" ) + value.toStringList().join( QLatin1String( "\",\"" ) ) + QStringLiteral( "\"}" );
          }
          else if ( value.type() == QVariant::List )
            v = "{" + value.toStringList().join( "," ) + "}";
          else if ( value.type() == QVariant::Map )
            v = QgsDamengConn::quotedValue( value ).remove( QRegularExpression( "^'|'$" ) );
          else
          {
            v = paramValue( value.toString(), defaultValues[ i ] );
          }

          if ( v != value.toString() )
          {
            QgsField fld = field( attrIdx );
            features->setAttribute( attrIdx, convertValue( fld.type(), fld.subType(), v, fld.typeName() ) );
          }
        }

        params << v;
      }

      QgsDamengResult result( conn->DMexecPrepared( geom_wkb, params ) );

      if ( !( flags & QgsFeatureSink::FastInsert ) && result.DMresultStatus() == DmResCommandOk )
      {
        for ( int i = 0; i < mPrimaryKeyAttrs.size(); ++i )
        {
          const int idx = mPrimaryKeyAttrs.at( i );
          const QgsField fld = mAttributeFields.at( idx );
          features->setAttribute( idx, convertValue( fld.type(), fld.subType(), result.result()->value( i ).toString(), fld.typeName() ) );
        }
      }
      else if ( result.DMresultStatus() != DmResCommandOk )
        throw DMException( result );

      if ( !( flags & QgsFeatureSink::FastInsert ) )
      {
        // update feature ids
        QVariant Fid;
        QVariantList primaryKeyVals;
        const auto constMPrimaryKeyAttrs = mPrimaryKeyAttrs;

        switch ( mPrimaryKeyType )
        {
        case PktRowId:
          Fid = result.result()->lastInsertId();
          if ( !Fid.isNull() )
          {
            features->setId( mShared->lookupFid( QList<QVariant>() << Fid ) );
          }
          break;
        case PktInt:
          Fid = attrs.at( mPrimaryKeyAttrs.at( 0 ) );
          features->setId( PKINT2FID( STRING_TO_FID( Fid ) ) );
          break;
        case PktInt64:
        case PktUint64:
        case PktFidMap:
          for ( int idx : constMPrimaryKeyAttrs )
          {
            Fid = attrs.at( idx );
            primaryKeyVals << Fid;
          }

          features->setId( mShared->lookupFid( primaryKeyVals ) );
          break;
        default:
          break;
        }

        if ( !Fid.isNull() )
        {
          QgsDebugMsgLevel( QStringLiteral( "new fid=%1" ).arg( features->id() ), 4 );
        }
        
      }
    }

    returnvalue &= conn->commit();
    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();

    mShared->addFeaturesCounted( flist.size() );
  }
  catch ( DMException &e )
  {
    pushError( tr( "Dameng error while adding features: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  conn->unlock();
  return returnvalue;
}

bool QgsDamengProvider::deleteFeatures( const QgsFeatureIds &ids )
{
  if ( ids.isEmpty() )
    return true;

  bool returnvalue = true;

  if ( mIsQuery )
  {
    QgsDebugError( QStringLiteral( "Cannot delete features ( is a query )" ) );
    return false;
  }

  QgsDamengConn *conn = connectionRW();
  if ( !conn )
  {
    return false;
  }
  conn->lock();

  try
  {
    conn->begin();

    QgsFeatureIds chunkIds;
    const int countIds = ids.size();
    int i = 0;
    for ( QgsFeatureIds::const_iterator it = ids.constBegin(); it != ids.constEnd(); ++it )
    {
      // create chunks of fids to delete, the last chunk may be smaller
      chunkIds.insert( *it );
      i++;
      if ( chunkIds.size() < 5000 && i < countIds )
        continue;

      const QString sql = QStringLiteral( "DELETE FROM %1 WHERE %2" )
                          .arg( mQuery, whereClause( chunkIds ) );
      QgsDebugMsgLevel( "delete sql: " + sql, 2 );

      //send DELETE statement and do error handling
      QgsDamengResult result( conn->DMexec( sql ) );
      if ( result.DMresultStatus() != DmResCommandOk && result.DMresultStatus() != DmResSuccessInfo )
        throw DMException( result );

      for ( QgsFeatureIds::const_iterator chunkIt = chunkIds.constBegin(); chunkIt != chunkIds.constEnd(); ++chunkIt )
      {
        mShared->removeFid( *chunkIt );
      }
      chunkIds.clear();
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

    mShared->addFeaturesCounted( -ids.size() );
  }
  catch ( DMException &e )
  {
    pushError( tr( "Dameng error while deleting features: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  conn->unlock();
  return returnvalue;
}

bool QgsDamengProvider::truncate()
{
  bool returnvalue = true;

  if ( mIsQuery )
  {
    QgsDebugError( QStringLiteral( "Cannot truncate ( is a query )" ) );
    return false;
  }

  QgsDamengConn *conn = connectionRW();
  if ( !conn )
  {
    return false;
  }
  conn->lock();

  try
  {
    conn->begin();

    QString sql = QStringLiteral( "TRUNCATE %1" ).arg( mQuery );
    QgsDebugMsgLevel( "truncate sql: " + sql, 2 );

    //send truncate statement and do error handling
    QgsDamengResult result( conn->DMexec( sql ) );
    if ( result.DMresultStatus() != DmResCommandOk && result.DMresultStatus() != DmResSuccessInfo )
      throw DMException( result );

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
  catch ( DMException &e )
  {
    pushError( tr( "Dameng error while truncating: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  conn->unlock();
  return returnvalue;
}

bool QgsDamengProvider::addAttributes( const QList<QgsField> &attributes )
{
  bool returnvalue = true;

  if ( mIsQuery )
    return false;

  if ( attributes.isEmpty() )
    return true;

  QgsDamengConn *conn = connectionRW();
  if ( !conn )
  {
    return false;
  }
  conn->lock();

  try
  {
    conn->begin();

    QString delim;
    QString sql = QStringLiteral( "ALTER TABLE %1 ADD COLUMN (" ).arg( mQuery );
    for ( QList<QgsField>::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter )
    {
      QString type = iter->typeName();
      if ( type == QLatin1String( "char" ) || type == QLatin1String( "character" ) || type == QLatin1String( "varchar" ) )
      {
        if ( iter->length() > 0 )
          type = QStringLiteral( "%1(%2)" ).arg( type ).arg( iter->length() );
      }
      else if ( type == QLatin1String( "number" ) || type == QLatin1String( "numeric" ) || type == QLatin1String( "decimal" ) )
      {
        if ( iter->length() > 0 && iter->precision() > 0 )
          type = QStringLiteral( "%1(%2,%3)" ).arg( type ).arg( iter->length() ).arg( iter->precision() );
      }
      else if ( type == QLatin1String( "binary" ) || type == QLatin1String( "varbinary" ) || type == QLatin1String( "raw" ) )
      {
        if ( iter->length() > 0 )
          type = QStringLiteral( "%1(%2)" ).arg( type ).arg( iter->length() );
      }
      else if ( type == QLatin1String( "float" ) || type == QLatin1String( "double" ) || type == QLatin1String( "double precision" ) )
      {
        if ( iter->length() > 0 )
          type = QStringLiteral( "%1(%2)" ).arg( type ).arg( iter->length() );
      }
      else if ( type == QLatin1String( "time" ) || type == QLatin1String( "datetime" ) || type == QLatin1String( "timestamp" ) )
      {
        if ( iter->length() > 0 )
          type = QStringLiteral( "%1(%2)" ).arg( type ).arg( iter->length() );
      }
      else if ( type.toLower() == QLatin1String( "JSON" ) || type.toLower() == QLatin1String( "json" ) )
      {
          if ( iter->length() > 0 )
              type = QStringLiteral( "varchar(%1)" ).arg( iter->length() );
          sql.append( QStringLiteral( "%1 %2 VARCHAR CHECK (%3 IS JSON( LAX ) )" ).arg( delim, quotedIdentifier( iter->name() ),quotedIdentifier( iter->name() ) ) );
          delim = ',';
          continue;
      }
      else
      {
        type = QStringLiteral( "varchar" );
      }
      sql.append( QStringLiteral( "%1 %2 %3" ).arg( delim, quotedIdentifier( iter->name() ), type ) );
      delim = ',';
    }
    sql += ')';

    //send sql statement and do error handling
    QgsDamengResult result( conn->DMexec( sql ) );
    if ( result.DMresultStatus() != DmResCommandOk )
      throw DMException( result );

    for ( QList<QgsField>::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter )
    {
      if ( !iter->comment().isEmpty() )
      {
        sql = QStringLiteral( "COMMENT ON COLUMN %1.%2 IS %3" )
              .arg( mQuery, quotedIdentifier( iter->name() ), quotedValue( iter->comment() ) );
        result = conn->DMexec( sql );
        if ( result.DMresultStatus() != DmResCommandOk )
          throw DMException( result );
      }
    }

    returnvalue &= conn->commit();
    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();
  }
  catch ( DMException &e )
  {
    pushError( tr( "Dameng error while adding attributes: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  loadFields();
  conn->unlock();
  return returnvalue;
}

bool QgsDamengProvider::deleteAttributes( const QgsAttributeIds &ids )
{
  bool returnvalue = true;

  if ( mIsQuery )
    return false;

  QgsDamengConn *conn = connectionRW();
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
                    .arg( mQuery, quotedIdentifier( column ) );

      //send sql statement and do error handling
      QgsDamengResult result( conn->DMexec( sql ) );
      if ( result.DMresultStatus() != DmResCommandOk )
        throw DMException( result );

      //delete the attribute from mAttributeFields
      mAttributeFields.remove( index );
    }

    returnvalue &= conn->commit();
    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();
  }
  catch ( DMException &e )
  {
    pushError( tr( "Dameng error while deleting attributes: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  loadFields();
  conn->unlock();
  return returnvalue;
}

bool QgsDamengProvider::renameAttributes( const QgsFieldNameMap &renamedAttributes )
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

  QgsDamengConn *conn = connectionRW();
  if ( !conn )
  {
    return false;
  }
  conn->lock();

  try
  {
    conn->begin();
    //send sql statement and do error handling
    QgsDamengResult result( conn->DMexec( sql ) );
    if ( result.DMresultStatus() != DmResCommandOk )
      throw DMException( result );
    returnvalue = conn->commit();
    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();
  }
  catch ( DMException &e )
  {
    pushError( tr( "Dameng error while renaming attributes: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  loadFields();
  conn->unlock();
  return returnvalue;
}

bool QgsDamengProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  bool returnvalue = true;

  if ( mIsQuery )
    return false;

  if ( attr_map.isEmpty() )
    return true;

  QgsDamengConn *conn = connectionRW();
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
      int numChangedFields = 0;
      for ( QgsAttributeMap::const_iterator siter = attrs.constBegin(); siter != attrs.constEnd(); ++siter )
      {
        try
        {
          QgsField fld = field( siter.key() );

          pkChanged = pkChanged || mPrimaryKeyAttrs.contains( siter.key() );

          if ( mGeneratedValues.contains( siter.key() ) )
          {
            QgsLogger::warning( tr( "Changing the value of GENERATED field %1 is not allowed." ).arg( fld.name() ) );
            continue;
          }

          numChangedFields++;

          sql += delim + QStringLiteral( "%1=" ).arg( quotedIdentifier( fld.name() ) );
          delim = ',';

          QString defVal = defaultValueClause( siter.key() );
          if ( qgsVariantEqual( *siter, defVal ) )
          {
            sql += defVal.isNull() ? "NULL" : defVal;
          }
          else if ( fld.typeName() == QLatin1String( "geometry" ) )
          {
            sql += QStringLiteral( "%1(%2)" )
                   .arg( "DMGEO2.st_geomfromewkt",
                         quotedValue( siter->toString() ) );
          }
          else if ( fld.typeName() == QLatin1String( "geography" ) )
          {
            sql += QStringLiteral( "DMGEO2.st_geographyfromewkt(%1)" )
                   .arg( quotedValue( siter->toString() ) );
          }
          else if ( fld.typeName() == QLatin1String( "byte" ) )
          {
            sql += quotedByteaValue( siter.value() );
          }
          else
          {
            sql += quotedValue( *siter );
          }
        }
        catch ( DMFieldNotFound )
        {
          // Field was missing - shouldn't happen
        }
      }

      sql += QStringLiteral( " WHERE %1" ).arg( whereClause( fid ) );

      // Don't try to UPDATE an empty set of values ( might happen if the table only has GENERATED fields,
      // or if the user only changed GENERATED fields in the form/attribute table.
      if ( numChangedFields > 0 )
      {
        QgsDamengResult result( conn->DMexec( sql ) );
        if ( result.DMresultStatus() != DmResCommandOk && result.DMresultStatus() != DmResSuccessInfo )
          throw DMException( result );
      }
      else // let the user know that no field was actually changed
      {
        QgsLogger::warning( tr( "No fields were updated on the database." ) );
      }

      // update feature id map if key was changed
      // PktInt64 also uses a fid map even if it is a stand alone field.
      if ( pkChanged && ( mPrimaryKeyType == PktFidMap || mPrimaryKeyType == PktInt64 ) )
      {
        QVariantList k = mShared->removeFid( fid );

        int keyCount = std::min( mPrimaryKeyAttrs.size(), k.size() );

        for ( int i = 0; i < keyCount; i++ )
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
  catch ( DMException &e )
  {
    pushError( tr( "Dameng error while changing attributes: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  conn->unlock();
  return returnvalue;
}

bool QgsDamengProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  if ( mIsQuery || mGeometryColumn.isNull() )
    return false;

  QgsDamengConn *conn = connectionRW();
  if ( !conn )
  {
    return false;
  }
  conn->lock();

  bool returnvalue = true;

  try
  {
    // Start the Dameng transaction
    conn->begin();

    QString update;
    QgsDamengResult result;

    if ( mSpatialColType == SctTopoGeometry )
    {
      // We will create a new TopoGeometry object with the new shape.
      // Later, we'll replace the old TopoGeometry with the new one,
      // to avoid orphans and retain higher level in an eventual
      // hierarchical definition
      update = QStringLiteral( "SELECT id(%1) FROM %2 o WHERE %3" )
               .arg( geomParam( 1 ), mQuery, pkParamWhereClause( 2 ) );

      QString getid = QStringLiteral( "SELECT id(%1) FROM %2 WHERE %3" )
                      .arg( quotedIdentifier( mGeometryColumn ), mQuery, pkParamWhereClause( 1 ) );

      QgsDebugMsgLevel( "getting old topogeometry id: " + getid, 2 );

      ExecStatusType ret = connectionRO()->DMprepare( getid, 1, nullptr );
      if ( ret != DmResCommandOk )
      {
        QgsDebugError( QStringLiteral( "Exception thrown due to DMprepare of this query returning != DMRES_COMMAND_OK (%1 != expected %2 ): %3" )
                     .arg( connectionRO()->DMgetResult()->getMsg() ).arg( DmResCommandOk ).arg( getid ) );
        throw DMException( result );
      }

      QString replace = QString( "UPDATE %1 SET %2="
                                 "( topology_id(%2),layer_id(%2),$1,type(%2) )"
                                 "WHERE %3" )
                        .arg( mQuery, quotedIdentifier( mGeometryColumn ), pkParamWhereClause( 2 ) );
      QgsDebugMsgLevel( "TopoGeom swap: " + replace, 2 );
      ret = conn->DMprepare( replace, 2, nullptr );
      if ( ret != DmResCommandOk )
      {
        QgsDebugError( QStringLiteral( "Exception thrown due to DMprepare of this query returning != DMRES_COMMAND_OK (%1 != expected %2 ): %3" )
                     .arg( connectionRO()->DMgetResult()->getMsg() ).arg( DmResCommandOk ).arg( replace ) );
        throw DMException( result );
      }

    }
    else
    {
      update = QStringLiteral( "UPDATE %1 SET %2=%3 WHERE %4" )
               .arg( mQuery, quotedIdentifier( mGeometryColumn ), geomParam( 1 ), pkParamWhereClause( 2 ) );
    }

    QgsDebugMsgLevel( "updating: " + update, 2 );

    ExecStatusType ret = conn->DMprepare( update, 2, nullptr );
    if ( ret != DmResCommandOk )
    {
      QgsDebugError( QStringLiteral( "Exception thrown due to DMprepare of this query returning != DMRES_COMMAND_OK (%1 != expected %2 ): %3" )
                   .arg( conn->DMgetResult()->getMsg() ).arg( DmResCommandOk ).arg( update ) );
      throw DMException( result );
    }

    QgsDebugMsgLevel( QStringLiteral( "iterating over the map of changed geometries..." ), 2 );

    for ( QgsGeometryMap::const_iterator iter = geometry_map.constBegin();
          iter != geometry_map.constEnd();
          ++iter )
    {
      QgsDebugMsgLevel( "iterating over feature id " + FID_TO_STRING( iter.key() ), 2 );

      // Save the id of the current topogeometry
      long long old_tg_id = -1;
      if ( mSpatialColType == SctTopoGeometry )
      {
        QStringList params;
        QByteArray geom_wkb;
        appendGeomParam(*iter, geom_wkb );
        appendPkParams( iter.key(), params );
        result = connectionRO()->DMexecPrepared( geom_wkb, params );
        if ( result.DMresultStatus() != DmResCommandOk && result.DMresultStatus() != DmResSuccessInfo )
        {
          QgsDebugError( QStringLiteral( "Exception thrown due to DMexecPrepared of 'getid' returning != DMRES_SUCCESS_INFO (%1 != expected %2 )" )
                       .arg( result.DMresultStatus() ).arg( DmResCommandOk ) );
          throw DMException( result );
        }
        // TODO: watch out for NULL, handle somehow
        old_tg_id = result.result()->value( 0 ).toLongLong();
        QgsDebugMsgLevel( QStringLiteral( "Old TG id is %1" ).arg( old_tg_id ), 2 );
      }

      QStringList params;
      QByteArray geom_wkb;
      appendGeomParam( *iter, geom_wkb );
      appendPkParams( iter.key(), params );

      result = conn->DMexecPrepared( geom_wkb, params );
      if ( result.DMresultStatus() != DmResCommandOk && result.DMresultStatus() != DmResSuccessInfo )
        throw DMException( result );

      if ( mSpatialColType == SctTopoGeometry )
      {
        long long new_tg_id = result.result()->value( 0 ).toLongLong(); // new topogeo_id
        
        // Replace old TopoGeom with new TopoGeom, so that
        // any hierarchically defined TopoGeom will still have its
        // definition and we'll leave no orphans
        QString replace = QString( "DELETE FROM %1.relation WHERE "
                                   "layer_id = %2 AND topogeo_id = %3" )
                          .arg( quotedIdentifier( mTopoLayerInfo.topologyName ) )
                          .arg( mTopoLayerInfo.layerId )
                          .arg( old_tg_id );
        result = conn->DMexec( replace );
        if ( result.DMresultStatus() != DmResCommandOk )
        {
          QgsDebugError( QStringLiteral( "Exception thrown due to DMexec of this query returning != DMRES_COMMAND_OK (%1 != expected %2 ): %3" )
                       .arg( result.DMresultStatus() ).arg( DmResCommandOk ).arg( replace ) );
          throw DMException( result );
        }
        // TODO: use prepared query here
        replace = QString( "UPDATE %1.relation SET topogeo_id = %2 "
                           "WHERE layer_id = %3 AND topogeo_id = %4" )
                  .arg( quotedIdentifier( mTopoLayerInfo.topologyName ) )
                  .arg( old_tg_id )
                  .arg( mTopoLayerInfo.layerId )
                  .arg( new_tg_id );
        QgsDebugMsgLevel( "relation swap: " + replace, 2 );
        result = conn->DMexec( replace );
        if ( result.DMresultStatus() != DmResCommandOk )
        {
          QgsDebugError( QStringLiteral( "Exception thrown due to DMexec of this query returning != DMRES_COMMAND_OK (%1 != expected %2 ): %3" )
                       .arg( result.DMresultStatus() ).arg( DmResCommandOk ).arg( replace ) );
          throw DMException( result );
        }
      } // if TopoGeometry

    } // for each feature

    returnvalue &= conn->commit();
    if ( mTransaction )
      mTransaction->dirtyLastSavePoint();
  }
  catch ( DMException &e )
  {
    pushError( tr( "Dameng error while changing geometry values: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  conn->unlock();

  QgsDebugMsgLevel( QStringLiteral( "leaving." ), 4 );

  return returnvalue;
}

bool QgsDamengProvider::changeFeatures( const QgsChangedAttributesMap &attr_map, const QgsGeometryMap &geometry_map )
{
  Q_ASSERT( mSpatialColType != SctTopoGeometry );

  bool returnvalue = true;

  if ( mIsQuery )
    return false;

  if ( attr_map.isEmpty() )
    return true;

  QgsDamengConn *conn = connectionRW();
  if ( !conn )
    return false;

  conn->lock();

  try
  {
    conn->begin();

    QgsFeatureIds ids( qgis::listToSet( attr_map.keys() ) );
    ids |= qgis::listToSet( geometry_map.keys() );

    // cycle through the features
    const auto constIds = ids;
    for ( QgsFeatureId fid : constIds )
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
      int numChangedFields = 0;

      for ( QgsAttributeMap::const_iterator siter = attrs.constBegin(); siter != attrs.constEnd(); ++siter )
      {
        try
        {
          QgsField fld = field( siter.key() );

          pkChanged = pkChanged || mPrimaryKeyAttrs.contains( siter.key() );

          if ( mGeneratedValues.contains( siter.key() ) )
          {
            QgsLogger::warning( tr( "Changing the value of GENERATED field %1 is not allowed." ).arg( fld.name() ) );
            continue;
          }

          numChangedFields++;

          sql += delim + QStringLiteral( "%1=" ).arg( quotedIdentifier( fld.name() ) );
          delim = ',';

          if ( fld.typeName() == QLatin1String( "geometry" ) )
          {
            sql += QStringLiteral( "%1(%2)" )
                   .arg( "DMGEO2.st_geomfromewkt",
                         quotedValue( siter->toString() ) );
          }
          else if ( fld.typeName() == QLatin1String( "geography" ) )
          {
            sql += QStringLiteral( "DMGEO2.st_geographyfromewkt(%1)" )
                   .arg( quotedValue( siter->toString() ) );
          }
          else
          {
            sql += quotedValue( *siter );
          }
        }
        catch ( DMFieldNotFound )
        {
          // Field was missing - shouldn't happen
        }
      }

      if ( !geometry_map.contains( fid ) )
      {
        // Don't try to UPDATE an empty set of values ( might happen if the table only has GENERATED fields,
        // or if the user only changed GENERATED fields in the form/attribute table.
        if ( numChangedFields > 0 )
        {
          sql += QStringLiteral( " WHERE %1" ).arg( whereClause( fid ) );

          QgsDamengResult result( conn->DMexec( sql ) );
          if ( result.DMresultStatus() != DmResCommandOk && result.DMresultStatus() != DmResSuccessInfo )
            throw DMException( result );
        }
        else // let the user know that nothing has actually changed
        {
          QgsLogger::warning( tr( "No fields/geometries were updated on the database." ) );
        }
      }
      else
      {
        sql += QStringLiteral( "%1%2=%3" ).arg( delim, quotedIdentifier( mGeometryColumn ), geomParam( 1 ) );
        sql += QStringLiteral( " WHERE %1" ).arg( whereClause( fid ) );

        ExecStatusType ret = conn->DMprepare( sql, 1, nullptr );
        if ( ret != DmResCommandOk )
        {
          QgsDebugError( QStringLiteral( "Exception thrown due to DMprepare of this query returning != DMRES_COMMAND_OK (%1 != expected %2 ): %3" )
                       .arg( conn->DMgetResult()->getMsg() ).arg( DmResCommandOk ).arg( sql ) );
          throw DMException( conn->DMgetResult()->getMsg() );
        }

        QStringList params;
        QByteArray geom_wkb;
        const QgsGeometry &geom = geometry_map[ fid ];
        appendGeomParam( geom, geom_wkb );

        QgsDMResult *result = conn->DMexecPrepared( geom_wkb, params );
        if ( result && result->execstatus() )
        {
          conn->rollback();
          throw DMException( result->getMsg() );
        }
      }

      // update feature id map if key was changed
      // PktInt64 also uses a fid map even though it is a single field.
      if ( pkChanged && ( mPrimaryKeyType == PktFidMap || mPrimaryKeyType == PktInt64 ) )
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
  catch ( DMException &e )
  {
    pushError( tr( "Dameng error while changing attributes: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  conn->unlock();

  QgsDebugMsgLevel( QStringLiteral( "leaving." ), 4 );

  return returnvalue;
}


Qgis::VectorProviderCapabilities QgsDamengProvider::capabilities() const
{
  return mEnabledCapabilities;
}

QgsAttributeList QgsDamengProvider::attributeIndexes() const
{
  QgsAttributeList lst;
  lst.reserve( mAttributeFields.count() );
  for ( int i = 0; i < mAttributeFields.count(); ++i )
    lst.append( i );
  return lst;
}

Qgis::SpatialIndexPresence QgsDamengProvider::hasSpatialIndex() const
{
  QgsDamengProviderConnection conn( mUri.uri(), QVariantMap() );
  try
  {
    return conn.spatialIndexExists( mUri.schema(), mUri.table(), mUri.geometryColumn() ) ? Qgis::SpatialIndexPresence::Present : Qgis::SpatialIndexPresence::NotPresent;
  }
  catch ( QgsProviderConnectionException & )
  {
    return Qgis::SpatialIndexPresence::Unknown;
  }
}

bool QgsDamengProvider::setSubsetString( const QString &theSQL, bool updateFeatureCount )
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

  QgsDamengResult res( connectionRO()->DMexec( sql ) );
  if ( res.DMresultStatus() != DmResCommandOk )
  {
    pushError( res.DMresultErrorMessage() );
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
    reloadData();
  }
  else
  {
    mLayerExtent.reset();
    emit dataChanged();
  }

  return true;
}

/**
 * Returns the feature count
 */
long long QgsDamengProvider::featureCount() const
{
  long long featuresCounted = mShared->featuresCounted();
  if ( featuresCounted >= 0 )
    return featuresCounted;

  if ( ! connectionRO() )
    return 0;

  // get total number of features
  QString sql;

  // use estimated metadata even when there is a where clause,
  // although we get an incorrect feature count for the subset
  // - but make huge dataset usable.
  long long num = -1;

  if ( !mIsQuery && mUseEstimatedMetadata )
  {
    sql = QStringLiteral( "select num_rows from SYS.ALL_TABLES where owner = %1 and table_name = %2;" )
                        .arg( quotedValue( mSchemaName ) ).arg( quotedValue( mTableName ) );
    QgsDMResult *result( connectionRO()->DMexec( sql ) );
    if ( result->fetchNext() )
    {
      num = result->value( 0 ).toLongLong();
      if ( num == 0 )
        num = -1;
    }
  }
  else
  {
    sql = QStringLiteral( "SELECT count(*) FROM %1%2" ).arg( mQuery, filterWhereClause() );
    QgsDMResult *result( connectionRO()->DMexec( sql ) );
    if ( result->fetchNext() )
    {
      num = result->value( 0 ).toLongLong();
    }
  }

  
  mShared->setFeaturesCounted( num );
  
  QgsDebugMsgLevel( "number of features: " + QString::number( num ), 2 );

  return num;
}

bool QgsDamengProvider::empty() const
{
  QString sql = QStringLiteral( "SELECT count(*) FROM %1%2 LIMIT 1;" ).arg( mQuery, filterWhereClause() );
  QgsDMResult *res( connectionRO()->DMexec( sql ) );
  res->fetchNext();
  if ( !res || !res->execstatus() )
  {
    pushError( res->getMsg() );
    return false;
  }

  return res->value( 0 ).toInt() != 1;
}

void QgsDamengProvider::setExtent( const QgsRectangle &newExtent )
{
  mLayerExtent.emplace( newExtent );
}

QgsRectangle QgsDamengProvider::extent() const
{
  return extent3D().toRectangle();
}

QgsBox3D QgsDamengProvider::extent3D() const
{
  if ( !isValid() || mGeometryColumn.isNull() )
    return QgsRectangle();
  
  if ( mLayerExtent.has_value() )
    return *mLayerExtent;

  // Return the estimated extents, if requested and possible
  if ( mUseEstimatedMetadata )
    estimateExtent();
  
  // Compute the extents, if estimation failed or was disabled
  if ( !mLayerExtent.has_value() )
    computeExtent3D();
  
  if ( mLayerExtent.has_value() )
  {
    return *mLayerExtent;
  }
  else
  {
    pushError( tr( "Could not extract layer extent" ) );
    return QgsBox3D();
  }
}

bool QgsDamengProvider::estimateExtent() const
{
  QString sql;
  QgsDMResult *result;

  sql = QStringLiteral( "select %1(%2,%3,%4).ST_AsText() " )
        .arg(
              "DMGEO2.st_estimatedextent",
              quotedValue( mSchemaName ),
              quotedValue( mTableName ),
              quotedValue( mGeometryColumn )
        );

  result = connectionRO()->DMexec( sql );

  if ( !result || !result->execstatus() )
  {
    pushError( result->getMsg() );
    return false;
  }

  if ( !result->fetchNext() )
  {
    pushError( tr( "Unexpected number of tuples from estimated extent query %1: %2 (1 expected)." )
                  .arg( sql )
                  .arg( 0 ) );
    return false;
  }

  if ( result->value( 0 ).isNull() )
    return false;

  QString box2dString = result->value( 0 ).toString();

  QgsDebugMsgLevel( QStringLiteral( "Got extents extent (%1) using: %2" ).arg( box2dString ).arg( sql ), 2 );

  const QRegularExpression rx2d( "\\((.+) (.+),(.+) (.+)\\)" );
  const QRegularExpressionMatch match = rx2d.match( box2dString );
  if ( !match.hasMatch() )
  {
    QgsMessageLog::logMessage( tr( "result of extents query invalid: %1" ).arg( box2dString ), tr( "Dameng" ) );
  }
  mLayerExtent.emplace(
    match.captured( 1 ).toDouble(),             // xmin
    match.captured( 2 ).toDouble(),             // ymin
    std::numeric_limits<double>::quiet_NaN(), // zmin
    match.captured( 3 ).toDouble(),             // xmax
    match.captured( 4 ).toDouble(),             // ymax
    std::numeric_limits<double>::quiet_NaN()  // zmax
  );

  QgsDebugMsgLevel( "Set extents to estimated value: " + mLayerExtent.toString(), 2 );
  return true;
}

bool QgsDamengProvider::computeExtent3D() const
{
  QString sql = QStringLiteral( "SELECT box.st_astext() from( SELECT DMGEO2.box3d(DMGEO2.ST_GeomFromGserialized(%1(%2%3))) as box FROM %4%5);" )
                  .arg( "ST_3DExtent", quotedIdentifier( mBoundingBoxColumn ),
                    mSpatialColType == SctTopoGeometry
                    ? QStringLiteral( "SYSTOPOLOGY.DMTOPOLOGY.Geometry(%1)" ).arg( quotedIdentifier( mBoundingBoxColumn ) )
                    : "",
                    mQuery, filterWhereClause() );

  QgsDMResult* result = connectionRO()->DMexec( sql );
  
  if ( !result || !result->execstatus() )
  {
    pushError( result->getMsg() );
    return false;
  }

  if ( !result->fetchNext() )
  {
    pushError( tr( "Unexpected number of tuples from compute extent query %1: %2 (1 expected)." )
                 .arg( sql )
                 .arg( 0 ) );
    return false;
  }

  if ( result->value( 0 ).isNull() )
  {
    // Layer is empty, set layerExtent to null (default-construct)
    QgsDebugMsgLevel( QStringLiteral( "Got null from extent aggregate, setting layer extent to null as well" ), 2 );
    mLayerExtent.emplace(); // constructs a NULL
    return true;
  }

  QString ext = result->value( 0 ).toString();

  if ( ext.isEmpty() )
  {
    pushError( tr( "Unexpected empty result from extent query %1." ).arg( sql ) );
    return false;
  }

  QgsDebugMsgLevel( QStringLiteral( "Got extents (%1) using: %2" ).arg( ext ).arg( sql ), 2 );

  // Try the BOX3D format
  const thread_local QRegularExpression rx3d( "\\((.+) (.+) (.+),(.+) (.+) (.+)\\)" );
  QRegularExpressionMatch match = rx3d.match( ext );
  if ( match.hasMatch() )
  {
    mLayerExtent.emplace(
      match.captured( 1 ).toDouble(), // xmin
      match.captured( 2 ).toDouble(), // ymin
      match.captured( 3 ).toDouble(), // zmin
      match.captured( 4 ).toDouble(), // xmax
      match.captured( 5 ).toDouble(), // ymax
      match.captured( 6 ).toDouble()  // zmax
    );
    QgsDebugMsgLevel( "Set extents to computed 3D value: " + mLayerExtent->toString(), 2 );
    if ( !elevationProperties()->containsElevationData() )
    {
      // TODO: add a QgsBox3D::force2D method
      mLayerExtent->setZMinimum( std::numeric_limits<double>::quiet_NaN() );
      mLayerExtent->setZMaximum( std::numeric_limits<double>::quiet_NaN() );
      QgsDebugMsgLevel( "Removed Z from extent as layer is configured to not have elevation properties", 2 );
    }
    return true;
  }

  // Try the BOX2D format
  const thread_local QRegularExpression rx2d( "\\((.+) (.+),(.+) (.+)\\)" );
  match = rx2d.match( ext );
  if ( match.hasMatch() )
  {
    mLayerExtent.emplace(
      match.captured( 1 ).toDouble(),           // xmin
      match.captured( 2 ).toDouble(),           // ymin
      std::numeric_limits<double>::quiet_NaN(), // zmin
      match.captured( 3 ).toDouble(),           // xmax
      match.captured( 4 ).toDouble(),           // ymax
      std::numeric_limits<double>::quiet_NaN()  // zmax
    );
    QgsDebugMsgLevel( "Set extents to computed 2D value: " + mLayerExtent->toString(), 2 );
    return true;
  }

  QgsMessageLog::logMessage( tr( "Unexpected result from extent query %1: %2" ).arg( sql, ext ), tr( "Dameng" ) );
  return false;
}

void QgsDamengProvider::updateExtents()
{
  mLayerExtent.reset();
}

bool QgsDamengProvider::getGeometryDetails()
{
  if ( mGeometryColumn.isNull() )
  {
    mDetectedGeomType = Qgis::WkbType::NoGeometry;
    mValid = true;
    return true;
  }

  QgsDMResult *result;
  QString sql;

  QString schemaName = mSchemaName;
  QString tableName = mTableName;
  QString geomCol = mGeometryColumn;
  QString geomColType;

  // Trust the datasource config means that we used requested geometry type and srid
  // We only need to get the spatial column type
  if ( ( mReadFlags & Qgis::DataProviderReadFlag::TrustDataSource ) &&
       mRequestedGeomType != Qgis::WkbType::Unknown &&
       !mRequestedSrid.isEmpty() )
  {
    if ( mIsQuery )
    {
      sql = QStringLiteral( "SELECT %1 FROM %2 WHERE 1=0" ).arg( quotedIdentifier( mGeometryColumn ) ).arg( mQuery );
      result = connectionRO()->DMexec( sql );
      if ( !result || !result->execstatus() )
      {
        QgsMessageLog::logMessage( tr( "Could not execute query.\nThe error message from the database was:\n%1.\nSQL: %2" )
          .arg( result->getMsg() ).arg( sql ), tr( "Dameng" ) );
        mValid = false;
        return false;
      }

      schemaName = "";
      tableName = mQuery;
    }
    else
    {
      sql = QStringLiteral(
              "select TYPTYPE from "
                  "( select F_TABLE_SCHEMA, F_TABLE_NAME, F_GEOMETRY_COLUMN, 1 typtype "
                      "from SYSGEO2.GEOMETRY_COLUMNS union "
                  "select F_TABLE_SCHEMA, F_TABLE_NAME, F_GEOGRAPHY_COLUMN, 2 typtype "
                      "from SYSGEO2.GEOGRAPHY_COLUMNS union "
                  "select SCHEMA_NAME F_TABLE_SCHEMA, TABLE_NAME F_TABLE_NAME, FEATURE_COLUMN F_GEOMETRY_COLUMN, 3 typtype "
                      "from SYSTOPOLOGY.SYSLAYER ) "
              " where F_TABLE_SCHEMA = %1 and F_TABLE_NAME = %2 "
              " and F_GEOMETRY_COLUMN = %3; "
            ).arg(  quotedValue( schemaName ),
                    quotedValue( tableName ),
                    quotedValue( geomCol ) );
    }
    QgsDebugMsgLevel( QStringLiteral( "Getting the spatial column type: %1" ).arg( sql ), 2 );

    result = connectionRO()->DMexec( sql );
    result->fetchNext();
    if ( result && result->execstatus() )
    {
      int geomColType2 = result->value( 0 ).toInt();

      // Get spatial col type
      if ( geomColType2 == 1 )
        mSpatialColType = SctGeometry;
      else if ( geomColType2 == 2 )
        mSpatialColType = SctGeography;
      else if ( geomColType == 3 )
        mSpatialColType = SctTopoGeometry;
      else
        mSpatialColType = SctNone;

      // Use requested geometry type and srid
      mDetectedGeomType = mRequestedGeomType;
      mDetectedSrid     = mRequestedSrid;
      mValid = true;
      return true;
    }
    else
    {
      mValid = false;
      return false;
    }
  }

  if ( mIsQuery )
  {
    udint4 tableoid;
    if ( !mSchemaName.isEmpty() || !mTableName.isEmpty() )
    {
      sql = QStringLiteral( "SELECT %1 FROM %2 LIMIT 0" ).arg( quotedIdentifier( mGeometryColumn ), mQuery );
      QgsDMResult *result( connectionRO()->DMexec( sql ) );
      if ( !result || !result->execstatus() )
      {
        mValid = false;
        return false;
      }
      tableoid = result->ftable( mSchemaName, mTableName );
    }
    else
    {
      sql = mQuery;
      result = connectionRO()->DMexec( sql );
      if ( result && result->execstatus() )
      {
        int i;
        int cols_num = result->nfields();
        for ( i = 0; i < cols_num; ++i )
        {
          if ( result->fname( i ) == geomCol )
            break;
        }
        tableoid = result->ftable( i );
      }
      else
      {
        mValid = false;
        return false;
      }
    }

    QgsDebugMsgLevel( QStringLiteral( "Getting geometry column: %1" ).arg( sql ), 2 );
    if ( tableoid > 0 )
    {
      sql = QStringLiteral( "SELECT A.NAME SCHEMA_NAME,B.NAME TABLE_NAME from sysobjects B "
                            " LEFT OUTER JOIN ( select ID, name from SYS.SYSOBJECTS ) A "
                            " on A.ID = B.SCHID where B.ID = %1;" ).arg( tableoid );
      result = connectionRO()->DMexec( sql );

      if ( result && result->execstatus() && result->fetchNext() )
      {
        schemaName = result->value( 0 ).toString();
        tableName = result->value( 1 ).toString();

        sql = QStringLiteral( "select F_GEOMETRY_COLUMN,TYPTYPE from "
                                "( select F_TABLE_SCHEMA, F_TABLE_NAME, F_GEOMETRY_COLUMN, 1 typtype "
                                    "from SYSGEO2.GEOMETRY_COLUMNS union "
                                "select F_TABLE_SCHEMA, F_TABLE_NAME, F_GEOGRAPHY_COLUMN, 2 typtype "
                                    "from SYSGEO2.GEOGRAPHY_COLUMNS union "
                                "select SCHEMA_NAME F_TABLE_SCHEMA, TABLE_NAME F_TABLE_NAME, FEATURE_COLUMN F_GEOMETRY_COLUMN, 3 typtype "
                                    "from SYSTOPOLOGY.SYSLAYER ) "
                              "where F_TABLE_SCHEMA = %1 and F_TABLE_NAME = %2; "
                            ).arg( quotedValue( schemaName ) ).arg( quotedValue( tableName ) );
        result = connectionRO()->DMexec( sql );
        if ( result && result->execstatus() && result->fetchNext() )
        {
          geomCol = result->value( 0 ).toString();
          int geomColType2 = result->value( 1 ).toInt();
          if ( geomColType2 == 1 )
            mSpatialColType = SctGeometry;
          else if ( geomColType2 == 2 )
            mSpatialColType = SctGeography;
          else if ( geomColType2 == 3 )
            mSpatialColType = SctTopoGeometry;
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

  QString detectedType;
  QString detectedSrid;
  if ( !schemaName.isEmpty() )
  {
    // check geometry columns
    sql = QStringLiteral( "SELECT upper( type ),srid,coord_dimension FROM SYSGEO2.geometry_columns "
                        "WHERE F_TABLE_NAME = %1 AND F_GEOMETRY_COLUMN = %2 AND F_TABLE_SCHEMA = %3;" )
          .arg( quotedValue( tableName ),
                quotedValue( geomCol ),
                quotedValue( schemaName ) );

    QgsDebugMsgLevel( QStringLiteral( "Getting geometry column: %1" ).arg( sql ), 2 );
    result = connectionRO()->DMexec( sql, true, true );
    QgsDebugMsgLevel( QStringLiteral( "Geometry column query returned %1 rows" ).arg( result.DMntuples() ), 2 );

    if ( result->fetchNext() )
    {
      detectedType = result->value( 0 ).toString();

      // Do not override the SRID if set in the data source URI
      if ( detectedSrid.isEmpty() )
      {
        detectedSrid = result->value( 1 ).toString();
      }

      QString dim = result->value( 2 ).toString();
      if ( dim == QLatin1String( "3" ) && !detectedType.endsWith( 'M' ) )
        detectedType += QLatin1Char( 'Z' );
      else if ( dim == QLatin1String( "4" ) )
        detectedType += QLatin1String( "ZM" );

      QString ds = result->value( 1 ).toString();
      if ( ds != QLatin1String( "0" ) ) detectedSrid = ds;
      mSpatialColType = SctGeometry;
    }
    else
    {
      connectionRO()->DMexecNR( QStringLiteral( "COMMIT" ) );
    }

    if ( detectedType.isEmpty() )
    {
      // check geography columns
      sql = QStringLiteral( "SELECT upper( type ),srid FROM SYSGEO2.geography_columns WHERE "
                            " f_table_name = %1 AND f_geography_column = %2 AND f_table_schema = %3;" )
            .arg( quotedValue( tableName ),
                  quotedValue( geomCol ),
                  quotedValue( schemaName ) );

      QgsDebugMsgLevel( QStringLiteral( "Getting geography column: %1" ).arg( sql ), 2 );
      result = connectionRO()->DMexec( sql, false, true );
      QgsDebugMsgLevel( QStringLiteral( "Geography column query returned %1" ).arg( result.DMntuples() ), 2 );

      if ( result->fetchNext() )
      {
        QString dt = result->value( 0 ).toString();
        if ( dt != "GEOMETRY" ) detectedType = dt;
        QString ds = result->value( 1 ).toString();
        if ( ds != "0" ) detectedSrid = ds;
        mSpatialColType = SctGeography;
      }
      else
      {
        connectionRO()->DMexecNR( QStringLiteral( "COMMIT" ) );
      }
    }

    if ( detectedType.isEmpty() && connectionRO()->hasTopology() )
    {
      // check topology.layer
      sql = QString( "SELECT CASE "
                     "WHEN l.feature_type = 1 THEN 'ST_MULTIPOINT' "
                     "WHEN l.feature_type = 2 THEN 'ST_MULTILINESTRING' "
                     "WHEN l.feature_type = 3 THEN 'ST_MULTIPOLYGON' "
                     "WHEN l.feature_type = 4 THEN 'ST_GEOMETRYCOLLECTION' "
                     "END AS type, t.srid FROM SYSTOPOLOGY.SYSLAYER l, SYSTOPOLOGY.SYSTOPOLOGY t "
                     "WHERE l.topology_id = t.id AND l.schema_name=%3 "
                     "AND l.table_name=%1 AND l.feature_column=%2" )
            .arg( quotedValue( tableName ),
                  quotedValue( geomCol ),
                  quotedValue( schemaName ) );

      QgsDebugMsgLevel( QStringLiteral( "Getting TopoGeometry column: %1" ).arg( sql ), 2 );
      result = connectionRO()->DMexec( sql, false, true );
      QgsDebugMsgLevel( QStringLiteral( "TopoGeometry column query returned %1" ).arg( result.DMntuples() ), 2 );

      if ( result->fetchNext() )
      {
        detectedType = result->value( 0 ).toString();
        detectedSrid = result->value( 1 ).toString();
        mSpatialColType = SctTopoGeometry;
      }
      else
      {
        connectionRO()->DMexecNR( QStringLiteral( "COMMIT" ) );
      }
    }

    if ( mSpatialColType == SctNone )
    {
      sql = QString( "select OWNER,TYPE_NAME,TYPE_OID,SUPERTYPE_NAME from SYS.ALL_TYPES where TYPE_OID = "
                         "( select cast( substr( DATA_TYPE, 6 ) as BIGINT ) from SYS.ALL_TAB_COLS "
                     "where OWNER = %3 and table_name = %1 and COLUMN_NAME = %2 );"
            ).arg( quotedValue( tableName ),
                  quotedValue( geomCol ),
                  quotedValue( schemaName ) );
      QgsDebugMsgLevel( QStringLiteral( "Getting column datatype: %1" ).arg( sql ), 2 );
      result = connectionRO()->DMexec( sql, false, true );
      QgsDebugMsgLevel( QStringLiteral( "Column datatype query returned %1" ).arg( result.DMntuples() ), 2 );
      if ( result->fetchNext() )
      {
        geomColType = result->value( 0 ).toString();

        if ( geomColType == QLatin1String( "ST_GEOGRAPHY" ) )
          mSpatialColType = SctGeography;
        else if ( geomColType == QLatin1String( "TOPOGEOMETRY" ) )
          mSpatialColType = SctTopoGeometry;
        else
          mSpatialColType = SctGeometry;
      }
      else
      {
        connectionRO()->DMexecNR( QStringLiteral( "COMMIT" ) );
      }
    }
  }
  else
  {
    sql = QStringLiteral( "select %1,DMGEO2.st_srid(%1) from %2 limit 1; " )
                    .arg( quotedIdentifier( mGeometryColumn ), mQuery );
    result = connectionRO()->DMexec( sql, false );
    if ( result && result->execstatus() )
    {
      mSpatialColType = result->getGeoType( 0 );
      detectedType = result->getGeoSubTypeName( 0 );

      if ( result->fetchNext() )
      {
        detectedSrid = result->value( 1 ).toString();

        if ( mSpatialColType == SctNone )
        {
          detectedType = mRequestedGeomType == Qgis::WkbType::Unknown ? QString() : QgsDamengConn::dmSpatialWkbTypeName( mRequestedGeomType );
          detectedSrid = mRequestedSrid;
        }
      }
      else
      {
        connectionRO()->DMexecNR( QStringLiteral( "COMMIT" ) );
        detectedType = mRequestedGeomType == Qgis::WkbType::Unknown ? QString() : QgsDamengConn::dmSpatialWkbTypeName( mRequestedGeomType );
      }
    }
    else
    {
      mValid = false;
      return false;
    }
  }

  mDetectedGeomType = QgsDamengConn::wkbTypeFromDmSpatial( detectedType );
  mDetectedSrid     = detectedSrid;

  if ( mDetectedGeomType == Qgis::WkbType::Unknown )
  {
    QgsDamengLayerProperty layerProperty;
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
      // no data - so take what's requested/detected
      if ( mRequestedGeomType == Qgis::WkbType::Unknown || mDetectedSrid.isEmpty() )
      {
        QgsMessageLog::logMessage( tr( "Geometry type and srid for empty column %1 of %2 undefined." ).arg( mGeometryColumn, mQuery ) );
      }
    }
    else
    {
      int i;
      for ( i = 0; i < layerProperty.size(); i++ )
      {
        Qgis::WkbType wkbType = layerProperty.types.at( i );

        if ( ( wkbType != Qgis::WkbType::Unknown && ( mRequestedGeomType == Qgis::WkbType::Unknown || mRequestedGeomType == wkbType ) ) &&
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

  QgsDebugMsgLevel( QStringLiteral( "Detected SRID is %1" ).arg( mDetectedSrid ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Requested SRID is %1" ).arg( mRequestedSrid ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Detected type is %1" ).arg( mDetectedGeomType ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Requested type is %1" ).arg( mRequestedGeomType ), 2 );

  mValid = ( mDetectedGeomType != Qgis::WkbType::Unknown || mRequestedGeomType != Qgis::WkbType::Unknown )
           && ( !mDetectedSrid.isEmpty() || !mRequestedSrid.isEmpty() );

  if ( !mValid )
    return false;

  QgsDebugMsgLevel( QStringLiteral( "Spatial column type is %1" ).arg( QgsDamengConn::displayStringForGeomType( mSpatialColType ) ), 2 );

  return mValid;
}

bool QgsDamengProvider::convertField( QgsField &field, const QMap<QString, QVariant> *options )
{
  //determine field type to use for strings
  QString stringFieldType = QStringLiteral( "varchar" );
  if ( options && options->value( QStringLiteral( "dropStringConstraints" ), false ).toBool() )
  {
    //drop string length constraints by using Dameng text type for strings
    stringFieldType = QStringLiteral( "text" );
  }

  QString fieldType = stringFieldType; //default to string
  int fieldSize = field.length();
  int fieldPrec = field.precision();

  switch ( field.type() )
  {
    case QVariant::LongLong:
      fieldType = QStringLiteral( "bigint" );
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QVariant::DateTime:
      fieldType = QStringLiteral( "datetime" );
      break;

    case QVariant::Time:
      fieldType = QStringLiteral( "time" );
      break;

    case QVariant::String:
      fieldType = stringFieldType;
      fieldPrec = 0;
      break;

    case QVariant::Int:
      fieldType = QStringLiteral( "int" );
      fieldSize = -1;
      fieldPrec = 0;
      break;

    case QVariant::Date:
      fieldType = QStringLiteral( "date" );
      fieldPrec = 0;
      break;

    case QVariant::Map:
        fieldType = field.typeName();
        if ( fieldType.isEmpty() )
            fieldType = QStringLiteral( "text" );
        fieldPrec = 0;
        break;

	case QVariant::StringList:
      fieldType = QStringLiteral( "text" );
      fieldPrec = 0;
      break;

    case QVariant::List:
    {
      QgsField sub( QString(), field.subType(), QString(), fieldSize, fieldPrec );
      if ( !convertField( sub, nullptr ) ) return false;
      fieldType = "_" + sub.typeName();
      fieldPrec = 0;
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
        fieldType = QStringLiteral( "float" );
      }
      fieldPrec = 0;
      break;

    case QVariant::Bool:
      fieldType = QStringLiteral( "bit" );
      fieldPrec = 0;
      fieldSize = -1;
      break;

    case QVariant::ByteArray:
      fieldType = QStringLiteral( "varbinary" );
      fieldPrec = 0;
      break;

    default:
      return false;
  }

  field.setTypeName( fieldType );
  field.setLength( fieldSize );
  field.setPrecision( fieldPrec );
  return true;
}


void dmSpatialGeometryType( Qgis::WkbType wkbType, QString &geometryType, int &dim )
{
  dim = 2;
  Qgis::WkbType flatType = QgsWkbTypes::flatType( wkbType );
  geometryType = QgsWkbTypes::displayString( flatType ).toUpper();
  switch ( flatType )
  {
    case Qgis::WkbType::Unknown:
      geometryType = QStringLiteral( "GEOMETRY" );
      break;

    case Qgis::WkbType::NoGeometry:
      geometryType.clear();
      dim = 0;
      break;

    default:
      break;
  }

  if ( QgsWkbTypes::hasZ( wkbType ) && QgsWkbTypes::hasM( wkbType ) )
  {
    dim = 4;
  }
  else if ( QgsWkbTypes::hasZ( wkbType ) )
  {
    dim = 3;
  }
  else if ( QgsWkbTypes::hasM( wkbType ) )
  {
    geometryType += QLatin1Char( 'M' );
    dim = 3;
  }
  else if ( wkbType >= Qgis::WkbType::Point25D && wkbType <= Qgis::WkbType::MultiPolygon25D )
  {
    dim = 3;
  }
}

Qgis::VectorExportResult QgsDamengProvider::createEmptyLayer( const QString &uri,
    const QgsFields &fields,
    Qgis::WkbType wkbType,
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

  QgsDebugMsgLevel( QStringLiteral( "Connection info is: %1" ).arg( dsUri.connectionInfo( false ) ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Geometry column is: %1" ).arg( geometryColumn ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Schema is: %1" ).arg( schemaName ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Table name is: %1" ).arg( tableName ), 2 );

  // create the table
  QgsDamengConn *conn = QgsDamengConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Connection to database failed" );
    return Qgis::VectorExportResult::ErrorConnectionFailed;
  }

  // get the pk's name and type
  // Try to find a PK candidate from numeric NOT NULL / UNIQUE columns
  if ( primaryKey.isEmpty() )
  {
    for ( const auto &field : std::as_const( fields ) )
    {
      if ( field.isNumeric() &&
           ( field.constraints().constraints() & QgsFieldConstraints::Constraint::ConstraintUnique ) &&
           ( field.constraints().constraints() & QgsFieldConstraints::Constraint::ConstraintNotNull ) &&
           ( field.constraints().constraints() & QgsFieldConstraints::ConstraintOrigin::ConstraintOriginProvider ) )
      {
        primaryKey = field.name();
        break;
      }
    }
  }

  // if no pk name was passed or guessed, define the new pk field name
  if ( primaryKey.isEmpty() )
  {
    int index = 0;
    QString pk = primaryKey = QStringLiteral( "ID_PK" );
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
    pkType = QStringList( QStringLiteral( "bigint identity(1,1)" ) );
  }
  else
  {
    pkList = parseUriKey( primaryKey );
    const auto constPkList = pkList;
    for ( const QString &col : constPkList )
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
      if ( type.isEmpty() ) type = QStringLiteral( "int" );
      else
      {
        // if the pk field's type is one of the dameng integer types,
        // use the equivalent autoincremental type ( serialN )
        if ( primaryKeyType == QLatin1String( "smallint" ) || primaryKeyType == QLatin1String( "int" ) )
        {
          primaryKeyType = QStringLiteral( "int" );
        }
        else if ( primaryKeyType == QLatin1String( "bigint" ) )
        {
          primaryKeyType = QStringLiteral( "bigint" );
        }
      }
      pkType << type;
    }
  }

  try
  {
    // We want a valid schema name ...
    if ( schemaName.isEmpty() )
    {
      QString sql = QString( "select SYS_CONTEXT(\'userenv\', \'current_schema\') from dual;" );
      QgsDMResult *result( conn->DMexec( sql ) );
      result->fetchNext();
      
      if ( !result || !result->execstatus() )
        throw DMException( result->getMsg() );
      schemaName = result->value( 0 ).toString();
      if ( schemaName.isEmpty() )
      {
        schemaName = QStringLiteral( "SYSDBA" );
      }
    }

    QString sql = QString( "select count( distinct( TABLE_NAME ) ) FROM ALL_TAB_COLUMNS "
                           " where OWNER = %1 and TABLE_NAME = %2;" )
                  .arg( quotedValue( tableName ),
                        quotedValue( schemaName ) );

    QgsDMResult *result( conn->DMexec( sql ) );
    if ( !result || !result->execstatus() )
      throw DMException( result->getMsg() );

    result->fetchNext();
    bool exists = result->value( 0 ).toInt();

    if ( exists && overwrite )
    {
      // delete the table if exists, then re-create it
      QString sql = QString( "CALL DMGEO2.DropGeometryTable(%1,%2);" )
                    .arg( quotedValue( schemaName ),
                          quotedValue( tableName ) );

      result = conn->DMexec( sql );
      if ( !result || !result->execstatus() )
        throw DMException( result->getMsg() );
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
        pk  += QLatin1Char( ',' );
        sql += QLatin1Char( ',' );
      }

      pk += col;
      sql += col + " " + type;
    }
    sql += QStringLiteral( ", PRIMARY KEY (%1) )" ) .arg( pk );

    result = conn->DMexec( sql );
    if ( !result || !result->execstatus() )
      throw DMException( result->getMsg() );

    // get geometry type, dim and srid
    int dim = 2;
    long srid = srs.postgisSrid();

    dmSpatialGeometryType( wkbType, geometryType, dim );

    // create geometry column
    if ( !geometryType.isEmpty() )
    {
      sql = QStringLiteral( "CALL DMGEO2.AddGeometryColumn(%1,%2,%3,%4,%5,%6);" )
            .arg( quotedValue( schemaName ),
                  quotedValue( tableName ),
                  quotedValue( geometryColumn ) )
            .arg( srid )
            .arg( quotedValue( geometryType ) )
            .arg( dim );

      result = conn->DMexec( sql );
      if ( !result || !result->execstatus() )
      {
        conn->DMexec( QStringLiteral( "drop table if exists %1;" ).arg( schemaTableName ) );
        throw DMException( result->getMsg() );
      }
    }
    else
    {
      geometryColumn.clear();
    }

    conn->DMexecNR( QStringLiteral( "COMMIT" ) );
  }
  catch ( DMException &e )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Creation of data source %1 failed: \n%2" )
                      .arg( schemaTableName,
                            e.errorMessage() );

    conn->DMexecNR( QStringLiteral( "ROLLBACK" ) );
    conn->unref();
    return Qgis::VectorExportResult::ErrorCreatingLayer;
  }
  conn->unref();

  QgsDebugMsgLevel( QStringLiteral( "layer %1 created" ).arg( schemaTableName ), 2 );

  // use the provider to edit the table
  dsUri.setDataSource( schemaName, tableName, geometryColumn, QString(), primaryKey );

  QgsDataProvider::ProviderOptions providerOptions;
  Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags();
  std::unique_ptr<QgsDamengProvider> provider = std::make_unique<QgsDamengProvider>( dsUri.uri( false ), providerOptions, flags );
  if ( !provider->isValid() )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Loading of the layer %1 failed" ).arg( schemaTableName );

    return Qgis::VectorExportResult::ErrorInvalidLayer;
  }

  QgsDebugMsgLevel( QStringLiteral( "layer loaded" ), 2 );

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
        QgsDebugMsgLevel( QStringLiteral( "Found a field with the same name of the geometry column. Skip it!" ), 2 );
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
          //convert field name to lowercase ( TODO: avoid doing this over and over )
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

      if ( !( options && options->value( QStringLiteral( "skipConvertFields" ), false ).toBool() ) && !convertField( fld, options ) )
      {
        if ( errorMessage )
          *errorMessage = QObject::tr( "Unsupported type for field %1" ).arg( fld.name() );

        return Qgis::VectorExportResult::ErrorAttributeTypeUnsupported;
      }

      QgsDebugMsgLevel( QStringLiteral( "creating field #%1 -> #%2 name %3 type %4 typename %5 width %6 precision %7" )
                        .arg( fldIdx ).arg( offset ).arg( fld.name(), QVariant::typeToName( fld.type() ), fld.typeName() ).arg( fld.length() ).arg( fld.precision() )
                      , 2);

      flist.append( fld );
      if ( oldToNewAttrIdxMap )
        oldToNewAttrIdxMap->insert( fldIdx, offset++ );
    }
    
    if ( !provider->addAttributes( flist ) )
    {
      if ( errorMessage )
        *errorMessage = QObject::tr( "Creation of fields failed:\n%1" ).arg( provider->errors().join( '\n' ) );

      return Qgis::VectorExportResult::ErrorAttributeCreationFailed;
    }

    QgsDebugMsgLevel( QStringLiteral( "Done creating fields" ), 2 );
  }
  return Qgis::VectorExportResult::Success;
}

QgsCoordinateReferenceSystem QgsDamengProvider::sridToCrs( int srid, QgsDamengConn *conn )
{
  QgsCoordinateReferenceSystem crs;
  if ( conn )
    crs = conn->sridToCrs( srid );
  return crs;
}

QgsCoordinateReferenceSystem QgsDamengProvider::crs() const
{
  QgsCoordinateReferenceSystem srs;
  int srid = mRequestedSrid.isEmpty() ? mDetectedSrid.toInt() : mRequestedSrid.toInt();
  
  return sridToCrs( srid, connectionRO() );
}

QString QgsDamengProvider::subsetString() const
{
  return mSqlWhereClause;
}

QString QgsDamengProvider::getTableName()
{
  return mTableName;
}

size_t QgsDamengProvider::layerCount() const
{
  return 1; // XXX need to return actual number of layers
} 


QString  QgsDamengProvider::name() const
{
  return DAMENG_KEY;
} 

QString  QgsDamengProvider::description() const
{
  QString dmVersion( tr( "Dameng version: unknown" ) );
  QString dmSpatialVersion( tr( "unknown" ) );

  if ( auto *lConnectionRO = connectionRO() )
  {
    QgsDMResult *result = lConnectionRO->DMexec( QStringLiteral( "select DB_VERSION from SYS.V$INSTANCE;" ) );
    result->fetchNext();
    if ( result && result->execstatus() )
    {
      dmVersion = result->value( 0 ).toString();
    }

    dmSpatialVersion = lConnectionRO->dmSpatialVersion();
  }
  else
  {
    dmVersion = tr( "Dameng not connected" );
  }

  return tr( "Dameng/Dameng provider\n%1\nDAMENG %2" ).arg( dmVersion, dmSpatialVersion );
} 

static void jumpSpace( const QString &txt, int &i )
{
  while ( i < txt.length() && txt.at( i ).isSpace() )
    ++i;
}

QString QgsDamengProvider::getNextString( const QString &txt, int &i, const QString &sep )
{
  jumpSpace( txt, i );
  if ( i < txt.length() && txt.at( i ) == '"' )
  {
    const thread_local QRegularExpression stringRe( QRegularExpression::anchoredPattern( "^\"((?:\\\\.|[^\"\\\\])*)\".*" ) );
    const QRegularExpressionMatch match = stringRe.match( txt.mid( i ) );
    if ( !match.hasMatch() )
    {
      QgsMessageLog::logMessage( tr( "Cannot find end of double quoted string: %1" ).arg( txt ), tr( "Dameng" ) );
      return QString();
    }
    i += match.captured( 1 ).length() + 2;
    jumpSpace( txt, i );
    if ( !QStringView{txt}.mid( i ).startsWith( sep ) && i < txt.length() )
    {
      QgsMessageLog::logMessage( tr( "Cannot find separator: %1" ).arg( txt.mid( i ) ), tr( "Dameng" ) );
      return QString();
    }
    i += sep.length();
    return match.captured( 1 ).replace( QLatin1String( "\\\"" ), QLatin1String( "\"" ) ).replace( QLatin1String( "\\\\" ), QLatin1String( "\\" ) );
  }
  else
  {
    int start = i;
    for ( ; i < txt.length(); i++ )
    {
      if ( QStringView{txt}.mid( i ).startsWith( sep ) )
      {
        QStringView v( QStringView{txt}.mid( start, i - start ) );
        i += sep.length();
        return v.trimmed().toString();
      }
    }
    return QStringView{txt}.mid( start, i - start ).trimmed().toString();
  }
}

QVariant QgsDamengProvider::convertValue( QMetaType::Type type, QMetaType::Type subType, const QString &value, const QString &typeName )
{
  Q_UNUSED( subType )
  QVariant result;
  switch ( type )
  {
    case QVariant::Map:
      if ( typeName == QLatin1String( "json" ) || typeName == QLatin1String( "jsonb" ) )
        result = QgsJsonUtils::parseJson( value );
      break;
    case QVariant::Bool:
      if ( value == "1" )
        result = true;
      else if ( value == "0" )
        result = false;
      else
        result = QgsVariantUtils::createNullVariant( type );
      break;
    default:
      result = value;
      if ( !result.convert( type ) || value.isNull() )
        result = QgsVariantUtils::createNullVariant( type );
      break;
  }

  return result;
}

QList<QgsVectorLayer *> QgsDamengProvider::searchLayers( const QList<QgsVectorLayer *> &layers, const QString &connectionInfo, const QString &schema, const QString &tableName )
{
  QList<QgsVectorLayer *> result;
  const auto constLayers = layers;
  for ( QgsVectorLayer *layer : constLayers )
  {
    const QgsDamengProvider *dmProvider = qobject_cast<QgsDamengProvider *>( layer->dataProvider() );
    if ( dmProvider && dmProvider->mUri.connectionInfo( false ) == connectionInfo && dmProvider->mSchemaName == schema && dmProvider->mTableName == tableName )
    {
      result.append( layer );
    }
  }
  return result;
}

void QgsDamengProvider::setQuery( const QString &query )
{
  mQuery = query;

  mKind = Relkind::NotSet;
}

QgsDamengProvider::Relkind QgsDamengProvider::relkind() const
{
  if ( mKind != Relkind::NotSet )
    return mKind;

  if ( mIsQuery || !connectionRO() )
  {
    mKind = Relkind::Unknown;
  }
  else
  {
    QString sql = QStringLiteral( "SELECT distinct( SUBTYPE$),INFO5 FROM SYS.SYSOBJECTS A where "
                                  " ( TYPE$ = \'SCHOBJ\' or TYPE$ = \'TABOBJ\') and NAME = %2 and "
                                  " SCHID = ( select ID from SYS.SYSOBJECTS B where "
                                  " NAME = %1 and TYPE$ = \'SCH\'); " ).arg( quotedValue( mSchemaName ),quotedValue( mTableName ) );
    QgsDMResult *res( connectionRO()->DMexec( sql ) );
    res->fetchNext();
    QString type = res->value( 0 ).toString();
    QString viewInfo = res->value( 1 ).toString();

    mKind = Relkind::Unknown;

    if ( type == QLatin1String( "UTAB" ) )
    {
      mKind = Relkind::OrdinaryTable;
    }
    else if ( type == QLatin1String( "VIEW" ) && viewInfo == QLatin1String( "0x" ) )
    {
      mKind = Relkind::View;
    }
    else if ( type == QLatin1String( "VIEW" ) && viewInfo != QLatin1String( "0x" ) )
    {
      mKind = Relkind::MaterializedView;
    }
  }

  return mKind;
}

bool QgsDamengProvider::hasMetadata() const
{
  bool hasMetadata = true;
  QgsDamengProvider::Relkind kind = relkind();

  if ( kind == Relkind::View || kind == Relkind::MaterializedView )
  {
    hasMetadata = false;
  }

  return hasMetadata;
}


// ---------------------  providermetadata  --------------------------------------------

QgsDataProvider *QgsDamengProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  return new QgsDamengProvider( uri, options, flags );
}

QList<QgsDataItemProvider *> QgsDamengProviderMetadata::dataItemProviders() const
{
  QList<QgsDataItemProvider *> providers;
  providers << new QgsDamengDataItemProvider;
  return providers;
}

Qgis::VectorExportResult QgsDamengProviderMetadata::createEmptyLayer(
  const QString &uri,
  const QgsFields &fields,
  Qgis::WkbType wkbType,
  const QgsCoordinateReferenceSystem &srs,
  bool overwrite,
  QMap<int, int> &oldToNewAttrIdxMap,
  QString &errorMessage,
  const QMap<QString, QVariant> *options )
{
  return QgsDamengProvider::createEmptyLayer(
           uri, fields, wkbType, srs, overwrite,
           &oldToNewAttrIdxMap, &errorMessage, options
         );
}

bool QgsDamengProviderMetadata::styleExists( const QString &uri, const QString &styleId, QString &errorCause )
{
  errorCause.clear();

  QgsDataSourceUri dsUri( uri );
  QgsDamengConn *conn = QgsDamengConn::connectDb( dsUri.connectionInfo( false ), true);
  if ( !conn )
  {
    errorCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  if ( !tableExists( *conn, QStringLiteral( "LAYER_STYLES" ) ) )
  {
    return false;
  }
  else if ( !columnExists( *conn, QStringLiteral( "LAYER_STYLES" ), QStringLiteral( "type" ) ) )
  {
    return false;
  }

  if ( dsUri.database().isEmpty() ) // typically when a service file is used
  {
    dsUri.setDatabase( conn->currentDatabase() );
  }

  const QString wkbTypeString = QgsDamengConn::quotedValue( QgsWkbTypes::geometryDisplayString( QgsWkbTypes::geometryType( dsUri.wkbType() ) ) );

  const QString checkQuery = QString( "SELECT styleName"
                                      " FROM %1.LAYER_STYLES"
                                      " WHERE f_table_catalog=%1"
                                      " AND f_table_schema=%2"
                                      " AND f_table_name=%3"
                                      " AND f_geometry_column %4"
                                      " AND (type=%5 OR type IS NULL)"
                                      " AND styleName=%6" ).arg( "SYSDBA" )
                               .arg( QgsDamengConn::quotedValue( dsUri.database() ) )
                               .arg( QgsDamengConn::quotedValue( dsUri.schema() ) )
                               .arg( QgsDamengConn::quotedValue( dsUri.table() ) )
                               .arg( dsUri.geometryColumn().isEmpty() ? QStringLiteral( "IS NULL" ) : QStringLiteral( "= %1" ).arg( QgsDamengConn::quotedValue( dsUri.geometryColumn() ) ) )
                               .arg( wkbTypeString )
                               .arg( QgsDamengConn::quotedValue( styleId.isEmpty() ? dsUri.table() : styleId ) );

  QgsDamengResult res( conn->DMexec( checkQuery ) );
  if ( res.DMresultStatus() == DmResCommandOk)
  {
    return res.result()->fetchNext();
  }
  else
  {
    errorCause = res.DMresultErrorMessage();
    return false;
  }

}

bool QgsDamengProviderMetadata::saveStyle( const QString &uri, const QString &qmlStyleIn, const QString &sldStyleIn, const QString &styleName, const QString &styleDescription, const QString &uiFileContent, bool useAsDefault, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );

  // Replace invalid XML characters
  QString qmlStyle { qmlStyleIn };
  QgsDamengUtils::replaceInvalidXmlChars( qmlStyle );
  QString sldStyle { sldStyleIn };
  QgsDamengUtils::replaceInvalidXmlChars( sldStyle );

  QgsDamengConn *conn = QgsDamengConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  if ( !tableExists( *conn, QStringLiteral( "LAYER_STYLES" ) ) )
  {
    QgsDamengResult res( conn->DMexec( QStringLiteral( "CREATE TABLE %1.LAYER_STYLES("
                                         "ID int identity( 1,1 ) PRIMARY KEY"
                                         ",f_table_schema varchar"
                                         ",f_table_name varchar"
                                         ",f_geometry_column varchar"
                                         ",styleName text"
                                         ",styleQML xmltype"
                                         ",styleSLD xmltype"
                                         ",useAsDefault bit"
                                         ",description text"
                                         ",owner varchar( 63 ) DEFAULT CURRENT_USER"
                                         ",ui xmltype"
                                         ",update_time timestamp DEFAULT CURRENT_TIMESTAMP"
                                         ",TYPE varchar"
                                         ")" ).arg( "SYSDBA" ) ) );
    if ( res.DMresultStatus() != DmResCommandOk )
    {
      errCause = QObject::tr( "Unable to save layer style. It's not possible to create the destination table on the database. Maybe this is due to table permissions (user=%1). Please contact your database admin" ).arg( dsUri.username() );
      conn->unref();
      return false;
    }
  }
  else
  {
    if ( !columnExists( *conn, QStringLiteral( "LAYER_STYLES" ), QStringLiteral( "TYPE" ) ) )
    {
      QgsDamengResult res( conn->DMexec( QStringLiteral( "ALTER TABLE %1.LAYER_STYLES ADD COLUMN TYPE varchar NULL" ).arg( "SYSDBA" ) ) );
      if ( res.DMresultStatus() != DmResCommandOk )
      {
        errCause = QObject::tr( "Unable to add column TYPE to LAYER_STYLES table. Maybe this is due to table permissions (user=%1). Please contact your database admin" ).arg( dsUri.username() );
        conn->unref();
        return false;
      }
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
    uiFileValue = QStringLiteral( ", %1" ).arg( QgsDamengConn::quotedValue( uiFileContent ) );
  }

  const QString wkbTypeString = QgsDamengConn::quotedValue( QgsWkbTypes::geometryDisplayString( QgsWkbTypes::geometryType( dsUri.wkbType() ) ) );

  // Note: in the construction of the INSERT and UPDATE strings the qmlStyle and sldStyle values
  // can contain user entered strings, which may themselves include %## values that would be
  // replaced by the QString.arg function.  To ensure that the final SQL string is not corrupt these
  // two values are both replaced in the final .arg call of the string construction.

  QString sql = QString( "INSERT INTO %1.LAYER_STYLES("
                         "f_table_schema,f_table_name,f_geometry_column,styleName,styleQML,styleSLD,useAsDefault,description,owner,TYPE%10"
                         ") VALUES ("
                         "%2,%3,%4,%5, %12, %13,%6,%7,%8,%9%11"
                         ")" )
                .arg( "SYSDBA" )
                .arg( QgsDamengConn::quotedValue( dsUri.schema() ) )
                .arg( QgsDamengConn::quotedValue( dsUri.table() ) )
                .arg( QgsDamengConn::quotedValue( dsUri.geometryColumn() ) )
                .arg( QgsDamengConn::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) )
                .arg( useAsDefault ? "1" : "0" )
                .arg( QgsDamengConn::quotedValue( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription ) )
                .arg( "CURRENT_USER" )
                .arg( uiFileColumn )
                .arg( uiFileValue )
                .arg( wkbTypeString )
                // Must be the final .arg replacement - see above
                .arg( QgsDamengConn::quotedValue( qmlStyle ),
                      QgsDamengConn::quotedValue( sldStyle ) );

  QString checkQuery = QString( "SELECT styleName"
                                " FROM %1.LAYER_STYLES"
                                " WHERE f_table_schema=%2"
                                " AND f_table_name=%3"
                                " AND f_geometry_column=%4"
                                " AND ( TYPE=%5 OR TYPE IS NULL )"
                                " AND styleName=%6" )
                       .arg( "SYSDBA" )
                       .arg( QgsDamengConn::quotedValue( dsUri.schema() ) )
                       .arg( QgsDamengConn::quotedValue( dsUri.table() ) )
                       .arg( QgsDamengConn::quotedValue( dsUri.geometryColumn() ) )
                       .arg( wkbTypeString )
                       .arg( QgsDamengConn::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) );

  QgsDMResult *res( conn->DMexec( checkQuery ) );
  
  if ( res->fetchNext() )
  {
    sql = QString( "UPDATE %5.LAYER_STYLES"
                   " SET useAsDefault=%1"
                   ",styleQML= %10"
                   ",styleSLD= %11"
                   ",description=%3"
                   ",owner=%4"
                   ",TYPE=%2"
                   " WHERE f_table_schema=%6"
                   " AND f_table_name=%7"
                   " AND f_geometry_column=%8"
                   " AND styleName=%9"
                   " AND ( TYPE=%2 OR TYPE IS NULL )" )
          .arg( useAsDefault ? "1" : "0" )
          .arg( wkbTypeString )
          .arg( QgsDamengConn::quotedValue( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription ) )
          .arg( "CURRENT_USER" )
          .arg( "SYSDBA" )
          .arg( QgsDamengConn::quotedValue( dsUri.schema() ) )
          .arg( QgsDamengConn::quotedValue( dsUri.table() ) )
          .arg( QgsDamengConn::quotedValue( dsUri.geometryColumn() ) )
          .arg( QgsDamengConn::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) )
          // Must be the final .arg replacement - see above
          .arg( QgsDamengConn::quotedValue( qmlStyle ),
                QgsDamengConn::quotedValue( sldStyle ) );
  }

  if ( useAsDefault )
  {
    QString removeDefaultSql = QString( "UPDATE %1.LAYER_STYLES"
                                        " SET useAsDefault=0"
                                        " WHERE f_table_schema=%2"
                                        " AND f_table_name=%3"
                                        " AND f_geometry_column=%4"
                                        " AND ( TYPE=%5 OR TYPE IS NULL )" )
                               .arg( "SYSDBA" )
                               .arg( QgsDamengConn::quotedValue( dsUri.schema() ) )
                               .arg( QgsDamengConn::quotedValue( dsUri.table() ) )
                               .arg( QgsDamengConn::quotedValue( dsUri.geometryColumn() ) )
                               .arg( wkbTypeString );

    sql = QStringLiteral( "%1; %2; COMMIT;" ).arg( removeDefaultSql, sql );
  }

  res = conn->DMexec( sql );

  bool saved = res && res && res->execstatus();
  if ( !saved )
    errCause = QObject::tr( "Unable to save layer style. It's not possible to insert a new record into the style table. Maybe this is due to table permissions (user=%1). Please contact your database administrator." ).arg( dsUri.username() );

  conn->unref();

  return saved;
}


QString QgsDamengProviderMetadata::loadStyle( const QString &uri, QString &errCause )
{
  QString styleName;
  return loadStoredStyle( uri, styleName, errCause );
}

QString QgsDamengProviderMetadata::loadStoredStyle( const QString &uri, QString &styleName, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  QString selectQmlQuery;

  QgsDamengConn *conn = QgsDamengConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return QString();
  }

  if ( dsUri.database().isEmpty() ) // typically when a service file is used
  {
    dsUri.setDatabase( conn->currentDatabase() );
  }

  if ( !tableExists( *conn, QStringLiteral( "LAYER_STYLES" ) ) )
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
    geomColumnExpr = QStringLiteral( "=" ) + QgsDamengConn::quotedValue( dsUri.geometryColumn() );
  }

  QString wkbTypeString = QgsDamengConn::quotedValue( QgsWkbTypes::geometryDisplayString( QgsWkbTypes::geometryType( dsUri.wkbType() ) ) );

  if ( !columnExists( *conn, QStringLiteral( "LAYER_STYLES" ), QStringLiteral( "type" ) ) )
  {
    selectQmlQuery = QString( "SELECT styleQML"
                              " FROM %1.LAYER_STYLES"
                              " WHERE f_table_schema=%2"
                              " AND f_table_name=%3"
                              " AND f_geometry_column %4"
                              " ORDER BY CASE WHEN useAsDefault THEN 1 ELSE 2 END"
                              ",update_time DESC LIMIT 1" )
                     .arg( "SYSDBA" )
                     .arg( QgsDamengConn::quotedValue( dsUri.schema() ) )
                     .arg( QgsDamengConn::quotedValue( dsUri.table() ) )
                     .arg( geomColumnExpr );
  }
  else
  {
    selectQmlQuery = QString( "SELECT styleQML"
                              " FROM %1.LAYER_STYLES"
                              " WHERE f_table_schema=%2"
                              " AND f_table_name=%3"
                              " AND f_geometry_column %4"
                              " AND ( type=%5 OR type IS NULL )"
                              " ORDER BY CASE WHEN useAsDefault THEN 1 ELSE 2 END"
                              ",update_time DESC LIMIT 1" )
                     .arg( "SYSDBA" )
                     .arg( QgsDamengConn::quotedValue( dsUri.schema() ) )
                     .arg( QgsDamengConn::quotedValue( dsUri.table() ) )
                     .arg( geomColumnExpr )
                     .arg( wkbTypeString );
  }

  QgsDMResult *result( conn->DMexec( selectQmlQuery ) );
  QString style = result->fetchNext() ? result->value( 0 ).toString() : QString();
  conn->unref();

  QgsDamengUtils::restoreInvalidXmlChars( style );

  return style;
}

int QgsDamengProviderMetadata::listStyles( const QString &uri, QStringList &ids, QStringList &names,
    QStringList &descriptions, QString &errCause )
{
  errCause.clear();
  QgsDataSourceUri dsUri( uri );

  QgsDamengConn *conn = QgsDamengConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed using username: %1" ).arg( dsUri.username() );
    return -1;
  }
  
  if ( !tableExists( *conn, QStringLiteral( "layer_styles" ) ) )
  {
    return -1;
  }

  if ( dsUri.database().isEmpty() ) // typically when a service file is used
  {
    dsUri.setDatabase( conn->currentDatabase() );
  }

  QString wkbTypeString = QgsDamengConn::quotedValue( QgsWkbTypes::geometryDisplayString( QgsWkbTypes::geometryType( dsUri.wkbType() ) ) );

  QString selectRelatedQuery = QString( "SELECT id,styleName,description"
                                        " FROM %1.LAYER_STYLES"
                                        " WHERE f_table_schema=%2"
                                        " AND f_table_name=%3"
                                        " AND %4"
                                        " AND ( type=%5 OR type IS NULL )"
                                        " ORDER BY useasdefault DESC, update_time DESC" )
                               .arg( "SYSDBA" )
                               .arg( QgsDamengConn::quotedValue( dsUri.schema() ) )
                               .arg( QgsDamengConn::quotedValue( dsUri.table() ) )
                               .arg( dsUri.geometryColumn().isEmpty() ? "f_geometry_column is NULL" :
                                     QString( "f_geometry_column=%1" ).arg( QgsDamengConn::quotedValue( dsUri.geometryColumn() ) ) )
                               .arg( wkbTypeString );

  QgsDMResult *result( conn->DMexec( selectRelatedQuery ) );
  if ( !result || !result->execstatus() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectRelatedQuery ) );
    errCause = QObject::tr( "Error executing the select query for related styles. The query was logged" );
    conn->unref();
    return -1;
  }

  int numberOfRelatedStyles = 0;
  while ( result->fetchNext() )
  {
    ids.append( result->value( 0 ).toString() );
    names.append( result->value( 1 ).toString() );
    descriptions.append( result->value( 2 ).toString() );
    ++numberOfRelatedStyles;
  }

  QString selectOthersQuery = QString( "SELECT id,styleName,description"
                                       " FROM %1.LAYER_STYLES"
                                       " WHERE NOT ( f_table_schema=%2 AND f_table_name=%3 AND f_geometry_column=%4 AND type=%5 )"
                                       " ORDER BY update_time DESC" )
                              .arg( "SYSDBA" )
                              .arg( QgsDamengConn::quotedValue( dsUri.schema() ) )
                              .arg( QgsDamengConn::quotedValue( dsUri.table() ) )
                              .arg( QgsDamengConn::quotedValue( dsUri.geometryColumn() ) )
                              .arg( wkbTypeString );

  result = conn->DMexec( selectOthersQuery );
  if ( !result || !result->execstatus() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectOthersQuery ) );
    errCause = QObject::tr( "Error executing the select query for unrelated styles. The query was logged" );
    conn->unref();
    return -1;
  }

  while ( result->fetchNext() )
  {
    ids.append( result->value( 0 ).toString() );
    names.append( result->value( 1 ).toString() );
    descriptions.append( result->value( 2 ).toString() );
  }

  conn->unref();

  return numberOfRelatedStyles;
}

bool QgsDamengProviderMetadata::deleteStyleById( const QString &uri, const QString &styleId, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  bool deleted;

  QgsDamengConn *conn = QgsDamengConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed using username: %1" ).arg( dsUri.username() );
    deleted = false;
  }
  else
  {
    QString deleteStyleQuery = QStringLiteral( "DELETE FROM %2.LAYER_STYLES WHERE id=%1" )
                                  .arg( QgsDamengConn::quotedValue( styleId ) )
                                  .arg( "SYSDBA" );
    QgsDamengResult result( conn->DMexec( deleteStyleQuery ) );
    if ( result.DMresultStatus() != DmResCommandOk )
    {
      QgsDebugError(
        QString( "DMexec of this query returning != DMRES_COMMAND_OK (%1 != expected %2 ): %3" )
        .arg( result.DMresultStatus() ).arg( DmResCommandOk ).arg( deleteStyleQuery ) );
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

QString QgsDamengProviderMetadata::getStyleById( const QString &uri, const QString &styleId, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );

  QgsDamengConn *conn = QgsDamengConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed using username: %1" ).arg( dsUri.username() );
    return QString();
  }

  QString style;
  QString selectQmlQuery = QStringLiteral( "SELECT styleQml FROM %2.LAYER_STYLES WHERE id=%1" )
                            .arg( QgsDamengConn::quotedValue( styleId ) ).arg( "SYSDBA" );
  QgsDMResult *result( conn->DMexec( selectQmlQuery ) );

  if ( result && result->execstatus() )
  {
    if ( result->fetchNext() )
      style = result->value( 0 ).toString();
    else
      errCause = QObject::tr( "Consistency error in table '%1'. Style id should be unique" ).arg( QLatin1String( "LAYER_STYLES" ) );
  }
  else
  {
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectQmlQuery ) );
    errCause = QObject::tr( "Error executing the select query. The query was logged" );
  }

  conn->unref();

  QgsDamengUtils::restoreInvalidXmlChars( style );

  return style;
}

QgsTransaction *QgsDamengProviderMetadata::createTransaction( const QString &connString )
{
  return new QgsDamengTransaction( connString );
}


QMap< QString, QgsAbstractProviderConnection *> QgsDamengProviderMetadata::connections( bool cached )
{
  return connectionsProtected<QgsDamengProviderConnection, QgsDamengConn>( cached );
}

QgsAbstractProviderConnection *QgsDamengProviderMetadata::createConnection( const QString &uri, const QVariantMap &configuration )
{
  return new QgsDamengProviderConnection( uri, configuration );
}

void QgsDamengProviderMetadata::deleteConnection( const QString &name )
{
  deleteConnectionProtected<QgsDamengProviderConnection>( name );
}

void QgsDamengProviderMetadata::saveConnection( const QgsAbstractProviderConnection *conn,  const QString &name )
{
  saveConnectionProtected( conn, name );
}

QgsAbstractProviderConnection *QgsDamengProviderMetadata::createConnection( const QString &name )
{
  return new QgsDamengProviderConnection( name );
}


QgsDamengProjectStorage *gDmProjectStorage = nullptr;   // when not null it is owned by QgsApplication::projectStorageRegistry()

void QgsDamengProviderMetadata::initProvider()
{
  Q_ASSERT( !gDmProjectStorage );
  gDmProjectStorage = new QgsDamengProjectStorage;
  QgsApplication::projectStorageRegistry()->registerProjectStorage( gDmProjectStorage );  // takes ownership
}

void QgsDamengProviderMetadata::cleanupProvider()
{
  QgsApplication::projectStorageRegistry()->unregisterProjectStorage( gDmProjectStorage );  // destroys the object
  gDmProjectStorage = nullptr;

  QgsDamengConnPool::cleanupInstance();
}


// ---------- shareddata ------------------

void QgsDamengSharedData::addFeaturesCounted( long long diff )
{
  QMutexLocker locker( &mMutex );

  if ( mFeaturesCounted >= 0 )
    mFeaturesCounted += diff;
}

void QgsDamengSharedData::ensureFeaturesCountedAtLeast( long long fetched )
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

long long QgsDamengSharedData::featuresCounted()
{
  QMutexLocker locker( &mMutex );
  return mFeaturesCounted;
}

void QgsDamengSharedData::setFeaturesCounted( long long count )
{
  QMutexLocker locker( &mMutex );
  mFeaturesCounted = count;
}


QgsFeatureId QgsDamengSharedData::lookupFid( const QVariantList &v )
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


QVariantList QgsDamengSharedData::removeFid( QgsFeatureId fid )
{
  QMutexLocker locker( &mMutex );

  QVariantList v = mFidToKey[ fid ];
  mFidToKey.remove( fid );
  mKeyToFid.remove( v );
  return v;
}

void QgsDamengSharedData::insertFid( QgsFeatureId fid, const QVariantList &k )
{
  QMutexLocker locker( &mMutex );

  mFidToKey.insert( fid, k );
  mKeyToFid.insert( k, fid );
}

QVariantList QgsDamengSharedData::lookupKey( QgsFeatureId featureId )
{
  QMutexLocker locker( &mMutex );

  QMap<QgsFeatureId, QVariantList>::const_iterator it = mFidToKey.constFind( featureId );
  if ( it != mFidToKey.constEnd() )
    return it.value();
  return QVariantList();
}

void QgsDamengSharedData::clear()
{
  QMutexLocker locker( &mMutex );
  mFidToKey.clear();
  mKeyToFid.clear();
  mFeaturesCounted = -1;
  mFidCounter = 0;
}

void QgsDamengSharedData::clearSupportsEnumValuesCache()
{
  QMutexLocker locker( &mMutex );
  mFieldSupportsEnumValues.clear();
}

bool QgsDamengSharedData::fieldSupportsEnumValuesIsSet( int index )
{
  QMutexLocker locker( &mMutex );
  return mFieldSupportsEnumValues.contains( index );
}

bool QgsDamengSharedData::fieldSupportsEnumValues( int index )
{
  QMutexLocker locker( &mMutex );
  return mFieldSupportsEnumValues.contains( index ) && mFieldSupportsEnumValues[index];
}

void QgsDamengSharedData::setFieldSupportsEnumValues( int index, bool isSupported )
{
  QMutexLocker locker( &mMutex );
  mFieldSupportsEnumValues[ index ] = isSupported;
}


QgsDamengProviderMetadata::QgsDamengProviderMetadata()
  : QgsProviderMetadata( QgsDamengProvider::DAMENG_KEY, QgsDamengProvider::DAMENG_DESCRIPTION )
{
}

QIcon QgsDamengProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconDameng.svg" ) );
}

#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsDamengProviderMetadata();
}
#endif


QVariantMap QgsDamengProviderMetadata::decodeUri( const QString &uri ) const
{
  const QgsDataSourceUri dsUri { uri };
  QVariantMap uriParts;

  if ( !dsUri.database().isEmpty() )
    uriParts[QStringLiteral( "dbname" )] = dsUri.database();
  if ( !dsUri.host().isEmpty() )
    uriParts[QStringLiteral( "host" )] = dsUri.host();
  if ( !dsUri.port().isEmpty() )
    uriParts[QStringLiteral( "port" )] = dsUri.port();
  if ( !dsUri.username().isEmpty() )
    uriParts[QStringLiteral( "username" )] = dsUri.username();
  if ( !dsUri.password().isEmpty() )
    uriParts[QStringLiteral( "password" )] = dsUri.password();
  if ( !dsUri.authConfigId().isEmpty() )
    uriParts[QStringLiteral( "authcfg" )] = dsUri.authConfigId();
  if ( dsUri.wkbType() != Qgis::WkbType::Unknown )
    uriParts[QStringLiteral( "type" )] = static_cast<quint32>( dsUri.wkbType() );

  if ( uri.contains( QStringLiteral( "selectatid=" ), Qt::CaseSensitivity::CaseInsensitive ) )
    uriParts[QStringLiteral( "selectatid" )] = !dsUri.selectAtIdDisabled();

  if ( !dsUri.table().isEmpty() )
    uriParts[QStringLiteral( "table" )] = dsUri.table();
  if ( !dsUri.schema().isEmpty() )
    uriParts[QStringLiteral( "schema" )] = dsUri.schema();
  if ( !dsUri.keyColumn().isEmpty() )
    uriParts[QStringLiteral( "key" )] = dsUri.keyColumn();
  if ( !dsUri.srid().isEmpty() )
    uriParts[QStringLiteral( "srid" )] = dsUri.srid();

  if ( uri.contains( QStringLiteral( "estimatedmetadata=" ), Qt::CaseSensitivity::CaseInsensitive ) )
    uriParts[QStringLiteral( "estimatedmetadata" )] = dsUri.useEstimatedMetadata();

  if ( !dsUri.sql().isEmpty() )
    uriParts[QStringLiteral( "sql" )] = dsUri.sql();
  if ( !dsUri.geometryColumn().isEmpty() )
    uriParts[QStringLiteral( "geometrycolumn" )] = dsUri.geometryColumn();

  return uriParts;
}

QString QgsDamengProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  if ( parts.contains( QStringLiteral( "dbname" ) ) )
    dsUri.setDatabase( parts.value( QStringLiteral( "dbname" ) ).toString() );
  if ( parts.contains( QStringLiteral( "port" ) ) )
    dsUri.setParam( QStringLiteral( "port" ), parts.value( QStringLiteral( "port" ) ).toString() );
  if ( parts.contains( QStringLiteral( "host" ) ) )
    dsUri.setParam( QStringLiteral( "host" ), parts.value( QStringLiteral( "host" ) ).toString() );
  if ( parts.contains( QStringLiteral( "username" ) ) )
    dsUri.setUsername( parts.value( QStringLiteral( "username" ) ).toString() );
  if ( parts.contains( QStringLiteral( "password" ) ) )
    dsUri.setPassword( parts.value( QStringLiteral( "password" ) ).toString() );
  if ( parts.contains( QStringLiteral( "authcfg" ) ) )
    dsUri.setAuthConfigId( parts.value( QStringLiteral( "authcfg" ) ).toString() );
  if ( parts.contains( QStringLiteral( "type" ) ) )
    dsUri.setParam( QStringLiteral( "type" ), QgsWkbTypes::displayString( static_cast<Qgis::WkbType>( parts.value( QStringLiteral( "type" ) ).toInt() ) ) );
  if ( parts.contains( QStringLiteral( "selectatid" ) ) )
    dsUri.setParam( QStringLiteral( "selectatid" ), parts.value( QStringLiteral( "selectatid" ) ).toString() );
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
  if ( parts.contains( QStringLiteral( "sql" ) ) )
    dsUri.setSql( parts.value( QStringLiteral( "sql" ) ).toString() );
  if ( parts.contains( QStringLiteral( "checkPrimaryKeyUnicity" ) ) )
    dsUri.setParam( QStringLiteral( "checkPrimaryKeyUnicity" ), parts.value( QStringLiteral( "checkPrimaryKeyUnicity" ) ).toString() );
  if ( parts.contains( QStringLiteral( "geometrycolumn" ) ) )
    dsUri.setGeometryColumn( parts.value( QStringLiteral( "geometrycolumn" ) ).toString() );
  return dsUri.uri( false );
}

QList<Qgis::LayerType> QgsDamengProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::Vector };
}
