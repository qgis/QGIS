/***************************************************************************
                                  qgsproject.h

                      Implements persistent project state.

                              -------------------
  begin                : July 23, 2004
  copyright            : (C) 2004 by Mark Coletti
  email                : mcoletti at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROJECT_H
#define QGSPROJECT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include <memory>
#include <QHash>
#include <QList>
#include <QObject>
#include <QPair>
#include <QFileInfo>
#include <QStringList>

#include "qgsunittypes.h"
#include "qgssnappingconfig.h"
#include "qgsprojectversion.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsprojectproperty.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerstore.h"
#include "qgsarchive.h"

class QFileInfo;
class QDomDocument;
class QDomElement;
class QDomNode;

class QgsLayerTreeGroup;
class QgsLayerTreeRegistryBridge;
class QgsMapLayer;
class QgsMapThemeCollection;
class QgsPathResolver;
class QgsProjectBadLayerHandler;
class QgsRelationManager;
class QgsTolerance;
class QgsTransactionGroup;
class QgsVectorLayer;
class QgsAnnotationManager;
class QgsLayoutManager;
class QgsLayerTree;
class QgsLabelingEngineSettings;
class QgsAuxiliaryStorage;

/**
 * \ingroup core
 * Reads and writes project states.
 *
  \note

  Has two general kinds of state to make persistent.  (I.e., to read and
  write.)  First, QGIS proprietary information.  Second plug-in information.

  A singleton since there shall only be one active project at a time; and
  provides canonical location for plug-ins and main app to find/set
  properties.

*/

class CORE_EXPORT QgsProject : public QObject, public QgsExpressionContextGenerator
{
    Q_OBJECT
    Q_PROPERTY( QStringList nonIdentifiableLayers READ nonIdentifiableLayers WRITE setNonIdentifiableLayers NOTIFY nonIdentifiableLayersChanged )
    Q_PROPERTY( QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged )
    Q_PROPERTY( QString homePath READ homePath NOTIFY homePathChanged )
    Q_PROPERTY( QgsCoordinateReferenceSystem crs READ crs WRITE setCrs NOTIFY crsChanged )
    Q_PROPERTY( QString ellipsoid READ ellipsoid WRITE setEllipsoid NOTIFY ellipsoidChanged )
    Q_PROPERTY( QgsMapThemeCollection *mapThemeCollection READ mapThemeCollection NOTIFY mapThemeCollectionChanged )
    Q_PROPERTY( QgsSnappingConfig snappingConfig READ snappingConfig WRITE setSnappingConfig NOTIFY snappingConfigChanged )
    Q_PROPERTY( QgsRelationManager *relationManager READ relationManager )
    Q_PROPERTY( QList<QgsVectorLayer *> avoidIntersectionsLayers READ avoidIntersectionsLayers WRITE setAvoidIntersectionsLayers NOTIFY avoidIntersectionsLayersChanged )

  public:
    //! Returns the QgsProject singleton instance
    static QgsProject *instance();

    /**
     * Create a new QgsProject.
     *
     * Most of the time you want to use QgsProject::instance() instead as many components of QGIS work with the singleton.
     */
    explicit QgsProject( QObject *parent SIP_TRANSFERTHIS = 0 );

    ~QgsProject();

    /**
     * Sets the project's title.
     * \param title new title
     * \since QGIS 2.4
     * \see title()
     */
    void setTitle( const QString &title );

    /**
     * Returns the project's title.
     * \see setTitle()
    */
    QString title() const;

    /**
     * Returns true if the project has been modified since the last write()
     */
    bool isDirty() const;

    /**
     * Sets the file name associated with the project. This is the file which contains the project's XML
     * representation.
     * \param name project file name
     * \see fileName()
     */
    void setFileName( const QString &name );

    /**
     * Returns the project's file name. This is the file which contains the project's XML
     * representation.
     * \see setFileName()
     * \see fileInfo()
    */
    QString fileName() const;

    /**
     * Returns QFileInfo object for the project's associated file.
     * \see fileName()
     * \since QGIS 2.9
     */
    QFileInfo fileInfo() const;

    /**
     * Returns the project's native coordinate reference system.
     * \since QGIS 3.0
     * \see setCrs()
     * \see ellipsoid()
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Sets the project's native coordinate reference system.
     * \since QGIS 3.0
     * \see crs()
     * \see setEllipsoid()
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns a proj string representing the project's ellipsoid setting, e.g., "WGS84".
     * \see setEllipsoid()
     * \see crs()
     * \since QGIS 3.0
     */
    QString ellipsoid() const;

