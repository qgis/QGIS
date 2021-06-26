/***************************************************************************
                         qgsmeshmemorydataprovider.cpp
                         -----------------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
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

#include <string>

#include "qgsdisplazprovider.h"

#include "qgsdisplazdataitems.h"

#include "qgis.h"
#include "qgseptpointcloudindex.h"
#include "qgseptdataitems.h"
#include "qgsruntimeprofiler.h"
#include "qgsapplication.h"

class QgsRectangle;
class QgsCoordinateReferenceSystem;
class QgsDataProviderTemporalCapabilities;

static const QString TEXT_PROVIDER_KEY = QStringLiteral("displaz");
static const QString TEXT_PROVIDER_DESCRIPTION = QStringLiteral("displaz provider");


static QString createFileFilter_(QString const &longName, QString const &glob)
{
	// return longName + " [OGR] (" + glob.toLower() + ' ' + glob.toUpper() + ");;";
	return longName + " (" + glob.toLower() + ' ' + glob.toUpper() + ");;";
} // createFileFilter_


QgsDisplazProvider::QgsDisplazProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options )
  : QgsPointCloudDataProvider(uri, options, FlagTrustDataSource)
  , mIndex(new QgsDisplazPointCloudIndex)
{
  std::unique_ptr< QgsScopedRuntimeProfile > profile;
  if (QgsApplication::profiler()->groupIsActive(QStringLiteral("projectload")))
    profile = qgis::make_unique< QgsScopedRuntimeProfile >(tr("Open data source"), QStringLiteral("projectload"));

  mIsValid = mIndex->load(uri);
  //m_geom = mIndex->getgeom();
}

QgsDisplazProvider::~QgsDisplazProvider()
{

}


QgsCoordinateReferenceSystem QgsDisplazProvider::crs() const
{
  return mIndex->crs();
}

QgsRectangle QgsDisplazProvider::extent() const
{
  std::shared_ptr<Geometry> m_geom = mIndex->getgeom();
  if (m_geom)
  {
   // m_geom->
    return QgsRectangle(m_geom->boundingBox().min.x, m_geom->boundingBox().min.y, m_geom->boundingBox().max.x, m_geom->boundingBox().max.y);
  }
  else
  {
    return mIndex->extent();
  }
 
}

QgsPointCloudAttributeCollection QgsDisplazProvider::attributes() const
{
  return mIndex->attributes();
}

bool QgsDisplazProvider::isValid() const
{
  return mIsValid;
}

QString QgsDisplazProvider::name() const
{
  return TEXT_PROVIDER_KEY;
}

QString QgsDisplazProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
}

void QgsDisplazProvider::filePointCloudExtensions(QStringList & filePointCloudExtensions)
{
    filePointCloudExtensions.clear();
    filePointCloudExtensions.append(QString("las"));
    filePointCloudExtensions.append(QString("laz"));
    filePointCloudExtensions.append(QString("hsp"));
   // QgsDebugMsg("PointCloud extensions list built: " + filePointCloudExtensions.join(QStringLiteral(";;")));
}

QgsPointCloudIndex *QgsDisplazProvider::index() const
{
  return mIndex.get();
}

int QgsDisplazProvider::pointCount() const
{
  return mIndex->pointCount();
}

QVariantList QgsDisplazProvider::metadataClasses(const QString &attribute) const
{
  return mIndex->metadataClasses(attribute);
}

QVariant QgsDisplazProvider::metadataClassStatistic(const QString &attribute, const QVariant &value, QgsStatisticalSummary::Statistic statistic) const
{
  return mIndex->metadataClassStatistic(attribute, value, statistic);
}

QVariantMap QgsDisplazProvider::originalMetadata() const
{
  return mIndex->originalMetadata();
}

QVariant QgsDisplazProvider::metadataStatistic(const QString &attribute, QgsStatisticalSummary::Statistic statistic) const
{
  return mIndex->metadataStatistic(attribute, statistic);
}

/*----------------------------------------------------------------------------------------------*/

/**
* Class factory to return a pointer to a newly created
* QgsGdalProvider object
*/
/*
QGISEXTERN QgsDisplazProvider *classFactory(const QString *uri, const QgsDataProvider::ProviderOptions &options)
{
	return new QgsDisplazProvider(*uri, options);
}
*/
/**
 * Required key function (used to map the plugin to a data store type)
*/
 QString providerKey()
{
  return TEXT_PROVIDER_KEY;
}

