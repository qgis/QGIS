/***************************************************************************
    qgsmaphittest.h
    ---------------------
    begin                : September 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMAPHITTEST_H
#define QGSMAPHITTEST_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmapsettings.h"
#include "qgsgeometry.h"
#include "qgstaskmanager.h"
#include "qgscoordinatetransform.h"

#include <QSet>

class QgsRenderContext;
class QgsSymbol;
class QgsVectorLayer;
class QgsExpression;
class QgsAbstractFeatureSource;
class QgsFeatureRenderer;

/**
 * \ingroup core
 * \brief Class that runs a hit test with given map settings. Based on the hit test it returns which symbols
 * will be visible on the map - this is useful for content based legend.
 *
 * \since QGIS 2.6
 */
class CORE_EXPORT QgsMapHitTest
{
  public:
    //! Maps an expression string to a layer id
    typedef QMap<QString, QString> LayerFilterExpression;

    /**
     * \param settings Map settings used to evaluate symbols
     * \param polygon Polygon geometry to refine the hit test
     * \param layerFilterExpression Expression string for each layer id to evaluate in order to refine the symbol selection
     */
    QgsMapHitTest( const QgsMapSettings &settings, const QgsGeometry &polygon = QgsGeometry(), const QgsMapHitTest::LayerFilterExpression &layerFilterExpression = QgsMapHitTest::LayerFilterExpression() );

    //! Constructor version used with only expressions to filter symbols (no extent or polygon intersection)
    QgsMapHitTest( const QgsMapSettings &settings, const QgsMapHitTest::LayerFilterExpression &layerFilterExpression );

    //! Runs the map hit test
    void run();

    /**
     * Returns the hit test results, which are a map of layer ID to
     * visible symbol legend keys.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.32
     */
    QMap<QString, QSet<QString>> results() const SIP_SKIP;

    ///@cond PRIVATE

    /**
     * Returns the hit test results, which are a map of layer ID to
     * visible symbol legend keys.
     *
     * \since QGIS 3.32
     */
    QMap<QString, QList<QString>> resultsPy() const SIP_PYNAME( results );
    ///@endcond PRIVATE

    /**
     * Tests whether a symbol is visible for a specified layer.
     * \param symbol symbol to find
     * \param layer vector layer
     * \see legendKeyVisible()
     * \since QGIS 2.12
     */
    bool symbolVisible( QgsSymbol *symbol, QgsVectorLayer *layer ) const;

    /**
     * Tests whether a given legend key is visible for a specified layer.
     * \param ruleKey legend rule key
     * \param layer vector layer
     * \see symbolVisible()
     * \since QGIS 2.14
     */
    bool legendKeyVisible( const QString &ruleKey, QgsVectorLayer *layer ) const;

  private:

    //! \note not available in Python bindings
    typedef QSet<QString> SymbolSet;

    //! Layer ID to symbol set
    typedef QMap<QString, SymbolSet> HitTest;

    /**
     * Runs test for visible symbols from a feature \a source
     * \param source feature source
     * \param layerId associated layer id
     * \param crs layer crs
     * \param fields layer fields
     * \param renderer layer renderer
     * \param usedSymbols set for storage of visible symbols
     * \param usedSymbolsRuleKey set of storage of visible legend rule keys
     * \param context render context
     * \param feedback optional feedback argument for cancel support
     * \note not available in Python bindings
     */
    void runHitTestFeatureSource( QgsAbstractFeatureSource *source,
                                  const QString &layerId,
                                  const QgsCoordinateReferenceSystem &crs,
                                  const QgsFields &fields,
                                  const QgsFeatureRenderer *renderer,
                                  SymbolSet &usedSymbols,
                                  SymbolSet &usedSymbolsRuleKey,
                                  QgsRenderContext &context,
                                  QgsFeedback *feedback );

    //! The initial map settings
    QgsMapSettings mSettings;

    //! The hit test
    HitTest mHitTest;

    //! The hit test, using legend rule keys
    HitTest mHitTestRuleKey;

    //! List of expression filter for each layer
    QgsMapHitTest::LayerFilterExpression mLayerFilterExpression;

    //! Polygon used for filtering items. May be empty
    QgsGeometry mPolygon;

    //! Whether to use only expressions during the filtering
    bool mOnlyExpressions;

    friend class QgsMapHitTestTask;
};


/**
 * \ingroup core
 * \brief Executes a QgsMapHitTest in a background thread.
 *
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsMapHitTestTask : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsMapHitTestTask, filtering by a visible geometry.
     *
     * \param settings Map settings used to evaluate symbols
     * \param polygon Polygon geometry to refine the hit test
     * \param layerFilterExpression Expression string for each layer id to evaluate in order to refine the symbol selection
     */
    QgsMapHitTestTask( const QgsMapSettings &settings, const QgsGeometry &polygon = QgsGeometry(), const QgsMapHitTest::LayerFilterExpression &layerFilterExpression = QgsMapHitTest::LayerFilterExpression() );

    /**
     * Constructor for QgsMapHitTestTask, filtering by expressions.
     *
     * \param settings Map settings used to evaluate symbols
     * \param layerFilterExpression Expression string for each layer id to evaluate in order to refine the symbol selection
     */
    QgsMapHitTestTask( const QgsMapSettings &settings, const QgsMapHitTest::LayerFilterExpression &layerFilterExpression );

    /**
     * Returns the hit test results, which are a map of layer ID to
     * visible symbol legend keys.
     * \note Not available in Python bindings
     */
    QMap<QString, QSet<QString>> results() const SIP_SKIP;

    ///@cond PRIVATE

    /**
     * Returns the hit test results, which are a map of layer ID to
     * visible symbol legend keys.
     */
    QMap<QString, QList<QString>> resultsPy() const SIP_PYNAME( results );
    ///@endcond PRIVATE

    void cancel() override;

  protected:

    bool run() override;

  private:

    void prepare();

    struct PreparedLayerData
    {
      std::unique_ptr< QgsAbstractFeatureSource > source;
      QString layerId;
      QgsCoordinateReferenceSystem crs;
      QgsFields fields;
      std::unique_ptr< QgsFeatureRenderer > renderer;
      QgsRectangle extent;
      QgsCoordinateTransform transform;
      std::unique_ptr< QgsExpressionContextScope > layerScope;
    };

    std::vector< PreparedLayerData > mPreparedData;

    QgsMapSettings mSettings;
    QgsMapHitTest::LayerFilterExpression mLayerFilterExpression;
    QgsGeometry mPolygon;
    bool mOnlyExpressions = false;
    QMap<QString, QSet<QString>> mResults;

    std::unique_ptr< QgsFeedback > mFeedback;
};

#endif // QGSMAPHITTEST_H
