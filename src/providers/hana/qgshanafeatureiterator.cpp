/***************************************************************************
   qgshanafeatureiterator.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsexception.h"
#include "qgsgeometry.h"
#include "qgsgeometryfactory.h"
#include "qgshanaexception.h"
#include "qgshanaexpressioncompiler.h"
#include "qgshanafeatureiterator.h"
#include "qgshanaprovider.h"
#include "qgshanautils.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"

#include "odbc/Connection.h"
#include "odbc/PreparedStatement.h"
#include "odbc/ResultSet.h"

using namespace odbc;

QgsHanaFeatureIterator::QgsHanaFeatureIterator(
  QgsHanaFeatureSource *source,
  bool ownSource,
  const QgsFeatureRequest &request)
  : QgsAbstractFeatureIteratorFromSource<QgsHanaFeatureSource>(source, ownSource, request)
  , mConnRef(source->mUri)
  , mSrsExtent(source->mSrsExtent)
  , mHasFidColumn(false)
  , mHasAttributes(false)
  , mHasGeometryColumn(false)
{
  mClosed = true;

  if (mConnRef.isNull())
  {
    iteratorClosed();
    return;
  }

  if (mRequest.destinationCrs().isValid() && mRequest.destinationCrs() != mSource->mCrs)
    mTransform = QgsCoordinateTransform(mSource->mCrs, mRequest.destinationCrs(), mRequest.transformContext());

  try
  {
    mFilterRect = filterRectToSourceCrs(mTransform);
  }
  catch (QgsCsException &)
  {
    iteratorClosed();
    return;
  }

  try
  {
    buildStatement(request);
    mClosed = false;
    rewind();
  }
  catch (odbc::Exception &)
  {
    iteratorClosed();
  }
}

QgsHanaFeatureIterator::~QgsHanaFeatureIterator()
{
  if (!mClosed)
    close();
}

bool QgsHanaFeatureIterator::rewind()
{
  if (mClosed)
    return false;

  try
  {
    mResultSet.reset();
    mResultSet = mStatement->executeQuery();
  }
  catch (const Exception& ex)
  {
    throw QgsHanaException(ex.what());
  }

  return true;
}

bool QgsHanaFeatureIterator::close()
{
  if (mClosed)
    return false;

  mResultSet->close();
  iteratorClosed();
  mClosed = true;

  return true;
}

bool QgsHanaFeatureIterator::fetchFeature(QgsFeature &feature)
{
  feature.setValid(false);

  if (mClosed)
    return false;

  try
  {
    if (!mResultSet->next())
      return false;

    unsigned short paramIndex = 1;

    // Read feature id
    if (mHasFidColumn)
    {
      feature.setId(*mResultSet->getLong(paramIndex));
      ++paramIndex;
    }
    else
    {
      feature.setId(0u);
    }

    // Read attributes
    feature.initAttributes(mSource->mFields.count());
    if (mHasAttributes)
    {
      Q_FOREACH(int idx, mAttributesToFetch)
      {
        fetchFeatureAttribute(idx, paramIndex, feature);
        ++paramIndex;
      }
    }

    // Read geometry
    if (mHasGeometryColumn)
    {
      fetchFeatureGeometry(paramIndex, feature);
      ++paramIndex;
    }
    else
    {
      feature.clearGeometry();
    }

    feature.setValid(true);
    feature.setFields(mSource->mFields); // allow name-based attribute lookups
    geometryToDestinationCrs(feature, mTransform);
  }
  catch (const Exception& ex)
  {
    throw QgsHanaException(ex.what());
  }

  return true;
}

void QgsHanaFeatureIterator::fetchFeatureAttribute(
  int attrIndex,
  unsigned short paramIndex,
  QgsFeature &feature)
{
  QgsField field = mSource->mFields.at(attrIndex);
  FieldInfo fieldInfo = mSource->mFieldInfos.at(attrIndex);
  switch (fieldInfo.type)
  {
  case SQLDataTypes::Boolean:
    feature.setAttribute(attrIndex, QgsHanaUtils::toVariant(mResultSet->getBoolean(paramIndex), QVariant::Bool));
    break;
  case SQLDataTypes::Char:
    feature.setAttribute(attrIndex, QgsHanaUtils::toVariant(mResultSet->getByte(paramIndex)));
    break;
  case SQLDataTypes::WChar:
    feature.setAttribute(attrIndex, QgsHanaUtils::toVariant(mResultSet->getUByte(paramIndex)));
    break;
  case SQLDataTypes::SmallInt:
    if (field.type() == QVariant::Int)
      feature.setAttribute(attrIndex, QgsHanaUtils::toVariant(mResultSet->getShort(paramIndex)));
    else
      feature.setAttribute(attrIndex, QgsHanaUtils::toVariant(mResultSet->getUShort(paramIndex)));
    break;
  case SQLDataTypes::Integer:
    if (field.type() == QVariant::Int)
      feature.setAttribute(attrIndex, QgsHanaUtils::toVariant(mResultSet->getInt(paramIndex), QVariant::Int));
    else
      feature.setAttribute(attrIndex, QgsHanaUtils::toVariant(mResultSet->getUInt(paramIndex), QVariant::UInt));
    break;
  case SQLDataTypes::BigInt:
    if (field.type() == QVariant::LongLong)
      feature.setAttribute(attrIndex, QgsHanaUtils::toVariant(mResultSet->getLong(paramIndex)));
    else
      feature.setAttribute(attrIndex, QgsHanaUtils::toVariant(mResultSet->getULong(paramIndex)));
    break;
  case SQLDataTypes::Double:
    feature.setAttribute(attrIndex, QgsHanaUtils::toVariant(mResultSet->getDouble(paramIndex), QVariant::Double));
    break;
  case SQLDataTypes::Decimal:
    feature.setAttribute(attrIndex, QgsHanaUtils::toVariant(mResultSet->getDouble(paramIndex), QVariant::Double));
    break;
  case SQLDataTypes::Numeric:
    feature.setAttribute(attrIndex, QgsHanaUtils::toVariant(mResultSet->getDouble(paramIndex), QVariant::Double));
    break;
  case SQLDataTypes::Date:
    feature.setAttribute(attrIndex, QgsHanaUtils::toVariant(mResultSet->getDate(paramIndex)));
    break;
  case SQLDataTypes::Time:
    feature.setAttribute(attrIndex, QgsHanaUtils::toVariant(mResultSet->getTime(paramIndex)));
    break;
  case SQLDataTypes::Timestamp:
    feature.setAttribute(attrIndex, QgsHanaUtils::toVariant(mResultSet->getTimestamp(paramIndex)));
    break;
  case SQLDataTypes::VarChar:
  case SQLDataTypes::LongVarChar:
    feature.setAttribute(attrIndex, QgsHanaUtils::toVariant(mResultSet->getString(paramIndex)));
    break;
  case SQLDataTypes::WVarChar:
  case SQLDataTypes::WLongVarChar:
    feature.setAttribute(attrIndex, QgsHanaUtils::toVariant(mResultSet->getNString(paramIndex)));
    break;
  case SQLDataTypes::Binary:
  case SQLDataTypes::VarBinary:
  case SQLDataTypes::LongVarBinary:
    feature.setAttribute(attrIndex, QgsHanaUtils::toVariant(mResultSet->getBinary(paramIndex)));
    break;
  default:
    break;
  }
}

void QgsHanaFeatureIterator::fetchFeatureGeometry(unsigned short paramIndex, QgsFeature &feature)
{
  size_t bufLength = mResultSet->getBinaryLength(paramIndex);
  unsigned char* bufPtr = nullptr;

  if (bufLength == ResultSet::UNKNOWN_LENGTH)
  {
    Binary wkb = mResultSet->getBinary(paramIndex);
    if (wkb.isNull() || wkb->size() == 0)
      bufLength = 0;
    else
      bufPtr = (unsigned char*)wkb->data();
  }
  else if (bufLength != 0 && bufLength != odbc::ResultSet::NULL_DATA)
  {
    ensureBufferCapacity(bufLength);
    mResultSet->getBinaryData(paramIndex, mBuffer.data(), bufLength);

    bufPtr = mBuffer.data();
  }

  if (bufLength == 0 || bufPtr == nullptr)
  {
    QgsDebugMsg("Geometry is empty");
    feature.clearGeometry();
  }
  else
  {
    unsigned char* wkbGeom = new unsigned char[bufLength];
    memcpy(wkbGeom, bufPtr, bufLength);
    QgsGeometry geom;
    geom.fromWkb(wkbGeom, static_cast<int>(bufLength));
    feature.setGeometry(geom);
  }
}

bool QgsHanaFeatureIterator::nextFeatureFilterExpression(QgsFeature &feature)
{
  if (!mExpressionCompiled)
    return QgsAbstractFeatureIterator::nextFeatureFilterExpression(feature);
  else
    return fetchFeature(feature);
}

QString QgsHanaFeatureIterator::getBBOXFilter(const QgsRectangle& bbox,
  const QVersionNumber& dbVersion) const
{
  if (dbVersion.majorVersion() == 1)
  {
    return QString("%1.ST_SRID(%2).ST_IntersectsRect(ST_GeomFromText('Point(%3 %4)', %2), ST_GeomFromText('Point(%5 %6)', %2)) = 1")
      .arg(QgsHanaUtils::quotedIdentifier(mSource->mGeometryColumn), QString::number(mSource->mSrid),
        qgsDoubleToString(bbox.xMinimum()), qgsDoubleToString(bbox.yMinimum()),
        qgsDoubleToString(bbox.xMaximum()), qgsDoubleToString(bbox.yMaximum()));
  }
  else
    return QString("%1.ST_IntersectsRectPlanar(ST_GeomFromText('Point(%2 %3)', %6), ST_GeomFromText('Point(%4 %5)', %6)) = 1")
    .arg(QgsHanaUtils::quotedIdentifier(mSource->mGeometryColumn),
      qgsDoubleToString(bbox.xMinimum()), qgsDoubleToString(bbox.yMinimum()),
      qgsDoubleToString(bbox.xMaximum()), qgsDoubleToString(bbox.yMaximum()),
      QString::number(mSource->mSrid));
}

QString andWhereClauses(const QString &c1, const QString &c2)
{
  if (c1.isEmpty())
    return c2;
  if (c2.isEmpty())
    return c1;

  return QStringLiteral("(%1) AND (%2)").arg(c1, c2);
}

void QgsHanaFeatureIterator::buildStatement(const QgsFeatureRequest &request)
{
  QgsRectangle filterRect = mFilterRect;
  if (!mSrsExtent.isEmpty())
    filterRect = mSrsExtent.intersect(filterRect);

  if (!filterRect.isFinite())
    QgsMessageLog::logMessage(QObject::tr("Infinite filter rectangle specified"), QObject::tr("HANA"));

  bool limitAtProvider = (mRequest.limit() >= 0);
  QString sqlFields = "";

  // Add feature id column
  if (!mSource->mFidColumn.isEmpty())
  {
    sqlFields += QgsHanaUtils::quotedIdentifier(mSource->mFidColumn) + ",";
    mHasFidColumn = true;
  }

  bool subsetOfAttributes = mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes;
  QgsAttributeList attrs = (subsetOfAttributes && !request.subsetOfAttributes().isEmpty()) ?
    request.subsetOfAttributes() : mSource->mFields.allAttributesList();

  // Add attributes
  // ensure that all attributes required for expression filter are being fetched
  if (subsetOfAttributes && mRequest.filterType() == QgsFeatureRequest::FilterExpression)
  {
    //ensure that all fields required for filter expressions are prepared
    QSet<int> attributeIndexes = request.filterExpression()->referencedAttributeIndexes(mSource->mFields);
    attributeIndexes += attrs.toSet();
    attrs = attributeIndexes.toList();
  }

  Q_FOREACH(int i, attrs)
  {
    QString fieldname = mSource->mFields.at(i).name();
    if (mSource->mFidColumn == fieldname)
      continue;
    sqlFields += QStringLiteral("%1,").arg(QgsHanaUtils::quotedIdentifier(fieldname));
    mAttributesToFetch.append(i);
  }

  mHasAttributes = !mAttributesToFetch.isEmpty();

  // Add geometry column
  if ((!(request.flags() & QgsFeatureRequest::NoGeometry)
    || (request.filterType() == QgsFeatureRequest::FilterExpression &&
    request.filterExpression()->needsGeometry())) && mSource->isSpatial())
  {
    sqlFields += QStringLiteral("%1").arg(QgsHanaUtils::quotedIdentifier(mSource->mGeometryColumn));
    mHasGeometryColumn = true;
  }

  if (sqlFields.endsWith(','))
    sqlFields.truncate(sqlFields.length() - 1);

  if (sqlFields.isEmpty())
    sqlFields = "*";

  QString sql = QStringLiteral("SELECT %1 FROM %2.%3").arg(
    sqlFields,
    QgsHanaUtils::quotedIdentifier(mSource->mSchemaName),
    QgsHanaUtils::quotedIdentifier(mSource->mTableName));

  QString sqlFilter;
  // Set spatial filter
  if (!filterRect.isNull() && mSource->isSpatial() && !filterRect.isEmpty() && mHasGeometryColumn)
    sqlFilter = getBBOXFilter(filterRect, QgsHanaUtils::toHANAVersion(mConnRef->getDatabaseVersion()));

  // Set fid filter
  if (request.filterType() == QgsFeatureRequest::FilterFid && !mSource->mFidColumn.isEmpty())
  {
    QString inClause = QStringLiteral(" %1 = %2").arg(
      QgsHanaUtils::quotedIdentifier(mSource->mFidColumn),
      FID_TO_STRING(request.filterFid()));
    sqlFilter = andWhereClauses(sqlFilter, inClause);
  }
  else if (request.filterType() == QgsFeatureRequest::FilterFids && !mSource->mFidColumn.isEmpty()
    && !mRequest.filterFids().isEmpty())
  {
    QString delim;
    QString inClause = QStringLiteral("%1 IN (").arg(QgsHanaUtils::quotedIdentifier(mSource->mFidColumn));
    Q_FOREACH(QgsFeatureId featureId, mRequest.filterFids())
    {
      inClause += delim + FID_TO_STRING(featureId);
      delim = ',';
    }
    inClause.append(')');

    sqlFilter = andWhereClauses(sqlFilter, inClause);
  }

  //IMPORTANT - this MUST be the last clause added
  mExpressionCompiled = false;
  mCompileStatus = NoCompilation;
  if (request.filterType() == QgsFeatureRequest::FilterExpression)
  {
    // ensure that all attributes required for expression filter are being fetched
    if ((mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes) &&
      request.filterType() == QgsFeatureRequest::FilterExpression)
    {
      QgsAttributeList attrs = request.subsetOfAttributes();
      //ensure that all fields required for filter expressions are prepared
      QSet<int> attributeIndexes = request.filterExpression()->referencedAttributeIndexes(mSource->mFields);
      attributeIndexes += attrs.toSet();
      mRequest.setSubsetOfAttributes(attributeIndexes.toList());
    }

    if (QgsSettings().value(QStringLiteral("qgis/compileExpressions"), true).toBool())
    {
      QgsHanaExpressionCompiler compiler = QgsHanaExpressionCompiler(mSource);

      QgsSqlExpressionCompiler::Result result = compiler.compile(request.filterExpression());

      if (result == QgsSqlExpressionCompiler::Complete || result == QgsSqlExpressionCompiler::Partial)
      {
        QString filterExpr = compiler.result();
        if (!filterExpr.isEmpty())
        {
          sqlFilter = andWhereClauses(sqlFilter, filterExpr);
          //if only partial success when compiling expression, we need to double-check results
          //using QGIS' expressions
          mExpressionCompiled = (result == QgsSqlExpressionCompiler::Complete);
          mCompileStatus = (mExpressionCompiled ? Compiled : PartiallyCompiled);
        }
      }
      if (result != QgsSqlExpressionCompiler::Complete)
      {
        //can't apply limit at provider side as we need to check all results using QGIS expressions
        limitAtProvider = false;
      }
    }
    else
    {
      limitAtProvider = false;
    }
  }

  if (!mSource->mQueryWhereClause.isEmpty())
    sqlFilter = andWhereClauses(sqlFilter, mSource->mQueryWhereClause);

  if (!sqlFilter.isEmpty())
    sql += " WHERE " + sqlFilter;

  if (limitAtProvider && request.limit() > 0)
    sql += QStringLiteral(" LIMIT %1").arg(mRequest.limit());

  QgsDebugMsg(sql);

  mStatement = mConnRef->getNativeRef()->prepareStatement(sql.toStdString().c_str());
}

void QgsHanaFeatureIterator::ensureBufferCapacity(size_t capacity)
{
  if (capacity > mBuffer.size())
  {
    mBuffer.reserve(capacity);
  }
}

QgsHanaFeatureSource::QgsHanaFeatureSource(const QgsHanaProvider *p)
  : mUri(p->mUri)
  , mSchemaName(p->mSchemaName)
  , mTableName(p->mTableName)
  , mFidColumn(p->mFidColumn)
  , mFields(p->mAttributeFields)
  , mFieldInfos(p->mFieldInfos)
  , mGeometryColumn(p->mGeometryColumn)
  , mGeometryType(p->mGeometryType)
  , mSrid(p->mSrid)
  , mSrsExtent(p->mSrsExtent)
  , mCrs(p->crs())
  , mQueryWhereClause(p->mQueryWhereClause)
{
  if (p->mHasSrsPlanarEquivalent && p->mDatabaseVersion.majorVersion() <= 1)
    mSrid = QgsHanaUtils::toPlanarSRID(p->mSrid);
}

QgsHanaFeatureSource::~QgsHanaFeatureSource()
{
}

QgsFeatureIterator QgsHanaFeatureSource::getFeatures(const QgsFeatureRequest &request)
{
  return QgsFeatureIterator(new QgsHanaFeatureIterator(this, false, request));
}
