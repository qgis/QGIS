/***************************************************************************
  qgsvectorlayerlabelprovider.h
  --------------------------------------
  Date                 : September 2015
  Copyright            : (C) 2015 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERLABELPROVIDER_H
#define QGSVECTORLAYERLABELPROVIDER_H

#include "qgslabelingenginev2.h"

/**
 * @brief The QgsVectorLayerLabelProvider class implements a label provider
 * for vector layers. Parameters for the labeling are taken from the layer's
 * custom properties.
 *
 * @note added in QGIS 2.12
 */
class CORE_EXPORT QgsVectorLayerLabelProvider : public QgsAbstractLabelProvider
{
  public:
    QgsVectorLayerLabelProvider( QgsVectorLayer* layer );
    ~QgsVectorLayerLabelProvider();

    virtual QString id() const override;

    virtual QList<QgsLabelFeature*> labelFeatures( const QgsMapSettings& mapSettings, const QgsRenderContext& context ) override;

    virtual void drawLabel( QgsRenderContext& context, pal::LabelPosition* label ) const override;


  protected:
    void drawLabelPrivate( pal::LabelPosition* label, QgsRenderContext& context, QgsPalLayerSettings& tmpLyr, QgsPalLabeling::DrawLabelType drawType, double dpiRatio = 1.0 ) const;

  protected:
    QgsVectorLayer* mLayer;

    QgsPalLayerSettings mSettings;
};


#endif // QGSVECTORLAYERLABELPROVIDER_H
