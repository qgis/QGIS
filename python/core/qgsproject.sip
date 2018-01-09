/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsproject.h                                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








class QgsProject : QObject, QgsExpressionContextGenerator
{
%Docstring
Reads and writes project states.

\note

Has two general kinds of state to make persistent.  (I.e., to read and
write.)  First, QGIS proprietary information.  Second plug-in information.

A singleton since there shall only be one active project at a time; and
provides canonical location for plug-ins and main app to find/set
properties.
%End

%TypeHeaderCode
#include "qgsproject.h"
%End
  public:
    static QgsProject *instance();
%Docstring
Returns the QgsProject singleton instance
%End

    explicit QgsProject( QObject *parent /TransferThis/ = 0 );
%Docstring
Create a new QgsProject.

Most of the time you want to use QgsProject.instance() instead as many components of QGIS work with the singleton.
%End

    ~QgsProject();

    void setTitle( const QString &title );
%Docstring
Sets the project's title.

:param title: new title

.. versionadded:: 2.4

.. seealso:: :py:func:`title`
%End

    QString title() const;
%Docstring
Returns the project's title.

.. seealso:: :py:func:`setTitle`
%End

    bool isDirty() const;
%Docstring
Returns true if the project has been modified since the last write()
%End

    void setFileName( const QString &name );
%Docstring
Sets the file name associated with the project. This is the file which contains the project's XML
representation.

:param name: project file name

.. seealso:: :py:func:`fileName`
%End

    QString fileName() const;
%Docstring
Returns the project's file name. This is the file which contains the project's XML
representation.

.. seealso:: :py:func:`setFileName`

.. seealso:: :py:func:`fileInfo`
%End

    QFileInfo fileInfo() const;
%Docstring
Returns QFileInfo object for the project's associated file.

.. seealso:: :py:func:`fileName`

.. versionadded:: 2.9
%End

    QgsCoordinateReferenceSystem crs() const;
%Docstring
Returns the project's native coordinate reference system.

.. versionadded:: 3.0

.. seealso:: :py:func:`setCrs`

.. seealso:: :py:func:`ellipsoid`
%End

    void setCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
Sets the project's native coordinate reference system.

.. versionadded:: 3.0

.. seealso:: :py:func:`crs`

.. seealso:: :py:func:`setEllipsoid`
%End

    QString ellipsoid() const;
%Docstring
Returns a proj string representing the project's ellipsoid setting, e.g., "WGS84".

.. seealso:: :py:func:`setEllipsoid`

.. seealso:: :py:func:`crs`

.. versionadded:: 3.0
%End

    void setEllipsoid( const QString &ellipsoid );
%Docstring
Sets the project's ellipsoid from a proj string representation, e.g., "WGS84".

.. seealso:: :py:func:`ellipsoid`

.. seealso:: :py:func:`setCrs`

.. versionadded:: 3.0
%End


    QgsCoordinateTransformContext transformContext() const;
%Docstring
Returns a copy of the project's coordinate transform context, which stores various
information regarding which datum transforms should be used when transforming points
from a source to destination coordinate reference system.

.. versionadded:: 3.0

.. seealso:: :py:func:`setTransformContext`

.. seealso:: :py:func:`transformContextChanged`
%End

    void setTransformContext( const QgsCoordinateTransformContext &context );
%Docstring
Sets the project's coordinate transform ``context``, which stores various
information regarding which datum transforms should be used when transforming points
from a source to destination coordinate reference system.

.. versionadded:: 3.0

.. seealso:: :py:func:`transformContext`

.. seealso:: :py:func:`transformContextChanged`
%End

    void clear();
%Docstring
Clear the project - removes all settings and resets it back to an empty, default state.

.. versionadded:: 2.4
%End

    bool read( const QString &filename );
%Docstring
Reads given project file from the given file.

:param filename: name of project file to read

:return: true if project file has been read successfully
%End

