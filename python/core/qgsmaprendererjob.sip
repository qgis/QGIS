
struct LayerRenderJob
{
  QgsRenderContext context;
  QImage* img; // may be null if it is not necessary to draw to separate image (e.g. sequential rendering)
  QgsMapLayerRenderer* renderer; // must be deleted
  QPainter::CompositionMode blendMode;
  bool cached; // if true, img already contains cached image from previous rendering
  QString layerId;
};

typedef QList<LayerRenderJob> LayerRenderJobs;

/**
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
 * @note added in 2.4
 */
class QgsMapRendererJob : QObject
{
%TypeHeaderCode
#include <qgsmaprendererjob.h>
%End

  public:

    QgsMapRendererJob( const QgsMapSettings& settings );

    virtual ~QgsMapRendererJob();

    //! Start the rendering job and immediately return.
    //! Does nothing if the rendering is already in progress.
    virtual void start() = 0;

    //! Stop the rendering job - does not return until the job has terminated.
    //! Does nothing if the rendering is not active.
    virtual void cancel() = 0;

    //! Block until the job has finished.
    virtual void waitForFinished() = 0;

    //! Tell whether the rendering job is currently running in background.
    virtual bool isActive() const = 0;

    //! Get pointer to internal labeling engine (in order to get access to the results)
    virtual QgsLabelingResults* takeLabelingResults() = 0 /Transfer/;

    struct Error
    {
      Error( const QString& lid, const QString& msg );

      QString layerID;
      QString message;
    };

    typedef QList<QgsMapRendererJob::Error> Errors;

    //! List of errors that happened during the rendering job - available when the rendering has been finished
    Errors errors() const;


    //! Assign a cache to be used for reading and storing rendered images of individual layers.
    //! Does not take ownership of the object.
    void setCache( QgsMapRendererCache* cache );

    //! Set which vector layers should be cached while rendering
    //! @note The way how geometries are cached is really suboptimal - this method may be removed in future releases
    void setRequestedGeometryCacheForLayers( const QStringList& layerIds );

    //! Find out how log it took to finish the job (in miliseconds)
    int renderingTime() const;

  signals:

    //! emitted when asynchronous rendering is finished (or canceled).
    void finished();

  protected:

    /** Convenience function to project an extent into the layer source
     * CRS, but also split it into two extents if it crosses
     * the +/- 180 degree line. Modifies the given extent to be in the
     * source CRS coordinates, and if it was split, returns true, and
     * also sets the contents of the r2 parameter
     */
    static bool reprojectToLayerExtent( const QgsCoordinateTransform* ct, bool layerCrsGeographic, QgsRectangle& extent, QgsRectangle& r2 );

    //! @note not available in python bindings
    // LayerRenderJobs prepareJobs( QPainter* painter, QgsPalLabeling* labelingEngine );

    //! @note not available in python bindings
    // void cleanupJobs( LayerRenderJobs& jobs );

    static QImage composeImage( const QgsMapSettings& settings, const LayerRenderJobs& jobs );

    bool needTemporaryImage( QgsMapLayer* ml );

    static void drawLabeling( const QgsMapSettings& settings, QgsRenderContext& renderContext, QgsPalLabeling* labelingEngine, QPainter* painter );
    static void drawOldLabeling( const QgsMapSettings& settings, QgsRenderContext& renderContext );
    static void drawNewLabeling( const QgsMapSettings& settings, QgsRenderContext& renderContext, QgsPalLabeling* labelingEngine );

    //! called when rendering has finished to update all layers' geometry caches
    void updateLayerGeometryCaches();
};


/** Intermediate base class adding functionality that allows client to query the rendered image.
 *  The image can be queried even while the rendering is still in progress to get intermediate result
 *
 * @note added in 2.4
 */
class QgsMapRendererQImageJob : QgsMapRendererJob
{
%TypeHeaderCode
#include <qgsmaprendererjob.h>
%End

  public:
    QgsMapRendererQImageJob( const QgsMapSettings& settings );

    //! Get a preview/resulting image
    virtual QImage renderedImage() = 0;
};
