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
#include "qgshanaprovider.h"

#include <odbc/PreparedStatement.h>
#include <odbc/ResultSet.h>
#include <odbc/ResultSetMetaDataUnicode.h>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsdataprovider.h"
#include "qgserror.h"
#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgshanaconnectionpool.h"
#include "qgshanacrsutils.h"
#include "qgshanadataitems.h"
#include "qgshanadatatypes.h"
#include "qgshanadriver.h"
#include "qgshanaexception.h"
#include "qgshanafeatureiterator.h"
#include "qgshanaprimarykeys.h"
#include "qgshanaproviderconnection.h"
#include "qgshanaresultset.h"
#include "qgshanautils.h"
#include "qgslogger.h"
#include "qgsrectangle.h"
#include "qgsthreadingutils.h"

#include <QtGlobal>

#include "moc_qgshanaprovider.cpp"

using namespace NS_ODBC;
using namespace std;

namespace
{
  bool sourceIsQuery( const QString &source )
  {
    QString trimmed = source.trimmed();
    return trimmed.startsWith( '(' ) && trimmed.endsWith( ')' );
  }

  QString buildQuery( const QString &source, const QString &columns, const QString &where, const QString &orderBy, int limit )
  {
    if ( sourceIsQuery( source ) && columns == "*"_L1 && where.isEmpty() && limit <= 0 )
      return source;

    QString sql = u"SELECT %1 FROM %2"_s.arg( columns, source );
    if ( !where.isEmpty() )
      sql += u" WHERE "_s + where;
    if ( !orderBy.isEmpty() )
      sql += u" ORDER BY "_s + orderBy;
    if ( limit >= 0 )
      sql += u" LIMIT "_s + QString::number( limit );
    return sql;
  }

  void checkAndCreateUnitOfMeasure( QgsHanaConnection &conn, const QString &name, const QString &type, double conversionFactor )
  {
    QString sql = u"SELECT COUNT(*) FROM SYS.ST_UNITS_OF_MEASURE WHERE UNIT_NAME = ? AND UNIT_TYPE = ?"_s;
    size_t numUnits = conn.executeCountQuery( sql, { name, type } );
    if ( numUnits > 0 )
      return;

    sql = u"SELECT COUNT(*) FROM SYS.ST_UNITS_OF_MEASURE WHERE UNIT_NAME = ?"_s;
    numUnits = conn.executeCountQuery( sql, { name } );
    if ( numUnits > 0 )
      throw QgsHanaException( QObject::tr( "Unable to create a new unit of measure. "
                                           "Unit of measure with name '%1' and different type already exist." )
                                .arg( name ) );

    sql = u"CREATE SPATIAL UNIT OF MEASURE %1 TYPE %2 CONVERT USING %3"_s.arg( QgsHanaUtils::quotedIdentifier( name ), type, QString::number( conversionFactor ) );
    conn.execute( sql );
  }

  void createCoordinateSystem( QgsHanaConnection &conn, const QgsCoordinateReferenceSystem &srs )
  {
    QString authName;
    long srid;
    if ( !QgsHanaCrsUtils::identifyCrs( srs, authName, srid ) )
    {
      QString errorMessage = QObject::tr( "Unable to retrieve the authority identifier for an CRS with id = %1." ).arg( srs.authid() );
      throw QgsHanaException( errorMessage.toStdString().c_str() );
    }

    QString units = QgsHanaUtils::toString( srs.mapUnits() );
    checkAndCreateUnitOfMeasure( conn, units, srs.isGeographic() ? u"ANGULAR"_s : u"LINEAR"_s, QgsHanaCrsUtils::getAngularUnits( srs ) );

    QgsCoordinateReferenceSystem srsWGS84;
    srsWGS84.createFromString( u"EPSG:4326"_s );
    QgsCoordinateTransformContext coordTransCntx;
    QgsCoordinateTransform ct( srsWGS84, srs, coordTransCntx );
    QgsRectangle bounds = ct.transformBoundingBox( srs.bounds() );

    QString linearUnits = srs.isGeographic() ? u"NULL"_s : QgsHanaUtils::quotedIdentifier( units );
    QString angularUnits = srs.isGeographic() ? QgsHanaUtils::quotedIdentifier( units ) : u"NULL"_s;

    QString xRange = u"%1 BETWEEN %2 AND %3"_s
                       .arg( ( srs.isGeographic() ? u"LONGITUDE"_s : u"X"_s ), QString::number( bounds.xMinimum() ), QString::number( bounds.xMaximum() ) );
    QString yRange = u"%1 BETWEEN %2 AND %3"_s
                       .arg( ( srs.isGeographic() ? u"LATITUDE"_s : u"Y"_s ), QString::number( bounds.yMinimum() ), QString::number( bounds.yMaximum() ) );

    // create new spatial reference system
    QString sql = QStringLiteral( "CREATE SPATIAL REFERENCE SYSTEM %1 "
                                  "IDENTIFIED BY %2 "
                                  "LINEAR UNIT OF MEASURE %3 "
                                  "ANGULAR UNIT OF MEASURE %4 "
                                  "TYPE %5 "
                                  "COORDINATE %6 "
                                  "COORDINATE %7 "
                                  "ORGANIZATION %8 IDENTIFIED BY %9 "
                                  "DEFINITION %10 "
                                  "TRANSFORM DEFINITION %11" )
                    .arg( QgsHanaUtils::quotedIdentifier( srs.description() ), QString::number( srid ), linearUnits, angularUnits, srs.isGeographic() ? u"ROUND EARTH"_s : u"PLANAR"_s, xRange, yRange, QgsHanaUtils::quotedIdentifier( authName ), QString::number( srid ) )
                    .arg( QgsHanaUtils::quotedString( srs.toWkt() ), QgsHanaUtils::quotedString( srs.toProj() ) );

    QString errorMessage;
    conn.execute( sql, &errorMessage );

    if ( !errorMessage.isEmpty() )
      throw QgsHanaException( errorMessage.toStdString().c_str() );
  }

