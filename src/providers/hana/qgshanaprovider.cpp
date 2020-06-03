/***************************************************************************
   qgshanaprovider.cpp  -  Data provider for SAP HANA
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgis.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgshanaconnectionpool.h"
#include "qgshanaexception.h"
#include "qgshanadriver.h"
#include "qgshanafeatureiterator.h"
#include "qgshanaprovider.h"
#include "qgshanaproviderconnection.h"
#include "qgshanaresultset.h"
#include "qgshanautils.h"
#ifdef HAVE_GUI
#include "qgshanadataitems.h"
#include "qgshanasourceselect.h"
#include "qgssourceselectprovider.h"
#endif
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsrectangle.h"

#include "ogr_srs_api.h"

#include <ctype.h>

#include "odbc/PreparedStatement.h"
#include "odbc/ResultSetMetaData.h"

using namespace odbc;
using namespace std;

namespace
{

  QString buildQuery( const QString &query, const QString &whereClause )
  {
    if ( whereClause.trimmed().isEmpty() )
      return query;
    return query + QStringLiteral( " WHERE " ) + whereClause;
  }


  void createCoordinateSystem( QgsHanaConnectionRef &conn, const QgsCoordinateReferenceSystem &srs )
  {
    OGRSpatialReferenceH hCRS = nullptr;
    hCRS = OSRNewSpatialReference( nullptr );
    int errcode = OSRImportFromProj4( hCRS, srs.toProj().toUtf8() );

    if ( errcode != OGRERR_NONE )
      throw exception();

    QgsCoordinateReferenceSystem srsWGS84;
    srsWGS84.createFromString( "EPSG:4326" );

    QgsCoordinateTransform transform;
    transform.setSourceCrs( srsWGS84 );
    transform.setDestinationCrs( srs );
    QgsRectangle bounds = transform.transformBoundingBox( srs.bounds() );

    char *linearUnits = nullptr;
    char *angularUnits = nullptr;
    OSRGetLinearUnits( hCRS, &linearUnits );
    OSRGetAngularUnits( hCRS, &angularUnits );

    // create new spatial reference system
    QString sql = QStringLiteral( "CREATE SPATIAL REFERENCE SYSTEM \"%1\" "
                                  "IDENTIFIED BY %2 "
                                  "LINEAR UNIT OF MEASURE \"%3\" "
                                  "ANGULAR UNIT OF MEASURE \"%4\" "
                                  "TYPE %5 "
                                  "COORDINATE X BETWEEN %6 "
                                  "COORDINATE Y BETWEEN %7 "
                                  "DEFINITION '%8' "
                                  "TRANSFORM DEFINITION '%9'" )
                  .arg( srs.description(), QString::number( srs.postgisSrid() ), QString( linearUnits ).toLower(), QString( angularUnits ).toLower(),
                        srs.isGeographic() ? QStringLiteral( "ROUND EARTH" ) : QStringLiteral( "PLANAR" ),
                        QStringLiteral( "%1 AND %2" ).arg( QString::number( bounds.xMinimum() ), QString::number( bounds.xMaximum() ) ),
                        QStringLiteral( "%1 AND %2" ).arg( QString::number( bounds.yMinimum() ), QString::number( bounds.yMaximum() ) ),
                        srs.toWkt(), srs.toProj() );

    QString errorMessage;
    conn->execute( sql, &errorMessage );

    if ( errorMessage.isEmpty() )
      QgsDebugMsg( errorMessage );
  }

  void setStatementValue(
    PreparedStatementRef &stmt,
    unsigned short paramIndex,
    const QgsField &field,
    const FieldInfo &fieldInfo,
    const QVariant &value )
  {
    bool isNull = ( value.isNull() || !value.isValid() );

    switch ( fieldInfo.type )
    {
      case SQLDataTypes::Bit:
      case SQLDataTypes::Boolean:
        stmt->setBoolean( paramIndex, isNull ? Boolean() : Boolean( value.toBool() ) );
        break;
      case SQLDataTypes::TinyInt:
        if ( fieldInfo.isSigned )
          stmt->setByte( paramIndex, isNull ? Byte() : Byte( static_cast<int8_t>( value.toInt() ) ) );
        else
          stmt->setUByte( paramIndex, isNull ? UByte() : UByte( static_cast<uint8_t>( value.toUInt() ) ) );
        break;
      case SQLDataTypes::SmallInt:
        if ( fieldInfo.isSigned )
          stmt->setShort( paramIndex, isNull ? Short() : Short( static_cast<int16_t>( value.toInt() ) ) );
        else
          stmt->setUShort( paramIndex, isNull ? UShort() : UShort( static_cast<uint16_t>( value.toUInt() ) ) );
        break;
      case SQLDataTypes::Integer:
        if ( fieldInfo.isSigned )
          stmt->setInt( paramIndex, isNull ? Int() : Int( value.toInt() ) );
        else
          stmt->setUInt( paramIndex, isNull ? UInt() : UInt( value.toUInt() ) );
        break;
      case SQLDataTypes::BigInt:
        if ( fieldInfo.isSigned )
          stmt->setLong( paramIndex, isNull ? Long() : Long( value.toLongLong() ) );
        else
          stmt->setULong( paramIndex, isNull ? ULong() : ULong( value.toULongLong() ) );
        break;
      case SQLDataTypes::Numeric:
      case SQLDataTypes::Decimal:
        stmt->setDecimal( paramIndex, isNull ? Decimal() :
                          makeNullable<decimal>( value.toString().toStdString(), field.length(), field.precision() ) );
        break;
      case SQLDataTypes::Real:
        stmt->setFloat( paramIndex, isNull ? Float() : Float( value.toFloat() ) );
        break;
      case SQLDataTypes::Float:
      case SQLDataTypes::Double:
        stmt->setDouble( paramIndex, isNull ? Double() : Double( value.toDouble() ) );
        break;
      case SQLDataTypes::Date:
      case SQLDataTypes::TypeDate:
        if ( isNull )
          stmt->setDate( paramIndex, Date() );
        else
        {
          QDate d = value.toDate();
          stmt->setDate( paramIndex, makeNullable<date>( d.year(), d.month(), d.day() ) );
        }
        break;
      case SQLDataTypes::Time:
      case SQLDataTypes::TypeTime:
        if ( isNull )
          stmt->setTime( paramIndex, Time() );
        else
        {
          QTime t = value.toTime();
          stmt->setTime( paramIndex, makeNullable<odbc::time>( t.hour(), t.minute(), t.second() ) );
        }
        break;
      case SQLDataTypes::Timestamp:
      case SQLDataTypes::TypeTimestamp:
        if ( isNull )
          stmt->setTimestamp( paramIndex, Timestamp() );
        else
        {
          QDateTime dt = value.toDateTime();
          QDate d = dt.date();
          QTime t = dt.time();
          stmt->setTimestamp( paramIndex, makeNullable<odbc::timestamp>( d.year(),
                              d.month(), d.day(), t.hour(), t.minute(), t.second(), t.msec() ) );
        }
        break;
      case SQLDataTypes::Char:
      case SQLDataTypes::VarChar:
      case SQLDataTypes::LongVarChar:
        stmt->setString( paramIndex, isNull ? String() : String( value.toString().toStdString() ) );
        break;
      case SQLDataTypes::WChar:
      case SQLDataTypes::WVarChar:
      case SQLDataTypes::WLongVarChar:
        stmt->setNString( paramIndex, isNull ? NString() : NString( value.toString().toStdU16String() ) );
        break;
      case SQLDataTypes::Binary:
      case SQLDataTypes::VarBinary:
      case SQLDataTypes::LongVarBinary:
        if ( isNull )
          stmt->setBinary( paramIndex, Binary() );
        else
        {
          QByteArray arr = value.toByteArray();
          vector<char> buffer( arr.begin(), arr.end() );
          stmt->setBinary( paramIndex, Binary( buffer ) );
        }
        break;
      default:
        QgsDebugMsg( QStringLiteral( "Unknown value type ('%1') for parameter %2" )
                     .arg( QString::number( fieldInfo.type ), QString::number( paramIndex ) ) );
    }
  }
}

static const size_t MAX_BATCH_SIZE = 4098;

const QString QgsHanaProvider::HANA_KEY = QStringLiteral( "hana" );
const QString QgsHanaProvider::HANA_DESCRIPTION = QStringLiteral( "HANA spatial data provider" );

QgsHanaProvider::QgsHanaProvider(
  const QString &uri,
  const ProviderOptions &options )
  : QgsVectorDataProvider( uri, options )
  , mUri( uri )
  , mFeaturesCount( -1 )
{
  mSchemaName = mUri.schema();
  mTableName = mUri.table();
  mFidColumn = mUri.keyColumn();
  mGeometryColumn = mUri.geometryColumn();
  mQueryWhereClause = mUri.sql();
  mRequestedGeometryType = mUri.wkbType();
  mSrid = ( !mUri.srid().isEmpty() ) ? mUri.srid().toInt() : -1;
  mSelectAtIdDisabled = mUri.selectAtIdDisabled();
  mHasSrsPlanarEquivalent = false;

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
      mQuery += QgsHanaUtils::quotedIdentifier( mSchemaName ) + '.';
    if ( !mTableName.isEmpty() )
      mQuery += QgsHanaUtils::quotedIdentifier( mTableName );
    mQuery = QStringLiteral( "SELECT * FROM " ) + mQuery;
  }

  QgsHanaConnectionRef connRef( mUri );
  if ( connRef.isNull() )
    return;

  if ( !checkPermissionsAndSetCapabilities() )
  {
    QgsMessageLog::logMessage( tr( "Provider does not have enough permissions" ), QObject::tr( "HANA" ) );
    return;
  }

  if ( mSrid < 0 )
    mSrid = readSrid();

  mDatabaseVersion = QgsHanaUtils::toHANAVersion( connRef->getDatabaseVersion() );
  readGeometryType();
  readAttributeFields();
  readSrsInformation();
  readMetadata();

  //fill type names into sets
  setNativeTypes( QList< NativeType >()
                  // boolean
                  << QgsVectorDataProvider::NativeType( tr( "Boolean" ), QStringLiteral( "BOOLEAN" ), QVariant::Bool, -1, -1, -1, -1 )
                  // integer types
                  << QgsVectorDataProvider::NativeType( tr( "8 bytes integer" ), QStringLiteral( "BIGINT" ), QVariant::Int )
                  << QgsVectorDataProvider::NativeType( tr( "4 bytes integer" ), QStringLiteral( "INTEGER" ), QVariant::Int )
                  << QgsVectorDataProvider::NativeType( tr( "2 bytes integer" ), QStringLiteral( "SMALLINT" ), QVariant::Int )
                  << QgsVectorDataProvider::NativeType( tr( "1 byte integer" ), QStringLiteral( "TINYINT" ), QVariant::Int )
                  << QgsVectorDataProvider::NativeType( tr( "Decimal number (DECIMAL)" ), QStringLiteral( "DECIMAL" ), QVariant::Double, 1, 31, 0, 31 )
                  // floating point
                  << QgsVectorDataProvider::NativeType( tr( "Decimal number (REAL)" ), QStringLiteral( "REAL" ), QVariant::Double )
                  << QgsVectorDataProvider::NativeType( tr( "Decimal number (DOUBLE)" ), QStringLiteral( "DOUBLE" ), QVariant::Double )
                  // date/time types
                  << QgsVectorDataProvider::NativeType( tr( "Date" ), QStringLiteral( "DATE" ), QVariant::Date, -1, -1, -1, -1 )
                  << QgsVectorDataProvider::NativeType( tr( "Time" ), QStringLiteral( "TIME" ), QVariant::Time, -1, -1, -1, -1 )
                  << QgsVectorDataProvider::NativeType( tr( "Date & Time" ), QStringLiteral( "TIMESTAMP" ), QVariant::DateTime, -1, -1, -1, -1 )
                  // string types
                  << QgsVectorDataProvider::NativeType( tr( "Text, variable length (VARCHAR)" ), QStringLiteral( "VARCHAR" ), QVariant::String, 1, 5000 )
                  << QgsVectorDataProvider::NativeType( tr( "Unicode text, variable length (NVARCHAR)" ), QStringLiteral( "NVARCHAR" ), QVariant::String, 1, 5000 )
                  << QgsVectorDataProvider::NativeType( tr( "Text, variable length large object (CLOB)" ), QStringLiteral( "CLOB" ), QVariant::String )
                  << QgsVectorDataProvider::NativeType( tr( "Unicode text, variable length large object (NCLOB)" ), QStringLiteral( "NCLOB" ), QVariant::String )
                );

  mValid = true;

  QgsDebugMsgLevel( QStringLiteral( "Connection info is %1" ).arg( QgsHanaUtils::connectionInfo( mUri ) ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "Schema is: %1" ).arg( mSchemaName ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "Table name is: %1" ).arg( mTableName ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "Geometry column is: %1" ).arg( mGeometryColumn ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "Query is: %1" ).arg( mQuery ), 4 );
}

QgsHanaProvider::~QgsHanaProvider()
{
  QgsDebugMsgLevel( QStringLiteral( "deconstructing." ), 4 );
}

QgsAbstractFeatureSource *QgsHanaProvider::featureSource() const
{
  return new QgsHanaFeatureSource( this );
}

QString QgsHanaProvider::storageType() const
{
  return QObject::tr( "SAP HANA database" );
}

QgsVectorDataProvider::Capabilities QgsHanaProvider::capabilities() const
{
  return mCapabilities;
}

QgsRectangle QgsHanaProvider::extent() const
{
  if ( mLayerExtent.isEmpty() )
    mLayerExtent = estimateExtent();
  return mLayerExtent;
}

void QgsHanaProvider::updateExtents()
{
  mLayerExtent.setMinimal();
}

QgsWkbTypes::Type QgsHanaProvider::wkbType() const
{
  return mRequestedGeometryType != QgsWkbTypes::Unknown ? mRequestedGeometryType : mDetectedGeometryType;
}

QgsLayerMetadata QgsHanaProvider::layerMetadata() const
{
  return mLayerMetadata;
}

QString QgsHanaProvider::dataComment() const
{
  return mLayerMetadata.abstract();
}

long QgsHanaProvider::featureCount() const
{
  if ( mFeaturesCount >= 0 )
    return mFeaturesCount;

  mFeaturesCount = getFeatureCount( mQueryWhereClause );
  return mFeaturesCount;
}

QgsAttributeList QgsHanaProvider::pkAttributeIndexes() const
{
  QgsAttributeList list;
  if ( !mFidColumn.isEmpty() )
  {
    int idx = mAttributeFields.indexFromName( mFidColumn );
    if ( idx >= 0 )
      list << idx;
  }
  return list;
}

QgsFields QgsHanaProvider::fields() const
{
  return mAttributeFields;
}

// Returns the minimum value of an attribute
QVariant QgsHanaProvider::minimumValue( int index ) const
{
  if ( index < 0 || index >= mAttributeFields.count() )
    return QVariant();

  QgsField fld = mAttributeFields[ index ];
  QString sql = QStringLiteral( "SELECT MIN(%1) FROM (%2)" )
                .arg( QgsHanaUtils::quotedIdentifier( fld.name() ), buildQuery( mQuery, mQueryWhereClause ) );
  QgsHanaConnectionRef conn( mUri );
  return conn->executeScalar( sql );
}

// Returns the maximum value of an attribute
QVariant QgsHanaProvider::maximumValue( int index ) const
{
  if ( index < 0 || index >= mAttributeFields.count() )
    return QVariant();

  QgsField fld = mAttributeFields[ index ];
  QString sql = QStringLiteral( "SELECT MAX(%1) FROM (%2)" )
                .arg( QgsHanaUtils::quotedIdentifier( fld.name() ), buildQuery( mQuery, mQueryWhereClause ) );
  QgsHanaConnectionRef conn( mUri );
  return conn->executeScalar( sql );
}

// Returns the list of unique values of an attribute
QSet<QVariant> QgsHanaProvider::uniqueValues( int index, int limit ) const
{
  QSet<QVariant> uniqueValues;
  if ( index < 0 || index >= mAttributeFields.count() )
    return uniqueValues;

  QString fieldName = mAttributeFields[ index ].name();
  QString sql = QStringLiteral( "SELECT * FROM (SELECT DISTINCT %1 FROM (%2)) ORDER BY %1" )
                .arg( QgsHanaUtils::quotedIdentifier( fieldName ), buildQuery( mQuery, mQueryWhereClause ) );
  if ( limit >= 0 )
    sql += QStringLiteral( " LIMIT %1" ).arg( limit );

  QgsHanaConnectionRef conn( mUri );
  QgsHanaResultSetRef resultSet = conn->executeQuery( sql );
  while ( resultSet->next() )
  {
    uniqueValues.insert( resultSet->getValue( 1 ) );
  }
  resultSet->close();

  return uniqueValues;
}

QString QgsHanaProvider::subsetString() const
{
  return mQueryWhereClause;
}

bool QgsHanaProvider::setSubsetString( const QString &subset, bool )
{
  QString whereClause = subset.trimmed();
  if ( whereClause == mQueryWhereClause )
    return true;

  bool hasErrors = false;
  try
  {
    getFeatureCount( whereClause );
    mQueryWhereClause = whereClause;
  }
  catch ( const QgsHanaException &ex )
  {
    hasErrors = true;
    pushError( ex.what() );
  }

  if ( hasErrors )
    return false;

  QgsDataSourceUri anUri = QgsDataSourceUri( dataSourceUri() );
  anUri.setSql( mQueryWhereClause );
  setDataSourceUri( anUri.uri() );
  mLayerExtent.setMinimal();
  mFeaturesCount = -1;

  emit dataChanged();

  return true;
}

bool QgsHanaProvider::isValid() const
{
  return mValid;
}

QgsFeatureIterator QgsHanaProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  if ( !mValid )
  {
    QgsDebugMsg( QStringLiteral( "Read attempt on an invalid HANA data source" ) );
    return QgsFeatureIterator();
  }

  return QgsFeatureIterator( new QgsHanaFeatureIterator( new QgsHanaFeatureSource( this ), true, request ) );
}

bool QgsHanaProvider::addFeatures( QgsFeatureList &flist, Flags flags )
{
  if ( flist.isEmpty() )
    return true;

  if ( mIsQuery )
    return false;

  QgsHanaConnectionRef conn( mUri );
  if ( conn.isNull() )
    return false;

  // Build insert statement
  QString columnNames;
  QString values;
  bool first = true;
  bool isPointDataType = false;

  if ( !mGeometryColumn.isEmpty() )
  {
    QString geometryDataType = conn->getColumnDataType( mSchemaName, mTableName, mGeometryColumn );
    isPointDataType = QString::compare( geometryDataType, "ST_POINT", Qt::CaseInsensitive ) == 0;

    columnNames += QgsHanaUtils::quotedIdentifier( mGeometryColumn );
    values += QStringLiteral( "ST_GeomFromWKB(?, %1)" ).arg( QString::number( mSrid ) );
    first = false;
  }

  const QgsAttributes attrs = flist[0].attributes();
  const bool hasSkippedFieldds = attrs.count() != mAttributeFields.size();
  QList<int> fieldIds;
  int idFieldIndex = -1;

  for ( int idx = 0; idx < mAttributeFields.count(); ++idx )
  {
    const QgsField &field = mAttributeFields.at( idx );
    if ( field.name().isEmpty() || field.name() == mGeometryColumn )
      continue;

    if ( field.name() == mFidColumn )
    {
      idFieldIndex = idx;
      const FieldInfo &fieldInfo = mFieldInfos.at( idx );
      if ( fieldInfo.isAutoIncrement )
        continue;
    }

    if ( !first )
    {
      columnNames += QStringLiteral( "," );
      values += QStringLiteral( "," );
    }

    columnNames += QgsHanaUtils::quotedIdentifier( field.name() );
    values += QStringLiteral( "?" );
    first = false;

    fieldIds.append( idx );
  }

  const bool allowBatchInserts = ( flags & QgsFeatureSink::FastInsert );
  const QString sql = QStringLiteral( "INSERT INTO %1.%2(%3) VALUES (%4)" ).arg(
                        QgsHanaUtils::quotedIdentifier( mSchemaName ), QgsHanaUtils::quotedIdentifier( mTableName ),
                        columnNames, values );

  size_t batchSize = 0;

  try
  {
    PreparedStatementRef stmtInsert = conn->prepareStatement( sql );
    PreparedStatementRef stmtIdentityValue;
    if ( !allowBatchInserts )
    {
      QString sqlIdentity = QStringLiteral( "SELECT CURRENT_IDENTITY_VALUE() \"current identity value\" FROM %1.%2" )
                            .arg( QgsHanaUtils::quotedIdentifier( mSchemaName ), QgsHanaUtils::quotedIdentifier( mTableName ) );
      stmtIdentityValue = conn->prepareStatement( sqlIdentity );
    }

    for ( auto &feature : flist )
    {
      unsigned short paramIndex = 1;

      if ( !mGeometryColumn.isEmpty() )
      {
        if ( feature.hasGeometry() )
        {
          const QgsGeometry &geom = feature.geometry();
          if ( isPointDataType && geom.wkbType() != wkbType() )
          {
            throw QgsHanaException( tr( "Could not add feature with geometry type %1 to layer of type %2" )
                                    .arg( QgsWkbTypes::displayString( geom.wkbType() ), QgsWkbTypes::displayString( wkbType() ) ).toStdString().c_str() );
          }

          QByteArray wkb = geom.asWkb();
          stmtInsert->setBinary( paramIndex, makeNullable<vector<char>>( wkb.begin(), wkb.end() ) );
        }
        else
        {
          stmtInsert->setBinary( paramIndex, Binary() );
        }

        ++paramIndex;
      }

      bool hasIdValue = false;
      const QgsAttributes attrs = feature.attributes();
      for ( int i = 0; i < fieldIds.size(); ++i )
      {
        const int fieldIndex = fieldIds[i];
        const QgsField &field = mAttributeFields.at( fieldIndex );
        const FieldInfo &fieldInfo = mFieldInfos.at( fieldIndex );

        const int attrIndex = ( hasSkippedFieldds ) ? i : fieldIndex;
        QVariant attrValue = attrIndex < attrs.length() ? attrs.at( attrIndex ) : QVariant( QVariant::LongLong );
        if ( fieldIndex == idFieldIndex )
        {
          hasIdValue = !attrValue.isNull();
          if ( !hasIdValue && !fieldInfo.isNullable )
            attrValue = 0;
        }
        else
        {
          if ( !fieldInfo.isNullable && attrValue.isNull() )
            attrValue = mDefaultValues[fieldIndex];
        }

        setStatementValue( stmtInsert, paramIndex, field, fieldInfo, attrValue );
        ++paramIndex;
      }

      if ( allowBatchInserts )
      {
        stmtInsert->addBatch();
        ++batchSize;

        if ( batchSize >= MAX_BATCH_SIZE )
        {
          stmtInsert->executeBatch();
          batchSize = 0;
        }
      }
      else
      {
        stmtInsert->executeUpdate();
        stmtInsert->clearParameters();

        if ( hasIdValue )
          feature.setId( static_cast<QgsFeatureId>( attrs.at( idFieldIndex ).toLongLong() ) );
        else
        {
          ResultSetRef rsIdentity = stmtIdentityValue->executeQuery();
          if ( rsIdentity->next() )
          {
            odbc::Long id = rsIdentity->getLong( 1 );
            if ( !id.isNull() )
              feature.setId( static_cast<QgsFeatureId>( *id ) );
          }
          rsIdentity->close();
        }
      }
    }

    if ( allowBatchInserts && batchSize > 0 )
      stmtInsert->executeBatch();

    conn->commit();

    mFeaturesCount = -1;
    return true;
  }
  catch ( const exception &ex )
  {
    pushError( tr( "HANA error while adding features: %1" )
               .arg( QgsHanaUtils::formatErrorMessage( ex.what(), false ) ) );
    conn->rollback();
  }

  return false;
}

bool QgsHanaProvider::deleteFeatures( const QgsFeatureIds &id )
{
  if ( mFidColumn.isEmpty() )
    return false;

  if ( mIsQuery )
  {
    QgsDebugMsg( QStringLiteral( "Cannot delete features (is a query)" ) );
    return false;
  }

  if ( id.empty() )
    return true; // for consistency providers return true to an empty list

  QgsHanaConnectionRef conn( mUri );
  if ( conn.isNull() )
    return false;

  QString featureIds;

  for ( auto featId : id )
  {
    if ( featureIds.isEmpty() )
      featureIds = FID_TO_STRING( featId );
    else
      featureIds += ',' + FID_TO_STRING( featId );
  }

  QString sql = QStringLiteral( "DELETE FROM %1.%2 WHERE %3 IN (%4)" ).arg(
                  QgsHanaUtils::quotedIdentifier( mSchemaName ), QgsHanaUtils::quotedIdentifier( mTableName ),
                  QgsHanaUtils::quotedIdentifier( mFidColumn ), featureIds );

  try
  {
    conn->execute( sql );
    conn->commit();
  }
  catch ( const QgsHanaException &ex )
  {
    pushError( tr( "HANA failed to delete features: %1" )
               .arg( QgsHanaUtils::formatErrorMessage( ex.what(), false ) ) );
    conn->rollback();
    return false;
  }

  return true;
}

bool QgsHanaProvider::truncate()
{
  if ( mIsQuery )
  {
    QgsDebugMsg( QStringLiteral( "Cannot truncate (is a query)" ) );
    return false;
  }

  QgsHanaConnectionRef conn( mUri );
  if ( conn.isNull() )
    return false;

  QString sql = QStringLiteral( "TRUNCATE TABLE %1.%2" ).arg(
                  QgsHanaUtils::quotedIdentifier( mSchemaName ), QgsHanaUtils::quotedIdentifier( mTableName ) );

  try
  {
    conn->execute( sql );
    conn->commit();
  }
  catch ( const QgsHanaException &ex )
  {
    pushError( tr( "HANA failed to truncate: %1" ).arg( QgsHanaUtils::formatErrorMessage( ex.what() ) ) );
    conn->rollback();
    return false;
  }

  return true;
}

bool QgsHanaProvider::addAttributes( const QList<QgsField> &attributes )
{
  if ( attributes.isEmpty() )
    return true;

  QgsHanaConnectionRef conn( mUri );
  if ( conn.isNull() )
    return false;

  QString columnDefs;
  for ( const QgsField &field : attributes )
  {
    if ( !columnDefs.isEmpty() )
      columnDefs += QStringLiteral( "," );

    columnDefs += QgsHanaUtils::quotedIdentifier( field.name() ) + " " + field.typeName();

    if ( !field.comment().isEmpty() )
      columnDefs += QStringLiteral( " COMMENT " ) + QgsHanaUtils::quotedString( field.comment() );
  }

  QString sql = QStringLiteral( "ALTER TABLE %1.%2 ADD (%3)" ).arg(
                  QgsHanaUtils::quotedIdentifier( mSchemaName ), QgsHanaUtils::quotedIdentifier( mTableName ), columnDefs );

  try
  {
    conn->execute( sql );
    conn->commit();
  }
  catch ( const QgsHanaException &ex )
  {
    pushError( tr( "HANA failed to add feature: %1" )
               .arg( QgsHanaUtils::formatErrorMessage( ex.what(), false ) ) );
    conn->rollback();
    return false;
  }

  readAttributeFields();

  return true;
}

bool QgsHanaProvider::deleteAttributes( const QgsAttributeIds &attributes )
{
  if ( attributes.isEmpty() )
    return false;

  QgsHanaConnectionRef conn( mUri );
  if ( conn.isNull() )
    return false;

  QString columnNames;
  for ( int attrId : attributes )
  {
    if ( !columnNames.isEmpty() )
      columnNames += QStringLiteral( "," );
    const QgsField &field = mAttributeFields.at( attrId );
    columnNames += QStringLiteral( "%1" ).arg( QgsHanaUtils::quotedIdentifier( field.name() ) );
  }

  QString sql = QStringLiteral( "ALTER TABLE %1.%2 DROP (%3)" ).arg(
                  QgsHanaUtils::quotedIdentifier( mSchemaName ), QgsHanaUtils::quotedIdentifier( mTableName ), columnNames );

  try
  {
    conn->execute( sql );
    conn->commit();
  }
  catch ( const QgsHanaException &ex )
  {
    pushError( tr( "HANA error while deleting attributes: %1" )
               .arg( QgsHanaUtils::formatErrorMessage( ex.what(), false ) ) );
    conn->rollback();
    return false;
  }

  readAttributeFields();

  return true;
}

bool QgsHanaProvider::renameAttributes( const QgsFieldNameMap &fieldMap )
{
  if ( mIsQuery )
    return false;

  QgsHanaConnectionRef conn( mUri );
  if ( conn.isNull() )
    return false;

  try
  {
    for ( QgsFieldNameMap::const_iterator it = fieldMap.begin(); it != fieldMap.end(); ++it )
    {
      int fieldIndex = it.key();
      if ( fieldIndex < 0 || fieldIndex >= mAttributeFields.count() )
      {
        pushError( tr( "Invalid attribute index: %1" ).arg( fieldIndex ) );
        return false;
      }

      if ( mAttributeFields.indexFromName( it.value() ) >= 0 )
      {
        pushError( tr( "Error renaming field %1: name '%2' already exists" ).arg( fieldIndex ).arg( it.value() ) );
        return false;
      }

      QString sql = QStringLiteral( "RENAME COLUMN %1.%2.%3 TO %4" ).arg(
                      QgsHanaUtils::quotedIdentifier( mSchemaName ), QgsHanaUtils::quotedIdentifier( mTableName ),
                      QgsHanaUtils::quotedIdentifier( mAttributeFields.at( fieldIndex ).name() ),
                      QgsHanaUtils::quotedIdentifier( it.value() ) );
      conn->execute( sql );
    }

    conn->commit();
  }
  catch ( const QgsHanaException &ex )
  {
    pushError( tr( "HANA error while renaming attributes: %1" )
               .arg( QgsHanaUtils::formatErrorMessage( ex.what(), false ) ) );
    conn->rollback();
    return false;
  }

  readAttributeFields();

  return true;
}

bool QgsHanaProvider::changeGeometryValues( const QgsGeometryMap &geometryMap )
{
  if ( geometryMap.isEmpty() )
    return true;

  if ( mIsQuery )
    return false;

  if ( mGeometryColumn.isEmpty() )
    return false;

  QgsHanaConnectionRef conn( mUri );
  if ( conn.isNull() )
    return false;

  QString sql = QStringLiteral( "UPDATE %1.%2 SET %3 = ST_GeomFromWKB(?, %4) WHERE %5 = ?" ).arg(
                  QgsHanaUtils::quotedIdentifier( mSchemaName ), QgsHanaUtils::quotedIdentifier( mTableName ),
                  QgsHanaUtils::quotedIdentifier( mGeometryColumn ), QString::number( mSrid ),
                  QgsHanaUtils::quotedIdentifier( mFidColumn ) );

  try
  {
    PreparedStatementRef stmt = conn->prepareStatement( sql );

    for ( QgsGeometryMap::const_iterator it = geometryMap.begin(); it != geometryMap.end(); ++it )
    {
      QgsFeatureId fid = it.key();
      // skip added features
      if ( FID_IS_NEW( fid ) )
        continue;

      QByteArray wkb = it->asWkb();
      stmt->setBinary( 1, makeNullable<vector<char>>( wkb.begin(), wkb.end() ) );
      stmt->setLong( 2, fid );
      stmt->executeUpdate();
    }

    conn->commit();
  }
  catch ( const exception &ex )
  {
    pushError( tr( "HANA error while changing feature geometry: %1" )
               .arg( QgsHanaUtils::formatErrorMessage( ex.what(), false ) ) );
    conn->rollback();
    return false;
  }

  return true;
}

bool QgsHanaProvider::changeFeatures( const QgsChangedAttributesMap &attrMap,
                                      const QgsGeometryMap &geometryMap )
{
  bool ret = changeAttributeValues( attrMap );
  if ( ret )
    ret = changeGeometryValues( geometryMap );
  return ret;
}

bool QgsHanaProvider::changeAttributeValues( const QgsChangedAttributesMap &attrMap )
{
  if ( mIsQuery || mFidColumn.isEmpty() )
    return false;

  if ( attrMap.isEmpty() )
    return true;

  QgsHanaConnectionRef conn( mUri );
  if ( conn.isNull() )
    return false;

  try
  {
    for ( QgsChangedAttributesMap::const_iterator attrIt = attrMap.begin(); attrIt != attrMap.end(); ++attrIt )
    {
      QgsFeatureId fid = attrIt.key();

      // skip added features
      if ( FID_IS_NEW( fid ) )
        continue;

      const QgsAttributeMap &attrs = attrIt.value();
      if ( attrs.isEmpty() )
        continue;

      QString sql = QStringLiteral( "UPDATE %1.%2 SET " ).arg(
                      QgsHanaUtils::quotedIdentifier( mSchemaName ), QgsHanaUtils::quotedIdentifier( mTableName ) );

      bool first = true;
      for ( QgsAttributeMap::const_iterator it2 = attrs.begin(); it2 != attrs.end(); ++it2 )
      {
        int fieldIndex = it2.key();
        const QgsField &field = mAttributeFields.at( fieldIndex );
        const FieldInfo &fieldInfo = mFieldInfos.at( fieldIndex );

        if ( field.name().isEmpty() || fieldInfo.isAutoIncrement )
          continue;

        if ( !first )
          sql += ',';
        else
          first = false;

        sql += QStringLiteral( "%1=?" ).arg( QgsHanaUtils::quotedIdentifier( field.name() ) );
      }

      if ( first )
        return true;

      if ( !mFidColumn.isEmpty() )
        sql += QStringLiteral( " WHERE %1=%2" ).arg( QgsHanaUtils::quotedIdentifier( mFidColumn ), FID_TO_STRING( fid ) );

      PreparedStatementRef stmt = conn->prepareStatement( sql );

      unsigned short paramIndex = 1;
      for ( QgsAttributeMap::const_iterator attrIt = attrs.begin(); attrIt != attrs.end(); ++attrIt )
      {
        int fieldIndex = attrIt.key();
        const QgsField &field = mAttributeFields.at( fieldIndex );
        const FieldInfo &fieldInfo = mFieldInfos.at( fieldIndex );

        if ( field.name().isEmpty() || fieldInfo.isAutoIncrement )
          continue;

        setStatementValue( stmt, paramIndex, field, fieldInfo, *attrIt );
        ++paramIndex;
      }

      stmt->executeUpdate();
    }

    conn->commit();
  }
  catch ( const exception &ex )
  {
    pushError( tr( "HANA error while changing feature attributes: %1" )
               .arg( QgsHanaUtils::formatErrorMessage( ex.what(), false ) ) );
    conn->rollback();
    return false;
  }

  return true;
}

QVariant QgsHanaProvider::defaultValue( int fieldId ) const
{
  return mDefaultValues.value( fieldId, QVariant() );
}

QString QgsHanaProvider::name() const
{
  return HANA_KEY;
}

QString QgsHanaProvider::description() const
{
  return HANA_DESCRIPTION;
}

bool QgsHanaProvider::checkPermissionsAndSetCapabilities()
{
  QgsHanaConnectionRef conn( mUri );
  if ( conn.isNull() )
    return false;

  if ( !mSelectAtIdDisabled )
    mCapabilities = QgsVectorDataProvider::SelectAtId;

  // Read access permissions
  if ( mIsQuery )
  {
    // Any changes are not allowed for queries
  }
  else
  {
    QString sql = QStringLiteral( "SELECT OBJECT_NAME, OBJECT_TYPE, PRIVILEGE FROM PUBLIC.EFFECTIVE_PRIVILEGES "
                                  "WHERE USER_NAME = CURRENT_USER AND SCHEMA_NAME = ? AND IS_VALID = 'TRUE'" );
    QgsHanaResultSetRef rsPrivileges = conn->executeQuery( sql, { mSchemaName} );
    while ( rsPrivileges->next() )
    {
      QString objName = rsPrivileges->getString( 1 );

      if ( !objName.isEmpty() && objName != mTableName )
        break;

      QString objType = rsPrivileges->getString( 2 );
      QString privType = rsPrivileges->getString( 3 );

      if ( privType == QStringLiteral( "ALL PRIVILEGES" ) || privType == QStringLiteral( "CREATE ANY" ) )
      {
        mCapabilities |= QgsVectorDataProvider::AddAttributes
                         | QgsVectorDataProvider::RenameAttributes
                         | QgsVectorDataProvider::AddFeatures
                         | QgsVectorDataProvider::DeleteAttributes
                         | QgsVectorDataProvider::DeleteFeatures
                         | QgsVectorDataProvider::FastTruncate
                         | QgsVectorDataProvider::ChangeAttributeValues
                         | QgsVectorDataProvider::ChangeFeatures
                         | QgsVectorDataProvider::ChangeGeometries;
      }
      else
      {
        if ( privType == QStringLiteral( "ALTER" ) )
          mCapabilities |= QgsVectorDataProvider::DeleteAttributes
                           | QgsVectorDataProvider::RenameAttributes;
        else if ( privType == QStringLiteral( "DELETE" ) )
          mCapabilities |= QgsVectorDataProvider::DeleteFeatures
                           | QgsVectorDataProvider::FastTruncate;
        else if ( privType == QStringLiteral( "INSERT" ) )
          mCapabilities |= QgsVectorDataProvider::AddAttributes
                           | QgsVectorDataProvider::AddFeatures;
        else if ( privType == QStringLiteral( "UPDATE" ) )
          mCapabilities |= QgsVectorDataProvider::ChangeAttributeValues
                           | QgsVectorDataProvider::ChangeFeatures
                           | QgsVectorDataProvider::ChangeGeometries;
      }
    }
    rsPrivileges->close();

    if ( mFidColumn.isEmpty() )
      mCapabilities &= ~( QgsVectorDataProvider::DeleteFeatures
                          | QgsVectorDataProvider::ChangeAttributeValues
                          | QgsVectorDataProvider::ChangeFeatures );
  }

  // TODO needs to be implemented in QgsHanaFeatureIterator class
  // supports geometry simplification on provider side
  //mCapabilities |= (QgsVectorDataProvider::SimplifyGeometries);
  // QgsVectorDataProvider::SimplifyGeometriesWithTopologicalValidation feature
  // is not supported in HANA QgsVectorDataProvider::SimplifyGeometriesWithTopologicalValidation

  mCapabilities |= QgsVectorDataProvider::TransactionSupport;

  mCapabilities |= QgsVectorDataProvider::CircularGeometries;

  mCapabilities |= QgsVectorDataProvider::ReadLayerMetadata;

  return true;
}

QgsRectangle QgsHanaProvider::estimateExtent() const
{
  if ( mGeometryColumn.isEmpty() )
    return QgsRectangle();

  QgsHanaConnectionRef conn( mUri );
  bool isRoundEarth = isSrsRoundEarth( mSrid );
  QString sql;
  if ( isRoundEarth )
  {
    QString geomColumn = !mHasSrsPlanarEquivalent ? QgsHanaUtils::quotedIdentifier( mGeometryColumn ) :
                         QStringLiteral( "%1.ST_SRID(%2)" ).arg( QgsHanaUtils::quotedIdentifier( mGeometryColumn ), QString::number( QgsHanaUtils::toPlanarSRID( mSrid ) ) );
    sql = QStringLiteral( "SELECT MIN(%1.ST_XMin()), MIN(%1.ST_YMin()), MAX(%1.ST_XMax()), MAX(%1.ST_YMax()) FROM (%2)" )
          .arg( geomColumn, buildQuery( mQuery, mQueryWhereClause ) );
  }
  else
  {
    sql = QStringLiteral( "SELECT \"ext\".ST_XMin(),\"ext\".ST_YMin(),\"ext\".ST_XMax(),"
                          "\"ext\".ST_YMax() FROM (SELECT ST_EnvelopeAggr(%1) AS \"ext\" FROM (%2))" )
          .arg( QgsHanaUtils::quotedIdentifier( mGeometryColumn ), buildQuery( mQuery, mQueryWhereClause ) );
  }

  QgsHanaResultSetRef rsExtent = conn->executeQuery( sql );
  QgsRectangle ret;
  if ( rsExtent->next() )
  {
    QVariant val = rsExtent->getValue( 1 );
    if ( !val.isNull() )
    {
      ret.setXMinimum( val.toDouble() );
      ret.setYMinimum( rsExtent->getValue( 2 ).toDouble() );
      ret.setXMaximum( rsExtent->getValue( 3 ).toDouble() );
      ret.setYMaximum( rsExtent->getValue( 4 ).toDouble() );
    }
  }
  rsExtent->close();

  return ret;
}

bool QgsHanaProvider::isSrsRoundEarth( int srsId ) const
{
  if ( mGeometryColumn.isEmpty() )
    return false;

  QString sql = QStringLiteral( "SELECT ROUND_EARTH FROM SYS.ST_SPATIAL_REFERENCE_SYSTEMS WHERE SRS_ID = ?" );
  QgsHanaConnectionRef conn( mUri );
  QVariant roundEarth = conn->executeScalar( sql, { srsId} );
  return roundEarth.toString() == QLatin1String( "TRUE" );
}

int QgsHanaProvider::readSrid()
{
  if ( mGeometryColumn.isEmpty() )
    return -1;

  QString sql = QStringLiteral( "SELECT SRS_ID FROM SYS.ST_GEOMETRY_COLUMNS WHERE SCHEMA_NAME = ? AND TABLE_NAME = ?" );
  QVariantList args = { mSchemaName, mTableName};
  if ( !mGeometryColumn.isEmpty() )
  {
    sql += QStringLiteral( " AND COLUMN_NAME = ?" );
    args.append( mGeometryColumn );
  }
  QgsHanaConnectionRef conn( mUri );
  QVariant srid = conn->executeScalar( sql, args );
  return srid.isNull() ? -1 : srid.toInt();
}

void QgsHanaProvider::readAttributeFields()
{
  mAttributeFields.clear();
  mFieldInfos.clear();
  mDefaultValues.clear();

  QgsHanaConnectionRef conn( mUri );

  QString sql = QStringLiteral( "SELECT * FROM (%1) LIMIT 0" ).arg( mQuery );
  QgsHanaResultSetRef rsAttributes = conn->executeQuery( sql );
  ResultSetMetaData &rsmd = rsAttributes->getMetadata();
  for ( unsigned short i = 1; i <= rsmd.getColumnCount(); ++i )
  {
    const QString fieldName( rsmd.getColumnName( i ).c_str() );
    if ( fieldName == mGeometryColumn )
      continue;

    QVariant::Type fieldType = QVariant::Invalid;
    const short sqlType = rsmd.getColumnType( i );
    const QString fieldTypeName( rsmd.getColumnTypeName( i ).c_str() );
    const bool isSigned = rsmd.isSigned( i );
    int fieldSize = static_cast<int>( rsmd.getColumnLength( i ) );
    int fieldPrec = -1;

    switch ( sqlType )
    {
      case SQLDataTypes::Bit:
      case SQLDataTypes::Boolean:
        fieldType = QVariant::Bool;
        break;
      case SQLDataTypes::TinyInt:
        fieldType = QVariant::UInt;
        break;
      case SQLDataTypes::SmallInt:
      case SQLDataTypes::Integer:
        fieldType = isSigned ? QVariant::Int : QVariant::UInt;
        break;
      case SQLDataTypes::BigInt:
        fieldType = isSigned ? QVariant::LongLong : QVariant::ULongLong;
        break;
      case SQLDataTypes::Numeric:
      case SQLDataTypes::Decimal:
        fieldType = QVariant::Double;
        fieldSize = rsmd.getPrecision( i );
        fieldPrec = rsmd.getScale( i );
        break;
      case SQLDataTypes::Double:
      case SQLDataTypes::Float:
      case SQLDataTypes::Real:
        fieldType = QVariant::Double;
        break;
      case SQLDataTypes::Char:
      case SQLDataTypes::WChar:
        fieldType = ( fieldSize == 1 ) ? QVariant::Char : QVariant::String;
        break;
      case SQLDataTypes::VarChar:
      case SQLDataTypes::WVarChar:
      case SQLDataTypes::LongVarChar:
      case SQLDataTypes::WLongVarChar:
        fieldType = QVariant::String;
        break;
      case SQLDataTypes::Binary:
      case SQLDataTypes::VarBinary:
        fieldType = QVariant::ByteArray;
        break;
      case SQLDataTypes::Date:
      case SQLDataTypes::TypeDate:
        fieldType = QVariant::Date;
        break;
      case SQLDataTypes::Time:
      case SQLDataTypes::TypeTime:
        fieldType = QVariant::Time;
        break;
      case SQLDataTypes::Timestamp:
      case SQLDataTypes::TypeTimestamp:
        fieldType = QVariant::DateTime;
        break;
      default:
        break;
    }

    if ( fieldType != QVariant::Invalid )
    {
      QgsField newField = QgsField( fieldName, fieldType, fieldTypeName, fieldSize, fieldPrec, QString(), QVariant::Invalid );

      bool isNullable = rsmd.isNullable( i );
      bool isAutoIncrement = rsmd.isAutoIncrement( i );
      if ( !isNullable || isAutoIncrement )
      {
        if ( !isNullable && isAutoIncrement && mFidColumn.isEmpty() )
          mFidColumn = fieldName;

        QgsFieldConstraints constraints;
        if ( !isNullable )
          constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
        if ( isAutoIncrement )
          constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
        newField.setConstraints( constraints );
      }

      mAttributeFields.append( newField );
      mFieldInfos.append( { sqlType, isAutoIncrement, isNullable, isSigned } );

      QString schemaName( rsmd.getSchemaName( i ).c_str() );
      if ( schemaName.isEmpty() )
        schemaName = mSchemaName;
      QString tableName( rsmd.getTableName( i ).c_str() );
      if ( tableName.isEmpty() )
        tableName = mTableName;
      QgsHanaResultSetRef rsColumns = conn->getColumns( schemaName, tableName, fieldName );
      if ( rsColumns->next() )
        mDefaultValues.insert( mAttributeFields.size() - 1, rsColumns->getValue( 13/*COLUMN_DEF*/ ) );
      rsColumns->close();
    }
  }
  rsAttributes->close();
}

