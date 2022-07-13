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

#include "qgslabelingenginesettings.h"
#include "qgslabeling.h"
#include "qgsfeedback.h"
#include "qgslabelobstaclesettings.h"

class QgsLabelingEngine;
class QgsLabelingResults;
class QgsLabelFeature;

namespace pal
{
  class Problem;
  class Pal;
  class LabelPosition;
}

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
      MergeConnectedLines     = 1 << 3,  //!< Whether adjacent lines (with the same label text) should be merged
      CentroidMustBeInside    = 1 << 4,  //!< Whether location of centroid must be inside of polygons
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    //! Returns list of label features (they are owned by the provider and thus deleted on its destruction)
    virtual QList<QgsLabelFeature *> labelFeatures( QgsRenderContext &context ) = 0;

    /**
     * Draw this label at the position determined by the labeling engine.
     *
     * Before any calls to drawLabel(), a provider should be prepared for rendering by a call to
     * startRender() and a corresponding call to stopRender().
     */
    virtual void drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const = 0;

    /**
     * Draw an unplaced label. These correspond to features which were registered for labeling,
     * but which could not be labeled (e.g. due to conflicting labels).
     *
     * The default behavior is to draw nothing for these labels.
     *
     * \note This method is only used if the QgsLabelingEngineSettings::DrawUnplacedLabels flag
     * is set on the labeling engine.
     *
     * \since QGIS 3.10
     */
    virtual void drawUnplacedLabel( QgsRenderContext &context, pal::LabelPosition *label ) const;

    /**
     * Draw the background for the specified \a label.
     *
     * This is called in turn for each label provider before any actual labels are rendered,
     * and allows the provider to render content which should be drawn below ALL map labels
     * (such as background rectangles or callout lines).
     *
     * Before any calls to drawLabelBackground(), a provider should be prepared for rendering by a call to
     * startRender() and a corresponding call to stopRender().
     *
     * \since QGIS 3.10
     */
    virtual void drawLabelBackground( QgsRenderContext &context, pal::LabelPosition *label ) const;

    /**
     * To be called before rendering of labels begins. Must be accompanied by
     * a corresponding call to stopRender()
     * \since QGIS 3.10
     */
    virtual void startRender( QgsRenderContext &context );

    /**
     * To be called after rendering is complete.
     * \see startRender()
     * \since QGIS 3.10
     */
    virtual void stopRender( QgsRenderContext &context );

    //! Returns list of child providers - useful if the provider needs to put labels into more layers with different configuration
    virtual QList<QgsAbstractLabelProvider *> subProviders() { return QList<QgsAbstractLabelProvider *>(); }

    //! Name of the layer (for statistics, debugging etc.) - does not need to be unique
    QString name() const { return mName; }

    //! Returns ID of associated layer, or empty string if no layer is associated with the provider.
    QString layerId() const { return mLayerId; }

    /**
     * Returns the associated layer, or NULLPTR if no layer is associated with the provider.
     *
     * \warning Accessing the layer is not thread safe, and this should never be called while the labeling engine is running from a background thread!
     */
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
    Qgis::LabelPlacement placement() const { return mPlacement; }

    //! Default priority of labels (may be overridden by individual labels)
    double priority() const { return mPriority; }

    //! How the feature geometries will work as obstacles
    QgsLabelObstacleSettings::ObstacleType obstacleType() const { return mObstacleType; }

    //! How to handle labels that would be upside down
    Qgis::UpsideDownLabelHandling upsidedownLabels() const { return mUpsidedownLabels; }

    /**
     * Returns the expression context scope created from the layer associated with this provider.
     *
     * \since QGIS 3.22
     */
    QgsExpressionContextScope *layerExpressionContextScope() const;

    /**
     * Returns the symbology reference scale of the layer associated with this provider.
     *
     * \since QGIS 3.22
     */
    double layerReferenceScale() const { return mLayerReferenceScale; }

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
    Flags mFlags = DrawLabels;
    //! Placement strategy
    Qgis::LabelPlacement mPlacement = Qgis::LabelPlacement::AroundPoint;
    //! Default priority of labels
    double mPriority = 0.5;
    //! Type of the obstacle of feature geometries
    QgsLabelObstacleSettings::ObstacleType mObstacleType = QgsLabelObstacleSettings::PolygonBoundary;
    //! How to handle labels that would be upside down
    Qgis::UpsideDownLabelHandling mUpsidedownLabels = Qgis::UpsideDownLabelHandling::FlipUpsideDownLabels;

  private:

    std::unique_ptr< QgsExpressionContextScope > mLayerExpressionContextScope;
    double mLayerReferenceScale = -1;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsAbstractLabelProvider::Flags )