  QPair<QString, QString> determinePrimaryKeyColumn( const QgsFields &fields, const QString &keyColumn )
  {
    int index = fields.indexFromName( keyColumn );
    if ( index >= 0 )
    {
      QgsField field = fields.at( index );
      const QgsFieldConstraints &constraints = field.constraints();
      if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintNotNull ) == QgsFieldConstraints::ConstraintOriginProvider && constraints.constraintOrigin( QgsFieldConstraints::ConstraintUnique ) == QgsFieldConstraints::ConstraintOriginProvider )
      {
        if ( QgsHanaUtils::convertField( field ) )
          return qMakePair( field.name(), field.typeName() );
      }
    }

    QString primaryKey = keyColumn;
    index = 0;
    while ( fields.indexFromName( primaryKey ) >= 0 )
    {
      primaryKey = u"%1_%2"_s.arg( keyColumn ).arg( index++ );
    }

    return qMakePair( primaryKey, u"BIGINT"_s );
  }

  bool isSrsRoundEarth( QgsHanaConnection &conn, int srsId )
  {
    QString sql = u"SELECT ROUND_EARTH FROM SYS.ST_SPATIAL_REFERENCE_SYSTEMS WHERE SRS_ID = ?"_s;
    QVariant roundEarth = conn.executeScalar( sql, { srsId } );
    return roundEarth.toString() == "TRUE"_L1;
  }

  void setStatementValue(
    PreparedStatementRef &stmt,
    unsigned short paramIndex,
    const AttributeField &field,
    const QVariant &value
  )
  {
    bool isNull = ( value.isNull() || !value.isValid() );

    switch ( field.type )
    {
      case QgsHanaDataType::Bit:
      case QgsHanaDataType::Boolean:
        stmt->setBoolean( paramIndex, isNull ? Boolean() : Boolean( value.toBool() ) );
        break;
      case QgsHanaDataType::TinyInt:
        if ( field.isSigned )
          stmt->setByte( paramIndex, isNull ? Byte() : Byte( static_cast<int8_t>( value.toInt() ) ) );
        else
          stmt->setUByte( paramIndex, isNull ? UByte() : UByte( static_cast<uint8_t>( value.toUInt() ) ) );
        break;
      case QgsHanaDataType::SmallInt:
        if ( field.isSigned )
          stmt->setShort( paramIndex, isNull ? Short() : Short( static_cast<int16_t>( value.toInt() ) ) );
        else
          stmt->setUShort( paramIndex, isNull ? UShort() : UShort( static_cast<uint16_t>( value.toUInt() ) ) );
        break;
      case QgsHanaDataType::Integer:
        if ( field.isSigned )
          stmt->setInt( paramIndex, isNull ? Int() : Int( value.toInt() ) );
        else
          stmt->setUInt( paramIndex, isNull ? UInt() : UInt( value.toUInt() ) );
        break;
      case QgsHanaDataType::BigInt:
        if ( field.isSigned )
          stmt->setLong( paramIndex, isNull ? Long() : Long( value.toLongLong() ) );
        else
          stmt->setULong( paramIndex, isNull ? ULong() : ULong( value.toULongLong() ) );
        break;
      case QgsHanaDataType::Numeric:
      case QgsHanaDataType::Decimal:
        if ( isNull )
          stmt->setDouble( paramIndex, Double() );
        else
        {
          double dvalue = value.toDouble();
          stmt->setDouble( paramIndex, Double( dvalue ) );
        }
        break;
      case QgsHanaDataType::Real:
        stmt->setFloat( paramIndex, isNull ? Float() : Float( value.toFloat() ) );
        break;
      case QgsHanaDataType::Float:
      case QgsHanaDataType::Double:
        stmt->setDouble( paramIndex, isNull ? Double() : Double( value.toDouble() ) );
        break;
      case QgsHanaDataType::Date:
      case QgsHanaDataType::TypeDate:
        if ( isNull )
          stmt->setDate( paramIndex, Date() );
        else
        {
          QDate d = value.toDate();
          stmt->setDate( paramIndex, makeNullable<date>( d.year(), d.month(), d.day() ) );
        }
        break;
      case QgsHanaDataType::Time:
      case QgsHanaDataType::TypeTime:
        if ( isNull )
          stmt->setTime( paramIndex, Time() );
        else
        {
          QTime t = value.toTime();
          stmt->setTime( paramIndex, makeNullable<NS_ODBC::time>( t.hour(), t.minute(), t.second() ) );
        }
        break;
      case QgsHanaDataType::Timestamp:
      case QgsHanaDataType::TypeTimestamp:
        if ( isNull )
          stmt->setTimestamp( paramIndex, Timestamp() );
        else
        {
          QDateTime dt = value.toDateTime();
          QDate d = dt.date();
          QTime t = dt.time();
          stmt->setTimestamp( paramIndex, makeNullable<NS_ODBC::timestamp>( d.year(), d.month(), d.day(), t.hour(), t.minute(), t.second(), t.msec() ) );
        }
        break;
      case QgsHanaDataType::Char:
      case QgsHanaDataType::VarChar:
      case QgsHanaDataType::LongVarChar:
        stmt->setString( paramIndex, isNull ? String() : String( value.toString().toStdString() ) );
        break;
      case QgsHanaDataType::WChar:
      case QgsHanaDataType::WVarChar:
      case QgsHanaDataType::WLongVarChar:
        stmt->setNString( paramIndex, isNull ? NString() : NString( value.toString().toStdU16String() ) );
        break;
      case QgsHanaDataType::Binary:
      case QgsHanaDataType::VarBinary:
      case QgsHanaDataType::LongVarBinary:
        if ( isNull )
          stmt->setBinary( paramIndex, Binary() );
        else
        {
          QByteArray arr = value.toByteArray();
          stmt->setBinary( paramIndex, Binary( vector<char>( arr.begin(), arr.end() ) ) );
        }
        break;
      case QgsHanaDataType::Geometry:
      case QgsHanaDataType::RealVector:
        if ( isNull )
          stmt->setString( paramIndex, String() );
        else
        {
          if ( value.userType() == QMetaType::Type::QString )
            stmt->setString( paramIndex, String( value.toString().toStdString() ) );
          else if ( value.userType() == QMetaType::Type::QByteArray )
          {
            QByteArray arr = value.toByteArray();
            stmt->setBinary( paramIndex, Binary( vector<char>( arr.begin(), arr.end() ) ) );
          }
        }
        break;
      default:
        QgsDebugError( u"Unknown value type ('%1') for parameter %2"_s
                         .arg( QString::number( static_cast<int>( field.type ) ), QString::number( paramIndex ) ) );
        break;
    }
  }

  void setStatementFidValue(
    PreparedStatementRef &stmt,
    unsigned short paramIndex,
    const AttributeFields &fields,
    QgsHanaPrimaryKeyType pkType,
    const QList<int> &pkAttrs,
    QgsHanaPrimaryKeyContext &pkContext,
    QgsFeatureId featureId
  )
  {
    switch ( pkType )
    {
      case QgsHanaPrimaryKeyType::PktInt:
        stmt->setInt( paramIndex, QgsHanaPrimaryKeyUtils::fidToInt( featureId ) );
        break;
      case QgsHanaPrimaryKeyType::PktInt64:
      {
        QVariantList pkValues = pkContext.lookupKey( featureId );
        if ( pkValues.empty() )
          throw QgsHanaException( u"Key values for feature %1 not found."_s.arg( featureId ) );
        setStatementValue( stmt, paramIndex, fields.at( pkAttrs[0] ), pkValues[0] );
      }
      break;
      case QgsHanaPrimaryKeyType::PktFidMap:
      {
        QVariantList pkValues = pkContext.lookupKey( featureId );
        Q_ASSERT( pkValues.size() == pkAttrs.size() );
        if ( pkValues.empty() )
          throw QgsHanaException( u"Key values for feature %1 not found."_s.arg( featureId ) );

        for ( int i = 0; i < pkAttrs.size(); i++ )
        {
          const QVariant &value = pkValues[i];
          Q_ASSERT( !value.isNull() );
          setStatementValue( stmt, static_cast<unsigned short>( paramIndex + i ), fields.at( pkAttrs[i] ), value );
        }
      }
      break;
      case QgsHanaPrimaryKeyType::PktUnknown:
        // Q_ASSERT( false );
        break;
    }
  }
} // namespace

static const size_t MAXIMUM_BATCH_DATA_SIZE = 4 * 1024 * 1024;

const QString QgsHanaProvider::HANA_KEY = u"hana"_s;
const QString QgsHanaProvider::HANA_DESCRIPTION = u"SAP HANA spatial data provider"_s;

QgsHanaProvider::QgsHanaProvider(
  const QString &uri,
  const ProviderOptions &options, Qgis::DataProviderReadFlags flags
)
  : QgsVectorDataProvider( uri, options, flags )
  , mUri( uri )
  , mFeaturesCount( -1 )
  , mPrimaryKeyCntx( new QgsHanaPrimaryKeyContext )
{
  mSchemaName = mUri.schema();
  mTableName = mUri.table();
  mGeometryColumn = mUri.geometryColumn();
  mQueryWhereClause = mUri.sql();
  mRequestedGeometryType = mUri.wkbType();
  mSrid = ( !mUri.srid().isEmpty() ) ? mUri.srid().toInt() : -1;
  mSelectAtIdDisabled = mUri.selectAtIdDisabled();
  mHasSrsPlanarEquivalent = false;
  mUseEstimatedMetadata = mUri.useEstimatedMetadata();

  auto appendError = [this]( const QString &message ) {
    this->appendError( QgsErrorMessage( message, u"SAP HANA"_s ) );
  };

  QgsHanaConnectionRef conn( mUri );
  if ( conn.isNull() )
  {
    appendError( tr( "Connection to database failed" ) );
    return;
  }

  if ( sourceIsQuery( mTableName ) )
  {
    mIsQuery = true;
    mQuerySource = mTableName;
    mTableName.clear();
  }
  else
  {
    if ( mSchemaName.isEmpty() || mTableName.isEmpty() )
    {
      appendError( tr( "Schema or table name cannot be empty" ) );
      return;
    }

    mIsQuery = false;
    mQuerySource = u"%1.%2"_s.arg(
      QgsHanaUtils::quotedIdentifier( mSchemaName ),
      QgsHanaUtils::quotedIdentifier( mTableName )
    );
  }

  try
  {
    if ( !checkPermissionsAndSetCapabilities( *conn ) )
    {
      appendError( tr( "Provider does not have enough permissions" ) );
      return;
    }

    mDatabaseVersion = QgsHanaUtils::toHANAVersion( conn->getDatabaseVersion() );
    readGeometryType( *conn );
    readAttributeFields( *conn );
    readSrsInformation( *conn );
    readMetadata( *conn );

    setNativeTypes( conn->getNativeTypes() );
  }
  catch ( const exception &ex )
  {
    appendError( QgsHanaUtils::formatErrorMessage( ex.what() ) );
    return;
  }

  mValid = true;

  QgsDebugMsgLevel( u"Connection info is %1"_s.arg( QgsHanaUtils::connectionInfo( mUri ) ), 4 );
  QgsDebugMsgLevel( u"Schema is: %1"_s.arg( mSchemaName ), 4 );
  QgsDebugMsgLevel( u"Table name is: %1"_s.arg( mTableName ), 4 );
  QgsDebugMsgLevel( u"Geometry column is: %1"_s.arg( mGeometryColumn ), 4 );
  QgsDebugMsgLevel( u"Query source is: %1"_s.arg( mQuerySource ), 4 );
}

QgsAbstractFeatureSource *QgsHanaProvider::featureSource() const
{
  return new QgsHanaFeatureSource( this );
}

QString QgsHanaProvider::storageType() const
{
  return QObject::tr( "SAP HANA database" );
}

Qgis::VectorProviderCapabilities QgsHanaProvider::capabilities() const
{
  auto capabilities = mCapabilities;

  if ( mPrimaryKeyAttrs.isEmpty() )
    capabilities &= ~( Qgis::VectorProviderCapability::DeleteFeatures | Qgis::VectorProviderCapability::ChangeAttributeValues | Qgis::VectorProviderCapability::ChangeFeatures );

  return capabilities;
}

