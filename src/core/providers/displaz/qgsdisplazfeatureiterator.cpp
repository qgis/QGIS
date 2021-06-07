/***************************************************************************
                         qgspointcloudindex.cpp
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

 /***************************************************************************
  *                                                                         *
  *   *
  *  
  *        *
  *                                     *
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

#include "qgscoordinatereferencesystem.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudattribute.h"
#include "qgslogger.h"
//class QgsdisplazfileLoader;
  ///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "displaz" )
#define PROVIDER_DESCRIPTION QStringLiteral( "displaz provider" )
  //////-------------------------------

QgsdisplazfileLoader *QgsdisplazfileLoader::sInstance = nullptr;

QgsdisplazfileLoader*QgsdisplazfileLoader::instance()
{
	if (sInstance == nullptr)
	{
		return new QgsdisplazfileLoader;
	}
	else
	{
		return sInstance;
	}
}

QgsdisplazfileLoader::QgsdisplazfileLoader()
{
	sInstance = this;
}
QgsdisplazfileLoader ::~QgsdisplazfileLoader()
{
};

QgsDisplazPointCloudIndex::QgsDisplazPointCloudIndex() = default;

QgsDisplazPointCloudIndex::~QgsDisplazPointCloudIndex() = default;

bool QgsDisplazPointCloudIndex::load(const QString &fileName)
{
  mName = fileName;
  const QDir directory = QFileInfo(fileName).absoluteDir();
  mDirectory = directory.absolutePath();
  isloaded = false;
  mScale.set(1.0, 1.0, 1.0);
  return true;
}
void QgsDisplazPointCloudIndex:: RootNode(QgsRectangle &extent)
{
	QgsdisplazfileLoader  *lasfileManager = QgsdisplazfileLoader::sInstance;

	GeometryCollection*  m_geometries = lasfileManager->getDisPlaz_las_geometry();
	//const GeometryCollection::GeometryVec GeometryVec = m_geometries->get();
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
				break;
			}
		}

	}
	m_renderextent = extent;
}
DrawCount QgsDisplazPointCloudIndex::getData()
{
	int decimal_step = 1;
	//m_renderextent;
	float scale = mExtent.width() / m_renderextent.width();
	float xmin = m_renderextent.xMinimum();
	float xmax = m_renderextent.xMaximum();
	float ymin = m_renderextent.yMinimum();
	float ymax = m_renderextent.yMaximum();
	Imath::Box2f filterbox(Imath::V2f(xmin, ymin), Imath::V2f(xmax, ymax));
	DrawCount mdrawlist = m_geom->getPointsOnlyInFilterRect(std::log10((std::pow(scale + 4, 1.5))) + 10.0, false, filterbox);
	return mdrawlist;
}

DrawCount QgsDisplazPointCloudIndex::getDataMore()
{
	int decimal_step = 1;
	//m_renderextent;
	float scale = mExtent.width() / m_renderextent.width();
	float xmin = m_renderextent.xMinimum();
	float xmax = m_renderextent.xMaximum();
	float ymin = m_renderextent.yMinimum();
	float ymax = m_renderextent.yMaximum();
	Imath::Box2f filterbox(Imath::V2f(xmin, ymin), Imath::V2f(xmax, ymax));
	DrawCount mdrawlist = m_geom->getPointsOnlyInFilterRect(std::log10((std::pow(scale + 4, 1.5))) + 15.0, true, filterbox);
	return mdrawlist;
}

QgsPointCloudBlock *QgsDisplazPointCloudIndex::nodeData(const IndexedPointCloudNode &n, const QgsPointCloudRequest &request)
{
	Q_UNUSED(n);
	//OctreeNode* m_rootNode =std::static_pointer_cast<PointArray>(m_geom)->GetrootNode().get();
	
	//const int count = mdrawlist.numVertices / decimal_step;
	//const std::size_t pointRecordSize = attributes().pointRecordSize();
//	const std::size_t requestedPointRecordSize = request.attributes().pointRecordSize();

	QgsPointCloudBlock*block;
	return block ;
}
QgsCoordinateReferenceSystem QgsDisplazPointCloudIndex::crs() const
{
	return QgsCoordinateReferenceSystem(QStringLiteral("EPSG:4326"));
 // return QgsCoordinateReferenceSystem::fromWkt(mWkt);
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
