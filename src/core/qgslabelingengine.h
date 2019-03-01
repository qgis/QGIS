/***************************************************************************
  qgslabelingengine.h
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

#ifndef QGSLABELINGENGINE_H
#define QGSLABELINGENGINE_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgsmapsettings.h"

#include "qgspallabeling.h"
#include "qgslabelingenginesettings.h"


class QgsLabelingEngine;


/**
 * \ingroup core
 * \brief The QgsAbstractLabelProvider class is an interface class. Implementations
 * return list of labels and their associated geometries - these are used by
 * QgsLabelingEngine to compute the final layout of labels.
 *
 * Implementations also take care of drawing the returned final label positions.
 *
 * \note this class is not a part of public API yet. See notes in QgsLabelingEngine
 * \note not available in Python bindings
 * \since QGIS 2.12
 */
class CORE_EXPORT QgsAbstractLabelProvider
{

  public:
    //! Construct the provider with default values
    QgsAbstractLabelProvider( QgsMapLayer *layer, const QString &providerId = QString() );

    virtual ~QgsAbstractLabelProvider() = default;

    //! Associate provider with a labeling engine (should be only called internally from QgsLabelingEngine)
    void setEngine( const QgsLabelingEngine *engine ) { mEngine = engine; }

    enum Flag
    {
      DrawLabels              = 1 << 1,  //!< Whether the labels should be rendered
      DrawAllLabels           = 1 << 2,  //!< Whether all features will be labelled even though overlaps occur
      MergeConnectedLines     = 1 << 3,  //!< Whether adjacent lines (with the same label text) should be merged
      CentroidMustBeInside    = 1 << 4,  //!< Whether location of centroid must be inside of polygons
      LabelPerFeaturePart     = 1 << 6,  //!< Whether to label each part of multi-part features separately
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    //! Returns list of label features (they are owned by the provider and thus deleted on its destruction)
    virtual QList<QgsLabelFeature *> labelFeatures( QgsRenderContext &context ) = 0;

    //! draw this label at the position determined by the labeling engine
    virtual void drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const = 0;

    //! Returns list of child providers - useful if the provider needs to put labels into more layers with different configuration
    virtual QList<QgsAbstractLabelProvider *> subProviders() { return QList<QgsAbstractLabelProvider *>(); }

    //! Name of the layer (for statistics, debugging etc.) - does not need to be unique
    QString name() const { return mName; }

    //! Returns ID of associated layer, or empty string if no layer is associated with the provider.
    QString layerId() const { return mLayerId; }

    //! Returns the associated layer, or NULLPTR if no layer is associated with the provider.
    QgsMapLayer *layer() const { return mLayer.data(); }

    /**
     * Returns provider ID - useful in case there is more than one label provider within a layer
     * (e.g. in case of rule-based labeling - provider ID = rule's key). May be empty string if
     * layer ID is sufficient for identification of provider's configuration.
     */
    QString providerId() const { return mProviderId; }

    //! Flags associated with the provider
    Flags flags() const { return mFlags; }

    //! What placement strategy to use for the labels
    QgsPalLayerSettings::Placement placement() const { return mPlacement; }

    //! For layers with linestring geometries - extra placement flags (or-ed combination of QgsPalLayerSettings::LinePlacementFlags)
    unsigned int linePlacementFlags() const { return mLinePlacementFlags; }

    //! Default priority of labels (may be overridden by individual labels)
    double priority() const { return mPriority; }

    //! How the feature geometries will work as obstacles
    QgsPalLayerSettings::ObstacleType obstacleType() const { return mObstacleType; }

    //! How to handle labels that would be upside down
    QgsPalLayerSettings::UpsideDownLabels upsidedownLabels() const { return mUpsidedownLabels; }

  protected:
    //! Associated labeling engine
    const QgsLabelingEngine *mEngine = nullptr;