    /**
     * Sets the project's ellipsoid from a proj string representation, e.g., "WGS84".
     * \see ellipsoid()
     * \see setCrs()
     * \since QGIS 3.0
     */
    void setEllipsoid( const QString &ellipsoid );

    /**
     * Clear the project - removes all settings and resets it back to an empty, default state.
     * \since QGIS 2.4
     */
    void clear();

    /**
     * Reads given project file from the given file.
     * \param filename name of project file to read
     * \returns true if project file has been read successfully
     */
    bool read( const QString &filename );

    /**
     * Reads the project from its currently associated file (see fileName() ).
     * \returns true if project file has been read successfully
     */
    bool read();

    /**
     * Reads the layer described in the associated DOM node.
     *
     * \note This method is mainly for use by QgsProjectBadLayerHandler subclasses
     * that may fix definition of bad layers with the user's help in GUI. Calling
     * this method with corrected DOM node adds the layer back to the project.
     *
     * \param layerNode represents a QgsProject DOM node that encodes a specific layer.
     */
    bool readLayer( const QDomNode &layerNode );

    /**
     * Writes the project to a file.
     * \param filename destination file
     * \note calling this implicitly sets the project's filename (see setFileName() )
     * \note isDirty() will be set to false if project is successfully written
     * \returns true if project was written successfully
     *
     * \since QGIS 3.0
     */
    bool write( const QString &filename );

    /**
     * Writes the project to its current associated file (see fileName() ).
     * \note isDirty() will be set to false if project is successfully written
     * \returns true if project was written successfully
     */
    bool write();

    /**
     * Write a boolean entry to the project file.
     *
     * Keys are '/'-delimited entries, implying
     * a hierarchy of keys and corresponding values
     *
     * \note The key string must be valid xml tag names in order to be saved to the file.
     * \note available in Python bindings as writeEntryBool
     */
    bool writeEntry( const QString &scope, const QString &key, bool value ) SIP_PYNAME( writeEntryBool );

    /**
     * Write a double entry to the project file.
     *
     * Keys are '/'-delimited entries, implying
     * a hierarchy of keys and corresponding values
     *
     * \note The key string must be valid xml tag names in order to be saved to the file.
     * \note available in Python bindings as writeEntryDouble
     */
    bool writeEntry( const QString &scope, const QString &key, double value ) SIP_PYNAME( writeEntryDouble );

    /**
     * Write an integer entry to the project file.
     *
     * Keys are '/'-delimited entries, implying
     * a hierarchy of keys and corresponding values
     *
     * \note The key string must be valid xml tag names in order to be saved to the file.
     */
    bool writeEntry( const QString &scope, const QString &key, int value );

    /**
     * Write a string entry to the project file.
     *
     * Keys are '/'-delimited entries, implying
     * a hierarchy of keys and corresponding values
     *
     * \note The key string must be valid xml tag names in order to be saved to the file.
     */
    bool writeEntry( const QString &scope, const QString &key, const QString &value );

    /**
     * Write a string list entry to the project file.
     *
     * Keys are '/'-delimited entries, implying
     * a hierarchy of keys and corresponding values
     *
     * \note The key string must be valid xml tag names in order to be saved to the file.
     */
    bool writeEntry( const QString &scope, const QString &key, const QStringList &value );

    /**
     * Key value accessors
     *
     * keys would be the familiar QgsSettings-like '/' delimited entries,
     * implying a hierarchy of keys and corresponding values
     */
    QStringList readListEntry( const QString &scope, const QString &key, const QStringList &def = QStringList(), bool *ok = nullptr ) const;

    QString readEntry( const QString &scope, const QString &key, const QString &def = QString(), bool *ok = nullptr ) const;
    int readNumEntry( const QString &scope, const QString &key, int def = 0, bool *ok = nullptr ) const;
    double readDoubleEntry( const QString &scope, const QString &key, double def = 0, bool *ok = nullptr ) const;
    bool readBoolEntry( const QString &scope, const QString &key, bool def = false, bool *ok = nullptr ) const;


    //! Remove the given key
    bool removeEntry( const QString &scope, const QString &key );


    /**
     * Return keys with values -- do not return keys that contain other keys
     *
     * \note equivalent to QgsSettings entryList()
     */
    QStringList entryList( const QString &scope, const QString &key ) const;

