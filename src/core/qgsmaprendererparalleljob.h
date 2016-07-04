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

#include "qgsmaprendererjob.h"

/** \ingroup core
 * Job implementation that renders all layers in parallel.
 *
 * The resulting map image can be retrieved with renderedImage() function.
 * It is safe to call that function while rendering is active to see preview of the map.
 *
 * @note added in 2.4
 */
class CORE_EXPORT QgsMapRendererParallelJob : public QgsMapRendererQImageJob
{
    Q_OBJECT
  public:
    QgsMapRendererParallelJob( const QgsMapSettings& settings );
    ~QgsMapRendererParallelJob();

    virtual void start() override;
    virtual void cancel() override;
    virtual void waitForFinished() override;
    virtual bool isActive() const override;

    virtual QgsLabelingResults* takeLabelingResults() override;

    // from QgsMapRendererJobWithPreview
    virtual QImage renderedImage() override;

  protected slots:
    //! layers are rendered, labeling is still pending
    void renderLayersFinished();
    //! all rendering is finished, including labeling
    void renderingFinished();

  protected:

    static void renderLayerStatic( LayerRenderJob& job );
    static void renderLabelsStatic( QgsMapRendererParallelJob* self );

  protected:

    QImage mFinalImage;

    //! @note not available in Python bindings
    enum { Idle, RenderingLayers, RenderingLabels } mStatus;

    QFuture<void> mFuture;
    QFutureWatcher<void> mFutureWatcher;

    LayerRenderJobs mLayerJobs;

    //! Old labeling engine
    QgsPalLabeling* mLabelingEngine;
    //! New labeling engine
    QgsLabelingEngineV2* mLabelingEngineV2;
    QgsRenderContext mLabelingRenderContext;
    QFuture<void> mLabelingFuture;
    QFutureWatcher<void> mLabelingFutureWatcher;
};


#endif // QGSMAPRENDERERPARALLELJOB_H
