/***************************************************************************
                         qgspointcloudindex.h
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

#ifndef QGSEPTPOINTCLOUDINDEX_H
#define QGSEPTPOINTCLOUDINDEX_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QStringList>
#include <QVector>
#include <QList>
#include <QFile>

#include "qgspointcloudindex.h"
#include "qgspointcloudattribute.h"
#include "qgsstatisticalsummary.h"
#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsCoordinateReferenceSystem;

class CORE_EXPORT QgsEptPointCloudIndex: public QgsPointCloudIndex
{
    Q_OBJECT
  public:

    explicit QgsEptPointCloudIndex();
    ~QgsEptPointCloudIndex();

    std::unique_ptr<QgsPointCloudIndex> clone() const override;

    void load( const QString &fileName ) override;

    QgsPointCloudBlock *nodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request ) override;
    QgsPointCloudBlockRequest *asyncNodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request ) override;

    QgsCoordinateReferenceSystem crs() const override;
    qint64 pointCount() const override;
    bool hasStatisticsMetadata() const override;
    QVariant metadataStatistic( const QString &attribute, QgsStatisticalSummary::Statistic statistic ) const override;
    QVariantList metadataClasses( const QString &attribute ) const override;
    QVariant metadataClassStatistic( const QString &attribute, const QVariant &value, QgsStatisticalSummary::Statistic statistic ) const override;
    QVariantMap originalMetadata() const override { return mOriginalMetadata; }

    bool isValid() const override;
    QgsPointCloudIndex::AccessType accessType() const override { return QgsPointCloudIndex::Local; };

    /**
     * Copies common properties to the \a destination index
     * \since QGIS 3.26
     */
    void copyCommonProperties( QgsEptPointCloudIndex *destination ) const;

  protected:
    bool loadSchema( const QByteArray &dataJson );
    void loadManifest( const QByteArray &manifestJson );
    bool loadSchema( QFile &f );
    bool loadHierarchy();

    bool mIsValid = false;
    QString mDataType;
    QString mDirectory;
    QString mWkt;

    qint64 mPointCount = 0;

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

///@endcond
#endif // QGSEPTPOINTCLOUDINDEX_H
