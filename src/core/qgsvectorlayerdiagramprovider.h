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


/**
 * Class that adds extra information to QgsLabelFeature for labeling of diagrams
 *
 * @note not part of public API
 */
class QgsDiagramLabelFeature : public QgsLabelFeature
{
  public:
    //! Create label feature, takes ownership of the geometry instance
    QgsDiagramLabelFeature( QgsFeatureId id, GEOSGeometry* geometry, const QSizeF& size )
        : QgsLabelFeature( id, geometry, size ) {}

    void setDiagramAttributes( const QgsAttributes& attrs ) { mDiagramAttributes = attrs; }
    const QgsAttributes& diagramAttributes() { return mDiagramAttributes; }

  protected:
    /** Stores attribute values for diagram rendering*/
    QgsAttributes mDiagramAttributes;
};


class QgsAbstractFeatureSource;


/**
 * @brief The QgsVectorLayerDiagramProvider class implements support for diagrams within
 * the labeling engine. Parameters for the diagrams are taken from the layer settings.
 *
 * @note added in QGIS 2.12
 */
class CORE_EXPORT QgsVectorLayerDiagramProvider : public QgsAbstractLabelProvider
{
  public:

    //! Convenience constructor to initialize the provider from given vector layer
    explicit QgsVectorLayerDiagramProvider( QgsVectorLayer* layer, bool ownFeatureLoop = true );

    QgsVectorLayerDiagramProvider( const QgsDiagramLayerSettings* diagSettings,
                                   const QgsDiagramRendererV2* diagRenderer,
                                   const QString& layerId,
                                   const QgsFields& fields,
                                   const QgsCoordinateReferenceSystem& crs,
                                   QgsAbstractFeatureSource* source,
                                   bool ownsSource );

    ~QgsVectorLayerDiagramProvider();

    virtual QString id() const override;

    virtual QList<QgsLabelFeature*> labelFeatures( const QgsRenderContext& context ) override;

    virtual void drawLabel( QgsRenderContext& context, pal::LabelPosition* label ) const override;

    // new virtual methods

    virtual bool prepare( const QgsRenderContext& context, QStringList& attributeNames );

    virtual void registerFeature( QgsFeature& feature, const QgsRenderContext& context );

  protected:
    void init();
    QgsLabelFeature* registerDiagram( QgsFeature& feat, const QgsRenderContext& context );

  protected:

    QgsDiagramLayerSettings mSettings;
    QgsDiagramRendererV2* mDiagRenderer;

    QString mLayerId;
    QgsFields mFields;
    QgsCoordinateReferenceSystem mLayerCrs;
    QgsAbstractFeatureSource* mSource;
    bool mOwnsSource;

    QList<QgsLabelFeature*> mFeatures;
};

#endif // QGSVECTORLAYERDIAGRAMPROVIDER_H