    bool read();
%Docstring
Reads the project from its currently associated file (see fileName() ).

:return: true if project file has been read successfully
%End

    bool readLayer( const QDomNode &layerNode );
%Docstring
Reads the layer described in the associated DOM node.

.. note::

   This method is mainly for use by QgsProjectBadLayerHandler subclasses
that may fix definition of bad layers with the user's help in GUI. Calling
this method with corrected DOM node adds the layer back to the project.

:param layerNode: represents a QgsProject DOM node that encodes a specific layer.
%End

    bool write( const QString &filename );
%Docstring
Writes the project to a file.

:param filename: destination file

:return: true if project was written successfully

.. note::

   calling this implicitly sets the project's filename (see setFileName() )

.. note::

   isDirty() will be set to false if project is successfully written

.. versionadded:: 3.0
%End

    bool write();
%Docstring
Writes the project to its current associated file (see fileName() ).

:return: true if project was written successfully

.. note::

   isDirty() will be set to false if project is successfully written
%End

    bool writeEntry( const QString &scope, const QString &key, bool value ) /PyName=writeEntryBool/;
%Docstring
Write a boolean entry to the project file.

Keys are '/'-delimited entries, implying
a hierarchy of keys and corresponding values

.. note::

   The key string must be valid xml tag names in order to be saved to the file.

.. note::

   available in Python bindings as writeEntryBool
%End

    bool writeEntry( const QString &scope, const QString &key, double value ) /PyName=writeEntryDouble/;
%Docstring
Write a double entry to the project file.

Keys are '/'-delimited entries, implying
a hierarchy of keys and corresponding values

.. note::

   The key string must be valid xml tag names in order to be saved to the file.

.. note::

   available in Python bindings as writeEntryDouble
%End

    bool writeEntry( const QString &scope, const QString &key, int value );
%Docstring
Write an integer entry to the project file.

Keys are '/'-delimited entries, implying
a hierarchy of keys and corresponding values

.. note::

   The key string must be valid xml tag names in order to be saved to the file.
%End

    bool writeEntry( const QString &scope, const QString &key, const QString &value );
%Docstring
Write a string entry to the project file.

Keys are '/'-delimited entries, implying
a hierarchy of keys and corresponding values

.. note::

   The key string must be valid xml tag names in order to be saved to the file.
%End

    bool writeEntry( const QString &scope, const QString &key, const QStringList &value );
%Docstring
Write a string list entry to the project file.

Keys are '/'-delimited entries, implying
a hierarchy of keys and corresponding values

.. note::

   The key string must be valid xml tag names in order to be saved to the file.
%End

    QStringList readListEntry( const QString &scope, const QString &key, const QStringList &def = QStringList(), bool *ok = 0 ) const;
%Docstring
Key value accessors

keys would be the familiar QgsSettings-like '/' delimited entries,
implying a hierarchy of keys and corresponding values
%End

    QString readEntry( const QString &scope, const QString &key, const QString &def = QString(), bool *ok = 0 ) const;
    int readNumEntry( const QString &scope, const QString &key, int def = 0, bool *ok = 0 ) const;
    double readDoubleEntry( const QString &scope, const QString &key, double def = 0, bool *ok = 0 ) const;
    bool readBoolEntry( const QString &scope, const QString &key, bool def = false, bool *ok = 0 ) const;


    bool removeEntry( const QString &scope, const QString &key );
%Docstring
Remove the given key
%End


    QStringList entryList( const QString &scope, const QString &key ) const;
%Docstring
Return keys with values -- do not return keys that contain other keys

.. note::

   equivalent to QgsSettings entryList()
%End

    QStringList subkeyList( const QString &scope, const QString &key ) const;
%Docstring
Return keys with keys -- do not return keys that contain only values

.. note::

   equivalent to QgsSettings subkeyList()
%End


    void dumpProperties() const;

    QgsPathResolver pathResolver() const;
%Docstring
Return path resolver object with considering whether the project uses absolute
or relative paths and using current project's path.

.. versionadded:: 3.0
%End

