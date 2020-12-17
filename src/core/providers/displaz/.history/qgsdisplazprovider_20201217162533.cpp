/***************************************************************************
                         qgsmeshmemorydataprovider.cpp
                         -----------------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
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

#include <string>

#include "qqgsdisplazprovider.h"
#include "qgslogger.h"
#include "qgsapplication.h"

#ifdef HAVE_GUI
//#include "qgssourceselectprovider.h"
//#include "qgsdisplazsourceselect.h"
#endif

#include "qgsdisplazdataitems.h"

static const QString TEXT_PROVIDER_KEY = QStringLiteral("pdal");
static const QString TEXT_PROVIDER_DESCRIPTION = QStringLiteral("PDAL provider");


static QString createFileFilter_(QString const &longName, QString const &glob)
{
	// return longName + " [OGR] (" + glob.toLower() + ' ' + glob.toUpper() + ");;";
	return longName + " (" + glob.toLower() + ' ' + glob.toUpper() + ");;";
} // createFileFilter_



QgsDisplazProvider::QgsDisplazProvider(const QString &uri, const QgsDataProvider::ProviderOptions &options)
: QgsVectorDataProvider(uri, options)
{
	//QString fileFilter = "*." + QFileInfo(uri).suffix();

	QUrl url = QUrl::fromEncoded(uri.toUtf8());
	const QUrlQuery query(url);
	mWkbType = QgsWkbTypes::MultiPointZ; //ltiPointZ;
	
	if (query.hasQueryItem(QStringLiteral("crs")))
	{
		QString crsDef = query.queryItemValue(QStringLiteral("crs"));
		mCrs.createFromString(crsDef);
	}
	else
	{
		// TODO - remove in HSLLC 4.0. Layers without an explicit CRS set SHOULD have an invalid CRS. But in order to maintain
		// 3.x api, we have to be tolerant/shortsighted(?) here and fallback to EPSG:4326
		mCrs = QgsCoordinateReferenceSystem(QStringLiteral("EPSG:4326"));
	}
	mNextFeatureId = 1;

	setNativeTypes(QList< NativeType >()
		<< QgsVectorDataProvider::NativeType(tr("Whole number (integer)"), QStringLiteral("integer"), QVariant::Int, 0, 10)
		// Decimal number from OGR/Shapefile/dbf may come with length up to 32 and
		// precision up to length-2 = 30 (default, if width is not specified in dbf is length = 24 precision = 15)
		// We know that double (QVariant::Double) has only 15-16 significant numbers,
		// but setting that correct limits would disable the use of memory provider with
		// data from Shapefiles. In any case, the data are handled as doubles.
		// So the limits set here are not correct but enable use of data from Shapefiles.
		<< QgsVectorDataProvider::NativeType(tr("Decimal number (real)"), QStringLiteral("double"), QVariant::Double, 0, 32, 0, 30)
		<< QgsVectorDataProvider::NativeType(tr("Text (string)"), QStringLiteral("string"), QVariant::String, 0, 255)

		// date type
		<< QgsVectorDataProvider::NativeType(tr("Date"), QStringLiteral("date"), QVariant::Date, -1, -1, -1, -1)
		<< QgsVectorDataProvider::NativeType(tr("Time"), QStringLiteral("time"), QVariant::Time, -1, -1, -1, -1)
		<< QgsVectorDataProvider::NativeType(tr("Date & Time"), QStringLiteral("datetime"), QVariant::DateTime, -1, -1, -1, -1)

		// integer types
		<< QgsVectorDataProvider::NativeType(tr("Whole number (smallint - 16bit)"), QStringLiteral("int2"), QVariant::Int, -1, -1, 0, 0)
		<< QgsVectorDataProvider::NativeType(tr("Whole number (integer - 32bit)"), QStringLiteral("int4"), QVariant::Int, -1, -1, 0, 0)
		<< QgsVectorDataProvider::NativeType(tr("Whole number (integer - 64bit)"), QStringLiteral("int8"), QVariant::LongLong, -1, -1, 0, 0)
		<< QgsVectorDataProvider::NativeType(tr("Decimal number (numeric)"), QStringLiteral("numeric"), QVariant::Double, 1, 20, 0, 20)
		<< QgsVectorDataProvider::NativeType(tr("Decimal number (decimal)"), QStringLiteral("decimal"), QVariant::Double, 1, 20, 0, 20)

		// floating point
		<< QgsVectorDataProvider::NativeType(tr("Decimal number (real)"), QStringLiteral("real"), QVariant::Double, -1, -1, -1, -1)
		<< QgsVectorDataProvider::NativeType(tr("Decimal number (double)"), QStringLiteral("double precision"), QVariant::Double, -1, -1, -1, -1)

		// string types
		<< QgsVectorDataProvider::NativeType(tr("Text, unlimited length (text)"), QStringLiteral("text"), QVariant::String, -1, -1, -1, -1)

		// boolean
		<< QgsVectorDataProvider::NativeType(tr("Boolean"), QStringLiteral("bool"), QVariant::Bool)

		// blob
		<< QgsVectorDataProvider::NativeType(tr("Binary object (BLOB)"), QStringLiteral("binary"), QVariant::ByteArray)

	);

}

QgsDisplazProvider::~QgsDisplazProvider()
{

}

QString QgsDisplazProvider::filePointCloudFilters() const
{
	QString sFileFilters;

	sFileFilters += createFileFilter_(QObject::tr("Lidar Point Cloud Data"), QStringLiteral("*.las"));

	sFileFilters += createFileFilter_(QObject::tr("Lidar Point Cloud Data"), QStringLiteral("*.laz"));

	return sFileFilters;
}
void QgsDisplazProvider::filePointCloudExtensions(QStringList &filePointCloudExtensions)
{
	filePointCloudExtensions.clear();
	filePointCloudExtensions.append(QString("las"));
	filePointCloudExtensions.append(QString("laz"));
	QgsDebugMsg("PointCloud extensions list built: " + filePointCloudExtensions.join(QStringLiteral(";;")));
}



QString QgsDisplazProvider::providerKey()
{
	return TEXT_PROVIDER_KEY;
}

QString QgsDisplazProvider::providerDescription()
{
	return TEXT_PROVIDER_DESCRIPTION;
}

/*QgsDisplazProvider *QgsDisplazProvider::createProvider(const QString &uri, const ProviderOptions &options)
{
	return new QgsDisplazProvider(geom, uri, options);
}*/

