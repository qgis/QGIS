/***************************************************************************
  qgsmaprendererparalleljob.h
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

#ifndef QGSMAPRENDERERPARALLELJOB_H
#define QGSMAPRENDERERPARALLELJOB_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaprendererjob.h"

/**
 * \ingroup core
 * Job implementation that renders all layers in parallel.
 *
 * The resulting map image can be retrieved with renderedImage() function.
 * It is safe to call that function while rendering is active to see preview of the map.
 *
 * \since QGIS 2.4
 */
class CORE_EXPORT QgsMapRendererParallelJob : public QgsMapRendererQImageJob
{
    Q_OBJECT
  public:
    QgsMapRendererParallelJob( const QgsMapSettings &settings );
    ~QgsMapRendererParallelJob() override;

    void start() override;
    void cancel() override;
    void cancelWithoutBlocking() override;
    void waitForFinished() override;
    bool isActive() const override;

    bool usedCachedLabels() const override;
    QgsLabelingResults *takeLabelingResults() SIP_TRANSFER override;

    // from QgsMapRendererJobWithPreview
    QImage renderedImage() override;

  private slots:
    //! layers are rendered, labeling is still pending
    void renderLayersFinished();
    //! all rendering is finished, including labeling
    void renderingFinished();

  private:

    //! \note not available in Python bindings
    static void renderLayerStatic( LayerRenderJob &job ) SIP_SKIP;
    //! \note not available in Python bindings
    static void renderLabelsStatic( QgsMapRendererParallelJob *self ) SIP_SKIP;

    QImage mFinalImage;

    //! \note not available in Python bindings
    enum { Idle, RenderingLayers, RenderingLabels } mStatus SIP_SKIP;

    QFuture<void> mFuture;
    QFutureWatcher<void> mFutureWatcher;

    LayerRenderJobs mLayerJobs;
    LabelRenderJob mLabelJob;

    //! New labeling engine
    std::unique_ptr< QgsLabelingEngine > mLabelingEngineV2;
    QFuture<void> mLabelingFuture;
    QFutureWatcher<void> mLabelingFutureWatcher;

};


#endif // QGSMAPRENDERERPARALLELJOB_H
