/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaprenderercustompainterjob.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsMapRendererCustomPainterJob : QgsMapRendererJob
{
%Docstring
Job implementation that renders everything sequentially using a custom painter.

Also supports synchronous rendering in main thread for cases when rendering in background
is not an option because of some technical limitations (e.g. printing to printer on some
platforms).

.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgsmaprenderercustompainterjob.h"
%End
  public:
    QgsMapRendererCustomPainterJob( const QgsMapSettings &settings, QPainter *painter );
    ~QgsMapRendererCustomPainterJob();

    virtual void start();

    virtual void cancel();

    virtual void cancelWithoutBlocking();

    virtual void waitForFinished();

    virtual bool isActive() const;

    virtual bool usedCachedLabels() const;

    virtual QgsLabelingResults *takeLabelingResults() /Transfer/;



    void waitForFinishedWithEventLoop( QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents );
%Docstring
Wait for the job to be finished - and keep the thread's event loop running while waiting.

With a call to waitForFinished(), the waiting is done with a synchronization primitive
and does not involve processing of messages. That may cause issues to code which requires
some events to be handled in the main thread. Some plugins hooking into the rendering
pipeline may require this in order to work properly - for example, OpenLayers plugin
which uses a QWebPage in the main thread.

Ideally the "wait for finished" method should not be used at all. The code triggering
rendering should not need to actively wait for rendering to finish.
%End

    void renderSynchronously();
%Docstring
Render the map synchronously in this thread. The function does not return until the map
is completely rendered.

This is an alternative to ordinary API (using start() + waiting for finished() signal).
Users are discouraged to use this method unless they have a strong reason for doing it.
The synchronous rendering blocks the main thread, making the application unresponsive.
Also, it is not possible to cancel rendering while it is in progress.
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaprenderercustompainterjob.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