QgsRectangle QgsHanaProvider::extent() const
{
  if ( mLayerExtent.isEmpty() )
    mLayerExtent = estimateExtent( mUseEstimatedMetadata );
  return mLayerExtent;
}

void QgsHanaProvider::updateExtents()
{
  mLayerExtent.setNull();
}

Qgis::WkbType QgsHanaProvider::wkbType() const
{
  return mRequestedGeometryType != Qgis::WkbType::Unknown ? mRequestedGeometryType : mDetectedGeometryType;
}

QgsLayerMetadata QgsHanaProvider::layerMetadata() const
{
  return mLayerMetadata;
}

QString QgsHanaProvider::dataComment() const
{
  return mLayerMetadata.abstract();
}

long long QgsHanaProvider::featureCount() const
{
  if ( mFeaturesCount >= 0 )
    return mFeaturesCount;

  try
  {
    mFeaturesCount = getFeatureCount( mQueryWhereClause );
  }
  catch ( const QgsHanaException &ex )
  {
    pushError( tr( "Failed to retrieve feature count: %1" ).arg( ex.what() ) );
  }

  return mFeaturesCount;
}

QString QgsHanaProvider::geometryColumnName() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mGeometryColumn;
}

QgsFields QgsHanaProvider::fields() const
{
  return mFields;
}

// Returns the minimum value of an attribute
QVariant QgsHanaProvider::minimumValue( int index ) const
{
  if ( index < 0 || index >= mAttributeFields.count() )
    return QVariant();

  QgsHanaConnectionRef conn = createConnection();
  if ( !conn.isNull() )
  {
    QString sql = buildQuery( u"MIN(%1)"_s.arg( QgsHanaUtils::quotedIdentifier( mAttributeFields[index].name ) ) );

    try
    {
      return conn->executeScalar( sql );
    }
    catch ( const QgsHanaException &ex )
    {
      pushError( tr( "Failed to retrieve minimum value: %1" ).arg( ex.what() ) );
    }
  }
  return QVariant();
}

// Returns the maximum value of an attribute
QVariant QgsHanaProvider::maximumValue( int index ) const
{
  if ( index < 0 || index >= mAttributeFields.count() )
    return QVariant();

  QgsHanaConnectionRef conn = createConnection();
  if ( !conn.isNull() )
  {
    QString sql = buildQuery( u"MAX(%1)"_s.arg( QgsHanaUtils::quotedIdentifier( mAttributeFields[index].name ) ) );
    try
    {
      return conn->executeScalar( sql );
    }
    catch ( const QgsHanaException &ex )
    {
      pushError( tr( "Failed to retrieve maximum value: %1" ).arg( ex.what() ) );
    }
  }
  return QVariant();
}

// Returns the list of unique values of an attribute
QSet<QVariant> QgsHanaProvider::uniqueValues( int index, int limit ) const
{
  QSet<QVariant> uniqueValues;
  if ( index < 0 || index >= mAttributeFields.count() )
    return uniqueValues;

  QgsHanaConnectionRef conn = createConnection();
  if ( !conn.isNull() )
  {
    QString fieldName = mAttributeFields[index].name;
    QString sql = buildQuery( u"DISTINCT %1"_s.arg( QgsHanaUtils::quotedIdentifier( fieldName ) ), mQueryWhereClause, QgsHanaUtils::quotedIdentifier( fieldName ), limit );

    try
    {
      QgsHanaResultSetRef resultSet = conn->executeQuery( sql );
      while ( resultSet->next() )
      {
        uniqueValues.insert( resultSet->getValue( 1 ) );
      }
      resultSet->close();
    }
    catch ( const QgsHanaException &ex )
    {
      pushError( tr( "Failed to retrieve unique values: %1" ).arg( ex.what() ) );
    }
  }

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

  try
  {
    getFeatureCount( whereClause );
    mQueryWhereClause = whereClause;
  }
  catch ( const QgsHanaException &ex )
  {
    pushError( tr( "Failed to retrieve feature count: %1" ).arg( ex.what() ) );
    return false;
  }

  QgsDataSourceUri anUri = QgsDataSourceUri( dataSourceUri() );
  anUri.setSql( mQueryWhereClause );
  setDataSourceUri( anUri.uri() );
  mLayerExtent.setNull();
  mFeaturesCount = -1;

  emit dataChanged();

  return true;
}

bool QgsHanaProvider::supportsSubsetString() const
{
  return true;
}

QString QgsHanaProvider::subsetStringDialect() const
{
  return tr( "SAP HANA SQL query" );
}

QString QgsHanaProvider::subsetStringHelpUrl() const
{
  return u"https://help.sap.com/docs/SAP_HANA_PLATFORM/4fe29514fd584807ac9f2a04f6754767/20ff532c751910148657c32fe3431a9f.html"_s;
}

bool QgsHanaProvider::isValid() const
{
  return mValid;
}

QgsFeatureIterator QgsHanaProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  return QgsFeatureIterator( new QgsHanaFeatureIterator( new QgsHanaFeatureSource( this ), true, request ) );
}

bool QgsHanaProvider::addFeatures( QgsFeatureList &flist, Flags flags )
{
  if ( flist.isEmpty() )
    return true;

  if ( mIsQuery )
    return false;

  QgsHanaConnectionRef conn = createConnection();
  if ( conn.isNull() )
    return false;

  // Build insert statement
  QStringList columnNames;
  QStringList values;
  bool isPointDataType = false;

  if ( !mGeometryColumn.isEmpty() )
  {
    QString geometryDataType = conn->getColumnDataType( mSchemaName, mTableName, mGeometryColumn );
    isPointDataType = QString::compare( geometryDataType, u"ST_POINT"_s, Qt::CaseInsensitive ) == 0;

    columnNames << QgsHanaUtils::quotedIdentifier( mGeometryColumn );
    values << u"ST_GeomFromWKB(?, %1)"_s.arg( QString::number( mSrid ) );
  }

  const QgsAttributes attrs = flist[0].attributes();
  QList<int> fieldIds;
  QList<bool> pkFields;

  for ( int idx = 0; idx < mAttributeFields.count(); ++idx )
  {
    const AttributeField &field = mAttributeFields.at( idx );
    if ( field.name.isEmpty() || field.name == mGeometryColumn )
      continue;

    if ( mPrimaryKeyAttrs.contains( idx ) )
    {
      if ( field.isAutoIncrement )
        continue;
      pkFields << true;
    }
    else
    {
      pkFields << false;
    }

    columnNames << QgsHanaUtils::quotedIdentifier( field.name );
    auto qType = mFields.at( idx ).type();
    if ( field.type == QgsHanaDataType::Geometry && qType == QMetaType::Type::QString )
      values << u"ST_GeomFromWKT(?, %1)"_s.arg( QString::number( field.srid ) );
    else if ( field.type == QgsHanaDataType::RealVector && qType == QMetaType::Type::QString )
      values << u"TO_REAL_VECTOR(?)"_s;
    else
      values << u"?"_s;
    fieldIds << idx;
  }

  const bool allowBatchInserts = ( flags & QgsFeatureSink::FastInsert );
  const QString sql = u"INSERT INTO %1.%2(%3) VALUES (%4)"_s.arg( QgsHanaUtils::quotedIdentifier( mSchemaName ), QgsHanaUtils::quotedIdentifier( mTableName ), columnNames.join( QLatin1Char( ',' ) ), values.join( QLatin1Char( ',' ) ) );

  try
  {
    PreparedStatementRef stmtInsert = conn->prepareStatement( sql );
    PreparedStatementRef stmtIdentityValue;
    if ( !allowBatchInserts )
    {
      QString sqlIdentity = u"SELECT CURRENT_IDENTITY_VALUE() \"current identity value\" FROM %1.%2"_s
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
                                      .arg( QgsWkbTypes::displayString( geom.wkbType() ), QgsWkbTypes::displayString( wkbType() ) )
                                      .toStdString()
                                      .c_str() );
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
        const AttributeField &field = mAttributeFields.at( fieldIndex );
        QVariant attrValue = fieldIndex < attrs.length() ? attrs.at( fieldIndex ) : QgsVariantUtils::createNullVariant( QMetaType::Type::LongLong );

        // no default value clause handling supported in this provider, best we can do for now is set to NULL
        if ( QgsVariantUtils::isUnsetAttributeValue( attrValue ) )
          attrValue = QVariant();

        if ( pkFields[i] )
        {
          hasIdValue = hasIdValue || !attrValue.isNull();
          if ( !hasIdValue && !field.isNullable )
            attrValue = 0;
        }
        else
        {
          if ( !field.isNullable && attrValue.isNull() )
            attrValue = mDefaultValues[fieldIndex];
        }

        setStatementValue( stmtInsert, paramIndex, field, attrValue );
        ++paramIndex;
      }

      if ( allowBatchInserts )
      {
        stmtInsert->addBatch();

        if ( stmtInsert->getBatchDataSize() >= MAXIMUM_BATCH_DATA_SIZE )
          stmtInsert->executeBatch();
      }
      else
      {
        stmtInsert->executeUpdate();
        stmtInsert->clearParameters();

        if ( hasIdValue )
        {
          if ( mPrimaryKeyType == PktInt )
          {
            feature.setId( QgsHanaPrimaryKeyUtils::intToFid( attrs.at( mPrimaryKeyAttrs.value( 0 ) ).toInt() ) );
          }
          else
          {
            QVariantList primaryKeyVals;
            primaryKeyVals.reserve( mPrimaryKeyAttrs.size() );
            for ( int idx : std::as_const( mPrimaryKeyAttrs ) )
              primaryKeyVals << attrs.at( idx );
            feature.setId( mPrimaryKeyCntx->lookupFid( primaryKeyVals ) );
          }
        }
        else
        {
          ResultSetRef rsIdentity = stmtIdentityValue->executeQuery();
          if ( rsIdentity->next() )
          {
            NS_ODBC::Long id = rsIdentity->getLong( 1 );
            if ( !id.isNull() )
              feature.setId( static_cast<QgsFeatureId>( *id ) );
          }
          rsIdentity->close();
        }
      }
    }

    if ( allowBatchInserts && stmtInsert->getBatchDataSize() > 0 )
      stmtInsert->executeBatch();

    conn->commit();

    mFeaturesCount = -1;
    return true;
  }
  catch ( const exception &ex )
  {
    pushError( tr( "Failed to add features: %1" )
                 .arg( QgsHanaUtils::formatErrorMessage( ex.what() ) ) );
    conn->rollback();
  }

  return false;
}