    /**
     * Return keys with keys -- do not return keys that contain only values
     *
     * \note equivalent to QgsSettings subkeyList()
     */
    QStringList subkeyList( const QString &scope, const QString &key ) const;


    /**
     * Dump out current project properties to stderr
     */
    // TODO Now slightly broken since re-factoring.  Won't print out top-level key
    //           and redundantly prints sub-keys.
    void dumpProperties() const;

    /**
     * Return path resolver object with considering whether the project uses absolute
     * or relative paths and using current project's path.
     * \since QGIS 3.0
     */
    QgsPathResolver pathResolver() const;

    /**
     * Prepare a filename to save it to the project file.
     * Creates an absolute or relative path according to the project settings.
     * Paths written to the project file should be prepared with this method.
    */
    QString writePath( const QString &filename ) const;

    //! Turn filename read from the project file to an absolute path
    QString readPath( const QString &filename ) const;

    //! Return error message from previous read/write
    QString error() const;

    /**
     * Change handler for missing layers.
     * Deletes old handler and takes ownership of the new one.
     */
    void setBadLayerHandler( QgsProjectBadLayerHandler *handler SIP_TRANSFER );

    //! Returns project file path if layer is embedded from other project file. Returns empty string if layer is not embedded
    QString layerIsEmbedded( const QString &id ) const;

    /**
     * Creates a maplayer instance defined in an arbitrary project file. Caller takes ownership
     * \returns the layer or 0 in case of error
     * \note not available in Python bindings
     */
    bool createEmbeddedLayer( const QString &layerId, const QString &projectFilePath, QList<QDomNode> &brokenNodes,
                              bool saveFlag = true ) SIP_SKIP;

    /**
     * Create layer group instance defined in an arbitrary project file.
     * \since QGIS 2.4
     */
    QgsLayerTreeGroup *createEmbeddedGroup( const QString &groupName, const QString &projectFilePath, const QStringList &invisibleLayers );

    //! Convenience function to set topological editing
    void setTopologicalEditing( bool enabled );

    //! Convenience function to query topological editing status
    bool topologicalEditing() const;

    /**
     * Convenience function to query default distance measurement units for project.
     * \since QGIS 2.14
     * \see setDistanceUnits()
     * \see areaUnits()
     */
    QgsUnitTypes::DistanceUnit distanceUnits() const;

    /**
     * Sets the default distance measurement units for the project.
     * \since QGIS 3.0
     * \see distanceUnits()
     * \see setAreaUnits()
     */
    void setDistanceUnits( QgsUnitTypes::DistanceUnit unit );

    /**
     * Convenience function to query default area measurement units for project.
     * \since QGIS 2.14
     * \see distanceUnits()
     */
    QgsUnitTypes::AreaUnit areaUnits() const;

    /**
     * Sets the default area measurement units for the project.
     * \since QGIS 3.0
     * \see areaUnits()
     * \see setDistanceUnits()
     */
    void setAreaUnits( QgsUnitTypes::AreaUnit unit );

    /**
     * Return project's home path
      \returns home path of project (or null QString if not set) */
    QString homePath() const;

    QgsRelationManager *relationManager() const;

    /**
     * Returns the project's layout manager, which manages compositions within
     * the project.
     * \since QGIS 3.0
     * \note not available in Python bindings
     */
    const QgsLayoutManager *layoutManager() const SIP_SKIP;

    /**
     * Returns the project's layout manager, which manages compositions within
     * the project.
     * \since QGIS 3.0
     */
    QgsLayoutManager *layoutManager();

    /**
     * Return pointer to the root (invisible) node of the project's layer tree
     * \since QGIS 2.4
     */
    QgsLayerTree *layerTreeRoot() const;

    /**
     * Return pointer to the helper class that synchronizes map layer registry with layer tree
     * \since QGIS 2.4
     */
    QgsLayerTreeRegistryBridge *layerTreeRegistryBridge() const { return mLayerTreeRegistryBridge; }

    /**
     * Returns pointer to the project's map theme collection.
     * \since QGIS 2.12
     * \note renamed in QGIS 3.0, formerly QgsVisibilityPresetCollection
     */
    QgsMapThemeCollection *mapThemeCollection();

    /**
     * Returns pointer to the project's annotation manager.
     * \since QGIS 3.0
     */
    QgsAnnotationManager *annotationManager();

    /**
     * Returns a const pointer to the project's annotation manager.
     * \since QGIS 3.0
     */
    const QgsAnnotationManager *annotationManager() const SIP_SKIP;

