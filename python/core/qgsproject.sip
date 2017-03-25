
/** \ingroup core
 * Reads and writes project states.


  @note

  Has two general kinds of state to make persistent.  (I.e., to read and
  write.)  First, QGIS proprietary information.  Second plug-in information.

  A singleton since there shall only be one active project at a time; and
  provides canonical location for plug-ins and main app to find/set
  properties.

*/
class QgsProject : QObject, QgsExpressionContextGenerator
{
%TypeHeaderCode
#include <qgsproject.h>
%End

  public:
    //! Returns the QgsProject singleton instance
    static QgsProject* instance();

    /**
     * Create a new QgsProject.
     *
     * Most of the time you want to use QgsProject::instance() instead as many components of QGIS work with the singleton.
     */
    explicit QgsProject( QObject* parent /TransferThis/ = nullptr );

    ~QgsProject();

    /** Sets the project's title.
     * @param title new title
     * @note added in 2.4
     * @see title()
     */
    void setTitle( const QString& title );

    /** Returns the project's title.
     * @see setTitle()
    */
    QString title() const;

    /**
     * Returns true if the project has been modified since the last write()
     */
    bool isDirty() const;

    /** Sets the file name associated with the project. This is the file which contains the project's XML
     * representation.
     * @param name project file name
     * @see fileName()
     */
    void setFileName( const QString& name );

    /** Returns the project's file name. This is the file which contains the project's XML
     * representation.
     * @see setFileName()
     * @see fileInfo()
    */
    QString fileName() const;

    /** Returns QFileInfo object for the project's associated file.
     * @see fileName()
     * @note added in QGIS 2.9
     */
    QFileInfo fileInfo() const;

    /**
     * Returns the project's native coordinate reference system.
     * @note added in QGIS 3.0
     * @see setCrs()
     * @see ellipsoid()
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Sets the project's native coordinate reference system.
     * @note added in QGIS 3.0
     * @see crs()
     * @see setEllipsoid()
     */
    void setCrs( const QgsCoordinateReferenceSystem& crs );

    /**
     * Returns a proj string representing the project's ellipsoid setting, e.g., "WGS84".
     * @see setEllipsoid()
     * @see crs()
     * @note added in QGIS 3.0
     */
    QString ellipsoid() const;

    /**
     * Sets the project's ellipsoid from a proj string representation, e.g., "WGS84".
     * @see ellipsoid()
     * @see setCrs()
     * @note added in QGIS 3.0
     */
    void setEllipsoid( const QString& ellipsoid );

    /** Clear the project - removes all settings and resets it back to an empty, default state.
     * @note added in 2.4
     */
    void clear();

    /** Reads given project file from the given file.
     * @param filename name of project file to read
     * @returns true if project file has been read successfully
     */
    bool read( const QString& filename );

    /** Reads the project from its currently associated file (see fileName() ).
     * @returns true if project file has been read successfully
     */
    bool read();

    /** Reads the layer described in the associated DOM node.
     *
     * @note This method is mainly for use by QgsProjectBadLayerHandler subclasses
     * that may fix definition of bad layers with the user's help in GUI. Calling
     * this method with corrected DOM node adds the layer back to the project.
     *
     * @param layerNode represents a QgsProject DOM node that encodes a specific layer.
     */
    bool readLayer( const QDomNode& layerNode );

    /**
     * Writes the project to a file.
     * @param filename destination file
     * @note calling this implicitly sets the project's filename (see setFileName() )
     * @note isDirty() will be set to false if project is successfully written
     * @returns true if project was written successfully
     *
     * \note Added in QGIS 3.0
     */
    bool write( const QString& filename );

    /** Writes the project to its current associated file (see fileName() ).
     * @note isDirty() will be set to false if project is successfully written
     * @returns true if project was written successfully
     */
    bool write();

    /**
     * Write a boolean entry to the project file.
     *
     * Keys are '/'-delimited entries, implying
     * a hierarchy of keys and corresponding values
     *
     * @note The key string must be valid xml tag names in order to be saved to the file.
     * @note available in python bindings as writeEntryBool
     */
    bool writeEntry( const QString& scope, const QString& key, bool value ) /PyName=writeEntryBool/;

    /**
     * Write a double entry to the project file.
     *
     * Keys are '/'-delimited entries, implying
     * a hierarchy of keys and corresponding values
     *
     * @note The key string must be valid xml tag names in order to be saved to the file.
     * @note available in python bindings as writeEntryDouble
     */
    bool writeEntry( const QString& scope, const QString& key, double value ) /PyName=writeEntryDouble/;

    /**
     * Write an integer entry to the project file.
     *
     * Keys are '/'-delimited entries, implying
     * a hierarchy of keys and corresponding values
     *
     * @note The key string must be valid xml tag names in order to be saved to the file.
     */
    bool writeEntry( const QString& scope, const QString& key, int value );