bool QgsHanaProvider::deleteFeatures( const QgsFeatureIds &ids )
{
  if ( mPrimaryKeyAttrs.isEmpty() )
    return false;

  if ( mIsQuery )
  {
    QgsDebugError( u"Cannot delete features (is a query)"_s );
    return false;
  }

  if ( ids.empty() )
    return true; // for consistency providers return true to an empty list

  QgsHanaConnectionRef conn = createConnection();
  if ( conn.isNull() )
    return false;

  const QString featureIdsWhereClause = QgsHanaPrimaryKeyUtils::buildWhereClause( ids, mFields, mPrimaryKeyType, mPrimaryKeyAttrs, *mPrimaryKeyCntx );
  if ( featureIdsWhereClause.isEmpty() )
  {
    pushError( tr( "Failed to delete features: Unable to find feature ids" ) );
    return false;
  }

  const QString sql = u"DELETE FROM %1.%2 WHERE %3"_s.arg( QgsHanaUtils::quotedIdentifier( mSchemaName ), QgsHanaUtils::quotedIdentifier( mTableName ), featureIdsWhereClause );

  try
  {
    conn->execute( sql );
    conn->commit();
  }
  catch ( const QgsHanaException &ex )
  {
    pushError( tr( "Failed to delete features: %1" ).arg( ex.what() ) );
    conn->rollback();
    return false;
  }

  mFeaturesCount = -1;

  return true;
}

bool QgsHanaProvider::truncate()
{
  if ( mIsQuery )
  {
    QgsDebugError( u"Cannot truncate (is a query)"_s );
    return false;
  }

  QgsHanaConnectionRef conn = createConnection();
  if ( conn.isNull() )
    return false;

  QString sql = u"TRUNCATE TABLE %1.%2"_s.arg( QgsHanaUtils::quotedIdentifier( mSchemaName ), QgsHanaUtils::quotedIdentifier( mTableName ) );

  try
  {
    conn->execute( sql );
    conn->commit();
  }
  catch ( const QgsHanaException &ex )
  {
    pushError( tr( "Failed to truncate: %1" ).arg( ex.what() ) );
    conn->rollback();
    return false;
  }

  return true;
}

bool QgsHanaProvider::addAttributes( const QList<QgsField> &attributes )
{
  if ( attributes.isEmpty() )
    return true;

  QgsHanaConnectionRef conn = createConnection();
  if ( conn.isNull() )
    return false;

  QString columnDefs;
  for ( const QgsField &field : attributes )
  {
    if ( !columnDefs.isEmpty() )
      columnDefs += QLatin1Char( ',' );

    columnDefs += QgsHanaUtils::quotedIdentifier( field.name() ) + u" "_s + field.typeName();

    if ( !field.comment().isEmpty() )
      columnDefs += u" COMMENT "_s + QgsHanaUtils::quotedString( field.comment() );
  }

  QString sql = u"ALTER TABLE %1.%2 ADD (%3)"_s.arg( QgsHanaUtils::quotedIdentifier( mSchemaName ), QgsHanaUtils::quotedIdentifier( mTableName ), columnDefs );

  try
  {
    conn->execute( sql );
    conn->commit();
  }
  catch ( const QgsHanaException &ex )
  {
    pushError( tr( "Failed to add feature: %1" ).arg( ex.what() ) );
    conn->rollback();
    return false;
  }

  try
  {
    readAttributeFields( *conn );
  }
  catch ( const exception &ex )
  {
    pushError( tr( "Failed to read attributes: %1" )
                 .arg( QgsHanaUtils::formatErrorMessage( ex.what() ) ) );
  }

  return true;
}

bool QgsHanaProvider::deleteAttributes( const QgsAttributeIds &attributes )
{
  if ( attributes.isEmpty() )
    return false;

  QgsHanaConnectionRef conn = createConnection();
  if ( conn.isNull() )
    return false;

  QString columnNames;
  for ( int attrId : attributes )
  {
    if ( !columnNames.isEmpty() )
      columnNames += QLatin1Char( ',' );
    const AttributeField &field = mAttributeFields.at( attrId );
    columnNames += QgsHanaUtils::quotedIdentifier( field.name );
  }

  QString sql = u"ALTER TABLE %1.%2 DROP (%3)"_s.arg( QgsHanaUtils::quotedIdentifier( mSchemaName ), QgsHanaUtils::quotedIdentifier( mTableName ), columnNames );

  try
  {
    conn->execute( sql );
    conn->commit();
  }
  catch ( const QgsHanaException &ex )
  {
    pushError( tr( "Failed to delete attributes: %1" )
                 .arg( QgsHanaUtils::formatErrorMessage( ex.what() ) ) );
    conn->rollback();
    return false;
  }

  try
  {
    readAttributeFields( *conn );
  }
  catch ( const exception &ex )
  {
    pushError( tr( "Failed to read attributes: %1" )
                 .arg( QgsHanaUtils::formatErrorMessage( ex.what() ) ) );
  }

  return true;
}

bool QgsHanaProvider::renameAttributes( const QgsFieldNameMap &fieldMap )
{
  if ( mIsQuery )
    return false;

  QgsHanaConnectionRef conn = createConnection();
  if ( conn.isNull() )
    return false;

  QSet<QPair<QString, QString>> renameCandidates;
  for ( QgsFieldNameMap::const_iterator it = fieldMap.begin(); it != fieldMap.end(); ++it )
  {
    int fieldIndex = it.key();
    if ( fieldIndex < 0 || fieldIndex >= mAttributeFields.count() )
    {
      pushError( tr( "Invalid attribute index: %1" ).arg( fieldIndex ) );
      return false;
    }

    QString fromName = mAttributeFields.at( fieldIndex ).name;
    QString toName = it.value();
    if ( fromName == toName )
      continue;

    renameCandidates.insert( { fromName, toName } );
  }

  if ( renameCandidates.empty() )
    return true;

  QSet<QString> resultFieldNames;
  for ( int i = 0; i < mAttributeFields.count(); ++i )
    resultFieldNames.insert( mAttributeFields[i].name );

  // Ordered list of renaming pairs
  QList<QPair<QString, QString>> fieldsToRename;

  while ( !renameCandidates.empty() )
  {
    bool found = false;
    for ( const QPair<QString, QString> &candidate : std::as_const( renameCandidates ) )
    {
      if ( resultFieldNames.contains( candidate.first ) && !resultFieldNames.contains( candidate.second ) )
      {
        resultFieldNames.remove( candidate.first );
        resultFieldNames.insert( candidate.second );
        fieldsToRename.append( candidate );
        renameCandidates.remove( candidate );
        found = true;
        break;
      }
    }

    if ( !found )
    {
      QPair<QString, QString> candidate = *renameCandidates.begin();
      pushError( tr( "Error renaming field '%1' to '%2'. Field with the same name already exists" ).arg( candidate.first, candidate.second ) );
      return false;
    }
  }

  try
  {
    for ( const QPair<QString, QString> &kv : std::as_const( fieldsToRename ) )
    {
      QString sql = u"RENAME COLUMN %1.%2.%3 TO %4"_s.arg( QgsHanaUtils::quotedIdentifier( mSchemaName ), QgsHanaUtils::quotedIdentifier( mTableName ), QgsHanaUtils::quotedIdentifier( kv.first ), QgsHanaUtils::quotedIdentifier( kv.second ) );
      conn->execute( sql );
    }

    conn->commit();
  }
  catch ( const QgsHanaException &ex )
  {
    pushError( tr( "Failed to rename attributes: %1" ).arg( ex.what() ) );
    conn->rollback();
    return false;
  }

  try
  {
    readAttributeFields( *conn );
  }
  catch ( const exception &ex )
  {
    pushError( tr( "Failed to read attributes: %1" ).arg( QgsHanaUtils::formatErrorMessage( ex.what() ) ) );
  }

  return true;
}

