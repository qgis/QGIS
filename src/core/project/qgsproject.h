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
#include <QTranslator>

#include "qgsunittypes.h"
#include "qgssnappingconfig.h"
#include "qgsprojectversion.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgsexpressioncontextscopegenerator.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgsprojectproperty.h"
#include "qgsmaplayerstore.h"
#include "qgsarchive.h"
#include "qgsreadwritecontext.h"
#include "qgsprojectmetadata.h"
#include "qgstranslationcontext.h"
#include "qgsprojecttranslator.h"
#include "qgscolorscheme.h"
#include "qgssettings.h"
#include "qgspropertycollection.h"
#include "qgsvectorlayereditbuffergroup.h"

#include "qgsrelationmanager.h"
#include "qgsmapthemecollection.h"

class QFileInfo;
class QDomDocument;
class QDomElement;
class QDomNode;

class QgsLayerTreeGroup;
class QgsLayerTreeRegistryBridge;
class QgsMapLayer;
class QgsPathResolver;
class QgsProjectBadLayerHandler;
class QgsProjectStorage;
class QgsTolerance;
class QgsTransactionGroup;
class QgsVectorLayer;
class QgsAnnotationManager;
class QgsLayoutManager;
class QgsLayerTree;
class QgsLabelingEngineSettings;
class QgsAuxiliaryStorage;
class QgsMapLayer;
class QgsBookmarkManager;
class QgsProjectViewSettings;
class QgsProjectDisplaySettings;
class QgsProjectTimeSettings;
class QgsAnnotationLayer;
class QgsAttributeEditorContainer;
class QgsPropertyCollection;
class QgsMapViewsManager;

/**
 * \ingroup core
 * \brief Encapsulates a QGIS project, including sets of map layers and their styles,
 * layouts, annotations, canvases, etc.
 *
 * QgsProject is available both as a singleton (QgsProject::instance()) and for use as
 * standalone objects. The QGIS project singleton always gives access to the canonical project reference
 * open within the main QGIS application.
 *
 * \note QgsProject has two general kinds of state to make persistent. (I.e., to read and
 * write.) First, QGIS proprietary information. Second plugin information.
*/

class CORE_EXPORT QgsProject : public QObject, public QgsExpressionContextGenerator, public QgsExpressionContextScopeGenerator, public QgsProjectTranslator
{
    Q_OBJECT
    Q_PROPERTY( QStringList nonIdentifiableLayers READ nonIdentifiableLayers WRITE setNonIdentifiableLayers NOTIFY nonIdentifiableLayersChanged )
    Q_PROPERTY( QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged )
    Q_PROPERTY( QString homePath READ homePath WRITE setPresetHomePath NOTIFY homePathChanged )
    Q_PROPERTY( QgsCoordinateReferenceSystem crs READ crs WRITE setCrs NOTIFY crsChanged )
    Q_PROPERTY( QgsCoordinateTransformContext transformContext READ transformContext WRITE setTransformContext NOTIFY transformContextChanged )
    Q_PROPERTY( QString ellipsoid READ ellipsoid WRITE setEllipsoid NOTIFY ellipsoidChanged )
    Q_PROPERTY( QgsMapThemeCollection *mapThemeCollection READ mapThemeCollection NOTIFY mapThemeCollectionChanged )
    Q_PROPERTY( QgsSnappingConfig snappingConfig READ snappingConfig WRITE setSnappingConfig NOTIFY snappingConfigChanged )
    Q_PROPERTY( QgsRelationManager *relationManager READ relationManager )
    Q_PROPERTY( AvoidIntersectionsMode avoidIntersectionsMode READ avoidIntersectionsMode WRITE setAvoidIntersectionsMode NOTIFY avoidIntersectionsModeChanged )
    Q_PROPERTY( QList<QgsVectorLayer *> avoidIntersectionsLayers READ avoidIntersectionsLayers WRITE setAvoidIntersectionsLayers NOTIFY avoidIntersectionsLayersChanged )
    Q_PROPERTY( QgsProjectMetadata metadata READ metadata WRITE setMetadata NOTIFY metadataChanged )
    Q_PROPERTY( QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged )
    Q_PROPERTY( QColor selectionColor READ selectionColor WRITE setSelectionColor NOTIFY selectionColorChanged )
    Q_PROPERTY( bool topologicalEditing READ topologicalEditing WRITE setTopologicalEditing NOTIFY topologicalEditingChanged )

  public:

    /**
     * Flags which control project read behavior.
     * \since QGIS 3.10
     */
    enum class ReadFlag SIP_MONKEYPATCH_SCOPEENUM
    {
      FlagDontResolveLayers = 1 << 0, //!< Don't resolve layer paths (i.e. don't load any layer content). Dramatically improves project read time if the actual data from the layers is not required.
      FlagDontLoadLayouts = 1 << 1, //!< Don't load print layouts. Improves project read time if layouts are not required, and allows projects to be safely read in background threads (since print layouts are not thread safe).
      FlagTrustLayerMetadata = 1 << 2, //!< Trust layer metadata. Improves project read time. Do not use it if layers' extent is not fixed during the project's use by QGIS and QGIS Server.
      FlagDontStoreOriginalStyles = 1 << 3, //!< Skip the initial XML style storage for layers. Useful for minimising project load times in non-interactive contexts.
      FlagDontLoad3DViews = 1 << 4, //!<
    };
    Q_DECLARE_FLAGS( ReadFlags, ReadFlag )

    /**
     * Flags which control project read behavior.
     * \since QGIS 3.12
     */
    enum class FileFormat
    {
      Qgz, //!< Archive file format, supports auxiliary data
      Qgs, //!< Project saved in a clear text, does not support auxiliary data
    };
    Q_ENUM( FileFormat )

    /**
     * Flags which control how intersections of pre-existing feature are handled when digitizing new features.
     * \since QGIS 3.14
     */
    enum class AvoidIntersectionsMode
    {
      AllowIntersections, //!< Overlap with any feature allowed when digitizing new features
      AvoidIntersectionsCurrentLayer, //!< Overlap with features from the active layer when digitizing new features not allowed
      AvoidIntersectionsLayers, //!< Overlap with features from a specified list of layers when digitizing new features not allowed
    };
    Q_ENUM( AvoidIntersectionsMode )

    /**
     * Data defined properties.
     * Overrides of user defined server parameters are stored in a
     * property collection and they can be retrieved using the
     * indexes specified in this enum.
     *
     * \since QGIS 3.14
     */
    enum DataDefinedServerProperty
    {
      NoProperty = 0, //!< No property
      AllProperties = 1, //!< All properties for item
      WMSOnlineResource = 2, //!< Alias
    };

    //! Returns the QgsProject singleton instance
    static QgsProject *instance();

    /**
     * Set the current project singleton instance to \a project
     *
     * \note this method is provided mainly for the server, which caches the projects and (potentially) needs to switch the current instance on every request.
     * \warning calling this method can have serious, unintended consequences, including instability, data loss and undefined behavior. Use with EXTREME caution!
     * \see instance()
     * \since QGIS 3.10.11
     */
    static void setInstance( QgsProject *project ) ;


