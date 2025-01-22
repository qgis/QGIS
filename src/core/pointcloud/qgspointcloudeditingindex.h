/***************************************************************************
    qgspointcloudeditingindex.h
    ---------------------
    begin                : December 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDEDITINGINDEX_H
#define QGSPOINTCLOUDEDITINGINDEX_H

#include "qgspointcloudindex.h"
#include "qgis_core.h"

#define SIP_NO_FILE

class QgsPointCloudLayer;

/**
 * \ingroup core
 *
 * \brief The QgsPointCloudEditingIndex class is a QgsPointCloudIndex that is used as an editing
 * buffer when editing point cloud data.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.42
 */
class CORE_EXPORT QgsPointCloudEditingIndex : public QgsAbstractPointCloudIndex
{
  public:
    //! Ctor
    explicit QgsPointCloudEditingIndex( QgsPointCloudLayer *layer );

    std::unique_ptr<QgsAbstractPointCloudIndex> clone() const override;
    void load( const QString &fileName ) override;
    bool isValid() const override;
    Qgis::PointCloudAccessType accessType() const override;
    QgsCoordinateReferenceSystem crs() const override;
    qint64 pointCount() const override;
    QVariantMap originalMetadata() const override;

    bool hasNode( const QgsPointCloudNodeId &n ) const override;
    QgsPointCloudNode getNode( const QgsPointCloudNodeId &id ) const override;

    std::unique_ptr< QgsPointCloudBlock > nodeData( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request ) override;
    QgsPointCloudBlockRequest *asyncNodeData( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request ) override;

    bool updateNodeData( const QHash<QgsPointCloudNodeId, QByteArray> &data ) override;

    /**
     * Tries to store pending changes to the data provider.
     * If errorMessage is not a null pointer, it will receive
     * an error message in case the call failed.
     * \return TRUE on success, otherwise FALSE
     */
    bool commitChanges( QString *errorMessage = nullptr );

    //! Returns TRUE if there are uncommitted changes, FALSE otherwise
    bool isModified() const;


  private:
    QgsPointCloudIndex mIndex;
    bool mIsValid = false;
    QHash<QgsPointCloudNodeId, QByteArray> mEditedNodeData;

    friend class QgsPointCloudLayerEditUtils;
    friend class QgsPointCloudLayerUndoCommandChangeAttribute;
};

#endif // QGSPOINTCLOUDEDITINGINDEX_H
