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
#include "qgslabelfeature.h"

/** \ingroup core
 * Class that adds extra information to QgsLabelFeature for labeling of diagrams
 *
 * @note this class is not a part of public API yet. See notes in QgsLabelingEngineV2
 * @note not available in Python bindings
 */
class QgsDiagramLabelFeature : public QgsLabelFeature
{
  public:
    //! Create label feature, takes ownership of the geometry instance
    QgsDiagramLabelFeature( QgsFeatureId id, GEOSGeometry* geometry, QSizeF size )
        : QgsLabelFeature( id, geometry, size ) {}

    //! Store feature's attributes - used for rendering of diagrams
    void setAttributes( const QgsAttributes& attrs ) { mAttributes = attrs; }
    //! Get feature's attributes - used for rendering of diagrams
    const QgsAttributes& attributes() { return mAttributes; }

  protected:
    /** Stores attribute values for diagram rendering*/
    QgsAttributes mAttributes;
};


class QgsAbstractFeatureSource;


/** \ingroup core
 * @brief The QgsVectorLayerDiagramProvider class implements support for diagrams within
 * the labeling engine. Parameters for the diagrams are taken from the layer settings.
 *
 * @note added in QGIS 2.12
 * @note this class is not a part of public API yet. See notes in QgsLabelingEngineV2
 * @note not available in Python bindings
 */
class CORE_EXPORT QgsVectorLayerDiagramProvider : public QgsAbstractLabelProvider
{
  public:

    //! Convenience constructor to initialize the provider from given vector layer
    explicit QgsVectorLayerDiagramProvider( QgsVectorLayer* layer, bool ownFeatureLoop = true );

    //! Construct diagram provider with all the necessary configuration parameters
    QgsVectorLayerDiagramProvider( const QgsDiagramLayerSettings* diagSettings,
                                   const QgsDiagramRendererV2* diagRenderer,
                                   const QString& layerId,
                                   const QgsFields& fields,
                                   const QgsCoordinateReferenceSystem& crs,
                                   QgsAbstractFeatureSource* source,
                                   bool ownsSource );

    //! Clean up
    ~QgsVectorLayerDiagramProvider();

    virtual QList<QgsLabelFeature*> labelFeatures( QgsRenderContext& context ) override;

    virtual void drawLabel( QgsRenderContext& context, pal::LabelPosition* label ) const override;

    // new virtual methods

    /**
     * Prepare for registration of features. Must be called after provider has been added to engine (uses its map settings)
     * @param context render context.
     * @param attributeNames list of attribute names to which additional required attributes shall be added
     * @return Whether the preparation was successful - if not, the provider shall not be used
     */
    virtual bool prepare( const QgsRenderContext& context, QStringList& attributeNames );

    /**
     * Register a feature for labeling as one or more QgsLabelFeature objects stored into mFeatures
     *
     * @param feature feature for diagram
     * @param context render context. The QgsExpressionContext contained within the render context
     * must have already had the feature and fields sets prior to calling this method.
     * @param obstacleGeometry optional obstacle geometry, if a different geometry to the feature's geometry
     * should be used as an obstacle for labels (eg, if the feature has been rendered with an offset point
     * symbol, the obstacle geometry should represent the bounds of the offset symbol). If not set,
     * the feature's original geometry will be used as an obstacle for labels. Ownership of obstacleGeometry
     * is transferred.
     */
    virtual void registerFeature( QgsFeature& feature, QgsRenderContext &context, QgsGeometry* obstacleGeometry = nullptr );

  protected:
    //! initialization method - called from constructors
    void init();
    //! helper method to register one diagram feautre
    QgsLabelFeature* registerDiagram( QgsFeature& feat, QgsRenderContext& context, QgsGeometry* obstacleGeometry = nullptr );

  protected:

    //! Diagram layer settings
    QgsDiagramLayerSettings mSettings;
    //! Diagram renderer instance (owned by mSettings)
    QgsDiagramRendererV2* mDiagRenderer;

    // these are needed only if using own renderer loop

    //! Layer's fields
    QgsFields mFields;
    //! Layer's CRS
    QgsCoordinateReferenceSystem mLayerCrs;
    //! Layer's feature source
    QgsAbstractFeatureSource* mSource;
    //! Whether layer's feature source is owned
    bool mOwnsSource;

    //! List of generated label features (owned by the provider)
    QList<QgsLabelFeature*> mFeatures;
};

#endif // QGSVECTORLAYERDIAGRAMPROVIDER_H
