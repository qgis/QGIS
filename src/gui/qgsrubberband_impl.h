/***************************************************************************
    qgsrubberband_impl.h
     --------------------------------------
    Date                 : July 2026
    Copyright            : (C) 2026 by Nyall Dawson
    Email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRUBBERBANDIMPL_H
#define QGSRUBBERBANDIMPL_H

#include "qgis_gui.h"
#include "qgsfeature.h"
#include "qgsrubberband.h"

#include <QPointer>

#define SIP_NO_FILE

class QgsVectorLayer;


/**
 * \ingroup gui
 * \brief Rubber band preview item for showing vector layer labels.
 *
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsVectorLayerLabelRubberBandPreview : public QgsRubberBandPreviewItem
{
  public:
    /**
     * Constructor for QgsVectorLayerLabelRubberBandPreview.
     * \param rubberBand associated rubber band
     * \param fids Feature IDs for the features being manipulated
     * \param layer The vector layer containing labeling configuration.
   */
    QgsVectorLayerLabelRubberBandPreview( QgsRubberBand *rubberBand, const QList< QgsFeatureId > &fids, QgsVectorLayer *layer );

    void render( QgsRenderContext &context ) final;

  private:
    QList< QgsFeature > mFeatures;
    QPointer< QgsVectorLayer > mLayer;
};

#endif // QGSRUBBERBANDIMPL_H
