/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsauxiliarystorage.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsAuxiliaryField : QgsField
{
%Docstring


 Class allowing to manage fields from of auxiliary layers

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsauxiliarystorage.h"
%End
  public:

    explicit QgsAuxiliaryField( const QgsPropertyDefinition &def );
%Docstring
 Constructor

 \param def Definition of the property to be stored by this auxiliary
  field.
%End

    virtual ~QgsAuxiliaryField();
%Docstring
 Destructor
%End

    QgsPropertyDefinition propertyDefinition() const;
%Docstring
 Returns the property definition corresponding to this field.
 :rtype: QgsPropertyDefinition
%End

    static QString nameFromProperty( const QgsPropertyDefinition &def, bool joined = false );
%Docstring
 Returns the name of the auxiliary field for a property definition.

 :return: def The property definition
 :return: joined The join prefix is tok into account if true
 :rtype: str
%End

};

typedef QList<QgsAuxiliaryField> QgsAuxiliaryFields;

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

    QgsAuxiliaryLayer( const QString &pkField, const QString &filename, const QString &table, const QgsVectorLayer *vlayer );
%Docstring
 Constructor

 \param pkField The primary key to use for joining
 \param filename The database path
 \param table The table name
 \param vlayer The target vector layer in join definition
%End

    virtual ~QgsAuxiliaryLayer();
%Docstring
 Destructor
%End



    QgsVectorLayer *toSpatialLayer() const;
%Docstring
 An auxiliary layer is not spatial. This method returns a spatial
 representation of auxiliary data.

 :return: A new spatial vector layer
 :rtype: QgsVectorLayer
%End

    bool clear();
%Docstring
 Deletes all features from the layer. Changes are automatically committed
 and the layer remains editable.

 :return: true if changes are committed without error, false otherwise.
 :rtype: bool
%End

    QgsVectorLayerJoinInfo joinInfo() const;
%Docstring
 Returns information to use for joining with primary key and so on.
 :rtype: QgsVectorLayerJoinInfo
%End

    bool exists( const QgsPropertyDefinition &definition ) const;
%Docstring
 Returns true if the property is stored in the layer already, false
 otherwise.

 \param definition The property definition to check

 :return: true if the property is stored, false otherwise
 :rtype: bool
%End

    bool addAuxiliaryField( const QgsPropertyDefinition &definition );
%Docstring
 Add an an auxiliary field for the given property.

 \param definition The definition of the property to add

 :return: true if the auxiliary field is well added, false otherwise
 :rtype: bool
%End

    QgsAuxiliaryFields auxiliaryFields() const;
%Docstring
 Returns a list of all auxiliary fields currently managed by the layer.
 :rtype: QgsAuxiliaryFields
%End

    bool save();
%Docstring
 Commit changes and starts editing then.

 :return: true if commit step passed, false otherwise
 :rtype: bool
%End

    virtual bool deleteAttribute( int attr );
%Docstring
 Remove attribute from the layer and commit changes. The layer remains
 editable.

 \param attr The index of the attribute to remove

 :return: true if the attribute is well deleted, false otherwise
 :rtype: bool
%End

    static int createProperty( QgsPalLayerSettings::Property property, const QString &providerId, QgsVectorLayer *vlayer );
%Docstring
 Create if necessary a new auxiliary field for a PAL property and
 activate this property in settings.

 \param property The property to create
 \param providerId The id of the provider to use
 \param vlayer The vector layer

 :return: The index of the auxiliary field or -1
 :rtype: int
%End

    static int createProperty( QgsDiagramLayerSettings::Property property, QgsVectorLayer *vlayer );
%Docstring
 Create if necessary a new auxiliary field for a diagram's property and
 activate this this property in settings.

 \param property The property to create
 \param providerId The id of the provider to use
 \param vlayer The vector layer

 :return: The index of the auxiliary field or -1
 :rtype: int
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
.. seealso:: QgsAuxiliaryStorage(const QString &, bool copy).


 \param project The project for which the auxiliary storage has to be used
 \param copy Parameter indicating if a copy of the database has to be used
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

 \param filename The path of the database
 \param copy Parameter indicating if a copy of the database has to be used
%End

    virtual ~QgsAuxiliaryStorage();
%Docstring
 Destructor.
%End

    bool isValid() const;
%Docstring
 Returns the status of the auxiliary storage currently definied.

 :return: true if the auxiliary storage is valid, false otherwise
 :rtype: bool
%End

    QString fileName() const;
%Docstring
 Returns the target filename of the database.
 :rtype: str
%End

    QString currentFileName() const;
%Docstring
 Returns the path of current database used. It may be different from the
 target filename if the auxiliary storage is opened in copy mode.
 :rtype: str
%End

    bool saveAs( const QString &filename ) const;
%Docstring
 Saves the current database to a new path.

 :return: true if everything is saved, false otherwise
 :rtype: bool
%End

    bool saveAs( const QgsProject &project ) const;
%Docstring
 Saves the current database to a new path for a specific project.
 Actually, the current filename of the project is used to deduce the
 path of the database to save.

 :return: true if everything is saved, false otherwise
 :rtype: bool
%End

    bool save() const;
%Docstring
 Saves the current database.

 :return: true if everything is saved, false otherwise
 :rtype: bool
%End

    QgsAuxiliaryLayer *createAuxiliaryLayer( const QgsField &field, const QgsVectorLayer *layer ) const /Factory/;
%Docstring
 Creates an auxiliary layer for a vector layer. A new table is created if
 necessary. The primary key to use to construct the auxiliary layer is
 given in parameter.

 \param field The primary key to join
 \param layer The vector layer for which the auxiliary layer has to be created

 :return: A new auxiliary layer or a None if an error happened.
 :rtype: QgsAuxiliaryLayer
%End

    static bool deleteTable( const QgsDataSourceUri &uri );
%Docstring
 Removes a table from the auxiliary storage.

 :return: true if the table is well deleted, false otherwise
 :rtype: bool
%End

    static QString extension();
%Docstring
 Returns the extension used for auxiliary databases.
 :rtype: str
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsauxiliarystorage.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
