/***************************************************************************
    qgsapplayerhandling.h
    -------------------------
    begin                : July 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAPPLAYERHANDLING_H
#define QGSAPPLAYERHANDLING_H

#include "qgis.h"
#include "qgis_app.h"
#include "qgsconfig.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayerref.h"

#include <QObject>

class QgsMapLayer;
class QgsProviderSublayerDetails;
class QgsPointCloudLayer;
class QgsVectorLayer;
class QgsRasterLayer;
class QgsMeshLayer;
class QgsPluginLayer;
class QgsVectorTileLayer;

/**
 * Contains logic related to general layer handling in QGIS app.
 */
class APP_EXPORT QgsAppLayerHandling
{
    Q_GADGET

  public:

    enum class SublayerHandling
    {
      AskUser,
      LoadAll,
      AbortLoading
    };

    /**
     * Adds a vector layer from a given \a uri and \a provider.
     *
     * The \a baseName parameter will be used as the layer name (and shown in the map legend).
     *
     * \note This may trigger a dialog asking users to select from available sublayers in the datasource,
     * depending on the contents of the datasource and the user's current QGIS settings.
     */
    static QgsVectorLayer *addVectorLayer( const QString &uri, const QString &baseName, const QString &provider = QLatin1String( "ogr" ), bool addToLegend = true );

    /**
     * Adds a list of vector layers from a list of layer \a uris supported by the OGR provider.
     *
     * \note This may trigger a dialog asking users to select from available sublayers in the datasource,
     * depending on the contents of the datasource and the user's current QGIS settings.
     *
     * If \a showWarningOnInvalid layers is TRUE then a user facing warning will be raised
     * if a uri does not result in a valid vector layer.
     */
    static QList< QgsMapLayer * > addOgrVectorLayers( const QStringList &uris, const QString &encoding, const QString &dataSourceType, bool &ok, bool showWarningOnInvalid = true );

    /**
     * Adds a raster layer from a given \a uri and \a provider.
     *
     * The \a baseName parameter will be used as the layer name (and shown in the map legend).
     *
     * \note This may trigger a dialog asking users to select from available sublayers in the datasource,
     * depending on the contents of the datasource and the user's current QGIS settings.
     */
    static QgsRasterLayer *addRasterLayer( QString const &uri, const QString &baseName, const QString &provider = QLatin1String( "gdal" ), bool addToLegend = true );

    /**
     * Adds a list of raster layers from a list of layer \a uris supported by the GDAL provider.
     *
     * \note This may trigger a dialog asking users to select from available sublayers in the datasource,
     * depending on the contents of the datasource and the user's current QGIS settings.
     *
     * If \a showWarningOnInvalid layers is TRUE then a user facing warning will be raised
     * if a uri does not result in a valid vector layer.
     */
    static QList< QgsMapLayer * > addGdalRasterLayers( const QStringList &uris, bool &ok, bool showWarningOnInvalid = true );

    /**
     * Adds a mesh layer from a given \a uri and \a provider.
     *
     * The \a baseName parameter will be used as the layer name (and shown in the map legend).
     *
     * \note This may trigger a dialog asking users to select from available sublayers in the datasource,
     * depending on the contents of the datasource and the user's current QGIS settings.
     */
    static QgsMeshLayer *addMeshLayer( const QString &uri, const QString &baseName, const QString &provider, bool addToLegend = true );

    /**
     * Adds a point cloud layer from a given \a uri and \a provider.
     *
     * The \a baseName parameter will be used as the layer name (and shown in the map legend).
     *
     * If \a showWarningOnInvalid layers is TRUE then a user facing warning will be raised
     * if the \a uri does not result in a valid point cloud layer.
     */
    static QgsPointCloudLayer *addPointCloudLayer( const QString &uri,
        const QString &baseName,
        const QString &provider,
        bool showWarningOnInvalid = true,
        bool addToLegend = true );

    /**
     * Adds a plugin layer from a given \a uri and \a provider.
     *
     * The \a baseName parameter will be used as the layer name (and shown in the map legend).
     */
    static QgsPluginLayer *addPluginLayer( const QString &uri, const QString &baseName, const QString &providerKey, bool addToLegend = true );

    /**
     * Adds a vector tile layer from a given \a uri.
     *
     * The \a baseName parameter will be used as the layer name (and shown in the map legend).
     *
     * If \a showWarningOnInvalid layers is TRUE then a user facing warning will be raised
     * if the \a uri does not result in a valid vector tile layer.
     */
    static QgsVectorTileLayer *addVectorTileLayer( const QString &uri, const QString &baseName, bool showWarningOnInvalid = true, bool addToLegend = true );

    /**
     * Post processes an entire group of added \a layers.
     *
     * \note This method will be called for the group AFTER the postProcessAddedLayer()
     * method has been called for each layer in turn. All added layers will already
     * have been added to the project.
     */
    static void postProcessAddedLayers( const QList< QgsMapLayer * > &layers );

