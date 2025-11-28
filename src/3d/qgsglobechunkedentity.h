/***************************************************************************
  qgsglobechunkedentity.h
  --------------------------------------
  Date                 : March 2025
  Copyright            : (C) 2025 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGLOBECHUNKEDENTITY_H
#define QGSGLOBECHUNKEDENTITY_H

#include "qgis_3d.h"

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#define SIP_NO_FILE


#include "qgschunkedentity.h"

#include "qgschunkloader.h"
#include "qgscoordinatetransform.h"
#include "qgsdistancearea.h"
#include <QImage>

class QgsMapLayer;
class QgsGlobeMapUpdateJobFactory;
class QgsTerrainTextureGenerator;

class QgsGlobeChunkLoader : public QgsChunkLoader
{
    Q_OBJECT
  public:
    QgsGlobeChunkLoader( QgsChunkNode *node, QgsTerrainTextureGenerator *textureGenerator, const QgsCoordinateTransform &globeCrsToLatLon );
    void start() override;

    Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) override;

  private:
    QgsTerrainTextureGenerator *mTextureGenerator;
    QgsCoordinateTransform mGlobeCrsToLatLon;
    int mJobId = -1;
    QImage mTexture;
};


//! Handles asynchronous updates of globe's map images when layers change
class QgsGlobeMapUpdateJob : public QgsChunkQueueJob
{
    Q_OBJECT
  public:
    QgsGlobeMapUpdateJob( QgsTerrainTextureGenerator *textureGenerator, QgsChunkNode *node );
    void start() override;

    void cancel() override;

  private:
    QgsTerrainTextureGenerator *mTextureGenerator = nullptr;
    int mJobId = -1;
};

class QgsGlobeChunkLoaderFactory : public QgsChunkLoaderFactory
{
    Q_OBJECT
  public:
    QgsGlobeChunkLoaderFactory( Qgs3DMapSettings *mapSettings );

    ~QgsGlobeChunkLoaderFactory();

    QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override;

    QgsChunkNode *createRootNode() const override;

    QVector<QgsChunkNode *> createChildren( QgsChunkNode *node ) const override;

  private:
    Qgs3DMapSettings *mMapSettings = nullptr;
    QgsTerrainTextureGenerator *mTextureGenerator = nullptr; // owned by the factory
    QgsDistanceArea mDistanceArea;
    QgsCoordinateTransform mGlobeCrsToLatLon;
    double mRadiusX, mRadiusY, mRadiusZ;
};

/**
 * 3D chunked entity implementation to generate globe mesh with constant elevation
 *
 * \since QGIS 3.44
 */
class _3D_EXPORT QgsGlobeEntity : public QgsChunkedEntity
{
    Q_OBJECT

  public:
    QgsGlobeEntity( Qgs3DMapSettings *mapSettings );
    ~QgsGlobeEntity();

    QList<QgsRayCastHit> rayIntersection( const QgsRay3D &ray, const QgsRayCastContext &context ) const override;

  private slots:
    void invalidateMapImages();
    void onLayersChanged();

  private:
    void connectToLayersRepaintRequest();

  private:
    std::unique_ptr<QgsGlobeMapUpdateJobFactory> mUpdateJobFactory;

    //! layers that are currently being used for map rendering (and thus being watched for renderer updates)
    QList<QgsMapLayer *> mLayers;
};


/// @endcond

#endif // QGSGLOBECHUNKEDENTITY_H
