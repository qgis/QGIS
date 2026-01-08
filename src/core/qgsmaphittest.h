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

#include <utility>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgscoordinatetransform.h"
#include "qgsgeometry.h"
#include "qgslayertreefiltersettings.h"
#include "qgsmapsettings.h"
#include "qgsmeshlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterminmaxorigin.h"
#include "qgstaskmanager.h"

#include <QSet>

class QgsRenderContext;
class QgsSymbol;
class QgsVectorLayer;
class QgsExpression;
class QgsAbstractFeatureSource;
class QgsFeatureRenderer;
class QgsLayerTreeFilterSettings;

/**
 * \ingroup core
 * \brief Runs a hit test with given map settings.
 *
 * Based on the hit test it returns which symbols will be visible
 * on the map - this is useful for content based legend.
 */
class CORE_EXPORT QgsMapHitTest
{
  public:
    //! Maps an expression string to a layer id
    typedef QMap<QString, QString> LayerFilterExpression;

    /**
     * Constructor for QgsMapHitTest.
     *
     * \param settings Map settings used to evaluate symbols
     * \param polygon Polygon geometry to refine the hit test
     * \param layerFilterExpression Expression string for each layer id to evaluate in order to refine the symbol selection
     */
    QgsMapHitTest( const QgsMapSettings &settings, const QgsGeometry &polygon = QgsGeometry(), const QgsMapHitTest::LayerFilterExpression &layerFilterExpression = QgsMapHitTest::LayerFilterExpression() );

    //! Constructor version used with only expressions to filter symbols (no extent or polygon intersection)
    QgsMapHitTest( const QgsMapSettings &settings, const QgsMapHitTest::LayerFilterExpression &layerFilterExpression );

    /**
     * Constructor based off layer tree filter \a settings.
     *
     * \since QGIS 3.32
     */
    QgsMapHitTest( const QgsLayerTreeFilterSettings &settings );

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
     * Returns the hit test results, for layers with UpdatedCanvas renderers (raster/mesh layers).
     * Results are given as QMap of layer IDs to pairs of (min, max) values.
     *
     * \since QGIS 4.0
     */
    QMap<QString, std::pair<double, double>> resultsRenderersUpdatedCanvas() const SIP_SKIP { return mHitTestRenderersUpdatedCanvas; }


#ifdef SIP_RUN
    /**
     * Returns the hit test results, for layers with UpdatedCanvas renderers (raster/mesh layers).
     * Results are given as QMap of layer IDs to pairs of (min, max) values.
     *
     * \since QGIS 4.0
     */
    SIP_PYOBJECT resultsRenderersUpdatedCanvasPy() const SIP_PYNAME( resultsRenderersUpdatedCanvas );
    % MethodCode
    QMap<QString, std::pair<double, double>> results = sipCpp->resultsRenderersUpdatedCanvas();
    sipRes = PyDict_New();

    for ( auto it = results.constBegin(); it != results.constEnd(); ++it )
    {
      PyObject *tuple = PyTuple_New( 2 );
      PyTuple_SET_ITEM( tuple, 0, PyFloat_FromDouble( it.value().first ) );
      PyTuple_SET_ITEM( tuple, 1, PyFloat_FromDouble( it.value().second ) );
      PyDict_SetItem( sipRes, PyUnicode_FromString( it.key().toUtf8().constData() ), tuple );
      Py_DECREF( tuple );
    }
    % End
#endif

    /**
     * Tests whether a symbol is visible for a specified layer.
     * \param symbol symbol to find
     * \param layer vector layer
     * \see legendKeyVisible()
     */
    bool symbolVisible( QgsSymbol *symbol, QgsVectorLayer *layer ) const;

    /**
     * Tests whether a given legend key is visible for a specified layer.
     * \param ruleKey legend rule key
     * \param layer vector layer
     * \see symbolVisible()
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
     * \param fields layer fields
     * \param renderer layer renderer
     * \param usedSymbols set for storage of visible symbols
     * \param usedSymbolsRuleKey set of storage of visible legend rule keys
     * \param context render context
     * \param feedback optional feedback argument for cancel support
     * \param visibleExtent total visible area of layer
     * \note not available in Python bindings
     */
    void runHitTestFeatureSource( QgsAbstractFeatureSource *source,
                                  const QString &layerId,
                                  const QgsFields &fields,
                                  const QgsFeatureRenderer *renderer,
                                  SymbolSet &usedSymbols,
                                  SymbolSet &usedSymbolsRuleKey,
                                  QgsRenderContext &context,
                                  QgsFeedback *feedback,
                                  const QgsGeometry &visibleExtent );