void QgsDisplazProvider::setattribute()
{
	if (m_geom)
	{
		QList<QgsField> attributes;
		QRegExp reFieldDef("\\:"
			"(int|integer|long|int8|real|double|string|date|time|datetime|binary|bool|boolean)" // type
			"(?:\\((\\-?\\d+)"                // length
			"(?:\\,(\\-?\\d+))?"                  // precision
			"\\))?(\\[\\])?"                  // array
			"$", Qt::CaseInsensitive);

		const std::vector<PointCloudGeomField>* field_geom = m_geom->GetFiled();

		for (int i = 0; i < field_geom->size(); i++)
		{
			QString name = QString::fromStdString((*field_geom)[i].name);
			TypeSpec::Type  geom_type = (*field_geom)[i].spec.type;
			QVariant::Type type;
			QString typeName(QStringLiteral("float"));
			int length = 255;
			int precision = 0;
			if (geom_type == TypeSpec::Type::Float)
			{
				typeName = QStringLiteral("float");
				type = QVariant::Type::Double;
				length = -1;
			}
			if (geom_type == TypeSpec::Type::Int)
			{
				typeName = QStringLiteral("int");
				type = QVariant::Type::Int;
				length = -1;
			}
			if (geom_type == TypeSpec::Type::Uint)
			{
				typeName = QStringLiteral("uint");
				type = QVariant::Type::Int;
				length = -1;
			}
			if (geom_type == TypeSpec::Type::Unknown)
			{
				typeName = QStringLiteral("uint");
				type = QVariant::Type::Int;
				length = -1;
			}
			if (!name.isEmpty())
				attributes.append(QgsField(name, type, typeName, length, precision, QString(), type));
		}
		addAttributes(attributes);
	}
}