/**
 * \ingroup core
 * \brief QgsFeedback subclass for granular reporting of labeling engine progress.
 * \note not available in Python bindings
 * \since QGIS 3.24
 */
class CORE_EXPORT QgsLabelingEngineFeedback : public QgsFeedback
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLabelingEngineFeedback, with the specified \a parent object.
     */
    QgsLabelingEngineFeedback( QObject *parent SIP_TRANSFERTHIS = nullptr )
      : QgsFeedback( parent )
    {}

  signals:

    /**
     * Emitted when the label registration is about to begin.
     */
    void labelRegistrationAboutToBegin();

    /**
     * Emitted when the label registration has completed for all providers.
     */
    void labelRegistrationFinished();

    /**
     * Emitted when the label registration is about to begin for a \a provider.
     */
    void providerRegistrationAboutToBegin( QgsAbstractLabelProvider *provider );

    /**
     * Emitted when the label registration has completed for a \a provider.
     */
    void providerRegistrationFinished( QgsAbstractLabelProvider *provider );

    /**
     * Emitted when the label candidate creation is about to begin for a \a provider.
     */
    void candidateCreationAboutToBegin( QgsAbstractLabelProvider *provider );

    /**
     * Emitted when the label candidate creation has completed for a \a provider.
     */
    void candidateCreationFinished( QgsAbstractLabelProvider *provider );

    /**
     * Emitted when the obstacle costing is about to begin.
     */
    void obstacleCostingAboutToBegin();

    /**
     * Emitted when the obstacle costing has completed.
     */
    void obstacleCostingFinished();

    /**
     * Emitted when the conflict handling step is about to begin.
     */
    void calculatingConflictsAboutToBegin();

    /**
     * Emitted when the conflict handling step has completed.
     */
    void calculatingConflictsFinished();

    /**
     * Emitted when the label candidates are about to be finalized.
     */
    void finalizingCandidatesAboutToBegin();

    /**
     * Emitted when the label candidates are finalized.
     */
    void finalizingCandidatesFinished();

    /**
     * Emitted when the candidate reduction step is about to begin.
     */
    void reductionAboutToBegin();

    /**
     * Emitted when the candidate reduction step is finished.
     */
    void reductionFinished();

    /**
     * Emitted when the problem solving step is about to begin.
     */
    void solvingPlacementAboutToBegin();

    /**
     * Emitted when the problem solving step is finished.
     */
    void solvingPlacementFinished();
};

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
    virtual ~QgsLabelingEngine();

    //! QgsLabelingEngine cannot be copied.
    QgsLabelingEngine( const QgsLabelingEngine &rh ) = delete;
    //! QgsLabelingEngine cannot be copied.
    QgsLabelingEngine &operator=( const QgsLabelingEngine &rh ) = delete;

    //! Associate map settings instance
    void setMapSettings( const QgsMapSettings &mapSettings );
    //! Gets associated map settings
    const QgsMapSettings &mapSettings() const { return mMapSettings; }

    //! Gets associated labeling engine settings
    const QgsLabelingEngineSettings &engineSettings() const { return mMapSettings.labelingEngineSettings(); }

    /**
     * Returns a list of layers with providers in the engine.
     * \since QGIS 3.0
     */
    QList< QgsMapLayer * > participatingLayers() const;

    /**
     * Returns a list of layer IDs for layers with providers in the engine.
     * \since QGIS 3.10
     */
    QStringList participatingLayerIds() const;

    //! Add provider of label features. Takes ownership of the provider
    void addProvider( QgsAbstractLabelProvider *provider );

    //! Remove provider if the provider's initialization failed. Provider instance is deleted.
    void removeProvider( QgsAbstractLabelProvider *provider );

    /**
     * Runs the labeling job.
     *
     * Depending on the concrete labeling engine class, this will either run the whole
     * labeling job, including rendering the labels themselves, OR possibly just run the labeling
     * job but leave the rendering to a future, deferred stage.
     */
    virtual void run( QgsRenderContext &context ) = 0;

    //! Returns pointer to recently computed results and pass the ownership of results to the caller
    QgsLabelingResults *takeResults();

    //! For internal use by the providers
    QgsLabelingResults *results() const { return mResults.get(); }

  protected:
    void processProvider( QgsAbstractLabelProvider *provider, QgsRenderContext &context, pal::Pal &p );

  protected:

    /**
     * Runs the label registration step.
     *
     * Must be called by subclasses prior to solve() and drawLabels()
     * \since QGIS 3.10
     */
    void registerLabels( QgsRenderContext &context );

    /**
     * Solves the label problem.
     *
     * Must be called by subclasses prior to drawLabels(), and must be
     * preceded by a call to registerLabels()
     *
     * \since QGIS 3.10
     */
    void solve( QgsRenderContext &context );

    /**
     * Draws labels to the specified render \a context.
     *
     * If \a layerId is specified, only labels from the matching layer will
     * be rendered.
     *
     * Must be preceded by a call to registerLabels() and solve()
     *
     * \since QGIS 3.10
     */
    void drawLabels( QgsRenderContext &context, const QString &layerId = QString() );

    /**
     * Cleans up the engine following a call to registerLabels() or solve().
     * \since QGIS 3.10
     */
    void cleanup();

    //! Associated map settings instance
    QgsMapSettings mMapSettings;

    //! List of providers (the are owned by the labeling engine)
    QList<QgsAbstractLabelProvider *> mProviders;
    QList<QgsAbstractLabelProvider *> mSubProviders;

    //! Resulting labeling layout
    std::unique_ptr< QgsLabelingResults > mResults;

    std::unique_ptr< pal::Pal > mPal;
    std::unique_ptr< pal::Problem > mProblem;
    QList<pal::LabelPosition *> mUnlabeled;
    QList<pal::LabelPosition *> mLabels;

};