/**
 * Required description function
 */
 QString description()
{
  return TEXT_PROVIDER_DESCRIPTION;
}

/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
 bool isProvider()
{
  return true;
}

 void cleanupProvider()
{
}

 QString filePointCloudFilters()
{
	QString sFileFilters;

	sFileFilters += createFileFilter_(QObject::tr("Lidar Point Cloud Data"), QStringLiteral("*.las"));

	sFileFilters += createFileFilter_(QObject::tr("Lidar Point Cloud Data"), QStringLiteral("*.laz"));

  sFileFilters += createFileFilter_(QObject::tr("Hyperspectral Lidar Point Cloud Data"), QStringLiteral("*.hsp"));

	return sFileFilters;
}


/*
QgsDisplazFeatureSource::QgsDisplazFeatureSource(const QgsDisplazProvider * p)
{



}
* /
QgsDisplazFeatureSource::~QgsDisplazFeatureSource()
{
}
/*
QgsFeatureIterator QgsDisplazFeatureSource::getFeatures(const QgsFeatureRequest & request)
{
	return QgsFeatureIterator();
}
*/

 //--------------------------------------------------wp-----------------


 /*----------------------------------------------------------------------------------------------*/
 
 QgsDisplazProvider *QgsDisplazProviderMetadata::createProvider(const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags)
 {
   return new QgsDisplazProvider(uri, options);// , flags);
 }
 

 QList<QgsDataItemProvider *> QgsDisplazProviderMetadata::dataItemProviders() const
 {
	 QList<QgsDataItemProvider *> providers;
	 providers << new QgsDisplazDataItemProvider;
	 return providers;
 }

 int QgsDisplazProviderMetadata::priorityForUri(const QString &uri) const
 {
   const QVariantMap parts = decodeUri(uri);
   QFileInfo fi(parts.value(QStringLiteral("path")).toString());
   if (fi.suffix().compare(QLatin1String("las"), Qt::CaseInsensitive) == 0)
   {
     return 100;
   }
   if (fi.suffix().compare(QLatin1String("laz"), Qt::CaseInsensitive) == 0)
   {
     return 100;
   }
   if (fi.suffix().compare(QLatin1String("hsp"), Qt::CaseInsensitive) == 0)
   {
     return 100;
   }
   return 0;
 }

 QList<QgsMapLayerType> QgsDisplazProviderMetadata::validLayerTypesForUri(const QString &uri) const
 {
   const QVariantMap parts = decodeUri(uri);
   QFileInfo fi(parts.value(QStringLiteral("path")).toString());
   if (fi.suffix().compare(QLatin1String("las"), Qt::CaseInsensitive) == 0)
     return QList< QgsMapLayerType>() << QgsMapLayerType::PointCloudLayer;
   if (fi.suffix().compare(QLatin1String("laz"), Qt::CaseInsensitive) == 0)
     return QList< QgsMapLayerType>() << QgsMapLayerType::PointCloudLayer;
   if (fi.suffix().compare(QLatin1String("hsp"), Qt::CaseInsensitive) == 0)
     return QList< QgsMapLayerType>() << QgsMapLayerType::PointCloudLayer;
   return QList< QgsMapLayerType>();
 }
 QVariantMap  QgsDisplazProviderMetadata::decodeUri(const QString &uri) const
 {
   const QString path = uri;
   QVariantMap uriComponents;
   uriComponents.insert(QStringLiteral("path"), path);
   return uriComponents;
 }
 QString QgsDisplazProviderMetadata::filters(QgsProviderMetadata::FilterType type)
 {
   switch (type)
   {
   case QgsProviderMetadata::FilterType::FilterVector:
   case QgsProviderMetadata::FilterType::FilterRaster:
   case QgsProviderMetadata::FilterType::FilterMesh:
   case QgsProviderMetadata::FilterType::FilterMeshDataset:
     return QString();
   case QgsProviderMetadata::FilterType::FilterPointCloud:
     return  filePointCloudFilters();
   }
   return QString();
 }

 QString QgsDisplazProviderMetadata::encodeUri(const QVariantMap &parts) const
 {
   const QString path = parts.value(QStringLiteral("path")).toString();
   return path;
 }

 QgsDisplazProviderMetadata::QgsDisplazProviderMetadata()
	 : QgsProviderMetadata(TEXT_PROVIDER_KEY, TEXT_PROVIDER_DESCRIPTION)
 {
 
 }

 QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
 {
	 return new QgsDisplazProviderMetadata();
 }

