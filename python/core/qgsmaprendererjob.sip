
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

/** abstract base class renderer jobs that asynchronously start map rendering */
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



/** job implementation that renders everything sequentially in one thread */
class QgsMapRendererSequentialJob : QgsMapRendererQImageJob
{
%TypeHeaderCode
#include <qgsmaprendererjob.h>
%End

  public:
    QgsMapRendererSequentialJob( const QgsMapSettings& settings );
    ~QgsMapRendererSequentialJob();

    virtual void start();
    virtual void cancel();
    virtual void waitForFinished();
    virtual bool isActive() const;

    virtual QgsLabelingResults* takeLabelingResults() /Transfer/;

    // from QgsMapRendererJobWithPreview
    virtual QImage renderedImage();

  public slots:

    void internalFinished();
};




/** job implementation that renders all layers in parallel */
class QgsMapRendererParallelJob : QgsMapRendererQImageJob
{
%TypeHeaderCode
#include <qgsmaprendererjob.h>
%End

  public:
    QgsMapRendererParallelJob( const QgsMapSettings& settings );
    ~QgsMapRendererParallelJob();

    virtual void start();
    virtual void cancel();
    virtual void waitForFinished();
    virtual bool isActive() const;

    virtual QgsLabelingResults* takeLabelingResults() /Transfer/;

    // from QgsMapRendererJobWithPreview
    virtual QImage renderedImage();

  protected slots:
    //! layers are rendered, labeling is still pending
    void renderLayersFinished();
    //! all rendering is finished, including labeling
    void renderingFinished();

  protected:

    static void renderLayerStatic( LayerRenderJob& job );
    static void renderLabelsStatic( QgsMapRendererParallelJob* self );
};



/** job implementation that renders everything sequentially using a custom painter.
 *  The returned image is always invalid (because there is none available).
 */
class QgsMapRendererCustomPainterJob : QgsMapRendererJob
{
%TypeHeaderCode
#include <qgsmaprendererjob.h>
%End

  public:
    QgsMapRendererCustomPainterJob( const QgsMapSettings& settings, QPainter* painter );
    ~QgsMapRendererCustomPainterJob();

    virtual void start();
    virtual void cancel();
    virtual void waitForFinished();
    virtual bool isActive() const;
    virtual QgsLabelingResults* takeLabelingResults() /Transfer/;

    //! @note not available in python bindings
    // const LayerRenderJobs& jobs() const { return mLayerJobs; }

  protected slots:
    void futureFinished();

  protected:
    static void staticRender( QgsMapRendererCustomPainterJob* self ); // function to be used within the thread

    // these methods are called within worker thread
    void doRender();
};