    /**
     * Write a string entry to the project file.
     *
     * Keys are '/'-delimited entries, implying
     * a hierarchy of keys and corresponding values
     *
     * @note The key string must be valid xml tag names in order to be saved to the file.
     */
    bool writeEntry( const QString& scope, const QString& key, const QString& value );

    /**
     * Write a string list entry to the project file.
     *
     * Keys are '/'-delimited entries, implying
     * a hierarchy of keys and corresponding values
     *
     * @note The key string must be valid xml tag names in order to be saved to the file.
     */
    bool writeEntry( const QString& scope, const QString& key, const QStringList& value );

    /**
     * Key value accessors
     *
     * keys would be the familiar QgsSettings-like '/' delimited entries,
     * implying a hierarchy of keys and corresponding values
     */
    QStringList readListEntry( const QString& scope, const QString& key, const QStringList& def = QStringList(), bool *ok = 0 ) const;

    QString readEntry( const QString& scope, const QString& key, const QString& def = QString::null, bool* ok = 0 ) const;
    int readNumEntry( const QString& scope, const QString& key, int def = 0, bool* ok = 0 ) const;
    double readDoubleEntry( const QString& scope, const QString& key, double def = 0, bool* ok = 0 ) const;
    bool readBoolEntry( const QString& scope, const QString& key, bool def = false, bool* ok = 0 ) const;

    /** Remove the given key */
    bool removeEntry( const QString& scope, const QString& key );


    /** Return keys with values -- do not return keys that contain other keys
     *
     * @note equivalent to QgsSettings entryList()
     */
    QStringList entryList( const QString& scope, const QString& key ) const;

    /** Return keys with keys -- do not return keys that contain only values
     *
     * @note equivalent to QgsSettings subkeyList()
     */
    QStringList subkeyList( const QString& scope, const QString& key ) const;


    /** Dump out current project properties to stderr
     *
     * @todo XXX Now slightly broken since re-factoring.  Won't print out top-level key
     *           and redundantly prints sub-keys.
     */
    void dumpProperties() const;

    QgsPathResolver pathResolver() const;

    /**
     * Prepare a filename to save it to the project file.
     * Creates an absolute or relative path according to the project settings.
     * Paths written to the project file should be prepared with this method.
    */
    QString writePath( const QString& filename ) const;

    /** Turn filename read from the project file to an absolute path */
    QString readPath( QString filename ) const;

    /** Return error message from previous read/write */
    QString error() const;

    /** Change handler for missing layers.
     * Deletes old handler and takes ownership of the new one.
     */
    void setBadLayerHandler( QgsProjectBadLayerHandler* handler /Transfer/ );

    /** Returns project file path if layer is embedded from other project file. Returns empty string if layer is not embedded*/
    QString layerIsEmbedded( const QString& id ) const;

    /** Creates a maplayer instance defined in an arbitrary project file. Caller takes ownership
     * @return the layer or 0 in case of error
     * @note not available in Python bindings
     */
    /*
    bool createEmbeddedLayer( const QString& layerId, const QString& projectFilePath, QList<QDomNode>& brokenNodes,
                              QList< QPair< QgsVectorLayer*, QDomElement > >& vectorLayerList, bool saveFlag = true );
    */

    /** Create layer group instance defined in an arbitrary project file.
     * @note: added in version 2.4
     */
    QgsLayerTreeGroup* createEmbeddedGroup( const QString& groupName, const QString& projectFilePath, const QStringList &invisibleLayers );

    /** Convenience function to set topological editing */
    void setTopologicalEditing( bool enabled );

    /** Convenience function to query topological editing status */
    bool topologicalEditing() const;

    /** Convenience function to query default distance measurement units for project.
     * @note added in QGIS 2.14
     * @see setDistanceUnits()
     * @see areaUnits()
     */
    QgsUnitTypes::DistanceUnit distanceUnits() const;

    /**
     * Sets the default distance measurement units for the project.
     * @note added in QGIS 3.0
     * @see distanceUnits()
     * @see setAreaUnits()
     */
    void setDistanceUnits( QgsUnitTypes::DistanceUnit unit );

    /** Convenience function to query default area measurement units for project.
     * @note added in QGIS 2.14
     * @see distanceUnits()
     */
    QgsUnitTypes::AreaUnit areaUnits() const;

    /**
     * Sets the default area measurement units for the project.
     * @note added in QGIS 3.0
     * @see areaUnits()
     * @see setDistanceUnits()
     */
    void setAreaUnits( QgsUnitTypes::AreaUnit unit );

    /** Return project's home path
      @return home path of project (or QString::null if not set) */
    QString homePath() const;

