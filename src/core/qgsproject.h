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

//for the snap settings
#include "qgssnapper.h"
#include "qgstolerance.h"

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

/** \ingroup core
 * Reads and writes project states.


  @note

  Has two general kinds of state to make persistent.  (I.e., to read and
  write.)  First, Qgis proprietary information.  Second plug-in information.

  A singleton since there shall only be one active project at a time; and
  provides canonical location for plug-ins and main app to find/set
  properties.

  Might want to consider moving from Singleton; i.e., allowing more than one
  project.  Just as the GIMP can have simultaneous multiple images, perhaps
  qgis can one day have simultaneous multiple projects.

*/
class CORE_EXPORT QgsProject : public QObject
{
    Q_OBJECT

  public:

    /**
       @todo XXX Should have semantics for saving project if dirty as last gasp?
    */
    ~QgsProject();

    /// access to canonical QgsProject instance
    static QgsProject * instance();

    /**
       Every project has an associated title string

       ### QGIS 3: remove in favor of setTitle(...)
     */
    //@{
    void title( const QString & title );

    /** Set project title
     *  @note added in 2.4 */
    void setTitle( const QString& title );

    /** Returns title */
    const QString & title() const;
    //@}

    /**
       the dirty flag is true if the project has been modified since the last
       write()
     */
    //@{
    bool isDirty() const;

    // ### QGIS 3: remove in favor of setDirty(...)
    void dirty( bool b );

    /** Set project as dirty (modified).
     *  @note added in 2.4 */
    void setDirty( bool b );
    //@}


    /**
       Every project has an associated file that contains its XML
     */
    //@{
    void setFileName( const QString & name );

    /** Returns file name */
    QString fileName() const;
    //@}

    /** Clear the project
     * @note added in 2.4
     */
    void clear();


    /** Read project file

       @note Any current plug-in state is erased

       @note dirty set to false after successful invocation

       @note file name argument implicitly sets file

       @note

       - Gets the extents
       - Creates maplayers
       - Registers maplayers

       @note it's presumed that the caller has already reset the map canvas, map registry, and legend
     */
    //@{
    bool read( const QFileInfo & file );
    bool read();
    //@}


    /** Read the layer described in the associated Dom node

        @param layerNode   represents a QgsProject Dom node that maps to a specific layer.

        QgsProject raises an exception when one of the QgsProject::read()
        implementations fails.  Since the read()s are invoked from qgisapp,
        then qgisapp handles the exception.  It prompts the user for the new
        location of the data, if any.  If there is a new location, the Dom
        node associated with the layer has its datasource tag corrected.
        Then that node is passed to this member function to be re-opened.

     */
    bool read( QDomNode & layerNode );


    /** Write project file

       XXX How to best get read access to Qgis state?  Actually we can finagle
       that by searching for qgisapp in object hiearchy.

       @note file name argument implicitly sets file

       @note dirty set to false after successful invocation
     */
    //@{
    bool write( const QFileInfo & file );
    bool write();
    //@}

    /**
       removes all project properties

       ### QGIS 3: remove in favor of clear()
    */
    void clearProperties();


    /* key value mutators

      keys would be the familiar QSettings-like '/' delimited entries, implying
      a hierarchy of keys and corresponding values

      @note The key string <em>must</em> include '/'s.  E.g., "/foo" not "foo".
    */
    //@{
    //! @note available in python bindings as writeEntryBool
    bool writeEntry( const QString & scope, const QString & key, bool value );
    //! @note available in python bindings as writeEntryDouble
    bool writeEntry( const QString & scope, const QString & key, double value );
    bool writeEntry( const QString & scope, const QString & key, int value );
    bool writeEntry( const QString & scope, const QString & key, const QString & value );
    bool writeEntry( const QString & scope, const QString & key, const QStringList & value );
    //@}

    /** Key value accessors

        keys would be the familiar QSettings-like '/' delimited entries,
        implying a hierarchy of keys and corresponding values


        @note The key string <em>must</em> include '/'s.  E.g., "/foo" not "foo".
    */
    //@{
    QStringList readListEntry( const QString & scope, const QString & key, QStringList def = QStringList(), bool *ok = 0 ) const;

