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

#include <QObject>

#include "qgsrectangle.h"

class Qgs3DMapSettings;

/** \ingroup 3d
 * Class responsible for rendering map images in background for the purposes of their use
 * as textures for terrain's tiles.
 *
 * Tiles are asynchronously requested with render() call, when rendering is done the tileReady()
 * signal will be emitted. Handles multiple rendering requests at a time - each request gets
 * a unique job ID assigned.
 *
 * \since QGIS 3.0
 */
class QgsTerrainTextureGenerator : public QObject
{
    Q_OBJECT
  public:
    //! Initializes the object
    QgsTerrainTextureGenerator( const Qgs3DMapSettings &map );

    //! Starts async rendering of a map for the given extent (must be a square!).
    //! Returns job ID. The class will emit tileReady() signal with the job ID when rendering is done.
    int render( const QgsRectangle &extent, const QString &debugText = QString() );

    //! Cancels a rendering job
    void cancelJob( int jobId );

    //! Renders a map and returns rendered image. Blocks until the map rendering has finished
    QImage renderSynchronously( const QgsRectangle &extent, const QString &debugText = QString() );

  signals:
    //! Signal emitted when rendering of a map tile has finished and passes the output image
    void tileReady( int jobId, const QImage &image );

  private slots:
    void onRenderingFinished();

  private:
    QgsMapSettings baseMapSettings();

    const Qgs3DMapSettings &mMap;

    struct JobData
    {
      int jobId;
      QgsMapRendererSequentialJob *job;
      QgsRectangle extent;
      QString debugText;
    };

    QHash<QgsMapRendererSequentialJob *, JobData> mJobs;
    int mLastJobId;
};

/// @endcond

#endif // QGSTERRAINTEXTUREGENERATOR_P_H
