/***************************************************************************
    QgsDisplazprovider.h
    -----------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HSLLCDISPLAZPROVIDER_H
#define HSLLCDISPLAZPROVIDER_H

#include <QString>
#include <QVector>
#include <QStringList>
#include <qfileinfo.h>
#include "qgis.h"
#include "qgscoordinatereferencesystem.h"
#include "qgspointclouddataprovider.h"

#include "qgswkbtypes.h"

#include "qgsdisplazfeatureiterator.h"
#include "qgsprovidermetadata.h"

#include "Geometry.h"
#include "PointArray.h"


class QgsDisplazProvider :public QgsPointCloudDataProvider//public QgsDataProvider, public QgsFeatureSink, public QgsFeatureSource
{
	Q_OBJECT

  public:
	
	 QgsDisplazProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options);
    ~QgsDisplazProvider() override;

	QString filePointCloudFilters() const;
	static void filePointCloudExtensions(QStringList &filePointCloudExtensions);

	//! Returns the memory provider key
	static QString providerKey();
	//! Returns the memory provider description
	static QString providerDescription();

	void setGeom(std::shared_ptr<Geometry> geom) override
	{
		m_geom = geom;
	};

	std::shared_ptr<Geometry>getGeom() const
	{
		return m_geom;
	}

	void setattribute();

	/**
	* Creates a new memory provider, with provider properties embedded within the given \a uri and \a options
	* argument.
	*/
	//static QgsDisplazProvider *createProvider(const QString &uri, const QgsVectorDataProvider::ProviderOptions &coordinateTransformContext);

	/* Implementation of functions from QgsVectorDataProvider */

	QgsAbstractFeatureSource *featureSource() const override;

	QString dataSourceUri(bool expandAuthConfig = true) const override;
	QString storageType() const override;
	QgsFeatureIterator getFeatures(const QgsFeatureRequest &request) const override;
	QgsWkbTypes::Type wkbType() const override;
	long featureCount() const override;
	QgsFields fields() const override;
	//bool addFeatures(QgsFeatureList &flist, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags()) override;
	//bool deleteFeatures(const QgsFeatureIds &id) override;
	bool addAttributes(const QList<QgsField> &attributes) override;
	bool renameAttributes(const QgsFieldNameMap &renamedAttributes) override;
	bool deleteAttributes(const QgsAttributeIds &attributes) override;
	bool changeAttributeValues(const QgsChangedAttributesMap &attr_map) override;
	bool changeGeometryValues(const QgsGeometryMap &geometry_map) override;
	QString subsetString() const override;
	bool setSubsetString(const QString &theSQL, bool updateFeatureCount = true) override;
	bool supportsSubsetString() const override { return true; }
	bool createSpatialIndex() override;
	QgsFeatureSource::SpatialIndexPresence hasSpatialIndex() const override;
	QgsVectorDataProvider::Capabilities capabilities() const override;
	bool truncate() override;

	/* Implementation of functions from QgsDataProvider */

	QString name() const override;
	QString description() const override;
	QgsRectangle extent() const override;
	void updateExtents() override;
	bool isValid() const override;
	QgsCoordinateReferenceSystem crs() const override;
	

  private:

	bool mValid = false;

	std::shared_ptr<Geometry>  m_geom;

	// Coordinate reference system
	QgsCoordinateReferenceSystem mCrs;

	// fields
	QgsFields mFields;
	QgsWkbTypes::Type mWkbType;
	mutable QgsRectangle mExtent;

	// features
	//QgsFeatureMap mFeatures;
	QgsFeatureId mNextFeatureId;

	// indexing
	//QgsSpatialIndex *mSpatialIndex = nullptr;

	QString mSubsetString;
};


class QgsDisplazProviderMetadata : public QgsProviderMetadata
{
	Q_OBJECT
public:
	QgsDisplazProviderMetadata();
  //QgsProviderMetadata::ProviderMetadataCapabilities capabilities() const override;
  QgsDisplazProvider *createProvider(const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags()) override;

  QList<QgsDataItemProvider *> dataItemProviders() const override;
  int priorityForUri(const QString &uri) const override;
  QList< QgsMapLayerType > validLayerTypesForUri(const QString &uri) const override;
  QString encodeUri(const QVariantMap &parts) const override;
  QVariantMap decodeUri(const QString &uri) const override;

  QString filters(QgsProviderMetadata::FilterType type) override;
};

#endif //HSLLCDISPLAZPROVIDER_H