    QString writePath( const QString &filename ) const;
%Docstring
Prepare a filename to save it to the project file.
Creates an absolute or relative path according to the project settings.
Paths written to the project file should be prepared with this method.
%End

    QString readPath( const QString &filename ) const;
%Docstring
Turn filename read from the project file to an absolute path
%End

    QString error() const;
%Docstring
Return error message from previous read/write
%End

    void setBadLayerHandler( QgsProjectBadLayerHandler *handler /Transfer/ );
%Docstring
Change handler for missing layers.
Deletes old handler and takes ownership of the new one.
%End

    QString layerIsEmbedded( const QString &id ) const;
%Docstring
Returns project file path if layer is embedded from other project file. Returns empty string if layer is not embedded
%End


    QgsLayerTreeGroup *createEmbeddedGroup( const QString &groupName, const QString &projectFilePath, const QStringList &invisibleLayers );
%Docstring
Create layer group instance defined in an arbitrary project file.

.. versionadded:: 2.4
%End

    void setTopologicalEditing( bool enabled );
%Docstring
Convenience function to set topological editing
%End

    bool topologicalEditing() const;
%Docstring
Convenience function to query topological editing status
%End

    QgsUnitTypes::DistanceUnit distanceUnits() const;
%Docstring
Convenience function to query default distance measurement units for project.

.. versionadded:: 2.14

.. seealso:: :py:func:`setDistanceUnits`

.. seealso:: :py:func:`areaUnits`
%End

    void setDistanceUnits( QgsUnitTypes::DistanceUnit unit );
%Docstring
Sets the default distance measurement units for the project.

.. versionadded:: 3.0

.. seealso:: :py:func:`distanceUnits`

.. seealso:: :py:func:`setAreaUnits`
%End

    QgsUnitTypes::AreaUnit areaUnits() const;
%Docstring
Convenience function to query default area measurement units for project.

.. versionadded:: 2.14

.. seealso:: :py:func:`distanceUnits`
%End

    void setAreaUnits( QgsUnitTypes::AreaUnit unit );
%Docstring
Sets the default area measurement units for the project.

.. versionadded:: 3.0

.. seealso:: :py:func:`areaUnits`

.. seealso:: :py:func:`setDistanceUnits`
%End

    QString homePath() const;
%Docstring
Return project's home path

:return: home path of project (or null QString if not set) *
%End

    QgsRelationManager *relationManager() const;


    QgsLayoutManager *layoutManager();
%Docstring
Returns the project's layout manager, which manages compositions within
the project.

.. versionadded:: 3.0
%End

    QgsLayerTree *layerTreeRoot() const;
%Docstring
Return pointer to the root (invisible) node of the project's layer tree

.. versionadded:: 2.4
%End

    QgsLayerTreeRegistryBridge *layerTreeRegistryBridge() const;
%Docstring
Return pointer to the helper class that synchronizes map layer registry with layer tree

.. versionadded:: 2.4
%End

    QgsMapThemeCollection *mapThemeCollection();
%Docstring
Returns pointer to the project's map theme collection.

.. versionadded:: 2.12

.. note::

   renamed in QGIS 3.0, formerly :py:class:`QgsVisibilityPresetCollection`
%End

    QgsAnnotationManager *annotationManager();
%Docstring
Returns pointer to the project's annotation manager.

.. versionadded:: 3.0
%End


    void setNonIdentifiableLayers( const QList<QgsMapLayer *> &layers );
%Docstring
Set a list of layers which should not be taken into account on map identification
%End

    void setNonIdentifiableLayers( const QStringList &layerIds );
%Docstring
Set a list of layers which should not be taken into account on map identification
%End

    QStringList nonIdentifiableLayers() const;
%Docstring
Get the list of layers which currently should not be taken into account on map identification
%End

