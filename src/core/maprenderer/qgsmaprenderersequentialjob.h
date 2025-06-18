/***************************************************************************
  qgsmaprenderersequentialjob.h
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

#ifndef QGSMAPRENDERERSEQUENTIALJOB_H
#define QGSMAPRENDERERSEQUENTIALJOB_H

#include "qgis_core.h"
#include "qgsmaprendererjob.h"

class QgsMapRendererCustomPainterJob;

/**
 * \ingroup core
 * \brief Job implementation that renders everything sequentially in one thread.
 *
 * The resulting map image can be retrieved with renderedImage() function.
 * It is safe to call that function while rendering is active to see preview of the map.
 *
 */
class CORE_EXPORT QgsMapRendererSequentialJob : public QgsMapRendererQImageJob
{
    Q_OBJECT
  public:
    QgsMapRendererSequentialJob( const QgsMapSettings &settings );
    ~QgsMapRendererSequentialJob() override;

    void cancel() final;
    void cancelWithoutBlocking() final;
    void waitForFinished() final;
    bool isActive() const final;

    bool usedCachedLabels() const final;
    QgsLabelingResults *takeLabelingResults() final SIP_TRANSFER;

    // from QgsMapRendererJobWithPreview
    QImage renderedImage() final;

  public slots:

    void internalFinished();

  private:

    void startPrivate() override;

    QgsMapRendererCustomPainterJob *mInternalJob = nullptr;
    QImage mImage;
    QPainter *mPainter = nullptr;
    std::unique_ptr< QgsLabelingResults > mLabelingResults;
    bool mUsedCachedLabels = false;

};


#endif // QGSMAPRENDERERSEQUENTIALJOB_H
