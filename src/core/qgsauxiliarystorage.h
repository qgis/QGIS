/***************************************************************************
                          qgsauxiliarystorage.h  -  description
                           -------------------
    begin                : Aug 28, 2017
    copyright            : (C) 2017 by Paul Blottiere
    email                : paul.blottiere@oslandia.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUXILIARYSTORAGE_H
#define QGSAUXILIARYSTORAGE_H

#include "qgis_core.h"
#include "qgsdatasourceuri.h"
#include "qgspallabeling.h"
#include "qgsdiagramrenderer.h"
#include "qgsvectorlayerjoininfo.h"
#include "qgsproperty.h"
#include "qgssqliteutils.h"
#include "qgsvectorlayer.h"
#include "qgscallout.h"
#include <QString>

class QgsProject;

/**
 * \class QgsAuxiliaryLayer
 *
 * \ingroup core
 *
 * \brief Class allowing to manage the auxiliary storage for a vector layer.
 *
 * Such auxiliary data are data used mostly for the needs of QGIS (symbology)
 * and have no real interest in being stored with the native raw geospatial
 * data.
 *
 * The need arises from the restrictions existing in the manual placement of
 * labels. Manual placement of labels are possible in QGIS by setting some
 * labeling properties (X and Y position, and rotation angle optionally) as
 * being "data-defined", meaning that values come from a column (or an
 * expression). But setting this up on an existing layer requires either to
 * add new columns to the source layer, while it is not always possible or
 * desirable.
 *
 * This QgsAuxiliaryLayer provides the solution to this limitation. Actually
 * it's an editable join to the original vector layer with some
 * synchronisation mechanisms activated such as "Upsert On Edit" or "Delete
 * Cascade". Thus, auxiliary fields are editable even if the
 * source layer is not and edition of a joined field is also possible.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsAuxiliaryLayer : public QgsVectorLayer
{
    Q_OBJECT

  public:

    /**
     * Constructor
     *
     * \param pkField The primary key to use for joining
     * \param filename The database path
     * \param table The table name
     * \param vlayer The target vector layer in join definition
     */
    QgsAuxiliaryLayer( const QString &pkField, const QString &filename, const QString &table, QgsVectorLayer *vlayer );

    /**
     * Copy constructor deactivated
     */
    QgsAuxiliaryLayer( const QgsAuxiliaryLayer &rhs ) = delete;

    QgsAuxiliaryLayer &operator=( QgsAuxiliaryLayer const &rhs ) = delete;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsAuxiliaryLayer: '%1'>" ).arg( sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

    /**
     * Returns a new instance equivalent to this one. The underlying table
     * is duplicate for the layer given in parameter. Note that the current
     * auxiliary layer should be saved to have a proper duplicated table.
     *
     * \param layer The layer for which the clone is made
     */
    QgsAuxiliaryLayer *clone( QgsVectorLayer *layer ) const SIP_FACTORY;