    //! Name of the layer
    QString mName;
    //! Associated layer's ID, if applicable
    QString mLayerId;
    //! Weak pointer to source layer
    QgsWeakMapLayerPointer mLayer;
    //! Associated provider ID (one layer may have multiple providers, e.g. in rule-based labeling)
    QString mProviderId;
    //! Flags altering drawing and registration of features
    Flags mFlags;
    //! Placement strategy
    QgsPalLayerSettings::Placement mPlacement;
    //! Extra placement flags for linestring geometries
    unsigned int mLinePlacementFlags;
    //! Default priority of labels
    double mPriority;
    //! Type of the obstacle of feature geometries
    QgsPalLayerSettings::ObstacleType mObstacleType;
    //! How to handle labels that would be upside down
    QgsPalLayerSettings::UpsideDownLabels mUpsidedownLabels;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsAbstractLabelProvider::Flags )


/**
 * \ingroup core
 * \brief The QgsLabelingEngine class provides map labeling functionality.
 * The input for the engine is a list of label provider objects and map settings.
 * Based on the input, the engine computes layout of labels for the given map view
 * with no collisions between the labels. Drawing of resulting labels is done
 * again by label providers.
 *
 * The labeling engine is used for the map rendering in QgsMapRendererJob instances,
 * individual map layer renderers may add label providers - for example,
 * QgsVectorLayerRenderer may add text label provider and diagram provider
 * (if labeling / diagrams were configured for such vector layer).
 *
 * The labeling engine may also be used independently from map rendering loop:
 * \code{.cpp}
 *   QgsLabelingEngine engine;
 *   engine.setMapSettings( mapSettings );
 *   // add one or more providers
 *   engine.addProvider( ... );
 *   // compute the labeling and draw labels (using painter from the context)
 *   engine.run( context );
 * \endcode
 *
 * \note this class is not a part of public API yet. The provider's interface still
 * uses pal::LabelPosition as an argument in drawLabels() method - this should be
 * sorted out first (a class common to API and pal?). Also, the API may need more
 * polishing to be easy to use - e.g. use concept of labeling layers in API
 * (equivalent of pal::Layer) instead of subProviders(), label providers integrated
 * into feature loop vs providers with independent feature loop), split labeling
 * computation from drawing of labels, improved results class with label iterator).
 * \note not available in Python bindings
 * \since QGIS 2.12
 */
class CORE_EXPORT QgsLabelingEngine
{
  public:
    //! Construct the labeling engine with default settings
    QgsLabelingEngine();
    //! Clean up everything (especially the registered providers)
    ~QgsLabelingEngine();

    //! QgsLabelingEngine cannot be copied.
    QgsLabelingEngine( const QgsLabelingEngine &rh ) = delete;
    //! QgsLabelingEngine cannot be copied.
    QgsLabelingEngine &operator=( const QgsLabelingEngine &rh ) = delete;

    //! Associate map settings instance
    void setMapSettings( const QgsMapSettings &mapSettings ) { mMapSettings = mapSettings; }
    //! Gets associated map settings
    const QgsMapSettings &mapSettings() const { return mMapSettings; }

    //! Gets associated labeling engine settings
    const QgsLabelingEngineSettings &engineSettings() const { return mMapSettings.labelingEngineSettings(); }

    /**
     * Returns a list of layers with providers in the engine.
     * \since QGIS 3.0
     */
    QList< QgsMapLayer * > participatingLayers() const;

    //! Add provider of label features. Takes ownership of the provider
    void addProvider( QgsAbstractLabelProvider *provider );

    //! Remove provider if the provider's initialization failed. Provider instance is deleted.
    void removeProvider( QgsAbstractLabelProvider *provider );

    //! compute the labeling with given map settings and providers
    void run( QgsRenderContext &context );

    //! Returns pointer to recently computed results and pass the ownership of results to the caller
    QgsLabelingResults *takeResults();

    //! For internal use by the providers
    QgsLabelingResults *results() const { return mResults.get(); }

  protected:
    void processProvider( QgsAbstractLabelProvider *provider, QgsRenderContext &context, pal::Pal &p );

  protected:
    //! Associated map settings instance
    QgsMapSettings mMapSettings;

    //! List of providers (the are owned by the labeling engine)
    QList<QgsAbstractLabelProvider *> mProviders;
    QList<QgsAbstractLabelProvider *> mSubProviders;

    //! Resulting labeling layout
    std::unique_ptr< QgsLabelingResults > mResults;

};


/**
 * \ingroup core
 * \class QgsLabelingUtils
 * \brief Contains helper utilities for working with QGIS' labeling engine.
 * \note this class is not a part of public API yet. See notes in QgsLabelingEngine
 * \note not available in Python bindings
 * \since QGIS 2.14
 */

class CORE_EXPORT QgsLabelingUtils
{
  public:

    /**
     * Encodes an ordered list of predefined point label positions to a string.
     * \param positions order list of positions
     * \returns list encoded to string
     * \see decodePredefinedPositionOrder()
     */
    static QString encodePredefinedPositionOrder( const QVector< QgsPalLayerSettings::PredefinedPointPosition > &positions );

    /**
     * Decodes a string to an ordered list of predefined point label positions.
     * \param positionString encoded string of positions
     * \returns decoded list
     * \see encodePredefinedPositionOrder()
     */
    static QVector< QgsPalLayerSettings::PredefinedPointPosition > decodePredefinedPositionOrder( const QString &positionString );

};

#endif // QGSLABELINGENGINE_H