    QgsRelationManager *relationManager() const;

    //const QgsLayoutManager *layoutManager() const;
    QgsLayoutManager *layoutManager();
    QgsLayerTreeGroup *layerTreeRoot() const;

    /** Return pointer to the helper class that synchronizes map layer registry with layer tree
     * @note added in 2.4
     */
    QgsLayerTreeRegistryBridge* layerTreeRegistryBridge() const;

    /** Returns pointer to the project's map theme collection.
     * @note added in QGIS 2.12
     * @note renamed in QGIS 3.0, formerly QgsVisibilityPresetCollection
     */
    QgsMapThemeCollection* mapThemeCollection();

    QgsAnnotationManager* annotationManager();

    /**
     * Set a list of layers which should not be taken into account on map identification
     */
    void setNonIdentifiableLayers( const QList<QgsMapLayer*>& layers );

    /**
     * Set a list of layers which should not be taken into account on map identification
     */
    void setNonIdentifiableLayers( const QStringList& layerIds );

    /**
     * Get the list of layers which currently should not be taken into account on map identification
     */
    QStringList nonIdentifiableLayers() const;

    /**
     * Transactional editing means that on supported datasources (postgres databases) the edit state of
     * all tables that originate from the same database are synchronized and executed in a server side
     * transaction.
     *
     * @note Added in QGIS 2.16
     */
    bool autoTransaction() const;

    /**
     * Transactional editing means that on supported datasources (postgres databases) the edit state of
     * all tables that originate from the same database are synchronized and executed in a server side
     * transaction.
     *
     * Make sure that this is only called when all layers are not in edit mode.
     *
     * @note Added in QGIS 2.16
     */
    void setAutoTransaction( bool autoTransaction );

    /**
     * Should default values be evaluated on provider side when requested and not when committed.
     *
     * @note added in 2.16
     */
    bool evaluateDefaultValues() const;


    /**
     * Defines if default values should be evaluated on provider side when requested and not when committed.
     *
     * @note added in 2.16
     */
    void setEvaluateDefaultValues( bool evaluateDefaultValues );

    QgsExpressionContext createExpressionContext() const;

    /**
     * The snapping configuration for this project.
     *
     * @note Added in QGIS 3.0
     */
    QgsSnappingConfig snappingConfig() const;

    /**
     * The snapping configuration for this project.
     *
     * @note Added in QGIS 3.0
     */
    void setSnappingConfig( const QgsSnappingConfig& snappingConfig );

    /**
     * A list of layers with which intersections should be avoided.
     *
     * @note Added in QGIS 3.0
     */
    QList<QgsVectorLayer*> avoidIntersectionsLayers() const;

    /**
     * A list of layers with which intersections should be avoided.
     *
     * @note Added in QGIS 3.0
     */
    void setAvoidIntersectionsLayers( const QList<QgsVectorLayer*>& layers );
    QVariantMap customVariables() const;
    void setCustomVariables( const QVariantMap& customVariables );
    int count() const;

    QgsMapLayer* mapLayer( const QString& layerId ) const;

    /** Retrieve a list of matching registered layers by layer name.
     * @param layerName name of layers to match
     * @returns list of matching layers
     * @see mapLayer()
     * @see mapLayers()
     */
    QList<QgsMapLayer *> mapLayersByName( const QString& layerName ) const;

    /** Returns a map of all registered layers by layer ID.
     * @see mapLayer()
     * @see mapLayersByName()
     * @see layers()
     */
    QMap<QString, QgsMapLayer*> mapLayers() const;

    QList<QgsMapLayer *> addMapLayers( const QList<QgsMapLayer *>& layers /Transfer/,
                                       bool addToLegend = true );

    QgsMapLayer* addMapLayer( QgsMapLayer * layer /Transfer/, bool addToLegend = true );

    void removeMapLayers( const QStringList& layerIds );

    /**
     * @brief
     * Remove a set of layers from the registry.
     *
     * The specified layers will be removed from the registry. If the registry has ownership
     * of any layers these layers will also be deleted.
     *
     * @param layers A list of layers to remove. Null pointers are ignored.
     *
     * @note As a side-effect the QgsProject instance is marked dirty.
     * @see removeMapLayer()
     * @see removeAllMapLayers()
     */
    void removeMapLayers( const QList<QgsMapLayer*>& layers );

    void removeMapLayer( const QString& layerId );

    /**
     * @brief
     * Remove a layer from the registry.
     *
     * The specified layer will be removed from the registry. If the registry has ownership
     * of the layer then it will also be deleted.
     *
     * @param layer The layer to remove. Null pointers are ignored.
     *
     * @note As a side-effect the QgsProject instance is marked dirty.
     * @see removeMapLayers()
     * @see removeAllMapLayers()
     */
    void removeMapLayer( QgsMapLayer* layer );