void QgsHanaProvider::readGeometryType()
{
  if ( mGeometryColumn.isNull() || mGeometryColumn.isEmpty() )
    mDetectedGeometryType = QgsWkbTypes::NoGeometry;

  QgsHanaLayerProperty layerProperty;
  layerProperty.tableName = mTableName;
  layerProperty.schemaName = mSchemaName;
  layerProperty.geometryColName = mGeometryColumn;
  QgsHanaConnectionRef conn( mUri );
  mDetectedGeometryType = conn->getLayerGeometryType( layerProperty );
}

void QgsHanaProvider::readMetadata()
{
  QString sql = QStringLiteral( "SELECT COMMENTS FROM TABLES WHERE SCHEMA_NAME = ? AND TABLE_NAME = ?" );
  QgsHanaConnectionRef conn( mUri );
  QVariant comment = conn->executeScalar( sql, { mSchemaName, mTableName } );
  if ( !comment.isNull() )
    mLayerMetadata.setAbstract( comment.toString() );
  mLayerMetadata.setType( QStringLiteral( "dataset" ) );
  mLayerMetadata.setCrs( crs() );
}

void QgsHanaProvider::readSrsInformation()
{
  if ( mGeometryColumn.isEmpty() )
    return;

  QgsRectangle ext;
  bool isRoundEarth = false;
  QString sql = QStringLiteral( "SELECT MIN_X, MIN_Y, MAX_X, MAX_Y, ROUND_EARTH FROM SYS.ST_SPATIAL_REFERENCE_SYSTEMS "
                                "WHERE SRS_ID = ?" );
  QgsHanaConnectionRef conn( mUri );
  QgsHanaResultSetRef rs = conn->executeQuery( sql, { mSrid } );
  if ( rs->next() )
  {
    ext.setXMinimum( rs->getDouble( 1 ) );
    ext.setYMinimum( rs->getDouble( 2 ) );
    ext.setXMaximum( rs->getDouble( 3 ) );
    ext.setYMaximum( rs->getDouble( 4 ) );
    isRoundEarth = ( rs->getString( 5 ) == QLatin1String( "TRUE" ) );
  }
  rs->close();
  mSrsExtent = ext;

  if ( isRoundEarth )
  {
    sql = QStringLiteral( "SELECT COUNT(*) FROM SYS.ST_SPATIAL_REFERENCE_SYSTEMS WHERE SRS_ID = ?" );
    mHasSrsPlanarEquivalent = conn->executeCountQuery( sql, { QgsHanaUtils::toPlanarSRID( mSrid ) } ) > 0;
  }
}

