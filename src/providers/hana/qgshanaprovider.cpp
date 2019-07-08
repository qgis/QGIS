/***************************************************************************
   qgshanaprovider.cpp  -  Data provider for SAP HANA
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
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
#include "qgshanadriver.h"
#include "qgshanafeatureiterator.h"
#include "qgshanaprovider.h"
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

#include "odbc/Connection.h"
#include "odbc/DatabaseMetaData.h"
#include "odbc/Exception.h"
#include "odbc/PreparedStatement.h"
#include "odbc/ResultSet.h"
#include "odbc/ResultSetMetaData.h"
#include "odbc/Statement.h"

using namespace odbc;
using namespace std;

namespace {

bool executeQuery(ConnectionRef& conn, const QString& sql, QString *errorMessage)
{
  try
  {
    StatementRef stmt = conn->createStatement();
    stmt->execute(sql.toStdString().c_str());
    conn->commit();
    return true;
  }
  catch (const Exception& ex)
  {
    if (errorMessage)
      *errorMessage = QgsHanaUtils::formatErrorMessage(ex.what());
  }

  return false;
}

size_t executeCountQuery(ConnectionRef& conn, const QString& sql)
{
  StatementRef stmt = conn->createStatement();
  ResultSetRef rs = stmt->executeQuery(sql.toStdString().c_str());
  rs->next();
  size_t ret = static_cast<size_t>(*rs->getLong(1));
  rs->close();
  return ret;
}

void createCoordinateSystem(ConnectionRef& conn, const QgsCoordinateReferenceSystem& srs)
{
  OGRSpatialReferenceH hCRS = nullptr;
  hCRS = OSRNewSpatialReference(nullptr);
  int errcode = OSRImportFromProj4(hCRS, srs.toProj4().toUtf8());

  if (errcode != OGRERR_NONE)
    throw exception();

  QgsCoordinateReferenceSystem srsWGS84;
  srsWGS84.createFromSrid(4326);

  QgsCoordinateTransform transform;
  transform.setSourceCrs(srsWGS84);
  transform.setDestinationCrs(srs);
  QgsRectangle bounds = transform.transformBoundingBox(srs.bounds());

  char* linearUnits = nullptr;
  char *angularUnits = nullptr;
  OSRGetLinearUnits(hCRS, &linearUnits);
  OSRGetAngularUnits(hCRS, &angularUnits);

  // create new spatial reference system
  QString sql = QStringLiteral("CREATE SPATIAL REFERENCE SYSTEM \"%1\" "
    "IDENTIFIED BY %2 "
    "LINEAR UNIT OF MEASURE \"%3\" "
    "ANGULAR UNIT OF MEASURE \"%4\" "
    "TYPE %5 "
    "COORDINATE X BETWEEN %6 "
    "COORDINATE Y BETWEEN %7 "
    "DEFINITION '%8' "
    "TRANSFORM DEFINITION '%9'")
    .arg(srs.description(), QString::number(srs.postgisSrid()), QString(linearUnits).toLower(), QString(angularUnits).toLower(),
      srs.isGeographic() ? QStringLiteral("ROUND EARTH") : QStringLiteral("PLANAR"),
      QStringLiteral("%1 AND %2").arg(QString::number(bounds.xMinimum()), QString::number(bounds.xMaximum())),
      QStringLiteral("%1 AND %2").arg(QString::number(bounds.yMinimum()), QString::number(bounds.yMaximum())),
      srs.toWkt(), srs.toProj4());

  StatementRef stmt = conn->createStatement();
  stmt->execute(sql.toStdString().c_str());
  conn->commit();
}

void setStatementValue(
  PreparedStatementRef& stmt,
  unsigned short paramIndex,
  const QgsField& field,
  const FieldInfo& fieldInfo,
  const QVariant& value)
{
  bool isNull = (value.isNull() || !value.isValid());

  switch (fieldInfo.type)
  {
  case SQLDataTypes::Boolean:
    stmt->setBoolean(paramIndex, isNull ? Boolean() : Boolean(value.toBool()));
    break;
  case SQLDataTypes::TinyInt:
    if (fieldInfo.isSigned)
      stmt->setByte(paramIndex, isNull ? Byte() : Byte(value.toInt()));
    else
      stmt->setUByte(paramIndex, isNull ? UByte() : UByte(value.toInt()));
    break;
  case SQLDataTypes::SmallInt:
    if (fieldInfo.isSigned)
      stmt->setShort(paramIndex, isNull ? Short() : Short(value.toInt()));
    else
      stmt->setUShort(paramIndex, isNull ? UShort() : UShort(value.toInt()));
    break;
  case SQLDataTypes::Integer:
    if (fieldInfo.isSigned)
      stmt->setInt(paramIndex, isNull ? Int() : Int(value.toInt()));
    else
      stmt->setUInt(paramIndex, isNull ? UInt() : UInt(value.toInt()));
    break;
  case SQLDataTypes::BigInt:
    if (fieldInfo.isSigned)
      stmt->setLong(paramIndex, isNull ? Long() : Long(value.toLongLong()));
    else
      stmt->setULong(paramIndex, isNull ? ULong() : ULong(value.toULongLong()));
    break;
  case SQLDataTypes::Numeric:
  case SQLDataTypes::Decimal:
    stmt->setDecimal(paramIndex, isNull ? Decimal() :
      makeNullable<decimal>(value.toString().toStdString(), field.length(), field.precision()));
    break;
  case SQLDataTypes::Float:
  case SQLDataTypes::Real:
    stmt->setFloat(paramIndex, isNull ? Float() : Float(value.toFloat()));
    break;
  case SQLDataTypes::Double:
    stmt->setDouble(paramIndex, isNull ? Double() : Double(value.toDouble()));
    break;
  case SQLDataTypes::Date:
    if (isNull)
      stmt->setDate(paramIndex, Date());
    else
    {
      QDate d = value.toDate();
      stmt->setDate(paramIndex, makeNullable<date>(d.year(), d.month(), d.day()));
    }
    break;
  case SQLDataTypes::Time:
    if (isNull)
      stmt->setTime(paramIndex, Time());
    else
    {
      QTime t = value.toTime();
      stmt->setTime(paramIndex, makeNullable<odbc::time>(t.hour(), t.minute(), t.second()));
    }
    break;
  case SQLDataTypes::Timestamp:
    if (isNull)
      stmt->setTimestamp(paramIndex, Timestamp());
    else
    {
      QDateTime dt = value.toDateTime();
      QDate d = dt.date();
      QTime t = dt.time();
      stmt->setTimestamp(paramIndex, makeNullable<odbc::timestamp>(d.year(),
        d.month(), d.day(), t.hour(), t.minute(), t.second(), t.msec()));
    }
    break;
  case SQLDataTypes::Char:
  case SQLDataTypes::VarChar:
  case SQLDataTypes::LongVarChar:
    stmt->setString(paramIndex, isNull ? String() : String(value.toString().toStdString()));
    break;
  case SQLDataTypes::WChar:
  case SQLDataTypes::WVarChar:
  case SQLDataTypes::WLongVarChar:
    stmt->setNString(paramIndex, isNull ? NString() : NString(value.toString().toStdU16String()));
    break;
  case SQLDataTypes::Binary:
  case SQLDataTypes::VarBinary:
  case SQLDataTypes::LongVarBinary:
    if (isNull)
      stmt->setBinary(paramIndex, Binary());
    else
    {
      QByteArray arr = value.toByteArray();
      vector<char> buffer(arr.begin(), arr.end());
      stmt->setBinary(paramIndex, Binary(buffer));
    }
    break;
  }
}
}

static const size_t MAX_BATCH_SIZE = 1024;

const QString QgsHanaProvider::HANA_KEY = QStringLiteral("hana");
const QString QgsHanaProvider::HANA_DESCRIPTION = QStringLiteral("HANA spatial data provider");

QgsHanaProvider::QgsHanaProvider(
  const QString &uri,
  const ProviderOptions &options)
  : QgsVectorDataProvider(uri, options)
  , mUri(uri)
  , mFeaturesCount(-1)
{
  mSchemaName = mUri.schema();
  mTableName = mUri.table();
  mFidColumn = mUri.keyColumn();
  mGeometryColumn = mUri.geometryColumn();
  mQueryWhereClause = mUri.sql();
  mGeometryType = mUri.wkbType();
  mSrid = (!mUri.srid().isEmpty()) ? mUri.srid().toInt() : -1;
  mSelectAtIdDisabled = mUri.selectAtIdDisabled();
  mHasSrsPlanarEquivalent = false;

  if (mSchemaName.isEmpty() && mTableName.startsWith('(') && mTableName.endsWith(')'))
  {
    mIsQuery = true;
    mQuery = mTableName;
    mTableName.clear();
  }
  else
  {
    mIsQuery = false;
    if (!mSchemaName.isEmpty())
      mQuery += QgsHanaUtils::quotedIdentifier(mSchemaName) + '.';
    if (!mTableName.isEmpty())
      mQuery += QgsHanaUtils::quotedIdentifier(mTableName);
    mQuery = QStringLiteral("SELECT * FROM ") + mQuery;
    if (!mQueryWhereClause.isEmpty())
      mQuery += QStringLiteral(" WHERE ") + mQueryWhereClause;
  }

  QgsHanaConnectionRef connRef(mUri);
  if (connRef.isNull())
    return;

  if (!checkPermissionsAndSetCapabilities())
  {
    QgsMessageLog::logMessage(tr("Provider does not have enough permissions"), QObject::tr("HANA"));
    return;
  }

  if (mSrid < 0)
    mSrid = readSrid();

  mDatabaseVersion = QgsHanaUtils::toHANAVersion(connRef->getDatabaseVersion());
  readAttributeFields();
  readSrsInformation();

  //fill type names into sets
  setNativeTypes(QList< NativeType >()
    // boolean
    << QgsVectorDataProvider::NativeType(tr("Boolean"), QStringLiteral("BOOLEAN"), QVariant::Bool, -1, -1, -1, -1)
    // integer types
    << QgsVectorDataProvider::NativeType(tr("8 bytes integer"), QStringLiteral("BIGINT"), QVariant::Int)
    << QgsVectorDataProvider::NativeType(tr("4 bytes integer"), QStringLiteral("INTEGER"), QVariant::Int)
    << QgsVectorDataProvider::NativeType(tr("2 bytes integer"), QStringLiteral("SMALLINT"), QVariant::Int)
    << QgsVectorDataProvider::NativeType(tr("1 byte integer"), QStringLiteral("TINYINT"), QVariant::Int)
    << QgsVectorDataProvider::NativeType(tr("Decimal number (DECIMAL)"), QStringLiteral("DECIMAL"), QVariant::Double, 1, 31, 0, 31)
    // floating point
    << QgsVectorDataProvider::NativeType(tr("Decimal number (REAL)"), QStringLiteral("REAL"), QVariant::Double)
    << QgsVectorDataProvider::NativeType(tr("Decimal number (DOUBLE)"), QStringLiteral("DOUBLE"), QVariant::Double)
    // date/time types
    << QgsVectorDataProvider::NativeType(tr("Date"), QStringLiteral("DATE"), QVariant::Date, -1, -1, -1, -1)
    << QgsVectorDataProvider::NativeType(tr("Time"), QStringLiteral("TIME"), QVariant::Time, -1, -1, -1, -1)
    << QgsVectorDataProvider::NativeType(tr("Date & Time"), QStringLiteral("TIMESTAMP"), QVariant::DateTime, -1, -1, -1, -1)
    // string types
    << QgsVectorDataProvider::NativeType(tr("Text, variable length (VARCHAR)"), QStringLiteral("VARCHAR"), QVariant::String, 1, 5000)
    << QgsVectorDataProvider::NativeType(tr("Unicode text, variable length (NVARCHAR)"), QStringLiteral("NVARCHAR"), QVariant::String, 1, 5000)
    << QgsVectorDataProvider::NativeType(tr("Text, variable length large object (CLOB)"), QStringLiteral("CLOB"), QVariant::String)
    << QgsVectorDataProvider::NativeType(tr("Unicode text, variable length large object (NCLOB)"), QStringLiteral("NCLOB"), QVariant::String)
  );

  mValid = true;

  QgsDebugMsg(QStringLiteral("Connection info is %1").arg(mUri.connectionInfo(false)));
  QgsDebugMsg(QStringLiteral("Schema is: %1").arg(mSchemaName));
  QgsDebugMsg(QStringLiteral("Table name is: %1").arg(mTableName));
  QgsDebugMsg(QStringLiteral("Geometry column is: %1").arg(mGeometryColumn));
  QgsDebugMsg(QStringLiteral("Query is: %1").arg(mQuery));
  QgsDebugMsg(QStringLiteral("Where clause is: %1").arg(mQuery));
}

QgsHanaProvider::~QgsHanaProvider()
{
  QgsDebugMsg(QStringLiteral("deconstructing."));
}

QgsAbstractFeatureSource *QgsHanaProvider::featureSource() const
{
  return new QgsHanaFeatureSource(this);
}

QString QgsHanaProvider::storageType() const
{
  return QObject::tr("SAP HANA database");
}

QgsVectorDataProvider::Capabilities QgsHanaProvider::capabilities() const
{
  return mCapabilities;
}

QgsRectangle QgsHanaProvider::extent() const
{
  if (mLayerExtent.isEmpty())
    mLayerExtent = estimateExtent();
  return mLayerExtent;
}

QgsWkbTypes::Type QgsHanaProvider::wkbType() const
{
  return mGeometryType;
}

long QgsHanaProvider::featureCount() const
{
  if (mFeaturesCount >= 0)
    return mFeaturesCount;

  try
  {
    mFeaturesCount = getFeatureCount(mQueryWhereClause);
  }
  catch (odbc::Exception&)
  {
  }

  return mFeaturesCount;
}

QgsFields QgsHanaProvider::fields() const
{
  return mAttributeFields;
}

QString QgsHanaProvider::subsetString() const
{
  return mQueryWhereClause;
}

bool QgsHanaProvider::setSubsetString(const QString &subset, bool)
{
  QString whereClause = subset.trimmed();
  if (whereClause == mQueryWhereClause)
    return true;

  QgsDebugMsg(whereClause);

  bool hasErrors = false;
  try
  {
    getFeatureCount(whereClause);
    mQueryWhereClause = whereClause;
  }
  catch (const Exception& ex)
  {
    hasErrors = true;
    pushError(QgsHanaUtils::formatErrorMessage(ex.what()));
  }

  if (hasErrors)
    return false;

  QgsDataSourceUri anUri = QgsDataSourceUri(dataSourceUri());
  anUri.setSql(mQueryWhereClause);
  setDataSourceUri(anUri.uri());
  mLayerExtent.setMinimal();
  mFeaturesCount = -1;

  emit dataChanged();

  return true;
}

bool QgsHanaProvider::isValid() const
{
  return mValid;
}

QgsFeatureIterator QgsHanaProvider::getFeatures(const QgsFeatureRequest &request) const
{
  if (!mValid)
  {
    QgsDebugMsg(QStringLiteral("Read attempt on an invalid HANA data source"));
    return QgsFeatureIterator();
  }

  return QgsFeatureIterator(new QgsHanaFeatureIterator(new QgsHanaFeatureSource(this), true, request));
}

bool QgsHanaProvider::addFeatures(QgsFeatureList &flist, Flags )
{
  if (flist.isEmpty())
    return true;

  if (mIsQuery)
    return false;

  QgsHanaConnectionRef connRef(mUri);
  if (connRef.isNull())
    return false;

  // Build insert statement
  QString columnNames;
  QString values;
  bool first = true;

  if (!mGeometryColumn.isEmpty())
  {
    columnNames += QgsHanaUtils::quotedIdentifier(mGeometryColumn);
    values += QStringLiteral("ST_GeomFromWKB(?, %1)").arg(QString::number(mSrid));
    first = false;
  }

  QgsAttributes attrs = flist[0].attributes();

  for (int i = 0; i < attrs.count(); ++i)
  {
    if (i >= mAttributeFields.count())
      continue;

    const QgsField& field = mAttributeFields.at(i);
    const FieldInfo& fieldInfo = mFieldInfos.at(i);

    if (field.name().isEmpty() || field.name() == mGeometryColumn || fieldInfo.isAutoIncrement)
      continue;

    if (!first)
    {
      columnNames += QStringLiteral(",");
      values += QStringLiteral(",");
    }

    columnNames += QgsHanaUtils::quotedIdentifier(field.name());
    values += QStringLiteral("?");
    first = false;
  }

  QString sql = QStringLiteral("INSERT INTO %1.%2(%3) VALUES (%4)").arg(
    QgsHanaUtils::quotedIdentifier(mSchemaName), QgsHanaUtils::quotedIdentifier(mTableName),
    columnNames, values);

  ConnectionRef& conn = connRef->getNativeRef();

  try
  {
    PreparedStatementRef stmt = conn->prepareStatement(sql.toStdString().c_str());
    size_t batchSize = 0;

    for (const auto& feature : flist)
    {
      unsigned short paramIndex = 1;

      if (!mGeometryColumn.isEmpty())
      {
        QByteArray wkb = feature.geometry().asWkb();

        if (wkb.size() == 0)
          stmt->setBinary(paramIndex, Binary());
        else
          stmt->setBinary(paramIndex, makeNullable<vector<char>>(wkb.begin(), wkb.end()));
        ++paramIndex;
      }

      QgsAttributes attrs = feature.attributes();
      for (int i = 0; i < attrs.count(); ++i)
      {
        if (i >= mAttributeFields.count())
          break;

        const QgsField& field = mAttributeFields.at(i);
        const FieldInfo& fieldInfo = mFieldInfos.at(i);
        if (field.name().isEmpty() || field.name() == mGeometryColumn || fieldInfo.isAutoIncrement)
          continue;

        setStatementValue(stmt, paramIndex, field, fieldInfo, attrs.at(i));
        ++paramIndex;
      }

      stmt->addBatch();
      ++batchSize;

      if (batchSize >= MAX_BATCH_SIZE)
      {
        stmt->executeBatch();
        batchSize = 0;
      }
    }

    if (batchSize > 0)
      stmt->executeBatch();

    conn->commit();
  }
  catch (const Exception& ex)
  {
    pushError(tr("HANA error while adding features: %1")
      .arg(QgsHanaUtils::formatErrorMessage(ex.what(), false)));
    conn->rollback();
    return false;
  }

  mFeaturesCount = -1;

  return true;
}

bool QgsHanaProvider::deleteFeatures(const QgsFeatureIds &id)
{
  if (mIsQuery)
  {
    QgsDebugMsg(QStringLiteral("Cannot delete features (is a query)"));
    return false;
  }

  QgsHanaConnectionRef connRef(mUri);
  if (connRef.isNull())
    return false;

  ConnectionRef& conn = connRef->getNativeRef();
  QString featureIds;

  for (int featId : id)
  {
    if (featureIds.isEmpty())
      featureIds = FID_TO_STRING(featId);
    else
      featureIds += ',' + FID_TO_STRING(featId);
  }

  try
  {
    StatementRef stmt = conn->createStatement();
    QString sql = QStringLiteral("DELETE FROM %1.%2 WHERE %3 IN (%4)").arg(
      QgsHanaUtils::quotedIdentifier(mSchemaName), QgsHanaUtils::quotedIdentifier(mTableName),
      QgsHanaUtils::quotedIdentifier(mFidColumn), featureIds);
    stmt->execute(sql.toStdString().c_str());
    conn->commit();
  }
  catch (const Exception& ex)
  {
    pushError(tr("HANA failed to delete features: %1")
      .arg(QgsHanaUtils::formatErrorMessage(ex.what(), false)));
    conn->rollback();
    return false;
  }

  return true;
}

bool QgsHanaProvider::truncate()
{
  if (mIsQuery)
  {
    QgsDebugMsg(QStringLiteral("Cannot truncate (is a query)"));
    return false;
  }

  QgsHanaConnectionRef connRef(mUri);
  if (connRef.isNull())
    return false;

  ConnectionRef& conn = connRef->getNativeRef();

  try
  {
    StatementRef stmt = conn->createStatement();
    QString sql = QStringLiteral("TRUNCATE TABLE %1.%2").arg(
      QgsHanaUtils::quotedIdentifier(mSchemaName), QgsHanaUtils::quotedIdentifier(mTableName));
    stmt->execute(sql.toStdString().c_str());
    conn->commit();
  }
  catch (const Exception& ex)
  {
    pushError(tr("HANA failed to truncate: %1").arg(QgsHanaUtils::formatErrorMessage(ex.what())));
    conn->rollback();
    return false;
  }

  return true;
}

bool QgsHanaProvider::addAttributes(const QList<QgsField> &attributes)
{
  if (attributes.isEmpty())
    return true;

  QgsHanaConnectionRef connRef(mUri);
  if (connRef.isNull())
    return false;

  QString columnDefs;
  for (const QgsField& field : attributes)
  {
    if (!columnDefs.isEmpty())
      columnDefs += QStringLiteral(",");

    columnDefs += QgsHanaUtils::quotedIdentifier(field.name()) + " " + field.typeName();

    if (!field.comment().isEmpty())
      columnDefs += QStringLiteral(" COMMENT ") + QgsHanaUtils::quotedString(field.comment());
  }

  ConnectionRef& conn = connRef->getNativeRef();

  try
  {
    StatementRef stmt = conn->createStatement();
    QString sql = QStringLiteral("ALTER TABLE %1.%2 ADD (%3)").arg(
      QgsHanaUtils::quotedIdentifier(mSchemaName), QgsHanaUtils::quotedIdentifier(mTableName), columnDefs);
    stmt->execute(sql.toStdString().c_str());
    conn->commit();
  }
  catch (const Exception& ex)
  {
    pushError(tr("HANA failed to add feature: %1")
      .arg(QgsHanaUtils::formatErrorMessage(ex.what(), false)));
    conn->rollback();
    return false;
  }

  readAttributeFields();

  return true;
}

bool QgsHanaProvider::deleteAttributes(const QgsAttributeIds &attributes)
{
  if (attributes.isEmpty())
    return false;

  QgsHanaConnectionRef connRef(mUri);
  if (connRef.isNull())
    return false;

  QString columnNames;
  for (int attrId : attributes)
  {
    if (!columnNames.isEmpty())
      columnNames += QStringLiteral(",");
    const QgsField& field = mAttributeFields.at(attrId);
    columnNames += QStringLiteral("%1").arg(QgsHanaUtils::quotedIdentifier(field.name()));
  }

  QString sql = QStringLiteral("ALTER TABLE %1.%2 DROP (%3)").arg(
    QgsHanaUtils::quotedIdentifier(mSchemaName), QgsHanaUtils::quotedIdentifier(mTableName), columnNames);
  ConnectionRef& conn = connRef->getNativeRef();

  try
  {
    StatementRef stmt = conn->createStatement();
    stmt->execute(sql.toStdString().c_str());
    conn->commit();
  }
  catch (const Exception& ex)
  {
    pushError(tr("HANA error while deleting attributes: %1")
      .arg(QgsHanaUtils::formatErrorMessage(ex.what(), false)));
    conn->rollback();
    return false;
  }

  readAttributeFields();

  return true;
}

bool QgsHanaProvider::renameAttributes(const QgsFieldNameMap &fieldMap)
{
  if (mIsQuery)
    return false;

  QgsHanaConnectionRef connRef(mUri);
  if (connRef.isNull())
    return false;

  ConnectionRef& conn = connRef->getNativeRef();

  try
  {
    for (QgsFieldNameMap::const_iterator it = fieldMap.begin(); it != fieldMap.end(); ++it)
    {
      int fieldIndex = it.key();
      if (fieldIndex < 0 || fieldIndex >= mAttributeFields.count())
      {
        pushError(tr("Invalid attribute index: %1").arg(fieldIndex));
        return false;
      }

      if (mAttributeFields.indexFromName(it.value()) >= 0)
      {
        pushError(tr("Error renaming field %1: name '%2' already exists").arg(fieldIndex).arg(it.value()));
        return false;
      }

      QString sql = QStringLiteral("RENAME COLUMN %1.%2.%3 TO %4").arg(
        QgsHanaUtils::quotedIdentifier(mSchemaName), QgsHanaUtils::quotedIdentifier(mTableName),
        QgsHanaUtils::quotedIdentifier(mAttributeFields.at(fieldIndex).name()),
        QgsHanaUtils::quotedIdentifier(it.value()));

      StatementRef stmt = conn->createStatement();
      stmt->execute(sql.toStdString().c_str());
    }

    conn->commit();
  }
  catch (const Exception& ex)
  {
    pushError(tr("HANA error while renaming attributes: %1")
      .arg(QgsHanaUtils::formatErrorMessage(ex.what(), false)));
    conn->rollback();
    return false;
  }

  readAttributeFields();

  return true;
}

bool QgsHanaProvider::changeGeometryValues(const QgsGeometryMap &geometryMap)
{
  if (geometryMap.isEmpty())
    return true;

  if (mIsQuery)
    return false;

  if (mGeometryColumn.isEmpty())
    return false;

  QgsHanaConnectionRef connRef(mUri);
  if (connRef.isNull())
    return false;

  ConnectionRef& conn = connRef->getNativeRef();

  try
  {
    QString sql = QStringLiteral("UPDATE %1.%2 SET %3 = ST_GeomFromWKB(?, %4) WHERE %5 = ?").arg(
      QgsHanaUtils::quotedIdentifier(mSchemaName), QgsHanaUtils::quotedIdentifier(mTableName),
      QgsHanaUtils::quotedIdentifier(mGeometryColumn), QString::number(mSrid),
      QgsHanaUtils::quotedIdentifier(mFidColumn));

    PreparedStatementRef stmt = conn->prepareStatement(sql.toStdString().c_str());

    for (QgsGeometryMap::const_iterator it = geometryMap.begin(); it != geometryMap.end(); ++it)
    {
      QgsFeatureId fid = it.key();
      // skip added features
      if (FID_IS_NEW(fid))
        continue;

      QByteArray wkb = it->asWkb();
      stmt->setBinary(1, makeNullable<vector<char>>(wkb.begin(), wkb.end()));
      stmt->setLong(2, fid);
      stmt->executeUpdate();
    }

    conn->commit();
  }
  catch (const Exception& ex)
  {
    pushError(tr("HANA error while changing feature geometry: %1")
      .arg(QgsHanaUtils::formatErrorMessage(ex.what(), false)));
    conn->rollback();
    return false;
  }

  return true;
}

bool QgsHanaProvider::changeFeatures(const QgsChangedAttributesMap &attrMap,
  const QgsGeometryMap &geometryMap)
{
  bool ret = changeAttributeValues(attrMap);
  if (ret)
    ret = changeGeometryValues(geometryMap);
  return ret;
}

bool QgsHanaProvider::changeAttributeValues(const QgsChangedAttributesMap &attrMap)
{
  if (attrMap.isEmpty())
    return true;

  QgsHanaConnectionRef connRef(mUri);
  if (connRef.isNull())
    return false;

  ConnectionRef& conn = connRef->getNativeRef();

  try
  {
    for (QgsChangedAttributesMap::const_iterator attrIt = attrMap.begin(); attrIt != attrMap.end(); ++attrIt)
    {
      QgsFeatureId fid = attrIt.key();

      // skip added features
      if (FID_IS_NEW(fid))
        continue;

      const QgsAttributeMap &attrs = attrIt.value();
      if (attrs.isEmpty())
        continue;

      QString sql = QStringLiteral("UPDATE %1.%2 SET ").arg(
        QgsHanaUtils::quotedIdentifier(mSchemaName), QgsHanaUtils::quotedIdentifier(mTableName));

      bool first = true;
      for (QgsAttributeMap::const_iterator it2 = attrs.begin(); it2 != attrs.end(); ++it2)
      {
        int fieldIndex = it2.key();
        const QgsField& field = mAttributeFields.at(fieldIndex);
        const FieldInfo& fieldInfo = mFieldInfos.at(fieldIndex);

        if (field.name().isEmpty() || fieldInfo.isAutoIncrement)
          continue;

        if (!first)
          sql += ',';
        else
          first = false;

        sql += QStringLiteral("%1=?").arg(QgsHanaUtils::quotedIdentifier(field.name()));
      }

      if (first)
        return true;

      sql += QStringLiteral(" WHERE %1=%2").arg(QgsHanaUtils::quotedIdentifier(mFidColumn), FID_TO_STRING(fid));

      PreparedStatementRef stmt= conn->prepareStatement(sql.toStdString().c_str());

      unsigned short paramIndex = 1;
      for (QgsAttributeMap::const_iterator attrIt = attrs.begin(); attrIt != attrs.end(); ++attrIt)
      {
        int fieldIndex = attrIt.key();
        const QgsField& field = mAttributeFields.at(fieldIndex);
        const FieldInfo& fieldInfo = mFieldInfos.at(fieldIndex);

        if (field.name().isEmpty() || fieldInfo.isAutoIncrement)
          continue;

        setStatementValue(stmt, paramIndex, field, fieldInfo, *attrIt);
        ++paramIndex;
      }

      stmt->executeUpdate();
    }

    conn->commit();
  }
  catch (const Exception& ex)
  {
    pushError(tr("HANA error while changing feature attributes: %1")
      .arg(QgsHanaUtils::formatErrorMessage(ex.what(), false)));
    conn->rollback();
    return false;
  }

  return true;
}

QVariant QgsHanaProvider::defaultValue(int fieldId) const
{
  return mDefaultValues.value(fieldId, QVariant());
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
  QgsHanaConnectionRef connRef(mUri);
  if (connRef.isNull())
    return false;

  if (!mSelectAtIdDisabled)
    mCapabilities = QgsVectorDataProvider::SelectAtId;

  // Read access permissions
  if (mIsQuery)
  {
    // Any changes are not allowed for queries
  }
  else
  {
    ConnectionRef& conn = connRef->getNativeRef();
    DatabaseMetaDataRef dbmd = conn->getDatabaseMetaData();
    StatementRef stmt = conn->createStatement();
    QString sql = QStringLiteral("SELECT OBJECT_NAME, OBJECT_TYPE, PRIVILEGE FROM PUBLIC.EFFECTIVE_PRIVILEGES "
      "WHERE USER_NAME = CURRENT_USER AND SCHEMA_NAME = '%1' AND IS_VALID = 'TRUE'").arg(mSchemaName);
    ResultSetRef rsPrivileges = stmt->executeQuery(sql.toStdString().c_str());
    while (rsPrivileges->next())
    {
      QString objName = QgsHanaUtils::toQString(*rsPrivileges->getString(1));

      if (!objName.isEmpty() && objName != mTableName)
        break;

      QString objType = QgsHanaUtils::toQString(*rsPrivileges->getString(2));
      QString privType = QgsHanaUtils::toQString(*rsPrivileges->getString(3));

      if (privType == QStringLiteral("ALL PRIVILEGES") || privType == QStringLiteral("CREATE ANY"))
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
        if (privType == QStringLiteral("ALTER"))
          mCapabilities |= QgsVectorDataProvider::DeleteAttributes
                         | QgsVectorDataProvider::RenameAttributes;
        else if (privType == QStringLiteral("DELETE"))
          mCapabilities |= QgsVectorDataProvider::DeleteFeatures
                         | QgsVectorDataProvider::FastTruncate;
        else if (privType == QStringLiteral("INSERT"))
          mCapabilities |= QgsVectorDataProvider::AddAttributes
                         | QgsVectorDataProvider::AddFeatures;
        else if (privType == QStringLiteral("UPDATE"))
          mCapabilities |= QgsVectorDataProvider::ChangeAttributeValues
                         | QgsVectorDataProvider::ChangeFeatures
                         | QgsVectorDataProvider::ChangeGeometries;
      }
    }
    rsPrivileges->close();

    if (mFidColumn.isEmpty())
      mCapabilities &= ~(QgsVectorDataProvider::DeleteFeatures
                       | QgsVectorDataProvider::ChangeAttributeValues
                       | QgsVectorDataProvider::ChangeFeatures);
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
  if (mGeometryColumn.isEmpty())
    return QgsRectangle();

  QgsHanaConnectionRef connRef(mUri);
  ConnectionRef& conn = connRef->getNativeRef();
  StatementRef stmt = conn->createStatement();
  bool isRoundEarth = isSrsRoundEarth(mSrid);

  QString sql;
  if (isRoundEarth)
  {
    sql = QStringLiteral("SELECT MIN(%1.ST_XMin()), MIN(%1.ST_YMin()), "
      "MAX(%1.ST_XMax()), MAX(%1.ST_YMax()) FROM (SELECT * FROM (%2))")
      .arg(QgsHanaUtils::quotedIdentifier(mGeometryColumn), mQuery);
  }
  else
  {
    sql = QStringLiteral("SELECT \"ext\".ST_XMin(),\"ext\".ST_YMin(),\"ext\".ST_XMax(),"
      "\"ext\".ST_YMax() FROM (SELECT ST_EnvelopeAggr(%1) AS \"ext\" FROM (%2))")
      .arg(QgsHanaUtils::quotedIdentifier(mGeometryColumn), mQuery);
  }
  ResultSetRef rsExtent = stmt->executeQuery(sql.toStdString().c_str());

  QgsRectangle ret;
  if (rsExtent->next())
  {
    Double val = rsExtent->getDouble(1);
    if (!val.isNull())
    {
      ret.setXMinimum(*val);
      ret.setYMinimum(*rsExtent->getDouble(2));
      ret.setXMaximum(*rsExtent->getDouble(3));
      ret.setYMaximum(*rsExtent->getDouble(4));
    }
  }
  rsExtent->close();

  return ret;
}

bool QgsHanaProvider::isSrsRoundEarth(int srsId) const
{
  if (mGeometryColumn.isEmpty())
    return false;

  QgsHanaConnectionRef connRef(mUri);
  ConnectionRef& conn = connRef->getNativeRef();
  StatementRef stmt = conn->createStatement();
  QString sql = QStringLiteral("SELECT ROUND_EARTH FROM SYS.ST_SPATIAL_REFERENCE_SYSTEMS "
    "WHERE SRS_ID = %1").arg(QString::number(srsId));
  ResultSetRef rsRoundEarth = stmt->executeQuery(sql.toStdString().c_str());
  bool ret = true;
  while (rsRoundEarth->next())
  {
    ret = (*rsRoundEarth->getString(1) == "TRUE");
  }
  rsRoundEarth->close();

  return ret;
}

int QgsHanaProvider::readSrid()
{
  if (mGeometryColumn.isEmpty())
    return -1;

  QString sql = QStringLiteral("SELECT SRS_ID FROM SYS.ST_GEOMETRY_COLUMNS "
    "WHERE SCHEMA_NAME='%1' AND TABLE_NAME='%2'").arg(mSchemaName, mTableName);
  if (!mGeometryColumn.isEmpty())
    sql += QStringLiteral(" AND COLUMN_NAME='%1'").arg(mGeometryColumn);
  QgsHanaConnectionRef connRef(mUri);
  ConnectionRef& conn = connRef->getNativeRef();
  StatementRef stmt = conn->createStatement();
  ResultSetRef rs = stmt->executeQuery(sql.toStdString().c_str());
  int ret = -1;
  while (rs->next())
  {
    ret = *rs->getLong(1);
    break;
  }
  rs->close();
  return ret;
}

void QgsHanaProvider::readSrsInformation()
{
  if (mGeometryColumn.isEmpty())
    return;

  QgsHanaConnectionRef connRef(mUri);
  ConnectionRef& conn = connRef->getNativeRef();
  StatementRef stmt = conn->createStatement();

  QgsRectangle ext;
  bool isRoundEarth = false;
  QString sql = QStringLiteral("SELECT MIN_X, MIN_Y, MAX_X, MAX_Y, ROUND_EARTH FROM SYS.ST_SPATIAL_REFERENCE_SYSTEMS "
    "WHERE SRS_ID = %1").arg(QString::number(mSrid));
  ResultSetRef rs = stmt->executeQuery(sql.toStdString().c_str());
  if (rs->next())
  {
    ext.setXMinimum(*rs->getDouble(1));
    ext.setYMinimum(*rs->getDouble(2));
    ext.setXMaximum(*rs->getDouble(3));
    ext.setYMaximum(*rs->getDouble(4));

    isRoundEarth = (*rs->getString(5) == "TRUE");
  }
  rs->close();
  mSrsExtent = ext;

  if (isRoundEarth)
  {
    int srid = QgsHanaUtils::toPlanarSRID(mSrid);

    sql = QStringLiteral("SELECT COUNT(*) FROM SYS.ST_SPATIAL_REFERENCE_SYSTEMS "
      "WHERE SRS_ID = %1").arg(QString::number(srid));
    mHasSrsPlanarEquivalent = executeCountQuery(conn, sql) > 0;
  }
}

void QgsHanaProvider::readAttributeFields()
{
  mAttributeFields.clear();
  mFieldInfos.clear();
  mDefaultValues.clear();

  QgsHanaConnectionRef connRef(mUri);
  ConnectionRef& conn = connRef->getNativeRef();
  DatabaseMetaDataRef dmd = conn->getDatabaseMetaData();
  StatementRef stmt = conn->createStatement();
  QString sql = QStringLiteral("SELECT * FROM (%1) LIMIT 0").arg(mQuery);
  ResultSetRef rsAttributes = stmt->executeQuery(sql.toStdString().c_str());
  ResultSetMetaDataRef rsmd = rsAttributes->getMetaData();
  for (unsigned short i = 1; i <= rsmd->getColumnCount(); ++i)
  {
    const QString fieldName(rsmd->getColumnName(i).c_str());
    if (fieldName == mFidColumn || fieldName == mGeometryColumn)
      continue;

    QVariant::Type fieldType = QVariant::Invalid;
    const short sqlType = rsmd->getColumnType(i);
    const QString fieldTypeName(rsmd->getColumnTypeName(i).c_str());
    const bool isSigned = rsmd->isSigned(i);
    int fieldSize = static_cast<int>(rsmd->getColumnLength(i));
    int fieldPrec = -1;

    switch (sqlType)
    {
    case SQLDataTypes::Boolean:
      fieldType = QVariant::Bool;
      break;
    case SQLDataTypes::TinyInt:
    case SQLDataTypes::SmallInt:
      // we try to make it more compatible with other providers
      fieldType = QVariant::Int;
      break;
    case SQLDataTypes::Integer:
      fieldType = isSigned ? QVariant::Int : QVariant::UInt;
      break;
    case SQLDataTypes::BigInt:
      fieldType = isSigned ? QVariant::LongLong : QVariant::ULongLong;
      break;
    case SQLDataTypes::Numeric:
    case SQLDataTypes::Decimal:
      fieldType = QVariant::Double;
      fieldSize = rsmd->getPrecision(i);
      fieldPrec = rsmd->getScale(i);
      break;
    case SQLDataTypes::Double:
    case SQLDataTypes::Float:
    case SQLDataTypes::Real:
      fieldType = QVariant::Double;
      break;
    case SQLDataTypes::Char:
    case SQLDataTypes::WChar:
      fieldType = QVariant::Char;
      break;
    case SQLDataTypes::VarChar:
    case SQLDataTypes::WVarChar:
    case SQLDataTypes::LongVarChar:
    case SQLDataTypes::WLongVarChar:
        fieldType = QVariant::String;
      break;
    case SQLDataTypes::Binary:
    case SQLDataTypes::VarBinary:
      fieldType = QVariant::BitArray;
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

    if (fieldType != QVariant::Invalid)
    {
      QgsField newField = QgsField(fieldName, fieldType, fieldTypeName, fieldSize, fieldPrec, QString(), QVariant::Invalid);

      bool isNullable = rsmd->isNullable(i);
      bool isAutoIncrement = rsmd->isAutoIncrement(i);
      if (!isNullable || isAutoIncrement)
      {
        QgsFieldConstraints constraints;
        if (!isNullable)
          constraints.setConstraint(QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider);
        if (isAutoIncrement)
          constraints.setConstraint(QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider);
        newField.setConstraints(constraints);
      }

      mAttributeFields.append(newField);
      mFieldInfos.append({ sqlType, isAutoIncrement, isNullable, isSigned });

      std::string schemaName = rsmd->getSchemaName(i);
      if (schemaName.empty())
        schemaName = mSchemaName.toStdString();
      ResultSetRef rsColumns = dmd->getColumns(nullptr, schemaName.c_str(),
        rsmd->getTableName(i).c_str(), fieldName.toStdString().c_str());
      if (rsColumns->next())
        mDefaultValues.insert(mAttributeFields.size() - 1, QgsHanaUtils::toVariant(rsColumns->getString(13/*COLUMN_DEF*/), sqlType));
      rsColumns->close();
    }
  }
  rsAttributes->close();
}

