/***************************************************************************
  qgsmaprendererstagedrenderjob.h
  --------------------------------------
  Date                 : August 2019
  Copyright            : (C) 2019 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPRENDERERSTAGEDRENDERJOB_H
#define QGSMAPRENDERERSTAGEDRENDERJOB_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaprenderercustompainterjob.h"

#define SIP_NO_FILE

/**
 * \ingroup core
 * Render job implementation that renders maps in stages, allowing different stages (e.g. individual map
 * layers) to be rendered to different paint devices.
 *
 * Unlike other renderer jobs, this only supports synchronous rendering in main thread.
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsMapRendererStagedRenderJob : public QgsMapRendererAbstractCustomPainterJob
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsMapRendererStagedRenderJob, using the given
     * map \a settings.
     */
    QgsMapRendererStagedRenderJob( const QgsMapSettings &settings );
    ~QgsMapRendererStagedRenderJob() override;

    void start() override;
    void cancel() override;
    void cancelWithoutBlocking() override;
    void waitForFinished() override;
    bool isActive() const override;
    bool usedCachedLabels() const override;
    QgsLabelingResults *takeLabelingResults() SIP_TRANSFER override;

    /**
     * Renders the current part of the map to the specified \a painter.
     *
     * Returns TRUE if a part was rendered or FALSE if nothing was rendered.
     */
    bool renderCurrentPart( QPainter *painter );

    /**
     * Iterates to the next part to render.
     *
     * Returns TRUE if another part exists to render, or FALSE if all parts
     * have been rendered and nothing remains.
     */
    bool nextPart();

    /**
     * Returns TRUE if the job is finished, and nothing remains to render.
     */
    bool isFinished();

  private:

    std::unique_ptr< QgsLabelingEngine > mLabelingEngineV2;

    LayerRenderJobs mLayerJobs;
    LabelRenderJob mLabelJob;
    LayerRenderJobs::iterator mJobIt;

    bool mNextIsLabel = false;
    bool mExportedLabels = false;

};

#endif // QGSMAPRENDERERSTAGEDRENDERJOB_H
