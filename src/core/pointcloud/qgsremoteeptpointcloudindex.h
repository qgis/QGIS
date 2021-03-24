/***************************************************************************
                         qgsremoteeptpointcloudindex.h
                         --------------------
    begin                : March 2021
    copyright            : (C) 2021 by Belgacem Nedjima
    email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSREMOTEEPTPOINTCLOUDINDEX_H
#define QGSREMOTEEPTPOINTCLOUDINDEX_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QStringList>
#include <QVector>
#include <QList>
#include <QFile>
#include <QUrl>

#include "qgspointcloudindex.h"
#include "qgspointcloudattribute.h"
#include "qgsstatisticalsummary.h"
#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsCoordinateReferenceSystem;
class QgsTileDownloadManager;

class CORE_EXPORT QgsRemoteEptPointCloudIndex: public QgsPointCloudIndex
{
    Q_OBJECT
  public:

    explicit QgsRemoteEptPointCloudIndex();
    ~QgsRemoteEptPointCloudIndex();

    void load( const QString &fileName ) override;

    QgsPointCloudBlock *nodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request ) override;
    QgsPointCloudBlockHandle *asyncNodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request ) override;

    QgsCoordinateReferenceSystem crs() const override;
    int pointCount() const override;
    QVariant metadataStatistic( const QString &attribute, QgsStatisticalSummary::Statistic statistic ) const override;
    QVariantList metadataClasses( const QString &attribute ) const override;
    QVariant metadataClassStatistic( const QString &attribute, const QVariant &value, QgsStatisticalSummary::Statistic statistic ) const override;
    QVariantMap originalMetadata() const override { return mOriginalMetadata; }

    bool isValid() const override;

    QgsPointCloudIndex::AccessType accessType() const override { return QgsPointCloudIndex::Remote; }

  private:
    bool loadSchema( const QByteArray &data );
    bool loadHierarchy();

    bool mIsValid = false;
    QString mDataType;
    QString mUrlDirectoryPart;
    QString mUrlFileNamePart;
    QString mWkt;

    QUrl mUrl;

    int mPointCount = 0;

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

    QgsTileDownloadManager *mTileDownloadManager = nullptr;
};

///@endcond

#endif // QGSREMOTEEPTPOINTCLOUDINDEX_H
