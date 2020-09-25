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
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsMapRendererStagedRenderJob : public QgsMapRendererAbstractCustomPainterJob
{
    Q_OBJECT
  public:

    /**
     * Flags which control the staged render job behavior.
     */
    enum Flag
    {
      RenderLabelsByMapLayer = 0x01, //!< Labels should be rendered in individual stages by map layer. This allows separation of labels belonging to different layers, but may affect label stacking order as the order will become layer-dependent, instead of per-label-dependent.
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Represents the stages of a rendering job.
     */
    enum RenderStage
    {
      Symbology, //!< Rendering layer symbology
      Labels, //!< Rendering labels
      Finished, //!< Rendering is finished
    };

    /**
     * Constructor for QgsMapRendererStagedRenderJob, using the given
     * map \a settings.
     *
     * The optional \a flags argument can be used to control the staged render job behavior.
     */
    QgsMapRendererStagedRenderJob( const QgsMapSettings &settings, Flags flags = Flags() );
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
    bool isFinished() const;

    /**
     * Returns the ID of the current layer about to be rendered in the next render operation.
     */
    QString currentLayerId() const;

    /**
     * Returns the opacity for the current layer about to be rendered in the next render operation.
     *
     * \since QGIS 3.14
     */
    double currentLayerOpacity() const;

    /**
     * Returns the composition mode for the current layer about to be rendered in the next render operation.
     *
     * \since QGIS 3.14
     */
    QPainter::CompositionMode currentLayerCompositionMode() const;

    /**
     * Returns the current stage which will be rendered in the next render operation.
     */
    RenderStage currentStage() const;

  private:

    std::unique_ptr< QgsLabelingEngine > mLabelingEngineV2;

    LayerRenderJobs mLayerJobs;
    LabelRenderJob mLabelJob;
    LayerRenderJobs::iterator mJobIt;

    bool mNextIsLabel = false;
    bool mExportedLabels = false;
    Flags mFlags = Flags();
    bool mPreparedStagedLabelJob = false;
    QStringList mLabelingLayers;
    QStringList::iterator mLabelLayerIt;
};

#endif // QGSMAPRENDERERSTAGEDRENDERJOB_H