    /**
     * Set a list of layers which should not be taken into account on map identification
     */
    void setNonIdentifiableLayers( const QList<QgsMapLayer *> &layers );

    /**
     * Set a list of layers which should not be taken into account on map identification
     */
    void setNonIdentifiableLayers( const QStringList &layerIds );

    /**
     * Get the list of layers which currently should not be taken into account on map identification
     */
    QStringList nonIdentifiableLayers() const;

    /**
     * Transactional editing means that on supported datasources (postgres databases) the edit state of
     * all tables that originate from the same database are synchronized and executed in a server side
     * transaction.
     *
     * \since QGIS 2.16
     */
    bool autoTransaction() const;

    /**
     * Transactional editing means that on supported datasources (postgres databases) the edit state of
     * all tables that originate from the same database are synchronized and executed in a server side
     * transaction.
     *
     * Make sure that this is only called when all layers are not in edit mode.
     *
     * \since QGIS 2.16
     */
    void setAutoTransaction( bool autoTransaction );

    /**
     * Map of transaction groups
     *
     * QPair( providerKey, connString ) -> transactionGroup
     *
     * \since QGIS 2.16
     * \note Not available in Python bindings
     */
    QMap< QPair< QString, QString>, QgsTransactionGroup *> transactionGroups() SIP_SKIP;

    /**
     * Should default values be evaluated on provider side when requested and not when committed.
     *
     * \since QGIS 2.16
     */
    bool evaluateDefaultValues() const;


    /**
     * Defines if default values should be evaluated on provider side when requested and not when committed.
     *
     * \since QGIS 2.16
     */
    void setEvaluateDefaultValues( bool evaluateDefaultValues );

    QgsExpressionContext createExpressionContext() const override;

    /**
     * The snapping configuration for this project.
     *
     * \since QGIS 3.0
     */
    QgsSnappingConfig snappingConfig() const;

    /**
     * A list of layers with which intersections should be avoided.
     *
     * \since QGIS 3.0
     */
    QList<QgsVectorLayer *> avoidIntersectionsLayers() const;

    /**
     * A list of layers with which intersections should be avoided.
     *
     * \since QGIS 3.0
     */
    void setAvoidIntersectionsLayers( const QList<QgsVectorLayer *> &layers );

    /**
     * A map of custom project variables.
     * To get all available variables including generated ones
     * use QgsExpressionContextUtils::projectScope() instead.
     */
    QVariantMap customVariables() const;

    /**
     * A map of custom project variables.
     * Be careful not to set generated variables.
     */
    void setCustomVariables( const QVariantMap &customVariables );

    /**
     * Sets project's global labeling engine settings
     * \since QGIS 3.0
     */
    void setLabelingEngineSettings( const QgsLabelingEngineSettings &settings );

    /**
     * Returns project's global labeling engine settings
     * \since QGIS 3.0
     */
    const QgsLabelingEngineSettings &labelingEngineSettings() const;

    //
    // Functionality from QgsMapLayerRegistry
    //

    /**
     * Returns a pointer to the project's internal layer store.
     * /since QGIS 3.0
     */
    QgsMapLayerStore *layerStore();

    /**
     * Returns a pointer to the project's internal layer store.
     * /since QGIS 3.0
     */
    SIP_SKIP const QgsMapLayerStore *layerStore() const;

    //! Returns the number of registered layers.
    int count() const;

    /**
     * Retrieve a pointer to a registered layer by layer ID.
     * \param layerId ID of layer to retrieve
     * \returns matching layer, or nullptr if no matching layer found
     * \see mapLayersByName()
     * \see mapLayers()
     */
    QgsMapLayer *mapLayer( const QString &layerId ) const;

    /**
     * Retrieve a list of matching registered layers by layer name.
     * \param layerName name of layers to match
     * \returns list of matching layers
     * \see mapLayer()
     * \see mapLayers()
     */
    QList<QgsMapLayer *> mapLayersByName( const QString &layerName ) const;

    /**
     * Returns a map of all registered layers by layer ID.
     * \see mapLayer()
     * \see mapLayersByName()
     * \see layers()
     */
    QMap<QString, QgsMapLayer *> mapLayers() const;

    /**
     * Returns true if the project comes from a zip archive, false otherwise.
     */
    bool isZipped() const;

#ifndef SIP_RUN