bool QgsHanaProvider::changeGeometryValues( const QgsGeometryMap &geometryMap )
{
  if ( geometryMap.isEmpty() )
    return true;

  if ( mIsQuery || mGeometryColumn.isEmpty() || mPrimaryKeyAttrs.isEmpty() )
    return false;

  QgsHanaConnectionRef conn = createConnection();
  if ( conn.isNull() )
    return false;

  QString fidWhereClause = QgsHanaPrimaryKeyUtils::buildWhereClause( mFields, mPrimaryKeyType, mPrimaryKeyAttrs );
  QString sql = u"UPDATE %1.%2 SET %3 = ST_GeomFromWKB(?, %4) WHERE %5"_s.arg( QgsHanaUtils::quotedIdentifier( mSchemaName ), QgsHanaUtils::quotedIdentifier( mTableName ), QgsHanaUtils::quotedIdentifier( mGeometryColumn ), QString::number( mSrid ), fidWhereClause );

  try
  {
    PreparedStatementRef stmtUpdate = conn->prepareStatement( sql );

    for ( QgsGeometryMap::const_iterator it = geometryMap.begin(); it != geometryMap.end(); ++it )
    {
      QgsFeatureId fid = it.key();
      // skip added features
      if ( FID_IS_NEW( fid ) )
        continue;

      QByteArray wkb = it->asWkb();
      stmtUpdate->setBinary( 1, makeNullable<vector<char>>( wkb.begin(), wkb.end() ) );
      setStatementFidValue( stmtUpdate, 2, mAttributeFields, mPrimaryKeyType, mPrimaryKeyAttrs, *mPrimaryKeyCntx, fid );
      stmtUpdate->addBatch();

      if ( stmtUpdate->getBatchDataSize() >= MAXIMUM_BATCH_DATA_SIZE )
        stmtUpdate->executeBatch();
    }

    if ( stmtUpdate->getBatchDataSize() > 0 )
      stmtUpdate->executeBatch();

    conn->commit();
  }
  catch ( const exception &ex )
  {
    pushError( tr( "Failed to change feature geometry: %1" )
                 .arg( QgsHanaUtils::formatErrorMessage( ex.what() ) ) );
    conn->rollback();
    return false;
  }

  return true;
}

bool QgsHanaProvider::changeFeatures( const QgsChangedAttributesMap &attrMap, const QgsGeometryMap &geometryMap )
{
  bool ret = changeAttributeValues( attrMap );
  if ( ret )
    ret = changeGeometryValues( geometryMap );
  return ret;
}