QgsAbstractFeatureSource *QgsDisplazProvider::featureSource() const
{
	return new QgsDisplazFeatureSource(this);
}

QString QgsDisplazProvider::dataSourceUri(bool expandAuthConfig) const
{
	return QgsDataProvider::dataSourceUri();
}

QString QgsDisplazProvider::storageType() const
{
	return QStringLiteral("Memory storage");
}

QgsFeatureIterator QgsDisplazProvider::getFeatures(const QgsFeatureRequest &request) const
{
	return QgsFeatureIterator(new QgsDisplazFeatureIterator(new QgsDisplazFeatureSource(this), true, request));
}


QgsRectangle QgsDisplazProvider::extent() const
{
	//mExtent.setMinimal();
	Imath::Box3d m_bbox = m_geom->boundingBox();
	mExtent.set(QgsPointXY(m_bbox.min.x,m_bbox.min.y), QgsPointXY(m_bbox.max.x, m_bbox.max.y));
	return mExtent;
}

QgsWkbTypes::Type QgsDisplazProvider::wkbType() const
{
	return QgsWkbTypes::Type::MultiPointZ;
}

long QgsDisplazProvider::featureCount() const
{
	
	return m_geom->pointCount();
}

QgsFields QgsDisplazProvider::fields() const
{
	return mFields;
}

bool QgsDisplazProvider::isValid() const
{
	return (mWkbType != QgsWkbTypes::Unknown);
}

QgsCoordinateReferenceSystem QgsDisplazProvider::crs() const
{
	// TODO: make provider projection-aware
	return mCrs; // return default CRS
}

bool QgsDisplazProvider::addAttributes(const QList<QgsField> &attributes)
{
	for (QList<QgsField>::const_iterator it = attributes.begin(); it != attributes.end(); ++it)
	{
		switch (it->type())
		{
		case QVariant::Int:
		case QVariant::Double:
		case QVariant::String:
		case QVariant::Date:
		case QVariant::Time:
		case QVariant::DateTime:
		case QVariant::LongLong:
		case QVariant::StringList:
		case QVariant::List:
		case QVariant::Bool:
		case QVariant::ByteArray:
			break;
		default:
			QgsDebugMsg("Field type not supported: " + it->typeName());
			continue;
		}
		// add new field as a last one
		mFields.append(*it);
		/*
		for (QgsFeatureMap::iterator fit = mFeatures.begin(); fit != mFeatures.end(); ++fit)
		{
			QgsFeature &f = fit.value();
			QgsAttributes attr = f.attributes();
			attr.append(QVariant());
			f.setAttributes(attr);
		}
		*/
		
	}
	return true;
}

bool QgsDisplazProvider::renameAttributes(const QgsFieldNameMap &renamedAttributes)
{
	QgsFieldNameMap::const_iterator renameIt = renamedAttributes.constBegin();
	bool result = true;
	for (; renameIt != renamedAttributes.constEnd(); ++renameIt)
	{
		int fieldIndex = renameIt.key();
		if (fieldIndex < 0 || fieldIndex >= mFields.count())
		{
			result = false;
			continue;
		}
		if (mFields.indexFromName(renameIt.value()) >= 0)
		{
			//field name already in use
			result = false;
			continue;
		}

		mFields.rename(fieldIndex, renameIt.value());
	}
	return result;
}

bool QgsDisplazProvider::deleteAttributes(const QgsAttributeIds &attributes)
{
	QList<int> attrIdx = qgis::setToList(attributes);
	std::sort(attrIdx.begin(), attrIdx.end(), std::greater<int>());

	// delete attributes one-by-one with decreasing index
	for (QList<int>::const_iterator it = attrIdx.constBegin(); it != attrIdx.constEnd(); ++it)
	{
		int idx = *it;
		mFields.remove(idx);
		/*
		for (QgsFeatureMap::iterator fit = mFeatures.begin(); fit != mFeatures.end(); ++fit)
		{
			QgsFeature &f = fit.value();
			QgsAttributes attr = f.attributes();
			attr.remove(idx);
			f.setAttributes(attr);
		}
		*/
	}
	clearMinMaxCache();
	return true;
}

