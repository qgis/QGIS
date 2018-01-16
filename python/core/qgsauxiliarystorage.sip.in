/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsauxiliarystorage.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsAuxiliaryLayer : QgsVectorLayer
{
%Docstring


Class allowing to manage the auxiliary storage for a vector layer.

Such auxiliary data are data used mostly for the needs of QGIS (symbology)
and have no real interest in being stored with the native raw geospatial
data.

The need arises from the restrictions existing in the manual placement of
labels. Manual placement of labels are possible in QGIS by setting some
labeling properties (X and Y position, and rotation angle optionally) as
being "data-defined", meaning that values come from a column (or an
expression). But setting this up on an existing layer requires either to
add new columns to the source layer, while it is not always possible or
desirable.

This QgsAuxiliaryLayer provides the solution to this limitation. Actually
it's an editable join to the original vector layer with some
synchronisation mechanisms activated such as "Upsert On Edit" or "Delete
Cascade". Thus, auxiliary fields are editable even if the
source layer is not and edition of a joined field is also possible.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsauxiliarystorage.h"
%End
  public:

    QgsAuxiliaryLayer( const QString &pkField, const QString &filename, const QString &table, QgsVectorLayer *vlayer );
%Docstring
Constructor

:param pkField: The primary key to use for joining
:param filename: The database path
:param table: The table name
:param vlayer: The target vector layer in join definition
%End



    QgsAuxiliaryLayer *clone( QgsVectorLayer *layer ) const /Factory/;
%Docstring
Returns a new instance equivalent to this one. The underlying table
is duplicate for the layer given in parameter. Note that the current
auxiliary layer should be saved to have a proper duplicated table.

:param layer: The layer for which the clone is made
%End

    QgsVectorLayer *toSpatialLayer() const;
%Docstring
An auxiliary layer is not spatial. This method returns a spatial
representation of auxiliary data.

:return: A new spatial vector layer
%End

    bool clear();
%Docstring
Deletes all features from the layer. Changes are automatically committed
and the layer remains editable.

:return: true if changes are committed without error, false otherwise.
%End

    QgsVectorLayerJoinInfo joinInfo() const;
%Docstring
Returns information to use for joining with primary key and so on.
%End

    bool exists( const QgsPropertyDefinition &definition ) const;
%Docstring
Returns true if the property is stored in the layer already, false
otherwise.

:param definition: The property definition to check

:return: true if the property is stored, false otherwise
%End

    bool addAuxiliaryField( const QgsPropertyDefinition &definition );
%Docstring
Adds an auxiliary field for the given property. Setup for widget
editors are updated in the target layer as well as the attribute
table config to hide auxiliary fields by default.

:param definition: The definition of the property to add

:return: true if the auxiliary field is well added, false otherwise
%End

    QgsFields auxiliaryFields() const;
%Docstring
Returns a list of all auxiliary fields currently managed by the layer.
%End

    bool save();
%Docstring
Commits changes and starts editing then.

:return: true if commit step passed, false otherwise
%End

    virtual bool deleteAttribute( int attr );

%Docstring
Removes attribute from the layer and commits changes. The layer remains
editable.

:param attr: The index of the attribute to remove

:return: true if the attribute is well deleted, false otherwise
%End

    bool isHiddenProperty( int index ) const;
%Docstring
Returns true if the underlying field has to be hidden from editing
tools like attribute table, false otherwise.

:param index: The index of the field for which visibility is checked
%End

    int indexOfPropertyDefinition( const QgsPropertyDefinition &definition ) const;
%Docstring
Returns the index of the auxiliary field for a specific property
definition.

:param definition: The property definition

:return: The index of the field corresponding to the property or -1
%End

    int propertyFromIndex( int index ) const;
%Docstring
Returns the underlying property key for the field index. The key may be
a PAL, diagram or symbology property according to the underlying
property definition of the field. The key -1 is returned if an error
happened.

:param index: The index of the field
%End

    QgsPropertyDefinition propertyDefinitionFromIndex( int index ) const;
%Docstring
Returns the property definition for the underlying field index.

:param index: The index of the field
%End

    static int createProperty( QgsPalLayerSettings::Property property, QgsVectorLayer *vlayer );
%Docstring
Creates if necessary a new auxiliary field for a PAL property and
activates this property in settings.

:param property: The property to create
:param vlayer: The vector layer

:return: The index of the auxiliary field or -1
%End

    static int createProperty( QgsDiagramLayerSettings::Property property, QgsVectorLayer *vlayer );
%Docstring
Creates if necessary a new auxiliary field for a diagram's property and
activates this property in settings.

:param property: The property to create
:param vlayer: The vector layer

:return: The index of the auxiliary field or -1
%End

    static QgsField createAuxiliaryField( const QgsPropertyDefinition &definition );
%Docstring
Creates a new auxiliary field from a property definition.

:param definition: The property definition of the auxiliary field to create
%End

    static QgsField createAuxiliaryField( const QgsField &field );
%Docstring
Creates a new auxiliary field from a field.

:param field: The field to use to create the auxiliary field
%End

    static QString nameFromProperty( const QgsPropertyDefinition &def, bool joined = false );
%Docstring
Returns the name of the auxiliary field for a property definition.

:param def: The property definition
:param joined: The join prefix is taken into account if true
%End

    static QgsPropertyDefinition propertyDefinitionFromField( const QgsField &field );
%Docstring
Returns the property definition from an auxiliary field.

:param field: The auxiliary field
%End

};