bool QgsHanaProvider::changeAttributeValues( const QgsChangedAttributesMap &attrMap )
{
  if ( attrMap.isEmpty() )
    return true;

  if ( mIsQuery || mPrimaryKeyAttrs.isEmpty() )
    return false;

  QgsHanaConnectionRef conn = createConnection();
  if ( conn.isNull() )
    return false;

  try
  {
    for ( QgsChangedAttributesMap::const_iterator attrIt = attrMap.begin(); attrIt != attrMap.end(); ++attrIt )
    {
      const QgsFeatureId fid = attrIt.key();

      // skip added features
      if ( FID_IS_NEW( fid ) )
        continue;

      const QgsAttributeMap &attrValues = attrIt.value();
      if ( attrValues.isEmpty() )
        continue;

      bool pkChanged = false;
      QStringList attrs;
      for ( QgsAttributeMap::const_iterator it2 = attrValues.begin(); it2 != attrValues.end(); ++it2 )
      {
        int fieldIndex = it2.key();
        const AttributeField &field = mAttributeFields.at( fieldIndex );

        if ( field.name.isEmpty() || field.isAutoIncrement )
          continue;

        if ( QgsVariantUtils::isUnsetAttributeValue( it2.value() ) )
          continue;

        pkChanged = pkChanged || mPrimaryKeyAttrs.contains( fieldIndex );
        auto qType = mFields.at( fieldIndex ).type();
        if ( field.type == QgsHanaDataType::Geometry && qType == QMetaType::Type::QString )
          attrs << u"%1=ST_GeomFromWKT(?, %2)"_s.arg( QgsHanaUtils::quotedIdentifier( field.name ), QString::number( field.srid ) );
        else if ( field.type == QgsHanaDataType::RealVector && qType == QMetaType::Type::QString )
          attrs << u"%1=TO_REAL_VECTOR(?)"_s.arg( QgsHanaUtils::quotedIdentifier( field.name ) );
        else
          attrs << u"%1=?"_s.arg( QgsHanaUtils::quotedIdentifier( field.name ) );
      }

      if ( attrs.empty() )
        return true;

      const QString fidWhereClause = QgsHanaPrimaryKeyUtils::buildWhereClause( mFields, mPrimaryKeyType, mPrimaryKeyAttrs );
      const QString sql = u"UPDATE %1.%2 SET %3 WHERE %4"_s.arg( QgsHanaUtils::quotedIdentifier( mSchemaName ), QgsHanaUtils::quotedIdentifier( mTableName ), attrs.join( QLatin1Char( ',' ) ), fidWhereClause );

      PreparedStatementRef stmtUpdate = conn->prepareStatement( sql );

      unsigned short paramIndex = 1;
      for ( QgsAttributeMap::const_iterator attrIt = attrValues.begin(); attrIt != attrValues.end(); ++attrIt )
      {
        int fieldIndex = attrIt.key();
        const AttributeField &field = mAttributeFields.at( fieldIndex );

        if ( field.name.isEmpty() || field.isAutoIncrement )
          continue;

        const QVariant attrValue = *attrIt;
        if ( QgsVariantUtils::isUnsetAttributeValue( attrValue ) )
          continue;

        setStatementValue( stmtUpdate, paramIndex, field, attrValue );
        ++paramIndex;
      }

      setStatementFidValue( stmtUpdate, paramIndex, mAttributeFields, mPrimaryKeyType, mPrimaryKeyAttrs, *mPrimaryKeyCntx, fid );

      stmtUpdate->executeUpdate();

      if ( pkChanged )
        updateFeatureIdMap( fid, attrValues );
    }

    conn->commit();
  }
  catch ( const exception &ex )
  {
    pushError( tr( "Failed to change feature attributes: %1" )
                 .arg( QgsHanaUtils::formatErrorMessage( ex.what() ) ) );
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

QgsHanaConnectionRef QgsHanaProvider::createConnection() const
{
  QgsHanaConnectionRef conn( mUri );
  if ( conn.isNull() )
    pushError( tr( "Connection to database failed" ) );
  return conn;
}

QString QgsHanaProvider::buildQuery( const QString &columns, const QString &where, const QString &orderBy, int limit ) const
{
  return ::buildQuery( mQuerySource, columns, where, orderBy, limit );
}

QString QgsHanaProvider::buildQuery( const QString &columns, const QString &where ) const
{
  return buildQuery( columns, where, QString(), -1 );
}

QString QgsHanaProvider::buildQuery( const QString &columns ) const
{
  return buildQuery( columns, mQueryWhereClause );
}

bool QgsHanaProvider::checkPermissionsAndSetCapabilities( QgsHanaConnection &conn )
{
  if ( !mSelectAtIdDisabled )
    mCapabilities = Qgis::VectorProviderCapability::SelectAtId;

  // Read access permissions
  if ( !mIsQuery )
  {
    QString sql = QStringLiteral( "SELECT OBJECT_NAME, OBJECT_TYPE, PRIVILEGE FROM PUBLIC.EFFECTIVE_PRIVILEGES "
                                  "WHERE USER_NAME = CURRENT_USER AND SCHEMA_NAME = ? AND IS_VALID = 'TRUE'" );
    QgsHanaResultSetRef rsPrivileges = conn.executeQuery( sql, { mSchemaName } );
    while ( rsPrivileges->next() )
    {
      QString objName = rsPrivileges->getString( 1 );

      if ( !objName.isEmpty() && objName != mTableName )
        continue;

      QString privType = rsPrivileges->getString( 3 );

      if ( privType == "ALL PRIVILEGES"_L1 || privType == "CREATE ANY"_L1 )
      {
        mCapabilities |= Qgis::VectorProviderCapability::AddAttributes
                         | Qgis::VectorProviderCapability::RenameAttributes
                         | Qgis::VectorProviderCapability::AddFeatures
                         | Qgis::VectorProviderCapability::DeleteAttributes
                         | Qgis::VectorProviderCapability::DeleteFeatures
                         | Qgis::VectorProviderCapability::FastTruncate
                         | Qgis::VectorProviderCapability::ChangeAttributeValues
                         | Qgis::VectorProviderCapability::ChangeFeatures
                         | Qgis::VectorProviderCapability::ChangeGeometries;
      }
      else
      {
        if ( privType == "ALTER"_L1 )
          mCapabilities |= Qgis::VectorProviderCapability::DeleteAttributes
                           | Qgis::VectorProviderCapability::RenameAttributes;
        else if ( privType == "DELETE"_L1 )
          mCapabilities |= Qgis::VectorProviderCapability::DeleteFeatures
                           | Qgis::VectorProviderCapability::FastTruncate;
        else if ( privType == "INSERT"_L1 )
          mCapabilities |= Qgis::VectorProviderCapability::AddAttributes
                           | Qgis::VectorProviderCapability::AddFeatures;
        else if ( privType == "UPDATE"_L1 )
          mCapabilities |= Qgis::VectorProviderCapability::ChangeAttributeValues
                           | Qgis::VectorProviderCapability::ChangeFeatures
                           | Qgis::VectorProviderCapability::ChangeGeometries;
      }
    }
    rsPrivileges->close();
  }

  // TODO needs to be implemented in QgsHanaFeatureIterator class
  // supports geometry simplification on provider side
  //mCapabilities |= (Qgis::VectorProviderCapability::SimplifyGeometries);
  // Qgis::VectorProviderCapability::SimplifyGeometriesWithTopologicalValidation feature
  // is not supported in HANA Qgis::VectorProviderCapability::SimplifyGeometriesWithTopologicalValidation

  mCapabilities |= Qgis::VectorProviderCapability::TransactionSupport;

  mCapabilities |= Qgis::VectorProviderCapability::CircularGeometries;

  mCapabilities |= Qgis::VectorProviderCapability::ReadLayerMetadata;

  return true;
}

static bool checkHANAVersion( QgsHanaConnection &conn, const QVersionNumber &premise, const QVersionNumber &cloud )
{
  try
  {
    QVersionNumber version = QgsHanaUtils::toHANAVersion( conn.getDatabaseVersion() );
    switch ( version.majorVersion() )
    {
      case 2:
        return version >= premise;
      case 4:
        return QgsHanaUtils::toHANAVersion( conn.getDatabaseCloudVersion() ) >= cloud;
      default:
        return false;
    }
  }
  catch ( const QgsHanaException &ex )
  {
    return false;
  }

  return false;
}

QgsRectangle QgsHanaProvider::estimateExtent( bool useEstimatedMetadata ) const
{
  if ( mGeometryColumn.isEmpty() )
    return QgsRectangle();

  QgsHanaConnectionRef conn = createConnection();
  if ( conn.isNull() )
    return QgsRectangle();

  if ( useEstimatedMetadata && !checkHANAVersion( *conn, QVersionNumber( 2, 0, 80 ), QVersionNumber( 2024, 2, 0 ) ) )
    useEstimatedMetadata = false;

  QString sql;
  if ( useEstimatedMetadata )
  {
    sql = ::buildQuery(
      "SYS.M_ST_GEOMETRY_COLUMNS",
      "MIN_X,MIN_Y,MAX_X,MAX_Y",
      u"SCHEMA_NAME=%1 AND TABLE_NAME=%2 AND COLUMN_NAME=%3"_s
        .arg(
          QgsHanaUtils::quotedString( mSchemaName ),
          QgsHanaUtils::quotedString( mTableName ),
          QgsHanaUtils::quotedString( mGeometryColumn )
        ),
      QString(), 1
    );
  }
  else
  {
    if ( isSrsRoundEarth( *conn, mSrid ) )
    {
      QString geomColumn = !mHasSrsPlanarEquivalent ? QgsHanaUtils::quotedIdentifier( mGeometryColumn ) : u"%1.ST_SRID(%2)"_s.arg( QgsHanaUtils::quotedIdentifier( mGeometryColumn ), QString::number( QgsHanaUtils::toPlanarSRID( mSrid ) ) );
      sql = buildQuery( u"MIN(%1.ST_XMin()), MIN(%1.ST_YMin()), MAX(%1.ST_XMax()), MAX(%1.ST_YMax())"_s.arg( geomColumn ) );
    }
    else
    {
      QString subQuery = buildQuery( u"ST_EnvelopeAggr(%1) AS ext"_s.arg( QgsHanaUtils::quotedIdentifier( mGeometryColumn ) ) );
      sql = u"SELECT ext.ST_XMin(),ext.ST_YMin(),ext.ST_XMax(),ext.ST_YMax() FROM (%1)"_s.arg( subQuery );
    }
  }

  try
  {
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

    if ( useEstimatedMetadata && ret.isEmpty() )
      // In this case it is very likely that the fast extent is not yet available, try again without using cached data
      return estimateExtent( false );

    return ret;
  }
  catch ( const QgsHanaException &ex )
  {
    pushError( tr( "Failed to estimate the data extent: %1" ).arg( ex.what() ) );
  }

  return QgsRectangle();
}

void QgsHanaProvider::readAttributeFields( QgsHanaConnection &conn )
{
  mAttributeFields.clear();
  mFields.clear();
  mDefaultValues.clear();

  QMap<QString, QMap<QString, QVariant>> defaultValues;
  auto getColumnDefaultValue = [&defaultValues, &conn]( const QString &schemaName, const QString &tableName, const QString &columnName ) {
    if ( schemaName.isEmpty() || tableName.isEmpty() )
      return QVariant();

    const QString key = u"%1.%2"_s.arg( schemaName, tableName );
    if ( defaultValues.contains( key ) )
      return defaultValues[key].value( columnName );

    QgsHanaResultSetRef rsColumns = conn.getColumns( schemaName, tableName, u"%"_s );
    while ( rsColumns->next() )
    {
      QString name = rsColumns->getString( 4 /*COLUMN_NAME*/ );
      QVariant value = rsColumns->getValue( 13 /*COLUMN_DEF*/ );
      defaultValues[key].insert( name, value );
    }
    rsColumns->close();
    return defaultValues[key].value( columnName );
  };

  auto processField = [&]( const AttributeField &field ) {
    if ( field.name == mGeometryColumn )
      return;

    mAttributeFields.append( field );
    mFields.append( field.toQgsField() );

    const QString schemaName = field.schemaName.isEmpty() ? mSchemaName : field.schemaName;
    const QString tableName = field.tableName.isEmpty() ? mTableName : field.tableName;
    mDefaultValues.insert( mAttributeFields.size() - 1, getColumnDefaultValue( schemaName, tableName, field.name ) );
  };

  if ( mIsQuery )
    conn.readQueryFields( mSchemaName, buildQuery( u"*"_s ), processField );
  else
    conn.readTableFields( mSchemaName, mTableName, processField );

  determinePrimaryKey( conn );
}

void QgsHanaProvider::readGeometryType( QgsHanaConnection &conn )
{
  if ( mGeometryColumn.isNull() || mGeometryColumn.isEmpty() )
  {
    mDetectedGeometryType = Qgis::WkbType::NoGeometry;
    return;
  }

  if ( mIsQuery )
  {
    QString query = buildQuery( u"*"_s );
    if ( !sourceIsQuery( query ) )
      query = "(" + query + ")";
    mDetectedGeometryType = conn.getColumnGeometryType( query, mGeometryColumn );
  }
  else
    mDetectedGeometryType = conn.getColumnGeometryType( mSchemaName, mTableName, mGeometryColumn );
}

void QgsHanaProvider::readMetadata( QgsHanaConnection &conn )
{
  mLayerMetadata.setCrs( crs() );
  mLayerMetadata.setType( u"dataset"_s );

  if ( !mIsQuery )
  {
    QString sql = u"SELECT COMMENTS FROM SYS.TABLES WHERE SCHEMA_NAME = ? AND TABLE_NAME = ?"_s;
    QVariant comment = conn.executeScalar( sql, { mSchemaName, mTableName } );
    if ( !comment.isNull() )
      mLayerMetadata.setAbstract( comment.toString() );
  }
}

void QgsHanaProvider::readSrsInformation( QgsHanaConnection &conn )
{
  if ( mGeometryColumn.isEmpty() )
    return;

  if ( mSrid < 0 )
  {
    if ( mIsQuery )
      mSrid = conn.getColumnSrid( mQuerySource, mGeometryColumn );
    else
      mSrid = conn.getColumnSrid( mSchemaName, mTableName, mGeometryColumn );

    if ( mSrid < 0 )
      return;
  }

  bool isRoundEarth = false;
  QString sql = QStringLiteral( "SELECT ROUND_EARTH FROM SYS.ST_SPATIAL_REFERENCE_SYSTEMS "
                                "WHERE SRS_ID = ?" );
  QgsHanaResultSetRef rs = conn.executeQuery( sql, { mSrid } );
  if ( rs->next() )
    isRoundEarth = ( rs->getString( 1 ) == "TRUE"_L1 );
  rs->close();

  if ( isRoundEarth )
  {
    sql = u"SELECT COUNT(*) FROM SYS.ST_SPATIAL_REFERENCE_SYSTEMS WHERE SRS_ID = ?"_s;
    mHasSrsPlanarEquivalent = conn.executeCountQuery( sql, { QgsHanaUtils::toPlanarSRID( mSrid ) } ) > 0;
  }
}

void QgsHanaProvider::determinePrimaryKey( QgsHanaConnection &conn )
{
  QPair<QgsHanaPrimaryKeyType, QList<int>> primaryKey;
  if ( !mIsQuery )
  {
    if ( conn.isTable( mSchemaName, mTableName ) )
    {
      QStringList layerPrimaryKey = conn.getLayerPrimaryKey( mSchemaName, mTableName );
      primaryKey = QgsHanaPrimaryKeyUtils::determinePrimaryKeyFromColumns( layerPrimaryKey, mFields );
    }
    else
      primaryKey = QgsHanaPrimaryKeyUtils::determinePrimaryKeyFromUriKeyColumn( mUri.keyColumn(), mFields );
  }
  else
  {
    primaryKey = QgsHanaPrimaryKeyUtils::determinePrimaryKeyFromUriKeyColumn( mUri.keyColumn(), mFields );
  }

  mPrimaryKeyType = primaryKey.first;
  mPrimaryKeyAttrs = primaryKey.second;

  if ( mPrimaryKeyAttrs.size() == 1 )
  {
    //primary keys are unique, not null
    QgsFieldConstraints constraints = mFields.at( mPrimaryKeyAttrs.value( 0 ) ).constraints();
    constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
    constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
    mFields[mPrimaryKeyAttrs[0]].setConstraints( constraints );
  }
}

long long QgsHanaProvider::getFeatureCount( const QString &whereClause ) const
{
  QgsHanaConnectionRef conn = createConnection();
  if ( conn.isNull() )
    return -1;
  QString sql = buildQuery( u"COUNT(*)"_s, whereClause );
  size_t count = conn->executeCountQuery( sql );
  return static_cast<long long>( count );
}

void QgsHanaProvider::updateFeatureIdMap( QgsFeatureId fid, const QgsAttributeMap &attributes )
{
  // update feature id map if key was changed
  // PktInt64 also uses a fid map even if it is a stand alone field.
  if ( !( mPrimaryKeyType == PktFidMap || mPrimaryKeyType == PktInt64 ) )
    return;

  QVariantList values = mPrimaryKeyCntx->removeFid( fid );
  int keyCount = std::min( mPrimaryKeyAttrs.size(), values.size() );
  for ( int i = 0; i < keyCount; i++ )
  {
    int idx = mPrimaryKeyAttrs.at( i );
    if ( !attributes.contains( idx ) )
      continue;
    values[i] = attributes[idx];
  }

  mPrimaryKeyCntx->insertFid( fid, values );
}


Qgis::VectorLayerTypeFlags QgsHanaProvider::vectorLayerTypeFlags() const
{
  Qgis::VectorLayerTypeFlags flags;
  if ( mValid && mIsQuery )
  {
    flags.setFlag( Qgis::VectorLayerTypeFlag::SqlQuery );
  }
  return flags;
}

QgsCoordinateReferenceSystem QgsHanaProvider::crs() const
{
  static QMutex sMutex;
  QMutexLocker locker( &sMutex );
  static QMap<int, QgsCoordinateReferenceSystem> sCrsCache;
  if ( sCrsCache.contains( mSrid ) )
    return sCrsCache.value( mSrid );

  QgsCoordinateReferenceSystem srs;

  QgsHanaConnectionRef conn = createConnection();
  if ( conn.isNull() )
    return srs;

  try
  {
    srs = conn->getCrs( mSrid );
    if ( srs.isValid() )
      sCrsCache.insert( mSrid, srs );
  }
  catch ( const QgsHanaException &ex )
  {
    pushError( tr( "Failed to retrieve crs: %1" ).arg( ex.what() ) );
  }

  return srs;
}

Qgis::VectorExportResult QgsHanaProvider::createEmptyLayer( const QString &uri, const QgsFields &fields, Qgis::WkbType wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, QMap<int, int> *oldToNewAttrIdxMap, QString &createdLayerUri, QString *errorMessage, const QMap<QString, QVariant> * )
{
  QgsDataSourceUri dsUri( uri );
  QgsHanaConnectionRef conn( dsUri );
  if ( conn.isNull() )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Connection to database failed" );
    return Qgis::VectorExportResult::ErrorConnectionFailed;
  }

  createdLayerUri = uri;

  QString schemaName = dsUri.schema();
  QString tableName = dsUri.table();

  if ( schemaName.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Schema name cannot be empty" );
    return Qgis::VectorExportResult::ErrorCreatingLayer;
  }

  if ( wkbType != Qgis::WkbType::Unknown && wkbType != Qgis::WkbType::NoGeometry && !QgsHanaUtils::isGeometryTypeSupported( wkbType ) )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Geometry type '%1' is not supported" ).arg( QgsWkbTypes::displayString( wkbType ) );
    return Qgis::VectorExportResult::ErrorCreatingLayer;
  }

  QString geometryColumn = dsUri.geometryColumn();
  QString schemaTableName = QgsHanaUtils::quotedIdentifier( schemaName ) + '.' + QgsHanaUtils::quotedIdentifier( tableName );

  bool fieldsInUpperCase = false;
  if ( fields.size() > 0 )
  {
    int count = QgsHanaUtils::countFieldsWithFirstLetterInUppercase( fields );
    fieldsInUpperCase = count > fields.size() / 2;
  }

  if ( wkbType != Qgis::WkbType::NoGeometry && geometryColumn.isEmpty() )
    geometryColumn = fieldsInUpperCase ? u"GEOM"_s : u"geom"_s;

  QString keyColumn = !dsUri.keyColumn().isEmpty() ? dsUri.keyColumn() : ( fieldsInUpperCase ? u"ID"_s : u"id"_s );
  auto pk = determinePrimaryKeyColumn( fields, keyColumn );
  QString primaryKey = pk.first;
  QString primaryKeyType = pk.second;

  QString sql;

  // set up spatial reference id
  long srid = 0;
  if ( srs.isValid() )
  {
    QString authName;
    if ( QgsHanaCrsUtils::identifyCrs( srs, authName, srid ) )
    {
      sql = QStringLiteral( "SELECT COUNT(*) FROM SYS.ST_SPATIAL_REFERENCE_SYSTEMS "
                            "WHERE SRS_ID = ? AND ORGANIZATION = ? AND ORGANIZATION_COORDSYS_ID = ?" );
      try
      {
        size_t numCrs = conn->executeCountQuery( sql, { static_cast<qulonglong>( srid ), authName, static_cast<qulonglong>( srid ) } );
        if ( numCrs == 0 )
          createCoordinateSystem( *conn, srs );
      }
      catch ( const QgsHanaException &ex )
      {
        if ( errorMessage )
          *errorMessage = QgsHanaUtils::formatErrorMessage( ex.what(), true );
        return Qgis::VectorExportResult::ErrorCreatingLayer;
      }
    }
  }

  sql = u"SELECT COUNT(*) FROM SYS.TABLES WHERE SCHEMA_NAME = ? AND TABLE_NAME = ?"_s;
  size_t numTables = 0;
  try
  {
    numTables = conn->executeCountQuery( sql, { schemaName, tableName } );
  }
  catch ( const QgsHanaException &ex )
  {
    if ( errorMessage )
      *errorMessage = QgsHanaUtils::formatErrorMessage( ex.what(), true );
    return Qgis::VectorExportResult::ErrorCreatingLayer;
  }

  if ( numTables != 0 )
  {
    if ( overwrite )
    {
      QString sql = u"DROP TABLE %1.%2"_s
                      .arg( QgsHanaUtils::quotedIdentifier( schemaName ), QgsHanaUtils::quotedIdentifier( tableName ) );
      if ( !conn->execute( sql, errorMessage ) )
        return Qgis::VectorExportResult::ErrorCreatingLayer;
    }
    else
    {
      if ( errorMessage )
        *errorMessage = QObject::tr( "Table %1.%2 already exists" ).arg( schemaName, tableName );

      return Qgis::VectorExportResult::ErrorCreatingLayer;
    }
  }

  if ( geometryColumn.isEmpty() )
  {
    sql = u"CREATE COLUMN TABLE %1 (%2 %3 GENERATED BY DEFAULT AS IDENTITY, PRIMARY KEY (%2))"_s
            .arg( schemaTableName, QgsHanaUtils::quotedIdentifier( primaryKey ), primaryKeyType );
  }
  else
  {
    sql = u"CREATE COLUMN TABLE %1 (%2 %3 GENERATED BY DEFAULT AS IDENTITY, %4 ST_GEOMETRY(%5), PRIMARY KEY (%2))"_s
            .arg( schemaTableName, QgsHanaUtils::quotedIdentifier( primaryKey ), primaryKeyType, QgsHanaUtils::quotedIdentifier( geometryColumn ), QString::number( srid ) );
  }

  if ( !conn->execute( sql, errorMessage ) )
    return Qgis::VectorExportResult::ErrorCreatingLayer;

  dsUri.setDataSource( dsUri.schema(), dsUri.table(), geometryColumn, dsUri.sql(), primaryKey );
  dsUri.setSrid( QString::number( srid ) );

  QgsDataProvider::ProviderOptions providerOptions;
  unique_ptr<QgsHanaProvider> provider = std::make_unique<QgsHanaProvider>( dsUri.uri( false ), providerOptions );

  if ( !provider->isValid() )
  {
    if ( errorMessage )
      *errorMessage = QObject::tr( "Loading of the layer %1 failed" ).arg( schemaTableName );

    return Qgis::VectorExportResult::ErrorInvalidLayer;
  }

  // add fields to the layer
  if ( oldToNewAttrIdxMap )
    oldToNewAttrIdxMap->clear();

  if ( fields.size() > 0 )
  {
    // if we create a new primary key column, we start the old columns from 1
    int offset = ( fields.indexFromName( primaryKey ) >= 0 ) ? 0 : 1;
    QList<QgsField> flist;
    for ( int i = 0, n = fields.size(); i < n; ++i )
    {
      QgsField fld = fields.at( i );
      if ( fld.name() == geometryColumn )
        continue;

      if ( !QgsHanaUtils::convertField( fld ) )
      {
        if ( errorMessage )
          *errorMessage = QObject::tr( "Unsupported type for field %1" ).arg( fld.name() );

        return Qgis::VectorExportResult::ErrorAttributeTypeUnsupported;
      }

      if ( fld.name() != primaryKey )
        flist.append( fld );

      if ( oldToNewAttrIdxMap )
        oldToNewAttrIdxMap->insert( fields.lookupField( fld.name() ), offset++ );
    }

    if ( !provider->addAttributes( flist ) )
    {
      if ( errorMessage )
        *errorMessage = QObject::tr( "Creation of fields failed" );

      return Qgis::VectorExportResult::ErrorAttributeCreationFailed;
    }
  }

  return Qgis::VectorExportResult::Success;
}

