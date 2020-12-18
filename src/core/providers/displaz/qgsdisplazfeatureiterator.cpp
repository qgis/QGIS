/***************************************************************************
                         qgspointcloudindex.cpp
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

 /***************************************************************************
  *                                                                         *
  *   This program is free software; you can redistribute it and/or modify  *
  *   it under the terms of the GNU General Public License as published by  *
  *   the Free Software Foundation; either version 2 of the License, or     *
  *   (at your option) any later version.                                   *
  *                                                                         *
  ***************************************************************************/

#include "qgsdisplazfeatureiterator.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTime>
#include <QtDebug>
#include <QQueue>

#include "qgseptdecoder.h"
#include "qgscoordinatereferencesystem.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudattribute.h"
#include "qgslogger.h"
//class QgsdisplazfileLoader;
  ///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "displaz" )
#define PROVIDER_DESCRIPTION QStringLiteral( "displaz provider" )


QgsDisplazPointCloudIndex::QgsDisplazPointCloudIndex() = default;

QgsDisplazPointCloudIndex::~QgsDisplazPointCloudIndex() = default;

bool QgsDisplazPointCloudIndex::load(const QString &fileName)
{
  const QDir directory = QFileInfo(fileName).absoluteDir();
  mDirectory = directory.absolutePath();
  QgsdisplazfileLoader  *lasfileManager = Qgis::getlasfileManager();
  const GeometryCollection::GeometryVec& geoms = lasfileManager ->getDisPlaz_las_geometry()->get();
  bool isloaded = false;
  for (auto g = geoms.begin(); g != geoms.end(); ++g)
  {
    if ((*g)->fileName() == mDirectory)
    {
      isloaded = true;
      break;
    }
  }
  if (!isloaded)
  {
    FileLoadInfo loadInfo(fileName);
    loadInfo.replaceLabel = false;
    lasfileManager ->getDisPlaz_las_loader()->loadFile(loadInfo);
  }
  return isloaded;
}
QgsPointCloudBlock *QgsDisplazPointCloudIndex::nodeData(const IndexedPointCloudNode &n, const QgsPointCloudRequest &request)
{
    QgsPointCloudBlock * block;
  return block;
}
QgsCoordinateReferenceSystem QgsDisplazPointCloudIndex::crs() const
{
  return QgsCoordinateReferenceSystem::fromWkt(mWkt);
}

int QgsDisplazPointCloudIndex::pointCount() const
{
  return mPointCount;
}

QVariant QgsDisplazPointCloudIndex::metadataStatistic(const QString &attribute, QgsStatisticalSummary::Statistic statistic) const
{
  if (!mMetadataStats.contains(attribute))
    return QVariant();

  const AttributeStatistics &stats = mMetadataStats[attribute];
  switch (statistic)
  {
  case QgsStatisticalSummary::Count:
    return stats.count >= 0 ? QVariant(stats.count) : QVariant();

  case QgsStatisticalSummary::Mean:
    return std::isnan(stats.mean) ? QVariant() : QVariant(stats.mean);

  case QgsStatisticalSummary::StDev:
    return std::isnan(stats.stDev) ? QVariant() : QVariant(stats.stDev);

  case QgsStatisticalSummary::Min:
    return stats.minimum;

  case QgsStatisticalSummary::Max:
    return stats.maximum;

  case QgsStatisticalSummary::Range:
    return stats.minimum.isValid() && stats.maximum.isValid() ? QVariant(stats.maximum.toDouble() - stats.minimum.toDouble()) : QVariant();

  case QgsStatisticalSummary::CountMissing:
  case QgsStatisticalSummary::Sum:
  case QgsStatisticalSummary::Median:
  case QgsStatisticalSummary::StDevSample:
  case QgsStatisticalSummary::Minority:
  case QgsStatisticalSummary::Majority:
  case QgsStatisticalSummary::Variety:
  case QgsStatisticalSummary::FirstQuartile:
  case QgsStatisticalSummary::ThirdQuartile:
  case QgsStatisticalSummary::InterQuartileRange:
  case QgsStatisticalSummary::First:
  case QgsStatisticalSummary::Last:
  case QgsStatisticalSummary::All:
    return QVariant();
  }
  return QVariant();
}

QVariantList QgsDisplazPointCloudIndex::metadataClasses(const QString &attribute) const
{
  QVariantList classes;
  const QMap< int, int > values = mAttributeClasses.value(attribute);
  for (auto it = values.constBegin(); it != values.constEnd(); ++it)
  {
    classes << it.key();
  }
  return classes;
}

QVariant QgsDisplazPointCloudIndex::metadataClassStatistic(const QString &attribute, const QVariant &value, QgsStatisticalSummary::Statistic statistic) const
{
  if (statistic != QgsStatisticalSummary::Count)
    return QVariant();

  const QMap< int, int > values = mAttributeClasses.value(attribute);
  if (!values.contains(value.toInt()))
    return QVariant();
  return values.value(value.toInt());
}

bool QgsDisplazPointCloudIndex::loadHierarchy()
{
  QQueue<QString> queue;
  queue.enqueue(QStringLiteral("0-0-0-0"));
  while (!queue.isEmpty())
  {
    const QString filename = QStringLiteral("%1/ept-hierarchy/%2.json").arg(mDirectory).arg(queue.dequeue());
    QFile fH(filename);
    if (!fH.open(QIODevice::ReadOnly))
    {
      QgsDebugMsgLevel(QStringLiteral("unable to read hierarchy from file %1").arg(filename), 2);
      return false;
    }

    QByteArray dataJsonH = fH.readAll();
    QJsonParseError errH;
    QJsonDocument docH = QJsonDocument::fromJson(dataJsonH, &errH);
    if (errH.error != QJsonParseError::NoError)
    {
      QgsDebugMsgLevel(QStringLiteral("QJsonParseError when reading hierarchy from file %1").arg(filename), 2);
      return false;
    }

    QJsonObject rootHObj = docH.object();
    for (auto it = rootHObj.constBegin(); it != rootHObj.constEnd(); ++it)
    {
      QString nodeIdStr = it.key();
      int nodePointCount = it.value().toInt();
      if (nodePointCount < 0)
      {
        queue.enqueue(nodeIdStr);
      }
      else
      {
        IndexedPointCloudNode nodeId = IndexedPointCloudNode::fromString(nodeIdStr);
        mHierarchy[nodeId] = nodePointCount;
      }
    }
  }
  return true;
}

///@endcond