    /**
     * Create a new QgsProject.
     *
     * Most of the time you want to use QgsProject::instance() instead as many components of QGIS work with the singleton.
     */
    explicit QgsProject( QObject *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsProject() override;

    /**
    * Sets the project's title.
    * \param title new title
    *
    * \note Since QGIS 3.2 this is just a shortcut to setting the title in the project's metadata().
    *
    * \see title()
    * \since QGIS 2.4
    */
    void setTitle( const QString &title );

    /**
     * Returns the project's title.
     * \see setTitle()
     *
     * \note Since QGIS 3.2 this is just a shortcut to retrieving the title from the project's metadata().
    */
    QString title() const;

    /**
    * Returns the user name that did the last save.
    *
    * \see saveUserFullName()
    *
    * \since QGIS 3.12
    */
    QString saveUser() const;

    /**
    * Returns the full user name that did the last save.
    *
    * \see saveUser()
    *
    * \since QGIS 3.12
    */
    QString saveUserFullName() const;

    /**
     * Returns the date and time when the project was last saved.
     *
     * \since QGIS 3.14
     */
    QDateTime lastSaveDateTime() const;

    /**
     * Returns the QGIS version which the project was last saved using.
     *
     * \since QGIS 3.14
     */
    QgsProjectVersion lastSaveVersion() const;

    /**
     * Returns TRUE if the project has been modified since the last write()
     */
    bool isDirty() const;

    /**
     * Sets the file name associated with the project. This is the file or the storage URI which contains the project's XML
     * representation.
     * \param name project file name
     * \see fileName()
     */
    void setFileName( const QString &name );

    /**
     * Returns the project's file name. This is the file or the storage URI which contains the project's XML
     * representation.
     * \see setFileName()
     * \see fileInfo()
    */
    QString fileName() const;

    /**
     * Sets the original \a path associated with the project.
     *
     * This is intended for use with non-qgs/qgz project files (see QgsCustomProjectOpenHandler) in order to allow
     * custom project open handlers to specify the original file name of the project. For custom project formats,
     * it is NOT appropriate to call setFileName() with the original project path, as this causes the original (non
     * QGIS) project file to be overwritten when the project is next saved.
     *
     * \see originalPath()
     * \since QGIS 3.14
     */
    void setOriginalPath( const QString &path );

    /**
     * Returns the original path associated with the project.
     *
     * This is intended for use with non-qgs/qgz project files (see QgsCustomProjectOpenHandler) in order to allow
     * custom project open handlers to specify the original file name of the project. For custom project formats,
     * it is NOT appropriate to call setFileName() with the original project path, as this causes the original (non
     * QGIS) project file to be overwritten when the project is next saved.

     * \see setOriginalPath()
     * \since QGIS 3.14
    */
    QString originalPath() const;

    /**
     * Returns QFileInfo object for the project's associated file.
     *
     * \note The use of this method is discouraged since QGIS 3.2 as it only works with project files stored
     * in the file system. It is recommended to use absoluteFilePath(), baseName(), lastModifiedTime() as
     * replacements that are aware of the fact that projects may be saved in other project storages.
     *
     * \see fileName()
     * \deprecated since QGIS 3.2 use absoluteFilePath(), baseName() or lastModifiedTime() instead
     */
    Q_DECL_DEPRECATED QFileInfo fileInfo() const SIP_DEPRECATED;

    /**
     * Returns pointer to project storage implementation that handles read/write of the project file.
     * If the project file is stored in the local file system, returns NULLPTR.
     * The project storage object is inferred from fileName() of the project.
     * \since QGIS 3.2
     */
    QgsProjectStorage *projectStorage() const;

    /**
     * Returns last modified time of the project file as returned by the file system (or other project storage).
     * \since QGIS 3.2
     */
    QDateTime lastModified() const;

    /**
     * Returns full absolute path to the project file if the project is stored in a file system - derived from fileName().
     * Returns empty string when the project is stored in a project storage (there is no concept of paths for custom project storages).
     * \since QGIS 3.2
     */
    QString absoluteFilePath() const;

    /**
     * Returns full absolute path to the project folder if the project is stored in a file system - derived from fileName().
     * Returns empty string when the project is stored in a project storage (there is no concept of paths for custom project storages).
     * \since QGIS 3.2
     */
    QString absolutePath() const;

    /**
     * Returns the base name of the project file without the path and without extension - derived from fileName().
     * \since QGIS 3.2
     */
    QString baseName() const;

    /**
     * Returns the type of paths used when storing file paths in a QGS/QGZ project file.
     *
     * \see setFilePathStorage()
     * \since QGIS 3.22
     */
    Qgis::FilePathType filePathStorage() const;

    /**
     * Sets the \a type of paths used when storing file paths in a QGS/QGZ project file.
     *
     * \see filePathStorage()
     * \since QGIS 3.22
     */
    void setFilePathStorage( Qgis::FilePathType type );

    /**
     * Returns the project's native coordinate reference system.
     * \see setCrs()
     * \see ellipsoid()
     * \since QGIS 3.0
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Sets the project's native coordinate reference system.
     * If \a adjustEllipsoid is set to TRUE, the ellpsoid of this project will be set to
     * the ellipsoid imposed by the CRS.
     *
     * \see crs()
     * \see setEllipsoid()
     * \since QGIS 3.0
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs, bool adjustEllipsoid = false );

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
     * Returns a copy of the project's coordinate transform context, which stores various
     * information regarding which datum transforms should be used when transforming points
     * from a source to destination coordinate reference system.
     *
     * \see setTransformContext()
     * \see transformContextChanged()
     * \since QGIS 3.0
     */
    QgsCoordinateTransformContext transformContext() const;

    /**
     * Sets the project's coordinate transform \a context, which stores various
     * information regarding which datum transforms should be used when transforming points
     * from a source to destination coordinate reference system.
     *
     * \see transformContext()
     * \see transformContextChanged()
     * \since QGIS 3.0
     */
    void setTransformContext( const QgsCoordinateTransformContext &context );

    /**
     * Clears the project, removing all settings and resetting it back to an empty, default state.
     * \see cleared()
     * \since QGIS 2.4
     */
    void clear();

    /**
     * Reads given project file from the given file.
     * \param filename name of project file to read
     * \param flags optional flags which control the read behavior of projects
     * \returns TRUE if project file has been read successfully
     */
    bool read( const QString &filename, QgsProject::ReadFlags flags = QgsProject::ReadFlags() );

    /**
     * Reads the project from its currently associated file (see fileName() ).
     *
     * The \a flags argument can be used to specify optional flags which control
     * the read behavior of projects.
     *
     * \returns TRUE if project file has been read successfully
     */
    bool read( QgsProject::ReadFlags flags = QgsProject::ReadFlags() );

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
     * \returns TRUE if project was written successfully
     * \note calling this implicitly sets the project's filename (see setFileName() )
     * \note isDirty() will be set to FALSE if project is successfully written
     * \since QGIS 3.0
     */
    bool write( const QString &filename );

    /**
     * Writes the project to its current associated file (see fileName() ).
     * \returns TRUE if project was written successfully
     * \note isDirty() will be set to FALSE if project is successfully written
     */
    bool write();

    /**
     * Write a boolean \a value to the project file.
     *
     * Keys are '/'-delimited entries, implying
     * a hierarchy of keys and corresponding values
     *
     * \note The key string must be valid xml tag names in order to be saved to the file.
     * \note available in Python bindings as writeEntryBool
     *
     * \see readBoolEntry()
     */
    bool writeEntry( const QString &scope, const QString &key, bool value ) SIP_PYNAME( writeEntryBool );

    /**
     * Write a double \a value to the project file.
     *
     * Keys are '/'-delimited entries, implying
     * a hierarchy of keys and corresponding values
     *
     * \note The key string must be valid xml tag names in order to be saved to the file.
     * \note available in Python bindings as writeEntryDouble
     *
     * \see readDoubleEntry()
     */
    bool writeEntry( const QString &scope, const QString &key, double value ) SIP_PYNAME( writeEntryDouble );

    /**
     * Write an integer \a value to the project file.
     *
     * Keys are '/'-delimited entries, implying
     * a hierarchy of keys and corresponding values
     *
     * \note The key string must be valid xml tag names in order to be saved to the file.
     *
     * \see readNumEntry()
     */
    bool writeEntry( const QString &scope, const QString &key, int value );

    /**
     * Write a string \a value to the project file.
     *
     * Keys are '/'-delimited entries, implying
     * a hierarchy of keys and corresponding values
     *
     * \note The key string must be valid xml tag names in order to be saved to the file.
     *
     * \see readEntry()
     */
    bool writeEntry( const QString &scope, const QString &key, const QString &value );

    /**
     * Write a string list \a value to the project file.
     *
     * Keys are '/'-delimited entries, implying
     * a hierarchy of keys and corresponding values
     *
     * \note The key string must be valid xml tag names in order to be saved to the file.
     *
     * \see readListEntry()
     */
    bool writeEntry( const QString &scope, const QString &key, const QStringList &value );

    /**
     * Reads a string list from the specified \a scope and \a key.
     *
     * \param scope entry scope (group) name
     * \param key entry key name. Keys are '/'-delimited entries, implying a hierarchy of keys and corresponding values.
     * \param def default value to return if the specified \a key does not exist within the \a scope.
     * \param ok set to TRUE if key exists and has been successfully retrieved as a string list
     *
     * \returns entry value as a string list
     */
    QStringList readListEntry( const QString &scope, const QString &key, const QStringList &def = QStringList(), bool *ok SIP_OUT = nullptr ) const;

    /**
     * Reads a string from the specified \a scope and \a key.
     *
     * \param scope entry scope (group) name
     * \param key entry key name. Keys are '/'-delimited entries, implying a hierarchy of keys and corresponding values.
     * \param def default value to return if the specified \a key does not exist within the \a scope.
     * \param ok set to TRUE if key exists and has been successfully retrieved as a string value
     *
     * \returns entry value as string from \a scope given its \a key
     */
    QString readEntry( const QString &scope, const QString &key, const QString &def = QString(), bool *ok SIP_OUT = nullptr ) const;

    /**
     * Reads an integer from the specified \a scope and \a key.
     *
     * \param scope entry scope (group) name
     * \param key entry key name. Keys are '/'-delimited entries, implying a hierarchy of keys and corresponding values.
     * \param def default value to return if the specified \a key does not exist within the \a scope.
     * \param ok set to TRUE if key exists and has been successfully retrieved as an integer
     *
     * \returns entry value as integer from \a scope given its \a key
     */
    int readNumEntry( const QString &scope, const QString &key, int def = 0, bool *ok SIP_OUT = nullptr ) const;

    /**
     * Reads a double from the specified \a scope and \a key.
     *
     * \param scope entry scope (group) name
     * \param key entry key name. Keys are '/'-delimited entries, implying a hierarchy of keys and corresponding values.
     * \param def default value to return if the specified \a key does not exist within the \a scope.
     * \param ok set to TRUE if key exists and has been successfully retrieved as a double
     *
     * \returns entry value as double from \a scope given its \a key
     */
    double readDoubleEntry( const QString &scope, const QString &key, double def = 0, bool *ok SIP_OUT = nullptr ) const;

    /**
     * Reads a boolean from the specified \a scope and \a key.
     *
     * \param scope entry scope (group) name
     * \param key entry key name. Keys are '/'-delimited entries, implying a hierarchy of keys and corresponding values.
     * \param def default value to return if the specified \a key does not exist within the \a scope.
     * \param ok set to TRUE if key exists and has been successfully retrieved as a boolean
     *
     * \returns entry value as boolean from \a scope given its \a key
     */
    bool readBoolEntry( const QString &scope, const QString &key, bool def = false, bool *ok SIP_OUT = nullptr ) const;

    /**
     * Remove the given \a key from the specified \a scope.
     */
    bool removeEntry( const QString &scope, const QString &key );

    /**
     * Returns a list of child keys with values which exist within the the specified \a scope and \a key.
     *
     * This method does not return keys that contain other keys. See subkeyList() to retrieve keys
     * which contain other keys.
     *
     * \note equivalent to QgsSettings entryList()
     */
    QStringList entryList( const QString &scope, const QString &key ) const;

    /**
     * Returns a list of child keys which contain other keys that exist within the the specified \a scope and \a key.
     *
     * This method only returns keys with keys, it will not return keys that contain only values. See
     * entryList() to retrieve keys with values.
     *
     * \note equivalent to QgsSettings subkeyList()
     */
    QStringList subkeyList( const QString &scope, const QString &key ) const;

    // TODO Now slightly broken since re-factoring.  Won't print out top-level key
    //           and redundantly prints sub-keys.

    /**
     * Dump out current project properties to stderr
     */
    void dumpProperties() const;

    /**
     * Returns path resolver object with considering whether the project uses absolute
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

    /**
     * Transforms a \a filename read from the project file to an absolute path.
     */
    QString readPath( const QString &filename ) const;

    //! Returns error message from previous read/write
    QString error() const;

    /**
     * Change handler for missing layers.
     * Deletes old handler and takes ownership of the new one.
     */
    void setBadLayerHandler( QgsProjectBadLayerHandler *handler SIP_TRANSFER );

    /**
     * Returns the source project file path if the layer with matching \a id is embedded from other project file.
     *
     * Returns an empty string if the matching layer is not embedded.
     */
    QString layerIsEmbedded( const QString &id ) const;

    /**
     * Creates a maplayer instance defined in an arbitrary project file. Caller takes ownership.
     *
     * The optional \a flags argument can be used to specify flags which control layer reading.
     *
     * \returns the layer or 0 in case of error
     * \note not available in Python bindings
     */
    bool createEmbeddedLayer( const QString &layerId, const QString &projectFilePath, QList<QDomNode> &brokenNodes,
                              bool saveFlag = true, QgsProject::ReadFlags flags = QgsProject::ReadFlags() ) SIP_SKIP;

    /**
     * Create layer group instance defined in an arbitrary project file.
     *
     * The optional \a flags argument can be used to control layer reading behavior.
     *
     * \since QGIS 2.4
     */
    QgsLayerTreeGroup *createEmbeddedGroup( const QString &groupName, const QString &projectFilePath, const QStringList &invisibleLayers, QgsProject::ReadFlags flags = QgsProject::ReadFlags() );

    //! Convenience function to set topological editing
    void setTopologicalEditing( bool enabled );

    //! Convenience function to query topological editing status
    bool topologicalEditing() const;

    /**
     * Convenience function to query default distance measurement units for project.
     * \see setDistanceUnits()
     * \see areaUnits()
     * \since QGIS 2.14
     */
    QgsUnitTypes::DistanceUnit distanceUnits() const;

    /**
     * Sets the default distance measurement units for the project.
     * \see distanceUnits()
     * \see setAreaUnits()
     * \since QGIS 3.0
     */
    void setDistanceUnits( QgsUnitTypes::DistanceUnit unit );

    /**
     * Convenience function to query default area measurement units for project.
     * \see distanceUnits()
     * \since QGIS 2.14
     */
    QgsUnitTypes::AreaUnit areaUnits() const;

    /**
     * Sets the default area measurement units for the project.
     * \see areaUnits()
     * \see setDistanceUnits()
     * \since QGIS 3.0
     */
    void setAreaUnits( QgsUnitTypes::AreaUnit unit );

    /**
     * Returns the project's home path. This will either be a manually set home path
     * (see presetHomePath()) or the path containing the project file itself.
     *
     * This method always returns the absolute path to the project's home. See
     * presetHomePath() to retrieve any manual project home path override (e.g.
     * relative home paths).
     *
     * \see setPresetHomePath()
     * \see presetHomePath()
     * \see homePathChanged()
    */
    QString homePath() const;

    /**
     * Returns any manual project home path setting, or an empty string if not set.
     *
     * This path may be a relative path. See homePath() to retrieve a path which is always
     * an absolute path.
     *
     * \see homePath()
     * \see setPresetHomePath()
     * \see homePathChanged()
     *
     * \since QGIS 3.2
    */
    QString presetHomePath() const;

    QgsRelationManager *relationManager() const;

    /**
     * Returns the project's layout manager, which manages print layouts, atlases and reports within
     * the project.
     * \note not available in Python bindings
     * \since QGIS 3.0
     */
    const QgsLayoutManager *layoutManager() const SIP_SKIP;

    /**
     * Returns the project's layout manager, which manages print layouts, atlases and reports within
     * the project.
     * \since QGIS 3.0
     */
    QgsLayoutManager *layoutManager();

    /**
     * Returns the project's views manager, which manages map views (including 3d maps)
     * in the project.
     * \note not available in Python bindings
     * \since QGIS 3.24
     */
    const QgsMapViewsManager *viewsManager() const SIP_SKIP;

    /**
     * Returns the project's views manager, which manages map views (including 3d maps)
     * in the project.
     * \since QGIS 3.24
     */
    QgsMapViewsManager *viewsManager();

    /**
     * Returns the project's bookmark manager, which manages bookmarks within
     * the project.
     * \note not available in Python bindings
     * \since QGIS 3.10
     */
    const QgsBookmarkManager *bookmarkManager() const SIP_SKIP;

    /**
     * Returns the project's bookmark manager, which manages bookmarks within
     * the project.
     * \since QGIS 3.10
     */
    QgsBookmarkManager *bookmarkManager();

    /**
     * Returns the project's view settings, which contains settings and properties
     * relating to how a QgsProject should be viewed and behave inside a map canvas
     * (e.g. map scales and default view extent)
     * \note not available in Python bindings
     * \since QGIS 3.10.1
     */
    const QgsProjectViewSettings *viewSettings() const SIP_SKIP;

    /**
     * Returns the project's view settings, which contains settings and properties
     * relating to how a QgsProject should be viewed and behave inside a map canvas
     * (e.g. map scales and default view extent)
     * \since QGIS 3.10.1
     */
    QgsProjectViewSettings *viewSettings();

    /**
     * Returns the project's time settings, which contains the project's temporal range and other
     * time based settings.
     *
     * \note not available in Python bindings
     * \since QGIS 3.14
     */
    const QgsProjectTimeSettings *timeSettings() const SIP_SKIP;

    /**
     * Returns the project's time settings, which contains the project's temporal range and other
     * time based settings.
     *
     * \since QGIS 3.14
     */
    QgsProjectTimeSettings *timeSettings();

    /**
     * Returns the project's display settings, which settings and properties relating
     * to how a QgsProject should display values such as map coordinates and bearings.
     * \note not available in Python bindings
     * \since QGIS 3.12
     */
    const QgsProjectDisplaySettings *displaySettings() const SIP_SKIP;

    /**
     * Returns the project's display settings, which settings and properties relating
     * to how a QgsProject should display values such as map coordinates and bearings.
     * \since QGIS 3.12
     */
    QgsProjectDisplaySettings *displaySettings();

    /**
     * Returns pointer to the root (invisible) node of the project's layer tree
     * \since QGIS 2.4
     */
    QgsLayerTree *layerTreeRoot() const;

    /**
     * Returns pointer to the helper class that synchronizes map layer registry with layer tree
     * \since QGIS 2.4
     */
    QgsLayerTreeRegistryBridge *layerTreeRegistryBridge() const { return mLayerTreeRegistryBridge; }

    /**
     * Returns pointer to the project's map theme collection.
     * \note renamed in QGIS 3.0, formerly QgsVisibilityPresetCollection
     * \since QGIS 2.12
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
     * \deprecated since QGIS 3.4 use QgsMapLayer::setFlags() instead
     */
    Q_DECL_DEPRECATED void setNonIdentifiableLayers( const QList<QgsMapLayer *> &layers );

    /**
     * Set a list of layers which should not be taken into account on map identification
     * \deprecated since QGIS 3.4 use QgsMapLayer::setFlags() instead
     */
    Q_DECL_DEPRECATED void setNonIdentifiableLayers( const QStringList &layerIds );

    /**
     * Gets the list of layers which currently should not be taken into account on map identification
     * \deprecated since QGIS 3.4 use QgsMapLayer::setFlags() instead
     */
    Q_DECL_DEPRECATED QStringList nonIdentifiableLayers() const;

    /**
     * Transactional editing means that on supported datasources (postgres databases) the edit state of
     * all tables that originate from the same database are synchronized and executed in a server side
     * transaction.
     *
     * \since QGIS 2.16
     * \deprecated QGIS 3.26 use transactionMode instead
     */
    Q_DECL_DEPRECATED bool autoTransaction() const SIP_DEPRECATED;

    /**
     * Transactional editing means that on supported datasources (postgres databases) the edit state of
     * all tables that originate from the same database are synchronized and executed in a server side
     * transaction.
     *
     * \warning Make sure that this is only called when all layers are not in edit mode.
     *
     * \since QGIS 2.16
     * \deprecated QGIS 3.26 use setTransactionMode instead
     */
    Q_DECL_DEPRECATED void setAutoTransaction( bool autoTransaction ) SIP_DEPRECATED;

    /**
     * Returns the transaction mode
     *
     * \see Qgis::TransactionMode
     * \since QGIS 3.26
     */
    Qgis::TransactionMode transactionMode() const;

    /**
     * Set transaction mode
     *
     * \returns TRUE if the transaction mode could be changed
     *
     * \note Transaction mode can be changed only when all layers are not in edit mode.
     * \see Qgis::TransactionMode
     * \since QGIS 3.26
     */
    bool setTransactionMode( Qgis::TransactionMode transactionMode );

    /**
     * Map of transaction groups
     *
     * QPair( providerKey, connString ) -> transactionGroup
     *
     * \note Not available in Python bindings
     * \since QGIS 2.16
     */
    QMap< QPair< QString, QString>, QgsTransactionGroup *> transactionGroups() SIP_SKIP;

    /**
     * Returns the matching transaction group from a provider key and connection string.
     *
     * Returns NULLPTR if a matching transaction group is not available.
     *
     * \since QGIS 3.2
     */
    QgsTransactionGroup *transactionGroup( const QString &providerKey, const QString &connString );

    /**
     * Returns the edit buffer group
     *
     * \since QGIS 3.26
     */
    QgsVectorLayerEditBufferGroup *editBufferGroup();

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
    QgsExpressionContextScope *createExpressionContextScope() const override;

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
     * Sets the list of layers with which intersections should be avoided.
     * Only used if the avoid intersection mode is set to advanced.
     *
     * \since QGIS 3.0
     */
    void setAvoidIntersectionsLayers( const QList<QgsVectorLayer *> &layers );

    /**
     * Sets the avoid intersections mode.
     *
     * \since QGIS 3.14
     */
    void setAvoidIntersectionsMode( const AvoidIntersectionsMode mode );

    /**
     * Returns the current avoid intersections mode.
     *
     * \since QGIS 3.14
     */
    AvoidIntersectionsMode avoidIntersectionsMode() const { return mAvoidIntersectionsMode; }

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

    //! Returns the number of registered valid layers.
    int validCount() const;

    /**
     * Retrieve a pointer to a registered layer by layer ID.
     * \param layerId ID of layer to retrieve
     * \returns matching layer, or NULLPTR if no matching layer found
     * \see mapLayersByName()
     * \see mapLayers()
     */
    Q_INVOKABLE QgsMapLayer *mapLayer( const QString &layerId ) const;

#ifndef SIP_RUN

    /**
     * Retrieve a pointer to a registered layer by \p layerId converted
     * to type T. This is a convenience template.
     * A NULLPTR will be returned if the layer is not found or
     * if it cannot be cast to type T.
     *
     * \code{cpp}
     * QgsVectorLayer *layer = project->mapLayer<QgsVectorLayer*>( layerId );
     * \endcode
     *
     * \see mapLayer()
     * \see mapLayers()
     *
     * \since QGIS 3.6
     */
    template <class T>
    T mapLayer( const QString &layerId ) const
    {
      return qobject_cast<T>( mapLayer( layerId ) );
    }
#endif

    /**
     * Retrieve a list of matching registered layers by layer name.
     * \param layerName name of layers to match
     * \returns list of matching layers
     * \see mapLayer()
     * \see mapLayers()
     */
    QList<QgsMapLayer *> mapLayersByName( const QString &layerName ) const;

    /**
     * Retrieves a list of matching registered layers by layer \a shortName.
     * If layer's short name is empty a match with layer's name is attempted.
     *
     * \returns list of matching layers
     * \see mapLayer()
     * \see mapLayers()
     * \since QGIS 3.10
     */
    QList<QgsMapLayer *> mapLayersByShortName( const QString &shortName ) const;


    /**
     * Returns a map of all registered layers by layer ID.
     *
     * \param validOnly if set only valid layers will be returned
     * \see mapLayer()
     * \see mapLayersByName()
     * \see layers()
     */
    QMap<QString, QgsMapLayer *> mapLayers( const bool validOnly = false ) const;

    /**
     * Returns TRUE if the project comes from a zip archive, FALSE otherwise.
     */
    bool isZipped() const;

#ifndef SIP_RUN

    /**
     * Returns a list of registered map layers with a specified layer type.
     *
     * ### Example
     *
     *     QVector<QgsVectorLayer*> vectorLayers = QgsProject::instance()->layers<QgsVectorLayer*>();
     *
     * \note not available in Python bindings
     * \see mapLayers()
     * \since QGIS 2.16
     */
    template <typename T>
    QVector<T> layers() const
    {
      return mLayerStore->layers<T>();
    }

    /**
     * Retrieves a list of matching registered layers by layer \a shortName with a specified layer type,
     * if layer's short name is empty a match with layer's name is attempted.
     *
     * \param shortName short name of layers to match
     * \returns list of matching layers
     * \see mapLayer()
     * \see mapLayers()
     * \note not available in Python bindings
     * \since QGIS 3.10
     */
    template <typename T>
    QVector<T> mapLayersByShortName( const QString &shortName ) const
    {
      QVector<T> layers;
      const auto constMapLayers { mLayerStore->layers<T>() };
      for ( const auto l : constMapLayers )
      {
        if ( ! l->shortName().isEmpty() )
        {
          if ( l->shortName() == shortName )
            layers << l;
        }
        else if ( l->name() == shortName )
        {
          layers << l;
        }
      }
      return layers;
    }

#endif

    /**
     * \brief
     * Add a list of layers to the map of loaded layers.
     *
     * The layersAdded() and layerWasAdded() signals will always be emitted.
     * The legendLayersAdded() signal is emitted only if addToLegend is TRUE.
     *
     * \param mapLayers  A list of layer which should be added to the registry
     * \param addToLegend   If TRUE (by default), the layers will be added to the
     *                      legend and to the main canvas. If you have a private
     *                      layer you can set this parameter to FALSE to hide it.
     * \param takeOwnership Ownership will be transferred to the layer registry.
     *                      If you specify FALSE here you have take care of deleting
     *                      the layers yourself. Not available in Python.
     *
     * \returns a list of the map layers that were added
     *         successfully. If a layer or already exists in the registry,
     *         it will not be part of the returned QList.
     *
     * \note As a side-effect QgsProject is made dirty.
     * \note takeOwnership is not available in the Python bindings - the registry will always
     * take ownership
     * \see addMapLayer()
     * \since QGIS 1.8
     */
    QList<QgsMapLayer *> addMapLayers( const QList<QgsMapLayer *> &mapLayers SIP_TRANSFER,
                                       bool addToLegend = true,
                                       bool takeOwnership SIP_PYARGREMOVE = true );

    /**
     * \brief
     * Add a layer to the map of loaded layers.
     *
     * The layersAdded() and layerWasAdded() signals will always be emitted.
     * The legendLayersAdded() signal is emitted only if addToLegend is TRUE.
     * If you are adding multiple layers at once, you should use
     * addMapLayers() instead.
     *
     * \param mapLayer A layer to add to the registry
     * \param addToLegend If TRUE (by default), the layer will be added to the
     *                    legend and to the main canvas. If you have a private
     *                    layer you can set this parameter to FALSE to hide it.
     * \param takeOwnership Ownership will be transferred to the layer registry.
     *                      If you specify FALSE here you have take care of deleting
     *                      the layer yourself. Not available in Python.
     *
     * \returns NULLPTR if unable to add layer, otherwise pointer to newly added layer
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
     * \see removeMapLayer()
     * \see removeAllMapLayers()
     * \since QGIS 1.8
     */
    void removeMapLayers( const QStringList &layerIds );

    //TODO QGIS 4.0 - add PyName alias to avoid list type conversion error

    /**
     * \brief
     * Remove a set of layers from the registry.
     *
     * The specified layers will be removed from the registry. If the registry has ownership
     * of any layers these layers will also be deleted.
     *
     * \param layers A list of layers to remove. NULLPTR values are ignored.
     *
     * \note As a side-effect the QgsProject instance is marked dirty.
     * \see removeMapLayer()
     * \see removeAllMapLayers()
     */
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
     * \param layer The layer to remove. NULLPTR values are ignored.
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
     * Returns the main annotation layer associated with the project.
     *
     * This layer is always present in projects, and will always be rendered
     * above any other map layers during map render jobs.
     *
     * It forms the default location to place new annotation items which
     * should appear above all map layers.
     *
     * \since QGIS 3.16
     */
    QgsAnnotationLayer *mainAnnotationLayer();

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
     * \param trust TRUE to trust the project, FALSE otherwise
     *
     * \since QGIS 3.0
     */
    void setTrustLayerMetadata( bool trust );

    /**
     * Returns TRUE if the trust option is activated, FALSE otherwise. This
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

    /**
     * Attaches a file to the project
     * \param nameTemplate Any filename template, used as a basename for attachment file, i.e. "myfile.ext"
     * \return The path to the file where the contents can be written to.
     * \note If the attachment file is used as a source for a project layer, the attachment will be removed
     * automatically when the layer is deleted.
     * \since QGIS 3.22
     */
    QString createAttachedFile( const QString &nameTemplate );

    /**
     * Returns a map of all attached files with identifier and real paths.
     *
     * \see createAttachedFile()
     * \since QGIS 3.22
     */
    QStringList attachedFiles() const;

    /**
     * Removes the attached file
     * \param path Path to the attached file
     * \return Whether removal succeeded.
     * \see createAttachedFile()
     * \since QGIS 3.22
     */
    bool removeAttachedFile( const QString &path );

    /**
     * Returns an identifier for an attachment file path
     * An attachment identifier is a string which does not depend on the project archive
     * storage location.
     * \param attachedFile An attachment file path
     * \return An identifier for the attached file
     * \since QGIS 3.22
     */
    QString attachmentIdentifier( const QString &attachedFile ) const;

    /**
     * Resolves an attachment identifier to a attachment file path
     * \param identifier An attachment identifier
     * \return The attachment file path, or an empty string if the identifier is invalid
     * \since QGIS 3.22
     */
    QString resolveAttachmentIdentifier( const QString &identifier ) const;

    /**
     * Returns a reference to the project's metadata store.
     * \see setMetadata()
     * \see metadataChanged()
     * \since QGIS 3.2
     */
    const QgsProjectMetadata &metadata() const;

    /**
     * Sets the project's \a metadata store.
     * \see metadata()
     * \see metadataChanged()
     * \since QGIS 3.2
     */
    void setMetadata( const QgsProjectMetadata &metadata );

    /**
     * Returns a set of map layers that are required in the project and therefore they should not get
     * removed from the project. The set of layers may be configured by users in project properties.
     * and it is mainly a hint for the user interface to protect users from removing layers that important
     * in the project. The removeMapLayer(), removeMapLayers() calls do not block removal of layers listed here.
     * \deprecated since QGIS 3.4 use QgsMapLayer::flags() instead
     * \since QGIS 3.2
     */
    Q_DECL_DEPRECATED QSet<QgsMapLayer *> requiredLayers() const;

    /**
     * Configures a set of map layers that are required in the project and therefore they should not get
     * removed from the project. The set of layers may be configured by users in project properties.
     * and it is mainly a hint for the user interface to protect users from removing layers that important
     * in the project. The removeMapLayer(), removeMapLayers() calls do not block removal of layers listed here.
     * \deprecated since QGIS 3.4 use QgsMapLayer::setFlags() instead
     * \since QGIS 3.2
     */
    Q_DECL_DEPRECATED void setRequiredLayers( const QSet<QgsMapLayer *> &layers );

    /**
     * Sets the \a colors for the project's color scheme (see QgsProjectColorScheme).
     *
     * \see projectColorsChanged()
     * \since QGIS 3.6
     */
    void setProjectColors( const QgsNamedColorList &colors );

    /**
     * Sets the default background \a color used by default map canvases.
     *
     * \see backgroundColor()
     * \since QGIS 3.10
     */
    void setBackgroundColor( const QColor &color );

    /**
     * Returns the default background color used by default map canvases.
     *
     * \see setBackgroundColor()
     * \since QGIS 3.10
     */
    QColor backgroundColor() const;

    /**
     * Sets the \a color used to highlight selected features.
     *
     * \see selectionColor()
     * \since QGIS 3.10
     */
    void setSelectionColor( const QColor &color );

    /**
     * Returns the color used to highlight selected features
     *
     * \see setSelectionColor()
     * \since QGIS 3.10
     */
    QColor selectionColor() const;

    /**
     * Sets the list of custom project map \a scales.
     *
     * The \a scales list consists of a list of scale denominator values, e.g.
     * 1000 for a 1:1000 scale.
     *
     * \see mapScales()
     * \see mapScalesChanged()
     *
     * \deprecated Use viewSettings() instead
     */
    Q_DECL_DEPRECATED void setMapScales( const QVector<double> &scales ) SIP_DEPRECATED;

    /**
     * Returns the list of custom project map scales.
     *
     * The scales list consists of a list of scale denominator values, e.g.
     * 1000 for a 1:1000 scale.
     *
     * \see setMapScales()
     * \see mapScalesChanged()
     *
     * \deprecated Use viewSettings() instead
     */
    Q_DECL_DEPRECATED QVector<double> mapScales() const SIP_DEPRECATED;

    /**
     * Sets whether project mapScales() are \a enabled.
     *
     * \see useProjectScales()
     * \see setMapScales()
     *
     * \deprecated Use viewSettings() instead
     */
    Q_DECL_DEPRECATED void setUseProjectScales( bool enabled ) SIP_DEPRECATED;

    /**
     * Returns TRUE if project mapScales() are enabled.
     *
     * \see setUseProjectScales()
     * \see mapScales()
     *
     * \deprecated Use viewSettings() instead
     */
    Q_DECL_DEPRECATED bool useProjectScales() const SIP_DEPRECATED;

    /**
     * Triggers the collection strings of .qgs to be included in ts file and calls writeTsFile()
     * \since QGIS 3.4
     */
    void generateTsFile( const QString &locale );

    /**
     * Translates the project with QTranslator and qm file
     * \returns the result string (in case there is no QTranslator loaded the sourceText)
     *
     * \param context describing layer etc.
     * \param sourceText is the identifier of this text
     * \param disambiguation it's the disambiguation
     * \param n if -1 uses the appropriate form
     * \since QGIS 3.4
     */
    QString translate( const QString &context, const QString &sourceText, const char *disambiguation = nullptr, int n = -1 ) const override;

    /**
     * Accepts the specified style entity \a visitor, causing it to visit all style entities associated
     * with the project.
     *
     * Returns TRUE if the visitor should continue visiting other objects, or FALSE if visiting
     * should be canceled.
     *
     * \since QGIS 3.10
     */
    bool accept( QgsStyleEntityVisitorInterface *visitor ) const;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsProject: '%1'%2>" ).arg( sipCpp->fileName(),
                  sipCpp == QgsProject::instance() ? QStringLiteral( " (singleton instance)" ) : QString() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  signals:

    /**
     * Emitted when the project is cleared (and additionally when an open project is cleared
     * just before a new project is read).
     *
     * \see clear()
     * \since QGIS 3.2
     */
    void cleared();

    /**
     * Emitted when a project is being read.
     */
    void readProject( const QDomDocument & );

    /**
     * Emitted when a project is being read. And passing the /a context
     */
    void readProjectWithContext( const QDomDocument &, QgsReadWriteContext &context );

    /**
     * Emitted when the project is being written.
     */
    void writeProject( QDomDocument & );

    /**
     * Emitted after the basic initialization of a layer from the project
     * file is done. You can use this signal to read additional information
     * from the project file.
     *
     * \param mapLayer  The map layer which is being initialized
     * \param layerNode The layer node from the project file
     */
    void readMapLayer( QgsMapLayer *mapLayer, const QDomElement &layerNode );

    /**
     * Emitted when a layer is being saved. You can use this method to save
     * additional information to the layer.
     *
     * \param mapLayer  The map layer which is being initialized
     * \param layerElem The layer element from the project file
     * \param doc The document
     */
    void writeMapLayer( QgsMapLayer *mapLayer, QDomElement &layerElem, QDomDocument &doc );

    /**
     * Emitted when the project file has been written and closed.
     */
    void projectSaved();

    /**
     * Emitted when an old project file is read.
     *
     * \deprecated use readVersionMismatchOccurred() instead.
     */
    Q_DECL_DEPRECATED void oldProjectVersionWarning( const QString & ) SIP_DEPRECATED;

    /**
     * Emitted when a project is read and the version of QGIS used to save
     * the project differs from the current QGIS version.
     *
     * The \a fileVersion argument indicates the version of QGIS used to save
     * the project.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.26
     */
    void readVersionMismatchOccurred( const QString &fileVersion );

    /**
     * Emitted when a layer from a projects was read.
     * \param i current layer
     * \param n number of layers
     */
    void layerLoaded( int i, int n );

    //! Emitted when a layer is loaded
    void loadingLayer( const QString &layerName );

    /**
     * \brief Emitted when loading layers has produced some messages
     * \param layerName the layer name
     * \param messages a list of pairs of Qgis::MessageLevel and messages
     * \since 3.2
     */
    void loadingLayerMessageReceived( const QString &layerName, const QList<QgsReadWriteContext::ReadWriteMessage> &messages );

    /**
     * Emitted when the list of layer which are excluded from map identification changes
     * \deprecated since QGIS 3.4
     */
    Q_DECL_DEPRECATED void nonIdentifiableLayersChanged( QStringList nonIdentifiableLayers );

    //! Emitted when the file name of the project changes
    void fileNameChanged();

    /**
     * Emitted when the home path of the project changes.
     * \see setPresetHomePath()
     * \see homePath()
     * \see presetHomePath()
     */
    void homePathChanged();

    /**
     * Emitted whenever the configuration for snapping has changed.
     */
    void snappingConfigChanged( const QgsSnappingConfig &config );

    /**
     * Emitted whenever the avoid intersections mode has changed.
     *
     * \since QGIS 3.14
     */
    void avoidIntersectionsModeChanged();

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
     * \see setEllipsoid()
     * \see ellipsoid()
     * \since QGIS 3.0
     */
    void ellipsoidChanged( const QString &ellipsoid );


    /**
     * Emitted when the project transformContext() is changed.
     *
     * \see transformContext()
     * \since QGIS 3.0
     */
    void transformContextChanged();

    /**
     * Emitted when datum transforms stored in the project are not available locally.
     * \since QGIS 3.0
     */
    void missingDatumTransforms( const QStringList &missingTransforms );

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

    /**
     * Emitted when the project's metadata is changed.
     * \see setMetadata()
     * \see metadata()
     * \since QGIS 3.2
     */
    void metadataChanged();

    /**
     * Emitted whenever the project's color scheme has been changed.

     * \see setProjectColors()
     * \since QGIS 3.6
     */
    void projectColorsChanged();

    /**
     * Emitted whenever the project's canvas background color has been changed.

     * \see setBackgroundColor()
     * \since QGIS 3.10
     */
    void backgroundColorChanged();

    /**
     * Emitted whenever the project's selection color has been changed.

     * \see setSelectionColor()
     * \since QGIS 3.10
     */
    void selectionColorChanged();

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

    //TODO QGIS 4.0 - rename to past tense

    /**
     * Emitted when all layers are removed, before layersWillBeRemoved() and
     * layerWillBeRemoved() signals are emitted. The layersWillBeRemoved() and
     * layerWillBeRemoved() signals will still be emitted following this signal.
     * You can use this signal to do easy (and fast) cleanup.
     */
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

    /**
     * Emitted when the project dirty status changes.
     *
     * \param dirty TRUE if the project is in a dirty state and has pending unsaved changes.
     *
     * \since QGIS 3.2
     */
    void isDirtyChanged( bool dirty );

    /**
     * Emitted when setDirty(true) is called.
     * \note As opposed to isDirtyChanged(), this signal is invoked every time setDirty(true)
     * is called, regardless of whether the project was already dirty.
     *
     * \since QGIS 3.20
     */
    void dirtySet();

    /**
     * Emitted when the list of custom project map scales changes.
     *
     * \see mapScales()
     * \see setMapScales()
     *
     * \deprecated Use viewSettings() instead
     */
    Q_DECL_DEPRECATED void mapScalesChanged() SIP_DEPRECATED;

  public slots:

    /**
     * The snapping configuration for this project.
     *
     * \since QGIS 3.0
     */
    void setSnappingConfig( const QgsSnappingConfig &snappingConfig );

    // TODO QGIS 4.0 - rename b to dirty

    /**
     * Flag the project as dirty (modified). If this flag is set, the user will
     * be asked to save changes to the project before closing the current project.
     *
     * \note promoted to public slot in 2.16
     * \since QGIS 2.4
     */
    void setDirty( bool b = true );

    /**
     * Sets the project's home \a path. If an empty path is specified than the
     * home path will be automatically determined from the project's file path.
     * \see presetHomePath()
     * \see homePath()
     * \see homePathChanged()
     * \since QGIS 3.2
    */
    void setPresetHomePath( const QString &path );

    /**
     * Registers the containers that require translation into the translationContext.
     * This is a recursive function to get all the child containers.
     *
     * \param translationContext where the objects will be registered
     * \param parent parent-container containing list of children
     * \param layerId to store under the correct context
     * \since QGIS 3.4
    */
    void registerTranslatableContainers( QgsTranslationContext *translationContext, QgsAttributeEditorContainer *parent, const QString &layerId );

    /**
     * Registers the objects that require translation into the \a translationContext.
     * So there can be created a ts file with these values.
     *
     * \since QGIS 3.4
    */
    void registerTranslatableObjects( QgsTranslationContext *translationContext );

    /**
     * Sets the data defined properties used for overrides in user defined server
     * parameters to \a properties
     *
     * \since QGIS 3.14
     */
    void setDataDefinedServerProperties( const QgsPropertyCollection &properties );

    /**
     * Returns the data defined properties used for overrides in user defined server
     * parameters
     *
     * \since QGIS 3.14
     */
    QgsPropertyCollection dataDefinedServerProperties() const;

    /**
     * Makes the layer editable.
     *
     * This starts an edit session on vectorLayer. Changes made in this edit session will not
     * be made persistent until commitChanges() is called, and can be reverted by calling
     * rollBack().
     *
     * \returns TRUE if the layer was successfully made editable, or FALSE if the operation
     * failed (e.g. due to an underlying read-only data source, or lack of edit support
     * by the backend data provider).
     *
     * \see commitChanges()
     * \see rollBack()
     *
     * \since QGIS 3.26
     */
    bool startEditing( QgsVectorLayer *vectorLayer = nullptr );

    /**
     * Attempts to commit to the underlying data provider any buffered changes made since the
     * last to call to startEditing().
     *
     * Returns the result of the attempt. If a commit fails (i.e. FALSE is returned), the
     * in-memory changes are left untouched and are not discarded. This allows editing to
     * continue if the commit failed on e.g. a disallowed value in a Postgres
     * database - the user can re-edit and try again.
     *
     * The commits occur in distinct stages,
     * (add attributes, add features, change attribute values, change
     * geometries, delete features, delete attributes)
     * so if a stage fails, it can be difficult to roll back cleanly.
     * Therefore any error message returned by commitErrors also includes which stage failed so
     * that the user has some chance of repairing the damage cleanly.
     *
     * By setting \a stopEditing to FALSE, the layer will stay in editing mode.
     * Otherwise the layer editing mode will be disabled if the commit is successful.
     *
     * \see startEditing()
     * \see rollBack()
     *
     * \since QGIS 3.26
     */
    bool commitChanges( QStringList &commitErrors SIP_OUT, bool stopEditing = true, QgsVectorLayer *vectorLayer = nullptr );

    /**
     * Stops a current editing operation on vectorLayer and discards any uncommitted edits.
     *
     * \see startEditing()
     * \see commitChanges()
     *
     * \since QGIS 3.26
     */
    bool rollBack( QStringList &rollbackErrors SIP_OUT, bool stopEditing = true, QgsVectorLayer *vectorLayer = nullptr );

  private slots:
    void onMapLayersAdded( const QList<QgsMapLayer *> &layers );
    void onMapLayersRemoved( const QList<QgsMapLayer *> &layers );
    void cleanTransactionGroups( bool force = false );
    void updateTransactionGroups();

  private:

    static QgsProject *sProject;


    /**
     * Read map layers from project file.
     * \param doc DOM document to parse
     * \param brokenNodes a list of DOM nodes corresponding to layers that we were unable to load; this could be
     * because the layers were removed or re-located after the project was last saved
     * \param flags optional project reading flags
     * \returns TRUE if function worked; else is FALSE
    */
    bool _getMapLayers( const QDomDocument &doc, QList<QDomNode> &brokenNodes, QgsProject::ReadFlags flags = QgsProject::ReadFlags() );

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
     * Creates layer and adds it to maplayer registry.
     *
     * The optional \a flags argument can be used to control layer reading behavior.
     *
     * \note not available in Python bindings
     */
    bool addLayer( const QDomElement &layerElem, QList<QDomNode> &brokenNodes, QgsReadWriteContext &context, QgsProject::ReadFlags flags = QgsProject::ReadFlags() ) SIP_SKIP;

    /**
     * The optional \a flags argument can be used to control layer reading behavior.
     *
     * \note not available in Python bindings
    */
    void initializeEmbeddedSubtree( const QString &projectFilePath, QgsLayerTreeGroup *group, QgsProject::ReadFlags flags = QgsProject::ReadFlags() ) SIP_SKIP;

    /**
     * The optional \a flags argument can be used to control layer reading behavior.
     * \note not available in Python bindings
     */
    bool loadEmbeddedNodes( QgsLayerTreeGroup *group, QgsProject::ReadFlags flags = QgsProject::ReadFlags() ) SIP_SKIP;

    //! Read .qgs file
    bool readProjectFile( const QString &filename, QgsProject::ReadFlags flags = QgsProject::ReadFlags() );

    //! Write .qgs file
    bool writeProjectFile( const QString &filename );

    //! Unzip .qgz file then read embedded .qgs file
    bool unzip( const QString &filename, QgsProject::ReadFlags flags = QgsProject::ReadFlags() );

    //! Zip project
    bool zip( const QString &filename );

    //! Save auxiliary storage to database
    bool saveAuxiliaryStorage( const QString &filename = QString() );

    //! Returns the property definition used for a data defined server property
    static QgsPropertiesDefinition &dataDefinedServerPropertyDefinitions();

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
    AvoidIntersectionsMode mAvoidIntersectionsMode = AvoidIntersectionsMode::AllowIntersections;

    QgsRelationManager *mRelationManager = nullptr;

    std::unique_ptr<QgsAnnotationManager> mAnnotationManager;
    std::unique_ptr<QgsLayoutManager> mLayoutManager;
    std::unique_ptr<QgsMapViewsManager> m3DViewsManager;

    QgsBookmarkManager *mBookmarkManager = nullptr;

    QgsProjectViewSettings *mViewSettings = nullptr;

    QgsProjectTimeSettings *mTimeSettings = nullptr;

    QgsProjectDisplaySettings *mDisplaySettings = nullptr;

    QgsLayerTree *mRootGroup = nullptr;

    QgsLayerTreeRegistryBridge *mLayerTreeRegistryBridge = nullptr;

    QgsAnnotationLayer *mMainAnnotationLayer = nullptr;

    //! map of transaction group: QPair( providerKey, connString ) -> transactionGroup
    QMap< QPair< QString, QString>, QgsTransactionGroup *> mTransactionGroups;

    QgsVectorLayerEditBufferGroup mEditBufferGroup;

    std::unique_ptr<QgsMapThemeCollection> mMapThemeCollection;

    std::unique_ptr<QgsLabelingEngineSettings> mLabelingEngineSettings;

    QVariantMap mCustomVariables;

    std::unique_ptr<QgsArchive> mArchive;

    std::unique_ptr<QgsAuxiliaryStorage> mAuxiliaryStorage;

    QFile mFile;                 // current physical project file

    QString mOriginalPath;

    QString mSaveUser;              // last saved user.
    QString mSaveUserFull;          // last saved user full name.
    QDateTime mSaveDateTime;
    QgsProjectVersion mSaveVersion;

    /**
     * Manual override for project home path - if empty, home path is automatically
     * created based on file name.
     */
    QString mHomePath;
    mutable QString mCachedHomePath;

    QColor mBackgroundColor;
    QColor mSelectionColor;

    mutable QgsProjectPropertyKey mProperties;  // property hierarchy, TODO: this shouldn't be mutable
    Qgis::TransactionMode mTransactionMode = Qgis::TransactionMode::Disabled; // transaction grouped editing
    bool mEvaluateDefaultValues = false; // evaluate default values immediately
    QgsCoordinateReferenceSystem mCrs;
    bool mDirty = false;                 // project has been modified since it has been read or saved
    int mDirtyBlockCount = 0;
    bool mTrustLayerMetadata = false;

    QgsPropertyCollection mDataDefinedServerProperties;

    QgsCoordinateTransformContext mTransformContext;

    QgsProjectMetadata mMetadata;

    std::unique_ptr< QTranslator > mTranslator;

    bool mIsBeingDeleted = false;

    QgsSettings mSettings;

    mutable std::unique_ptr< QgsExpressionContextScope > mProjectScope;

    int mBlockSnappingUpdates = 0;

    friend class QgsProjectDirtyBlocker;

    // Required to avoid creating a new project in it's destructor
    friend class QgsProviderRegistry;

    // Required by QGIS Server for switching the current project instance
    friend class QgsServer;

    friend class TestQgsProject;

    Q_DISABLE_COPY( QgsProject )
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsProject::ReadFlags )

/**
 * \brief Temporarily blocks QgsProject "dirtying" for the lifetime of the object.
 *
 * QgsProjectDirtyBlocker supports "stacked" blocking, so two QgsProjectDirtyBlockers created
 * for the same project will both need to be destroyed before the project can be dirtied again.
 *
 * Note that QgsProjectDirtyBlocker only blocks calls which set the project as dirty - calls
 * which set the project as clean are not blocked.
 *
 * Python scripts should not use QgsProjectDirtyBlocker directly. Instead, use QgsProject.blockDirtying()
 * \code{.py}
 *   project = QgsProject.instance()
 *   with QgsProject.blockDirtying(project):
 *     # do something
 * \endcode
 *
 * \see QgsProject::setDirty()
 *
 * \ingroup core
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProjectDirtyBlocker
{
  public:

    /**
     * Constructor for QgsProjectDirtyBlocker.
     *
     * This will block dirtying the specified \a project for the lifetime of this object.
     */
    QgsProjectDirtyBlocker( QgsProject *project )
      : mProject( project )
    {
      mProject->mDirtyBlockCount++;
    }

