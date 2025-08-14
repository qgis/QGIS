/***************************************************************************
  qgsmapoverlaytexturegenerator_p.h
  --------------------------------------
  Date                 : July 2025
  Copyright            : (C) 2025 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPOVERLAYTEXTUREGENERATOR_P_H
#define QGSMAPOVERLAYTEXTUREGENERATOR_P_H

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

#include "qgsrectangle.h"

#include <QObject>

class Qgs3DMapSettings;
class QgsMapRendererSequentialJob;
class QgsMapSettings;
class QgsProject;
class QgsRasterLayer;

/**
 * \ingroup qgis_3d
 * \brief This class renders background map images, which are used as textures in 2D terrain overlays.
 *
 * Textures are asynchronously requested with render() call, when rendering is done the ImageReady()
 * signal is emitted.
 *
 * \since QGIS 4.0
 */
class QgsMapOverlayTextureGenerator : public QObject
{
    Q_OBJECT
  public:
    //! Initializes the object
    QgsMapOverlayTextureGenerator( const Qgs3DMapSettings &mapSettings, int size );

    ~QgsMapOverlayTextureGenerator();

    /**
     * Starts async rendering of a map for the given extent.
     * Returns job ID. The class will emit ImageReady() signal with the job ID when rendering is done.
     */
    int render( const QgsRectangle &extent, double azimuthDegrees );

    //! Cancels active rendering job
    void cancelActiveJob();

    //! Waits for the texture generator to finish
    void waitForFinished();

  signals:
    //! Signal emitted when rendering of a map tile has finished and passes the output image
    void textureReady( const QImage &image );

  private slots:
    void onRenderingFinished();

  private:
    QgsMapSettings baseMapSettings() const;

    const Qgs3DMapSettings &m3DMapSettings;
    QSize mSize;
    QgsMapRendererSequentialJob *mActiveJob = nullptr;
    double mRotation = 0.;
    int mLastJobId;
};

/// @endcond

#endif // QGSMAPOVERLAYTEXTUREGENERATOR_P_H
