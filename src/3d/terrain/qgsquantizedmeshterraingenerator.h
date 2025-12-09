/***************************************************************************
  qgsterraingenerator.h
  --------------------------------------
  Date                 : August 2024
  Copyright            : (C) 2024 by David Koňařík
  Email                : dvdkon at konarici dot cz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#pragma once

#include "qgschunknode.h"
#include "qgscoordinatetransform.h"
#include "qgsquantizedmeshdataprovider.h"
#include "qgsrectangle.h"
#include "qgsterrainentity.h"
#include "qgsterraingenerator.h"
#include "qgstiledsceneindex.h"
#include "qgstiledscenelayer.h"
#include "qgstiles.h"

#include <QPointer>

#define SIP_NO_FILE

/**
 * \ingroup qgis_3d
 * \brief Terrain generator using a Quantized Mesh tile layer
 * \since QGIS 3.40
 */
class _3D_EXPORT QgsQuantizedMeshTerrainGenerator : public QgsTerrainGenerator
{
    Q_OBJECT
  public:
    /**
     * Creates a new instance of a QgsQuantizedMeshTerrainGenerator object.
     */
    static QgsTerrainGenerator *create() SIP_FACTORY;

    QgsQuantizedMeshTerrainGenerator() { mIsValid = false; }

    void setTerrain( QgsTerrainEntity *t ) override;
    QgsTerrainGenerator *clone() const override SIP_FACTORY;
    QgsTerrainGenerator::Type type() const override;
    void setExtent( const QgsRectangle &extent ) override;
    QgsRectangle rootChunkExtent() const override;
    float rootChunkError( const Qgs3DMapSettings &map ) const override;
    void rootChunkHeightRange( float &hMin, float &hMax ) const override;
    float heightAt( double x, double y, const Qgs3DRenderContext &context ) const override;
    QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override;
    // Root node has zoom=0, x=0, y=0.
    // It corresponds to a fake zoom=-1 tile for QgsTileMatrix
    QgsChunkNode *createRootNode() const override;
    QVector<QgsChunkNode *> createChildren( QgsChunkNode *node ) const override;

    /**
     * Set layer to take tiles from
     * \returns true on success
     */
    bool setLayer( QgsTiledSceneLayer *layer );
    //! Returns the layer we take tiles from
    QgsTiledSceneLayer *layer() const;

  private:
    QPointer<QgsTiledSceneLayer> mLayer;
    std::optional<QgsQuantizedMeshMetadata> mMetadata;
    QgsCoordinateTransform mTileCrsToMapCrs;
    QgsTiledSceneIndex mIndex;
    QgsRectangle mMapExtent;
    QgsTileXYZ nodeIdToTile( QgsChunkNodeId nodeId ) const;
};