QgsHanaProviderMetadata::QgsHanaProviderMetadata()
  : QgsProviderMetadata( QgsHanaProvider::HANA_KEY, QgsHanaProvider::HANA_DESCRIPTION )
{
}

void QgsHanaProviderMetadata::cleanupProvider()
{
  QgsHanaConnectionPool::cleanupInstance();
}

QgsHanaProvider *QgsHanaProviderMetadata::createProvider(
  const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags
)
{
  QgsDataSourceUri dsUri { uri };
  QgsHanaDriver *drv = QgsHanaDriver::instance();

  auto isDriverValid = [&drv]( const QString &driver ) {
#ifdef Q_OS_WIN
    return drv->isInstalled( driver );
#else
    return drv->isInstalled( driver ) || QgsHanaDriver::isValidPath( driver );
#endif
  };

  // The following block is intended to resolve an issue when a data source was created under
  // another operating system. In this case, the driver parameter may differ.
  if ( !drv->driver().isEmpty() && drv->driver() != dsUri.driver() && !isDriverValid( dsUri.driver() ) && isDriverValid( drv->driver() ) )
  {
    dsUri.setDriver( drv->driver() );
    return new QgsHanaProvider( dsUri.uri(), options, flags );
  }
  return new QgsHanaProvider( uri, options, flags );
}

