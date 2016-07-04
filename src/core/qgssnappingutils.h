/***************************************************************************
  qgssnappingutils.h
  --------------------------------------
  Date                 : November 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSNAPPINGUTILS_H
#define QGSSNAPPINGUTILS_H


#include "qgsmapsettings.h"
#include "qgstolerance.h"
#include "qgspointlocator.h"

/** \ingroup core
 * This class has all the configuration of snapping and can return answers to snapping queries.
 * Internally, it keeps a cache of QgsPointLocator instances for multiple layers.
 *
 * Currently it supports the following queries:
 * - snapToMap() - has multiple modes of operation
 * - snapToCurrentLayer()
 * For more complex queries it is possible to use locatorForLayer() method that returns
 * point locator instance with layer's indexed data.
 *
 * Indexing strategy determines how fast the queries will be and how much memory will be used.
 *
 * When working with map canvas, it may be useful to use derived class QgsMapCanvasSnappingUtils
 * which keeps the configuration in sync with map canvas (e.g. current view, active layer).
 *
 * @note added in 2.8
 */
class CORE_EXPORT QgsSnappingUtils : public QObject
{
    Q_OBJECT
  public:
    QgsSnappingUtils( QObject* parent = nullptr );
    ~QgsSnappingUtils();

    // main actions

    /** Get a point locator for the given layer. If such locator does not exist, it will be created */
    QgsPointLocator* locatorForLayer( QgsVectorLayer* vl );

    /** Snap to map according to the current configuration (mode). Optional filter allows discarding unwanted matches. */
    QgsPointLocator::Match snapToMap( QPoint point, QgsPointLocator::MatchFilter* filter = nullptr );
    QgsPointLocator::Match snapToMap( const QgsPoint& pointMap, QgsPointLocator::MatchFilter* filter = nullptr );

    /** Snap to current layer */
    QgsPointLocator::Match snapToCurrentLayer( QPoint point, int type, QgsPointLocator::MatchFilter* filter = nullptr );

    // environment setup

    /** Assign current map settings to the utils - used for conversion between screen coords to map coords */
    void setMapSettings( const QgsMapSettings& settings );
    const QgsMapSettings& mapSettings() const { return mMapSettings; }

    /** Set current layer so that if mode is SnapCurrentLayer we know which layer to use */
    void setCurrentLayer( QgsVectorLayer* layer );
    /** The current layer used if mode is SnapCurrentLayer */
    QgsVectorLayer* currentLayer() const { return mCurrentLayer; }


    // configuration

    //! modes for "snap to background"
    enum SnapToMapMode
    {
      SnapCurrentLayer,    //!< snap just to current layer (tolerance and type from defaultSettings())
      SnapAllLayers,       //!< snap to all rendered layers (tolerance and type from defaultSettings())
      SnapAdvanced,        //!< snap according to the configuration set in setLayers()
    };

    /** Set how the snapping to map is done */
    void setSnapToMapMode( SnapToMapMode mode );
    /** Find out how the snapping to map is done */
    SnapToMapMode snapToMapMode() const { return mSnapToMapMode; }

    enum IndexingStrategy
    {
      IndexAlwaysFull,    //!< For all layers build index of full extent. Uses more memory, but queries are faster.
      IndexNeverFull,     //!< For all layers only create temporary indexes of small extent. Low memory usage, slower queries.
      IndexHybrid         //!< For "big" layers using IndexNeverFull, for the rest IndexAlwaysFull. Compromise between speed and memory usage.
    };

    /** Set a strategy for indexing geometry data - determines how fast and memory consuming the data structures will be */
    void setIndexingStrategy( IndexingStrategy strategy ) { mStrategy = strategy; }
    /** Find out which strategy is used for indexing - by default hybrid indexing is used */
    IndexingStrategy indexingStrategy() const { return mStrategy; }

    /** Configure options used when the mode is snap to current layer or to all layers */
    void setDefaultSettings( int type, double tolerance, QgsTolerance::UnitType unit );
    /** Query options used when the mode is snap to current layer or to all layers */
    void defaultSettings( int& type, double& tolerance, QgsTolerance::UnitType& unit );

    /**
     * Configures how a certain layer should be handled in a snapping operation
     */
    struct LayerConfig
    {
      /**
       * Create a new configuration for a snapping layer.

        ```py
        snapper = QgsMapCanvasSnappingUtils(mapCanvas)

        snapping_layer1 = QgsSnappingUtils.LayerConfig(layer1, QgsPointLocator.Vertex, 10, QgsTolerance.Pixels)
        snapping_layer2 = QgsSnappingUtils.LayerConfig(layer2, QgsPointLocator.Vertex and QgsPointLocator.Edge, 10, QgsTolerance.Pixels)

        snapper.setLayers([snapping_layer1, snapping_layer2])
        snapper.setSnapToMapMode(QgsSnappingUtils.SnapAdvanced)
        ```

       * @param l   The vector layer for which this configuration is
       * @param t   Which parts of the geometry should be snappable
       * @param tol The tolerance radius in which the snapping will trigger
       * @param u   The unit in which the tolerance is specified
       */
      LayerConfig( QgsVectorLayer* l, QgsPointLocator::Types t, double tol, QgsTolerance::UnitType u )
          : layer( l )
          , type( t )
          , tolerance( tol )
          , unit( u )
      {}