    /**
     * Returns a list of registered map layers with a specified layer type.
     *
     * Example:
     *
     *     QVector<QgsVectorLayer*> vectorLayers = QgsProject::instance()->layers<QgsVectorLayer*>();
     *
     * \note not available in Python bindings
     * \since QGIS 2.16
     * \see mapLayers()
     */
    template <typename T>
    QVector<T> layers() const
    {
      return mLayerStore->layers<T>();
    }
#endif

    /**
     * \brief
     * Add a list of layers to the map of loaded layers.
     *
     * The layersAdded() and layerWasAdded() signals will always be emitted.
     * The legendLayersAdded() signal is emitted only if addToLegend is true.
     *
     * \param mapLayers  A list of layer which should be added to the registry
     * \param addToLegend   If true (by default), the layers will be added to the
     *                      legend and to the main canvas. If you have a private
     *                      layer you can set this parameter to false to hide it.
     * \param takeOwnership Ownership will be transferred to the layer registry.
     *                      If you specify false here you have take care of deleting
     *                      the layers yourself. Not available in Python.
     *
     * \returns a list of the map layers that were added
     *         successfully. If a layer is invalid, or already exists in the registry,
     *         it will not be part of the returned QList.
     *
     * \note As a side-effect QgsProject is made dirty.
     * \note takeOwnership is not available in the Python bindings - the registry will always
     * take ownership
     * \since QGIS 1.8
     * \see addMapLayer()
     */
    QList<QgsMapLayer *> addMapLayers( const QList<QgsMapLayer *> &mapLayers SIP_TRANSFER,
                                       bool addToLegend = true,
                                       bool takeOwnership SIP_PYARGREMOVE = true );

    /**
     * \brief
     * Add a layer to the map of loaded layers.
     *
     * The layersAdded() and layerWasAdded() signals will always be emitted.
     * The legendLayersAdded() signal is emitted only if addToLegend is true.
     * If you are adding multiple layers at once, you should use
     * addMapLayers() instead.
     *
     * \param mapLayer A layer to add to the registry
     * \param addToLegend If true (by default), the layer will be added to the
     *                    legend and to the main canvas. If you have a private
     *                    layer you can set this parameter to false to hide it.
     * \param takeOwnership Ownership will be transferred to the layer registry.
     *                      If you specify false here you have take care of deleting
     *                      the layer yourself. Not available in Python.
     *
     * \returns nullptr if unable to add layer, otherwise pointer to newly added layer
     *
     * \see addMapLayers
     *
     * \note As a side-effect QgsProject is made dirty.
     * \note Use addMapLayers if adding more than one layer at a time
     * \note takeOwnership is not available in the Python bindings - the registry will always
     * take ownership
     * \see addMapLayers()
     */
    QgsMapLayer *addMapLayer( QgsMapLayer *mapLayer SIP_TRANSFER,
                              bool addToLegend = true,
                              bool takeOwnership SIP_PYARGREMOVE = true );

    /**
     * \brief
     * Remove a set of layers from the registry by layer ID.
     *
     * The specified layers will be removed from the registry. If the registry has ownership
     * of any layers these layers will also be deleted.
     *
     * \param layerIds list of IDs of the layers to remove
     *
     * \note As a side-effect the QgsProject instance is marked dirty.
     * \since QGIS 1.8
     * \see removeMapLayer()
     * \see removeAllMapLayers()
     */
    void removeMapLayers( const QStringList &layerIds );

    /**
     * \brief
     * Remove a set of layers from the registry.
     *
     * The specified layers will be removed from the registry. If the registry has ownership
     * of any layers these layers will also be deleted.
     *
     * \param layers A list of layers to remove. Null pointers are ignored.
     *
     * \note As a side-effect the QgsProject instance is marked dirty.
     * \see removeMapLayer()
     * \see removeAllMapLayers()
     */
    //TODO QGIS 3.0 - add PyName alias to avoid list type conversion error
    void removeMapLayers( const QList<QgsMapLayer *> &layers );

    /**
     * \brief
     * Remove a layer from the registry by layer ID.
     *
     * The specified layer will be removed from the registry. If the registry has ownership
     * of the layer then it will also be deleted.
     *
     * \param layerId ID of the layer to remove
     *
     * \note As a side-effect the QgsProject instance is marked dirty.
     * \see removeMapLayers()
     * \see removeAllMapLayers()
     */
    void removeMapLayer( const QString &layerId );