QList<QgsDataItemProvider *> QgsHanaProviderMetadata::dataItemProviders() const
{
  QList<QgsDataItemProvider *> providers;
  providers << new QgsHanaDataItemProvider;
  return providers;
}

Qgis::VectorExportResult QgsHanaProviderMetadata::createEmptyLayer( const QString &uri, const QgsFields &fields, Qgis::WkbType wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, QMap<int, int> &oldToNewAttrIdxMap, QString &errorMessage, const QMap<QString, QVariant> *options, QString &createdLayerUri )
{
  return QgsHanaProvider::createEmptyLayer(
    uri, fields, wkbType, srs, overwrite,
    &oldToNewAttrIdxMap, createdLayerUri, &errorMessage, options
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

void QgsHanaProviderMetadata::saveConnection( const QgsAbstractProviderConnection *conn, const QString &name )
{
  saveConnectionProtected( conn, name );
}

QVariantMap QgsHanaProviderMetadata::decodeUri( const QString &uri ) const
{
  const QgsDataSourceUri dsUri { uri };
  QVariantMap uriParts;

  auto setUriPart = [&dsUri, &uriParts]( const QString &key ) {
    if ( !dsUri.hasParam( key ) )
      return;
    QString value = dsUri.param( key );
    if ( !value.isEmpty() )
      uriParts[key] = value;
  };

  setUriPart( u"connectionType"_s );
  setUriPart( u"dsn"_s );
  if ( !dsUri.driver().isEmpty() )
    uriParts[u"driver"_s] = dsUri.driver();
  if ( !dsUri.database().isEmpty() )
    uriParts[u"dbname"_s] = dsUri.database();
  if ( !dsUri.host().isEmpty() )
    uriParts[u"host"_s] = dsUri.host();
  if ( !dsUri.port().isEmpty() )
    uriParts[u"port"_s] = dsUri.port();
  if ( !dsUri.username().isEmpty() )
    uriParts[u"username"_s] = dsUri.username();
  if ( !dsUri.password().isEmpty() )
    uriParts[u"password"_s] = dsUri.password();
  if ( !dsUri.authConfigId().isEmpty() )
    uriParts[u"authcfg"_s] = dsUri.authConfigId();
  if ( dsUri.wkbType() != Qgis::WkbType::Unknown )
    uriParts[u"type"_s] = static_cast<quint32>( dsUri.wkbType() );
  if ( !dsUri.schema().isEmpty() )
    uriParts[u"schema"_s] = dsUri.schema();
  if ( !dsUri.table().isEmpty() )
    uriParts[u"table"_s] = dsUri.table();
  if ( !dsUri.keyColumn().isEmpty() )
    uriParts[u"key"_s] = dsUri.keyColumn();
  if ( !dsUri.srid().isEmpty() )
    uriParts[u"srid"_s] = dsUri.srid();
  uriParts[u"selectatid"_s] = dsUri.selectAtIdDisabled();

  // SSL parameters
  setUriPart( u"sslEnabled"_s );
  setUriPart( u"sslCryptoProvider"_s );
  setUriPart( u"sslValidateCertificate"_s );
  setUriPart( u"sslHostNameInCertificate"_s );
  setUriPart( u"sslKeyStore"_s );
  setUriPart( u"sslTrustStore"_s );

  // Proxy parameters
  setUriPart( u"proxyEnabled"_s );
  setUriPart( u"proxyHttp"_s );
  setUriPart( u"proxyHost"_s );
  setUriPart( u"proxyPort"_s );
  setUriPart( u"proxyUsername"_s );
  setUriPart( u"proxyPassword"_s );

  if ( !dsUri.sql().isEmpty() )
    uriParts[u"sql"_s] = dsUri.sql();
  if ( !dsUri.geometryColumn().isEmpty() )
    uriParts[u"geometrycolumn"_s] = dsUri.geometryColumn();

  return uriParts;
}

QString QgsHanaProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;

  auto setUriParam = [&parts, &dsUri]( const QString &key ) {
    if ( parts.contains( key ) )
      dsUri.setParam( key, parts.value( key ).toString() );
  };

  setUriParam( u"connectionType"_s );
  setUriParam( u"dsn"_s );
  if ( parts.contains( u"driver"_s ) )
    dsUri.setDriver( parts.value( u"driver"_s ).toString() );
  if ( parts.contains( u"dbname"_s ) )
    dsUri.setDatabase( parts.value( u"dbname"_s ).toString() );
  setUriParam( u"host"_s );
  setUriParam( u"port"_s );
  if ( parts.contains( u"username"_s ) )
    dsUri.setUsername( parts.value( u"username"_s ).toString() );
  if ( parts.contains( u"password"_s ) )
    dsUri.setPassword( parts.value( u"password"_s ).toString() );
  if ( parts.contains( u"authcfg"_s ) )
    dsUri.setAuthConfigId( parts.value( u"authcfg"_s ).toString() );
  if ( parts.contains( u"type"_s ) )
    dsUri.setParam( u"type"_s, QgsWkbTypes::displayString( static_cast<Qgis::WkbType>( parts.value( u"type"_s ).toInt() ) ) );
  if ( parts.contains( u"schema"_s ) )
    dsUri.setSchema( parts.value( u"schema"_s ).toString() );
  if ( parts.contains( u"table"_s ) )
    dsUri.setTable( parts.value( u"table"_s ).toString() );
  if ( parts.contains( u"key"_s ) )
    dsUri.setKeyColumn( parts.value( u"key"_s ).toString() );
  if ( parts.contains( u"srid"_s ) )
    dsUri.setSrid( parts.value( u"srid"_s ).toString() );
  setUriParam( u"selectatid"_s );

  // SSL parameters
  setUriParam( u"sslEnabled"_s );
  setUriParam( u"sslCryptoProvider"_s );
  setUriParam( u"sslValidateCertificate"_s );
  setUriParam( u"sslHostNameInCertificate"_s );
  setUriParam( u"sslKeyStore"_s );
  setUriParam( u"sslTrustStore"_s );

  // Proxy parameters
  setUriParam( u"proxyEnabled"_s );
  setUriParam( u"proxyHttp"_s );
  setUriParam( u"proxyHost"_s );
  setUriParam( u"proxyPort"_s );
  setUriParam( u"proxyUsername"_s );
  setUriParam( u"proxyPassword"_s );

  if ( parts.contains( u"sql"_s ) )
    dsUri.setSql( parts.value( u"sql"_s ).toString() );
  if ( parts.contains( u"geometrycolumn"_s ) )
    dsUri.setGeometryColumn( parts.value( u"geometrycolumn"_s ).toString() );
  return dsUri.uri();
}

QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsHanaProviderMetadata();
}

QList<Qgis::LayerType> QgsHanaProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::Vector };
}

QIcon QgsHanaProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( u"mIconHana.svg"_s );
}