    /**
     * Runs test for minimum and maximum value for a raster data provider
     * \param provider raster data provider
     * \param layerId associated layer id
     * \param band raster band to calculate min/max on
     * \param minMaxOrigin min/max origin settings
     * \param rangeLimit range limit settings
     * \param transform coordinate transform to map CRS
     * \param context render context
     * \param feedback optional feedback argument for cancel support
     * \note not available in Python bindings
     */
    void runHitTestRasterSource( QgsRasterDataProvider *provider,
                                 const QString &layerId,
                                 const int band,
                                 const QgsRasterMinMaxOrigin minMaxOrigin,
                                 const Qgis::RasterRangeLimit rangeLimit,
                                 const QgsCoordinateTransform &transform,
                                 QgsRenderContext &context,
                                 QgsFeedback *feedback );

    /**
     * Runs test for visible symbols from a mesh layer
     * \param layer mesh layer
     * \param layerId associated layer id
     * \param datasetIndex dataset index within the mesh layer
     * \param transform coordinate transform to map CRS
     * \param context render context
     * \param feedback optional feedback argument for cancel support
     * \note not available in Python bindings
     */
    void runHitTestMeshSource( QgsMeshLayer *layer,
                               const QString &layerId,
                               const QgsMeshDatasetIndex datasetIndex,
                               const QgsCoordinateTransform &transform,
                               QgsRenderContext &context,
                               QgsFeedback *feedback );
    //! The hit test
    HitTest mHitTest;

    //! The hit test, using legend rule keys
    HitTest mHitTestRuleKey;

    QgsLayerTreeFilterSettings mSettings;

    QMap<QString, std::pair<double, double>> mHitTestRenderersUpdatedCanvas;

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
     * Constructor for QgsMapHitTestTask, using the specified filter \a settings.
     */
    QgsMapHitTestTask( const QgsLayerTreeFilterSettings &settings );

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

    /**
     * Returns the hit test results, for layers with UpdatedCanvas renderers (raster/mesh layers).
     * Results are given as QMap of layer IDs to pairs of (min, max) values.
     *
     * \since QGIS 4.0
     */
    QMap<QString, QPair<double, double>> resultsRenderersUpdatedCanvas() const SIP_SKIP { return mResultsRenderersUpdatedCanvas; };

#ifdef SIP_RUN
    /**
     * Returns the hit test results, for layers with UpdatedCanvas renderers (raster/mesh layers).
     * Results are given as QMap of layer IDs to pairs of (min, max) values.
     *
     * \since QGIS 4.0
     */
    SIP_PYOBJECT resultsRenderersUpdatedCanvasPy() const SIP_PYNAME( resultsRenderersUpdatedCanvas );
    % MethodCode
    QMap<QString, std::pair<double, double>> results = sipCpp->resultsRenderersUpdatedCanvas();
    sipRes = PyDict_New();

    for ( auto it = results.constBegin(); it != results.constEnd(); ++it )
    {
      PyObject *tuple = PyTuple_New( 2 );
      PyTuple_SET_ITEM( tuple, 0, PyFloat_FromDouble( it.value().first ) );
      PyTuple_SET_ITEM( tuple, 1, PyFloat_FromDouble( it.value().second ) );
      PyDict_SetItem( sipRes, PyUnicode_FromString( it.key().toUtf8().constData() ), tuple );
      Py_DECREF( tuple );
    }
    % End
#endif

  protected:

    bool run() override;

  private:

    void prepare();

    struct PreparedLayerData
    {
      std::unique_ptr< QgsAbstractFeatureSource > source;
      QString layerId;
      QgsFields fields;
      std::unique_ptr< QgsFeatureRenderer > renderer;
      QgsGeometry extent;
      QgsCoordinateTransform transform;
      std::unique_ptr< QgsExpressionContextScope > layerScope;
    };

    struct PreparedRasterData
    {
      std::unique_ptr< QgsRasterDataProvider > provider;
      QString layerId;
      int band;
      QgsRasterMinMaxOrigin minMaxOrigin;
      Qgis::RasterRangeLimit rangeLimit;
      QgsCoordinateTransform transform;
    };

    struct PreparedMeshData
    {
      QString layerId;
      QString name;
      QString source;
      QStringList extraDatasetUris;
      QgsMeshRendererSettings rendererSettings;
      QString providerKey;
      QgsCoordinateReferenceSystem crs;
      QgsMeshDatasetIndex datasetIndex;
      QgsCoordinateTransform transform;
    };

    std::vector< PreparedLayerData > mPreparedData;

    std::vector< PreparedRasterData > mPreparedRasterData;

    std::vector< PreparedMeshData > mPreparedMeshData;

    QgsLayerTreeFilterSettings mSettings;

    QMap<QString, QSet<QString>> mResults;

    QMap<QString, QPair<double, double>> mResultsRenderersUpdatedCanvas;

    std::unique_ptr< QgsFeedback > mFeedback;
};

#endif // QGSMAPHITTEST_H
