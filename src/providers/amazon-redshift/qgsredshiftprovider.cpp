/***************************************************************************
   qgsredshiftprovider.cpp
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsredshiftprovider.h"

#include <QMessageBox>
#include <QRandomGenerator>

#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsfeature.h"
#include "qgsfeedback.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsjsonutils.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsmessageoutput.h"
#include "qgsprojectstorageregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsrectangle.h"
#include "qgsredshiftconn.h"
#include "qgsredshiftconnpool.h"
#include "qgsredshiftdataitems.h"
#include "qgsredshiftfeatureiterator.h"
#include "qgsredshiftprojectstorage.h"
#include "qgsredshiftproviderconnection.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerexporter.h"
#include "qgsxmlutils.h"

const QString QgsRedshiftProvider::REDSHIFT_KEY = QStringLiteral( "redshift" );
const QString QgsRedshiftProvider::REDSHIFT_DESCRIPTION =
  QStringLiteral( "Amazon Redshift/Amazon Redshift data provider" );

static const QString EDITOR_WIDGET_STYLES_TABLE = QStringLiteral( "qgis_editor_widget_styles" );

static bool tableExists( QgsRedshiftConn &conn, const QString &name )
{
  QgsRedshiftResult res( conn.PQexec( "SELECT EXISTS ( SELECT oid FROM pg_catalog.pg_class WHERE relname=" +
                                      QgsRedshiftConn::quotedValue( name ) + ")" ) );
  return res.PQgetvalue( 0, 0 ).startsWith( 't' );
}

QgsRedshiftProvider::QgsRedshiftProvider( QString const &uri, const ProviderOptions &options,
    QgsDataProvider::ReadFlags flags )
  : QgsVectorDataProvider( uri, options, flags ), mShared( new QgsRedshiftSharedData )
{
  QgsDebugMsgLevel( QStringLiteral( "URI: %1 " ).arg( uri ), 2 );

  mUri = QgsDataSourceUri( uri );

  // Populate members from the uri structure.
  mExternalDatabaseName = mUri.externalDatabase() != mUri.database() ? mUri.externalDatabase().toLower() : QString();
  mSchemaName = mUri.schema().toLower();
  mTableName = mUri.table().toLower();
  mGeometryColumn = mUri.geometryColumn().toLower();
  mBoundingBoxColumn = mUri.param( "bbox" );
  if ( mBoundingBoxColumn.isEmpty() )
  {
    mBoundingBoxColumn = mGeometryColumn;
  }
  mSqlWhereClause = mUri.sql();

  // This sets default SRID and GeometryType for the case that the table is empty
  // and has no geometry constraints on the geometry column.
  mRequestedSrid = mUri.srid();
  mRequestedGeomType = mUri.wkbType();

  const QString checkUnicityKey{QStringLiteral( "checkPrimaryKeyUnicity" )};
  if ( mUri.hasParam( checkUnicityKey ) )
  {
    if ( mUri.param( checkUnicityKey ).compare( QLatin1String( "0" ) ) == 0 )
    {
      mCheckPrimaryKeyUnicity = false;
    }
    else
    {
      mCheckPrimaryKeyUnicity = true;
    }
    if ( mReadFlags & QgsDataProvider::FlagTrustDataSource )
    {
      mCheckPrimaryKeyUnicity = false;
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

    if ( !mExternalDatabaseName.isEmpty() )
    {
      mQuery += quotedIdentifier( mExternalDatabaseName ) + '.';
    }

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
  if ( mReadFlags & QgsDataProvider::FlagTrustDataSource )
  {
    mUseEstimatedMetadata = true;
  }
  mSelectAtIdDisabled = mUri.selectAtIdDisabled();

  QgsDebugMsgLevel( QStringLiteral( "Connection info is %1" ).arg( mUri.connectionInfo( false ) ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Geometry column is: %1" ).arg( mGeometryColumn ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "External Database is: %1" ).arg( mExternalDatabaseName ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Schema is: %1" ).arg( mSchemaName ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Table name is: %1" ).arg( mTableName ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Query is: %1" ).arg( mQuery ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Where clause is: %1" ).arg( mSqlWhereClause ), 2 );

  // No table/query passed, the provider could be used to get tables.

  if ( mQuery.isEmpty() )
  {
    return;
  }

  mConnectionRO = QgsRedshiftConn::connectDb( mUri.connectionInfo( false ), true );
  if ( !mConnectionRO )
  {
    QgsMessageLog::logMessage( tr( "Failed to connect to database." ),
                               tr( "Redshift Spatial" ) );
    return;
  }
  // Check permissions and set capabilities.
  if ( !setPermsAndCapabilities() )
  {
    disconnectDb();
    return;
  }

  // Gets srid, geometry and data type.
  if ( !getGeometryDetails() )
  {
    // The table is not a geometry table.
    QgsMessageLog::logMessage( tr( "Invalid Redshift layer: failed to get "
                                   "geometry type or SRID of layer." ),
                               tr( "Redshift Spatial" ) );
    disconnectDb();
    return;
  }

  mLayerExtent.setMinimal();

  // Set the primary key.
  bool pkChosen = true;
  if ( !mExternalDatabaseName.isEmpty() )
  {
    if ( !loadFields() )
    {
      pkChosen = false;
    }
    else
    {
      determinePrimaryKeyFromUriKeyColumn();
      pkChosen = !mPrimaryKeyAttrs.isEmpty();
    }
  }
  else
  {
    pkChosen = determinePrimaryKeyLocalDatabase();
  }
  if ( !pkChosen )
  {
    QgsMessageLog::logMessage( tr( "Spatial layer has no primary key." ), tr( "Redshift Spatial" ) );
    mValid = false;
    disconnectDb();
    return;
  }

  setNativeTypes( mConnectionRO->nativeTypes() );

  QString key;

  QString delim;
  const auto constMPrimaryKeyAttrs = mPrimaryKeyAttrs;
  for ( int idx : constMPrimaryKeyAttrs )
  {
    key += delim + mAttributeFields.at( idx ).name();
    delim = ',';
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

QgsRedshiftProvider::~QgsRedshiftProvider()
{
  disconnectDb();

  QgsDebugMsgLevel( QStringLiteral( "deconstructing." ), 3 );
}

QgsAbstractFeatureSource *QgsRedshiftProvider::featureSource() const
{
  return new QgsRedshiftFeatureSource( this );
}

QgsRedshiftConn *QgsRedshiftProvider::connectionRO() const
{
  return mConnectionRO;
}

void QgsRedshiftProvider::reloadProviderData()
{
  mShared->setFeaturesCounted( -1 );
  mLayerExtent.setMinimal();
}

QgsRedshiftConn *QgsRedshiftProvider::connectionRW()
{
  if ( !mConnectionRW )
  {
    mConnectionRW = QgsRedshiftConn::connectDb( mUri.connectionInfo( false ), false );
  }
  return mConnectionRW;
}

QString QgsRedshiftProvider::providerKey()
{
  return REDSHIFT_KEY;
}

void QgsRedshiftProvider::disconnectDb()
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

QString QgsRedshiftProvider::storageType() const
{
  return QStringLiteral( "Amazon Redshift database" );
}

QgsFeatureIterator QgsRedshiftProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  if ( !mValid )
  {
    QgsMessageLog::logMessage( tr( "Read attempt on an invalid Redshift data source" ), tr( "Redshift Spatial" ) );
    return QgsFeatureIterator();
  }

  QgsRedshiftFeatureSource *featureSrc = static_cast<QgsRedshiftFeatureSource *>( featureSource() );
  return QgsFeatureIterator( new QgsRedshiftFeatureIterator( featureSrc, true, request ) );
}

QString QgsRedshiftProvider::pkParamWhereClause( int offset, const char *alias ) const
{
  QString whereClause;

  QString aliased;
  if ( alias )
    aliased = QStringLiteral( "%1." ).arg( alias );

  QString delim;
  for ( int i = 0; i < mPrimaryKeyAttrs.size(); i++ )
  {
    int idx = mPrimaryKeyAttrs[i];
    QgsField fld = field( idx );

    whereClause += delim + QStringLiteral( "%3%1=$%2" )
                   .arg( connectionRO()->fieldExpressionForWhereClause( fld ) )
                   .arg( offset++ )
                   .arg( aliased );
    delim = QStringLiteral( " AND " );
  }

  if ( !mSqlWhereClause.isEmpty() )
  {
    if ( !whereClause.isEmpty() )
      whereClause += QLatin1String( " AND " );

    whereClause += '(' + mSqlWhereClause + ')';
  }

  return whereClause;
}

void QgsRedshiftProvider::appendPkParams( QgsFeatureId featureId, QStringList &params ) const
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
      QgsDebugMsg( QStringLiteral( "FAILURE: Key value %1 for feature %2 not found." )
                   .arg( mPrimaryKeyAttrs[i] )
                   .arg( featureId ) );
      params << QStringLiteral( "NULL" );
    }
  }
}

QString QgsRedshiftProvider::whereClause( QgsFeatureId featureId ) const
{
  return QgsRedshiftUtils::whereClause( featureId, mAttributeFields, connectionRO(), mPrimaryKeyAttrs, mShared );
}

QString QgsRedshiftProvider::whereClause( QgsFeatureIds featureIds ) const
{
  return QgsRedshiftUtils::whereClause( featureIds, mAttributeFields, connectionRO(), mPrimaryKeyAttrs, mShared );
}

QString QgsRedshiftUtils::whereClause( QgsFeatureId featureId, const QgsFields &fields, QgsRedshiftConn *conn,
                                       const QList<int> &pkAttrs,
                                       const std::shared_ptr<QgsRedshiftSharedData> &sharedData )
{
  QString whereClause;

  QVariantList pkVals = sharedData->lookupKey( featureId );
  if ( !pkVals.isEmpty() )
  {
    Q_ASSERT( pkVals.size() == pkAttrs.size() );

    QString delim;
    for ( int i = 0; i < pkAttrs.size(); i++ )
    {
      int idx = pkAttrs[i];
      QgsField fld = fields.at( idx );

      whereClause += delim + conn->fieldExpressionForWhereClause( fld, pkVals[i].type() );
      if ( pkVals[i].isNull() )
        whereClause += QLatin1String( " IS NULL" );
      else
        whereClause += '=' + QgsRedshiftConn::quotedValue( pkVals[i] );

      delim = QStringLiteral( " AND " );
    }
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "FAILURE: Key values for feature %1 not found." ).arg( featureId ) );
    whereClause = QStringLiteral( "FALSE" );
  }

  return whereClause;
}

QString QgsRedshiftUtils::whereClause( const QgsFeatureIds &featureIds, const QgsFields &fields, QgsRedshiftConn *conn,
                                       const QList<int> &pkAttrs,
                                       const std::shared_ptr<QgsRedshiftSharedData> &sharedData )
{
  // TODO(marcel): for primary keys which are just one int4 or int8 column use
  // ... IN (...) query

  // complex primary key, need to build up where string
  QStringList whereClauses;
  for ( const QgsFeatureId featureId : std::as_const( featureIds ) )
  {
    const QString &fidWhereClause = whereClause( featureId, fields, conn, pkAttrs, sharedData );

    if ( fidWhereClause != "FALSE" )
      whereClauses << fidWhereClause;
  }
  return whereClauses.isEmpty() ? QString( "FALSE" )
         : whereClauses.join( QStringLiteral( " OR " ) ).prepend( '(' ).append( ')' );
}

QString QgsRedshiftUtils::andWhereClauses( const QString &c1, const QString &c2 )
{
  if ( c1.isEmpty() )
    return c2;
  if ( c2.isEmpty() )
    return c1;

  return QStringLiteral( "(%1) AND (%2)" ).arg( c1, c2 );
}

void QgsRedshiftUtils::replaceInvalidXmlChars( QString &xml )
{
  static const QRegularExpression replaceRe{QStringLiteral( "([\x00-\x08\x0B-\x1F\x7F])" )};
  QRegularExpressionMatchIterator it{replaceRe.globalMatch( xml )};
  while ( it.hasNext() )
  {
    const QRegularExpressionMatch match{it.next()};
    const QChar c{match.captured( 1 ).at( 0 )};
    xml.replace( c, QStringLiteral( "UTF-8[%1]" ).arg( c.unicode() ) );
  }
}

void QgsRedshiftUtils::restoreInvalidXmlChars( QString &xml )
{
  static const QRegularExpression replaceRe{QStringLiteral( R"raw(UTF-8\[(\d+)\])raw" )};
  QRegularExpressionMatchIterator it{replaceRe.globalMatch( xml )};
  while ( it.hasNext() )
  {
    const QRegularExpressionMatch match{it.next()};
    bool ok;
    const ushort code{match.captured( 1 ).toUShort( &ok )};
    if ( ok )
    {
      xml.replace( QStringLiteral( "UTF-8[%1]" ).arg( code ), QChar( code ) );
    }
  }
}

QString QgsRedshiftProvider::filterWhereClause() const
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
             .arg( "st_srid", quotedIdentifier( mGeometryColumn ),
                   mSpatialColType == SctGeography ? "::geography" : "", mRequestedSrid );
    delim = QStringLiteral( " AND " );
  }

  if ( mRequestedGeomType != Qgis::WkbType::Unknown && mRequestedGeomType != mDetectedGeomType )
  {
    where += delim + QgsRedshiftConn::spatialTypeFilter( mGeometryColumn, ( Qgis::WkbType )mRequestedGeomType,
             mSpatialColType == SctGeography );
    delim = QStringLiteral( " AND " );
  }

  return where;
}

void QgsRedshiftProvider::setExtent( QgsRectangle &newExtent )
{
  mLayerExtent.setXMaximum( newExtent.xMaximum() );
  mLayerExtent.setXMinimum( newExtent.xMinimum() );
  mLayerExtent.setYMaximum( newExtent.yMaximum() );
  mLayerExtent.setYMinimum( newExtent.yMinimum() );
}

/**
 * Returns the feature type
 */
Qgis::WkbType QgsRedshiftProvider::wkbType() const
{
  return mRequestedGeomType != Qgis::WkbType::Unknown ? mRequestedGeomType : mDetectedGeomType;
}

QgsLayerMetadata QgsRedshiftProvider::layerMetadata() const
{
  return mLayerMetadata;
}

QgsField QgsRedshiftProvider::field( int index ) const
{
  if ( index < 0 || index >= mAttributeFields.count() )
  {
    QgsMessageLog::logMessage( tr( "FAILURE: Field %1 not found." ).arg( index ), tr( "Redshift Spatial" ) );
    throw PGFieldNotFound();
  }

  return mAttributeFields.at( index );
}

QgsFields QgsRedshiftProvider::fields() const
{
  return mAttributeFields;
}

QString QgsRedshiftProvider::dataComment() const
{
  return mDataComment;
}

struct RSTypeInfo
{
  QString typeName;
  QString typeType;
  int typeLen;
};

// Returns the field type corresponging to the type return by an SQL query
Oid QgsRedshiftProvider::getFldType( Oid typeoid )
{
  Oid fldtyp = typeoid;
  // TODO(marcel): In Redshift "GEOMETRY" is returned with 3999 OID type,
  // so set the field type to the correct one. Remove once fixed.
  if ( fldtyp == 3999 )
    fldtyp = 3000;

  return fldtyp;
}