    QString readEntry( const QString & scope, const QString & key, const QString & def = QString::null, bool * ok = 0 ) const;
    int readNumEntry( const QString & scope, const QString & key, int def = 0, bool * ok = 0 ) const;
    double readDoubleEntry( const QString & scope, const QString & key, double def = 0, bool * ok = 0 ) const;
    bool readBoolEntry( const QString & scope, const QString & key, bool def = false, bool * ok = 0 ) const;
    //@}


    /** Remove the given key */
    bool removeEntry( const QString & scope, const QString & key );


    /** Return keys with values -- do not return keys that contain other keys

      @note equivalent to QSettings entryList()
    */
    QStringList entryList( const QString & scope, const QString & key ) const;

    /** Return keys with keys -- do not return keys that contain only values

      @note equivalent to QSettings subkeyList()
    */
    QStringList subkeyList( const QString & scope, const QString & key ) const;


    /** Dump out current project properties to stderr

      @todo XXX Now slightly broken since re-factoring.  Won't print out top-level key
                and redundantly prints sub-keys.
    */
    void dumpProperties() const;

    /** Prepare a filename to save it to the project file */
    QString writePath( QString filename, QString relativeBasePath = QString::null ) const;

    /** Turn filename read from the project file to an absolute path */
    QString readPath( QString filename ) const;

    /** Return error message from previous read/write */
    QString error() const;

    /** Change handler for missing layers.
      Deletes old handler and takes ownership of the new one. */
    void setBadLayerHandler( QgsProjectBadLayerHandler* handler );

    /** Returns project file path if layer is embedded from other project file. Returns empty string if layer is not embedded*/
    QString layerIsEmbedded( const QString& id ) const;

    /** Creates a maplayer instance defined in an arbitrary project file. Caller takes ownership
      @return the layer or 0 in case of error
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

  protected:

    /** Set error message from read/write operation */
    void setError( QString errorMessage );

    /** Clear error message */
    void clearError();

    //Creates layer and adds it to maplayer registry
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
     * Emitted, after the basic initialisation of a layer from the project
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
    void oldProjectVersionWarning( QString );

    //! emitted when a layer from a projects was read
    // @param i current layer
    // @param n number of layers
    void layerLoaded( int i, int n );

    void loadingLayer( QString );

    void snapSettingsChanged();

  private:

    QgsProject(); // private 'cause it's a singleton

    QgsProject( QgsProject const & ); // private 'cause it's a singleton

    struct Imp;

    /// implementation handle
    QScopedPointer<Imp> imp_;

    static QgsProject * theProject_;

    QPair< bool, QList<QDomNode> > _getMapLayers( QDomDocument const &doc );

    QString mErrorMessage;

    QgsProjectBadLayerHandler* mBadLayerHandler;

    /** Embeded layers which are defined in other projects. Key: layer id,
    value: pair< project file path, save layer yes / no (e.g. if the layer is part of an embedded group, loading/saving is done by the legend)
       If the project file path is empty, QgsProject is going to ignore the layer for saving (e.g. because it is part and managed by an embedded group)*/
    QHash< QString, QPair< QString, bool> > mEmbeddedLayers;

    void snapSettings( QStringList& layerIdList, QStringList& enabledList, QStringList& snapTypeList, QStringList& snapUnitList, QStringList& toleranceUnitList,
                       QStringList& avoidIntersectionList ) const;

    QgsRelationManager* mRelationManager;

    QgsLayerTreeGroup* mRootGroup;

    QgsLayerTreeRegistryBridge* mLayerTreeRegistryBridge;

}; // QgsProject


/** Interface for classes that handle missing layer files when reading project file. */
class CORE_EXPORT QgsProjectBadLayerHandler
{
  public:
    virtual void handleBadLayers( QList<QDomNode> layers, QDomDocument projectDom ) = 0;
    virtual ~QgsProjectBadLayerHandler() {}
};


/** Default bad layer handler which ignores any missing layers. */
class CORE_EXPORT QgsProjectBadLayerDefaultHandler : public QgsProjectBadLayerHandler
{
  public:
    virtual void handleBadLayers( QList<QDomNode> layers, QDomDocument projectDom ) override;

};

#endif