long QgsHanaProvider::getFeatureCount(const QString& whereClause) const
{
  QgsHanaConnectionRef connRef(mUri);
  QString sql = QStringLiteral("SELECT COUNT(*) FROM (%1)").arg(mQuery);
  if (!whereClause.isEmpty())
    sql += QStringLiteral(" WHERE ") + whereClause;
  size_t count = executeCountQuery(connRef->getNativeRef(), sql);
  return static_cast<long>(count);
}

QgsCoordinateReferenceSystem QgsHanaProvider::crs() const
{
  int srid = mSrid;
  QgsCoordinateReferenceSystem srs;
  srs.createFromSrid(srid);
  if (srs.isValid())
    return srs;

  static QMutex sMutex;
  QMutexLocker locker(&sMutex);
  static QMap<int, QgsCoordinateReferenceSystem> sCrsCache;
  if (sCrsCache.contains(srid))
    srs = sCrsCache.value(srid);
  else
  {
    QgsHanaConnectionRef connRef(mUri);
    ConnectionRef& conn = connRef->getNativeRef();
    StatementRef stmt = conn->createStatement();
    QString sql = QStringLiteral("SELECT TRANSFORM_DEFINITION FROM SYS.ST_SPATIAL_REFERENCE_SYSTEMS WHERE SRS_ID = %1").arg(srid);
    ResultSetRef rs = stmt->executeQuery(sql.toStdString().c_str());
    if (rs->next())
    {
      String str = rs->getString(1);
      if (!str.isNull())
      {
        srs = QgsCoordinateReferenceSystem::fromProj4(str->c_str());
        sCrsCache.insert(srid, srs);
      }
    }
    rs->close();
  }
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
  const QMap<QString, QVariant> *options
)
{
  QgsDataSourceUri dsUri(uri);

  QgsHanaConnectionRef connRef(dsUri);
  if (connRef.isNull())
  {
    if (errorMessage)
      *errorMessage = QObject::tr("Connection to database failed");
    return QgsVectorLayerExporter::ErrConnectionFailed;
  }

  QString schemaName = dsUri.schema();
  QString tableName = dsUri.table();

  if (schemaName.isEmpty())
  {
    if (errorMessage)
      *errorMessage = QObject::tr("Schema name cannot be empty");
    return QgsVectorLayerExporter::ErrCreateLayer;
  }

  QString geometryColumn = dsUri.geometryColumn();
  QString geometryType;

  QString primaryKey = dsUri.keyColumn();
  QString primaryKeyType;

  QString schemaTableName = QgsHanaUtils::quotedIdentifier(schemaName) + '.' +
    QgsHanaUtils::quotedIdentifier(tableName);

  if (wkbType != QgsWkbTypes::NoGeometry && geometryColumn.isEmpty())
    geometryColumn = "geom";

  QgsDebugMsg(QStringLiteral("Connection info is: %1").arg(dsUri.connectionInfo(false)));
  QgsDebugMsg(QStringLiteral("Geometry column is: %1").arg(geometryColumn));
  QgsDebugMsg(QStringLiteral("Schema is: %1").arg(schemaName));
  QgsDebugMsg(QStringLiteral("Table name is: %1").arg(tableName));

  bool fieldsInUpperCase = false;
  if (fields.size() > 0)
  {
    int count = QgsHanaUtils::countFieldsInUppercase(fields);
    fieldsInUpperCase = count > fields.size() / 2;
  }

  bool createdNewPk = false;

  if (primaryKey.isEmpty())
  {
    QString pk = primaryKey = fieldsInUpperCase ? "ID" : "id";
    int index = 0;
    while (fields.indexFromName(primaryKey) >= 0)
    {
      primaryKey = QStringLiteral("%1_%2").arg(pk).arg(index++);
    }

    createdNewPk = true;
  }
  else
  {
    int idx = fields.indexFromName(primaryKey);
    if (idx >= 0)
    {
      QgsField fld = fields.at(idx);
      if (QgsHanaUtils::convertField(fld))
        primaryKeyType = fld.typeName();
    }
  }

  if (primaryKeyType.isEmpty())
    primaryKeyType = QStringLiteral("BIGINT");

  ConnectionRef& conn = connRef->getNativeRef();

  StatementRef stmt = conn->createStatement();
  QString sql;

  // set up spatial reference id
  long srid = 0;
  if (srs.isValid())
  {
    srid = srs.postgisSrid();
    QString authSrid = QStringLiteral("null");
    QString authName = QStringLiteral("null");
    QStringList sl = srs.authid().split(':');
    if (sl.length() == 2)
    {
      authName = '\'' + sl[0] + '\'';
      authSrid = sl[1];
    }

    try
    {
      sql = QStringLiteral("SELECT COUNT(*) FROM SYS.ST_SPATIAL_REFERENCE_SYSTEMS "
        "WHERE SRS_ID=%1 AND ORGANIZATION=%2 AND ORGANIZATION_COORDSYS_ID=%3")
        .arg(QString::number(srid), authName, authSrid);

      size_t numCrs = executeCountQuery(conn, sql);
      if (numCrs == 0)
        createCoordinateSystem(conn, srs);
    }
    catch (...)
    {
      if (errorMessage)
        *errorMessage = QObject::tr("Connection to database failed");
      return QgsVectorLayerExporter::ErrConnectionFailed;
    }
  }

  sql = QStringLiteral("SELECT COUNT(*) FROM SYS.TABLES WHERE SCHEMA_NAME = '%1' AND TABLE_NAME = '%2'")
    .arg(schemaName, tableName);
  size_t numTables = executeCountQuery(conn, sql);
  if (numTables != 0)
  {
    if (overwrite)
    {
      if (!connRef->dropTable(schemaName, tableName, errorMessage))
        return QgsVectorLayerExporter::ErrCreateLayer;
    }
    else
    {
      if (errorMessage)
        *errorMessage = QObject::tr("Table %1.%2 already exists").arg(schemaName, tableName);

      return QgsVectorLayerExporter::ErrCreateLayer;
    }
  }

  if (geometryColumn.isEmpty())
  {
    sql = QStringLiteral("CREATE COLUMN TABLE %1 (%2 %3 GENERATED BY DEFAULT AS IDENTITY, PRIMARY KEY (%2))")
      .arg(schemaTableName, QgsHanaUtils::quotedIdentifier(primaryKey), primaryKeyType);
  }
  else
  {
    sql = QStringLiteral("CREATE COLUMN TABLE %1 (%2 %3 GENERATED BY DEFAULT AS IDENTITY, %4 ST_GEOMETRY(%5), PRIMARY KEY (%2))")
      .arg(schemaTableName, QgsHanaUtils::quotedIdentifier(primaryKey), primaryKeyType,
        QgsHanaUtils::quotedIdentifier(geometryColumn), QString::number(srid));
  }

  if (!executeQuery(conn, sql, errorMessage))
    return QgsVectorLayerExporter::ErrCreateLayer;

  dsUri.setDataSource(dsUri.schema(), dsUri.table(), geometryColumn, dsUri.sql(), primaryKey);
  dsUri.setSrid(QString::number(srid));

  QgsDataProvider::ProviderOptions providerOptions;
  unique_ptr< QgsHanaProvider > provider = qgis::make_unique< QgsHanaProvider >(dsUri.uri(false), providerOptions);

  if (!provider->isValid())
  {
    if (errorMessage)
      *errorMessage = QObject::tr("Loading of the layer %1 failed").arg(schemaTableName);

    return QgsVectorLayerExporter::ErrInvalidLayer;
  }

  // add fields to the layer
  if (oldToNewAttrIdxMap)
    oldToNewAttrIdxMap->clear();

  if (fields.size() > 0)
  {
    int offset = createdNewPk ? 1 : 0;

    QList<QgsField> flist;
    for (int i = 0, n = fields.size(); i < n; ++i)
    {
      QgsField fld = fields.at(i);
      if (oldToNewAttrIdxMap && fld.name() == primaryKey)
      {
        oldToNewAttrIdxMap->insert(fields.lookupField(fld.name()), 0);
        continue;
      }

      if (fld.name() == geometryColumn)
        continue;

      if (!QgsHanaUtils::convertField(fld))
      {
        if (errorMessage)
          *errorMessage = QObject::tr("Unsupported type for field %1").arg(fld.name());

        return QgsVectorLayerExporter::ErrAttributeTypeUnsupported;
      }

      flist.append(fld);
      if (oldToNewAttrIdxMap)
        oldToNewAttrIdxMap->insert(fields.lookupField(fld.name()), offset++);
    }

    if (!provider->addAttributes(flist))
    {
      if (errorMessage)
        *errorMessage = QObject::tr("Creation of fields failed");

      return QgsVectorLayerExporter::ErrAttributeCreationFailed;
    }

    QgsDebugMsg(QStringLiteral("Done creating fields"));
  }

  return QgsVectorLayerExporter::NoError;
}

void QgsHanaProviderMetadata::initProvider()
{
}

void QgsHanaProviderMetadata::cleanupProvider()
{
  QgsHanaConnectionPool::cleanupInstance();
  QgsHanaDriver::cleanupInstance();
}

QgsHanaProvider *QgsHanaProviderMetadata::createProvider(
  const QString &uri, const QgsDataProvider::ProviderOptions &options)
{
  return new QgsHanaProvider(uri, options);
}

QgsHanaProviderMetadata::QgsHanaProviderMetadata()
  : QgsProviderMetadata(QgsHanaProvider::HANA_KEY, QgsHanaProvider::HANA_DESCRIPTION)
{
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
  const QMap<QString, QVariant> *options)
{
  return QgsHanaProvider::createEmptyLayer(
    uri, fields, wkbType, srs, overwrite,
    &oldToNewAttrIdxMap, &errorMessage, options
  );
}

QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  if (QgsHanaDriver::instance()->isInstalled())
    return new QgsHanaProviderMetadata();
  else
  {
    QgsMessageLog::logMessage(QStringLiteral("HANA ODBC driver cannot be found"), QStringLiteral("HANA"));
    return nullptr;
  }
}