    /**
     * \brief
     * Remove a layer from the registry.
     *
     * The specified layer will be removed from the registry. If the registry has ownership
     * of the layer then it will also be deleted.
     *
     * \param layer The layer to remove. Null pointers are ignored.
     *
     * \note As a side-effect the QgsProject instance is marked dirty.
     * \see removeMapLayers()
     * \see removeAllMapLayers()
     */
    void removeMapLayer( QgsMapLayer *layer );

    /**
     * Takes a layer from the registry. If the layer was owned by the project, the
     * layer will be returned without deleting it. The caller takes ownership of
     * the layer and is responsible for deleting it.
     * \see removeMapLayer()
     * \since QGIS 3.0
     */
    QgsMapLayer *takeMapLayer( QgsMapLayer *layer ) SIP_TRANSFERBACK;

    /**
     * Removes all registered layers. If the registry has ownership
     * of any layers these layers will also be deleted.
     *
     * \note As a side-effect the QgsProject instance is marked dirty.
     * \note Calling this method will cause the removeAll() signal to
     * be emitted.
     * \see removeMapLayer()
     * \see removeMapLayers()
     */
    void removeAllMapLayers();

    /**
     * Reload all registered layer's provider data caches, synchronising the layer
     * with any changes in the datasource.
     * \see QgsMapLayer::reload()
     */
    void reloadAllLayers();

    /**
     * Returns the default CRS for new layers based on the settings and
     * the current project CRS
     */
    QgsCoordinateReferenceSystem defaultCrsForNewLayers() const;

    /**
     * Sets the trust option allowing to indicate if the extent has to be
     * read from the XML document when data source has no metadata or if the
     * data provider has to determine it. Moreover, when this option is
     * activated, primary key unicity is not checked for views and
     * materialized views with Postgres provider.
     *
     * \param trust True to trust the project, false otherwise
     *
     * \since QGIS 3.0
     */
    void setTrustLayerMetadata( bool trust );

    /**
     * Returns true if the trust option is activated, false otherwise. This
     * option allows indicateing if the extent has to be read from the XML
     * document when data source has no metadata or if the data provider has
     * to determine it. Moreover, when this option is activated, primary key
     * unicity is not checked for views and materialized views with Postgres
     * provider.
     *
     * \since QGIS 3.0
     */
    bool trustLayerMetadata() const { return mTrustLayerMetadata; }

    /**
     * Returns the current const auxiliary storage.
     *
     * \since QGIS 3.0
     */
    const QgsAuxiliaryStorage *auxiliaryStorage() const SIP_SKIP;

    /**
     * Returns the current auxiliary storage.
     *
     * \since QGIS 3.0
     */
    QgsAuxiliaryStorage *auxiliaryStorage();

  signals:
    //! emitted when project is being read
    void readProject( const QDomDocument & );

    //! emitted when project is being written
    void writeProject( QDomDocument & );

    /**
     * Emitted, after the basic initialization of a layer from the project
     * file is done. You can use this signal to read additional information
     * from the project file.
     *
     * \param mapLayer  The map layer which is being initialized
     * \param layerNode The layer node from the project file
     */
    void readMapLayer( QgsMapLayer *mapLayer, const QDomElement &layerNode );

    /**
     * Emitted, when a layer is being saved. You can use this method to save
     * additional information to the layer.
     *
     * \param mapLayer  The map layer which is being initialized
     * \param layerElem The layer element from the project file
     * \param doc The document
     */
    void writeMapLayer( QgsMapLayer *mapLayer, QDomElement &layerElem, QDomDocument &doc );

    //! emitted when the project file has been written and closed
    void projectSaved();

    //! emitted when an old project file is read.
    void oldProjectVersionWarning( const QString & );

    /**
     * Emitted when a layer from a projects was read.
     * \param i current layer
     * \param n number of layers
     */
    void layerLoaded( int i, int n );

    void loadingLayer( const QString & );

    //! Emitted when the list of layer which are excluded from map identification changes
    void nonIdentifiableLayersChanged( QStringList nonIdentifiableLayers );

    //! Emitted when the file name of the project changes
    void fileNameChanged();

    //! Emitted when the home path of the project changes
    void homePathChanged();

    //! emitted whenever the configuration for snapping has changed
    void snappingConfigChanged( const QgsSnappingConfig &config );

    /**
     * Emitted whenever the expression variables stored in the project have been changed.
     * \since QGIS 3.0
     */
    void customVariablesChanged();

    /**
     * Emitted when the CRS of the project has changed.
     *
     * \since QGIS 3.0
     */
    void crsChanged();

