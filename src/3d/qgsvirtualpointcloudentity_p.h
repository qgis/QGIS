/***************************************************************************
  qgsvirtualpointcloudentity_p.h
  --------------------------------------
  Date                 : April 2023
  Copyright            : (C) 2023 by Stefanos Natsis
  Email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVIRTUALPOINTCLOUDENTITY_P_H
#define QGSVIRTUALPOINTCLOUDENTITY_P_H

#define SIP_NO_FILE
///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <Qt3DCore/QEntity>

#include "qgscoordinatetransform.h"
#include "qgspointcloudsubindex.h"

class QgsAABB;
class QgsChunkedEntity;
class QgsChunkBoundsEntity;
class QgsPointCloudLayer;
class QgsVirtualPointCloudProvider;
class QgsPointCloud3DSymbol;
class Qgs3DMapSettings;

/**
 * \ingroup 3d
 * \brief Implementation of entity that handles virtual point cloud sub indexes
 *
 * This entity is parent to a QgsChunkBoundsEntity that renders all sub indexes' bounding boxes, as well as all individual
 * QgsPointCloudLayerChunkedEntities.
 *
 *
 * \since QGIS 3.32
 */
class QgsVirtualPointCloudEntity : public Qt3DCore::QEntity
{
    Q_OBJECT
  public:
    //! Constructs
    QgsVirtualPointCloudEntity( QgsPointCloudLayer *layer, const Qgs3DMapSettings &map, const QgsCoordinateTransform &coordinateTransform, QgsPointCloud3DSymbol *symbol, float maxScreenError, bool showBoundingBoxes,
                                double zValueScale, double zValueOffset, int pointBudget );

    //! creates a child QgsPointCloudLayerChunkedEntity for each of the already loaded sub indexes
    void createChunkedEntitiesForLoadedSubIndexes();

    //! Updates the Bbox child entity to display the sub indexes set with setRenderSubIndexAsBbox()
    void updateBboxEntity();

    //! Returns a list with pointers to all child QgsPointCloudLayerChunkedEntity
    QList<QgsChunkedEntity *> chunkedEntities() const;

    //! Returns a pointer to the associated layer's provider
    QgsVirtualPointCloudProvider *provider() const;

    //! Returns the bounding box for sub index \a i
    QgsAABB boundingBox( int i ) const;

  signals:
    //! Emitted when a new point cloud chunked entity has been created for a sub index
    void newEntityCreated( QgsChunkedEntity *entity );

  public slots:
    //! Creates a child QgsPointCloudLayerChunkedEntity for the \a i th sub index
    void createChunkedEntityForSubIndex( int i );

    //! If \a asBbox is TRUE only the bounding box will be rendered for the sub index \a i. If it is FALSE, the sub index will be rendered as a chunked entity.
    void setRenderSubIndexAsBbox( const int i, const bool asBbox );

  private:
    QgsPointCloudLayer *mLayer = nullptr;
    QList<QgsChunkedEntity *> mChunkedEntities;
    QMap<int, QgsChunkedEntity *> mChunkedEntitiesMap;
    QgsChunkBoundsEntity *mBboxesEntity = nullptr;
    QList<QgsAABB> mBboxes;
    const Qgs3DMapSettings &mMap;
    QgsCoordinateTransform mCoordinateTransform;
    QgsPointCloudIndex *mPointCloudIndex;
    std::unique_ptr< QgsPointCloud3DSymbol > mSymbol;
    double mZValueScale = 1.0;
    double mZValueOffset = 0;
    int mPointBudget = 1000000;
    float mMaximumScreenSpaceError = -1.;
    bool mShowBoundingBoxes = false;
};

/// @endcond

#endif // QGSVIRTUALPOINTCLOUDENTITY_P_H
