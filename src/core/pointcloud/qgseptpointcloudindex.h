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

#include <QString>
#include <QHash>
#include <QStringList>
#include <QVector>
#include <QList>
#include <QFile>

#include "qgspointcloudindex.h"
#include "qgis_sip.h"
#include "qgsvector3d.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsCoordinateReferenceSystem;

class CORE_EXPORT QgsEptPointCloudIndex: public QgsAbstractPointCloudIndex
{
  public:

    explicit QgsEptPointCloudIndex();
    ~QgsEptPointCloudIndex();

    std::unique_ptr<QgsAbstractPointCloudIndex> clone() const override;

    void load( const QString &fileName ) override;

    std::unique_ptr<QgsPointCloudBlock> nodeData( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request ) override;
    QgsPointCloudBlockRequest *asyncNodeData( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request ) override;
    bool hasNode( const QgsPointCloudNodeId &n ) const override;

    QgsCoordinateReferenceSystem crs() const override;
    qint64 pointCount() const override;
    QgsPointCloudNode getNode( const QgsPointCloudNodeId &id ) const override;
    QVariantMap originalMetadata() const override { return mOriginalMetadata; }
    QgsPointCloudStatistics metadataStatistics() const override;

    bool isValid() const override;
    Qgis::PointCloudAccessType accessType() const override;

    /**
     * Copies common properties to the \a destination index
     * \since QGIS 3.26
     */
    void copyCommonProperties( QgsEptPointCloudIndex *destination ) const;

  protected:
    bool loadSchema( const QByteArray &dataJson );
    void loadManifest( const QByteArray &manifestJson );
    bool loadSchema( QFile &f );
    bool loadSingleNodeHierarchy( const QgsPointCloudNodeId &nodeId ) const;
    QVector<QgsPointCloudNodeId> nodePathToRoot( const QgsPointCloudNodeId &nodeId ) const;
    bool loadNodeHierarchy( const QgsPointCloudNodeId &nodeId ) const;

    bool mIsValid = false;
    Qgis::PointCloudAccessType mAccessType = Qgis::PointCloudAccessType::Local;
    QString mDataType;
    QString mWkt;

    QString mUrlDirectoryPart;

    //! Contains the nodes that will have */ept-hierarchy/d-x-y-z.json file
    mutable QSet<QgsPointCloudNodeId> mHierarchyNodes;

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