    bool autoTransaction() const;
%Docstring
Transactional editing means that on supported datasources (postgres databases) the edit state of
all tables that originate from the same database are synchronized and executed in a server side
transaction.

.. versionadded:: 2.16
%End

    void setAutoTransaction( bool autoTransaction );
%Docstring
Transactional editing means that on supported datasources (postgres databases) the edit state of
all tables that originate from the same database are synchronized and executed in a server side
transaction.

Make sure that this is only called when all layers are not in edit mode.

.. versionadded:: 2.16
%End


    bool evaluateDefaultValues() const;
%Docstring
Should default values be evaluated on provider side when requested and not when committed.

.. versionadded:: 2.16
%End


    void setEvaluateDefaultValues( bool evaluateDefaultValues );
%Docstring
Defines if default values should be evaluated on provider side when requested and not when committed.

.. versionadded:: 2.16
%End

    virtual QgsExpressionContext createExpressionContext() const;


    QgsSnappingConfig snappingConfig() const;
%Docstring
The snapping configuration for this project.

.. versionadded:: 3.0
%End

    QList<QgsVectorLayer *> avoidIntersectionsLayers() const;
%Docstring
A list of layers with which intersections should be avoided.

.. versionadded:: 3.0
%End

    void setAvoidIntersectionsLayers( const QList<QgsVectorLayer *> &layers );
%Docstring
A list of layers with which intersections should be avoided.

.. versionadded:: 3.0
%End

    QVariantMap customVariables() const;
%Docstring
A map of custom project variables.
To get all available variables including generated ones
use QgsExpressionContextUtils.projectScope() instead.
%End

    void setCustomVariables( const QVariantMap &customVariables );
%Docstring
A map of custom project variables.
Be careful not to set generated variables.
%End

    void setLabelingEngineSettings( const QgsLabelingEngineSettings &settings );
%Docstring
Sets project's global labeling engine settings

.. versionadded:: 3.0
%End

    const QgsLabelingEngineSettings &labelingEngineSettings() const;
%Docstring
Returns project's global labeling engine settings

.. versionadded:: 3.0
%End


    QgsMapLayerStore *layerStore();
%Docstring
Returns a pointer to the project's internal layer store.
/since QGIS 3.0
%End


    int count() const;
%Docstring
Returns the number of registered layers.
%End

    QgsMapLayer *mapLayer( const QString &layerId ) const;
%Docstring
Retrieve a pointer to a registered layer by layer ID.

:param layerId: ID of layer to retrieve

:return: matching layer, or None if no matching layer found

.. seealso:: :py:func:`mapLayersByName`

.. seealso:: :py:func:`mapLayers`
%End

    QList<QgsMapLayer *> mapLayersByName( const QString &layerName ) const;
%Docstring
Retrieve a list of matching registered layers by layer name.

:param layerName: name of layers to match

:return: list of matching layers

.. seealso:: :py:func:`mapLayer`

.. seealso:: :py:func:`mapLayers`
%End

    QMap<QString, QgsMapLayer *> mapLayers() const;
%Docstring
Returns a map of all registered layers by layer ID.

.. seealso:: :py:func:`mapLayer`

.. seealso:: :py:func:`mapLayersByName`

.. seealso:: :py:func:`layers`
%End

    bool isZipped() const;
%Docstring
Returns true if the project comes from a zip archive, false otherwise.
%End


    QList<QgsMapLayer *> addMapLayers( const QList<QgsMapLayer *> &mapLayers /Transfer/,
                                       bool addToLegend = true);

%Docstring
Add a list of layers to the map of loaded layers.

The layersAdded() and layerWasAdded() signals will always be emitted.
The legendLayersAdded() signal is emitted only if addToLegend is true.

:param mapLayers:  A list of layer which should be added to the registry
:param addToLegend:   If true (by default), the layers will be added to the
legend and to the main canvas. If you have a private
layer you can set this parameter to false to hide it.
:param takeOwnership: Ownership will be transferred to the layer registry.
If you specify false here you have take care of deleting
the layers yourself. Not available in Python.

:return: a list of the map layers that were added
successfully. If a layer is invalid, or already exists in the registry,
it will not be part of the returned QList.


.. note::

