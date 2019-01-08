/***************************************************************************
  qgsmaprendererjob.h
  --------------------------------------
  Date                 : December 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPRENDERERJOB_H
#define QGSMAPRENDERERJOB_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include <QFutureWatcher>
#include <QImage>
#include <QPainter>
#include <QObject>
#include <QTime>

#include "qgsrendercontext.h"

#include "qgsmapsettings.h"


class QgsLabelingEngine;
class QgsLabelingResults;
class QgsMapLayerRenderer;
class QgsMapRendererCache;
class QgsFeatureFilterProvider;

#ifndef SIP_RUN
/// @cond PRIVATE

/**
 * \ingroup core
 * Structure keeping low-level rendering job information.
 */
struct LayerRenderJob
{
  QgsRenderContext context;
  QImage *img; // may be null if it is not necessary to draw to separate image (e.g. sequential rendering)
  //! True when img has been initialized (filled with transparent pixels) and is safe to compose
  bool imageInitialized = false;
  QgsMapLayerRenderer *renderer; // must be deleted
  QPainter::CompositionMode blendMode;
  double opacity;
  bool cached; // if true, img already contains cached image from previous rendering
  QgsWeakMapLayerPointer layer;
  int renderingTime; //!< Time it took to render the layer in ms (it is -1 if not rendered or still rendering)
};

typedef QList<LayerRenderJob> LayerRenderJobs;

/**
 * \ingroup core
 * Structure keeping low-level label rendering job information.
 */
struct LabelRenderJob
{
  QgsRenderContext context;

  /**
   * May be null if it is not necessary to draw to separate image (e.g. using composition modes which prevent "flattening" the layer).
   * Note that if complete is false then img will be uninitialized and contain random data!.
   */
  QImage *img = nullptr;
  //! If true, img already contains cached image from previous rendering
  bool cached = false;
  //! Will be true if labeling is eligible for caching
  bool canUseCache = false;
  //! If true then label render is complete
  bool complete = false;
  //! Time it took to render the labels in ms (it is -1 if not rendered or still rendering)
  int renderingTime = -1;
  //! List of layers which participated in the labeling solution
  QList< QPointer< QgsMapLayer > > participatingLayers;
};

///@endcond PRIVATE
#endif

/**
 * \ingroup core
 * Abstract base class for map rendering implementations.
 *
 * The API is designed in a way that rendering is done asynchronously, therefore
 * the caller is not blocked while the rendering is in progress. Non-blocking
 * operation is quite important because the rendering can take considerable
 * amount of time.
 *
 * Common use case:
 * 0. prepare QgsMapSettings with rendering configuration (extent, layer, map size, ...)
 * 1. create QgsMapRendererJob subclass with QgsMapSettings instance
 * 2. connect to job's finished() signal
 * 3. call start(). Map rendering will start in background, the function immediately returns
 * 4. at some point, slot connected to finished() signal is called, map rendering is done
 *
 * It is possible to cancel the rendering job while it is active by calling cancel() function.
 *
 * The following subclasses are available:
 * - QgsMapRendererSequentialJob - renders map in one background thread to an image
 * - QgsMapRendererParallelJob - renders map in multiple background threads to an image
 * - QgsMapRendererCustomPainterJob - renders map with given QPainter in one background thread
 *
 * \since QGIS 2.4
 */
class CORE_EXPORT QgsMapRendererJob : public QObject
{
    Q_OBJECT
  public:

    QgsMapRendererJob( const QgsMapSettings &settings );

    /**
     * Start the rendering job and immediately return.
     * Does nothing if the rendering is already in progress.
     */
    virtual void start() = 0;

    /**
     * Stop the rendering job - does not return until the job has terminated.
     * Does nothing if the rendering is not active.
     */
    virtual void cancel() = 0;

    /**
     * Triggers cancelation of the rendering job without blocking. The render job will continue
     * to operate until it is able to cancel, at which stage the finished() signal will be emitted.
     * Does nothing if the rendering is not active.
     */
    virtual void cancelWithoutBlocking() = 0;

    //! Block until the job has finished.
    virtual void waitForFinished() = 0;

    //! Tell whether the rendering job is currently running in background.
    virtual bool isActive() const = 0;

    /**
     * Returns true if the render job was able to use a cached labeling solution.
     * If so, any previously stored labeling results (see takeLabelingResults())
     * should be retained.
     * \see takeLabelingResults()
     * \since QGIS 3.0
     */
    virtual bool usedCachedLabels() const = 0;

    /**
     * Gets pointer to internal labeling engine (in order to get access to the results).
     * This should not be used if cached labeling was redrawn - see usedCachedLabels().
     * \see usedCachedLabels()
     */
    virtual QgsLabelingResults *takeLabelingResults() = 0 SIP_TRANSFER;

    /**
     * Set the feature filter provider used by the QgsRenderContext of
     * each LayerRenderJob.
     * Ownership is not transferred and the provider must not be deleted
     * before the render job.
     * \since QGIS 3.0
     */
    void setFeatureFilterProvider( const QgsFeatureFilterProvider *f ) { mFeatureFilterProvider = f; }