    static void addSortedLayersToLegend( QList< QgsMapLayer * > &layers );

    /**
     * Open a map layer from a file.
     *
     * Set \a allowInteractive to TRUE if it is OK to ask the user for information (mostly for
     * when a vector layer has sublayers and we want to ask which sublayers to use).
     *
     * \returns a list of added map layers if the file is successfully opened
     */
    static QList< QgsMapLayer * > openLayer( const QString &fileName, bool &ok, bool allowInteractive = false, bool suppressBulkLayerPostProcessing = false, bool addToLegend = true );

    //! Add a 'pre-made' map layer to the project
    static void addMapLayer( QgsMapLayer *mapLayer, bool addToLegend = true );

    static void openLayerDefinition( const QString &filename );

    //! Add a Layer Definition file
    static void addLayerDefinition();

    //! Add a list of database layers to the map
    static QList< QgsMapLayer * > addDatabaseLayers( const QStringList &layerPathList, const QString &providerKey, bool &ok );

    /**
     * Flags which control the behavior of loading layer dependencies.
     */
    enum class DependencyFlag : int
    {
      LoadAllRelationships = 1 << 1, //!< Causes all relationships to be loaded, regardless of whether the originating table is the referenced or referencing table. By default relationships are only loaded when the originating table is the referencing table.
      SilentLoad = 1 << 2, //!< Dependencies are loaded without any user-visible notifications.
    };
    Q_ENUM( DependencyFlag )
    Q_DECLARE_FLAGS( DependencyFlags, DependencyFlag )
    Q_FLAG( DependencyFlags )

    /**
     * Searches for layer dependencies by querying the form widgets and the
     * \a vectorLayer itself for broken relations. Style \a categories can be
     * used to limit the search to one or more of the currently implemented search
     * categories ("Forms" for the form widgets and "Relations" for layer weak relations).
     * \return a list of weak references to broken layer dependencies
     */
    static const QList< QgsVectorLayerRef > findBrokenLayerDependencies( QgsVectorLayer *vectorLayer,
        QgsMapLayer::StyleCategories categories = QgsMapLayer::StyleCategory::AllStyleCategories,
        QgsVectorLayerRef::MatchType matchType = QgsVectorLayerRef::MatchType::Name,
        DependencyFlags dependencyFlags = DependencyFlags() );

    /**
     * Scans the \a vectorLayer for broken dependencies and automatically
     * try to load the missing layers, users are notified about the operation
     * result. Style \a categories can be
     * used to exclude one of the currently implemented search categories
     * ("Forms" for the form widgets and "Relations" for layer weak relations).
     */
    static void resolveVectorLayerDependencies( QgsVectorLayer *vectorLayer,
        QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories,
        QgsVectorLayerRef::MatchType matchType = QgsVectorLayerRef::MatchType::Name,
        DependencyFlags dependencyFlags = DependencyFlags() );

    /**
     * Scans the \a vectorLayer for weak relations and automatically
     * try to resolve and create the broken relations.
     *
     * This method will automatically attempt to repair any relations using
     * other layers already present in the current project.
     *
     * If \a guiWarnings is TRUE then the explanation for invalid relationships
     * will be shown to the user.
     */
    static void resolveVectorLayerWeakRelations( QgsVectorLayer *vectorLayer, QgsVectorLayerRef::MatchType matchType = QgsVectorLayerRef::MatchType::Name, bool guiWarnings = false );

    /**
     * Triggered when a vector layer style has changed, checks for widget config layer dependencies
     * \param categories style categories
     */
    static void onVectorLayerStyleLoaded( QgsVectorLayer *vl, const QgsMapLayer::StyleCategories categories );

  private:

    template<typename T> static T *addLayerPrivate( QgsMapLayerType type, const QString &uri, const QString &baseName, const QString &providerKey, bool guiWarnings = true, bool addToLegend = true );

    /**
     * Post processes a single added \a layer, applying any default behavior which should
     * happen to newly added layers.
     *
     * \note If a group of a layers is added at once, this method will be called one-by-one
     * for each layer BEFORE the postProcessAddedLayers() method is called for the entire
     * group.
     */
    static void postProcessAddedLayer( QgsMapLayer *layer );

    /**
     * This method will open a dialog so the user can select GDAL sublayers to load
     * \returns TRUE if any items were loaded
     */
    static bool askUserForZipItemLayers( const QString &path, const QList< QgsMapLayerType > &acceptableTypes );

    static SublayerHandling shouldAskUserForSublayers( const QList< QgsProviderSublayerDetails > &layers, bool hasNonLayerItems = false );

    static QList< QgsMapLayer * > addSublayers( const QList< QgsProviderSublayerDetails> &layers, const QString &baseName, const QString &groupName, bool addToLegend = true );

};
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsAppLayerHandling::DependencyFlags );

#endif // QGSAPPLAYERHANDLING_H
