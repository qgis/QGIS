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

#include <fstream>

#include "qgspointcloudindex.h"
#include "qgspointcloudstatistics.h"

#include "qgslazinfo.h"
#include "lazperf/vlr.hpp"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsCoordinateReferenceSystem;

class CORE_EXPORT QgsCopcPointCloudIndex: public QgsPointCloudIndex
{
    Q_OBJECT
  public:

    explicit QgsCopcPointCloudIndex();
    ~QgsCopcPointCloudIndex();

    std::unique_ptr<QgsPointCloudIndex> clone() const override;

    void load( const QString &fileName ) override;

    bool hasNode( const IndexedPointCloudNode &n ) const override;
    QList<IndexedPointCloudNode> nodeChildren( const IndexedPointCloudNode &n ) const override;

    std::unique_ptr< QgsPointCloudBlock> nodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request ) override;
    QgsPointCloudBlockRequest *asyncNodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request ) override;

    QgsCoordinateReferenceSystem crs() const override;
    qint64 pointCount() const override;
    bool hasStatisticsMetadata() const override { return false; };
    QVariantMap originalMetadata() const override { return mOriginalMetadata; }

    bool isValid() const override;
    QgsPointCloudIndex::AccessType accessType() const override { return mAccessType; };

    /**
     * Writes the statistics object \a stats into the COPC dataset as an Extended Variable Length Record (EVLR).
     * Returns true if the data was written successfully.
     * \since QGIS 3.26
     */
    bool writeStatistics( QgsPointCloudStatistics &stats );

    /**
     * Returns the statistics object contained in the COPC dataset.
     * If the dataset doesn't contain statistics EVLR, an object with 0 samples will be returned.
     * \since QGIS 3.26
     */
    QgsPointCloudStatistics readStatistics();

    /**
     * Copies common properties to the \a destination index
     * \since QGIS 3.26
     */
    void copyCommonProperties( QgsCopcPointCloudIndex *destination ) const;

    /**
     * Returns the gps time flag from LAS header
     * \since QGIS 3.34.12
     */
    bool gpsTimeFlag() const;

  protected:
    bool loadSchema( QgsLazInfo &lazInfo );
    bool loadHierarchy();

    //! Fetches all nodes leading to node \a node into memory
    bool fetchNodeHierarchy( const IndexedPointCloudNode &n ) const;

    /**
     * Fetches the COPC hierarchy page at offset \a offset and of size \a byteSize into memory
     */
    virtual void fetchHierarchyPage( uint64_t offset, uint64_t byteSize ) const;

    void populateHierarchy( const char *hierarchyPageData, uint64_t byteSize ) const;

    QByteArray fetchCopcStatisticsEvlrData();

    bool mIsValid = false;
    QgsPointCloudIndex::AccessType mAccessType = Local;
    mutable std::ifstream mCopcFile;
    mutable lazperf::copc_info_vlr mCopcInfoVlr;
    mutable QHash<IndexedPointCloudNode, QPair<uint64_t, int32_t>> mHierarchyNodePos; //!< Additional data hierarchy for COPC

    QVariantMap mOriginalMetadata;

    std::unique_ptr<QgsLazInfo> mLazInfo = nullptr;
};

///@endcond
#endif // QGSCOPCPOINTCLOUDINDEX_H