long QgsHanaProvider::getFeatureCount( const QString &whereClause ) const
{
  QString sql = QStringLiteral( "SELECT COUNT(*) FROM (%1)" ).arg( buildQuery( mQuery, whereClause ) );
  QgsHanaConnectionRef conn( mUri );
  size_t count = conn->executeCountQuery( sql );
  return static_cast<long>( count );
}

QgsCoordinateReferenceSystem QgsHanaProvider::crs() const
{
  QgsCoordinateReferenceSystem srs;

  static QMutex sMutex;
  QMutexLocker locker( &sMutex );
  static QMap<int, QgsCoordinateReferenceSystem> sCrsCache;
  if ( sCrsCache.contains( mSrid ) )
  {
    srs = sCrsCache.value( mSrid );
    return srs;
  }

  QgsHanaConnectionRef conn( mUri );
  srs = conn->getCrs( mSrid );

  if ( srs.isValid() )
    sCrsCache.insert( mSrid, srs );

  return srs;
}

QgsVectorLayerExporter::ExportError QgsHanaProvider::createEmptyLayer(
  const QString &uri,
  const QgsFields &fields,
  QgsWkbTypes::Type wkbType,
  const QgsCoordinateReferenceSystem &srs,
  bool overwrite,
  QMap<int, int> *oldToNewAttrIdxMap,
  QString *errorMessage,
  const QMap<QString, QVariant> *
)
{
  QgsDataSourceUri dsUri( uri );

  QgsHanaConnectionRef conn( dsUri );
  if ( conn.isNull() )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Connection to database failed" );
    return QgsVectorLayerExporter::ErrConnectionFailed;
  }

  QString schemaName = dsUri.schema();
  QString tableName = dsUri.table();

  if ( schemaName.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Schema name cannot be empty" );
    return QgsVectorLayerExporter::ErrCreateLayer;
  }

  QString geometryColumn = dsUri.geometryColumn();
  QString geometryType;

  QString primaryKey = dsUri.keyColumn();
  QString primaryKeyType;

  QString schemaTableName = QgsHanaUtils::quotedIdentifier( schemaName ) + '.' +
                            QgsHanaUtils::quotedIdentifier( tableName );

  if ( wkbType != QgsWkbTypes::NoGeometry && geometryColumn.isEmpty() )
    geometryColumn = "geom";

  bool fieldsInUpperCase = false;
  if ( fields.size() > 0 )
  {
    int count = QgsHanaUtils::countFieldsWithFirstLetterInUppercase( fields );
    fieldsInUpperCase = count > fields.size() / 2;
  }

  bool createdNewPk = false;

  if ( primaryKey.isEmpty() )
  {
    QString pk = primaryKey = fieldsInUpperCase ? "ID" : "id";
    int index = 0;
    while ( fields.indexFromName( primaryKey ) >= 0 )
    {
      primaryKey = QStringLiteral( "%1_%2" ).arg( pk ).arg( index++ );
    }

    createdNewPk = true;
  }
  else
  {
    int idx = fields.indexFromName( primaryKey );
    if ( idx >= 0 )
    {
      QgsField fld = fields.at( idx );
      if ( QgsHanaUtils::convertField( fld ) )
        primaryKeyType = fld.typeName();
    }
  }

  if ( primaryKeyType.isEmpty() )
    primaryKeyType = QStringLiteral( "BIGINT" );

  QString sql;

  // set up spatial reference id
  long srid = 0;
  if ( srs.isValid() )
  {
    srid = srs.postgisSrid();
    QString authSrid = QStringLiteral( "null" );
    QString authName = QStringLiteral( "null" );
    QStringList sl = srs.authid().split( ':' );
    if ( sl.length() == 2 )
    {
      authName = '\'' + sl[0] + '\'';
      authSrid = sl[1];
    }

    try
    {
      sql = QStringLiteral( "SELECT COUNT(*) FROM SYS.ST_SPATIAL_REFERENCE_SYSTEMS "
                            "WHERE SRS_ID = ? AND ORGANIZATION = ? AND ORGANIZATION_COORDSYS_ID = ?" );
      size_t numCrs = conn->executeCountQuery( sql, { static_cast<qulonglong>( srid ), authName, authSrid} );
      if ( numCrs == 0 )
        createCoordinateSystem( conn, srs );
    }
    catch ( ... )
    {
      if ( errorMessage )
        *errorMessage = QObject::tr( "Connection to database failed" );
      return QgsVectorLayerExporter::ErrConnectionFailed;
    }
  }

  sql = QStringLiteral( "SELECT COUNT(*) FROM SYS.TABLES WHERE SCHEMA_NAME = ? AND TABLE_NAME = ?" );
  size_t numTables = conn->executeCountQuery( sql, {schemaName, tableName} );
  if ( numTables != 0 )
  {
    if ( overwrite )
    {
      QString sql = QStringLiteral( "DROP TABLE %1.%2" )
                    .arg( QgsHanaUtils::quotedIdentifier( schemaName ), QgsHanaUtils::quotedIdentifier( tableName ) );
      if ( !conn->execute( sql, errorMessage ) )
        return QgsVectorLayerExporter::ErrCreateLayer;
    }
    else
    {
      if ( errorMessage )
        *errorMessage = QObject::tr( "Table %1.%2 already exists" ).arg( schemaName, tableName );

      return QgsVectorLayerExporter::ErrCreateLayer;
    }
  }

  if ( geometryColumn.isEmpty() )
  {
    sql = QStringLiteral( "CREATE COLUMN TABLE %1 (%2 %3 GENERATED BY DEFAULT AS IDENTITY, PRIMARY KEY (%2))" )
          .arg( schemaTableName, QgsHanaUtils::quotedIdentifier( primaryKey ), primaryKeyType );
  }
  else
  {
    sql = QStringLiteral( "CREATE COLUMN TABLE %1 (%2 %3 GENERATED BY DEFAULT AS IDENTITY, %4 ST_GEOMETRY(%5), PRIMARY KEY (%2))" )
          .arg( schemaTableName, QgsHanaUtils::quotedIdentifier( primaryKey ), primaryKeyType,
                QgsHanaUtils::quotedIdentifier( geometryColumn ), QString::number( srid ) );
  }

  if ( !conn->execute( sql, errorMessage ) )
    return QgsVectorLayerExporter::ErrCreateLayer;

  dsUri.setDataSource( dsUri.schema(), dsUri.table(), geometryColumn, dsUri.sql(), primaryKey );
  dsUri.setSrid( QString::number( srid ) );

  QgsDataProvider::ProviderOptions providerOptions;
  unique_ptr< QgsHanaProvider > provider = qgis::make_unique< QgsHanaProvider >( dsUri.uri( false ), providerOptions );

  if ( !provider->isValid() )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Loading of the layer %1 failed" ).arg( schemaTableName );

    return QgsVectorLayerExporter::ErrInvalidLayer;
  }

  // add fields to the layer
  if ( oldToNewAttrIdxMap )
    oldToNewAttrIdxMap->clear();

  if ( fields.size() > 0 )
  {
    int offset = createdNewPk ? 1 : 0;

    QList<QgsField> flist;
    for ( int i = 0, n = fields.size(); i < n; ++i )
    {
      QgsField fld = fields.at( i );
      if ( oldToNewAttrIdxMap && fld.name() == primaryKey )
      {
        oldToNewAttrIdxMap->insert( fields.lookupField( fld.name() ), 0 );
        continue;
      }

      if ( fld.name() == geometryColumn )
        continue;

      if ( !QgsHanaUtils::convertField( fld ) )
      {
        if ( errorMessage )
          *errorMessage = QObject::tr( "Unsupported type for field %1" ).arg( fld.name() );

        return QgsVectorLayerExporter::ErrAttributeTypeUnsupported;
      }

      flist.append( fld );
      if ( oldToNewAttrIdxMap )
        oldToNewAttrIdxMap->insert( fields.lookupField( fld.name() ), offset++ );
    }

    if ( !provider->addAttributes( flist ) )
    {
      if ( errorMessage )
        *errorMessage = QObject::tr( "Creation of fields failed" );

      return QgsVectorLayerExporter::ErrAttributeCreationFailed;
    }
  }

  return QgsVectorLayerExporter::NoError;
}