    /**
     * Emitted when the project \a ellipsoid is changed.
     *
     * \since QGIS 3.0
     * \see setEllipsoid()
     * \see ellipsoid()
     */
    void ellipsoidChanged( const QString &ellipsoid );

    /**
     * Emitted whenever a new transaction group has been created or a
     * transaction group has been removed.
     *
     * \since QGIS 3.0
     */
    void transactionGroupsChanged();

    /**
     * Emitted when the topological editing flag has changed.
     *
     * \since QGIS 3.0
     */
    void topologicalEditingChanged();

    /**
     * Emitted whenever avoidIntersectionsLayers has changed.
     *
     * \since QGIS 3.0
     */
    void avoidIntersectionsLayersChanged();

    /**
     * Emitted when the map theme collection changes.
     * This only happens when the map theme collection is reset.
     * Any pointer previously received from mapThemeCollection()
     * must no longer be used after this signal is emitted.
     * You must still connect to signals from the map theme collection
     * if you want to be notified about new map themes being added and
     * map themes being removed.
     *
     * \since QGIS 3.0
     */
    void mapThemeCollectionChanged();

    /**
     * Emitted when global configuration of the labeling engine changes.
     * \since QGIS 3.0
     */
    void labelingEngineSettingsChanged();

    //
    // signals from QgsMapLayerRegistry
    //

    /**
     * Emitted when one or more layers are about to be removed from the registry.
     *
     * \param layerIds A list of IDs for the layers which are to be removed.
     * \see layerWillBeRemoved()
     * \see layersRemoved()
     */
    void layersWillBeRemoved( const QStringList &layerIds );

    /**
     * Emitted when one or more layers are about to be removed from the registry.
     *
     * \param layers A list of layers which are to be removed.
     * \see layerWillBeRemoved()
     * \see layersRemoved()
     */
    void layersWillBeRemoved( const QList<QgsMapLayer *> &layers );

    /**
     * Emitted when a layer is about to be removed from the registry.
     *
     * \param layerId The ID of the layer to be removed.
     *
     * \note Consider using layersWillBeRemoved() instead
     * \see layersWillBeRemoved()
     * \see layerRemoved()
     */
    void layerWillBeRemoved( const QString &layerId );

    /**
     * Emitted when a layer is about to be removed from the registry.
     *
     * \param layer The layer to be removed.
     *
     * \note Consider using layersWillBeRemoved() instead
     * \see layersWillBeRemoved()
     * \see layerRemoved()
     */
    void layerWillBeRemoved( QgsMapLayer *layer );

    /**
     * Emitted after one or more layers were removed from the registry.
     *
     * \param layerIds  A list of IDs of the layers which were removed.
     * \see layersWillBeRemoved()
     */
    void layersRemoved( const QStringList &layerIds );

    /**
     * Emitted after a layer was removed from the registry.
     *
     * \param layerId The ID of the layer removed.
     *
     * \note Consider using layersRemoved() instead
     * \see layerWillBeRemoved()
     */
    void layerRemoved( const QString &layerId );

    /**
     * Emitted when all layers are removed, before layersWillBeRemoved() and
     * layerWillBeRemoved() signals are emitted. The layersWillBeRemoved() and
     * layerWillBeRemoved() signals will still be emitted following this signal.
     * You can use this signal to do easy (and fast) cleanup.
     */
    //TODO QGIS 3.0 - rename to past tense
    void removeAll();

    /**
     * Emitted when one or more layers were added to the registry.
     * This signal is also emitted for layers added to the registry,
     * but not to the legend.
     *
     * \param layers List of layers which have been added.
     *
     * \see legendLayersAdded()
     * \see layerWasAdded()
     */
    void layersAdded( const QList<QgsMapLayer *> &layers );

    /**
     * Emitted when a layer was added to the registry.
     *
     * \note Consider using layersAdded() instead
     * \see layersAdded()
     */
    void layerWasAdded( QgsMapLayer *layer );

    /**
     * Emitted, when a layer was added to the registry and the legend.
     * Layers can also be private layers, which are signalled by
     * layersAdded() and layerWasAdded() but will not be
     * advertised by this signal.
     *
     * \param layers List of QgsMapLayer which were added to the legend.
     */
    void legendLayersAdded( const QList<QgsMapLayer *> &layers );



  public slots:

    /**
     * The snapping configuration for this project.
     *
     * \since QGIS 3.0
     */
    void setSnappingConfig( const QgsSnappingConfig &snappingConfig );