   As a side-effect QgsProject is made dirty.

.. note::

   takeOwnership is not available in the Python bindings - the registry will always
take ownership

.. versionadded:: 1.8

.. seealso:: :py:func:`addMapLayer`
%End

    QgsMapLayer *addMapLayer( QgsMapLayer *mapLayer /Transfer/,
                              bool addToLegend = true);

%Docstring
Add a layer to the map of loaded layers.

The layersAdded() and layerWasAdded() signals will always be emitted.
The legendLayersAdded() signal is emitted only if addToLegend is true.
If you are adding multiple layers at once, you should use
addMapLayers() instead.

:param mapLayer: A layer to add to the registry
:param addToLegend: If true (by default), the layer will be added to the
legend and to the main canvas. If you have a private
layer you can set this parameter to false to hide it.
:param takeOwnership: Ownership will be transferred to the layer registry.
If you specify false here you have take care of deleting
the layer yourself. Not available in Python.

:return: None if unable to add layer, otherwise pointer to newly added layer


.. seealso:: :py:func:`addMapLayers`


.. note::

   As a side-effect QgsProject is made dirty.

.. note::

   Use addMapLayers if adding more than one layer at a time

.. note::

   takeOwnership is not available in the Python bindings - the registry will always
take ownership

.. seealso:: :py:func:`addMapLayers`
%End

    void removeMapLayers( const QStringList &layerIds );
%Docstring
Remove a set of layers from the registry by layer ID.

The specified layers will be removed from the registry. If the registry has ownership
of any layers these layers will also be deleted.

:param layerIds: list of IDs of the layers to remove

.. note::

   As a side-effect the QgsProject instance is marked dirty.

.. versionadded:: 1.8

.. seealso:: :py:func:`removeMapLayer`

.. seealso:: :py:func:`removeAllMapLayers`
%End

    void removeMapLayers( const QList<QgsMapLayer *> &layers );

    void removeMapLayer( const QString &layerId );
%Docstring
Remove a layer from the registry by layer ID.

The specified layer will be removed from the registry. If the registry has ownership
of the layer then it will also be deleted.

:param layerId: ID of the layer to remove

.. note::

   As a side-effect the QgsProject instance is marked dirty.

.. seealso:: :py:func:`removeMapLayers`

.. seealso:: :py:func:`removeAllMapLayers`
%End

    void removeMapLayer( QgsMapLayer *layer );
%Docstring
Remove a layer from the registry.

The specified layer will be removed from the registry. If the registry has ownership
of the layer then it will also be deleted.

:param layer: The layer to remove. Null pointers are ignored.

.. note::

   As a side-effect the QgsProject instance is marked dirty.

.. seealso:: :py:func:`removeMapLayers`

.. seealso:: :py:func:`removeAllMapLayers`
%End

    QgsMapLayer *takeMapLayer( QgsMapLayer *layer ) /TransferBack/;
%Docstring
Takes a layer from the registry. If the layer was owned by the project, the
layer will be returned without deleting it. The caller takes ownership of
the layer and is responsible for deleting it.

.. seealso:: :py:func:`removeMapLayer`

.. versionadded:: 3.0
%End

    void removeAllMapLayers();
%Docstring
Removes all registered layers. If the registry has ownership
of any layers these layers will also be deleted.

.. note::

   As a side-effect the QgsProject instance is marked dirty.

.. note::

   Calling this method will cause the removeAll() signal to
be emitted.

.. seealso:: :py:func:`removeMapLayer`

.. seealso:: :py:func:`removeMapLayers`
%End

    void reloadAllLayers();
%Docstring
Reload all registered layer's provider data caches, synchronising the layer
with any changes in the datasource.

.. seealso:: :py:func:`QgsMapLayer.reload`
%End