QgsHanaProviderMetadata::QgsHanaProviderMetadata()
  : QgsProviderMetadata( QgsHanaProvider::HANA_KEY, QgsHanaProvider::HANA_DESCRIPTION )
{
}

void QgsHanaProviderMetadata::cleanupProvider()
{
  QgsHanaConnectionPool::cleanupInstance();
  QgsHanaDriver::cleanupInstance();
}

QgsHanaProvider *QgsHanaProviderMetadata::createProvider(
  const QString &uri, const QgsDataProvider::ProviderOptions &options )
{
  return new QgsHanaProvider( uri, options );
}

QList< QgsDataItemProvider *> QgsHanaProviderMetadata::dataItemProviders() const
{
  QList<QgsDataItemProvider *> providers;
  providers << new QgsHanaDataItemProvider;
  return providers;
}

QgsVectorLayerExporter::ExportError QgsHanaProviderMetadata::createEmptyLayer(
  const QString &uri,
  const QgsFields &fields,
  QgsWkbTypes::Type wkbType,
  const QgsCoordinateReferenceSystem &srs,
  bool overwrite,
  QMap<int, int> &oldToNewAttrIdxMap,
  QString &errorMessage,
  const QMap<QString, QVariant> *options )
{
  return QgsHanaProvider::createEmptyLayer(
           uri, fields, wkbType, srs, overwrite,
           &oldToNewAttrIdxMap, &errorMessage, options
         );
}