    /**
     * Returns the feature filter provider used by the QgsRenderContext of
     * each LayerRenderJob.
     * \since QGIS 3.0
     */
    const QgsFeatureFilterProvider *featureFilterProvider() const { return mFeatureFilterProvider; }

    struct Error
    {
      Error( const QString &lid, const QString &msg )
        : layerID( lid )
        , message( msg )
      {}

      QString layerID;
      QString message;
    };

    typedef QList<QgsMapRendererJob::Error> Errors;

    //! List of errors that happened during the rendering job - available when the rendering has been finished
    Errors errors() const;


    /**
     * Assign a cache to be used for reading and storing rendered images of individual layers.
     * Does not take ownership of the object.
     */
    void setCache( QgsMapRendererCache *cache );

    /**
     * Returns the total time it took to finish the job (in milliseconds).
     * \see perLayerRenderingTime()
     */
    int renderingTime() const { return mRenderingTime; }

    /**
     * Returns the render time (in ms) per layer.
     * \note Not available in Python bindings.
     * \since QGIS 3.0
     */
    QHash< QgsMapLayer *, int > perLayerRenderingTime() const SIP_SKIP;

    /**
     * Returns map settings with which this job was started.
     * \returns A QgsMapSettings instance with render settings
     * \since QGIS 2.8
     */
    const QgsMapSettings &mapSettings() const;

    /**
     * QgsMapRendererCache ID string for cached label image.
     * \note not available in Python bindings
     */
    static const QString LABEL_CACHE_ID SIP_SKIP;

  signals:

    /**
     * Emitted when the layers are rendered.
     * Rendering labels is not yet done. If the fully rendered layer including labels is required use
     * finished() instead.
     *
     * \since QGIS 3.0
     */
    void renderingLayersFinished();

    //! emitted when asynchronous rendering is finished (or canceled).
    void finished();

  protected:

    QgsMapSettings mSettings;
    QTime mRenderingStart;
    Errors mErrors;

    QgsMapRendererCache *mCache = nullptr;

    int mRenderingTime = 0;

    //! Render time (in ms) per layer, by layer ID
    QHash< QgsWeakMapLayerPointer, int > mPerLayerRenderingTime;

    /**
     * Prepares the cache for storing the result of labeling. Returns false if
     * the render cannot use cached labels and should not cache the result.
     * \note not available in Python bindings
     */
    bool prepareLabelCache() const SIP_SKIP;

    //! \note not available in Python bindings
    LayerRenderJobs prepareJobs( QPainter *painter, QgsLabelingEngine *labelingEngine2 ) SIP_SKIP;

    /**
     * Prepares a labeling job.
     * \note not available in Python bindings
     * \since QGIS 3.0
     */
    LabelRenderJob prepareLabelingJob( QPainter *painter, QgsLabelingEngine *labelingEngine2, bool canUseLabelCache = true ) SIP_SKIP;

    //! \note not available in Python bindings
    static QImage composeImage( const QgsMapSettings &settings, const LayerRenderJobs &jobs, const LabelRenderJob &labelJob ) SIP_SKIP;

    //! \note not available in Python bindings
    void logRenderingTime( const LayerRenderJobs &jobs, const LabelRenderJob &labelJob ) SIP_SKIP;

    //! \note not available in Python bindings
    void cleanupJobs( LayerRenderJobs &jobs ) SIP_SKIP;

    /**
     * Handles clean up tasks for a label job, including deletion of images and storing cached
     * label results.
     * \note not available in Python bindings
     * \since QGIS 3.0
     */
    void cleanupLabelJob( LabelRenderJob &job ) SIP_SKIP;

    /**
     * \note not available in Python bindings
     * \deprecated Will be removed in QGIS 4.0
     */
    Q_DECL_DEPRECATED static void drawLabeling( const QgsMapSettings &settings, QgsRenderContext &renderContext, QgsLabelingEngine *labelingEngine2, QPainter *painter ) SIP_SKIP;

    //! \note not available in Python bindings
    static void drawLabeling( QgsRenderContext &renderContext, QgsLabelingEngine *labelingEngine2, QPainter *painter ) SIP_SKIP;

  private:

    /**
     * Convenience function to project an extent into the layer source
     * CRS, but also split it into two extents if it crosses
     * the +/- 180 degree line. Modifies the given extent to be in the
     * source CRS coordinates, and if it was split, returns true, and
     * also sets the contents of the r2 parameter
     */
    static bool reprojectToLayerExtent( const QgsMapLayer *ml, const QgsCoordinateTransform &ct, QgsRectangle &extent, QgsRectangle &r2 );

    bool needTemporaryImage( QgsMapLayer *ml );

    const QgsFeatureFilterProvider *mFeatureFilterProvider = nullptr;
};


/**
 * \ingroup core
 * Intermediate base class adding functionality that allows client to query the rendered image.
 *  The image can be queried even while the rendering is still in progress to get intermediate result
 *
 * \since QGIS 2.4
 */
class CORE_EXPORT QgsMapRendererQImageJob : public QgsMapRendererJob
{
    Q_OBJECT

  public:
    QgsMapRendererQImageJob( const QgsMapSettings &settings );

    //! Gets a preview/resulting image
    virtual QImage renderedImage() = 0;

};


#endif // QGSMAPRENDERERJOB_H