    /**
     * Removes all registered layers. If the registry has ownership
     * of any layers these layers will also be deleted.
     *
     * @note As a side-effect the QgsProject instance is marked dirty.
     * @note Calling this method will cause the removeAll() signal to
     * be emitted.
     * @see removeMapLayer()
     * @see removeMapLayers()
     */
    void removeAllMapLayers();

    /**
     * Reload all registered layer's provider data caches, synchronising the layer
     * with any changes in the datasource.
     * @see QgsMapLayer::reload()
     */
    void reloadAllLayers();

  signals:
    //! emitted when project is being read
    void readProject( const QDomDocument& );

    //! emitted when project is being written
    void writeProject( QDomDocument& );

    /**
     * Emitted, after the basic initialization of a layer from the project
     * file is done. You can use this signal to read additional information
     * from the project file.
     *
     * @param mapLayer  The map layer which is being initialized
     * @param layerNode The layer node from the project file
     */
    void readMapLayer( QgsMapLayer* mapLayer, const QDomElement& layerNode );

    /**
     * Emitted, when a layer is being saved. You can use this method to save
     * additional information to the layer.
     *
     * @param mapLayer  The map layer which is being initialized
     * @param layerElem The layer element from the project file
     * @param doc The document
     */
    void writeMapLayer( QgsMapLayer *mapLayer, QDomElement &layerElem, QDomDocument &doc );

    //! emitted when the project file has been written and closed
    void projectSaved();

    //! emitted when an old project file is read.
    void oldProjectVersionWarning( const QString& );

    /**
     * Emitted when a layer from a projects was read.
     * @param i current layer
     * @param n number of layers
     */
    void layerLoaded( int i, int n );

    void loadingLayer( const QString& );

    //! Emitted when the list of layer which are excluded from map identification changes
    void nonIdentifiableLayersChanged( QStringList nonIdentifiableLayers );

    //! Emitted when the file name of the project changes
    void fileNameChanged();

    //! Emitted when the home path of the project changes
    void homePathChanged();

    //! emitted whenever the configuration for snapping has changed
    void snappingConfigChanged( const QgsSnappingConfig& config );

    /** Emitted whenever the expression variables stored in the project have been changed.
     * @note added in QGIS 3.0
     */
    void customVariablesChanged();

    void crsChanged();

    /**
     * Emitted whenever a new transaction group has been created or a
     * transaction group has been removed.
     *
     * @note Added in QGIS 3.0
     */
    void transactionGroupsChanged();

    /**
     * Emitted when the topological editing flag has changed.
     *
     * @note Added in QGIS 3.0
     */
    void topologicalEditingChanged();

    /**
     * Emitted whenever avoidIntersectionsLayers has changed.
     *
     * @note Added in QGIS 3.0
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
     * @note Added in QGIS 3.0
     */
    void mapThemeCollectionChanged();

    //
    // signals from QgsMapLayerRegistry
    //

    void layersWillBeRemoved( const QStringList& layerIds );

    /**
     * Emitted when one or more layers are about to be removed from the registry.
     *
     * @param layers A list of layers which are to be removed.
     * @see layerWillBeRemoved()
     * @see layersRemoved()
     */
    void layersWillBeRemoved( const QList<QgsMapLayer*>& layers );

    void layerWillBeRemoved( const QString& layerId );

    /**
     * Emitted when a layer is about to be removed from the registry.
     *
     * @param layer The layer to be removed.
     *
     * @note Consider using {@link layersWillBeRemoved()} instead
     * @see layersWillBeRemoved()
     * @see layerRemoved()
     */
    void layerWillBeRemoved( QgsMapLayer* layer );

    void layersRemoved( const QStringList& layerIds );

    void layerRemoved( const QString& layerId );

    /**
     * Emitted when all layers are removed, before {@link layersWillBeRemoved()} and
     * {@link layerWillBeRemoved()} signals are emitted. The layersWillBeRemoved() and
     * layerWillBeRemoved() signals will still be emitted following this signal.
     * You can use this signal to do easy (and fast) cleanup.
     */
    //TODO QGIS 3.0 - rename to past tense
    void removeAll();
    void layersAdded( const QList<QgsMapLayer *> &layers );

    void layerWasAdded( QgsMapLayer *layer );
    void legendLayersAdded( const QList<QgsMapLayer *> &layers );


  public slots:
    /**
     * Flag the project as dirty (modified). If this flag is set, the user will
     * be asked to save changes to the project before closing the current project.
     *
     * @note added in 2.4
     * @note promoted to public slot in 2.16
     */
    void setDirty( bool b = true );

};