    QgsCoordinateReferenceSystem defaultCrsForNewLayers() const;
%Docstring
Returns the default CRS for new layers based on the settings and
the current project CRS
%End

    void setTrustLayerMetadata( bool trust );
%Docstring
Sets the trust option allowing to indicate if the extent has to be
read from the XML document when data source has no metadata or if the
data provider has to determine it. Moreover, when this option is
activated, primary key unicity is not checked for views and
materialized views with Postgres provider.

:param trust: True to trust the project, false otherwise

.. versionadded:: 3.0
%End

    bool trustLayerMetadata() const;
%Docstring
Returns true if the trust option is activated, false otherwise. This
option allows indicateing if the extent has to be read from the XML
document when data source has no metadata or if the data provider has
to determine it. Moreover, when this option is activated, primary key
unicity is not checked for views and materialized views with Postgres
provider.

.. versionadded:: 3.0
%End


    QgsAuxiliaryStorage *auxiliaryStorage();
%Docstring
Returns the current auxiliary storage.

.. versionadded:: 3.0
%End

  signals:
    void readProject( const QDomDocument & );
%Docstring
emitted when project is being read
%End

    void writeProject( QDomDocument & );
%Docstring
emitted when project is being written
%End

    void readMapLayer( QgsMapLayer *mapLayer, const QDomElement &layerNode );
%Docstring
Emitted, after the basic initialization of a layer from the project
file is done. You can use this signal to read additional information
from the project file.

:param mapLayer:  The map layer which is being initialized
:param layerNode: The layer node from the project file
%End

    void writeMapLayer( QgsMapLayer *mapLayer, QDomElement &layerElem, QDomDocument &doc );
%Docstring
Emitted, when a layer is being saved. You can use this method to save
additional information to the layer.

:param mapLayer:  The map layer which is being initialized
:param layerElem: The layer element from the project file
:param doc: The document
%End

    void projectSaved();
%Docstring
emitted when the project file has been written and closed
%End

    void oldProjectVersionWarning( const QString & );
%Docstring
emitted when an old project file is read.
%End

    void layerLoaded( int i, int n );
%Docstring
Emitted when a layer from a projects was read.

:param i: current layer
:param n: number of layers
%End

    void loadingLayer( const QString & );

    void nonIdentifiableLayersChanged( QStringList nonIdentifiableLayers );
%Docstring
Emitted when the list of layer which are excluded from map identification changes
%End

    void fileNameChanged();
%Docstring
Emitted when the file name of the project changes
%End

    void homePathChanged();
%Docstring
Emitted when the home path of the project changes
%End

    void snappingConfigChanged( const QgsSnappingConfig &config );
%Docstring
emitted whenever the configuration for snapping has changed
%End

    void customVariablesChanged();
%Docstring
Emitted whenever the expression variables stored in the project have been changed.

.. versionadded:: 3.0
%End

    void crsChanged();
%Docstring
Emitted when the CRS of the project has changed.

.. versionadded:: 3.0
%End

    void ellipsoidChanged( const QString &ellipsoid );
%Docstring
Emitted when the project ``ellipsoid`` is changed.

.. versionadded:: 3.0

.. seealso:: :py:func:`setEllipsoid`

.. seealso:: :py:func:`ellipsoid`
%End


    void transformContextChanged();
%Docstring
Emitted when the project transformContext() is changed.

.. versionadded:: 3.0

.. seealso:: :py:func:`transformContext`
%End

    void missingDatumTransforms( const QStringList &missingTransforms );
%Docstring
Emitted when datum transforms stored in the project are not available locally.

.. versionadded:: 3.0
%End

    void transactionGroupsChanged();
%Docstring
Emitted whenever a new transaction group has been created or a
transaction group has been removed.

.. versionadded:: 3.0
%End

    void topologicalEditingChanged();
%Docstring
Emitted when the topological editing flag has changed.

.. versionadded:: 3.0
%End