    /**
     * Flag the project as dirty (modified). If this flag is set, the user will
     * be asked to save changes to the project before closing the current project.
     *
     * \since QGIS 2.4
     * \note promoted to public slot in 2.16
     */
    void setDirty( bool b = true );

  private slots:
    void onMapLayersAdded( const QList<QgsMapLayer *> &layers );
    void onMapLayersRemoved( const QList<QgsMapLayer *> &layers );
    void cleanTransactionGroups( bool force = false );

  private:

    static QgsProject *sProject;

    /**
     * Read map layers from project file.
     * \param doc DOM document to parse
     * \param brokenNodes a list of DOM nodes corresponding to layers that we were unable to load; this could be
     * because the layers were removed or re-located after the project was last saved
     * \returns true if function worked; else is false
    */
    bool _getMapLayers( const QDomDocument &doc, QList<QDomNode> &brokenNodes );

    /**
     * Set error message from read/write operation
     * \note not available in Python bindings
     */
    void setError( const QString &errorMessage ) SIP_SKIP;

    /**
     * Clear error message
     * \note not available in Python bindings
     */
    void clearError() SIP_SKIP;

    /**
     * Creates layer and adds it to maplayer registry
     * \note not available in Python bindings
     */
    bool addLayer( const QDomElement &layerElem, QList<QDomNode> &brokenNodes, const QgsReadWriteContext &context ) SIP_SKIP;

    //! \note not available in Python bindings
    void initializeEmbeddedSubtree( const QString &projectFilePath, QgsLayerTreeGroup *group ) SIP_SKIP;

    //! \note not available in Python bindings
    void loadEmbeddedNodes( QgsLayerTreeGroup *group ) SIP_SKIP;

    //! Read .qgs file
    bool readProjectFile( const QString &filename );

    //! Write .qgs file
    bool writeProjectFile( const QString &filename );

    //! Unzip .qgz file then read embedded .qgs file
    bool unzip( const QString &filename );

    //! Zip project
    bool zip( const QString &filename );

    //! Save auxiliary storage to database
    bool saveAuxiliaryStorage( const QString &filename = QString() );

    std::unique_ptr< QgsMapLayerStore > mLayerStore;

    QString mErrorMessage;

    QgsProjectBadLayerHandler *mBadLayerHandler = nullptr;

    /**
     * Embedded layers which are defined in other projects. Key: layer id,
     * value: pair< project file path, save layer yes / no (e.g. if the layer is part of an embedded group, loading/saving is done by the legend)
     *  If the project file path is empty, QgsProject is going to ignore the layer for saving (e.g. because it is part and managed by an embedded group)
     */
    QHash< QString, QPair< QString, bool> > mEmbeddedLayers;

    QgsSnappingConfig mSnappingConfig;

    QgsRelationManager *mRelationManager = nullptr;

    std::unique_ptr<QgsAnnotationManager> mAnnotationManager;
    std::unique_ptr<QgsLayoutManager> mLayoutManager;

    QgsLayerTree *mRootGroup = nullptr;

    QgsLayerTreeRegistryBridge *mLayerTreeRegistryBridge = nullptr;

    //! map of transaction group: QPair( providerKey, connString ) -> transactionGroup
    QMap< QPair< QString, QString>, QgsTransactionGroup *> mTransactionGroups;

    std::unique_ptr<QgsMapThemeCollection> mMapThemeCollection;

    std::unique_ptr<QgsLabelingEngineSettings> mLabelingEngineSettings;

    QVariantMap mCustomVariables;

    std::unique_ptr<QgsProjectArchive> mArchive;

    std::unique_ptr<QgsAuxiliaryStorage> mAuxiliaryStorage;

    QFile mFile;                 // current physical project file
    mutable QgsProjectPropertyKey mProperties;  // property hierarchy, TODO: this shouldn't be mutable
    QString mTitle;              // project title
    bool mAutoTransaction = false;       // transaction grouped editing
    bool mEvaluateDefaultValues = false; // evaluate default values immediately
    QgsCoordinateReferenceSystem mCrs;
    bool mDirty = false;                 // project has been modified since it has been read or saved
    bool mTrustLayerMetadata = false;
};

/**
 * Return the version string found in the given DOM document
   \returns the version string or an empty string if none found
   \note not available in Python bindings.
 */
CORE_EXPORT QgsProjectVersion getVersion( QDomDocument const &doc ) SIP_SKIP;

#endif
