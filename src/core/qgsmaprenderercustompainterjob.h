/***************************************************************************
  qgsmaprenderercustompainterjob.h
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

#ifndef QGSMAPRENDERERCUSTOMPAINTERJOB_H
#define QGSMAPRENDERERCUSTOMPAINTERJOB_H

#include "qgsmaprendererjob.h"

#include <QEventLoop>

/** Job implementation that renders everything sequentially using a custom painter.
 *
 * Also supports synchronous rendering in main thread for cases when rendering in background
 * is not an option because of some technical limitations (e.g. printing to printer on some
 * platforms).
 *
 * @note added in 2.4
 */
class CORE_EXPORT QgsMapRendererCustomPainterJob : public QgsMapRendererJob
{
    Q_OBJECT
  public:
    QgsMapRendererCustomPainterJob( const QgsMapSettings& settings, QPainter* painter );
    ~QgsMapRendererCustomPainterJob();

    virtual void start() override;
    virtual void cancel() override;
    virtual void waitForFinished() override;
    virtual bool isActive() const override;
    virtual QgsLabelingResults* takeLabelingResults() override;

    //! @note not available in python bindings
    const LayerRenderJobs& jobs() const { return mLayerJobs; }

    /**
     * Wait for the job to be finished - and keep the thread's event loop running while waiting.
     *
     * With a call to waitForFinished(), the waiting is done with a synchronization primitive
     * and does not involve processing of messages. That may cause issues to code which requires
     * some events to be handled in the main thread. Some plugins hooking into the rendering
     * pipeline may require this in order to work properly - for example, OpenLayers plugin
     * which uses a QWebPage in the main thread.
     *
     * Ideally the "wait for finished" method should not be used at all. The code triggering
     * rendering should not need to actively wait for rendering to finish.
     */
    void waitForFinishedWithEventLoop( QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents );

    /**
     * Render the map synchronously in this thread. The function does not return until the map
     * is completely rendered.
     *
     * This is an alternative to ordinary API (using start() + waiting for finished() signal).
     * Users are discouraged to use this method unless they have a strong reason for doing it.
     * The synchronous rendering blocks the main thread, making the application unresponsive.
     * Also, it is not possible to cancel rendering while it is in progress.
     */
    void renderSynchronously();

  protected slots:
    void futureFinished();

  protected:
    static void staticRender( QgsMapRendererCustomPainterJob* self ); // function to be used within the thread

    // these methods are called within worker thread
    void doRender();

  private:
    QPainter* mPainter;
    QFuture<void> mFuture;
    QFutureWatcher<void> mFutureWatcher;
    QgsRenderContext mLabelingRenderContext;
    QgsPalLabeling* mLabelingEngine;

    bool mActive;
    LayerRenderJobs mLayerJobs;
    bool mRenderSynchronously;
};


#endif // QGSMAPRENDERERCUSTOMPAINTERJOB_H
