/***************************************************************************
                         qgsremotecopcpointcloudindex.h
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

#ifndef QGSREMOTECOPCPOINTCLOUDINDEX_H
#define QGSREMOTECOPCPOINTCLOUDINDEX_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QStringList>
#include <QVector>
#include <QList>
#include <QFile>
#include <QUrl>
#include <QSet>

#include <sstream>

#include "qgspointcloudindex.h"
#include "qgspointcloudattribute.h"
#include "qgsstatisticalsummary.h"
#include "qgis_sip.h"
#include "qgscopcpointcloudindex.h"

#include "lazperf/header.hpp"
#include "lazperf/vlr.hpp"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsCoordinateReferenceSystem;
class QgsTileDownloadManager;
class QgsCopcPointCloudBlockRequest;

class CORE_EXPORT QgsRemoteCopcPointCloudIndex: public QgsCopcPointCloudIndex
{
    Q_OBJECT
  public:

    explicit QgsRemoteCopcPointCloudIndex();
    ~QgsRemoteCopcPointCloudIndex();

    QList<IndexedPointCloudNode> nodeChildren( const IndexedPointCloudNode &n ) const override;

    void load( const QString &fileName ) override;

    QgsPointCloudBlock *nodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request ) override;
    QgsPointCloudBlockRequest *asyncNodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request ) override;

    bool hasNode( const IndexedPointCloudNode &n ) const override;

    bool isValid() const override;

    QgsPointCloudIndex::AccessType accessType() const override { return QgsPointCloudIndex::Remote; }

    friend QgsCopcPointCloudBlockRequest;
  private:
    bool loadHeader();

    bool fetchNodeHierarchy( const IndexedPointCloudNode &nodeId ) const;

    /**
     * Fetches the COPC hierarchy page at offset \a offset and of size \a byteSize into memory
     * \note: This function is NOT thread safe and the mutex mHierarchyMutex needs to be locked before entering
     */
    void fetchHierarchyPage( uint64_t offset, uint64_t byteSize ) const;


    QString mUrlDirectoryPart;
    QString mUrlFileNamePart;

    QUrl mUrl;
    QByteArray mCopcHeaderData;
    QByteArray mVlrData;
    QByteArray mExtraBytesData;
    lazperf::wkt_vlr mWktVlr;
    lazperf::eb_vlr mExtraBytesVlr;
    mutable lazperf::header14 mCopcHeader;
    mutable lazperf::copc_info_vlr mCopcInfoVlr;

    mutable QSet<IndexedPointCloudNode> mHierarchyNodes;
};

///@endcond

#endif // QGSREMOTECOPCPOINTCLOUDINDEX_H
