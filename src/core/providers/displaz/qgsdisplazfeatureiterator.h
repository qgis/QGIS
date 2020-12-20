/***************************************************************************
    qgsogrfeatureiterator.h
    ---------------------
    begin                : Juli 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef HSLLCDISPLAZFEATUREITERATOR_H
#define HSLLCDISPLAZFEATUREITERATOR_H

#include <Geometry.h>
#include <PointArray.h>

#include <QObject>
#include <QString>
#include <QHash>
#include <QStringList>
#include <QVector>
#include <QList>

#include "qgspointcloudindex.h"
#include "qgspointcloudattribute.h"
#include "qgsstatisticalsummary.h"
#include "qgis.h"

#include <fileloader.h>
#include <geometrycollection.h>

//#include "PointViewerMainWindow.h"
class QgsDisplazProvider;
class QgsCoordinateReferenceSystem;

class CORE_EXPORT  QgsdisplazfileLoader :public QObject
{
	Q_OBJECT
public:
	QgsdisplazfileLoader();

	~QgsdisplazfileLoader();

public:
	int m_maxPointCount = 200000000;

	static  QgsdisplazfileLoader *sInstance;

	static QgsdisplazfileLoader *instance();

	FileLoader* getDisPlaz_las_loader()
	{
		return g_PointCloudfileLoader;
	}
	void setlasloader(FileLoader* _PointCloudfileLoader)
	{
		g_PointCloudfileLoader = _PointCloudfileLoader;
	}
	GeometryCollection* getDisPlaz_las_geometry()
	{
		return g_PointCloudGeoms;
	}
	void setlas_geometry(GeometryCollection* _PointCloudGeoms)
	{
		g_PointCloudGeoms = _PointCloudGeoms;
	}
     FileLoader* g_PointCloudfileLoader =nullptr ;
	 GeometryCollection* g_PointCloudGeoms =nullptr;

};

class QgsDisplazPointCloudIndex : public QgsPointCloudIndex
{
  Q_OBJECT
public:

  explicit QgsDisplazPointCloudIndex();
  ~QgsDisplazPointCloudIndex();
  

  bool load(const QString &fileName) override;

 void RootNode(QgsRectangle &extent);
  DrawCount  getData();
  DrawCount getDataMore();
  std::shared_ptr<Geometry> getgeom()
  {
	  return m_geom;
  }
  QgsPointCloudBlock *nodeData(const IndexedPointCloudNode &n, const QgsPointCloudRequest &request) override;

  QgsCoordinateReferenceSystem crs() const;
  int pointCount() const;
  QVariant metadataStatistic(const QString &attribute, QgsStatisticalSummary::Statistic statistic) const;
  QVariantList metadataClasses(const QString &attribute) const;
  QVariant metadataClassStatistic(const QString &attribute, const QVariant &value, QgsStatisticalSummary::Statistic statistic) const;

  QVariantMap originalMetadata() const { return mOriginalMetadata; }

private:
  bool loadHierarchy();

  QString mDataType;
  QString mDirectory;
  QString mWkt;
  QString mName;
  QgsRectangle m_renderextent;
  int mPointCount = 0;
  std::shared_ptr<Geometry> m_geom;
 
  struct AttributeStatistics
  {
    int count = -1;
    QVariant minimum;
    QVariant maximum;
    double mean = std::numeric_limits< double >::quiet_NaN();
    double stDev = std::numeric_limits< double >::quiet_NaN();
    double variance = std::numeric_limits< double >::quiet_NaN();
  };

  QMap< QString, AttributeStatistics > mMetadataStats;

  QMap< QString, QMap< int, int > > mAttributeClasses;
  QVariantMap mOriginalMetadata;
};

#endif // HSLLCDISPLAZFEATUREITERATOR_H
