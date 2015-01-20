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

/**
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
    QgsSnappingUtils( QObject* parent = 0 );
    ~QgsSnappingUtils();

    // main actions

    /** get a point locator for the given layer. If such locator does not exist, it will be created */
    QgsPointLocator* locatorForLayer( QgsVectorLayer* vl );

    /** snap to map according to the current configuration (mode). Optional filter allows to discard unwanted matches. */
    QgsPointLocator::Match snapToMap( const QPoint& point, QgsPointLocator::MatchFilter* filter = 0 );
    QgsPointLocator::Match snapToMap( const QgsPoint& pointMap, QgsPointLocator::MatchFilter* filter = 0 );

    /** snap to current layer */
    QgsPointLocator::Match snapToCurrentLayer( const QPoint& point, int type, QgsPointLocator::MatchFilter* filter = 0 );

    // environment setup

    /** assign current map settings to the utils - used for conversion between screen coords to map coords */
    void setMapSettings( const QgsMapSettings& settings );
    const QgsMapSettings& mapSettings() const { return mMapSettings; }

    /** set current layer so that if mode is SnapCurrentLayer we know which layer to use */
    void setCurrentLayer( QgsVectorLayer* layer ) { mCurrentLayer = layer; }
    QgsVectorLayer* currentLayer() const { return mCurrentLayer; }


    // configuration

    //! modes for "snap to background"
    enum SnapToMapMode
    {
      SnapCurrentLayer,    //!< snap just to current layer (tolerance+type from QSettings)
      SnapPerLayerConfig,  //!< snap according to the configuration set in setLayers()
    };

    /** Set how the snapping to map is done */
    void setSnapToMapMode( SnapToMapMode mode ) { mSnapToMapMode = mode; }
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

    /** configure options used when the mode is snap to current layer */
    void setDefaultSettings( int type, double tolerance, QgsTolerance::UnitType unit );
    /** query options used when the mode is snap to current layer */
    void defaultSettings( int& type, double& tolerance, QgsTolerance::UnitType& unit );

    struct LayerConfig
    {
      LayerConfig( QgsVectorLayer* l, int t, double tol, QgsTolerance::UnitType u ) : layer( l ), type( t ), tolerance( tol ), unit( u ) {}

      QgsVectorLayer* layer;
      int type;
      double tolerance;
      QgsTolerance::UnitType unit;
    };

    /** Set layers which will be used for snapping */
    void setLayers( const QList<LayerConfig>& layers ) { mLayers = layers; }
    /** Query layers used for snapping */
    QList<LayerConfig> layers() const { return mLayers; }

    /** Set whether to consider intersections of nearby segments for snapping */
    void setSnapOnIntersections( bool enabled ) { mSnapOnIntersection = enabled; }
    /** Query whether to consider intersections of nearby segments for snapping */
    bool snapOnIntersections() const { return mSnapOnIntersection; }

  public slots:
    /** Read snapping configuration from the project */
    void readConfigFromProject();

  private slots:
    void onLayersWillBeRemoved( QStringList layerIds );

  private:
    //! get from map settings pointer to destination CRS - or 0 if projections are disabled
    const QgsCoordinateReferenceSystem* destCRS();

    //! delete all existing locators (e.g. when destination CRS has changed and we need to reindex)
    void clearAllLocators();

    //! return a locator (temporary or not) according to the indexing strategy
    QgsPointLocator* locatorForLayerUsingStrategy( QgsVectorLayer* vl, const QgsPoint& pointMap, double tolerance );
    //! return a temporary locator with index only for a small area (will be replaced by another one on next request)
    QgsPointLocator* temporaryLocatorForLayer( QgsVectorLayer* vl, const QgsPoint& pointMap, double tolerance );

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
};


#endif // QGSSNAPPINGUTILS_H
