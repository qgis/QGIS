/***************************************************************************
  qgsvectorlayerdiagramprovider.h
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

#ifndef QGSVECTORLAYERDIAGRAMPROVIDER_H
#define QGSVECTORLAYERDIAGRAMPROVIDER_H

#include "qgslabelingenginev2.h"

//#include "qgsdiagramrendererv2.h"


/**
 * @brief The QgsVectorLayerDiagramProvider class implements support for diagrams within
 * the labeling engine. Parameters for the diagrams are taken from the layer settings.
 *
 * @note added in QGIS 2.12
 */
class CORE_EXPORT QgsVectorLayerDiagramProvider : public QgsAbstractLabelProvider
{
  public:
    QgsVectorLayerDiagramProvider( QgsVectorLayer* layer );
    ~QgsVectorLayerDiagramProvider();

    virtual QString id() const override;

    virtual QList<QgsLabelFeature*> labelFeatures( const QgsMapSettings& mapSettings, const QgsRenderContext& context ) override;

    virtual void drawLabel( QgsRenderContext& context, pal::LabelPosition* label ) const override;

  protected:
    QgsLabelFeature* registerDiagram( QgsFeature& feat, const QgsMapSettings& mapSettings, const QgsRenderContext& context );

  protected:
    QgsVectorLayer* mLayer;

    QgsDiagramLayerSettings mSettings;
};

#endif // QGSVECTORLAYERDIAGRAMPROVIDER_H