#ifdef __clang__
#pragma clang diagnostic pop
#endif

    /**
     * An auxiliary layer is not spatial. This method returns a spatial
     * representation of auxiliary data.
     *
     * \returns A new spatial vector layer
     */
    QgsVectorLayer *toSpatialLayer() const;

    /**
     * Deletes all features from the layer. Changes are automatically committed
     * and the layer remains editable.
     *
     * \returns TRUE if changes are committed without error, FALSE otherwise.
     */
    bool clear();

    /**
     * Returns information to use for joining with primary key and so on.
     */
    QgsVectorLayerJoinInfo joinInfo() const;

    /**
     * Returns TRUE if the property is stored in the layer already, FALSE
     * otherwise.
     *
     * \param definition The property definition to check
     *
     * \returns TRUE if the property is stored, FALSE otherwise
     */
    bool exists( const QgsPropertyDefinition &definition ) const;

    /**
     * Adds an auxiliary field for the given property. Setup for widget
     * editors are updated in the target layer as well as the attribute
     * table config to hide auxiliary fields by default.
     *
     * \param definition The definition of the property to add
     *
     * \returns TRUE if the auxiliary field is well added, FALSE otherwise
     */
    bool addAuxiliaryField( const QgsPropertyDefinition &definition );

    /**
     * Returns a list of all auxiliary fields currently managed by the layer.
     */
    QgsFields auxiliaryFields() const;

    /**
     * Commits changes and starts editing then.
     *
     * \returns TRUE if commit step passed, FALSE otherwise
     */
    bool save();

    /**
     * Removes attribute from the layer and commits changes. The layer remains
     * editable.
     *
     * \param attr The index of the attribute to remove
     *
     * \returns TRUE if the attribute is well deleted, FALSE otherwise
     */
    bool deleteAttribute( int attr ) override;

    /**
     * Returns TRUE if the underlying field has to be hidden from editing
     * tools like attribute table, FALSE otherwise.
     *
     * \param index The index of the field for which visibility is checked
     */
    bool isHiddenProperty( int index ) const;

    /**
     * Returns the index of the auxiliary field for a specific property
     * definition.
     *
     * \param definition The property definition
     *
     * \returns The index of the field corresponding to the property or -1
     */
    int indexOfPropertyDefinition( const QgsPropertyDefinition &definition ) const;

    /**
     * Returns the underlying property key for the field index. The key may be
     * a PAL, diagram or symbology property according to the underlying
     * property definition of the field. The key -1 is returned if an error
     * happened.
     *
     * \param index The index of the field
     */
    int propertyFromIndex( int index ) const;

    /**
     * Returns the property definition for the underlying field index.
     *
     * \param index The index of the field
     */
    QgsPropertyDefinition propertyDefinitionFromIndex( int index ) const;

    /**
     * Creates if necessary a new auxiliary field for a PAL property and
     * activates this property in settings.
     *
     * \param property The property to create
     * \param vlayer The vector layer
     * \param overwriteExisting since QGIS 3.22, controls whether an existing property should be completely overwritten or upgraded to a coalesce("new aux field", 'existing' || 'property' || 'expression') type property
     *
     * \returns The index of the auxiliary field or -1
     */
    static int createProperty( QgsPalLayerSettings::Property property, QgsVectorLayer *vlayer, bool overwriteExisting = true );

    /**
     * Creates if necessary a new auxiliary field for a diagram's property and
     * activates this property in settings.
     *
     * \param property The property to create
     * \param vlayer The vector layer
     * \param overwriteExisting since QGIS 3.22, controls whether an existing property should be completely overwritten or upgraded to a coalesce("new aux field", 'existing' || 'property' || 'expression') type property
     *
     * \returns The index of the auxiliary field or -1
     */
    static int createProperty( QgsDiagramLayerSettings::Property property, QgsVectorLayer *vlayer, bool overwriteExisting = true );

    /**
     * Creates if necessary a new auxiliary field for a callout's property and
     * activates this property in settings.
     *
     * \param property The property to create
     * \param vlayer The vector layer
     * \param overwriteExisting since QGIS 3.22, controls whether an existing property should be completely overwritten or upgraded to a coalesce("new aux field", 'existing' || 'property' || 'expression') type property
     *
     * \returns The index of the auxiliary field or -1
     * \since QGIS 3.20
     */
    static int createProperty( QgsCallout::Property property, QgsVectorLayer *vlayer, bool overwriteExisting = true );

    /**
     * Creates a new auxiliary field from a property definition.
     *
     * \param definition The property definition of the auxiliary field to create
     */
    static QgsField createAuxiliaryField( const QgsPropertyDefinition &definition );

    /**
     * Creates a new auxiliary field from a field.
     *
     * \param field The field to use to create the auxiliary field
     */
    static QgsField createAuxiliaryField( const QgsField &field );

    /**
     * Returns the name of the auxiliary field for a property definition.
     *
     * \param def The property definition
     * \param joined The join prefix is taken into account if TRUE
     */
    static QString nameFromProperty( const QgsPropertyDefinition &def, bool joined = false );

    /**
     * Returns the property definition from an auxiliary field.
     *
     * \param field The auxiliary field
     */
    static QgsPropertyDefinition propertyDefinitionFromField( const QgsField &field );

  private:
    QgsVectorLayerJoinInfo mJoinInfo;
    QString mFileName;
    QString mTable;
    QgsVectorLayer *mLayer = nullptr;
};