bool QgsRedshiftProvider::loadFields()
{
  QString sql;
  QString attroidsFilter;

  if ( !mIsQuery )
  {
    QgsDebugMsgLevel( QStringLiteral( "Loading fields for table %1" )
                      .arg( mTableName ), 2 );
    if ( mExternalDatabaseName.isEmpty() )
    {
      // Get the local table description
      sql = QStringLiteral( "SELECT description FROM pg_description WHERE "
                            "objoid=regclass(%1)::oid AND objsubid=0" )
            .arg( quotedValue( mQuery ) );
      QgsRedshiftResult tresult( connectionRO()->PQexec( sql ) );
      if ( tresult.PQntuples() > 0 )
      {
        mDataComment = tresult.PQgetvalue( 0, 0 );
        mLayerMetadata.setAbstract( mDataComment );
      }
    }
  }

  // Populate the field vector for this layer. The field vector contains
  // field name, type, length, and precision (if numeric)
  sql = QStringLiteral( "SELECT * FROM %1 LIMIT 1" ).arg( mQuery );
  // TODO(marcel): LIMIT 1 instead of LIMIT 0 because Redshift doesn't set
  // properly tableoid if we use LIMIT 0

  QgsRedshiftResult result( connectionRO()->PQexec( sql ) );

  QMap<Oid, QMap<int, QString>> fmtFieldTypeMap, descrMap, defValMap, identityMap;
  QMap<Oid, QMap<int, Oid>> attTypeIdMap;
  QMap<Oid, QMap<int, bool>> notNullMap, uniqueMap;
  if ( result.PQnfields() > 0 )
  {
    // Collect attribute oids
    QSet<Oid> attroids;
    for ( int i = 0; i < result.PQnfields(); i++ )
    {
      QgsDebugMsg( QStringLiteral( "column: %1" )
                   .arg( result.PQftype( i ) ) );
      Oid attroid = getFldType( result.PQftype( i ) );
      attroids.insert( attroid );
    }

    // Collect table oids
    QSet<Oid> tableoids;
    for ( int i = 0; i < result.PQnfields(); i++ )
    {
      Oid tableoid = result.PQftable( i );
      if ( tableoid > 0 )
      {
        tableoids.insert( tableoid );
      }
    }
    if ( !tableoids.isEmpty() )
    {
      QStringList tableoidsList;
      const auto constTableoids = tableoids;
      for ( Oid tableoid : constTableoids )
      {
        tableoidsList.append( QString::number( tableoid ) );
      }

      QString tableoidsFilter = '(' + tableoidsList.join( QStringLiteral( "," ) ) + ')';

      // Collect formatted field types
      sql = QStringLiteral( "SELECT attrelid, attnum, "
                            "pg_catalog.format_type(atttypid,atttypmod), "
                            "pg_catalog.col_description(attrelid,attnum), "
                            "pg_catalog.pg_get_expr(adbin,adrelid), atttypid, "
                            "attnotnull::int, indisunique::int,"
                            " (CASE WHEN adsrc LIKE '\"identity\"%' THEN 'a' WHEN adsrc LIKE "
                            "'%default_identity%' THEN 'd' ELSE NULL END)"
                            // identity type, // "d" - if generated by default as identity,
                            // "a" - always generated as identity, " "  - not an identity column
                            " FROM pg_attribute"
                            " LEFT OUTER JOIN pg_attrdef ON attrelid=adrelid AND attnum=adnum"
                            // find unique constraints if present. Text cast required to
                            // handle int2vector comparison. Distinct required as multiple
                            // unique constraints may exist
                            " LEFT OUTER JOIN ( SELECT DISTINCT indrelid, "
                            "textin(int2vectorout(indkey))::text as indkey, indisunique FROM "
                            "pg_index WHERE indisunique ) uniq ON attrelid=indrelid AND "
                            "attnum::text=indkey::text "

                            " WHERE attrelid IN %1" )
            .arg( tableoidsFilter );

      QgsRedshiftResult fmtFieldTypeResult( connectionRO()->PQexec( sql ) );
      for ( int i = 0; i < fmtFieldTypeResult.PQntuples(); ++i )
      {
        Oid attrelid = fmtFieldTypeResult.PQgetvalue( i, 0 ).toUInt();
        int attnum = fmtFieldTypeResult.PQgetvalue( i, 1 ).toInt(); // Int2
        QString formatType = fmtFieldTypeResult.PQgetvalue( i, 2 );
        QString descr = fmtFieldTypeResult.PQgetvalue( i, 3 );
        QString defVal = fmtFieldTypeResult.PQgetvalue( i, 4 );
        Oid attType = fmtFieldTypeResult.PQgetvalue( i, 5 ).toUInt();
        bool attNotNull = fmtFieldTypeResult.PQgetvalue( i, 6 ).toInt();
        bool uniqueConstraint = fmtFieldTypeResult.PQgetvalue( i, 7 ).toInt();
        QString attIdentity = fmtFieldTypeResult.PQgetvalue( i, 8 );

        fmtFieldTypeMap[attrelid][attnum] = formatType;
        attTypeIdMap[attrelid][attnum] = attType;
        descrMap[attrelid][attnum] = descr;
        defValMap[attrelid][attnum] = defVal;
        notNullMap[attrelid][attnum] = attNotNull;
        uniqueMap[attrelid][attnum] = uniqueConstraint;
        identityMap[attrelid][attnum] = attIdentity;

        attroids.insert( attType );
      }
    }

    // Prepare filter for fetching pg_type info
    if ( !attroids.isEmpty() )
    {
      QStringList attroidsList;
      for ( Oid attroid : std::as_const( attroids ) )
      {
        attroidsList.append( QString::number( attroid ) );
      }
      attroidsFilter = QStringLiteral( "WHERE oid in (%1)" ).arg( attroidsList.join( ',' ) );
    }
  }

  // Collect type info
  sql = QStringLiteral( "SELECT oid,typname,typtype,typlen FROM pg_type %1" ).arg( attroidsFilter );
  QgsRedshiftResult typeResult( connectionRO()->PQexec( sql ) );

  QMap<Oid, RSTypeInfo> typeMap;
  for ( int i = 0; i < typeResult.PQntuples(); ++i )
  {
    RSTypeInfo typeInfo = {/* typeName = */ typeResult.PQgetvalue( i, 1 ),
                                            /* typeType = */ typeResult.PQgetvalue( i, 2 ),
                                            /* typeLen = */ typeResult.PQgetvalue( i, 3 ).toInt()
                          };
    QgsDebugMsgLevel( QStringLiteral( "type added: %1 %2 %3 %4" )
                      .arg( typeResult.PQgetvalue( i, 0 ) )
                      .arg( typeInfo.typeName )
                      .arg( typeInfo.typeType )
                      .arg( typeInfo.typeLen ), 2 );
    typeMap.insert( typeResult.PQgetvalue( i, 0 ).toUInt(), typeInfo );
  }

  QSet<QString> fields;
  mAttributeFields.clear();
  mIdentityFields.clear();
  for ( int i = 0; i < result.PQnfields(); i++ )
  {
    QString fieldName = result.PQfname( i );
    if ( fieldName == mGeometryColumn )
      continue;
    Oid tableoid = result.PQftable( i );
    int attnum = result.PQftablecol( i );
    QgsDebugMsgLevel( QStringLiteral( "tableoid: %1, attnum: %2" )
                      .arg( tableoid ).arg( attnum ), 2 );

    QString fieldComment = descrMap[tableoid][attnum];
    QString formattedFieldType;
    Oid fldtyp;

    if ( mExternalDatabaseName.isEmpty() && tableoid > 0 )
    {
      fldtyp = attTypeIdMap[tableoid][attnum];
      formattedFieldType = fmtFieldTypeMap[tableoid][attnum];
    }
    else
    {
      fldtyp = getFldType( result.PQftype( i ) );

      int fldMod = result.PQfmod( i );
      sql = QStringLiteral( "SELECT format_type(%1, %2)" ).arg( fldtyp ).arg( fldMod );
      QgsRedshiftResult fmtFieldModResult( connectionRO()->PQexec( sql ) );
      if ( fmtFieldModResult.PQntuples() > 0 )
      {
        formattedFieldType = fmtFieldModResult.PQgetvalue( 0, 0 );
      }
    }
    QgsDebugMsgLevel( QStringLiteral( "fldtyp: %1" ).arg( fldtyp ), 2 );
    const RSTypeInfo &typeInfo = typeMap.value( fldtyp );

    QString fieldTypeName = typeInfo.typeName;
    QString fieldTType = typeInfo.typeType;
    int fieldSize = typeInfo.typeLen;
    int fieldPrec = 0;

    QVariant::Type fieldType;

    if ( fieldTType == QLatin1String( "b" ) )
    {
      if ( fieldTypeName == QLatin1String( "int8" ) )
      {
        fieldType = QVariant::LongLong;
        fieldSize = -1;
        fieldPrec = 0;
      }
      else if ( fieldTypeName == QLatin1String( "int2" ) || fieldTypeName == QLatin1String( "int4" ) ||
                fieldTypeName == QLatin1String( "oid" ) )
      {
        fieldType = QVariant::Int;
        fieldSize = -1;
        fieldPrec = 0;
      }
      else if ( fieldTypeName == QLatin1String( "float4" ) || fieldTypeName == QLatin1String( "float8" ) )
      {
        fieldType = QVariant::Double;
        fieldSize = -1;
        fieldPrec = 0;
      }
      else if ( fieldTypeName == QLatin1String( "numeric" ) )
      {
        fieldType = QVariant::Double;

        QRegExp re( "numeric\\((\\d+),(\\d+)\\)" );
        if ( re.exactMatch( formattedFieldType ) )
        {
          fieldSize = re.cap( 1 ).toInt();
          fieldPrec = re.cap( 2 ).toInt();
        }
        else
        {
          QgsMessageLog::logMessage(
            tr( "Unexpected formatted field type '%1' for field %2" ).arg( formattedFieldType, fieldName ),
            tr( "Redshift Spatial" ) );
          fieldSize = -1;
          fieldPrec = 0;
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
          QgsMessageLog::logMessage( tr( "Unexpected formatted field type '%1' "
                                         "for field %2. Field ignored." )
                                     .arg( formattedFieldType, fieldName ),
                                     tr( "Redshift Spatial" ) );

          continue;
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
      else if ( fieldTypeName == QLatin1String( "geometry" ) || fieldTypeName == QLatin1String( "geography" ) ||
                fieldTypeName == QLatin1String( "timetz" ) || fieldTypeName == QLatin1String( "timestamptz" ) ||
                fieldTypeName == QLatin1String( "hllsketch" ) )
      {
        // TODO(marcel): discuss how to set the field size in case of geometries
        // and hllsketches
        fieldType = QVariant::String;
        fieldSize = redshiftMaxVarcharLength();
      }
      else if ( fieldTypeName == QLatin1String( "bpchar" ) || fieldTypeName == QLatin1String( "char" ) )
      {
        fieldTypeName = QStringLiteral( "character" );

        fieldType = QVariant::String;

        QRegExp re( "character\\((\\d+)\\)" );
        if ( re.exactMatch( formattedFieldType ) )
        {
          fieldSize = re.cap( 1 ).toInt();
        }
        else
        {
          QgsMessageLog::logMessage( tr( "Unexpected formatted field type '%1' "
                                         "for field %2. Field ignored." )
                                     .arg( formattedFieldType, fieldName ),
                                     tr( "Redshift Spatial" ) );

          continue;
        }
      }
      else if ( fieldTypeName == QLatin1String( "bool" ) )
      {
        // enum
        fieldType = QVariant::Bool;
        fieldSize = -1;
      }
      else
      {
        // be tolerant in case of views: this might be a field used as a key
        const QgsRedshiftProvider::Relkind type = relkind();
        if ( ( type == Relkind::View || type == Relkind::MaterializedView ) &&
             parseUriKey( mUri.keyColumn() ).contains( fieldName ) )
        {
          // Assume it is convertible to text
          fieldType = QVariant::String;
          fieldSize = redshiftMaxVarcharLength();
        }
        else
        {
          QgsMessageLog::logMessage(
            tr( "Field %1 ignored, because of unsupported type: %2" ).arg( fieldName, fieldTypeName ),
            tr( "Redshift Spatial" ) );
          continue;
        }
      }
    }
    else
    {
      QgsMessageLog::logMessage(
        tr( "Field %1 ignored, because of unsupported typtype: %2. Field name: %3" ).arg( fieldName, fieldTType, fieldName ),
        tr( "Redshift Spatial" ) );
      continue;
    }

    if ( fields.contains( fieldName ) )
    {
      QgsMessageLog::logMessage( tr( "Duplicate field %1 found\n" ).arg( fieldName ), tr( "Redshift Spatial" ) );
      // In case of read-only query layers we can safely ignore the issue
      if ( !mIsQuery )
      {
        return false;
      }
    }

    fields << fieldName;

    mAttrPalIndexName.insert( i, fieldName );
    mDefaultValues.insert( mAttributeFields.size(), defValMap[tableoid][attnum] );

    QgsField newField =
      QgsField( fieldName, fieldType, fieldTypeName, fieldSize, fieldPrec, fieldComment, QVariant::Invalid );

    QgsFieldConstraints constraints;
    if ( notNullMap[tableoid][attnum] || ( mPrimaryKeyAttrs.size() == 1 && mPrimaryKeyAttrs[0] == i ) ||
         !identityMap[tableoid][attnum].isEmpty() )
      constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull,
                                 QgsFieldConstraints::ConstraintOriginProvider );
    if ( uniqueMap[tableoid][attnum] || ( mPrimaryKeyAttrs.size() == 1 && mPrimaryKeyAttrs[0] == i ) ||
         identityMap[tableoid][attnum] == "a" )
      constraints.setConstraint( QgsFieldConstraints::ConstraintUnique,
                                 QgsFieldConstraints::ConstraintOriginProvider );
    newField.setConstraints( constraints );

    if ( !identityMap[tableoid][attnum].isEmpty() )
      mIdentityFields.insert( mAttributeFields.size(), identityMap[tableoid][attnum][0].toLatin1() );

    mAttributeFields.append( newField );
  }

  setEditorWidgets();

  return true;
}

void QgsRedshiftProvider::setEditorWidgets()
{
  if ( !tableExists( *connectionRO(), EDITOR_WIDGET_STYLES_TABLE ) )
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
  // CREATE TABLE qgis_editor_widget_styles (schema_name TEXT NOT NULL,
  // table_name TEXT NOT NULL, field_name TEXT NOT NULL,
  //                                         type TEXT NOT NULL, config TEXT,
  //                                         PRIMARY KEY(schema_name,
  //                                         table_name, field_name));
  const QString sql =
    QStringLiteral( "SELECT field_name, type, config "
                    "FROM %1 WHERE schema_name = %2 "
                    "AND table_name = %3 "
                    "AND field_name IN ( %4 )" )
    .arg( EDITOR_WIDGET_STYLES_TABLE, quotedValue( mSchemaName ), quotedValue( mTableName ), quotedFnames.join( "," ) );
  QgsRedshiftResult result( connectionRO()->PQexec( sql ) );
  for ( int i = 0; i < result.PQntuples(); ++i )
  {
    if ( result.PQgetisnull( i, 2 ) )
      continue; // config can be null and it's OK

    const QString &configTxt = result.PQgetvalue( i, 2 );
    const QString &type = result.PQgetvalue( i, 1 );
    const QString &fname = result.PQgetvalue( i, 0 );
    QVariantMap config;
    QDomDocument doc;
    if ( doc.setContent( configTxt ) )
    {
      config = QgsXmlUtils::readVariant( doc.documentElement() ).toMap();
    }
    else
    {
      QgsMessageLog::logMessage(
        tr( "Cannot parse widget configuration for field %1.%2.%3\n" ).arg( mSchemaName, mTableName, fname ),
        tr( "Redshift Spatial" ) );
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

bool QgsRedshiftProvider::setPermsAndCapabilitiesForQuery()
{
  // Check if the sql is a select query.
  if ( !mQuery.startsWith( '(' ) && !mQuery.endsWith( ')' ) )
  {
    QgsMessageLog::logMessage( tr( "The custom query is not a select query." ), tr( "Redshift Spatial" ) );
    return false;
  }

  // Get a new alias for the subquery.
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

  // Convert the custom query into a subquery.
  mQuery = QStringLiteral( "%1 AS %2" ).arg( mQuery, quotedIdentifier( alias ) );

  QString sql = QStringLiteral( "SELECT * FROM %1 LIMIT 1" ).arg( mQuery );

  QgsRedshiftResult testAccess( connectionRO()->PQexec( sql ) );
  if ( testAccess.PQresultStatus() != PGRES_TUPLES_OK )
  {
    QgsMessageLog::logMessage( tr( "Unable to execute the query.\nThe error message from the "
                                   "database was:\n%1.\nSQL: %2" )
                               .arg( testAccess.PQresultErrorMessage(), sql ),
                               tr( "Redshift Spatial" ) );
    return false;
  }

  if ( !mSelectAtIdDisabled )
  {
    mEnabledCapabilities |= QgsVectorDataProvider::SelectAtId;
  }
  return true;
}

bool QgsRedshiftProvider::setPermsAndCapabilitiesForDatashare()
{

  // Datashares are read-only in Redshift. Set all modification related flags
  // to zero.
  mEnabledCapabilities &=
    ~( QgsVectorDataProvider::DeleteFeatures |
       QgsVectorDataProvider::FastTruncate |
       QgsVectorDataProvider::ChangeAttributeValues |
       QgsVectorDataProvider::ChangeGeometries |
       QgsVectorDataProvider::AddFeatures |
       QgsVectorDataProvider::AddAttributes |
       QgsVectorDataProvider::DeleteAttributes |
       QgsVectorDataProvider::RenameAttributes );
  return true;
}

bool QgsRedshiftProvider::setPermsAndCapabilitiesForLocalDatabase()
{
  QString sql = QString( "SELECT "
                         "has_table_privilege(%1,'DELETE'),"
                         "has_table_privilege(%1,'UPDATE'),"
                         "has_table_privilege(%1,'UPDATE'),"
                         "has_table_privilege(%1,'INSERT'),"
                         "current_schema()" )
                .arg( quotedValue( mQuery ) );

  QgsRedshiftResult testAccess( connectionRO()->PQexec( sql ) );
  if ( testAccess.PQresultStatus() != PGRES_TUPLES_OK )
  {
    QgsMessageLog::logMessage( tr( "Unable to determine table access privileges for the %1 "
                                   "relation.\nThe error message from the database was:\n%2.\nSQL: "
                                   "%3" )
                               .arg( mQuery, testAccess.PQresultErrorMessage(), sql ),
                               tr( "Redshift Spatial" ) );
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
    mEnabledCapabilities |= QgsVectorDataProvider::AddFeatures;
  }

  if ( mSchemaName.isEmpty() )
    mSchemaName = testAccess.PQgetvalue( 0, 4 );

  sql = QString( "SELECT 1 FROM pg_class,pg_namespace WHERE "
                 "pg_class.relnamespace=pg_namespace.oid AND "
                 "%3 AND "
                 "relname=%1 AND nspname=%2" )
        .arg( quotedValue( mTableName ), quotedValue( mSchemaName ), "pg_get_userbyid(relowner)=current_user" );

  testAccess = connectionRO()->PQexec( sql );
  if ( testAccess.PQresultStatus() == PGRES_TUPLES_OK && testAccess.PQntuples() == 1 )
  {
    mEnabledCapabilities |= QgsVectorDataProvider::AddAttributes | QgsVectorDataProvider::DeleteAttributes |
                            QgsVectorDataProvider::RenameAttributes;
  }

  return true;
}

// will disconnect on 'false' returned from this function, hence the transient
// state set with this function will be discarded with the provider.
bool QgsRedshiftProvider::setPermsAndCapabilities()
{
  QgsDebugMsgLevel( QStringLiteral( "Checking for permissions on the relation" ), 2 );

  bool isSuccessul = true;
  if ( !mIsQuery )
  {
    // Check that we can read from the table (i.e., we have select permission).
    QString sql = QStringLiteral( "SELECT * FROM %1 LIMIT 1" ).arg( mQuery );
    QgsRedshiftResult testAccess( connectionRO()->PQexec( sql ) );
    if ( testAccess.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsMessageLog::logMessage( tr( "Unable to access the %1 relation.\nThe error message from the "
                                     "database was:\n%2.\nSQL: %3" )
                                 .arg( mQuery, testAccess.PQresultErrorMessage(), sql ),
                                 tr( "Redshift Spatial" ) );
      return false;
    }

    if ( !mSelectAtIdDisabled )
    {
      mEnabledCapabilities |= QgsVectorDataProvider::SelectAtId;
    }

    if ( !mExternalDatabaseName.isEmpty() )
    {
      isSuccessul = setPermsAndCapabilitiesForDatashare();
    }
    else
    {
      isSuccessul = setPermsAndCapabilitiesForLocalDatabase();
    }
  }
  else
  {
    isSuccessul = setPermsAndCapabilitiesForQuery();
  }

  if ( ( mEnabledCapabilities & QgsVectorDataProvider::ChangeGeometries ) &&
       ( mEnabledCapabilities & QgsVectorDataProvider::ChangeAttributeValues ) )
  {
    mEnabledCapabilities |= QgsVectorDataProvider::ChangeFeatures;
  }

  // Supports geometry simplification on provider side.
  mEnabledCapabilities |= QgsVectorDataProvider::SimplifyGeometries;

  // Supports layer metadata.
  mEnabledCapabilities |= QgsVectorDataProvider::ReadLayerMetadata;

  return isSuccessul;
}

bool QgsRedshiftProvider::determinePrimaryKeyLocalDatabase()
{
  if ( !loadFields() )
  {
    return false;
  }

  // Check to see if there is an unique index on the relation, which
  // can be used as a key into the table. Primary keys are always
  // unique indices, so we catch them as well.

  QString sql;

  if ( !mIsQuery )
  {
    // Handle case in which the layer is a table/view.

    // Get indices of the table. Primary keys and unique columns are always considered an index in PostgreSQL.
    // We just need 1 and we prioritize primary keys over unique columns.
    sql = QStringLiteral( "SELECT indexrelid FROM pg_index WHERE indrelid=%1::regclass AND "
                          "(indisprimary OR indisunique) ORDER BY CASE WHEN indisprimary "
                          "THEN 1 ELSE 2 END LIMIT 1" )
          .arg( quotedValue( mQuery ) );
    QgsDebugMsgLevel( QStringLiteral( "Retrieving first primary or unique index: %1" ).arg( sql ), 2 );

    QgsRedshiftResult res( connectionRO()->PQexec( sql ) );
    QgsDebugMsgLevel( QStringLiteral( "Got %1 rows." ).arg( res.PQntuples() ), 2 );

    QStringList log;

    // No primary or unique indices found.
    if ( res.PQntuples() == 0 )
    {
      QgsDebugMsgLevel( QStringLiteral( "Relation has no primary key -- investigating alternatives" ), 2 );

      const QgsRedshiftProvider::Relkind type = relkind();

      if ( type == Relkind::OrdinaryTable )
      {
        QgsDebugMsgLevel( QStringLiteral( "Relation is a table. Checking to see "
                                          "if it has an identity column." ), 2 );

        mPrimaryKeyAttrs.clear();

        // If there is a generated identity on the table, use that instead,
        for ( int idx = 0; idx < mAttributeFields.size(); idx++ )
          if ( isIdentityField( idx ) )
          {
            mPrimaryKeyAttrs << idx;
            break;
          }

        if ( mPrimaryKeyAttrs.isEmpty() )
        {
          QgsMessageLog::logMessage( tr( "The table has no column suitable for use as a key. QGIS "
                                         "requires a primary key or an identity column for tables." ),
                                     tr( "Redshift Spatial" ) );
        }
      }
      else if ( type == Relkind::View || type == Relkind::MaterializedView )
      {
        //
        determinePrimaryKeyFromUriKeyColumn();
      }
      else
      {
        const QMetaEnum metaEnum( QMetaEnum::fromType<Relkind>() );
        QString typeName = metaEnum.valueToKey( type );
        QgsMessageLog::logMessage( tr( "Unexpected relation type '%1'." ).arg( typeName ), tr( "Redshift Spatial" ) );
      }
    }
    else
    {
      // Have a primary key or unique constraint.
      QString indrelid = res.PQgetvalue( 0, 0 );

      sql = QStringLiteral( "SELECT attname,attnotnull FROM pg_index,pg_attribute "
                            "WHERE indexrelid=%1 AND indrelid=attrelid AND "
                            "pg_attribute.attnum=any(string_to_array(textin("
                            "int2vectorout(pg_index.indkey)), ' '));" )
            .arg( indrelid );

      QgsDebugMsgLevel( "Retrieving key columns: " + sql, 2 );
      res = connectionRO()->PQexec( sql );
      QgsDebugMsgLevel( QStringLiteral( "Got %1 rows." ).arg( res.PQntuples() ), 2 );

      bool mightBeNull = false;
      QString primaryKey;
      QString delim;

      for ( int i = 0; i < res.PQntuples(); i++ )
      {
        QString name = res.PQgetvalue( i, 0 );
        if ( res.PQgetvalue( i, 1 ).startsWith( 'f' ) )
        {
          QgsMessageLog::logMessage( tr( "Unique column '%1' doesn't have a NOT NULL constraint." ).arg( name ),
                                     tr( "Redshift Spatial" ) );
          mightBeNull = true;
        }

        primaryKey += delim + quotedIdentifier( name );
        delim = ',';

        // Check if the column specified in the primary key string actually exists in the table/view.
        int idx = fieldNameIndex( name );
        if ( idx == -1 )
        {
          QgsDebugMsgLevel( "Skipping " + name, 2 );
          continue;
        }
        QgsField fld = mAttributeFields.at( idx );

        mPrimaryKeyAttrs << idx;
      }

      if ( mightBeNull && !mUseEstimatedMetadata && !uniqueData( primaryKey ) )
      {
        QgsMessageLog::logMessage( tr( "Ignoring key candidate because of NULL values or inheritance" ),
                                   tr( "Redshift Spatial" ) );
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
    // Primary keys are unique, not null.
    QgsFieldConstraints constraints = mAttributeFields.at( mPrimaryKeyAttrs[0] ).constraints();
    constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
    constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull,
                               QgsFieldConstraints::ConstraintOriginProvider );
    mAttributeFields[mPrimaryKeyAttrs[0]].setConstraints( constraints );
  }

  mValid &= !mPrimaryKeyAttrs.isEmpty();

  return mValid;
}

QStringList QgsRedshiftProvider::parseUriKey( const QString &key )
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

// For view and query layers, the primary key should be specified in the URI at layer creation.
// This function parses the primary key from the URI and checks its validity.
void QgsRedshiftProvider::determinePrimaryKeyFromUriKeyColumn()
{
  QString primaryKey = mUri.keyColumn();

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

    for ( const QString &col : cols )
    {
      int idx = fieldNameIndex( col );
      if ( idx < 0 )
      {
        QgsMessageLog::logMessage( tr( "Key field '%1' for view/query not found." ).arg( col ),
                                   tr( "Redshift Spatial" ) );
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

      if ( !mUseEstimatedMetadata && !unique )
      {
        QgsMessageLog::logMessage( tr( "Primary key field '%1' for view/query not unique." ).arg( primaryKey ),
                                   tr( "Redshift Spatial" ) );
        mPrimaryKeyAttrs.clear();
      }
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Keys for view/query undefined." ), tr( "Redshift Spatial" ) );
    }
  }
  else
  {
    QgsMessageLog::logMessage( tr( "No key field for view/query given." ), tr( "Redshift Spatial" ) );
  }
}

bool QgsRedshiftProvider::uniqueData( const QString &quotedColNames )
{
  // Check to see if the given columns contain unique data
  QString sql = QStringLiteral( "SELECT (SELECT count(*) FROM (SELECT distinct "
                                "%1 FROM %2%3))=(SELECT count(*) FROM %2%3)" )
                .arg( quotedColNames, mQuery, filterWhereClause() );

  QgsRedshiftResult unique( connectionRO()->PQexec( sql ) );

  if ( unique.PQresultStatus() != PGRES_TUPLES_OK )
  {
    pushError( unique.PQresultErrorMessage() );
    return false;
  }
  return unique.PQntuples() == 1 && unique.PQgetvalue( 0, 0 ).startsWith( 't' );
}

// Returns the list of unique values of an attribute.
QSet<QVariant> QgsRedshiftProvider::uniqueValues( int index, int limit ) const
{
  QSet<QVariant> uniqueValues;

  try
  {
    // get the field name
    QgsField fld = field( index );
    QString sql = QStringLiteral( "SELECT DISTINCT %1 FROM %2" ).arg( quotedIdentifier( fld.name() ), mQuery );

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

    QgsRedshiftResult res( connectionRO()->PQexec( sql ) );
    if ( res.PQresultStatus() == PGRES_TUPLES_OK )
    {
      for ( int i = 0; i < res.PQntuples(); i++ )
        uniqueValues.insert( convertValue( fld.type(), res.PQgetvalue( i, 0 ), fld.typeName() ) );
    }
  }
  catch ( PGFieldNotFound )
  {
  }
  return uniqueValues;
}

QStringList QgsRedshiftProvider::uniqueStringsMatching( int index, const QString &substring, int limit,
    QgsFeedback *feedback ) const
{
  QStringList results;

  try
  {
    // get the field name
    QgsField fld = field( index );
    QString sql = QStringLiteral( "SELECT DISTINCT %1 FROM %2 WHERE" ).arg( quotedIdentifier( fld.name() ), mQuery );

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

    QgsRedshiftResult res( connectionRO()->PQexec( sql ) );
    if ( res.PQresultStatus() == PGRES_TUPLES_OK )
    {
      for ( int i = 0; i < res.PQntuples(); i++ )
      {
        results << ( convertValue( fld.type(), res.PQgetvalue( i, 0 ), fld.typeName() ) ).toString();
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

// Returns the maximum/minimum value of an attribute.
QVariant QgsRedshiftProvider::extremeValue( int index, const QString &func ) const
{
  try
  {
    // Get the field name.
    QgsField fld = field( index );
    QString sql = QStringLiteral( "SELECT %3(%1) AS %1 FROM %2" ).arg( quotedIdentifier( fld.name() ), mQuery, func );

    if ( !mSqlWhereClause.isEmpty() )
    {
      sql += QStringLiteral( " WHERE %1" ).arg( mSqlWhereClause );
    }

    sql = QStringLiteral( "SELECT %1 FROM (%2)" ).arg( connectionRO()->fieldExpression( fld ), sql );

    QgsRedshiftResult res( connectionRO()->PQexec( sql ) );

    return convertValue( fld.type(), res.PQgetvalue( 0, 0 ), fld.typeName() );
  }
  catch ( PGFieldNotFound )
  {
    return QVariant( QString() );
  }
}


// Returns the maximum value of an attribute
QVariant QgsRedshiftProvider::maximumValue( int index ) const
{
  return extremeValue( index, QStringLiteral( "max" ) );
}


// Returns the minimum value of an attribute
QVariant QgsRedshiftProvider::minimumValue( int index ) const
{
  return extremeValue( index, QStringLiteral( "min" ) );
}


bool QgsRedshiftProvider::isValid() const
{
  return mValid;
}

QString QgsRedshiftProvider::defaultValueClause( int fieldId ) const
{

  if ( isIdentityField( fieldId ) )
    return "DEFAULT";

  QString defVal = mDefaultValues.value( fieldId, QString() );
  if ( !defVal.isEmpty() )
    return defVal;
  else
    return QString();
}

QVariant QgsRedshiftProvider::defaultValue( int fieldId ) const
{
  QString defVal = mDefaultValues.value( fieldId, QString() );

  if ( isIdentityField( fieldId ) )
    return QVariant();

  if ( providerProperty( EvaluateDefaultValues, false ).toBool() && !defVal.isEmpty() )
  {
    QgsField fld = field( fieldId );

    QgsRedshiftResult res( connectionRO()->PQexec( QStringLiteral( "SELECT %1" ).arg( defVal ) ) );

    if ( res.result() )
    {
      return convertValue( fld.type(), res.PQgetvalue( 0, 0 ), fld.typeName() );
    }
    else
    {
      pushError( tr( "Could not execute query" ) );
      return QVariant();
    }
  }

  return QVariant();
}

bool QgsRedshiftProvider::skipConstraintCheck( int, QgsFieldConstraints::Constraint,
    const QVariant & ) const
{
  return true;
}

QString QgsRedshiftProvider::paramValue( const QString &fieldValue, const QString &defaultValue ) const
{
  if ( fieldValue.isNull() )
    return QString();

  if ( fieldValue == defaultValue && !defaultValue.isNull() )
  {
    QgsRedshiftResult result( connectionRO()->PQexec( QStringLiteral( "SELECT %1" ).arg( defaultValue ) ) );
    if ( result.PQresultStatus() != PGRES_TUPLES_OK )
      throw PGException( result );

    return result.PQgetvalue( 0, 0 );
  }

  return fieldValue;
}

QString QgsRedshiftProvider::geomParam( int offset ) const
{
  QString geometry;

  bool forceMulti = QgsWkbTypes::isMultiType( wkbType() );

  if ( forceMulti )
    geometry += "st_multi(";

  geometry += QStringLiteral( "st_geomfromwkb($%1,%2)" )
              .arg( offset )
              .arg( mRequestedSrid.isEmpty() ? mDetectedSrid : mRequestedSrid );

  if ( forceMulti )
    geometry += ")";

  return geometry;
}

bool QgsRedshiftProvider::isIdentityField( int idx ) const
{
  return mIdentityFields[idx] == 'a' || mIdentityFields[idx] == 'd';
}

QStringList QgsRedshiftProvider::getValuesRowForInsert( QgsFeature &f, const QHash<int, QVariant> &defVals, const QVector<int> &insertCols )
{
  QStringList values;

  const QgsAttributes &attributevec = f.attributes();
  // Go through all the attributes of the feature.
  for ( int idx : insertCols )
  {
    QVariant v = attributevec.value( idx, QVariant() );
    QString attributeValue;
    // If the value of this attribute is the default value clause or is NULL and
    // is an identity field.
    if ( qgsVariantEqual( v, defaultValueClause( idx ) ) ||
         ( v.isNull() && isIdentityField( idx ) ) )
    {
      if ( isIdentityField( idx ) )
        attributeValue = "DEFAULT";
      else
      {
        attributeValue = quotedValue( defVals[idx] );
        f.setAttribute( idx,  defVals[idx] );
      }
    }
    else
    {
      QString fieldTypeName = field( idx ).typeName();
      if ( fieldTypeName == QLatin1String( "geometry" ) )
      {
        attributeValue = QStringLiteral( "st_geomfromewkt(%1)" ).arg( quotedValue( v.toString() ) );
      }
      else if ( fieldTypeName == QLatin1String( "geography" ) )
      {
        attributeValue = QStringLiteral( "st_geographyfromewkt(%2)" ).arg( quotedValue( v.toString() ) );
      }
      else
        attributeValue = quotedValue( v );

    }

    values << attributeValue;
  }
  return values;
}

bool QgsRedshiftProvider::addFeatures( QgsFeatureList &flist, Flags flags )
{
  if ( !( mEnabledCapabilities & QgsVectorDataProvider::AddFeatures ) ||
       mIsQuery )
  {
    QgsDebugMsgLevel( QStringLiteral( "AddFeatures is not supported." ), 2 );
    return false;
  }

  if ( flist.isEmpty() )
    return true;

  QgsRedshiftConn *conn = connectionRW();
  if ( !conn )
  {
    return false;
  }

  conn->lock();

  bool returnvalue = true;

  try
  {
    conn->begin();

    // Prepare the INSERT and RETURNING columns of the statement.
    QString insertCols, returnCols;
    QString delimInsert, delimReturn;

    // Default values for fields that have a default value.
    QHash<int, QVariant> defVals;
    QVector<int> insertColsIdx;
    QVector<int> returnColsIdx;

    if ( !mGeometryColumn.isNull() )
    {
      insertCols += quotedIdentifier( mGeometryColumn );
      delimInsert = ',';
    }

    for ( int idx = 0; idx < mAttributeFields.count(); ++idx )
    {
      const QgsField &fld = field( idx );

      if ( fld.name() == mGeometryColumn )
        continue;

      if ( mIdentityFields[idx] != 'a' )
      {
        insertCols += delimInsert + quotedIdentifier( fld.name() );
        delimInsert = ',';
        insertColsIdx.push_back( idx );
      }
      else
      {
        // Check that user hasn't set a non-default value for identity columns.
        // We have to return an warn user in this case that the value will be overwritten.
        static bool warn_user_insert_identity_col = true;
        for ( int i = 0; i < flist.size(); i++ )
        {
          QVariant v = flist[i].attributes().value( idx, QVariant() );

          if ( !v.isNull() && !qgsVariantEqual( v, defaultValueClause( idx ) ) )
          {
            if ( warn_user_insert_identity_col )
            {
              QgsMessageLog::logMessage( tr( "Trying to insert value into identity column. Value will be ignored" ), tr( "Redshift Spatial" ) );
              warn_user_insert_identity_col = false;
            }
          }
        }

      }

      if ( isIdentityField( idx ) )
      {
        returnCols += delimReturn + quotedIdentifier( fld.name() );
        delimReturn = ',';
        returnColsIdx.push_back( idx );
      }

      if ( !defaultValueClause( idx ).isEmpty() && !isIdentityField( idx ) )
      {
        QgsRedshiftResult result( connectionRO()->PQexec( QStringLiteral( "SELECT %1" ).arg( defaultValueClause( idx ) ) ) );
        if ( result.PQresultStatus() != PGRES_TUPLES_OK )
          throw PGException( result );

        defVals[idx] = convertValue( fld.type(), result.PQgetvalue( 0, 0 ), fld.typeName() );
      }
    }

    static const int max_rows_per_insert = 128; // max rows to insert per insert statement

    for ( int idx = 0; idx < flist.size(); idx += max_rows_per_insert )
    {
      QString insertStmt, delim;

      for ( int i = idx; i < idx + max_rows_per_insert && i < flist.size(); i++ )
      {
        QgsFeature &feature = flist[i];
        QStringList params;
        if ( !mGeometryColumn.isNull() )
        {
          appendGeomParam( feature.geometry(), params );
          params[0] = geomParam( 0 ).replace( QString( "$0" ), quotedValue( params[0] ) );
        }

        params.append( getValuesRowForInsert( feature, defVals, insertColsIdx ) );
        QString insertValues = "(" + params.join( ',' ) + ")";
        insertStmt += delim + insertValues;
        delim = ",";
      }
      bool query_failed = false;
      QgsRedshiftResult result;
      if ( returnCols.isEmpty() )
      {
        insertStmt = QStringLiteral( "INSERT INTO %1(%2) VALUES %3" )
                     .arg( mQuery, insertCols, insertStmt );

        result = conn->PQexec( insertStmt );
        if ( result.PQresultStatus() != PGRES_COMMAND_OK )
        {
          QgsDebugMsgLevel( QStringLiteral( "Inserting into table %1 failed." )
                            .arg( mQuery ),
                            2 );
          query_failed = true;
        }
      }
      else
      {
        // Use staging table to return generated keys.
        const QString stagingTableName = QStringLiteral( "staging_%1_%2" )
                                         // Transform remote tables e.g. "db"."schema"."table" to db_schema_table
                                         .arg( QString( mQuery ).remove( "\"" ).replace( ".", "_" ) )
                                         .arg( QString::number( QRandomGenerator::system()->generate() ) );
        QgsDebugMsgLevel( QStringLiteral( "Staging table %1" )
                          .arg( stagingTableName ), 2 );

        const QString createTempTableStmt = QStringLiteral( "CREATE TEMP TABLE %1 (like %2 including defaults)" )
                                            .arg( stagingTableName )
                                            .arg( mQuery );

        const QString selectNewlyGeneratedKeys = QStringLiteral( "SELECT %1 FROM %2 ORDER BY %1" )
            .arg( returnCols )
            .arg( stagingTableName );
        insertStmt = QStringLiteral( "INSERT INTO %1(%2) VALUES %3" )
                     .arg( stagingTableName, insertCols, insertStmt );
        const QString insertFromStagingStmt = QStringLiteral( "INSERT INTO %1(%2) SELECT %2 FROM %3" )
                                              .arg( mQuery )
                                              .arg( insertCols )
                                              .arg( stagingTableName );

        result = conn->PQexec( createTempTableStmt );
        if ( result.PQresultStatus() != PGRES_COMMAND_OK )
        {
          QgsDebugMsgLevel( QStringLiteral( "Creating staging table %1 failed." )
                            .arg( stagingTableName ), 2 );
          query_failed = true;
        }

        result = conn->PQexec( insertStmt );
        if ( result.PQresultStatus() != PGRES_COMMAND_OK )
        {
          QgsDebugMsgLevel( QStringLiteral( "Inserting into staging table %1 failed." )
                            .arg( stagingTableName ),
                            2 );
          query_failed = true;
        }

        QgsRedshiftResult insertionResult( conn->PQexec( selectNewlyGeneratedKeys ) );
        if ( insertionResult.PQresultStatus() != PGRES_TUPLES_OK )
        {
          QgsDebugMsgLevel( QStringLiteral( "Selecting in staging table %1 failed." )
                            .arg( stagingTableName ),
                            2 );
          result = insertionResult;
          query_failed = true;
        }

        result = conn->PQexec( insertFromStagingStmt );
        if ( result.PQresultStatus() != PGRES_COMMAND_OK )
        {
          QgsDebugMsgLevel( QStringLiteral( "Insert to table %1 from staging table %2" )
                            .arg( mQuery )
                            .arg( stagingTableName ),
                            2 );
          query_failed = true;
        }

        // Update feature values for the identity fields if not a fast insert.
        if ( !query_failed && !( flags & QgsFeatureSink::FastInsert ) )
        {
          for ( int i = idx; i < idx + max_rows_per_insert && i < flist.size(); i++ )
          {

            QgsFeature &feature = flist[i];
            int attr_pos = 0;
            for ( int col_idx : returnColsIdx )
            {

              const QgsField fld = mAttributeFields.at( col_idx );
              feature.setAttribute( col_idx, convertValue( fld.type(),
                                    insertionResult.PQgetvalue( i - idx, attr_pos++ ), fld.typeName() ) );

            }
          }
        }
      }
      if ( query_failed )
      {
        conn->rollback();
        throw PGException( result );
      }
    }
    if ( !( flags & QgsFeatureSink::FastInsert ) )
    {
      // update feature ids
      for ( QgsFeatureList::iterator features = flist.begin(); features != flist.end(); ++features )
      {
        QgsAttributes attrs = features->attributes();

        QVariantList primaryKeyVals;

        const auto constMPrimaryKeyAttrs = mPrimaryKeyAttrs;
        for ( int idx : constMPrimaryKeyAttrs )
        {
          primaryKeyVals << attrs.at( idx );
        }

        features->setId( mShared->lookupFid( primaryKeyVals ) );
      }
    }

    returnvalue &= conn->commit();
    QgsDebugMsgLevel( QStringLiteral( "commit: %1 new features: %2" )
                      .arg( returnvalue )
                      .arg( flist.size() ), 4 );

    if ( returnvalue )
      mShared->addFeaturesCounted( flist.size() );
  }
  catch ( PGException &e )
  {
    pushError( tr( "Redshift error while adding features: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  conn->unlock();
  return returnvalue;
}

bool QgsRedshiftProvider::deleteFeatures( const QgsFeatureIds &ids )
{
  if ( ids.isEmpty() )
    return true;

  bool returnvalue = true;

  if ( mIsQuery )
  {
    QgsDebugMsg( QStringLiteral( "Cannot delete features (is a query)" ) );
    return false;
  }

  QgsRedshiftConn *conn = connectionRW();
  if ( !conn )
  {
    return false;
  }
  conn->lock();

  try
  {
    conn->begin();

    QgsFeatureIds chunkIds;
    const QgsFeatureIds::const_iterator lastId = --ids.end();
    for ( QgsFeatureIds::const_iterator it = ids.begin(); it != ids.end(); ++it )
    {
      // create chunks of fids to delete, the last chunk may be smaller
      chunkIds.insert( *it );
      if ( chunkIds.size() < 5000 && it != lastId )
        continue;

      const QString sql = QStringLiteral( "DELETE FROM %1 WHERE %2" ).arg( mQuery, whereClause( chunkIds ) );
      QgsDebugMsgLevel( "delete sql: " + sql, 2 );

      // send DELETE statement and do error handling
      QgsRedshiftResult result( conn->PQexec( sql ) );
      if ( result.PQresultStatus() != PGRES_COMMAND_OK && result.PQresultStatus() != PGRES_TUPLES_OK )
        throw PGException( result );

      for ( QgsFeatureIds::const_iterator chunkIt = chunkIds.begin(); chunkIt != chunkIds.end(); ++chunkIt )
      {
        mShared->removeFid( *chunkIt );
      }
      chunkIds.clear();
    }

    returnvalue &= conn->commit();

    mShared->addFeaturesCounted( -ids.size() );
  }
  catch ( PGException &e )
  {
    pushError( tr( "Redshift error while deleting features: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  conn->unlock();
  return returnvalue;
}

bool QgsRedshiftProvider::truncate()
{
  bool returnvalue = true;

  if ( mIsQuery )
  {
    QgsDebugMsg( QStringLiteral( "Cannot truncate (is a query)" ) );
    return false;
  }

  QgsRedshiftConn *conn = connectionRW();
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

    // send truncate statement and do error handling
    QgsRedshiftResult result( conn->PQexec( sql ) );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK && result.PQresultStatus() != PGRES_TUPLES_OK )
      throw PGException( result );

    returnvalue &= conn->commit();

    if ( returnvalue )
    {
      mShared->clear();
    }
  }
  catch ( PGException &e )
  {
    pushError( tr( "Redshift error while truncating: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  conn->unlock();
  return returnvalue;
}

bool QgsRedshiftProvider::addAttributes( const QList<QgsField> &attributes )
{
  bool returnvalue = true;

  if ( mIsQuery )
    return false;

  if ( attributes.isEmpty() )
    return true;

  QgsRedshiftConn *conn = connectionRW();
  if ( !conn )
  {
    return false;
  }
  conn->lock();

  try
  {
    conn->begin();

    for ( QList<QgsField>::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter )
    {
      QString sql = QStringLiteral( "ALTER TABLE %1 " ).arg( mQuery );
      QString type = iter->typeName();
      if ( type == QLatin1String( "char" ) || type == QLatin1String( "varchar" ) )
      {
        if ( iter->length() > 0 )
          type = QStringLiteral( "%1(%2)" ).arg( type ).arg( iter->length() );
      }
      else if ( type == QLatin1String( "numeric" ) || type == QLatin1String( "decimal" ) )
      {
        if ( iter->length() > 0 && iter->precision() > 0 )
          type = QStringLiteral( "%1(%2,%3)" ).arg( type ).arg( iter->length() ).arg( iter->precision() );
      }
      sql.append( QStringLiteral( "ADD COLUMN %1 %2" ).arg( quotedIdentifier( iter->name() ), type ) );

      // send sql statement and do error handling
      QgsRedshiftResult result( conn->PQexec( sql ) );
      if ( result.PQresultStatus() != PGRES_COMMAND_OK )
        throw PGException( result );
    }

    for ( QList<QgsField>::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter )
    {
      if ( !iter->comment().isEmpty() )
      {
        QString sql = QStringLiteral( "COMMENT ON COLUMN %1.%2 IS %3" )
                      .arg( mQuery, quotedIdentifier( iter->name() ), quotedValue( iter->comment() ) );
        QgsRedshiftResult result( conn->PQexec( sql ) );
        if ( result.PQresultStatus() != PGRES_COMMAND_OK )
          throw PGException( result );
      }
    }

    returnvalue &= conn->commit();
  }
  catch ( PGException &e )
  {
    pushError( tr( "Redshift error while adding attributes: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  loadFields();
  conn->unlock();
  return returnvalue;
}

bool QgsRedshiftProvider::deleteAttributes( const QgsAttributeIds &ids )
{
  bool returnvalue = true;

  if ( mIsQuery )
    return false;

  QgsRedshiftConn *conn = connectionRW();
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
      QString sql = QStringLiteral( "ALTER TABLE %1 DROP COLUMN %2" ).arg( mQuery, quotedIdentifier( column ) );

      // send sql statement and do error handling
      QgsRedshiftResult result( conn->PQexec( sql ) );
      if ( result.PQresultStatus() != PGRES_COMMAND_OK )
        throw PGException( result );

      // delete the attribute from mAttributeFields
      mAttributeFields.remove( index );
    }

    returnvalue &= conn->commit();
  }
  catch ( PGException &e )
  {
    pushError( tr( "Redshift error while deleting attributes: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  loadFields();
  conn->unlock();
  return returnvalue;
}

bool QgsRedshiftProvider::renameAttributes( const QgsFieldNameMap &renamedAttributes )
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
      // field name already in use
      pushError( tr( "Error renaming field %1: name '%2' already exists" ).arg( fieldIndex ).arg( renameIt.value() ) );
      return false;
    }

    sql += QStringLiteral( "ALTER TABLE %1 RENAME COLUMN %2 TO %3;" )
           .arg( mQuery, quotedIdentifier( mAttributeFields.at( fieldIndex ).name() ),
                 quotedIdentifier( renameIt.value() ) );
  }
  sql += QLatin1String( "COMMIT;" );

  QgsRedshiftConn *conn = connectionRW();
  if ( !conn )
  {
    return false;
  }
  conn->lock();

  try
  {
    conn->begin();
    // send sql statement and do error handling
    QgsRedshiftResult result( conn->PQexec( sql ) );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK )
      throw PGException( result );
    returnvalue = conn->commit();
  }
  catch ( PGException &e )
  {
    pushError( tr( "Redshift error while renaming attributes: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  loadFields();
  conn->unlock();
  return returnvalue;
}

bool QgsRedshiftProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  bool returnvalue = true;

  if ( mIsQuery )
    return false;

  if ( attr_map.isEmpty() )
    return true;

  QgsRedshiftConn *conn = connectionRW();
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

          numChangedFields++;

          sql += delim + QStringLiteral( "%1=" ).arg( quotedIdentifier( fld.name() ) );
          delim = ',';

          if ( fld.typeName() == QLatin1String( "geometry" ) )
          {
            sql += QStringLiteral( "%1(%2)" ).arg( "st_geomfromewkt", quotedValue( siter->toString() ) );
          }
          else if ( fld.typeName() == QLatin1String( "geography" ) )
          {
            sql += QStringLiteral( "st_geographyfromewkt(%1)" ).arg( quotedValue( siter->toString() ) );
          }
          else
          {
            sql += quotedValue( *siter );
          }
        }
        catch ( PGFieldNotFound )
        {
          // Field was missing - shouldn't happen.
          QgsDebugMsgLevel( "Updated feature has an invalid field.", 2 );
        }
      }

      sql += QStringLiteral( " WHERE %1" ).arg( whereClause( fid ) );

      // Don't try to UPDATE an empty set of values (might happen if the table
      // only has GENERATED fields, or if the user only changed GENERATED fields
      // in the form/attribute table.
      if ( numChangedFields > 0 )
      {
        QgsRedshiftResult result( conn->PQexec( sql ) );
        if ( result.PQresultStatus() != PGRES_COMMAND_OK && result.PQresultStatus() != PGRES_TUPLES_OK )
          throw PGException( result );
      }
      else // let the user know that no field was actually changed
      {
        QgsLogger::warning( tr( "No fields were updated on the database." ) );
      }

      // update feature id map if key was changed
      // PktInt64 also uses a fid map even if it is a stand alone field.
      if ( pkChanged )
      {
        QVariantList k = mShared->removeFid( fid );

        int keyCount = std::min( mPrimaryKeyAttrs.size(), k.size() );

        for ( int i = 0; i < keyCount; i++ )
        {
          int idx = mPrimaryKeyAttrs.at( i );
          if ( !attrs.contains( idx ) )
            continue;

          k[i] = attrs[idx];
        }

        mShared->insertFid( fid, k );
      }
    }

    returnvalue &= conn->commit();
  }
  catch ( PGException &e )
  {
    pushError( tr( "Redshift error while changing attributes: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  conn->unlock();
  return returnvalue;
}

void QgsRedshiftProvider::appendGeomParam( const QgsGeometry &geom, QStringList &params ) const
{
  if ( geom.isNull() )
  {
    params << QString();
    return;
  }

  QString param;

  QgsGeometry convertedGeom( convertToProviderType( geom ) );
  QByteArray wkb( !convertedGeom.isNull() ? convertedGeom.asWkb() : geom.asWkb() );
  const unsigned char *buf = reinterpret_cast<const unsigned char *>( wkb.constData() );
  int wkbSize = wkb.length();

  for ( int i = 0; i < wkbSize; ++i )
  {
    if ( connectionRO()->useWkbHex() )
      param += QStringLiteral( "%1" ).arg( ( int )buf[i], 2, 16, QChar( '0' ) );
    else
      param += QStringLiteral( "\\%1" ).arg( ( int )buf[i], 3, 8, QChar( '0' ) );
  }
  params << param;
}

bool QgsRedshiftProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  if ( mIsQuery || mGeometryColumn.isNull() )
    return false;

  QgsRedshiftConn *conn = connectionRW();
  if ( !conn )
  {
    return false;
  }
  conn->lock();

  bool returnvalue = true;

  try
  {
    // Start the Redshift transaction
    conn->begin();

    QString update;
    QgsRedshiftResult result;

    update = QStringLiteral( "UPDATE %1 SET %2=%3 WHERE %4" )
             .arg( mQuery, quotedIdentifier( mGeometryColumn ), geomParam( 1 ), pkParamWhereClause( 2 ) );

    QgsDebugMsgLevel( "updating: " + update, 2 );

    result = conn->PQprepare( QStringLiteral( "updatefeatures" ), update, 2, nullptr );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK && result.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsDebugMsg( QStringLiteral( "Exception thrown due to PQprepare of this query "
                                   "returning != PGRES_COMMAND_OK (%1 != expected %2): %3" )
                   .arg( result.PQresultStatus() )
                   .arg( PGRES_COMMAND_OK )
                   .arg( update ) );
      throw PGException( result );
    }

    QgsDebugMsgLevel( QStringLiteral( "iterating over the map of changed geometries..." ), 2 );

    for ( QgsGeometryMap::const_iterator iter = geometry_map.constBegin(); iter != geometry_map.constEnd(); ++iter )
    {
      QgsDebugMsgLevel( "iterating over feature id " + FID_TO_STRING( iter.key() ), 2 );

      QStringList params;
      appendGeomParam( *iter, params );
      appendPkParams( iter.key(), params );

      result = conn->PQexecPrepared( QStringLiteral( "updatefeatures" ), params );
      if ( result.PQresultStatus() != PGRES_COMMAND_OK && result.PQresultStatus() != PGRES_TUPLES_OK )
        throw PGException( result );

    } // for each feature

    conn->PQexecNR( QStringLiteral( "DEALLOCATE updatefeatures" ) );

    returnvalue &= conn->commit();
  }
  catch ( PGException &e )
  {
    pushError( tr( "Redshift error while changing geometry values: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    conn->PQexecNR( QStringLiteral( "DEALLOCATE updatefeatures" ) );

    returnvalue = false;
  }

  conn->unlock();

  QgsDebugMsgLevel( QStringLiteral( "leaving." ), 4 );

  return returnvalue;
}

bool QgsRedshiftProvider::changeFeatures( const QgsChangedAttributesMap &attr_map, const QgsGeometryMap &geometry_map )
{
  bool returnvalue = true;

  if ( mIsQuery )
    return false;

  if ( attr_map.isEmpty() )
    return true;

  QgsRedshiftConn *conn = connectionRW();
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

          numChangedFields++;

          sql += delim + QStringLiteral( "%1=" ).arg( quotedIdentifier( fld.name() ) );
          delim = ',';

          if ( fld.typeName() == QLatin1String( "geometry" ) )
          {
            sql += QStringLiteral( "%1(%2)" ).arg( "st_geomfromewkt", quotedValue( siter->toString() ) );
          }
          else if ( fld.typeName() == QLatin1String( "geography" ) )
          {
            sql += QStringLiteral( "st_geographyfromewkt(%1)" ).arg( quotedValue( siter->toString() ) );
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
        // Don't try to UPDATE an empty set of values (might happen if the table
        // only has GENERATED fields, or if the user only changed GENERATED
        // fields in the form/attribute table.
        if ( numChangedFields > 0 )
        {
          sql += QStringLiteral( " WHERE %1" ).arg( whereClause( fid ) );

          QgsRedshiftResult result( conn->PQexec( sql ) );
          if ( result.PQresultStatus() != PGRES_COMMAND_OK && result.PQresultStatus() != PGRES_TUPLES_OK )
            throw PGException( result );
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

        QgsRedshiftResult result( conn->PQprepare( QStringLiteral( "updatefeature" ), sql, 1, nullptr ) );
        if ( result.PQresultStatus() != PGRES_COMMAND_OK && result.PQresultStatus() != PGRES_TUPLES_OK )
        {
          QgsDebugMsg( QStringLiteral( "Exception thrown due to PQprepare of this query returning "
                                       "!= PGRES_COMMAND_OK (%1 != expected %2): %3" )
                       .arg( result.PQresultStatus() )
                       .arg( PGRES_COMMAND_OK )
                       .arg( sql ) );
          throw PGException( result );
        }

        QStringList params;
        const QgsGeometry &geom = geometry_map[fid];
        appendGeomParam( geom, params );

        result = conn->PQexecPrepared( QStringLiteral( "updatefeature" ), params );
        if ( result.PQresultStatus() != PGRES_COMMAND_OK && result.PQresultStatus() != PGRES_TUPLES_OK )
          throw PGException( result );

        conn->PQexecNR( QStringLiteral( "DEALLOCATE updatefeature" ) );
      }

      // update feature id map if key was changed
      // PktInt64 also uses a fid map even though it is a single field.
      if ( pkChanged )
      {
        QVariantList k = mShared->removeFid( fid );

        for ( int i = 0; i < mPrimaryKeyAttrs.size(); i++ )
        {
          int idx = mPrimaryKeyAttrs.at( i );
          if ( !attrs.contains( idx ) )
            continue;

          k[i] = attrs[idx];
        }

        mShared->insertFid( fid, k );
      }
    }

    returnvalue &= conn->commit();
  }
  catch ( PGException &e )
  {
    pushError( tr( "Redshift error while changing attributes: %1" ).arg( e.errorMessage() ) );
    conn->rollback();
    returnvalue = false;
  }

  conn->unlock();

  QgsDebugMsgLevel( QStringLiteral( "leaving." ), 4 );

  return returnvalue;
}

QgsAttributeList QgsRedshiftProvider::attributeIndexes() const
{
  QgsAttributeList lst;
  lst.reserve( mAttributeFields.count() );
  for ( int i = 0; i < mAttributeFields.count(); ++i )
    lst.append( i );
  return lst;
}

QgsVectorDataProvider::Capabilities QgsRedshiftProvider::capabilities() const
{
  return mEnabledCapabilities;
}

QgsFeatureSource::SpatialIndexPresence QgsRedshiftProvider::hasSpatialIndex() const
{
  // TODO(marcel): Redshift doesn't have indexes, but if we return
  // SpatialIndexNotPresent we get a warning in QGIS, should we just return
  // Uknown?
  QgsRedshiftProviderConnection conn( mUri.uri(), QVariantMap() );
  try
  {
    return conn.spatialIndexExists( mUri.schema(), mUri.table(), mUri.geometryColumn() ) ? SpatialIndexPresent
           : SpatialIndexNotPresent;
  }
  catch ( QgsProviderConnectionException & )
  {
    return SpatialIndexUnknown;
  }
}

bool QgsRedshiftProvider::setSubsetString( const QString &theSQL, bool updateFeatureCount )
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

  QgsRedshiftResult res( connectionRO()->PQexec( sql ) );
  if ( res.PQresultStatus() != PGRES_TUPLES_OK )
  {
    pushError( res.PQresultErrorMessage() );
    mSqlWhereClause = prevWhere;
    return false;
  }

  // Update datasource uri too
  mUri.setSql( theSQL );
  setDataSourceUri( mUri.uri( false ) );

  if ( updateFeatureCount )
  {
    reloadData();
  }
  else
  {
    mLayerExtent.setMinimal();
    emit dataChanged();
  }

  return true;
}

/**
 * Returns the feature count
 */
long long QgsRedshiftProvider::featureCount() const
{
  long featuresCounted = mShared->featuresCounted();
  if ( featuresCounted >= 0 )
    return featuresCounted;

  if ( !connectionRO() )
  {
    return 0;
  }

  // get total number of features
  QString sql;
  long num = -1;

  sql = QStringLiteral( "SELECT count(*) FROM %1%2" ).arg( mQuery, filterWhereClause() );
  QgsRedshiftResult result( connectionRO()->PQexec( sql ) );

  QgsDebugMsgLevel( "number of features as text: " + result.PQgetvalue( 0, 0 ), 2 );

  num = result.PQgetvalue( 0, 0 ).toLong();

  mShared->setFeaturesCounted( num );

  QgsDebugMsgLevel( "number of features: " + QString::number( num ), 2 );

  return num;
}

bool QgsRedshiftProvider::empty() const
{
  QString sql = QStringLiteral( "SELECT EXISTS (SELECT * FROM %1%2 LIMIT 1)" ).arg( mQuery, filterWhereClause() );
  QgsRedshiftResult res( connectionRO()->PQexec( sql ) );
  if ( res.PQresultStatus() != PGRES_TUPLES_OK )
  {
    pushError( res.PQresultErrorMessage() );
    return false;
  }

  return res.PQgetvalue( 0, 0 ) != QLatin1String( "t" );
}

QgsRectangle QgsRedshiftProvider::extent() const
{
  if ( !isValid() || mGeometryColumn.isNull() )
    return QgsRectangle();

  if ( mSpatialColType == SctGeography )
    return QgsRectangle( -180.0, -90.0, 180.0, 90.0 );

  if ( mLayerExtent.isEmpty() )
  {
    QString sql;
    QgsRedshiftResult result;
    QString ext;

    // TODO(Marcel): change to ST_Extent when available
    sql = QStringLiteral( "with envelope(g) as (select st_envelope(%1) from %2%3) "
                          "select min(st_xmin(g)), min(st_ymin(g)), "
                          "max(st_xmax(g)), max(st_ymax(g)) from envelope" )
          .arg( quotedIdentifier( mGeometryColumn ), mQuery, filterWhereClause() );

    result = connectionRO()->PQexec( sql );
    if ( result.PQresultStatus() != PGRES_TUPLES_OK )
      connectionRO()->PQexecNR( QStringLiteral( "ROLLBACK" ) );
    else if ( result.PQntuples() == 1 )
    {
      QgsDebugMsgLevel( "Got extents using: " + sql, 2 );

      mLayerExtent.setXMinimum( result.PQgetvalue( 0, 0 ).toDouble() );
      mLayerExtent.setYMinimum( result.PQgetvalue( 0, 1 ).toDouble() );
      mLayerExtent.setXMaximum( result.PQgetvalue( 0, 2 ).toDouble() );
      mLayerExtent.setYMaximum( result.PQgetvalue( 0, 3 ).toDouble() );
    }

    QgsDebugMsgLevel( "Set extents to: " + mLayerExtent.toString(), 2 );
  }

  return mLayerExtent;
}

void QgsRedshiftProvider::updateExtents()
{
  mLayerExtent.setMinimal();
}

// This method is responsible for determining the geometry type and SRID of geometries inside the column
// represented by the provider instance.
bool QgsRedshiftProvider::getGeometryDetails()
{
  if ( mGeometryColumn.isNull() )
  {
    mDetectedGeomType = Qgis::WkbType::NoGeometry;
    mValid = true;
    return true;
  }

  QgsRedshiftResult result;
  QString sql;

  QString schemaName = mSchemaName;
  QString tableName = mTableName;
  QString geomCol = mGeometryColumn;
  QString geomColType;

  // Trust the datasource config means that we use the requested geometry type and
  // srid. We only need to get the spatial column type.
  if ( ( mReadFlags & QgsDataProvider::FlagTrustDataSource ) && mRequestedGeomType != Qgis::WkbType::Unknown &&
       !mRequestedSrid.isEmpty() )
  {
    sql = QStringLiteral( "SELECT %1 FROM %2 LIMIT 0" ).arg( quotedIdentifier( mGeometryColumn ), mQuery );
    result = connectionRO()->PQexec( sql );
    Oid typmod;
    if ( PGRES_TUPLES_OK == result.PQresultStatus() )
    {
      typmod = getFldType( result.PQfmod( 0 ) );
    }
    else
    {
      mValid = false;
      return false;
    }


    sql = QStringLiteral( "SELECT t.typname FROM pg_type t WHERE t.oid=%1 " )
          .arg( ( int )typmod );


    result = connectionRO()->PQexec( sql );
    if ( PGRES_TUPLES_OK == result.PQresultStatus() )
    {
      geomColType = result.PQgetvalue( 0, 0 );

      // Get spatial col type
      if ( geomColType == QLatin1String( "geometry" ) )
        mSpatialColType = SctGeometry;
      else if ( geomColType == QLatin1String( "geography" ) )
        mSpatialColType = SctGeography;
      else
        mSpatialColType = SctNone;

      // Use requested geometry type and srid
      mDetectedGeomType = mRequestedGeomType;
      mDetectedSrid = mRequestedSrid;
      mValid = true;
      return true;
    }
    else
    {
      mValid = false;
      return false;
    }
  }

  // We don't trust the requested type, so we have to check the types.
  // First handle query layers.
  if ( mIsQuery )
  {
    sql = QStringLiteral( "SELECT %1 FROM %2 LIMIT 1" ).arg( quotedIdentifier( mGeometryColumn ), mQuery );

    QgsDebugMsgLevel( QStringLiteral( "Getting geometry column: %1" ).arg( sql ), 2 );

    QgsRedshiftResult result( connectionRO()->PQexec( sql ) );
    if ( PGRES_TUPLES_OK == result.PQresultStatus() )
    {
      Oid tableoid = result.PQftable( 0 );
      int column = result.PQftablecol( 0 );

      result = connectionRO()->PQexec( sql );
      // If the geometry column from thr query layer comes from a simple select from a relation,
      // we retrieve the info from the relation itself about the selected column.
      if ( tableoid > 0 && PGRES_TUPLES_OK == result.PQresultStatus() )
      {
        sql = QStringLiteral( "SELECT pg_namespace.nspname,pg_class.relname FROM "
                              "pg_class,pg_namespace WHERE "
                              "pg_class.relnamespace=pg_namespace.oid AND pg_class.oid=%1" )
              .arg( tableoid );
        result = connectionRO()->PQexec( sql );
        // We have found the relation in the catalog.
        if ( PGRES_TUPLES_OK == result.PQresultStatus() && 1 == result.PQntuples() )
        {
          schemaName = result.PQgetvalue( 0, 0 );
          tableName = result.PQgetvalue( 0, 1 );
          // Try to also find the column.
          sql = QStringLiteral( "SELECT a.attname, t.typname FROM pg_attribute a, pg_type t "
                                "WHERE a.attrelid=%1 AND a.attnum=%2 AND a.atttypid = t.oid" )
                .arg( tableoid )
                .arg( column );
          result = connectionRO()->PQexec( sql );
          if ( PGRES_TUPLES_OK == result.PQresultStatus() && 1 == result.PQntuples() )
          {
            // Found the column, update the types.
            geomCol = result.PQgetvalue( 0, 0 );
            geomColType = result.PQgetvalue( 0, 1 );
            if ( geomColType == QLatin1String( "geometry" ) )
              mSpatialColType = SctGeometry;
            else if ( geomColType == QLatin1String( "geography" ) )
              mSpatialColType = SctGeography;
            else
              mSpatialColType = SctNone;
          }
          else
          {
            // Didn't find the column, set the schemaName to empty and tableName to the query itself.
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
  // The geometry column of the layer is part of some schema and some table, so we look into the catalog tables.
  if ( !schemaName.isEmpty() )
  {
    // Check svv_geometry_columns.
    sql = QStringLiteral( "SELECT upper(type),srid FROM svv_geometry_columns WHERE "
                          "f_table_name=%1 AND f_geometry_column=%2 AND f_table_schema=%3" )
          .arg( quotedValue( tableName ), quotedValue( geomCol ), quotedValue( schemaName ) );

    QgsDebugMsgLevel( QStringLiteral( "Getting geometry column: %1" ).arg( sql ), 2 );
    result = connectionRO()->PQexec( sql );
    QgsDebugMsgLevel( QStringLiteral( "Geometry column query returned %1 rows" ).arg( result.PQntuples() ), 2 );

    if ( result.PQntuples() == 1 )
    {
      detectedType = result.PQgetvalue( 0, 0 );

      QString ds = result.PQgetvalue( 0, 1 );
      if ( ds != QLatin1String( "0" ) )
        detectedSrid = ds;
      mSpatialColType = SctGeometry;
    }
    else
    {
      connectionRO()->PQexecNR( QStringLiteral( "COMMIT" ) );
    }

    // Check svv_geography_columns once supported.
    if ( detectedType.isEmpty() && connectionRO()->supportsGeography() )
    {
      // check geography columns
      sql = QStringLiteral( "SELECT upper(type),srid FROM svv_geography_columns WHERE "
                            "f_table_name=%1 AND f_geography_column=%2 AND f_table_schema=%3" )
            .arg( quotedValue( tableName ), quotedValue( geomCol ), quotedValue( schemaName ) );

      QgsDebugMsgLevel( QStringLiteral( "Getting geography column: %1" ).arg( sql ), 2 );
      result = connectionRO()->PQexec( sql, false );
      QgsDebugMsgLevel( QStringLiteral( "Geography column query returned %1" ).arg( result.PQntuples() ), 2 );

      if ( result.PQntuples() == 1 )
      {
        QString dt = result.PQgetvalue( 0, 0 );
        if ( dt != "GEOMETRY" )
          detectedType = dt;
        QString ds = result.PQgetvalue( 0, 1 );
        if ( ds != "0" )
          detectedSrid = ds;
        mSpatialColType = SctGeography;
      }
      else
      {
        connectionRO()->PQexecNR( QStringLiteral( "COMMIT" ) );
      }
    }

    // Haven't managed to find the column in the catalog, so try to find in the system wide catalog.
    if ( mSpatialColType == SctNone )
    {
      sql = QString( "SELECT t.typname FROM "
                     "pg_attribute a, pg_class c, pg_namespace n, pg_type t "
                     "WHERE a.attrelid=c.oid AND c.relnamespace=n.oid "
                     "AND a.atttypid=t.oid "
                     "AND n.nspname=%3 AND c.relname=%1 AND a.attname=%2" )
            .arg( quotedValue( tableName ), quotedValue( geomCol ), quotedValue( schemaName ) );
      QgsDebugMsgLevel( QStringLiteral( "Getting column datatype: %1" ).arg( sql ), 2 );
      result = connectionRO()->PQexec( sql, false );
      QgsDebugMsgLevel( QStringLiteral( "Column datatype query returned %1" ).arg( result.PQntuples() ), 2 );
      if ( result.PQntuples() == 1 )
      {
        geomColType = result.PQgetvalue( 0, 0 );
        if ( geomColType == QLatin1String( "geometry" ) )
          mSpatialColType = SctGeometry;
        else if ( geomColType == QLatin1String( "geography" ) )
          mSpatialColType = SctGeography;
      }
      else
      {
        connectionRO()->PQexecNR( QStringLiteral( "COMMIT" ) );
      }
    }
  }
  else
  {
    // For query layers we have to get the typmod of the result and retrieve the type from it.
    sql = QStringLiteral( "SELECT %1 FROM %2 LIMIT 0" ).arg( quotedIdentifier( mGeometryColumn ), mQuery );
    result = connectionRO()->PQexec( sql );

    if ( PGRES_TUPLES_OK == result.PQresultStatus() )
    {
      // TODO(reflectored): is not supported yet in Redshift
      // int typoid = getFldType( result.PQftype( 0 ) );
      // int typmod = result.PQfmod( 0 );
      // Get typmod_type and typmod_srid.
      // sql = QStringLiteral( "SELECT (SELECT t.typname FROM pg_type t WHERE oid = %1), "
      //                       "upper(geometry_typmod_type(%2)), geometry_typmod_srid(%2)" )
      //       .arg( QString::number( typoid ), QString::number( typmod ) );
      // result = connectionRO()->PQexec( sql, false );
      // if ( result.PQntuples() == 1 )
      // {
      //   geomColType = result.PQgetvalue( 0, 0 );
      //   detectedType = result.PQgetvalue( 0, 1 );
      //   detectedSrid = result.PQgetvalue( 0, 2 );
      //   if ( geomColType == QLatin1String( "geometry" ) )
      //     mSpatialColType = SctGeometry;
      //   else if ( geomColType == QLatin1String( "geography" ) )
      //     mSpatialColType = SctGeography;
      //   else
      //   {
      //     detectedType = mRequestedGeomType == Qgis::WkbType::Unknown
      //                    ? QString()
      //                    : QgsRedshiftConn::redshiftWkbTypeName( mRequestedGeomType );
      //     detectedSrid = mRequestedSrid;
      //   }
      // }
      // else
      // {
      connectionRO()->PQexecNR( QStringLiteral( "COMMIT" ) );
      detectedType = mRequestedGeomType == Qgis::WkbType::Unknown
                     ? QString()
                     : QgsRedshiftConn::redshiftWkbTypeName( mRequestedGeomType );
      // }
    }
    else
    {
      mValid = false;
      return false;
    }
  }

  // Check the current detected geom type and srid.
  mDetectedGeomType = QgsWkbTypes::parseType( detectedType );
  mDetectedSrid = detectedSrid;

  // If still uknown, we have to retrieve the layer type by querying through all the geometries inside the column.
  if ( mDetectedGeomType == Qgis::WkbType::Unknown )
  {
    mDetectedSrid.clear();

    QgsRedshiftLayerProperty layerProperty;
    if ( !mIsQuery )
    {
      layerProperty.databaseName = mExternalDatabaseName;
      layerProperty.schemaName = schemaName;
      layerProperty.tableName = tableName;
    }
    else
    {
      layerProperty.schemaName.clear();
      layerProperty.tableName = mQuery;
    }
    layerProperty.geometryColName = mGeometryColumn;
    layerProperty.geometryColType = mSpatialColType;

    QString delim;

    if ( !mSqlWhereClause.isEmpty() )
    {
      layerProperty.sql += delim + '(' + mSqlWhereClause + ')';
      delim = QStringLiteral( " AND " );
    }

    // Retrieve all the distinct layer type of the column.
    connectionRO()->retrieveLayerTypes( layerProperty, mUseEstimatedMetadata );

    mSpatialColType = layerProperty.geometryColType;

    if ( layerProperty.size() == 0 )
    {
      // If there are no rows, and no requested SRID or GeometryType set, set a default.
      mRequestedSrid = mUri.srid().isEmpty() ? QStringLiteral( "4326" ) : mUri.srid();
      mRequestedGeomType = mUri.wkbType() == Qgis::WkbType::Unknown ? Qgis::WkbType::Point : mUri.wkbType();
    }
    else
    {
      int i;
      for ( i = 0; i < layerProperty.size(); i++ )
      {
        Qgis::WkbType wkbType = layerProperty.types.at( i );

        // Find first compatible pair with requested type and srid.
        if ( ( wkbType != Qgis::WkbType::Unknown &&
               ( mRequestedGeomType == Qgis::WkbType::Unknown || mRequestedGeomType == wkbType ) ) &&
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
          mDetectedSrid = QString::number( layerProperty.srids.at( 0 ) );
        }
      }
      else
      {
        // Not found compa
        QgsMessageLog::logMessage( tr( "No compatible features in the column %1 of %2"
                                       "with the requested type and SRID." )
                                   .arg( mGeometryColumn, mQuery ) );
      }
    }
  }

  QgsDebugMsgLevel( QStringLiteral( "Detected SRID is %1" ).arg( mDetectedSrid ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Requested SRID is %1" ).arg( mRequestedSrid ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Detected type is %1" ).arg( qgsEnumValueToKey( mDetectedGeomType ) ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Requested type is %1" ).arg( qgsEnumValueToKey( mRequestedGeomType ) ), 2 );

  mValid = ( mDetectedGeomType != Qgis::WkbType::Unknown || mRequestedGeomType != Qgis::WkbType::Unknown ) &&
           ( !mDetectedSrid.isEmpty() || !mRequestedSrid.isEmpty() );

  if ( !mValid )
    return false;

  QgsDebugMsgLevel(
    QStringLiteral( "Spatial column type is %1" ).arg( QgsRedshiftConn::displayStringForGeomType( mSpatialColType ) ), 2 );

  return mValid;
}

bool QgsRedshiftProvider::convertField( QgsField &field, const QMap<QString, QVariant> * )
{
  QString fieldType = QString( "varchar" ); // default to string
  int fieldSize = field.length();
  int fieldPrec = field.precision();
  switch ( field.type() )
  {
    case QVariant::LongLong:
      fieldType = QStringLiteral( "int8" );
      fieldPrec = 0;
      break;

    case QVariant::DateTime:
      if ( field.typeName() != QLatin1String( "timestamptz" ) )
        fieldType = QStringLiteral( "timestamp" ); // set default without timezone in case
      // given field is not marked with timezone
      break;

    case QVariant::Time:
      if ( field.typeName() != QLatin1String( "timetz" ) )
        fieldType = QStringLiteral( "time" ); // set default without timezone in case
      // given field is not marked with timezone
      break;

    case QVariant::String:
      fieldType = QStringLiteral( "varchar" );
      fieldPrec = 0;
      break;

    case QVariant::Char:
      fieldType = QStringLiteral( "char" );
      fieldSize = 1;
      fieldPrec = 0;
      break;

    case QVariant::Int:
      fieldType = QStringLiteral( "int4" );
      fieldPrec = 0;
      break;

    case QVariant::Date:
      fieldType = QStringLiteral( "date" );
      fieldPrec = 0;
      break;

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
      fieldPrec = 0;
      break;

    case QVariant::Bool:
      fieldType = QStringLiteral( "bool" );
      fieldPrec = 0;
      fieldSize = -1;
      break;

    default:
      fieldType = QStringLiteral( "varchar" );
      fieldPrec = 0;
      break;
      // return false;
  }

  // cap max size
  if ( fieldType == QLatin1String( "varchar" ) && fieldSize == -1 )
    fieldSize = redshiftDefaultVarcharLength();

  field.setTypeName( fieldType );
  field.setLength( fieldSize );
  field.setPrecision( fieldPrec );
  return true;
}

void spatialGeometryType( Qgis::WkbType wkbType, QString &geometryType )
{
  Qgis::WkbType flatType = QgsWkbTypes::flatType( wkbType );
  switch ( flatType )
  {
    case Qgis::WkbType::Unknown:
      geometryType = QStringLiteral( "GEOMETRY" );
      break;
    case Qgis::WkbType::NoGeometry:
      geometryType.clear();
      break;
    default:
      // store QGIS Point25D geometries as 3DZ geometries in Redshift
      if ( wkbType >= Qgis::WkbType::Point25D && wkbType <= Qgis::WkbType::MultiPolygon25D )
      {
        geometryType = QgsWkbTypes::displayString( QgsWkbTypes::flatType( wkbType ) ).toUpper() + "Z";
      }
      else
      {
        geometryType = QgsWkbTypes::displayString( wkbType ).toUpper();
      }
  }
}

Qgis::VectorExportResult QgsRedshiftProvider::createEmptyLayer(
  const QString &uri, const QgsFields &fields, Qgis::WkbType wkbType, const QgsCoordinateReferenceSystem &srs,
  bool overwrite, QMap<int, int> *oldToNewAttrIdxMap, QString *errorMessage, const QMap<QString, QVariant> *options )
{
  // populate members from the uri structure
  QgsDataSourceUri dsUri( uri );

  QString schemaName = dsUri.schema().toLower();
  QString tableName = dsUri.table().toLower();

  QString geometryColumn = dsUri.geometryColumn().toLower();
  QString geometryType;

  QString primaryKey = dsUri.keyColumn().toLower();
  QString primaryKeyType;

  QStringList pkList;
  QStringList pkType;

  // TODO(marcel): use Redshift SRIDs later on, for now use postgis
  int srid = srs.postgisSrid();

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
  QgsRedshiftConn *conn = QgsRedshiftConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Connection to database failed" );
    return Qgis::VectorExportResult::ErrorConnectionFailed;
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
    pkType = QStringList( QStringLiteral( "int generated by default as identity(1,1)" ) );
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
      if ( type.isEmpty() )
        type = QStringLiteral( "int generated by default as identity(1,1)" );
      else
      {
        // if the pk field's type is one of the redshift integer types,
        // use the equivalent autoincremental type
        if ( primaryKeyType == QLatin1String( "int2" ) || primaryKeyType == QLatin1String( "int4" ) )
        {
          primaryKeyType = QStringLiteral( "int generated by default as identity(1,1)" );
        }
        else if ( primaryKeyType == QLatin1String( "int8" ) )
        {
          primaryKeyType = QStringLiteral( "int8 generated by default as identity(1,1)" );
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
      QString sql = QString( "SELECT current_schema()" );
      QgsRedshiftResult result( conn->PQexec( sql ) );
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
                  .arg( quotedValue( tableName ), quotedValue( schemaName ) );

    QgsRedshiftResult result( conn->PQexec( sql ) );
    if ( result.PQresultStatus() != PGRES_TUPLES_OK )
      throw PGException( result );

    bool exists = result.PQntuples() > 0;

    if ( exists && overwrite )
    {
      // delete the table if exists, then re-create it
      QString sql = QString( "DROP TABLE %1.%2" ).arg( quotedIdentifier( schemaName ), quotedIdentifier( tableName ) );

      result = conn->PQexec( sql );
      if ( result.PQresultStatus() != PGRES_COMMAND_OK )
        throw PGException( result );
    }

    sql = QStringLiteral( "CREATE TABLE %1(" ).arg( schemaTableName );
    QString pk;
    for ( int i = 0; i < pkList.size(); ++i )
    {
      QString col = pkList[i];
      const QString &type = pkType[i];

      if ( i )
      {
        pk += QLatin1String( "," );
        sql += QLatin1String( "," );
      }

      pk += col;
      sql += col + " " + type;
    }
    sql += QStringLiteral( ", PRIMARY KEY (%1) )" ).arg( pk );

    result = conn->PQexec( sql );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK )
      throw PGException( result );

    spatialGeometryType( wkbType, geometryType );

    // create geometry column
    if ( !geometryType.isEmpty() )
    {
      sql = QStringLiteral( "ALTER TABLE %1.%2 ADD COLUMN %3 GEOMETRY" )
            // TODO(reflectored): add back when geometry constrains are supported.
            // GEOMETRY(%4,%5)" )
            .arg( quotedIdentifier( schemaName ), quotedIdentifier( tableName ), quotedIdentifier( geometryColumn ) );
      // .arg( geometryType )
      // .arg( srid );

      result = conn->PQexec( sql );
      if ( result.PQresultStatus() != PGRES_COMMAND_OK )
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
      *errorMessage =
        QObject::tr( "Creation of data source %1 failed: \n%2" ).arg( schemaTableName, e.errorMessage() );

    conn->PQexecNR( QStringLiteral( "ROLLBACK" ) );
    conn->unref();
    return Qgis::VectorExportResult::ErrorCreatingLayer;
  }
  conn->unref();

  QgsDebugMsgLevel( QStringLiteral( "layer %1 created" ).arg( schemaTableName ), 2 );

  // use the provider to edit the table
  dsUri.setDataSource( schemaName, tableName, geometryColumn, QString(), primaryKey );

  // give hint to provider which type and srid to use in order to ensure that it
  // is initialized successfuly
  dsUri.setSrid( QString::number( srid ) );
  dsUri.setWkbType( wkbType );

  QgsDataProvider::ProviderOptions providerOptions;
  QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags();
  std::unique_ptr<QgsRedshiftProvider> provider =
    std::make_unique<QgsRedshiftProvider>( dsUri.uri( false ), providerOptions, flags );
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
      fld.setName( fld.name().toLower() );

      if ( fld.name() == geometryColumn )
      {
        QgsDebugMsgLevel( QStringLiteral( "Found a field with the same name of "
                                          "the geometry column. Skip it!" ),
                          2 );
        continue;
      }

      int pkIdx = -1;
      for ( int i = 0; i < pkList.size(); ++i )
      {
        QString col = pkList[i];

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

      if ( !( options && options->value( QStringLiteral( "skipConvertFields" ), false ).toBool() ) &&
           !convertField( fld, options ) )
      {
        if ( errorMessage )
          *errorMessage = QObject::tr( "Unsupported type for field %1" ).arg( fld.name() );

        return Qgis::VectorExportResult::ErrorAttributeTypeUnsupported;
      }

      QgsDebugMsgLevel( QStringLiteral( "creating field #%1 -> #%2 name %3 type %4 typename "
                                        "%5 width %6 precision %7" )
                        .arg( fldIdx )
                        .arg( offset )
                        .arg( fld.name(), QVariant::typeToName( fld.type() ), fld.typeName() )
                        .arg( fld.length() )
                        .arg( fld.precision() ),
                        2 );

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

QgsCoordinateReferenceSystem QgsRedshiftProvider::crs() const
{
  // TODO(marcel): use postgres srids for now
  int srid = mRequestedSrid.isEmpty() ? mDetectedSrid.toInt() : mRequestedSrid.toInt();
  Q_NOWARN_DEPRECATED_PUSH
  return QgsCoordinateReferenceSystem( srid );
  Q_NOWARN_DEPRECATED_POP

  // TODO(marcel): fix this when spatial_ref_sys is introduced in Redshift
}

QString QgsRedshiftProvider::subsetString() const
{
  return mSqlWhereClause;
}

QString QgsRedshiftProvider::getTableName()
{
  return mTableName;
}

size_t QgsRedshiftProvider::layerCount() const
{
  return 1; // XXX need to return actual number of layers
} // QgsRedshiftProvider::layerCount()

QString QgsRedshiftProvider::name() const
{
  return REDSHIFT_KEY;
} //  QgsRedshiftProvider::name()

QString QgsRedshiftProvider::description() const
{
  QString rsVersion( tr( "Provider version unknown" ) );
  QString spatialVersion( tr( "unknown" ) );

  if ( connectionRO() )
  {
    QgsRedshiftResult result;

    result = connectionRO()->PQexec( QStringLiteral( "SELECT version()" ) );
    if ( result.PQresultStatus() == PGRES_TUPLES_OK )
    {
      rsVersion = result.PQgetvalue( 0, 0 );
    }

    result = connectionRO()->PQexec( QStringLiteral( "SELECT spatial_version()" ) );
    if ( result.PQresultStatus() == PGRES_TUPLES_OK )
    {
      spatialVersion = result.PQgetvalue( 0, 0 );
    }
  }
  else
  {
    rsVersion = tr( "Redshift not connected" );
  }

  return tr( "Redshift provider: %1\nSpatial version: %2" ).arg( rsVersion, spatialVersion );
} //  QgsRedshiftProvider::description()

QVariant QgsRedshiftProvider::convertValue( QVariant::Type type, const QString &value, const QString & )
{
  QVariant result;

  if ( type == QVariant::Bool )
  {
    if ( value == QChar( 't' ) )
      result = true;
    else if ( value == QChar( 'f' ) )
      result = false;
    else
      result = QVariant( type );
  }
  else
  {
    result = value;
    if ( !result.convert( type ) || value.isNull() )
      result = QVariant( type );
  }

  return result;
}

QList<QgsVectorLayer *> QgsRedshiftProvider::searchLayers( const QList<QgsVectorLayer *> &layers,
    const QString &connectionInfo, const QString &schema,
    const QString &tableName )
{
  QList<QgsVectorLayer *> result;
  const auto constLayers = layers;
  for ( QgsVectorLayer *layer : constLayers )
  {
    const QgsRedshiftProvider *pgProvider = qobject_cast<QgsRedshiftProvider *>( layer->dataProvider() );
    if ( pgProvider && pgProvider->mUri.connectionInfo( false ) == connectionInfo &&
         pgProvider->mSchemaName == schema && pgProvider->mTableName == tableName )
    {
      result.append( layer );
    }
  }
  return result;
}

QgsRedshiftProvider::Relkind QgsRedshiftProvider::relkind() const
{
  if ( mIsQuery || !connectionRO() )
    return Relkind::Unknown;

  QString sql = QStringLiteral( "SELECT relkind FROM pg_class WHERE oid=regclass(%1)::oid" ).arg( quotedValue( mQuery ) );
  QgsRedshiftResult res( connectionRO()->PQexec( sql ) );
  QString type = res.PQgetvalue( 0, 0 );

  QgsRedshiftProvider::Relkind kind = Relkind::Unknown;

  if ( type == QLatin1String( "r" ) )
  {
    kind = Relkind::OrdinaryTable;
  }
  else if ( type == QLatin1String( "v" ) )
  {
    if ( connectionRO()->isMaterializedView( mSchemaName, mTableName ) )
      kind = Relkind::MaterializedView;
    else
      kind = Relkind::View;
  }

  return kind;
}

bool QgsRedshiftProvider::hasMetadata() const
{
  bool hasMetadata = true;
  if ( !mExternalDatabaseName.isEmpty() )
  {
    return false;
  }
  QgsRedshiftProvider::Relkind kind = relkind();

  if ( kind == Relkind::View || kind == Relkind::MaterializedView )
  {
    hasMetadata = false;
  }

  return hasMetadata;
}

QgsDataProvider *QgsRedshiftProviderMetadata::createProvider( const QString &uri,
    const QgsDataProvider::ProviderOptions &options,
    QgsDataProvider::ReadFlags flags )
{
  return new QgsRedshiftProvider( uri, options, flags );
}

QList<QgsDataItemProvider *> QgsRedshiftProviderMetadata::dataItemProviders() const
{
  QList<QgsDataItemProvider *> providers;
  providers << new QgsRedshiftDataItemProvider;
  return providers;
}

// ---------------------------------------------------------------------------

Qgis::VectorExportResult QgsRedshiftProviderMetadata::createEmptyLayer(
  const QString &uri, const QgsFields &fields, Qgis::WkbType wkbType, const QgsCoordinateReferenceSystem &srs,
  bool overwrite, QMap<int, int> &oldToNewAttrIdxMap, QString &errorMessage, const QMap<QString, QVariant> *options )
{
  return QgsRedshiftProvider::createEmptyLayer( uri, fields, wkbType, srs, overwrite, &oldToNewAttrIdxMap,
         &errorMessage, options );
}

bool QgsRedshiftProviderMetadata::saveStyle( const QString &uri, const QString &qmlStyleIn, const QString &sldStyleIn,
    const QString &styleName, const QString &styleDescription,
    const QString &uiFileContent, bool useAsDefault, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );

  // Replace invalid XML characters
  QString qmlStyle{qmlStyleIn};
  QgsRedshiftUtils::replaceInvalidXmlChars( qmlStyle );
  QString sldStyle{sldStyleIn};
  QgsRedshiftUtils::replaceInvalidXmlChars( sldStyle );

  QgsRedshiftConn *conn = QgsRedshiftConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return false;
  }

  if ( !tableExists( *conn, QStringLiteral( "layer_styles" ) ) )
  {
    QgsRedshiftResult res( conn->PQexec( "CREATE TABLE layer_styles("
                                         "id int IDENTITY(1,1) PRIMARY KEY"
                                         ",f_table_catalog varchar"
                                         ",f_table_schema varchar"
                                         ",f_table_name varchar"
                                         ",f_geometry_column varchar"
                                         ",styleName text"
                                         ",styleQML varchar(max)"
                                         ",styleSLD varchar(max)"
                                         ",useAsDefault boolean"
                                         ",description text"
                                         ",owner varchar(63) DEFAULT CURRENT_USER"
                                         ",ui varchar(max)"
                                         ",update_time timestamp DEFAULT CURRENT_TIMESTAMP"
                                         ",type varchar"
                                         ")" ) );
    if ( res.PQresultStatus() != PGRES_COMMAND_OK )
    {
      errCause = QObject::tr( "Unable to save layer style. It's not possible to create the "
                              "destination table on the database. Maybe this is due to table "
                              "permissions (user=%1). Please contact your database admin" )
                 .arg( dsUri.username() );
      conn->unref();
      return false;
    }
  }

  if ( dsUri.database().isEmpty() )
  {
    dsUri.setDatabase( conn->currentDatabase() );
  }

  QString uiFileColumn;
  QString uiFileValue;
  if ( !uiFileContent.isEmpty() )
  {
    uiFileColumn = QStringLiteral( ",ui" );
    uiFileValue = QgsRedshiftConn::quotedValue( uiFileContent );
  }

  const QString wkbTypeString =
    QgsRedshiftConn::quotedValue( QgsWkbTypes::geometryDisplayString( QgsWkbTypes::geometryType( dsUri.wkbType() ) ) );

  // Note: in the construction of the INSERT and UPDATE strings the qmlStyle and
  // sldStyle values can contain user entered strings, which may themselves
  // include %## values that would be replaced by the QString.arg function.  To
  // ensure that the final SQL string is not corrupt these two values are both
  // replaced in the final .arg call of the string construction.

  QString sql = QString( "INSERT INTO layer_styles("
                         "f_table_catalog,f_table_schema,f_table_name,f_geometry_column,"
                         "styleName,styleQML,styleSLD,useAsDefault,description,owner,type%12"
                         ") VALUES ("
                         "%1,%2,%3,%4,%5,%16,%17,%8,%9,%10,%11%13"
                         ")" )
                .arg( QgsRedshiftConn::quotedValue( dsUri.database() ) )
                .arg( QgsRedshiftConn::quotedValue( dsUri.schema() ) )
                .arg( QgsRedshiftConn::quotedValue( dsUri.table() ) )
                .arg( QgsRedshiftConn::quotedValue( dsUri.geometryColumn() ) )
                .arg( QgsRedshiftConn::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) )
                .arg( useAsDefault ? "true" : "false" )
                .arg( QgsRedshiftConn::quotedValue(
                        styleDescription.isEmpty() ? QDateTime::currentDateTime().toString() : styleDescription ) )
                .arg( "CURRENT_USER" )
                .arg( uiFileColumn )
                .arg( uiFileValue )
                .arg( wkbTypeString )
                // Must be the final .arg replacement - see above
                .arg( QgsRedshiftConn::quotedValue( qmlStyle ), QgsRedshiftConn::quotedValue( sldStyle ) );

  QString checkQuery = QString( "SELECT styleName"
                                " FROM layer_styles"
                                " WHERE f_table_catalog=%1"
                                " AND f_table_schema=%2"
                                " AND f_table_name=%3"
                                " AND f_geometry_column=%4"
                                " AND (type=%5 OR type IS NULL)"
                                " AND styleName=%6" )
                       .arg( QgsRedshiftConn::quotedValue( dsUri.database() ) )
                       .arg( QgsRedshiftConn::quotedValue( dsUri.schema() ) )
                       .arg( QgsRedshiftConn::quotedValue( dsUri.table() ) )
                       .arg( QgsRedshiftConn::quotedValue( dsUri.geometryColumn() ) )
                       .arg( wkbTypeString )
                       .arg( QgsRedshiftConn::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) );

  QgsRedshiftResult res( conn->PQexec( checkQuery ) );
  if ( res.PQntuples() > 0 )
  {
    if ( QMessageBox::question( nullptr, QObject::tr( "Save style in database" ),
                                QObject::tr( "A style named \"%1\" already exists in the database "
                                    "for this layer. Do you want to overwrite it?" )
                                .arg( styleName.isEmpty() ? dsUri.table() : styleName ),
                                QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
    {
      errCause = QObject::tr( "Operation aborted. No changes were made in the database" );
      conn->unref();
      return false;
    }

    sql = QString( "UPDATE layer_styles"
                   " SET useAsDefault=%1"
                   ",styleQML=%12"
                   ",styleSLD=%13"
                   ",description=%4"
                   ",owner=%5"
                   ",type=%2"
                   " WHERE f_table_catalog=%6"
                   " AND f_table_schema=%7"
                   " AND f_table_name=%8"
                   " AND f_geometry_column=%9"
                   " AND styleName=%10"
                   " AND (type=%2 OR type IS NULL)" )
          .arg( useAsDefault ? "true" : "false" )
          .arg( wkbTypeString )
          .arg( QgsRedshiftConn::quotedValue( styleDescription.isEmpty() ? QDateTime::currentDateTime().toString()
                : styleDescription ) )
          .arg( "CURRENT_USER" )
          .arg( QgsRedshiftConn::quotedValue( dsUri.database() ) )
          .arg( QgsRedshiftConn::quotedValue( dsUri.schema() ) )
          .arg( QgsRedshiftConn::quotedValue( dsUri.table() ) )
          .arg( QgsRedshiftConn::quotedValue( dsUri.geometryColumn() ) )
          .arg( QgsRedshiftConn::quotedValue( styleName.isEmpty() ? dsUri.table() : styleName ) )
          // Must be the final .arg replacement - see above
          .arg( QgsRedshiftConn::quotedValue( qmlStyle ), QgsRedshiftConn::quotedValue( sldStyle ) );
  }

  if ( useAsDefault )
  {
    QString removeDefaultSql = QString( "UPDATE layer_styles"
                                        " SET useAsDefault=false"
                                        " WHERE f_table_catalog=%1"
                                        " AND f_table_schema=%2"
                                        " AND f_table_name=%3"
                                        " AND f_geometry_column=%4"
                                        " AND (type=%5 OR type IS NULL)" )
                               .arg( QgsRedshiftConn::quotedValue( dsUri.database() ) )
                               .arg( QgsRedshiftConn::quotedValue( dsUri.schema() ) )
                               .arg( QgsRedshiftConn::quotedValue( dsUri.table() ) )
                               .arg( QgsRedshiftConn::quotedValue( dsUri.geometryColumn() ) )
                               .arg( wkbTypeString );

    sql = QStringLiteral( "BEGIN; %1; %2; COMMIT;" ).arg( removeDefaultSql, sql );
  }

  res = conn->PQexec( sql );

  bool saved = res.PQresultStatus() == PGRES_COMMAND_OK;
  if ( !saved )
    errCause = QObject::tr( "Unable to save layer style. It's not possible to "
                            "insert a new record into the style table. Maybe "
                            "this is due to table permissions (user=%1). Please "
                            "contact your database administrator." )
               .arg( dsUri.username() );

  conn->unref();

  return saved;
}

QString QgsRedshiftProviderMetadata::loadStyle( const QString &uri, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  QString selectQmlQuery;

  QgsRedshiftConn *conn = QgsRedshiftConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed" );
    return QString();
  }

  if ( dsUri.database().isEmpty() )
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
    geomColumnExpr = QStringLiteral( "=" ) + QgsRedshiftConn::quotedValue( dsUri.geometryColumn() );
  }

  QString wkbTypeString =
    QgsRedshiftConn::quotedValue( QgsWkbTypes::geometryDisplayString( QgsWkbTypes::geometryType( dsUri.wkbType() ) ) );

  selectQmlQuery = QString( "SELECT styleQML"
                            " FROM layer_styles"
                            " WHERE f_table_catalog=%1"
                            " AND f_table_schema=%2"
                            " AND f_table_name=%3"
                            " AND f_geometry_column %4"
                            " AND (type=%5 OR type IS NULL)"
                            " ORDER BY CASE WHEN useAsDefault THEN 1 ELSE 2 END"
                            ",update_time DESC LIMIT 1" )
                   .arg( QgsRedshiftConn::quotedValue( dsUri.database() ) )
                   .arg( QgsRedshiftConn::quotedValue( dsUri.schema() ) )
                   .arg( QgsRedshiftConn::quotedValue( dsUri.table() ) )
                   .arg( geomColumnExpr )
                   .arg( wkbTypeString );

  QgsRedshiftResult result( conn->PQexec( selectQmlQuery ) );

  QString style = result.PQntuples() == 1 ? result.PQgetvalue( 0, 0 ) : QString();
  conn->unref();

  QgsRedshiftUtils::restoreInvalidXmlChars( style );

  return style;
}

int QgsRedshiftProviderMetadata::listStyles( const QString &uri, QStringList &ids, QStringList &names,
    QStringList &descriptions, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );

  QgsRedshiftConn *conn = QgsRedshiftConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed using username: %1" ).arg( dsUri.username() );
    return -1;
  }

  if ( dsUri.database().isEmpty() )
  {
    dsUri.setDatabase( conn->currentDatabase() );
  }

  QString wkbTypeString =
    QgsRedshiftConn::quotedValue( QgsWkbTypes::geometryDisplayString( QgsWkbTypes::geometryType( dsUri.wkbType() ) ) );

  QString selectRelatedQuery = QString( "SELECT id,styleName,description"
                                        " FROM layer_styles"
                                        " WHERE f_table_catalog=%1"
                                        " AND f_table_schema=%2"
                                        " AND f_table_name=%3"
                                        " AND f_geometry_column=%4"
                                        " AND (type=%5 OR type IS NULL)"
                                        " ORDER BY useasdefault DESC, update_time DESC" )
                               .arg( QgsRedshiftConn::quotedValue( dsUri.database() ) )
                               .arg( QgsRedshiftConn::quotedValue( dsUri.schema() ) )
                               .arg( QgsRedshiftConn::quotedValue( dsUri.table() ) )
                               .arg( QgsRedshiftConn::quotedValue( dsUri.geometryColumn() ) )
                               .arg( wkbTypeString );

  QgsRedshiftResult result( conn->PQexec( selectRelatedQuery ) );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectRelatedQuery ) );
    errCause = QObject::tr( "Error executing the select query for related "
                            "styles. The query was logged" );
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
                                       " WHERE NOT (f_table_catalog=%1 AND f_table_schema=%2 AND "
                                       "f_table_name=%3 AND f_geometry_column=%4 AND type=%5)"
                                       " ORDER BY update_time DESC" )
                              .arg( QgsRedshiftConn::quotedValue( dsUri.database() ) )
                              .arg( QgsRedshiftConn::quotedValue( dsUri.schema() ) )
                              .arg( QgsRedshiftConn::quotedValue( dsUri.table() ) )
                              .arg( QgsRedshiftConn::quotedValue( dsUri.geometryColumn() ) )
                              .arg( wkbTypeString );

  result = conn->PQexec( selectOthersQuery );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectOthersQuery ) );
    errCause = QObject::tr( "Error executing the select query for unrelated "
                            "styles. The query was logged" );
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

bool QgsRedshiftProviderMetadata::deleteStyleById( const QString &uri,
    const QString &styleId, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );
  bool deleted;

  QgsRedshiftConn *conn = QgsRedshiftConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed using username: %1" ).arg( dsUri.username() );
    deleted = false;
  }
  else
  {
    QString deleteStyleQuery =
      QStringLiteral( "DELETE FROM layer_styles WHERE id=%1" ).arg( QgsRedshiftConn::quotedValue( styleId ) );
    QgsRedshiftResult result( conn->PQexec( deleteStyleQuery ) );
    if ( result.PQresultStatus() != PGRES_COMMAND_OK )
    {
      QgsDebugMsg( QString( "PQexec of this query returning != PGRES_COMMAND_OK "
                            "(%1 != expected %2): %3" )
                   .arg( result.PQresultStatus() )
                   .arg( PGRES_COMMAND_OK )
                   .arg( deleteStyleQuery ) );
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

QString QgsRedshiftProviderMetadata::getStyleById( const QString &uri, const QString &styleId, QString &errCause )
{
  QgsDataSourceUri dsUri( uri );

  QgsRedshiftConn *conn = QgsRedshiftConn::connectDb( dsUri.connectionInfo( false ), false );
  if ( !conn )
  {
    errCause = QObject::tr( "Connection to database failed using username: %1" ).arg( dsUri.username() );
    return QString();
  }

  QString style;
  QString selectQmlQuery =
    QStringLiteral( "SELECT styleQml FROM layer_styles WHERE id=%1" ).arg( QgsRedshiftConn::quotedValue( styleId ) );
  QgsRedshiftResult result( conn->PQexec( selectQmlQuery ) );
  if ( result.PQresultStatus() == PGRES_TUPLES_OK )
  {
    if ( result.PQntuples() == 1 )
      style = result.PQgetvalue( 0, 0 );
    else
      errCause = QObject::tr( "Consistency error in table '%1'. Style id should be unique" )
                 .arg( QStringLiteral( "layer_styles" ) );
  }
  else
  {
    QgsMessageLog::logMessage( QObject::tr( "Error executing query: %1" ).arg( selectQmlQuery ) );
    errCause = QObject::tr( "Error executing the select query. The query was logged" );
  }

  conn->unref();

  QgsRedshiftUtils::restoreInvalidXmlChars( style );

  return style;
}

QMap<QString, QgsAbstractProviderConnection *> QgsRedshiftProviderMetadata::connections( bool cached )
{
  return connectionsProtected<QgsRedshiftProviderConnection, QgsRedshiftConn>( cached );
}

QgsAbstractProviderConnection *QgsRedshiftProviderMetadata::createConnection( const QString &uri,
    const QVariantMap &configuration )
{
  return new QgsRedshiftProviderConnection( uri, configuration );
}

void QgsRedshiftProviderMetadata::deleteConnection( const QString &name )
{
  deleteConnectionProtected<QgsRedshiftProviderConnection>( name );
}

void QgsRedshiftProviderMetadata::saveConnection( const QgsAbstractProviderConnection *conn, const QString &name )
{
  saveConnectionProtected( conn, name );
}

QgsAbstractProviderConnection *QgsRedshiftProviderMetadata::createConnection( const QString &name )
{
  return new QgsRedshiftProviderConnection( name );
}

QgsRedshiftProjectStorage *gProjectStorage = nullptr; // when not null it is owned by
// QgsApplication::projectStorageRegistry()

void QgsRedshiftProviderMetadata::initProvider()
{
  Q_ASSERT( !gProjectStorage );
  gProjectStorage = new QgsRedshiftProjectStorage;
  QgsApplication::projectStorageRegistry()->registerProjectStorage( gProjectStorage ); // takes ownership
}

void QgsRedshiftProviderMetadata::cleanupProvider()
{
  QgsApplication::projectStorageRegistry()->unregisterProjectStorage( gProjectStorage ); // destroys the object
  gProjectStorage = nullptr;

  QgsRedshiftConnPool::cleanupInstance();
}

// ----------

void QgsRedshiftSharedData::addFeaturesCounted( long diff )
{
  QMutexLocker locker( &mMutex );

  if ( mFeaturesCounted >= 0 )
    mFeaturesCounted += diff;
}

void QgsRedshiftSharedData::ensureFeaturesCountedAtLeast( long fetched )
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

long QgsRedshiftSharedData::featuresCounted()
{
  QMutexLocker locker( &mMutex );
  return mFeaturesCounted;
}

void QgsRedshiftSharedData::setFeaturesCounted( long count )
{
  QMutexLocker locker( &mMutex );
  mFeaturesCounted = count;
}

QgsFeatureId QgsRedshiftSharedData::lookupFid( const QVariantList &v )
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

QVariantList QgsRedshiftSharedData::removeFid( QgsFeatureId fid )
{
  QMutexLocker locker( &mMutex );

  QVariantList v = mFidToKey[fid];
  mFidToKey.remove( fid );
  mKeyToFid.remove( v );
  return v;
}

void QgsRedshiftSharedData::insertFid( QgsFeatureId fid, const QVariantList &k )
{
  QMutexLocker locker( &mMutex );

  mFidToKey.insert( fid, k );
  mKeyToFid.insert( k, fid );
}

QVariantList QgsRedshiftSharedData::lookupKey( QgsFeatureId featureId )
{
  QMutexLocker locker( &mMutex );

  QMap<QgsFeatureId, QVariantList>::const_iterator it = mFidToKey.constFind( featureId );
  if ( it != mFidToKey.constEnd() )
    return it.value();
  return QVariantList();
}

void QgsRedshiftSharedData::clear()
{
  QMutexLocker locker( &mMutex );
  mFidToKey.clear();
  mKeyToFid.clear();
  mFeaturesCounted = -1;
  mFidCounter = 0;
}

QgsRedshiftProviderMetadata::QgsRedshiftProviderMetadata()
  : QgsProviderMetadata( QgsRedshiftProvider::REDSHIFT_KEY, QgsRedshiftProvider::REDSHIFT_KEY )
{
}

#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsRedshiftProviderMetadata();
}
#endif

QVariantMap QgsRedshiftProviderMetadata::decodeUri( const QString &uri ) const
{
  const QgsDataSourceUri dsUri{uri};
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
    uriParts[QStringLiteral( "type" )] = static_cast< quint32>( dsUri.wkbType() );

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

  if ( uri.contains( QStringLiteral( "sslmode=" ), Qt::CaseSensitivity::CaseInsensitive ) )
    uriParts[QStringLiteral( "sslmode" )] = dsUri.sslMode();

  if ( !dsUri.sql().isEmpty() )
    uriParts[QStringLiteral( "sql" )] = dsUri.sql();
  if ( !dsUri.geometryColumn().isEmpty() )
    uriParts[QStringLiteral( "geometrycolumn" )] = dsUri.geometryColumn();

  return uriParts;
}

QString QgsRedshiftProviderMetadata::encodeUri( const QVariantMap &parts ) const
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
    dsUri.setParam( QStringLiteral( "type" ), QgsWkbTypes::displayString( static_cast<Qgis::WkbType>(
                      parts.value( QStringLiteral( "type" ) ).toInt() ) ) );
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
    dsUri.setParam( QStringLiteral( "estimatedmetadata" ),
                    parts.value( QStringLiteral( "estimatedmetadata" ) ).toString() );
  if ( parts.contains( QStringLiteral( "sslmode" ) ) )
    dsUri.setParam( QStringLiteral( "sslmode" ),
                    QgsDataSourceUri::encodeSslMode(
                      static_cast<QgsDataSourceUri::SslMode>( parts.value( QStringLiteral( "sslmode" ) ).toInt() ) ) );
  if ( parts.contains( QStringLiteral( "sql" ) ) )
    dsUri.setSql( parts.value( QStringLiteral( "sql" ) ).toString() );
  if ( parts.contains( QStringLiteral( "checkPrimaryKeyUnicity" ) ) )
    dsUri.setParam( QStringLiteral( "checkPrimaryKeyUnicity" ),
                    parts.value( QStringLiteral( "checkPrimaryKeyUnicity" ) ).toString() );
  if ( parts.contains( QStringLiteral( "geometrycolumn" ) ) )
    dsUri.setGeometryColumn( parts.value( QStringLiteral( "geometrycolumn" ) ).toString() );
  return dsUri.uri( false );
}
