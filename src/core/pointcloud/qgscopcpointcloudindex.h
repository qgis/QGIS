/***************************************************************************
                         qgscopcpointcloudindex.h
                         --------------------
    begin                : March 2022
    copyright            : (C) 2022 by Belgacem Nedjima
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

#ifndef QGSCOPCPOINTCLOUDINDEX_H
#define QGSCOPCPOINTCLOUDINDEX_H

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

class CORE_EXPORT QgsCopcPointCloudIndex: public QgsPointCloudIndex
{
    Q_OBJECT
  public:

    explicit QgsCopcPointCloudIndex();
    ~QgsCopcPointCloudIndex();

    void load( const QString &fileName ) override;

    QgsPointCloudBlock *nodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request ) override;
    QgsPointCloudBlockRequest *asyncNodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request ) override;

    QgsCoordinateReferenceSystem crs() const override;
    qint64 pointCount() const override;
    QVariantMap originalMetadata() const override { return mOriginalMetadata; }

    bool isValid() const override;
    QgsPointCloudIndex::AccessType accessType() const override { return QgsPointCloudIndex::Local; };

  protected:
    bool loadSchema( const QString &filename );
    bool loadHierarchy( const QString &filename );

    bool mIsValid = false;
    QString mDataType;
    QString mFileName;
    QString mWkt;

    qint64 mPointCount = 0;

    mutable QHash<IndexedPointCloudNode, uint64_t> mHierarchyNodeOffset; //!< Additional data hierarchy for COPC
    mutable QHash<IndexedPointCloudNode, int32_t> mHierarchyNodeByteSize; //!< Additional data hierarchy for COPC

    struct AttributeStatistics
    {
      int count = -1;
      QVariant minimum;
      QVariant maximum;
      double mean = std::numeric_limits< double >::quiet_NaN();
      double stDev = std::numeric_limits< double >::quiet_NaN();
      double variance = std::numeric_limits< double >::quiet_NaN();
    };

    QVariantMap mOriginalMetadata;
};

///@endcond
#endif // QGSCOPCPOINTCLOUDINDEX_H
