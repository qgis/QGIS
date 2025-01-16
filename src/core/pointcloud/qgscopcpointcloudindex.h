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

#include <QString>
#include <QHash>
#include <QStringList>
#include <QVector>
#include <QList>
#include <QFile>

#include <fstream>
#include <optional>

#include "qgspointcloudindex.h"
#include "qgspointcloudstatistics.h"

#include "qgslazinfo.h"
#include "lazperf/vlr.hpp"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsCoordinateReferenceSystem;

class CORE_EXPORT QgsCopcPointCloudIndex: public QgsAbstractPointCloudIndex
{
  public:

    explicit QgsCopcPointCloudIndex();
    ~QgsCopcPointCloudIndex();

    std::unique_ptr<QgsAbstractPointCloudIndex> clone() const override;

    void load( const QString &fileName ) override;

    bool hasNode( const QgsPointCloudNodeId &n ) const override;
    QgsPointCloudNode getNode( const QgsPointCloudNodeId &id ) const override;

    std::unique_ptr< QgsPointCloudBlock> nodeData( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request ) override;
    QgsPointCloudBlockRequest *asyncNodeData( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request ) override;

    QgsCoordinateReferenceSystem crs() const override;
    qint64 pointCount() const override;
    QVariantMap originalMetadata() const override { return mOriginalMetadata; }

    bool isValid() const override;
    Qgis::PointCloudAccessType accessType() const override { return mAccessType; };

    /**
     * Writes the statistics object \a stats into the COPC dataset as an Extended Variable Length Record (EVLR).
     * Returns true if the data was written successfully.
     * \since QGIS 3.26
     */
    bool writeStatistics( QgsPointCloudStatistics &stats ) override;

    /**
     * Returns the statistics object contained in the COPC dataset.
     * If the dataset doesn't contain statistics EVLR, an object with 0 samples will be returned.
     * \since QGIS 3.42
     */
    QgsPointCloudStatistics metadataStatistics() const override;

    /**
     * Copies common properties to the \a destination index
     * \since QGIS 3.26
     */
    void copyCommonProperties( QgsCopcPointCloudIndex *destination ) const;

    /**
     * Returns one datapoint, "CopcGpsTimeFlag": The gps time flag from global_encoding field in LAS header,
     * 0 indicates GPS week time (seconds passed since the beginning of the week)
     * 1 indicates GPS adjusted time, which is seconds passed since the GPS base time minus 1e9
     */
    QVariantMap extraMetadata() const override;

  protected:
    bool loadSchema( QgsLazInfo &lazInfo );
    bool loadHierarchy() const;

    //! Fetches all nodes leading to node \a node into memory
    bool fetchNodeHierarchy( const QgsPointCloudNodeId &n ) const;

    /**
     * Fetches the COPC hierarchy page at offset \a offset and of size \a byteSize into memory
     */
    virtual void fetchHierarchyPage( uint64_t offset, uint64_t byteSize ) const;

    void populateHierarchy( const char *hierarchyPageData, uint64_t byteSize ) const;

    //! Utility function for reading sub-range of mUri
    QByteArray readRange( uint64_t offset, uint64_t length ) const;

    QByteArray fetchCopcStatisticsEvlrData() const;

    void reset();

    bool mIsValid = false;
    Qgis::PointCloudAccessType mAccessType = Qgis::PointCloudAccessType::Local;
    mutable std::ifstream mCopcFile;
    mutable lazperf::copc_info_vlr mCopcInfoVlr;
    mutable QHash<QgsPointCloudNodeId, QPair<uint64_t, int32_t>> mHierarchyNodePos; //!< Additional data hierarchy for COPC

    QVariantMap mOriginalMetadata;
    mutable std::optional<QgsPointCloudStatistics> mStatistics;

    std::unique_ptr<QgsLazInfo> mLazInfo = nullptr;

    friend class QgsPointCloudLayerEditUtils;
    friend class QgsPointCloudEditingIndex;
    friend class QgsPointCloudLayerUndoCommandChangeAttribute;
};

///@endcond
#endif // QGSCOPCPOINTCLOUDINDEX_H