QMap<QString, QgsAbstractProviderConnection *> QgsHanaProviderMetadata::connections( bool cached )
{
  return connectionsProtected<QgsHanaProviderConnection, QgsHanaConnection>( cached );
}

QgsAbstractProviderConnection *QgsHanaProviderMetadata::createConnection( const QString &name )
{
  return new QgsHanaProviderConnection( name );
}

QgsAbstractProviderConnection *QgsHanaProviderMetadata::createConnection( const QString &uri, const QVariantMap &configuration )
{
  return new QgsHanaProviderConnection( uri, configuration );
}

void QgsHanaProviderMetadata::deleteConnection( const QString &name )
{
  deleteConnectionProtected<QgsHanaProviderConnection>( name );
}

void QgsHanaProviderMetadata::saveConnection( const QgsAbstractProviderConnection *conn,  const QString &name )
{
  saveConnectionProtected( conn, name );
}

QVariantMap QgsHanaProviderMetadata::decodeUri( const QString &uri )
{
  const QgsDataSourceUri dsUri { uri };
  QVariantMap uriParts;

  if ( ! dsUri.driver().isEmpty() )
    uriParts[ QStringLiteral( "driver" ) ] = dsUri.driver();
  if ( ! dsUri.database().isEmpty() )
    uriParts[ QStringLiteral( "dbname" ) ] = dsUri.database();
  if ( ! dsUri.host().isEmpty() )
    uriParts[ QStringLiteral( "host" ) ] = dsUri.host();
  if ( ! dsUri.port().isEmpty() )
    uriParts[ QStringLiteral( "port" ) ] = dsUri.port();
  if ( ! dsUri.username().isEmpty() )
    uriParts[ QStringLiteral( "username" ) ] = dsUri.username();
  if ( ! dsUri.password().isEmpty() )
    uriParts[ QStringLiteral( "password" ) ] = dsUri.password();
  if ( ! dsUri.authConfigId().isEmpty() )
    uriParts[ QStringLiteral( "authcfg" ) ] = dsUri.authConfigId();
  if ( dsUri.wkbType() != QgsWkbTypes::Type::Unknown )
    uriParts[ QStringLiteral( "type" ) ] = dsUri.wkbType();

  uriParts[ QStringLiteral( "selectatid" ) ] = dsUri.selectAtIdDisabled();

  if ( ! dsUri.schema().isEmpty() )
    uriParts[ QStringLiteral( "schema" ) ] = dsUri.schema();
  if ( ! dsUri.table().isEmpty() )
    uriParts[ QStringLiteral( "table" ) ] = dsUri.table();
  if ( ! dsUri.keyColumn().isEmpty() )
    uriParts[ QStringLiteral( "key" ) ] = dsUri.keyColumn();
  if ( ! dsUri.srid().isEmpty() )
    uriParts[ QStringLiteral( "srid" ) ] = dsUri.srid();

  if ( dsUri.hasParam( QStringLiteral( "sslEnabled" ) ) )
  {
    QString value = dsUri.param( QStringLiteral( "sslEnabled" ) );
    if ( ! value.isEmpty() )
      uriParts[ QStringLiteral( "sslEnabled" ) ]  = value;
  }
  if ( dsUri.hasParam( QStringLiteral( "sslCryptoProvider" ) ) )
  {
    QString value = dsUri.param( QStringLiteral( "sslCryptoProvider" ) );
    if ( ! value.isEmpty() )
      uriParts[ QStringLiteral( "sslCryptoProvider" ) ]  = value;
  }
  if ( dsUri.hasParam( QStringLiteral( "sslValidateCertificate" ) ) )
  {
    QString value = dsUri.param( QStringLiteral( "sslValidateCertificate" ) );
    if ( ! value.isEmpty() )
      uriParts[ QStringLiteral( "sslValidateCertificate" ) ]  = value;
  }
  if ( dsUri.hasParam( QStringLiteral( "sslHostNameInCertificate" ) ) )
  {
    QString value = dsUri.param( QStringLiteral( "sslHostNameInCertificate" ) );
    if ( ! value.isEmpty() )
      uriParts[ QStringLiteral( "sslHostNameInCertificate" ) ]  = value;
  }
  if ( dsUri.hasParam( QStringLiteral( "sslKeyStore" ) ) )
  {
    QString value = dsUri.param( QStringLiteral( "sslKeyStore" ) );
    if ( ! value.isEmpty() )
      uriParts[ QStringLiteral( "sslKeyStore" ) ]  = value;
  }
  if ( dsUri.hasParam( QStringLiteral( "sslTrustStore" ) ) )
  {
    QString value = dsUri.param( QStringLiteral( "sslTrustStore" ) );
    if ( ! value.isEmpty() )
      uriParts[ QStringLiteral( "sslTrustStore" ) ]  = value;
  }

  if ( ! dsUri.sql().isEmpty() )
    uriParts[ QStringLiteral( "sql" ) ] = dsUri.sql();
  if ( ! dsUri.geometryColumn().isEmpty() )
    uriParts[ QStringLiteral( "geometrycolumn" ) ] = dsUri.geometryColumn();

  return uriParts;
}

