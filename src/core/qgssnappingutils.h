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
 * Has all the configuration of snapping and can return answers to snapping queries.
 * This one will be also available from iface for map tools.
 *
 * @note added in 2.8
 */
class QgsSnappingUtils : public QObject
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
    // TODO: multi-variant


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

#if 0
    /** Set topological editing status (used by some map tools) */
    void setTopologicalEditing( bool enabled );
    /** Query topological editing status (used by some map tools) */
    bool topologicalEditing() const;
#endif

  public slots:
    /** Read snapping configuration from the project */
    void readConfigFromProject();

    // requirements:
    // - support existing configurations
    // - handle updates from QgsProject::setSnapSettingsForLayer()

  private slots:
    void onLayersWillBeRemoved( QStringList layerIds );

  private:
    //! get from map settings pointer to destination CRS - or 0 if projections are disabled
    const QgsCoordinateReferenceSystem* destCRS();

    //! delete all existing locators (e.g. when destination CRS has changed and we need to reindex)
    void clearAllLocators();

  private:
    // environment
    QgsMapSettings mMapSettings;
    QgsVectorLayer* mCurrentLayer;

    // configuration
    SnapToMapMode mSnapToMapMode;
    int mDefaultType;
    double mDefaultTolerance;
    QgsTolerance::UnitType mDefaultUnit;
    QList<LayerConfig> mLayers;
    bool mSnapOnIntersection;

    // internal data
    typedef QMap<QgsVectorLayer*, QgsPointLocator*> LocatorsMap;
    //! on-demand locators used (locators are owned)
    LocatorsMap mLocators;
};


#endif // QGSSNAPPINGUTILS_H
