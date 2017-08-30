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
#include "qgsvectorlayerjoininfo.h"
#include "qgsproperty.h"

#include <sqlite3.h>

#include <QString>

class QgsProject;

/**
 * \class QgsAuxiliaryField
 *
 * \ingroup core
 *
 * \brief Class allowing to manage fields from of auxiliary layers
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsAuxiliaryField : public QgsField
{
  public:

    /**
     * Constructor
     *
     * \param def Definition of the property to be stored by this auxiliary
     *  field.
     */
    QgsAuxiliaryField( const QgsPropertyDefinition &def );

    /**
     * Destructor
     */
    virtual ~QgsAuxiliaryField() = default;

    /**
     * Returns the property definition corresponding to this field.
     */
    QgsPropertyDefinition propertyDefinition() const;

    /**
     * Returns the name of the field.
     */
    using QgsField::name SIP_SKIP;

    /**
     * Returns the name of the auxiliary field for a property definition.
     *
     * \returns def The property definition
     * \returns joined The join prefix is tok into account if true
     */
    static QString name( const QgsPropertyDefinition &def, bool joined = false );

  private:
    QgsAuxiliaryField( const QgsField &f ); // only for auxiliary layer

    void init( const QgsPropertyDefinition &def );

    QgsPropertyDefinition mPropertyDefinition;

    friend class QgsAuxiliaryLayer;
};

typedef QList<QgsAuxiliaryField> QgsAuxiliaryFields;

/**
 * \class QgsAuxiliaryLayer
 *
 * \ingroup core
 *
 * Class allowing to manage the auxiliary storage for a vector layer.
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
    QgsAuxiliaryLayer( const QString &pkField, const QString &filename, const QString &table, const QgsVectorLayer *vlayer );

    /**
     * Destructor
     */
    virtual ~QgsAuxiliaryLayer() = default;

    /**
     * Copy constructor deactivated
     */
    QgsAuxiliaryLayer( const QgsAuxiliaryLayer &rhs ) = delete;

    QgsAuxiliaryLayer &operator=( QgsAuxiliaryLayer const &rhs ) = delete;

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
     * \returns true if changes are committed without error, false otherwise.
     */
    bool clear();

    /**
     * Returns information to use for joining with primary key and so on.
     */
    QgsVectorLayerJoinInfo joinInfo() const;

    /**
     * Returns true if the property is stored in the layer yet, false
     * otherwise.
     *
     * \param definition The property definition to check
     *
     * \returns true if the property is stored, false otherwise
     */
    bool exists( const QgsPropertyDefinition &definition ) const;

    /**
     * Add an an auxiliary field for the given property.
     *
     * \param definition The definition of the property to add
     *
     * \returns true if the auxiliary field is well added, false otherwise
     */
    bool addAuxiliaryField( const QgsPropertyDefinition &definition );

    /**
     * Returns a list of all auxiliary fields currently managed by the layer.
     */
    QgsAuxiliaryFields auxiliaryFields() const;

    /**
     * Commit changes and starts editing then.
     *
     * \returns true if commit step passed, false otherwise
     */
    bool save();

    /**
     * Remove attribute from the layer and commit changes. The layer remains
     * editable.
     *
     * \param attr The index of the attribute to remove
     *
     * \returns true if the attribute is well deleted, false otherwise
     */
    virtual bool deleteAttribute( int attr ) override;

  private:
    QgsVectorLayerJoinInfo mJoinInfo;
    const QgsVectorLayer *mLayer = nullptr;
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
     * Returns the status of the auxiliary storage currently definied.
     *
     * \returns true if the auxiliary storage is valid, false otherwise
     */
    bool isValid() const;

    /**
     * Returns the target filename of the database.
     */
    QString fileName() const;

    /**
     * Returns the path of current database used. It may be different from the
     * target filename if the auxiliary storage is opened in copy mode.
     */
    QString currentFileName() const;

    /**
     * Saves the current database to a new path.
     *
     * \returns true if everything is saved, false otherwise
     */
    bool saveAs( const QString &filename ) const;

    /**
     * Saves the current database to a new path for a specific project.
     * Actually, the current filename of the project is used to deduce the
     * path of the database to save.
     *
     * \returns true if everything is saved, false otherwise
     */
    bool saveAs( const QgsProject &project ) const;

    /**
     * Saves the current database.
     *
     * \returns true if everything is saved, false otherwise
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
     * \returns A new auxiliary layer or a nullptr if an error happened.
     */
    QgsAuxiliaryLayer *createAuxiliaryLayer( const QgsField &field, const QgsVectorLayer *layer ) const SIP_FACTORY;

    /**
     * Removes a table from the auxiliary storage.
     *
     * \returns true if the table is well deleted, false otherwise
     */
    static bool deleteTable( const QgsDataSourceUri &uri );

    /**
     * Returns the extension used for auxiliary databases.
     */
    static QString extension();

  private:
    sqlite3 *open( const QString &filename = QString() );
    sqlite3 *open( const QgsProject &project );

    void initTmpFileName();

    static QString filenameForProject( const QgsProject &project );
    static sqlite3 *createDB( const QString &filename );
    static sqlite3 *openDB( const QString &filename );
    static void close( sqlite3 *handler );
    static bool tableExists( const QString &table, sqlite3 *handler );
    static bool createTable( const QString &type, const QString &table, sqlite3 *handler );

    static bool exec( const QString &sql, sqlite3 *handler );
    static void debugMsg( const QString &sql, sqlite3 *handler );

    bool mValid = false;
    QString mFileName; // original filename
    QString mTmpFileName; // temporary filename used in copy mode
    bool mCopy = false;
};

#endif
