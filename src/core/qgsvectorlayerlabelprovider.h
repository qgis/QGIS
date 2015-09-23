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

class QgsAbstractFeatureSource;

/**
 * @brief The QgsVectorLayerLabelProvider class implements a label provider
 * for vector layers. Parameters for the labeling are taken from the layer's
 * custom properties or from the given settings.
 *
 * @note added in QGIS 2.12
 */
class CORE_EXPORT QgsVectorLayerLabelProvider : public QgsAbstractLabelProvider
{
  public:

    //! Convenience constructor to initialize the provider from given vector layer
    explicit QgsVectorLayerLabelProvider( QgsVectorLayer* layer, bool withFeatureLoop = true, const QgsPalLayerSettings* settings = 0, const QString& layerName = QString() );

    //! Construct diagram provider with all the necessary configuration parameters
    QgsVectorLayerLabelProvider( const QgsPalLayerSettings& settings,
                                 const QString& layerId,
                                 const QgsFields& fields,
                                 const QgsCoordinateReferenceSystem& crs,
                                 QgsAbstractFeatureSource* source,
                                 bool ownsSource );

    ~QgsVectorLayerLabelProvider();

    virtual QList<QgsLabelFeature*> labelFeatures( const QgsRenderContext& context ) override;

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
     */
    virtual void registerFeature( QgsFeature& feature, const QgsRenderContext& context );

  protected:
    //! initialization method - called from constructors
    void init();
    //! Internal label drawing method
    void drawLabelPrivate( pal::LabelPosition* label, QgsRenderContext& context, QgsPalLayerSettings& tmpLyr, QgsPalLabeling::DrawLabelType drawType, double dpiRatio = 1.0 ) const;

  protected:
    //! Layer's labeling configuration
    QgsPalLayerSettings mSettings;
    //! Layer's ID
    QString mLayerId;

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