QString QgsHanaProviderMetadata::encodeUri( const QVariantMap &parts )
{
  QgsDataSourceUri dsUri;
  if ( parts.contains( QStringLiteral( "driver" ) ) )
    dsUri.setDriver( parts.value( QStringLiteral( "driver" ) ).toString() );
  if ( parts.contains( QStringLiteral( "dbname" ) ) )
    dsUri.setDatabase( parts.value( QStringLiteral( "dbname" ) ).toString() );
  if ( parts.contains( QStringLiteral( "host" ) ) )
    dsUri.setParam( QStringLiteral( "host" ), parts.value( QStringLiteral( "host" ) ).toString() );
  if ( parts.contains( QStringLiteral( "port" ) ) )
    dsUri.setParam( QStringLiteral( "port" ), parts.value( QStringLiteral( "port" ) ).toString() );
  if ( parts.contains( QStringLiteral( "username" ) ) )
    dsUri.setUsername( parts.value( QStringLiteral( "username" ) ).toString() );
  if ( parts.contains( QStringLiteral( "password" ) ) )
    dsUri.setPassword( parts.value( QStringLiteral( "password" ) ).toString() );
  if ( parts.contains( QStringLiteral( "authcfg" ) ) )
    dsUri.setAuthConfigId( parts.value( QStringLiteral( "authcfg" ) ).toString() );

  if ( parts.contains( QStringLiteral( "type" ) ) )
    dsUri.setParam( QStringLiteral( "type" ), QgsWkbTypes::displayString( static_cast<QgsWkbTypes::Type>( parts.value( QStringLiteral( "type" ) ).toInt() ) ) );
  if ( parts.contains( QStringLiteral( "selectatid" ) ) )
    dsUri.setParam( QStringLiteral( "selectatid" ), parts.value( QStringLiteral( "selectatid" ) ).toString() );
  if ( parts.contains( QStringLiteral( "schema" ) ) )
    dsUri.setSchema( parts.value( QStringLiteral( "schema" ) ).toString() );
  if ( parts.contains( QStringLiteral( "table" ) ) )
    dsUri.setTable( parts.value( QStringLiteral( "table" ) ).toString() );
  if ( parts.contains( QStringLiteral( "key" ) ) )
    dsUri.setParam( QStringLiteral( "key" ), parts.value( QStringLiteral( "key" ) ).toString() );
  if ( parts.contains( QStringLiteral( "srid" ) ) )
    dsUri.setSrid( parts.value( QStringLiteral( "srid" ) ).toString() );

  if ( parts.contains( QStringLiteral( "sslEnabled" ) ) )
    dsUri.setParam( QStringLiteral( "sslEnabled" ), parts.value( QStringLiteral( "sslEnabled" ) ).toString() );
  if ( parts.contains( QStringLiteral( "sslCryptoProvider" ) ) )
    dsUri.setParam( QStringLiteral( "sslCryptoProvider" ), parts.value( QStringLiteral( "sslCryptoProvider" ) ).toString() );
  if ( parts.contains( QStringLiteral( "sslValidateCertificate" ) ) )
    dsUri.setParam( QStringLiteral( "sslValidateCertificate" ), parts.value( QStringLiteral( "sslValidateCertificate" ) ).toString() );
  if ( parts.contains( QStringLiteral( "sslHostNameInCertificate" ) ) )
    dsUri.setParam( QStringLiteral( "sslHostNameInCertificate" ), parts.value( QStringLiteral( "sslHostNameInCertificate" ) ).toString() );
  if ( parts.contains( QStringLiteral( "sslKeyStore" ) ) )
    dsUri.setParam( QStringLiteral( "sslKeyStore" ), parts.value( QStringLiteral( "sslKeyStore" ) ).toString() );
  if ( parts.contains( QStringLiteral( "sslTrustStore" ) ) )
    dsUri.setParam( QStringLiteral( "sslTrustStore" ), parts.value( QStringLiteral( "sslTrustStore" ) ).toString() );

  if ( parts.contains( QStringLiteral( "sql" ) ) )
    dsUri.setSql( parts.value( QStringLiteral( "sql" ) ).toString() );
  if ( parts.contains( QStringLiteral( "geometrycolumn" ) ) )
    dsUri.setGeometryColumn( parts.value( QStringLiteral( "geometrycolumn" ) ).toString() );
  return dsUri.uri();
}

QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsHanaProviderMetadata();
}