/**
 * \ingroup core
 * \class QgsDefaultLabelingEngine
 * \brief Default QgsLabelingEngine implementation, which completes the whole
 * labeling operation (including label rendering) in the run() method.
 * \note this class is not a part of public API yet. See notes in QgsLabelingEngine
 * \note not available in Python bindings
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsDefaultLabelingEngine : public QgsLabelingEngine
{
  public:
    //! Construct the labeling engine with default settings
    QgsDefaultLabelingEngine();

    //! QgsDefaultLabelingEngine cannot be copied.
    QgsDefaultLabelingEngine( const QgsDefaultLabelingEngine &rh ) = delete;
    //! QgsDefaultLabelingEngine cannot be copied.
    QgsDefaultLabelingEngine &operator=( const QgsDefaultLabelingEngine &rh ) = delete;

    void run( QgsRenderContext &context ) override;

};

/**
 * \ingroup core
 * \class QgsStagedRenderLabelingEngine
 * \brief A QgsLabelingEngine implementation, which only calculates
 * the labeling solution during its run() method. The actual rendering
 * of labels themselves is deferred to follow up calls to ....
 *
 * \note this class is not a part of public API yet. See notes in QgsLabelingEngine
 * \note not available in Python bindings
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsStagedRenderLabelingEngine : public QgsLabelingEngine
{
  public:
    //! Construct the labeling engine with default settings
    QgsStagedRenderLabelingEngine();

    //! QgsStagedRenderLabelingEngine cannot be copied.
    QgsStagedRenderLabelingEngine( const QgsStagedRenderLabelingEngine &rh ) = delete;
    //! QgsStagedRenderLabelingEngine cannot be copied.
    QgsStagedRenderLabelingEngine &operator=( const QgsStagedRenderLabelingEngine &rh ) = delete;

    void run( QgsRenderContext &context ) override;

    /**
     * Renders all the labels which belong only to the layer with matching \a layerId
     * to the specified render \a context.
     */
    void renderLabelsForLayer( QgsRenderContext &context, const QString &layerId );

    /**
     * Finalizes and cleans up the engine following the rendering of labels for the last layer
     * to be labeled (via renderLabelsForLayer() ).
     */
    void finalize();
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
    static QString encodePredefinedPositionOrder( const QVector< Qgis::LabelPredefinedPointPosition > &positions );

    /**
     * Decodes a string to an ordered list of predefined point label positions.
     * \param positionString encoded string of positions
     * \returns decoded list
     * \see encodePredefinedPositionOrder()
     */
    static QVector< Qgis::LabelPredefinedPointPosition > decodePredefinedPositionOrder( const QString &positionString );

    /**
     * Encodes line placement \a flags to a string.
     * \see decodeLinePlacementFlags()
     */
    static QString encodeLinePlacementFlags( QgsLabeling::LinePlacementFlags flags );

    /**
     * Decodes a \a string to set of line placement flags.
     * \see encodeLinePlacementFlags()
     */
    static QgsLabeling::LinePlacementFlags decodeLinePlacementFlags( const QString &string );

};

#endif // QGSLABELINGENGINE_H
