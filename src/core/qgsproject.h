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

#include <memory>
#include "qgsprojectversion.h"
#include <QHash>
#include <QList>
#include <QObject>
#include <QPair>
#include <QFileInfo>

//for the snap settings
#include "qgssnapper.h"
#include "qgstolerance.h"
#include "qgsunittypes.h"

//#include <QDomDocument>

class QFileInfo;
class QDomDocument;
class QDomElement;
class QDomNode;

class QgsLayerTreeGroup;
class QgsLayerTreeRegistryBridge;
class QgsMapLayer;
class QgsProjectBadLayerHandler;
class QgsRelationManager;
class QgsVectorLayer;
class QgsVisibilityPresetCollection;
class QgsTransactionGroup;

/** \ingroup core
 * Reads and writes project states.
 *
  @note

  Has two general kinds of state to make persistent.  (I.e., to read and
  write.)  First, QGIS proprietary information.  Second plug-in information.

  A singleton since there shall only be one active project at a time; and
  provides canonical location for plug-ins and main app to find/set
  properties.

*/

// TODO Might want to consider moving from Singleton; i.e., allowing more than one
// project.  Just as the GIMP can have simultaneous multiple images, perhaps
// QGIS can one day have simultaneous multiple projects.

class CORE_EXPORT QgsProject : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QStringList nonIdentifiableLayers READ nonIdentifiableLayers WRITE setNonIdentifiableLayers NOTIFY nonIdentifiableLayersChanged )

  public:

    // TODO XXX Should have semantics for saving project if dirty as last gasp?
    ~QgsProject();

    //! Returns the QgsProject singleton instance
    static QgsProject * instance();

    /**
     * Every project has an associated title string
     *
     * @deprecated Use setTitle instead.
     */
    Q_DECL_DEPRECATED inline void title( const QString & title ) { setTitle( title ); }

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

    /**
     * Flag the project as dirty (modified). If this flag is set, the user will
     * be asked to save changes to the project before closing the current project.
     *
     * @deprecated use setDirty instead
     */
    Q_DECL_DEPRECATED inline void dirty( bool b ) { setDirty( b ); }

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

    /** Clear the project - removes all settings and resets it back to an empty, default state.
     * @note added in 2.4
     */
    void clear();

    /** Reads a project file.
     * @param file name of project file to read
     * @note Any current plug-in state is erased
     * @note Calling read() performs the following operations:
     *
     * - Gets the extents
     * - Creates maplayers
     * - Registers maplayers
     *
     * @note it's presumed that the caller has already reset the map canvas, map registry, and legend
     */
    bool read( const QFileInfo& file );

    /** Reads the current project file.
     * @note Any current plug-in state is erased
     * @note Calling read() performs the following operations:
     *
     * - Gets the extents
     * - Creates maplayers
     * - Registers maplayers
     *
     * @note it's presumed that the caller has already reset the map canvas, map registry, and legend
     */
    bool read();

    /** Reads the layer described in the associated DOM node.
     *
     * @param layerNode represents a QgsProject DOM node that encodes a specific layer.
     *
     * QgsProject raises an exception when one of the QgsProject::read()
     * implementations fails.  Since the read()s are invoked from qgisapp,
     * then qgisapp handles the exception.  It prompts the user for the new
     * location of the data, if any.  If there is a new location, the DOM
     * node associated with the layer has its datasource tag corrected.
     * Then that node is passed to this member function to be re-opened.
     *
     */
    bool read( QDomNode& layerNode );

    /** Writes the project to a file.
     * @param file destination file
     * @note calling this implicitly sets the project's filename (see setFileName() )
     * @note isDirty() will be set to false if project is successfully written
     * @returns true if project was written successfully
     */
    bool write( const QFileInfo& file );

    /** Writes the project to its current associated file (see fileName() ).
     * @note isDirty() will be set to false if project is successfully written
     * @returns true if project was written successfully
     */
    bool write();

    /**
     * Removes all project properties.
     *
     * @deprecated use clear() instead
     */
    Q_DECL_DEPRECATED void clearProperties();


    /* key value mutators
     *
     * keys would be the familiar QSettings-like '/' delimited entries, implying
     * a hierarchy of keys and corresponding values
     *
     * @note The key string must be valid xml tag names in order to be saved to the file.
     */
    //! @note available in python bindings as writeEntryBool
    bool writeEntry( const QString & scope, const QString & key, bool value );
    //! @note available in python bindings as writeEntryDouble
    bool writeEntry( const QString & scope, const QString & key, double value );
    bool writeEntry( const QString & scope, const QString & key, int value );
    bool writeEntry( const QString & scope, const QString & key, const QString & value );
    bool writeEntry( const QString & scope, const QString & key, const QStringList & value );

    /** Key value accessors
     *
     * keys would be the familiar QSettings-like '/' delimited entries,
     * implying a hierarchy of keys and corresponding values
     *
     */
    QStringList readListEntry( const QString & scope, const QString & key, const QStringList& def = QStringList(), bool *ok = nullptr ) const;

    QString readEntry( const QString & scope, const QString & key, const QString & def = QString::null, bool * ok = nullptr ) const;
    int readNumEntry( const QString & scope, const QString & key, int def = 0, bool * ok = nullptr ) const;
    double readDoubleEntry( const QString & scope, const QString & key, double def = 0, bool * ok = nullptr ) const;
    bool readBoolEntry( const QString & scope, const QString & key, bool def = false, bool * ok = nullptr ) const;


    /** Remove the given key */
    bool removeEntry( const QString & scope, const QString & key );


    /** Return keys with values -- do not return keys that contain other keys
     *
     * @note equivalent to QSettings entryList()
     */
    QStringList entryList( const QString & scope, const QString & key ) const;

    /** Return keys with keys -- do not return keys that contain only values
     *
     * @note equivalent to QSettings subkeyList()
     */
    QStringList subkeyList( const QString & scope, const QString & key ) const;


    /** Dump out current project properties to stderr
     */
    // TODO Now slightly broken since re-factoring.  Won't print out top-level key
    //           and redundantly prints sub-keys.
    void dumpProperties() const;

    /** Prepare a filename to save it to the project file. Creates an absolute or relative path to writes
     * it to the project file.
    */
    QString writePath( const QString& filename, const QString& relativeBasePath = QString::null ) const;

    /** Turn filename read from the project file to an absolute path */
    QString readPath( QString filename ) const;

    /** Return error message from previous read/write */
    QString error() const;

    /** Change handler for missing layers.
     * Deletes old handler and takes ownership of the new one.
     */
    void setBadLayerHandler( QgsProjectBadLayerHandler* handler );

    /** Returns project file path if layer is embedded from other project file. Returns empty string if layer is not embedded*/
    QString layerIsEmbedded( const QString& id ) const;

    /** Creates a maplayer instance defined in an arbitrary project file. Caller takes ownership
     * @return the layer or 0 in case of error
     * @note not available in Python bindings
     */
    bool createEmbeddedLayer( const QString& layerId, const QString& projectFilePath, QList<QDomNode>& brokenNodes,
                              QList< QPair< QgsVectorLayer*, QDomElement > >& vectorLayerList, bool saveFlag = true );

    /** Create layer group instance defined in an arbitrary project file.
     * @note: added in version 2.4
     */
    QgsLayerTreeGroup* createEmbeddedGroup( const QString& groupName, const QString& projectFilePath, const QStringList &invisibleLayers );

    /** Convenience function to set snap settings per layer */
    void setSnapSettingsForLayer( const QString& layerId, bool enabled, QgsSnapper::SnappingType type, QgsTolerance::UnitType unit, double tolerance,
                                  bool avoidIntersection );

    /** Convenience function to query snap settings of a layer */
    bool snapSettingsForLayer( const QString& layerId, bool& enabled, QgsSnapper::SnappingType& type, QgsTolerance::UnitType& units, double& tolerance,
                               bool& avoidIntersection ) const;

    /** Convenience function to set topological editing */
    void setTopologicalEditing( bool enabled );

    /** Convenience function to query topological editing status */
    bool topologicalEditing() const;

    /** Convenience function to query default distance measurement units for project.
     * @note added in QGIS 2.14
     * @see areaUnits()
     */
    QGis::UnitType distanceUnits() const;

    /** Convenience function to query default area measurement units for project.
     * @note added in QGIS 2.14
     * @see distanceUnits()
     */
    QgsUnitTypes::AreaUnit areaUnits() const;

    /** Return project's home path
      @return home path of project (or QString::null if not set) */
    QString homePath() const;

    QgsRelationManager* relationManager() const;

    /** Return pointer to the root (invisible) node of the project's layer tree
     * @note added in 2.4
     */
    QgsLayerTreeGroup* layerTreeRoot() const;

    /** Return pointer to the helper class that synchronizes map layer registry with layer tree
     * @note added in 2.4
     */
    QgsLayerTreeRegistryBridge* layerTreeRegistryBridge() const { return mLayerTreeRegistryBridge; }

    /** Returns pointer to the project's visibility preset collection.
     * @note added in QGIS 2.12
     */
    QgsVisibilityPresetCollection* visibilityPresetCollection();

    /**
     * Set a list of layers which should not be taken into account on map identification
     */
    void setNonIdentifiableLayers( QList<QgsMapLayer*> layers );

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
     * Map of transaction groups
     *
     * QPair( providerKey, connString ) -> transactionGroup
     *
     * @note Added in QGIS 2.16
     * @note Not available in python bindings
     */
    QMap< QPair< QString, QString>, QgsTransactionGroup*> transactionGroups();

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

  protected:
    /** Set error message from read/write operation
     * @note not available in Python bindings
     */
    void setError( const QString& errorMessage );

    /** Clear error message
     * @note not available in Python bindings
     */
    void clearError();

    //! Creates layer and adds it to maplayer registry
    //! @note not available in python bindings
    bool addLayer( const QDomElement& layerElem, QList<QDomNode>& brokenNodes, QList< QPair< QgsVectorLayer*, QDomElement > >& vectorLayerList );

    //! @note not available in python bindings
    void initializeEmbeddedSubtree( const QString& projectFilePath, QgsLayerTreeGroup* group );

    //! @note not available in python bindings
    void loadEmbeddedNodes( QgsLayerTreeGroup* group );

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
     * @param mapLayer  The map layer which is being initialized
     * @param layerNode The layer node from the project file
     */
    void readMapLayer( QgsMapLayer *mapLayer, const QDomElement &layerNode );

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

    //! emitted when a layer from a projects was read
    // @param i current layer
    // @param n number of layers
    void layerLoaded( int i, int n );

    void loadingLayer( const QString& );

    void snapSettingsChanged();

    //! Emitted when the list of layer which are excluded from map identification changes
    void nonIdentifiableLayersChanged( QStringList nonIdentifiableLayers );

  public slots:

    /**
     * Flag the project as dirty (modified). If this flag is set, the user will
     * be asked to save changes to the project before closing the current project.
     *
     * @note added in 2.4
     * @note promoted to public slot in 2.16
     */
    void setDirty( bool b = true );

  private slots:
    void onMapLayersAdded( const QList<QgsMapLayer*>& layers );
    void cleanTransactionGroups( bool force = false );

  private:

    QgsProject(); // private 'cause it's a singleton

    QgsProject( QgsProject const & ); // private 'cause it's a singleton

    struct Imp;

    /// implementation handle
    QScopedPointer<Imp> imp_;

    static QgsProject * theProject_;

    /** Read map layers from project file.
     * @param doc DOM document to parse
     * @param brokenNodes a list of DOM nodes corresponding to layers that we were unable to load; this could be
     * because the layers were removed or re-located after the project was last saved
     * @returns true if function worked; else is false
    */
    bool _getMapLayers( QDomDocument const &doc, QList<QDomNode>& brokenNodes );

    /** Processes any joins attached to a newly added layer.
     * @param layer layer to process
     */
    void processLayerJoins( QgsVectorLayer* layer );

    QString mErrorMessage;

    QgsProjectBadLayerHandler* mBadLayerHandler;

    /** Embeded layers which are defined in other projects. Key: layer id,
     * value: pair< project file path, save layer yes / no (e.g. if the layer is part of an embedded group, loading/saving is done by the legend)
     *  If the project file path is empty, QgsProject is going to ignore the layer for saving (e.g. because it is part and managed by an embedded group)
     */
    QHash< QString, QPair< QString, bool> > mEmbeddedLayers;

    void snapSettings( QStringList& layerIdList, QStringList& enabledList, QStringList& snapTypeList, QStringList& snapUnitList, QStringList& toleranceUnitList,
                       QStringList& avoidIntersectionList ) const;

    QgsRelationManager* mRelationManager;

    QgsLayerTreeGroup* mRootGroup;

    QgsLayerTreeRegistryBridge* mLayerTreeRegistryBridge;

    //! map of transaction group: QPair( providerKey, connString ) -> transactionGroup
    QMap< QPair< QString, QString>, QgsTransactionGroup*> mTransactionGroups;

    QScopedPointer<QgsVisibilityPresetCollection> mVisibilityPresetCollection;
};


/** \ingroup core
 * Interface for classes that handle missing layer files when reading project file. */
class CORE_EXPORT QgsProjectBadLayerHandler
{
  public:
    virtual void handleBadLayers( const QList<QDomNode>& layers, const QDomDocument& projectDom ) = 0;
    virtual ~QgsProjectBadLayerHandler() {}
};


/** \ingroup core
 * Default bad layer handler which ignores any missing layers. */
class CORE_EXPORT QgsProjectBadLayerDefaultHandler : public QgsProjectBadLayerHandler
{
  public:
    virtual void handleBadLayers( const QList<QDomNode>& layers, const QDomDocument& projectDom ) override;

};


/** Return the version string found in the given DOM document
   @returns the version string or an empty string if none found
 */
CORE_EXPORT QgsProjectVersion getVersion( QDomDocument const &doc );

#endif
