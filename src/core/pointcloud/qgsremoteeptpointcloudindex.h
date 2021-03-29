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

    QList<IndexedPointCloudNode> nodeChildren( const IndexedPointCloudNode &n ) const override;

    void load( const QString &fileName ) override;

    QgsPointCloudBlock *nodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request ) override;
    QgsPointCloudBlockHandle *asyncNodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request ) override;

    int pointCount() const override;

    bool isValid() const override;

    QgsPointCloudIndex::AccessType accessType() const override { return QgsPointCloudIndex::Remote; }

  private:
    bool loadSchema( const QByteArray &data );

    bool loadNodeHierarchy( const IndexedPointCloudNode &nodeId ) const;

    bool mIsValid = false;
    QString mDataType;
    QString mUrlDirectoryPart;
    QString mUrlFileNamePart;

    QUrl mUrl;

    int mPointCount = 0;

    QgsTileDownloadManager *mTileDownloadManager = nullptr;
};

///@endcond

#endif // QGSREMOTEEPTPOINTCLOUDINDEX_H
