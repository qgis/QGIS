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

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaprendererjob.h"

#include <QEventLoop>

/**
 * \ingroup core
 * \brief Abstract base class for map renderer jobs which use custom painters.
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsMapRendererAbstractCustomPainterJob : public QgsMapRendererJob SIP_ABSTRACT
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsMapRendererAbstractCustomPainterJob, using the given
     * map \a settings.
     */
    QgsMapRendererAbstractCustomPainterJob( const QgsMapSettings &settings );

  protected:

    /**
     * Prepares the given \a painter ready for a map render.
     *
     * The \a backgroundColor argument specifies the color to use for the map's background.
     */
    void preparePainter( QPainter *painter, const QColor &backgroundColor = Qt::transparent );

};

/**
 * \ingroup core
 * \brief Job implementation that renders everything sequentially using a custom painter.
 *
 * Also supports synchronous rendering in main thread for cases when rendering in background
 * is not an option because of some technical limitations (e.g. printing to printer on some
 * platforms).
 *
 */
class CORE_EXPORT QgsMapRendererCustomPainterJob : public QgsMapRendererAbstractCustomPainterJob
{
    Q_OBJECT
  public:
    QgsMapRendererCustomPainterJob( const QgsMapSettings &settings, QPainter *painter );
    ~QgsMapRendererCustomPainterJob() override;

    void cancel() override;
    void cancelWithoutBlocking() override;
    void waitForFinished() override;
    bool isActive() const override;
    bool usedCachedLabels() const override;
    QgsLabelingResults *takeLabelingResults() SIP_TRANSFER override;

    //! \note not available in Python bindings
    const std::vector< LayerRenderJob > &jobs() const SIP_SKIP { return mLayerJobs; }

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

    /**
     * Prepares the job for rendering synchronously in a background thread.
     *
     * Must be called from the main thread.
     *
     * This is an alternative to ordinary API (using start() + waiting for finished() signal),
     * and an alternative to renderSynchronously() (which should only ever be called from the main thread).
     *
     * \see renderPrepared()
     * \since QGIS 3.10
     */
    void prepare();

    /**
     * Render a pre-prepared job. Can be safely called in a background thread.
     *
     * Must be preceded by a call to prepare()
     *
     * This is an alternative to ordinary API (using start() + waiting for finished() signal),
     * and an alternative to renderSynchronously() (which should only ever be called from the main thread).
     *
     * \since QGIS 3.10
     */
    void renderPrepared();


  private slots:
    void futureFinished();

  private:
    static void staticRender( QgsMapRendererCustomPainterJob *self ); // function to be used within the thread

    void startPrivate() override;

    // these methods are called within worker thread
    void doRender();

    QPainter *mPainter = nullptr;
    QFuture<void> mFuture;
    QFutureWatcher<void> mFutureWatcher;
    std::unique_ptr< QgsLabelingEngine > mLabelingEngineV2;

    bool mActive;
    std::vector< LayerRenderJob > mLayerJobs;
    LabelRenderJob mLabelJob;
    bool mRenderSynchronously = false;
    bool mPrepared = false;
    bool mPrepareOnly = false;

    std::vector< LayerRenderJob > mSecondPassLayerJobs;
};


#endif // QGSMAPRENDERERCUSTOMPAINTERJOB_H
