/***************************************************************************
  qgsterraintexturegenerator_p.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTERRAINTEXTUREGENERATOR_P_H
#define QGSTERRAINTEXTUREGENERATOR_P_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//


class QgsMapRendererSequentialJob;
class QgsMapSettings;
class QgsProject;
class QgsRasterLayer;

#include "qgschunknode.h"
#include "qgsrectangle.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QImage>
#include <QObject>
#include <QSize>

#define SIP_NO_FILE

class Qgs3DMapSettings;

/**
 * \ingroup qgis_3d
 * \brief Class responsible for rendering map images in background for the purposes of their use
 * as textures for terrain's tiles.
 *
 * Tiles are asynchronously requested with render() call, when rendering is done the tileReady()
 * signal will be emitted. Handles multiple rendering requests at a time - each request gets
 * a unique job ID assigned.
 *
 */
class QgsTerrainTextureGenerator : public QObject
{
    Q_OBJECT
  public:
    //! Initializes the object
    QgsTerrainTextureGenerator( const Qgs3DMapSettings &map );

    /**
     * Starts async rendering of a map for the given extent (must be a square!).
     */
    QFuture<QImage> render( const QgsRectangle &extent, QgsChunkNodeId nodeId, const QString &debugText = QString() );

    //! Waits for the texture generator to finish
    void waitForFinished();

    //! Returns the generated texture size (in pixel)
    QSize textureSize() const { return mTextureSize; }
    //! Sets the generated textures size (in pixel)
    void setTextureSize( QSize textureSize ) { mTextureSize = textureSize; }

  private slots:
    void onRenderingFinished();

  private:
    QgsMapSettings baseMapSettings();

    const Qgs3DMapSettings &mMap;

    struct JobData
    {
        QgsChunkNodeId tileId;
        QgsMapRendererSequentialJob *job = nullptr;
        QgsRectangle extent;
        QString debugText;
        QPromise<QImage> promise;
        QFutureWatcher<QImage> watcher;
    };

    std::unordered_map<QgsMapRendererSequentialJob *, std::unique_ptr<JobData>> mJobs;
    QSize mTextureSize;
};

/// @endcond

#endif // QGSTERRAINTEXTUREGENERATOR_P_H
