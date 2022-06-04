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

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgslabelingengine.h"
#include "qgsrenderer.h"
#include "qgstextrenderer.h"
#include "qgspallabeling.h"

class QgsAbstractFeatureSource;
class QgsFeatureRenderer;
class QgsSymbol;

/**
 * \ingroup core
 * \brief The QgsVectorLayerLabelProvider class implements a label provider
 * for vector layers. Parameters for the labeling are taken from the layer's
 * custom properties or from the given settings.
 *
 * \note this class is not a part of public API yet. See notes in QgsLabelingEngine
 * \note not available in Python bindings
 * \since QGIS 2.12
 */
class CORE_EXPORT QgsVectorLayerLabelProvider : public QgsAbstractLabelProvider
{
  public:

    //! Convenience constructor to initialize the provider from given vector layer
    explicit QgsVectorLayerLabelProvider( QgsVectorLayer *layer,
                                          const QString &providerId,
                                          bool withFeatureLoop,
                                          const QgsPalLayerSettings *settings,
                                          const QString &layerName = QString() );

    //! Constructor to initialize the provider from any map layer (e.g. vector tile layer)
    explicit QgsVectorLayerLabelProvider( QgsWkbTypes::GeometryType geometryType,
                                          const QgsFields &fields,
                                          const QgsCoordinateReferenceSystem &crs,
                                          const QString &providerId,
                                          const QgsPalLayerSettings *settings,
                                          QgsMapLayer *layer,
                                          const QString &layerName = QString() );

    ~QgsVectorLayerLabelProvider() override;

    QList<QgsLabelFeature *> labelFeatures( QgsRenderContext &context ) override;

    void drawLabelBackground( QgsRenderContext &context, pal::LabelPosition *label ) const override;
    void drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const override;
    void drawUnplacedLabel( QgsRenderContext &context, pal::LabelPosition *label ) const override;
    void startRender( QgsRenderContext &context ) override;
    void stopRender( QgsRenderContext &context ) override;

    // new virtual methods

    /**
     * Prepare for registration of features. Must be called after provider has been added to engine (uses its map settings)
     * \param context render context.
     * \param attributeNames list of attribute names to which additional required attributes shall be added
     * \returns Whether the preparation was successful - if not, the provider shall not be used
     */
    virtual bool prepare( QgsRenderContext &context, QSet<QString> &attributeNames );

    /**
     * Register a feature for labeling as one or more QgsLabelFeature objects stored into mLabels
     *
     * \param feature feature to label
     * \param context render context. The QgsExpressionContext contained within the render context
     * must have already had the feature and fields sets prior to calling this method.
     * \param obstacleGeometry optional obstacle geometry, if a different geometry to the feature's geometry
     * should be used as an obstacle for labels (e.g., if the feature has been rendered with an offset point
     * symbol, the obstacle geometry should represent the bounds of the offset symbol). If not set,
     * the feature's original geometry will be used as an obstacle for labels.
     * \param symbol feature symbol to label (ownership is not transferred - the symbol must exist until after labeling is complete)
     * \returns a list of the newly generated label features. Ownership of these label features is not transferred
     * (it has already been assigned to the label provider).
     */
    virtual QList< QgsLabelFeature * > registerFeature( const QgsFeature &feature, QgsRenderContext &context, const QgsGeometry &obstacleGeometry = QgsGeometry(), const QgsSymbol *symbol = nullptr );

    /**
     * Returns the geometry for a point feature which should be used as an obstacle for labels. This
     * obstacle geometry will respect the dimensions and offsets of the symbol used to render the
     * point, and ensures that labels will not overlap large or offset points.
     * \param fet point feature
     * \param context render context
     * \param symbols symbols rendered for point feature
     * \since QGIS 2.14
     */
    static QgsGeometry getPointObstacleGeometry( QgsFeature &fet, QgsRenderContext &context, const QgsSymbolList &symbols );

    /**
     * Returns the layer's settings.
     * \since QGIS 3.10
     */
    const QgsPalLayerSettings &settings() const;

    /**
     * Sets fields of this label provider. Normally this is not needed, but when used for vector tiles,
     * fields are not known at the time of creation of label providers. It should be called before
     * a call to prepare() which uses the list of fields.
     * \since QGIS 3.14
     */
    void setFields( const QgsFields &fields ) { mFields = fields; }

  protected:
    //! initialization method - called from constructors
    void init();
    //! Internal label drawing method
    void drawLabelPrivate( pal::LabelPosition *label, QgsRenderContext &context, QgsPalLayerSettings &tmpLyr, QgsTextRenderer::TextPart drawType, double dpiRatio = 1.0 ) const;

  protected:
    //! Layer's labeling configuration
    QgsPalLayerSettings mSettings;
    //! Geometry type of layer
    QgsWkbTypes::GeometryType mLayerGeometryType;

    QgsFeatureRenderer *mRenderer = nullptr;

    // these are needed only if using own renderer loop

    //! Layer's fields
    QgsFields mFields;
    //! Layer's CRS
    QgsCoordinateReferenceSystem mCrs;
    //! Layer's feature source
    std::unique_ptr<QgsAbstractFeatureSource> mSource;

    //! List of generated
    QList<QgsLabelFeature *> mLabels;

  private:

    friend class TestQgsLabelingEngine;
    void drawCallout( QgsRenderContext &context, pal::LabelPosition *label ) const;
};

#endif // QGSVECTORLAYERLABELPROVIDER_H