      bool operator==( const LayerConfig& other ) const
      {
        return layer == other.layer && type == other.type && tolerance == other.tolerance && unit == other.unit;
      }
      bool operator!=( const LayerConfig& other ) const
      {
        return !operator==( other );
      }

      //! The layer to configure.
      QgsVectorLayer* layer;
      //! To which geometry properties of this layers a snapping should happen.
      QgsPointLocator::Types type;
      //! The range around snapping targets in which snapping should occur.
      double tolerance;
      //! The units in which the tolerance is specified.
      QgsTolerance::UnitType unit;
    };

    /** Set layers which will be used for snapping */
    void setLayers( const QList<LayerConfig>& layers );
    /** Query layers used for snapping */
    QList<LayerConfig> layers() const { return mLayers; }

    /** Set whether to consider intersections of nearby segments for snapping */
    void setSnapOnIntersections( bool enabled );
    /** Query whether to consider intersections of nearby segments for snapping */
    bool snapOnIntersections() const { return mSnapOnIntersection; }

    /** Get extra information about the instance
     * @note added in QGIS 2.14
     */
    QString dump();

  public slots:
    /** Read snapping configuration from the project */
    void readConfigFromProject();

  signals:
    /** Emitted when snapping configuration has been changed
     * @note added in QGIS 2.14
     */
    void configChanged();

  protected:
    //! Called when starting to index - can be overridden and e.g. progress dialog can be provided
    virtual void prepareIndexStarting( int count ) { Q_UNUSED( count ); }
    //! Called when finished indexing a layer. When index == count the indexing is complete
    virtual void prepareIndexProgress( int index ) { Q_UNUSED( index ); }

  private slots:
    void onLayersWillBeRemoved( const QStringList& layerIds );

  private:
    //! get from map settings pointer to destination CRS - or 0 if projections are disabled
    const QgsCoordinateReferenceSystem* destCRS();

    //! delete all existing locators (e.g. when destination CRS has changed and we need to reindex)
    void clearAllLocators();

    //! return a locator (temporary or not) according to the indexing strategy
    QgsPointLocator* locatorForLayerUsingStrategy( QgsVectorLayer* vl, const QgsPoint& pointMap, double tolerance );
    //! return a temporary locator with index only for a small area (will be replaced by another one on next request)
    QgsPointLocator* temporaryLocatorForLayer( QgsVectorLayer* vl, const QgsPoint& pointMap, double tolerance );

    typedef QPair< QgsVectorLayer*, QgsRectangle > LayerAndAreaOfInterest;

    //! find out whether the strategy would index such layer or just use a temporary locator
    bool isIndexPrepared( QgsVectorLayer* vl, const QgsRectangle& areaOfInterest );
    //! initialize index for layers where it makes sense (according to the indexing strategy)
    void prepareIndex( const QList<LayerAndAreaOfInterest>& layers );

  private:
    // environment
    QgsMapSettings mMapSettings;
    QgsVectorLayer* mCurrentLayer;

    // configuration
    SnapToMapMode mSnapToMapMode;
    IndexingStrategy mStrategy;
    int mDefaultType;
    double mDefaultTolerance;
    QgsTolerance::UnitType mDefaultUnit;
    QList<LayerConfig> mLayers;
    bool mSnapOnIntersection;

    // internal data
    typedef QMap<QgsVectorLayer*, QgsPointLocator*> LocatorsMap;
    //! on-demand locators used (locators are owned)
    LocatorsMap mLocators;
    //! temporary locators (indexing just a part of layers). owned by the instance
    LocatorsMap mTemporaryLocators;
    //! list of layer IDs that are too large to be indexed (hybrid strategy will use temporary locators for those)
    QSet<QString> mHybridNonindexableLayers;
    //! a record for each layer seen:
    //! - value -1  == it is small layer -> fully indexed
    //! - value > 0 == maximum area (in map units) for which it may make sense to build index.
    //!   This means that index is built in area around the point with this total area, because
    //!   for a larger area the number of features will likely exceed the limit. When the limit
    //!   is exceeded, the maximum area is lowered to prevent that from happening.
    //!   When requesting snap in area that is not currently indexed, layer's index is destroyed
    //!   and a new one is built in the different area.
    QHash<QString, double> mHybridMaxAreaPerLayer;
    //! if using hybrid strategy, how many features of one layer may be indexed (to limit amount of consumed memory)
    int mHybridPerLayerFeatureLimit;

    //! internal flag that an indexing process is going on. Prevents starting two processes in parallel.
    bool mIsIndexing;
};


#endif // QGSSNAPPINGUTILS_H
