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

    //! @note Added in QGIS 3.0
    //! Set the feature filter provider used by the QgsRenderContext of
    //! each LayerRenderJob.
    //! Ownership is not transferred and the provider must not be deleted
    //! before the render job.
    void setFeatureFilterProvider( const QgsFeatureFilterProvider *f );

    //! @note Added in QGIS 3.0
    //! Returns the feature filter provider used by the QgsRenderContext of
    //! each LayerRenderJob.
    const QgsFeatureFilterProvider* featureFilterProvider() const;

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

    //! Find out how long it took to finish the job (in milliseconds)
    int renderingTime() const;

    /**
     * Return map settings with which this job was started.
     * @return A QgsMapSettings instance with render settings
     * @note added in 2.8
     */
    const QgsMapSettings& mapSettings() const;

  signals:
    /**
     * Emitted when the layers are rendered.
     * Rendering labels is not yet done. If the fully rendered layer including labels is required use
     * finished() instead.
     *
     * @note Added in QGIS 3.0
     */
    void renderingLayersFinished();

    //! emitted when asynchronous rendering is finished (or canceled).
    void finished();

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
