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


#include "qgschunkedentity.h"
#include "qgschunkloader.h"
#include "qgscoordinatetransform.h"
#include "qgsdistancearea.h"
#include "qgs3drendercontext.h"
#include "qgslayerstylewatcher.h"
#include "qobjectuniqueptr.h"

#include <QImage>

class QgsMapLayer;
class QgsTerrainTextureGenerator;
class QgsLayerStyleWatcher;


class QgsGlobeChunkLoader : public QgsChunkLoader
{
    Q_OBJECT
  public:
    QgsGlobeChunkLoader( Qgs3DMapSettings *mapSettings );

    ~QgsGlobeChunkLoader() override;

    QFuture<QgsChunkLoaderResult> loadChunk( QgsChunkNode *node ) override;
    QFuture<QgsChunkLoaderResult> updateChunk( QgsChunkNode *node ) override;

    QgsChunkNode *createRootNode() const override;

    QFuture<QVector<QgsChunkNode *>> createChildren( QgsChunkNode *node ) override;

  private:
    Qgs3DMapSettings *mMapSettings = nullptr;
    std::unique_ptr<QgsTerrainTextureGenerator> mTextureGenerator;
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
    ~QgsGlobeEntity() override;

    QList<QgsRayCastHit> rayIntersection( const QgsRay3D &ray, const QgsRayCastContext &context ) const override;

  private slots:
    void invalidateMapImages();

  private:
    QObjectUniquePtr<QgsLayerStyleWatcher> mLayerWatcher = nullptr;
};


/// @endcond

#endif // QGSGLOBECHUNKEDENTITY_H
