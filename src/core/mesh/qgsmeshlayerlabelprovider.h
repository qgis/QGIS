/***************************************************************************
                         qgsmeshlayerlabelprovider.h
                         ---------------------
    begin                : November 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHLAYERLABELPROVIDER_H
#define QGSMESHLAYERLABELPROVIDER_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgslabelingengine.h"
#include "qgsrenderer.h"
#include "qgspallabeling.h"

class QgsMeshLayer;

/**
 * \ingroup core
 * \brief The QgsMeshLayerLabelProvider class implements a label provider
 * for mesh layers. Parameters for the labeling are taken from the layer's
 * custom properties or from the given settings.
 *
 * \note this class is not a part of public API yet. See notes in QgsLabelingEngine
 * \note not available in Python bindings
 * \since QGIS 3.36
 */
class CORE_EXPORT QgsMeshLayerLabelProvider : public QgsAbstractLabelProvider
{
  public:

    //! Convenience constructor to initialize the provider from given mesh layer
    explicit QgsMeshLayerLabelProvider( QgsMeshLayer *layer,
                                        const QString &providerId,
                                        const QgsPalLayerSettings *settings,
                                        const QString &layerName = QString(),
                                        bool labelFaces = false );

    ~QgsMeshLayerLabelProvider() override;

    QList<QgsLabelFeature *> labelFeatures( QgsRenderContext &context ) override;

    //void drawLabelBackground( QgsRenderContext &context, pal::LabelPosition *label ) const override;
    void drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const override;
    //void drawUnplacedLabel( QgsRenderContext &context, pal::LabelPosition *label ) const override;
    void startRender( QgsRenderContext &context ) override;
    void stopRender( QgsRenderContext &context ) override;

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
     * Returns the layer's settings.
     */
    const QgsPalLayerSettings &settings() const;

    //! Returns FALSE if labeling mesh vertices, TRUE if labeling mesh faces
    bool labelFaces() const { return mLabelFaces; }

  protected:
    //! initialization method - called from constructors
    void init();
    //! Internal label drawing method
    void drawLabelPrivate( pal::LabelPosition *label, QgsRenderContext &context, QgsPalLayerSettings &tmpLyr, Qgis::TextComponent drawType, double dpiRatio = 1.0 ) const;

  protected:
    //! Layer's labeling configuration
    QgsPalLayerSettings mSettings;

    bool mLabelFaces = false;

    //! Layer's CRS
    QgsCoordinateReferenceSystem mCrs;

    //! List of generated
    QList<QgsLabelFeature *> mLabels;

  private:
    std::unique_ptr<QgsVectorLayerLabelProvider> mVectorLabelProvider;

    friend class TestQgsLabelingEngine;
};

#endif // QGSMESHLAYERLABELPROVIDER_H
