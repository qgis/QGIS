/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaprendererjob.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








class QgsMapRendererJob : QObject
{
%Docstring
Abstract base class for map rendering implementations.

The API is designed in a way that rendering is done asynchronously, therefore
the caller is not blocked while the rendering is in progress. Non-blocking
operation is quite important because the rendering can take considerable
amount of time.

Common use case:
0. prepare QgsMapSettings with rendering configuration (extent, layer, map size, ...)
1. create QgsMapRendererJob subclass with QgsMapSettings instance
2. connect to job's finished() signal
3. call start(). Map rendering will start in background, the function immediately returns
4. at some point, slot connected to finished() signal is called, map rendering is done

It is possible to cancel the rendering job while it is active by calling cancel() function.

The following subclasses are available:
- QgsMapRendererSequentialJob - renders map in one background thread to an image
- QgsMapRendererParallelJob - renders map in multiple background threads to an image
- QgsMapRendererCustomPainterJob - renders map with given QPainter in one background thread

.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgsmaprendererjob.h"
%End
  public:

    QgsMapRendererJob( const QgsMapSettings &settings );

    virtual void start() = 0;
%Docstring
Start the rendering job and immediately return.
Does nothing if the rendering is already in progress.
%End

    virtual void cancel() = 0;
%Docstring
Stop the rendering job - does not return until the job has terminated.
Does nothing if the rendering is not active.
%End

    virtual void cancelWithoutBlocking() = 0;
%Docstring
Triggers cancelation of the rendering job without blocking. The render job will continue
to operate until it is able to cancel, at which stage the finished() signal will be emitted.
Does nothing if the rendering is not active.
%End

    virtual void waitForFinished() = 0;
%Docstring
Block until the job has finished.
%End

    virtual bool isActive() const = 0;
%Docstring
Tell whether the rendering job is currently running in background.
%End

    virtual bool usedCachedLabels() const = 0;
%Docstring
Returns true if the render job was able to use a cached labeling solution.
If so, any previously stored labeling results (see takeLabelingResults())
should be retained.

.. seealso:: :py:func:`takeLabelingResults`

.. versionadded:: 3.0
%End

    virtual QgsLabelingResults *takeLabelingResults() = 0 /Transfer/;
%Docstring
Get pointer to internal labeling engine (in order to get access to the results).
This should not be used if cached labeling was redrawn - see usedCachedLabels().

.. seealso:: :py:func:`usedCachedLabels`
%End

    void setFeatureFilterProvider( const QgsFeatureFilterProvider *f );
%Docstring

.. versionadded:: 3.0
Set the feature filter provider used by the QgsRenderContext of
each LayerRenderJob.
Ownership is not transferred and the provider must not be deleted
before the render job.
%End

    const QgsFeatureFilterProvider *featureFilterProvider() const;
%Docstring

.. versionadded:: 3.0
Returns the feature filter provider used by the QgsRenderContext of
each LayerRenderJob.
%End

    struct Error
    {
      Error( const QString &lid, const QString &msg );

      QString layerID;
      QString message;
    };

    typedef QList<QgsMapRendererJob::Error> Errors;

    Errors errors() const;
%Docstring
List of errors that happened during the rendering job - available when the rendering has been finished
%End


    void setCache( QgsMapRendererCache *cache );
%Docstring
Assign a cache to be used for reading and storing rendered images of individual layers.
Does not take ownership of the object.
%End

    int renderingTime() const;
%Docstring
Returns the total time it took to finish the job (in milliseconds).

.. seealso:: :py:func:`perLayerRenderingTime`
%End


    const QgsMapSettings &mapSettings() const;
%Docstring
Return map settings with which this job was started.

:return: A QgsMapSettings instance with render settings

.. versionadded:: 2.8
%End


  signals:

    void renderingLayersFinished();
%Docstring
Emitted when the layers are rendered.
Rendering labels is not yet done. If the fully rendered layer including labels is required use
finished() instead.

.. versionadded:: 3.0
%End

    void finished();
%Docstring
emitted when asynchronous rendering is finished (or canceled).
%End

  protected:













};


class QgsMapRendererQImageJob : QgsMapRendererJob
{
%Docstring
Intermediate base class adding functionality that allows client to query the rendered image.
The image can be queried even while the rendering is still in progress to get intermediate result

.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgsmaprendererjob.h"
%End
  public:
    QgsMapRendererQImageJob( const QgsMapSettings &settings );

    virtual QImage renderedImage() = 0;
%Docstring
Get a preview/resulting image
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaprendererjob.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