    //! QgsProjectDirtyBlocker cannot be copied
    QgsProjectDirtyBlocker( const QgsProjectDirtyBlocker &other ) = delete;

    //! QgsProjectDirtyBlocker cannot be copied
    QgsProjectDirtyBlocker &operator=( const QgsProjectDirtyBlocker &other ) = delete;

    ~QgsProjectDirtyBlocker()
    {
      mProject->mDirtyBlockCount--;
    }

  private:
    QgsProject *mProject = nullptr;

#ifdef SIP_RUN
    QgsProjectDirtyBlocker( const QgsProjectDirtyBlocker &other );
#endif
};

/**
 * Returns the version string found in the given DOM document
 * \returns the version string or an empty string if none found
 * \note not available in Python bindings.
 */
CORE_EXPORT QgsProjectVersion getVersion( QDomDocument const &doc ) SIP_SKIP;



/// @cond PRIVATE
#ifndef SIP_RUN
class GetNamedProjectColor : public QgsScopedExpressionFunction
{
  public:
    GetNamedProjectColor( const QgsProject *project );

    /**
     * Optimized constructor for GetNamedProjectColor when a list of map is already available
     * and does not need to be read from a project.
     */
    GetNamedProjectColor( const QHash< QString, QColor > &colors );

    QVariant func( const QVariantList &values, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * ) override;
    QgsScopedExpressionFunction *clone() const override;

  private:

    QHash< QString, QColor > mColors;

};

#endif
///@endcond

#endif
