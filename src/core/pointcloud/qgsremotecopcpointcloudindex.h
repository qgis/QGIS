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
#include "qgscopcpointcloudindex.h"
#include "qgslazinfo.h"

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

    std::unique_ptr<QgsPointCloudIndex> clone() const override;

    QList<IndexedPointCloudNode> nodeChildren( const IndexedPointCloudNode &n ) const override;

    void load( const QString &url ) override;

    QgsPointCloudBlock *nodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request ) override;
    QgsPointCloudBlockRequest *asyncNodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request ) override;

    bool hasNode( const IndexedPointCloudNode &n ) const override;

    bool isValid() const override;

    QgsPointCloudIndex::AccessType accessType() const override { return QgsPointCloudIndex::Remote; }

    /**
     * Copies common properties to the \a destination index
     * \since QGIS 3.26
     */
    void copyCommonProperties( QgsRemoteCopcPointCloudIndex *destination ) const;

  protected:
    virtual bool fetchNodeHierarchy( const IndexedPointCloudNode &nodeId ) const override;
    virtual void fetchHierarchyPage( uint64_t offset, uint64_t byteSize ) const override;

    QUrl mUrl;

    mutable QSet<IndexedPointCloudNode> mHierarchyNodes;
};

///@endcond

#endif // QGSREMOTECOPCPOINTCLOUDINDEX_H