/**
 * \class QgsAuxiliaryStorage
 *
 * \ingroup core
 *
 * \brief Class providing some utility methods to manage auxiliary storage.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsAuxiliaryStorage
{
  public:

    /**
     * Constructor.
     *
     * The project filename is used to build a database path at the same
     * location, but with a different extension. Then, it's the same logic as
     * described for \see QgsAuxiliaryStorage(const QString &, bool copy).
     *
     *
     * \param project The project for which the auxiliary storage has to be used
     * \param copy Parameter indicating if a copy of the database has to be used
     */
    QgsAuxiliaryStorage( const QgsProject &project, bool copy = true );

    /**
     * Constructor.
     *
     * If a valid database path is given in parameter and copy mode is
     * deactivated, then every changes is directly committed on the original
     * database. But if the copy mode is activated, then changes are committed
     * on a copy of the database (a temporary file) and a save action is
     * therefore necessary to keep modifications in the original file.
     *
     * If an empty string for the database path is given in parameter, then
     * a database is created in a temporary file whatever the copy mode.
     *
     * If the database path given in parameter is not empty but does not exist,
     * then a database is created at this location when copy mode is
     * deactivated. When copy mode is activated, a temporary database is used
     * instead and a save action will be necessary to create the database at
     * the original location given in parameter.
     *
     * \param filename The path of the database
     * \param copy Parameter indicating if a copy of the database has to be used
     */
    QgsAuxiliaryStorage( const QString &filename = QString(), bool copy = true );

    /**
     * Destructor.
     */
    virtual ~QgsAuxiliaryStorage();

    /**
     * Returns the status of the auxiliary storage currently defined.
     *
     * \returns TRUE if the auxiliary storage is valid, FALSE otherwise
     */
    bool isValid() const;

    /**
     * Returns the target filename of the database.
     */
    QString fileName() const;

    /**
     * Returns the path of the current database used. It may be different from
     * the target filename if the auxiliary storage is opened in copy mode.
     */
    QString currentFileName() const;

    /**
     * Returns the underlying error string describing potential errors
     * happening in saveAs(). Empty by default.
     *
     * \since QGIS 3.4
     */
    QString errorString() const;

    /**
     * Saves the current database to a new path.
     *
     * \returns TRUE if everything is saved, FALSE otherwise
     */
    bool saveAs( const QString &filename );

    /**
     * Saves the current database to a new path for a specific project.
     * Actually, the current filename of the project is used to deduce the
     * path of the database to save.
     *
     * \returns TRUE if everything is saved, FALSE otherwise
     */
    bool saveAs( const QgsProject &project );

    /**
     * Saves the current database.
     *
     * \returns TRUE if everything is saved, FALSE otherwise
     */
    bool save() const;

    /**
     * Creates an auxiliary layer for a vector layer. A new table is created if
     * necessary. The primary key to use to construct the auxiliary layer is
     * given in parameter.
     *
     * \param field The primary key to join
     * \param layer The vector layer for which the auxiliary layer has to be created
     *
     * \returns A new auxiliary layer or NULLPTR if an error happened.
     */
    QgsAuxiliaryLayer *createAuxiliaryLayer( const QgsField &field, QgsVectorLayer *layer ) const SIP_FACTORY;

    /**
     * Removes a table from the auxiliary storage.
     *
     * \param uri The uri of the table to remove
     *
     * \returns TRUE if the table is well deleted, FALSE otherwise
     */
    static bool deleteTable( const QgsDataSourceUri &uri );

    /**
     * Duplicates a table and its content.
     *
     * \param uri The uri of the table to duplicate
     * \param newTable The name of the new table
     *
     * \returns TRUE if the table is well duplicated, FALSE otherwise
     */
    static bool duplicateTable( const QgsDataSourceUri &uri, const QString &newTable );

    /**
     * Returns the extension used for auxiliary databases.
     */
    static QString extension();

    /**
     * Returns TRUE if the auxiliary database yet exists for a project, FALSE otherwise.
     *
     * \param project The project for which the database is checked
     *
     * \since QGIS 3.2
     */
    static bool exists( const QgsProject &project );

  private:
    sqlite3_database_unique_ptr open( const QString &filename = QString() );
    sqlite3_database_unique_ptr open( const QgsProject &project );

    void initTmpFileName();

    static QString filenameForProject( const QgsProject &project );
    static sqlite3_database_unique_ptr createDB( const QString &filename );
    static sqlite3_database_unique_ptr openDB( const QString &filename );
    static bool tableExists( const QString &table, sqlite3 *handler );
    static bool createTable( const QString &type, const QString &table, sqlite3 *handler, QString &errorMsg );

    static bool exec( const QString &sql, sqlite3 *handler );
    static QString debugMsg( const QString &sql, sqlite3 *handler );

    static QgsDataSourceUri parseOgrUri( const QgsDataSourceUri &uri );

    bool mValid = false;
    QString mFileName; // original filename
    QString mTmpFileName; // temporary filename used in copy mode
    bool mCopy = false;
    mutable QString mErrorString;
};

#endif