class QgsAuxiliaryStorage
{
%Docstring


 Class providing some utility methods to manage auxiliary storage.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsauxiliarystorage.h"
%End
  public:

    QgsAuxiliaryStorage( const QgsProject &project, bool copy = true );
%Docstring
Constructor.

The project filename is used to build a database path at the same
location, but with a different extension. Then, it's the same logic as
described for :py:class:`QgsAuxiliaryStorage`(const QString &, bool copy).

:param project: The project for which the auxiliary storage has to be used
:param copy: Parameter indicating if a copy of the database has to be used
%End

    QgsAuxiliaryStorage( const QString &filename = QString(), bool copy = true );
%Docstring
Constructor.

If a valid database path is given in parameter and copy mode is
deactivated, then every changes is directly committed on the original
database. But if the copy mode is activated, then changes are committed
on a copy of the database (a temporary file) and a save action is
therefore necessary to keep modifications in the original file.

If an empty string for the database path is given in parameter, then
a database is created in a temporary file whatever the copy mode.

If the database path given in parameter is not empty but does not exist,
then a database is created at this location when copy mode is
deactivated. When copy mode is activated, a temporary database is used
instead and a save action will be necessary to create the database at
the original location given in parameter.

:param filename: The path of the database
:param copy: Parameter indicating if a copy of the database has to be used
%End

    virtual ~QgsAuxiliaryStorage();

    bool isValid() const;
%Docstring
Returns the status of the auxiliary storage currently defined.

:return: true if the auxiliary storage is valid, false otherwise
%End

    QString fileName() const;
%Docstring
Returns the target filename of the database.
%End

    QString currentFileName() const;
%Docstring
Returns the path of the current database used. It may be different from
the target filename if the auxiliary storage is opened in copy mode.
%End

    bool saveAs( const QString &filename ) const;
%Docstring
Saves the current database to a new path.

:return: true if everything is saved, false otherwise
%End

    bool saveAs( const QgsProject &project ) const;
%Docstring
Saves the current database to a new path for a specific project.
Actually, the current filename of the project is used to deduce the
path of the database to save.

:return: true if everything is saved, false otherwise
%End

    bool save() const;
%Docstring
Saves the current database.

:return: true if everything is saved, false otherwise
%End

    QgsAuxiliaryLayer *createAuxiliaryLayer( const QgsField &field, QgsVectorLayer *layer ) const /Factory/;
%Docstring
Creates an auxiliary layer for a vector layer. A new table is created if
necessary. The primary key to use to construct the auxiliary layer is
given in parameter.

:param field: The primary key to join
:param layer: The vector layer for which the auxiliary layer has to be created

:return: A new auxiliary layer or a None if an error happened.
%End

    static bool deleteTable( const QgsDataSourceUri &uri );
%Docstring
Removes a table from the auxiliary storage.

:param uri: The uri of the table to remove

:return: true if the table is well deleted, false otherwise
%End

    static bool duplicateTable( const QgsDataSourceUri &uri, const QString &newTable );
%Docstring
Duplicates a table and its content.

:param uri: The uri of the table to duplicate
:param newTable: The name of the new table

:return: true if the table is well duplicated, false otherwise
%End

    static QString extension();
%Docstring
Returns the extension used for auxiliary databases.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsauxiliarystorage.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
