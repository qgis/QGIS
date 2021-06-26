/***************************************************************************
    qgsogrfeatureiterator.h
    ---------------------
    begin                : Juli 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
  std::shared_ptr<Geometry>& getgeom()
  {
    QgsdisplazfileLoader  *lasfileManager = QgsdisplazfileLoader::sInstance;
    GeometryCollection*  m_geometries = lasfileManager->getDisPlaz_las_geometry();
    const GeometryCollection::GeometryVec& geoms = m_geometries->get();
    if (isloaded == false)
    {
      for (auto g = geoms.begin(); g != geoms.end(); ++g)
      {
        if ((*g)->fileName() == mName)
        {
          //TODO
          Imath::Box3d m_bbox = (*g)->boundingBox();
          m_geom = (*g);
          mExtent.set(QgsPointXY(m_bbox.min.x, m_bbox.min.y), QgsPointXY(m_bbox.max.x, m_bbox.max.y));
          mZMin = m_bbox.min.z;
          mZMax = m_bbox.max.z;
          mPointCount = (*g)->pointCount();

          QgsPointCloudAttributeCollection attributes;

          const std::vector<PointCloudGeomField>*  m_pointarrayfields = (*g)->GetFiled();
          Imath::V3d offset = (*g)->offset();
          for (size_t i = 0; i < m_pointarrayfields->size(); ++i)
          {
            const PointCloudGeomField& field = (*m_pointarrayfields)[i];
            if (field.name == "position")
            {
              attributes.push_back(QgsPointCloudAttribute("X", QgsPointCloudAttribute::Float));
              attributes.push_back(QgsPointCloudAttribute("Y", QgsPointCloudAttribute::Float));
              attributes.push_back(QgsPointCloudAttribute("Z", QgsPointCloudAttribute::Float));
              // m_P = (V3f*)field.as<float>();
            }
            if (field.name == "classification")
            {
              // classificationindex = i;
              attributes.push_back(QgsPointCloudAttribute("Classification", QgsPointCloudAttribute::Char));
            }
            if (field.name == "intensity")
            {
              attributes.push_back(QgsPointCloudAttribute("Intensity", QgsPointCloudAttribute::UShort));
            }
            if (field.name == "returnNumber")
            {
              attributes.push_back(QgsPointCloudAttribute("ReturnNumber", QgsPointCloudAttribute::UShort));
            }
            if (field.name == "numberOfReturns")
            {
              attributes.push_back(QgsPointCloudAttribute("NumberOfReturns", QgsPointCloudAttribute::UShort));
            }
            if (field.name == "pointSourceId")
            {
              attributes.push_back(QgsPointCloudAttribute("PointSourceId", QgsPointCloudAttribute::UShort));
            }
            if (field.name == "color")
            {
              attributes.push_back(QgsPointCloudAttribute("Color", QgsPointCloudAttribute::UShort));
            }
          }
          isloaded = true;
          setAttributes(attributes);
          mRootBounds = QgsPointCloudDataBounds
          (m_bbox.min.x, m_bbox.min.y, m_bbox.min.z,
            m_bbox.max.x, m_bbox.max.y, m_bbox.max.z);
          //std::static_pointer_cast<PointArray>(m_geom)->GetrootNode().get();
          m_renderextent = mExtent;

          break;
        }
      }

    }

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
