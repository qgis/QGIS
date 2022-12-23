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
#include <QSet>

#include "qgspointcloudindex.h"
#include "qgspointcloudattribute.h"
#include "qgsstatisticalsummary.h"
#include "qgis_sip.h"
#include "qgseptpointcloudindex.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsCoordinateReferenceSystem;
class QgsTileDownloadManager;

class CORE_EXPORT QgsRemoteEptPointCloudIndex: public QgsEptPointCloudIndex
{
    Q_OBJECT
  public:

    explicit QgsRemoteEptPointCloudIndex();
    ~QgsRemoteEptPointCloudIndex();

    std::unique_ptr<QgsPointCloudIndex> clone() const override;

    QList<IndexedPointCloudNode> nodeChildren( const IndexedPointCloudNode &n ) const override;

    void load( const QString &fileName ) override;

    QgsPointCloudBlock *nodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request ) override;
    QgsPointCloudBlockRequest *asyncNodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request ) override;

    bool hasNode( const IndexedPointCloudNode &n ) const override;

    bool isValid() const override;

    QgsPointCloudIndex::AccessType accessType() const override { return QgsPointCloudIndex::Remote; }

    /**
     * Copies common properties to the \a destination index
     * \since QGIS 3.26
     */
    void copyCommonProperties( QgsRemoteEptPointCloudIndex *destination ) const;

  private:
    bool loadNodeHierarchy( const IndexedPointCloudNode &nodeId ) const;

    QString mUrlDirectoryPart;
    QString mUrlFileNamePart;

    QUrl mUrl;

    //! Contains the nodes that will have */ept-hierarchy/d-x-y-z.json file
    mutable QSet<IndexedPointCloudNode> mHierarchyNodes;
};

///@endcond

#endif // QGSREMOTEEPTPOINTCLOUDINDEX_H
