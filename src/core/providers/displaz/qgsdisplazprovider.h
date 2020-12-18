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
#include "qgis_core.h"
#include "qgspointclouddataprovider.h"
#include "qgsprovidermetadata.h"

#include <memory>

class QgsDisplazPointCloudIndex;
 ///@cond PRIVATE
#define SIP_NO_FILE

class QgsDisplazProvider :public QgsPointCloudDataProvider//public QgsDataProvider, public QgsFeatureSink, public QgsFeatureSource
{
	Q_OBJECT

  public:
	
	 QgsDisplazProvider(const QString &uri,
     const QgsDataProvider::ProviderOptions &providerOptions);
    ~QgsDisplazProvider() override;

    QgsCoordinateReferenceSystem crs() const override;
    QgsRectangle extent() const override;
    QgsPointCloudAttributeCollection attributes() const override;
    bool isValid() const override;

    QString name() const override;

    QString description() const override;

	QString filePointCloudFilters() const;
	static void filePointCloudExtensions(QStringList &filePointCloudExtensions);

  QgsPointCloudIndex *index() const override;

  int pointCount() const override;

  QVariant metadataStatistic(const QString &attribute, QgsStatisticalSummary::Statistic statistic) const override;
  QVariantList metadataClasses(const QString &attribute) const override;
  QVariant metadataClassStatistic(const QString &attribute, const QVariant &value, QgsStatisticalSummary::Statistic statistic) const override;
  QVariantMap originalMetadata() const override;

private:
  std::unique_ptr<QgsDisplazPointCloudIndex> mIndex;
  bool mIsValid = false;

	std::shared_ptr<Geometry>  m_geom;
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