bool QgsDisplazProvider::changeAttributeValues(const QgsChangedAttributesMap &attr_map)
{
	/*
	for (QgsChangedAttributesMap::const_iterator it = attr_map.begin(); it != attr_map.end(); ++it)
	{  
		
		QgsFeatureMap::iterator fit = mFeatures.find(it.key());
		if (fit == mFeatures.end())
			continue;
		

		const QgsAttributeMap &attrs = it.value();
		for (QgsAttributeMap::const_iterator it2 = attrs.constBegin(); it2 != attrs.constEnd(); ++it2)
			fit->setAttribute(it2.key(), it2.value());
	}
	clearMinMaxCache();
	*/
	return true;
}

bool QgsDisplazProvider::changeGeometryValues(const QgsGeometryMap &geometry_map)
{
	/*
	for (QgsGeometryMap::const_iterator it = geometry_map.begin(); it != geometry_map.end(); ++it)
	{
		QgsFeatureMap::iterator fit = mFeatures.find(it.key());
		if (fit == mFeatures.end())
			continue;

		fit->setGeometry(it.value());

	}

	updateExtents();
	*/
	return true;
}

QString QgsDisplazProvider::subsetString() const
{
	return mSubsetString;
}

bool QgsDisplazProvider::setSubsetString(const QString &theSQL, bool updateFeatureCount)
{
	Q_UNUSED(updateFeatureCount)

		if (!theSQL.isEmpty())
		{
			QgsExpression tempExpression(theSQL);
			if (tempExpression.hasParserError())
				return false;
		}

	if (theSQL == mSubsetString)
		return true;

	mSubsetString = theSQL;
	clearMinMaxCache();
	mExtent.setMinimal();

	emit dataChanged();
	return true;
}

bool QgsDisplazProvider::createSpatialIndex()
{

	return false;
}

QgsFeatureSource::SpatialIndexPresence QgsDisplazProvider::hasSpatialIndex() const
{
	return SpatialIndexNotPresent;
}

QgsVectorDataProvider::Capabilities QgsDisplazProvider::capabilities() const
{
	return AddFeatures | DeleteFeatures | ChangeGeometries |
		ChangeAttributeValues | AddAttributes | DeleteAttributes | RenameAttributes | CreateSpatialIndex |
		SelectAtId | CircularGeometries | FastTruncate;
}

bool QgsDisplazProvider::truncate()
{
	//mFeatures.clear();
	//clearMinMaxCache();
	//mExtent.setMinimal();
	return true;
}

void QgsDisplazProvider::updateExtents()
{
	//mExtent.setMinimal();
}

QString QgsDisplazProvider::name() const
{
	return TEXT_PROVIDER_KEY;
}

QString QgsDisplazProvider::description() const
{
	return TEXT_PROVIDER_DESCRIPTION;
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
 
 QgsDisplazProvider *QgsDisplazProviderMetadata::createProvider(const QString &uri, const QgsDataProvider::ProviderOptions &options)
 {
	 return new QgsDisplazProvider(uri, options);
 }
 

 QList<QgsDataItemProvider *> QgsDisplazProviderMetadata::dataItemProviders() const
 {
	 QList<QgsDataItemProvider *> providers;
	 providers << new QgsDisplazDataItemProvider;
	 return providers;
 }

 QString QgsDisplazProviderMetadata::filters(FilterType type)
 {

	return  filePointCloudFilters();

 }

 QgsDisplazProviderMetadata::QgsDisplazProviderMetadata()
	 : QgsProviderMetadata(TEXT_PROVIDER_KEY, TEXT_PROVIDER_DESCRIPTION)
 {
 
 }

 QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
 {
	 return new QgsDisplazProviderMetadata();
 }

 