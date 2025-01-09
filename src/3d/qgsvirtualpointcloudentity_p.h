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

#include "qgscoordinatetransform.h"
#include "qgschunkedentity.h"
#include "qgs3dmapsceneentity.h"
#include "qgs3drendercontext.h"
#include "qgspointcloudlayerchunkloader_p.h"

class QgsAABB;
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
class QgsVirtualPointCloudEntity : public Qgs3DMapSceneEntity
{
    Q_OBJECT
  public:
    //! Constructs
    QgsVirtualPointCloudEntity( Qgs3DMapSettings *map, QgsPointCloudLayer *layer, const QgsCoordinateTransform &coordinateTransform, QgsPointCloud3DSymbol *symbol, float maxScreenError, bool showBoundingBoxes, double zValueScale, double zValueOffset, int pointBudget );

    //! This is called when the camera moves. It's responsible for loading new indexes and decides if subindex will be rendered as bbox or chunked entity.
    void handleSceneUpdate( const SceneContext &sceneContext ) override;

    QgsRange<float> getNearFarPlaneRange( const QMatrix4x4 &viewMatrix ) const override;

    int pendingJobsCount() const override;

    bool needsUpdate() const override;

  public slots:
    //! Creates a child QgsPointCloudLayerChunkedEntity for the \a i th sub index
    void createChunkedEntityForSubIndex( int i );

    //! If \a asBbox is TRUE only the bounding box will be rendered for the sub index \a i. If it is FALSE, the sub index will be rendered as a chunked entity.
    void setRenderSubIndexAsBbox( int i, bool asBbox );

  signals:
    void subIndexNeedsLoading( int i );

  private:
    //! Updates the Bbox child entity to display the sub indexes set with setRenderSubIndexAsBbox()
    void updateBboxEntity();

    //! Returns a list with pointers to all child QgsPointCloudLayerChunkedEntity
    QList<QgsChunkedEntity *> chunkedEntities() const;

    //! Returns a pointer to the associated layer's provider
    QgsVirtualPointCloudProvider *provider() const;

    //! Returns the bounding box for sub index i
    QgsAABB boundingBox( int i ) const;

    QgsPointCloudLayer *mLayer = nullptr;
    QMap<int, QgsChunkedEntity *> mChunkedEntitiesMap;
    QgsChunkBoundsEntity *mBboxesEntity = nullptr;
    QgsPointCloudLayerChunkedEntity *mOverviewEntity = nullptr;
    QList<QgsAABB> mBboxes;
    QgsCoordinateTransform mCoordinateTransform;
    std::unique_ptr<QgsPointCloud3DSymbol> mSymbol;
    double mZValueScale = 1.0;
    double mZValueOffset = 0;
    int mPointBudget = 1000000;
    float mMaximumScreenSpaceError = -1.;
    bool mShowBoundingBoxes = false;
};

/// @endcond

#endif // QGSVIRTUALPOINTCLOUDENTITY_P_H