    void avoidIntersectionsLayersChanged();
%Docstring
Emitted whenever avoidIntersectionsLayers has changed.

.. versionadded:: 3.0
%End

    void mapThemeCollectionChanged();
%Docstring
Emitted when the map theme collection changes.
This only happens when the map theme collection is reset.
Any pointer previously received from mapThemeCollection()
must no longer be used after this signal is emitted.
You must still connect to signals from the map theme collection
if you want to be notified about new map themes being added and
map themes being removed.

.. versionadded:: 3.0
%End

    void labelingEngineSettingsChanged();
%Docstring
Emitted when global configuration of the labeling engine changes.

.. versionadded:: 3.0
%End


    void layersWillBeRemoved( const QStringList &layerIds );
%Docstring
Emitted when one or more layers are about to be removed from the registry.

:param layerIds: A list of IDs for the layers which are to be removed.

.. seealso:: :py:func:`layerWillBeRemoved`

.. seealso:: :py:func:`layersRemoved`
%End

    void layersWillBeRemoved( const QList<QgsMapLayer *> &layers );
%Docstring
Emitted when one or more layers are about to be removed from the registry.

:param layers: A list of layers which are to be removed.

.. seealso:: :py:func:`layerWillBeRemoved`

.. seealso:: :py:func:`layersRemoved`
%End

    void layerWillBeRemoved( const QString &layerId );
%Docstring
Emitted when a layer is about to be removed from the registry.

:param layerId: The ID of the layer to be removed.

.. note::

   Consider using layersWillBeRemoved() instead

.. seealso:: :py:func:`layersWillBeRemoved`

.. seealso:: :py:func:`layerRemoved`
%End

    void layerWillBeRemoved( QgsMapLayer *layer );
%Docstring
Emitted when a layer is about to be removed from the registry.

:param layer: The layer to be removed.

.. note::

   Consider using layersWillBeRemoved() instead

.. seealso:: :py:func:`layersWillBeRemoved`

.. seealso:: :py:func:`layerRemoved`
%End

    void layersRemoved( const QStringList &layerIds );
%Docstring
Emitted after one or more layers were removed from the registry.

:param layerIds:  A list of IDs of the layers which were removed.

.. seealso:: :py:func:`layersWillBeRemoved`
%End

    void layerRemoved( const QString &layerId );
%Docstring
Emitted after a layer was removed from the registry.

:param layerId: The ID of the layer removed.

.. note::

   Consider using layersRemoved() instead

.. seealso:: :py:func:`layerWillBeRemoved`
%End

    void removeAll();

    void layersAdded( const QList<QgsMapLayer *> &layers );
%Docstring
Emitted when one or more layers were added to the registry.
This signal is also emitted for layers added to the registry,
but not to the legend.

:param layers: List of layers which have been added.

.. seealso:: :py:func:`legendLayersAdded`

.. seealso:: :py:func:`layerWasAdded`
%End

    void layerWasAdded( QgsMapLayer *layer );
%Docstring
Emitted when a layer was added to the registry.

.. note::

   Consider using layersAdded() instead

.. seealso:: :py:func:`layersAdded`
%End

    void legendLayersAdded( const QList<QgsMapLayer *> &layers );
%Docstring
Emitted, when a layer was added to the registry and the legend.
Layers can also be private layers, which are signalled by
layersAdded() and layerWasAdded() but will not be
advertised by this signal.

:param layers: List of QgsMapLayer which were added to the legend.
%End



  public slots:

    void setSnappingConfig( const QgsSnappingConfig &snappingConfig );
%Docstring
The snapping configuration for this project.

.. versionadded:: 3.0
%End

    void setDirty( bool b = true );
%Docstring
Flag the project as dirty (modified). If this flag is set, the user will
be asked to save changes to the project before closing the current project.

.. versionadded:: 2.4

.. note::

   promoted to public slot in 2.16
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsproject.h                                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
