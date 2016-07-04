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
#include "qgsrendererv2.h"

class QgsAbstractFeatureSource;
class QgsFeatureRendererV2;
class QgsSymbolV2;

/** \ingroup core
 * @brief The QgsVectorLayerLabelProvider class implements a label provider
 * for vector layers. Parameters for the labeling are taken from the layer's
 * custom properties or from the given settings.
 *
 * @note added in QGIS 2.12
 * @note this class is not a part of public API yet. See notes in QgsLabelingEngineV2
 * @note not available in Python bindings
 */
class CORE_EXPORT QgsVectorLayerLabelProvider : public QgsAbstractLabelProvider
{
  public:

    //! Convenience constructor to initialize the provider from given vector layer
    explicit QgsVectorLayerLabelProvider( QgsVectorLayer* layer,
                                          const QString& providerId,
                                          bool withFeatureLoop = true,
                                          const QgsPalLayerSettings* settings = nullptr,
                                          const QString& layerName = QString() );

    //! Construct diagram provider with all the necessary configuration parameters
    QgsVectorLayerLabelProvider( const QgsPalLayerSettings& settings,
                                 const QString& layerId,
                                 const QgsFields& fields,
                                 const QgsCoordinateReferenceSystem& crs,
                                 QgsAbstractFeatureSource* source,
                                 bool ownsSource,
                                 QgsFeatureRendererV2* renderer = nullptr );

    ~QgsVectorLayerLabelProvider();

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
     * Register a feature for labeling as one or more QgsLabelFeature objects stored into mLabels
     *
     * @param feature feature to label
     * @param context render context. The QgsExpressionContext contained within the render context
     * must have already had the feature and fields sets prior to calling this method.
     * @param obstacleGeometry optional obstacle geometry, if a different geometry to the feature's geometry
     * should be used as an obstacle for labels (eg, if the feature has been rendered with an offset point
     * symbol, the obstacle geometry should represent the bounds of the offset symbol). If not set,
     * the feature's original geometry will be used as an obstacle for labels.
     */
    virtual void registerFeature( QgsFeature& feature, QgsRenderContext &context, QgsGeometry* obstacleGeometry = nullptr );

    /** Returns the geometry for a point feature which should be used as an obstacle for labels. This
     * obstacle geometry will respect the dimensions and offsets of the symbol used to render the
     * point, and ensures that labels will not overlap large or offset points.
     * @param fet point feature
     * @param context render context
     * @param symbols symbols rendered for point feature
     * @note added in QGIS 2.14
     */
    static QgsGeometry* getPointObstacleGeometry( QgsFeature& fet, QgsRenderContext& context, const QgsSymbolV2List& symbols );

  protected:
    //! initialization method - called from constructors
    void init();
    //! Internal label drawing method
    void drawLabelPrivate( pal::LabelPosition* label, QgsRenderContext& context, QgsPalLayerSettings& tmpLyr, QgsPalLabeling::DrawLabelType drawType, double dpiRatio = 1.0 ) const;

  protected:
    //! Layer's labeling configuration
    QgsPalLayerSettings mSettings;
    //! Geometry type of layer
    QGis::GeometryType mLayerGeometryType;

    QgsFeatureRendererV2* mRenderer;

    // these are needed only if using own renderer loop

    //! Layer's fields
    QgsFields mFields;
    //! Layer's CRS
    QgsCoordinateReferenceSystem mCrs;
    //! Layer's feature source
    QgsAbstractFeatureSource* mSource;
    //! Whether layer's feature source is owned
    bool mOwnsSource;

    //! List of generated
    QList<QgsLabelFeature*> mLabels;
};

#endif // QGSVECTORLAYERLABELPROVIDER_H
